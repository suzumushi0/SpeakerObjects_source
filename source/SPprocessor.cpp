//
// Copyright (c) 2021-2025 suzumushi
//
// 2025-3-20		SPprocessor.cpp
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 (CC BY-NC-SA 4.0).
//
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//

#include "SPprocessor.h"
#include "SPcids.h"

#include "base/source/fstreamer.h"

using namespace Steinberg;

namespace suzumushi {
//------------------------------------------------------------------------
// SpeakerObjectsProcessor
//------------------------------------------------------------------------
SpeakerObjectsProcessor:: SpeakerObjectsProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kSpeakerObjectsControllerUID);
}

//------------------------------------------------------------------------
SpeakerObjectsProcessor:: ~SpeakerObjectsProcessor ()
{
	//
}

//------------------------------------------------------------------------
tresult PLUGIN_API SpeakerObjectsProcessor:: initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk) {
		return result;
	}

	//--- create Audio IO ------
	// suzumushi:
	addAudioInput  (STR16 ("Input"), Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Output"), Vst::SpeakerArr::kStereo);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SpeakerObjectsProcessor:: terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API SpeakerObjectsProcessor:: setActive (TBool state)
{
	// suzumushi:
	if (state != 0)				// if (state == true)
		reset ();

	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SpeakerObjectsProcessor:: process (Vst::ProcessData& data)
{
	//--- First : Read inputs parameter changes-----------

    if (data.inputParameterChanges) {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        for (int32 index = 0; index < numParamsChanged; index++) {
            if (auto* paramQueue = data.inputParameterChanges->getParameterData (index)) {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
				// suzumushi: get the last change
				if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) == kResultTrue) {
					gui_param_update (paramQueue->getParameterId (), value);
				}
			}
		}
	}
	
	//--- Here you have to implement your processing

	gui_param_loading ();					// for setState ()

	// The first round of acoustic data processing after reset()
	if (gp.reset) {		
		dp.nrt_param_update (gp, data.outputParameterChanges, processSetup.sampleRate);
		unprocessed_len = 0;
		sphere_scattering_dLL.setup (1.0 / dp.inv_cT, dp.a_r);
		sphere_scattering_dLR.setup (1.0 / dp.inv_cT, dp.a_r);
		sphere_scattering_dRL.setup (1.0 / dp.inv_cT, dp.a_r);
		sphere_scattering_dRR.setup (1.0 / dp.inv_cT, dp.a_r);
		int SR = (processSetup.sampleRate + 0.5);
		pinna_scattering_dLL.setup_SR (SR, L_CH);
		pinna_scattering_dLR.setup_SR (SR, R_CH);
		pinna_scattering_dRL.setup_SR (SR, L_CH);
		pinna_scattering_dRR.setup_SR (SR, R_CH);
		for (int i = 0; i < 6; i++) {
			sphere_scattering_rLL [i].setup (1.0 / dp.inv_cT, dp.a_r);
			sphere_scattering_rLR [i].setup (1.0 / dp.inv_cT, dp.a_r);
			sphere_scattering_rRL [i].setup (1.0 / dp.inv_cT, dp.a_r);
			sphere_scattering_rRR [i].setup (1.0 / dp.inv_cT, dp.a_r);
			pinna_scattering_rLL [i].setup_SR (SR, L_CH);
			pinna_scattering_rLR [i].setup_SR (SR, R_CH);
			pinna_scattering_rRL [i].setup_SR (SR, L_CH);
			pinna_scattering_rRR [i].setup_SR (SR, R_CH);
		}
	}

	// numInputs == 0 and data.numOutputs == 0 mean parameters update only
	if (data.numInputs == 0 || data.numOutputs == 0) {
		return kResultOk;
	}
	// Speaker arrangements (stereo in and out are required) check.
	if (data.inputs[0].numChannels < 2 || data.outputs[0].numChannels < 2) {
		return kResultOk;
	}

	Vst::Sample32* in_L = data.inputs[0].channelBuffers32[0];
	Vst::Sample32* in_R = data.inputs[0].channelBuffers32[1];
	Vst::Sample32* out_L = data.outputs[0].channelBuffers32[0];
	Vst::Sample32* out_R = data.outputs[0].channelBuffers32[1];

	if (gp.bypass) {
		// bypass mode
		if (data.inputs[0].silenceFlags == 0)
			// all silenceFlags are false
			for (int32 i = 0; i < data.numSamples; i++) {
				*out_L++ = *in_L++;
				*out_R++ = *in_R++;
			}
		else
			// some silenceFlags are true
			for (int32 i = 0; i < data.numSamples; i++) {
				*out_L++ = *out_R++ = 0.0;
				data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;
			}
	} else {
		// DSP mode
		for (int32 i = 0; i < data.numSamples; i++) {				
			if (unprocessed_len == 0) {
				unprocessed_len = frame_len;
				dp.rt_param_update (gp, data.outputParameterChanges);
				up_down_sampling_dLL.setup (dp.inv_cT, dp.distance_L);
				up_down_sampling_dLR.setup (dp.inv_cT, dp.distance_R);
				up_down_sampling_dRL.setup (dp.inv_cT, dp.distance_R);
				up_down_sampling_dRR.setup (dp.inv_cT, dp.distance_L);
				pinna_scattering_dLL.setup (dp.theta_p, gp.hrir);
				pinna_scattering_dLR.setup (dp.theta_p, gp.hrir);
				pinna_scattering_dRL.setup (dp.theta_p, gp.hrir);
				pinna_scattering_dRR.setup (dp.theta_p, gp.hrir);
				for (int i = 0; i < 6; i++) {
					up_down_sampling_rLL [i].setup (dp.inv_cT, dp.v_distance_LL [i]);
					up_down_sampling_rLR [i].setup (dp.inv_cT, dp.v_distance_LR [i]);
					up_down_sampling_rRL [i].setup (dp.inv_cT, dp.v_distance_RL [i]);
					up_down_sampling_rRR [i].setup (dp.inv_cT, dp.v_distance_RR [i]);
					pinna_scattering_rLL [i].setup (dp.v_theta_p [i], gp.hrir);
					pinna_scattering_rLR [i].setup (dp.v_theta_p [i], gp.hrir);
					pinna_scattering_rRL [i].setup (dp.v_theta_p [i], gp.hrir);
					pinna_scattering_rRR [i].setup (dp.v_theta_p [i], gp.hrir);
				}
				LPF_rLL.setup (gp.fc, processSetup.sampleRate);
				LPF_rLR.setup (gp.fc, processSetup.sampleRate);
				LPF_rRL.setup (gp.fc, processSetup.sampleRate);
				LPF_rRR.setup (gp.fc, processSetup.sampleRate);
			}
			dp.param_smoothing ();

			double dLL, dLR, dRL, dRR;
			if (data.inputs[0].silenceFlags == 0) {
				dLL = up_down_sampling_dLL.process (*in_L, dp.decay_L);
				dLR = up_down_sampling_dLR.process (*in_L, dp.decay_R);
				dRL = up_down_sampling_dRL.process (*in_R, dp.decay_R);
				dRR = up_down_sampling_dRR.process (*in_R, dp.decay_L);
			} else {
				dLL = up_down_sampling_dLL.process (0.0, dp.decay_L);
				dLR = up_down_sampling_dLR.process (0.0, dp.decay_R);
				dRL = up_down_sampling_dRL.process (0.0, dp.decay_R);
				dRR = up_down_sampling_dRR.process (0.0, dp.decay_L);
			}
			dLL = sphere_scattering_dLL.process (dLL, dp.cos_theta_o);
			dLR = sphere_scattering_dLR.process (dLR, - dp.cos_theta_o);
			dRL = sphere_scattering_dRL.process (dRL, - dp.cos_theta_o);
			dRR = sphere_scattering_dRR.process (dRR, dp.cos_theta_o);
			dLL = pinna_scattering_dLL.process (dLL);
			dLR = pinna_scattering_dLR.process (dLR);
			dRL = pinna_scattering_dRL.process (dRL);
			dRR = pinna_scattering_dRR.process (dRR);

			double srLL = 0.0, srLR = 0.0, srRL = 0.0, srRR = 0.0;
			for (int i = 0; i < 6; i++) {
				double rLL, rLR, rRL, rRR;
				if (data.inputs[0].silenceFlags == 0) {
					rLL = up_down_sampling_rLL [i].process (*in_L, dp.v_decay_L [i]);
					rLR = up_down_sampling_rLR [i].process (*in_L, dp.v_decay_L [i]);
					rRL = up_down_sampling_rRL [i].process (*in_R, dp.v_decay_R [i]);
					rRR = up_down_sampling_rRR [i].process (*in_R, dp.v_decay_R [i]);
				} else {
					rLL = up_down_sampling_rLL [i].process (0.0, dp.v_decay_L [i]);
					rLR = up_down_sampling_rLR [i].process (0.0, dp.v_decay_L [i]);
					rRL = up_down_sampling_rRL [i].process (0.0, dp.v_decay_R [i]);
					rRR = up_down_sampling_rRR [i].process (0.0, dp.v_decay_R [i]);
				}
				rLL = sphere_scattering_rLL [i].process (rLL, dp.v_cos_theta_o_L [i]);
				rLR = sphere_scattering_rLR [i].process (rLR, - dp.v_cos_theta_o_L [i]);
				rRL = sphere_scattering_rRL [i].process (rRL, dp.v_cos_theta_o_R [i]);
				rRR = sphere_scattering_rRR [i].process (rRR, - dp.v_cos_theta_o_R [i]);
				srLL += pinna_scattering_rLL [i].process (rLL);
				srLR += pinna_scattering_rLR [i].process (rLR);
				srRL += pinna_scattering_rRL [i].process (rRL);
				srRR += pinna_scattering_rRR [i].process (rRR);
			}
			srLL = LPF_rLL.process (srLL);
			srLR = LPF_rLR.process (srLR);
			srRL = LPF_rRL.process (srRL);
			srRR = LPF_rRR.process (srRR);

			*out_L++ = (dLL + srLL + dRL + srRL) / 2.0;
			*out_R++ = (dLR + srLR + dRR + srRR) / 2.0;
			in_L++;
			in_R++;
			unprocessed_len--;
		}
	}
	return (kResultOk);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SpeakerObjectsProcessor:: setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SpeakerObjectsProcessor:: canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return (kResultFalse);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SpeakerObjectsProcessor:: setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	
	// suzumushi:
	if (gp_load.param_changed == true)
		return (kResultFalse);

	int version;
	if (streamer.readInt32 (version) == false)
		return (kResultFalse);

	if (streamer.readDouble (gp_load.s_x) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.s_y) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.s_z) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.r) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.theta) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.phi) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.xypad) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.yzpad) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.reflectance) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.fc) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.d_att) == false)
		return (kResultFalse);

	if (streamer.readDouble (gp_load.c) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.a) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.r_x) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.r_y) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.r_z) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.c_x) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.c_y) == false)
		return (kResultFalse);
	if (streamer.readDouble (gp_load.c_z) == false)
		return (kResultFalse);
	if (streamer.readInt32 (gp_load.hrir) == false)
		return (kResultFalse);

	if (streamer.readInt32 (gp_load.bypass) == false)
		return (kResultFalse);

	gp_load.param_changed = true;

	return (kResultOk);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SpeakerObjectsProcessor:: getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	// suzumushi:
	int version = 0;										// version 1.0.0
	if (streamer.writeInt32 (version) == false)				
		return (kResultFalse);

	if (streamer.writeDouble (gp.s_x) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.s_y) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.s_z) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.r) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.theta) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.phi) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.xypad) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.yzpad) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.reflectance) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.fc) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.d_att) == false)
		return (kResultFalse);

	if (streamer.writeDouble (gp.c) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.a) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.r_x) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.r_y) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.r_z) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.c_x) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.c_y) == false)
		return (kResultFalse);
	if (streamer.writeDouble (gp.c_z) == false)
		return (kResultFalse);
	if (streamer.writeInt32 (gp.hrir) == false)
		return (kResultFalse);

	if (streamer.writeInt32 (gp.bypass) == false)
		return (kResultFalse);

	return (kResultOk);
}

