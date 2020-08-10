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
	t_pxobject m_obj;

	double f_init;
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
	double freqext_patched;
	double f_num_channels;
	float 	f_sample_rate;

	plaits::Voice voice;
	plaits::Patch patch;
	plaits::Modulations modulations;

	plaits::Voice::Frame* obuf;
	int iobufsz;
	bool lpg;
	char shared_buffer[16 * 1024];
};


void rulos_perform64(t_rulos* self, t_object* dsp64, double** ins, long numins, double** outs, long numouts, long sampleframes, long flags, void* userparam) {

    double    *out = outs[0];   // first outlet
    double    *out2 = outs[1];   // first outlet

	if (self->iobufsz!=sampleframes)
	{
		self->iobufsz = sampleframes;
		self->obuf = new plaits::Voice::Frame[self->iobufsz];
	}

	if (!self->lpg) {
		self->patch.timbre = self->f_timbre;
		self->patch.morph = self->f_morph;
		self->patch.lpg_colour = 0.5f;
		self->patch.decay = 0.5f;
	}
	else {
		self->patch.lpg_colour = self->f_timbre;
		self->patch.decay = self->f_morph;
	}

	self->obuf = new plaits::Voice::Frame[sampleframes];
	// Render frames
	self->voice.Render(self->patch, self->modulations, self->obuf, sampleframes);
	
	for (int i = 0; i < sampleframes; i++) {
		*out++ = self->obuf[i].out / 32768.0f;
		*out2++ =self->obuf[i].aux / 32768.0f;
	}

}

void* rulos_new(void) {
	t_rulos* self = (t_rulos*)object_alloc(this_class);

	outlet_new(self, "signal");
	outlet_new(self, "signal");

	dsp_setup((t_pxobject*)self, 0);

	stmlib::BufferAllocator allocator(self->shared_buffer, sizeof(self->shared_buffer));
	self->voice.Init(&allocator);

	self->iobufsz = 64;
	self->obuf = new plaits::Voice::Frame[self->iobufsz];

	self->patch.engine = 1;
	self->patch.note = 48.0f;
	self->patch.harmonics = 0.3f;
	self->patch.timbre = 0.7f;
	self->patch.morph = 0.7f;
	self->patch.frequency_modulation_amount = 0.0f;
	self->patch.timbre_modulation_amount = 0.0f;
	self->patch.morph_modulation_amount = 0.0f;
	self->patch.decay = 0.1f;
	self->patch.lpg_colour = 0.0f;
	  
	self->modulations.note = 0.0f;
	self->modulations.engine = 0.0f;
	self->modulations.frequency = 0.0f;
	self->modulations.note = 0.0f;
	self->modulations.harmonics = 0.0f;
	self->modulations.morph = 0.0;
	self->modulations.level = 1.0f;
	self->modulations.trigger = 0.0f;
	self->modulations.frequency_patched = false;
	self->modulations.timbre_patched = false;
	self->modulations.morph_patched = false;
	self->modulations.trigger_patched = true;
	self->modulations.level_patched = false;


	self->lpg = false;
	self->patch.lpg_colour = 0.5f;
	self->patch.decay = 0.5f;
	self->f_init = 0;

	return (void *)self;
}


void rulos_free(t_rulos* self) {
	dsp_free((t_pxobject*)self);
}

void rulos_dsp64(t_rulos* self, t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
	object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
						 dsp64, gensym("dsp_add64"), (t_object*)self, (t_perfroutine64)rulos_perform64, 0, NULL);
}


void rulos_assist(t_rulos* self, void* unused, t_assist_function io, long index, char* string_dest) {
	if (io == ASSIST_INLET) {
		switch (index) {
			case 0: 
				strncpy(string_dest,"Properties", ASSIST_STRING_MAXSIZE); 
				break;
		}
	}
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

void rulos_note_patched(t_rulos *x, double f)
{
	x->modulations.frequency_patched = f>0.5f;//self->f_freq;
	if (x->freqext_patched>0.5f)
	{
		x->modulations.frequency_patched = x->freqext_patched>0.5f;
	}
}

void rulos_frequency_patched(t_rulos *x, double f){
	x->freqext_patched =  f;
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
	x->f_freq_in = constrain(f, -1.f, 1.f);
	x->modulations.frequency =  (x->f_freq_in * 6.f) - 60.f;
}

void rulos_note(t_rulos *x, double f)
{
	x->f_note = f;
	x->modulations.note = x->f_note;
	//x->modulations.note = 24.f + (x->f_note * 7.f) * 12.f;

	/*object_post(&x->x_obj, "rulos_note");
	std::string s = std::to_string(x->patch.note);
	object_post(&x->x_obj, s.c_str());*/

	//x->modulations.note = ((x->f_note * 10.f) - 3.f) * 12.f;
}

void rulos_init(t_rulos *x, double f)
{
	x->f_init = f;
}

void rulos_freq(t_rulos *x, double f)
{
  	x->f_freq = f;/*constrain(f, -3.f, 4.f);*/
	x->patch.note = x->f_freq;/*60.f + x->f_freq * 12.f;*/

	/*object_post(&x->x_obj, "rulos_freq");
	std::string s = std::to_string(x->patch.note);
	object_post(&x->x_obj, s.c_str());*/
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
	class_addmethod(this_class,(method) rulos_note_patched, "note_patched", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_frequency_patched, "frequency_patched", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_timbre_patched, "timbre_patched", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_morph_patched, "morph_patched", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_trigger_patched, "trigger_patched", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_level_patched, "level_patched", A_DEFFLOAT, 0);
	class_addmethod(this_class,(method) rulos_init, "init", A_DEFFLOAT, 0);

	/*class_addmethod(this_class,(method) rulos_freeze, "freeze", A_DEFFLOAT, 0);*/

	class_dspinit(this_class);
	class_register(CLASS_BOX, this_class);
}