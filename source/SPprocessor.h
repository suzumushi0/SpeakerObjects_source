//
// Copyright (c) 2021-2024 suzumushi
//
// 2024-3-23		SPprocessor.h
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 (CC BY-NC-SA 4.0).
//
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"

// suzumushi: 
#include "SOparam.h"
#include "SPDSPparam.h"
#include "SOudsampling.h"
#include "SO2ndordIIRfilters.h"
#include "SOpinna.h"
#include "SOLPF.h"


namespace suzumushi {

//------------------------------------------------------------------------
//  SpeakerObjectsProcessor
//------------------------------------------------------------------------
class SpeakerObjectsProcessor : public Steinberg::Vst::AudioEffect
{
public:
	SpeakerObjectsProcessor ();
	~SpeakerObjectsProcessor () SMTG_OVERRIDE;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/) 
	{ 
		return (Steinberg::Vst::IAudioProcessor*)new SpeakerObjectsProcessor; 
	}

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	
	/** Called at the end before destructor */
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	
	/** Switch the Plug-in on/off */
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;

	/** Will be called before any process call */
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
	
	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

	/** Here we go...the process call */
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
		
	/** For persistence */
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

//------------------------------------------------------------------------
private:
	// suzumushi: 
	// GUI and host facing parameters
	struct GUI_param gp;
	struct GUI_param gp_load;						// for setState ()

	// DSP facing parameters
	SPDSPparam dp; 

	// DSP instances 
	SOudsampling <double> up_down_sampling_dLL;
	SOudsampling <double> up_down_sampling_dLR;
	SOudsampling <double> up_down_sampling_dRL;
	SOudsampling <double> up_down_sampling_dRR;
	SOsphere_scattering <double> sphere_scattering_dLL;
	SOsphere_scattering <double> sphere_scattering_dLR;
	SOsphere_scattering <double> sphere_scattering_dRL;
	SOsphere_scattering <double> sphere_scattering_dRR;
	SOpinna_scattering <double> pinna_scattering_dLL;
	SOpinna_scattering <double> pinna_scattering_dLR;
	SOpinna_scattering <double> pinna_scattering_dRL;
	SOpinna_scattering <double> pinna_scattering_dRR;
	SOudsampling <double> up_down_sampling_rLL [6];
	SOudsampling <double> up_down_sampling_rLR [6];
	SOudsampling <double> up_down_sampling_rRL [6];
	SOudsampling <double> up_down_sampling_rRR [6];
	SOsphere_scattering <double> sphere_scattering_rLL [6];
	SOsphere_scattering <double> sphere_scattering_rLR [6];
	SOsphere_scattering <double> sphere_scattering_rRL [6];
	SOsphere_scattering <double> sphere_scattering_rRR [6];
	SOpinna_scattering <double> pinna_scattering_rLL [6];
	SOpinna_scattering <double> pinna_scattering_rLR [6];
	SOpinna_scattering <double> pinna_scattering_rRL [6];
	SOpinna_scattering <double> pinna_scattering_rRR [6];
	SOSLPF <double> LPF_rLL;
	SOSLPF <double> LPF_rLR;
	SOSLPF <double> LPF_rRL;
	SOSLPF <double> LPF_rRR;

	// internal functions and status
	void gui_param_loading ();				// for setState ()
	void gui_param_update (const ParamID paramID, const ParamValue paramValue);
	void reset ();	 	
	int unprocessed_len {0};				// unprocessed frame length
};

//------------------------------------------------------------------------
} // namespace suzumushi
