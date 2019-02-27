#include "c74_msp.h"
#include <iostream>
#include "plaits/dsp/voice.h"

using namespace c74::max;

static t_class* this_class = nullptr;

inline double constrain(double v, double vMin, double vMax) {
	return std::max<double>(vMin, std::min<double>(vMax, v));
}

inline int constrain(int v, int vMin, int vMax) {
  return std::max<int>(vMin,std::min<int>(vMax, v));
}

inline short TO_SHORTFRAME(float v) { return (short(v * 16384.f)); }
inline float FROM_SHORTFRAME(short v) { return (float(v) / 16384.f); }

struct t_rulos {
	t_object  x_obj;

	double f_model1;
	double f_model2;
	double f_freq;
	double f_freq_cv;
	double f_freq_in;
	double f_harmonics;
	double f_timbre;
	double f_timbre_cv;
	double f_timbre_in;
	double f_morph_cv;
	double f_morph_in;
	double f_morph;
	double f_num;
	double f_engine;
	double f_trigger;
	double f_level;
	double f_note;
	double f_num_channels;
	float 	f_sample_rate;

	plaits::Voice voice;
	plaits::Patch patch;
	plaits::Modulations modulations;

	plaits::Voice::Frame* obuf;
	int iobufsz;
	bool lpg;
};

char shared_buffer[16 * 1024];

void rulos_perform64(t_rulos* self, t_object* dsp64, double** ins, long numins, double** outs, long numouts, long sampleframes, long flags, void* userparam) {
	int       blockSize = self->iobufsz;
    double    *out = outs[0];   // first outlet
    double    *out2 = outs[1];   // first outlet
    if (numouts>0)
    {
		if (!self->lpg) {
			self->patch.timbre = self->f_timbre;
			self->patch.morph = self->f_morph;
		}
		else {
			self->patch.lpg_colour = self->f_timbre;
			self->patch.decay = self->f_morph;
		}

		self->obuf = new plaits::Voice::Frame[blockSize];
		// Render frames
		self->voice.Render(self->patch, self->modulations, self->obuf, blockSize);
		
		for (int i = 0; i < blockSize; i++) {
			*out++ = FROM_SHORTFRAME(self->obuf[i].out);
			*out2++ = FROM_SHORTFRAME(self->obuf[i].aux);
		}
    }

}

void* rulos_new(void) {
	t_rulos* self = (t_rulos*)object_alloc(this_class);
	memset(shared_buffer, 0, sizeof(shared_buffer));
	stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
	self->voice.Init(&allocator);
	//memset(&self->patch, 0, sizeof(self->patch));
	//memset(&self->modulations, 0, sizeof(self->modulations));
	self->iobufsz = 64;
	self->obuf = new plaits::Voice::Frame[self->iobufsz];

	self->patch.engine = 0;
	self->lpg = false;
	self->patch.lpg_colour = 0.5f;
	self->patch.decay = 0.5f;

	outlet_new(self, "signal");
	outlet_new(self, "signal");
	inlet_new(self, NULL);
	dsp_setup((t_pxobject*)self, 0);

	return (void *)self;
}


void rulos_free(t_rulos* self) {
	delete [] self->obuf;

	dsp_free((t_pxobject*)self);
}

void rulos_dsp64(t_rulos* self, t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
	object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
						 dsp64, gensym("dsp_add64"), (t_object*)self, (t_perfroutine64)rulos_perform64, 0, NULL);
}


void rulos_assist(t_rulos* self, void* unused, t_assist_function io, long index, char* string_dest) {
	if (io == ASSIST_OUTLET) {
		switch (index) {
			case 0: 
				strncpy(string_dest,"(signal) L Output", ASSIST_STRING_MAXSIZE); 
				break;
			case 1: 
				strncpy(string_dest,"(signal) R Output", ASSIST_STRING_MAXSIZE); 
				break;
		}
	}
}

void rulos_frequency_patched(t_rulos *x, double f){
	x->modulations.frequency_patched = f>0.5f;//self->f_freq;

}

void rulos_timbre_patched(t_rulos *x, double f){
	x->modulations.timbre_patched = f>0.5f;//self->f_freq;

}

void rulos_morph_patched(t_rulos *x, double f){
	x->modulations.morph_patched = f>0.5f;//self->f_freq;

}

void rulos_trigger_patched(t_rulos *x, double f){
	x->modulations.trigger_patched = f>0.5f;//self->f_freq;

}

