cmake_minimum_required(VERSION 3.0)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../min-api/script/min-pretarget.cmake)

include_directories( 
	"${C74_INCLUDES}"
)

include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/mi"
)

set(PLAITS_SRC
        mi/plaits/dsp/engine/additive_engine.cc
        mi/plaits/dsp/engine/bass_drum_engine.cc
        mi/plaits/dsp/engine/chord_engine.cc
        mi/plaits/dsp/engine/fm_engine.cc
        mi/plaits/dsp/engine/grain_engine.cc
        mi/plaits/dsp/engine/hi_hat_engine.cc
        mi/plaits/dsp/speech/lpc_speech_synth.cc
        mi/plaits/dsp/speech/lpc_speech_synth_controller.cc
        mi/plaits/dsp/speech/lpc_speech_synth_phonemes.cc
        mi/plaits/dsp/speech/lpc_speech_synth_words.cc
        mi/plaits/dsp/engine/modal_engine.cc
        mi/plaits/dsp/physical_modelling/modal_voice.cc
        mi/plaits/dsp/speech/naive_speech_synth.cc
        mi/plaits/dsp/engine/noise_engine.cc
        mi/plaits/dsp/engine/particle_engine.cc
        mi/plaits/dsp/physical_modelling/resonator.cc
        mi/plaits/resources.cc
        mi/plaits/dsp/speech/sam_speech_synth.cc
        mi/plaits/dsp/engine/snare_drum_engine.cc
        mi/plaits/dsp/engine/speech_engine.cc
        mi/plaits/dsp/engine/string_engine.cc
        mi/plaits/dsp/physical_modelling/string_voice.cc
        mi/plaits/dsp/physical_modelling/string.cc
        mi/plaits/dsp/engine/swarm_engine.cc
        mi/plaits/dsp/engine/virtual_analog_engine.cc
        mi/plaits/dsp/voice.cc
        mi/plaits/dsp/engine/waveshaping_engine.cc
        mi/plaits/dsp/engine/wavetable_engine.cc
)

set(MI_COMMON_SRC
       mi/stmlib/dsp/units.cc
       mi/stmlib/dsp/atan.cc 
       mi/stmlib/utils/random.cc
)

set(MIRULOLIB_SRC 
        ${MI_COMMON_SRC}
        ${PLAITS_SRC}
        )

add_library(MIPlaitsLib ${MIRULOLIB_SRC} )

add_library( 
	${PROJECT_NAME} 
	MODULE
	${PROJECT_NAME}.cpp
)

target_link_libraries(${PROJECT_NAME} MIPlaitsLib)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../min-api/script/min-posttarget.cmake)