//------------------------------------------------------------------------
// suzumushi:

void SpeakerObjectsProcessor:: gui_param_loading ()
{
	if (gp_load.param_changed) {
		gp.s_x = gp_load.s_x;
		gp.s_y = gp_load.s_y;
		gp.s_z = gp_load.s_z;
		gp.r = gp_load.r;
		gp.theta = gp_load.theta;
		gp.phi = gp_load.phi;
		gp.xypad = gp_load.xypad;
		gp.yzpad = gp_load.yzpad;
		gp.reflectance = gp_load.reflectance;
		gp.fc = gp_load.fc;
		gp.d_att = gp_load.d_att;

		gp.c = gp_load.c;
		gp.a = gp_load.a;
		gp.r_x = gp_load.r_x;
		gp.r_y = gp_load.r_y;
		gp.r_z = gp_load.r_z;
		gp.c_x = gp_load.c_x;
		gp.c_y = gp_load.c_y;
		gp.c_z = gp_load.c_z;
		gp.hrir = gp_load.hrir;

		gp.bypass = gp_load.bypass;

		gp_load.param_changed = false;
		reset ();
	}
}

void SpeakerObjectsProcessor:: gui_param_update (const ParamID paramID, const ParamValue paramValue) 
{
	Vst::ParamValue update;

	switch (paramID) {
		case s_x.tag:
			update = rangeParameter:: toPlain (paramValue, s_x);
			if (gp.s_x != update) {
				gp.s_x = update;
				gp.param_changed = true;
			}
			break;
		case s_y.tag:
			update = rangeParameter:: toPlain (paramValue, s_y);
			if (gp.s_y != update) {
				gp.s_y = update;
				gp.param_changed = true;
			}
			break;
		case s_z.tag:
			update = rangeParameter:: toPlain (paramValue, s_z);
			if (gp.s_z != update) {
				gp.s_z = update;
				gp.param_changed = true;
			}
			break;
		case r.tag:
			update = rangeParameter:: toPlain (paramValue, r);
			if (gp.r != update) {
				gp.r = update;
				gp.param_changed = gp.r_theta_changed = true;
			}
			break;
		case theta.tag:
			update = rangeParameter:: toPlain (paramValue, theta);
			if (gp.theta != update) {
				gp.theta = update;
				gp.param_changed = gp.r_theta_changed = true;
			}
			break;
		case phi.tag:
			update = rangeParameter:: toPlain (paramValue, phi);
			if (gp.phi != update) {
				gp.phi = update;
				gp.param_changed = gp.phi_changed = true;
			}
			break;
		case xypad.tag:
			update = rangeParameter:: toPlain (paramValue, xypad);
			if (gp.xypad != update) {
				gp.xypad = update;
				gp.param_changed = gp.xypad_changed = true;
			}
			break;
		case yzpad.tag:
			update = rangeParameter:: toPlain (paramValue, yzpad);
			if (gp.yzpad != update) {
				gp.yzpad = update;
				gp.param_changed = gp.yzpad_changed = true;
			}
			break;
		case reflectance.tag:
			update = rangeParameter:: toPlain (paramValue, reflectance);
			if (gp.reflectance != update) {
				gp.reflectance = update;
				gp.param_changed = true;
			}
			break;
		case fc.tag:
			gp.fc = InfLogTaperParameter:: toPlain (paramValue, fc); 
			break;
		case d_att.tag:
			update = rangeParameter:: toPlain (paramValue, d_att);
			if (gp.d_att != update) {
				gp.d_att = update;
				gp.param_changed = true;
			}
			break;

		case c.tag:
			gp.c = rangeParameter:: toPlain (paramValue, c);
			break;
		case a.tag:
			gp.a = rangeParameter:: toPlain (paramValue, a);
			break;
		case r_x.tag:
			update = rangeParameter:: toPlain (paramValue, r_x);
			if (gp.r_x != update)
				gp.r_x = update;
			break;
		case r_y.tag:
			update = rangeParameter:: toPlain (paramValue, r_y);
			if (gp.r_y != update)
				gp.r_y = update;
			break;
		case r_z.tag:
			update = rangeParameter:: toPlain (paramValue, r_z);
			if (gp.r_z != update)
				gp.r_z = update;
			break;
		case c_x.tag:
			update = rangeParameter:: toPlain (paramValue, c_x);
			if (gp.c_x != update)
				gp.c_x = update;
			break;
		case c_y.tag:
			update = rangeParameter:: toPlain (paramValue, c_y);
			if (gp.c_y != update)
				gp.c_y = update;
			break;
		case c_z.tag:
			update = rangeParameter:: toPlain (paramValue, c_z);
			if (gp.c_z != update)
				gp.c_z = update;
			break;
		case hrir.tag:
			gp.hrir = stringListParameter:: toPlain (paramValue, (int32) HRIR_L:: LIST_LEN);
			break;

		case bypass.tag:
			gp.bypass = paramValue;
			if (! gp.bypass)				// return from bypass
				reset ();
			break;
	}
}