void rulos_level_patched(t_rulos *x, double f){
	x->modulations.level_patched = f > 0.5f;//self->f_freq;

}

void rulos_timbrein(t_rulos *x, double f){
	x->f_timbre_in = constrain(f, -1.f, 1.f);
	x->modulations.timbre = x->f_timbre_in;
}

void rulos_timbrecv(t_rulos *x, double f){
	x->f_timbre_cv = constrain(f, -1.f, 1.f);
	x->patch.timbre_modulation_amount = x->f_timbre_cv;
}

void rulos_timbre(t_rulos *x, double f){
	x->f_timbre = constrain(f, 0.f, 1.f);
	x->patch.timbre = x->f_timbre;
}

void rulos_harmonics(t_rulos *x, double f){
	x->f_harmonics = constrain(f, -1.f, 1.f);
	x->patch.harmonics = x->f_harmonics;
	//x->modulations.harmonics = x->f_harmonics * 5.f;
}

void rulos_morphin(t_rulos *x, double f){
	x->f_morph_in = constrain(f, -1.f, 1.f);
	x->modulations.morph = x->f_morph_in;
}

void rulos_morph(t_rulos *x, double f){
	x->f_morph = constrain(f, 0.f, 1.f);
	x->patch.morph = x->f_morph/* * 8.f*/;
}

void rulos_morphcv(t_rulos *x, double f){
	x->f_morph_cv = constrain(f, -1.f, 1.f);
	x->patch.morph_modulation_amount = x->f_morph_cv;
}

void rulos_trigger(t_rulos *x, double f)
{
  	x->f_trigger = constrain(f, 0.f, 1.f);
	x->modulations.trigger = x->f_trigger;//x->f_trigger / 3.f;
}

void rulos_freqcv(t_rulos *x, double f)
{
	x->f_freq_cv = constrain(f, -1.f, 1.f);
	x->patch.frequency_modulation_amount = x->f_freq_cv;
}

void rulos_freqin(t_rulos *x, double f)
{
	x->f_freq_in = constrain(f, -1.f, 1.f) * 8.f;
	x->modulations.frequency =  x->f_freq_in * 6.f;
}

void rulos_note(t_rulos *x, double f)
{
	x->f_note = constrain(f, 0.f, 8.f) / 8.f;
	x->patch.note = ((x->f_note * 10.f) - 3.f) * 12.f;
	//x->modulations.note = ((x->f_note * 10.f) - 3.f) * 12.f;
}


void rulos_freq(t_rulos *x, double f)
{
  	x->f_freq = constrain(f, -4.f, 4.f);
	x->patch.note = 60.f + x->f_freq * 12.f;
}

void rulos_model2(t_rulos *x, double f){
	x->f_engine = f;
	x->patch.engine = x->f_engine;
}
void rulos_lpg(t_rulos *x, double f)
{	
	x->lpg = f > 0.5f;
}

void rulos_level(t_rulos *x, double f)
{
	x->f_level = constrain(f, -1.f, 1.f);
	x->modulations.level = x->f_level;
}

void ext_main(void* r) {
	this_class = class_new("rulos~", (method)rulos_new, (method)rulos_free, sizeof(t_rulos), NULL, A_GIMME, 0);

	class_addmethod(this_class,(method) rulos_assist, "assist",	A_CANT,		0);
	class_addmethod(this_class,(method) rulos_dsp64, "dsp64",	A_CANT,		0);
	
	class_addmethod(this_class,(method) rulos_trigger, "trigger", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_lpg, "lpg", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_model2, "model2", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_freq, "frequency", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_freqin, "fmin", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_freqcv, "fm", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_note, "note", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_level, "level", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_timbre, "timbre", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_timbrecv, "timbrecv", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_timbrein, "timbrein", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_morph, "morph", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_morphcv, "morphcv", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_morphin, "morphin", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_harmonics, "harmonics", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_frequency_patched, "frequency_patched", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_timbre_patched, "timbre_patched", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_morph_patched, "morph_patched", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_trigger_patched, "trigger_patched", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_level_patched, "level_patched", A_DEFFLOAT, 0);

	/*class_addmethod(this_class,(method) rulos_freeze, "freeze", A_DEFFLOAT, 0);*/

	class_dspinit(this_class);
	class_register(CLASS_BOX, this_class);
}