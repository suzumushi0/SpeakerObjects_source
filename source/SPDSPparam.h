//
// Copyright (c) 2021-2023 suzumushi
//
// 2023-12-1		SPDSPparam.h
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 (CC BY-NC-SA 4.0).
//
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//

#pragma once

#include "SOparam.h"
#include "SOXYPad.h"

#include "pluginterfaces/vst/ivstparameterchanges.h"
using namespace Steinberg;
using namespace Vst;

#include <valarray>
using std::valarray;
#ifdef _MSC_VER			// Visual C++
#include <numbers>
using std::numbers::pi;
#else	// __clang__	// XCode C++
constexpr double pi {3.141'592'653'589'793'116};
#endif


namespace suzumushi {

// DSP facing parameters and update functions

class SPDSPparam {
public:
	void rt_param_update (struct GUI_param &gp, IParameterChanges* outParam);
	void nrt_param_update (struct GUI_param &gp, IParameterChanges* outParam, const double sampleRate);
	void param_smoothing ();

	// real-time parameters for direct wave
	double cos_theta_o {0.0};
	double theta_p {0.0};
	double distance_L {1.0};
	double distance_R {1.0};
	double decay_L {1.0};
	double decay_R {1.0};
	// real-time parameters for reflected waves
	valarray <double> v_cos_theta_o {valarray <double> (0.0, 8)};
	valarray <double> v_theta_p {valarray <double> (0.0, 8)};
	valarray <double> v_distance_L {valarray <double> (1.0, 8)};
	valarray <double> v_distance_R {valarray <double> (1.0, 8)};
	valarray <double> v_decay {valarray <double> (1.0, 8)};
	// real-time parameter for crosstalk canceller
	double sin_phiL {1.0};

	// non real-time parameters
	double inv_cT {44'100.0 / c.def};		// 1 / (cT) T: sampling interval
	double a_r {a.def};						// radius of sphere

private:
	// non real-time parameters
	double a_2 {a.def * a.def};

	// parameters for velocity limiter
	double prev_s_x {0.0};
	double prev_s_y {0.0};
	double prev_s_z {0.0};
	double v_limit {c.def * max_speed * frame_len / 44'100};

	// per face parameters (real-time)
	valarray <double> v_X {valarray <double> (8)};
	valarray <double> v_Y {valarray <double> (8)};
	valarray <double> v_Z {valarray <double> (8)};
	valarray <double> v_zdivZz {valarray <double> (8)};
	valarray <double> v_x0 {valarray <double> (8)};
	valarray <double> v_x0_2 {valarray <double> (8)};
	valarray <double> v_y0 {valarray <double> (8)};
	valarray <double> v_y0_2 {valarray <double> (8)};
	valarray <double> v_in_dist {valarray <double> (8)};
	valarray <double> v_ref_dist {valarray <double> (8)};
	valarray <double> v_dist {valarray <double> (8)};
	valarray <double> v_ref_ydir {valarray <double> (8)};
	// per face parameters (non real-time)
	valarray <double> v_z {valarray <double> (8)};
	valarray <double> v_z_2 {valarray <double> (8)};

	// parameters for smoothing (real-time)
	bool smoothing {false};					// smoothing mode
	double delta_cos_theta_o {0.0};
	double delta_decay_L {1.0};
	double delta_decay_R {1.0};
	valarray <double> v_delta_cos_theta_o {valarray <double> (0.0, 8)};
	valarray <double> v_delta_decay {valarray <double> (1.0, 8)};
};

} // namespace suzumushi