void SpeakerObjectsProcessor:: reset ()
{
	up_down_sampling_dLL.reset ();
	up_down_sampling_dLR.reset ();
	up_down_sampling_dRL.reset ();
	up_down_sampling_dRR.reset ();
	sphere_scattering_dLL.reset ();
	sphere_scattering_dLR.reset ();
	sphere_scattering_dRL.reset ();
	sphere_scattering_dRR.reset ();
	pinna_scattering_dLL.reset ();
	pinna_scattering_dLR.reset ();
	pinna_scattering_dRL.reset ();
	pinna_scattering_dRR.reset ();
	for (int i = 0; i < 6; i++) {
		up_down_sampling_rLL [i].reset ();
		up_down_sampling_rLR [i].reset ();
		up_down_sampling_rRL [i].reset ();
		up_down_sampling_rRR [i].reset ();
		sphere_scattering_rLL [i].reset ();
		sphere_scattering_rLR [i].reset ();
		sphere_scattering_rRL [i].reset ();
		sphere_scattering_rRR [i].reset ();
		pinna_scattering_rLL [i].reset ();
		pinna_scattering_rLR [i].reset ();
		pinna_scattering_rRL [i].reset ();
		pinna_scattering_rRR [i].reset ();
	}
	LPF_rLL.reset ();
	LPF_rLR.reset ();
	LPF_rRL.reset ();
	LPF_rRR.reset ();

	gp.reset = true;
}

//------------------------------------------------------------------------
} // namespace suzumushi
