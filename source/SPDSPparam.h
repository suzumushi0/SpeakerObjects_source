//
// Copyright (c) 2021-2024 suzumushi
//
// 2024-1-4		SPDSPparam.h
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
	valarray <double> v_cos_theta_o_L {valarray <double> (0.0, 6)};
	valarray <double> v_cos_theta_o_R {valarray <double> (0.0, 6)};
	valarray <double> v_theta_p {valarray <double> (0.0, 6)};
	valarray <double> v_distance_LL {valarray <double> (1.0, 8)};
	valarray <double> v_distance_LR {valarray <double> (1.0, 8)};
	valarray <double> v_distance_RL {valarray <double> (1.0, 6)};
	valarray <double> v_distance_RR {valarray <double> (1.0, 6)};
	valarray <double> v_decay_L {valarray <double> (1.0, 6)};
	valarray <double> v_decay_R {valarray <double> (1.0, 6)};
	// real-time parameter for crosstalk canceller
	double sin_phiL {1.0};

	// non real-time parameters
	double inv_cT {44'100.0 / c.def};		// 1 / (cT) T: sampling interval
	double a_r {a.def};						// radius of sphere

private:
	// non real-time parameters
	double a_2 {a.def * a.def};
	double v_limit {c.def * max_speed * frame_len / 44'100};
	double edge [6];

	// parameters for velocity limiter
	double prev_s_x {0.0};
	double prev_s_y {0.0};
	double prev_s_z {0.0};

	// parameters for smoothing
	bool smoothing {false};					// smoothing mode
	double delta_cos_theta_o {0.0};
	double delta_decay_L {1.0};
	double delta_decay_R {1.0};
	valarray <double> v_delta_cos_theta_o_L {valarray <double> (0.0, 6)};
	valarray <double> v_delta_cos_theta_o_R {valarray <double> (0.0, 6)};
	valarray <double> v_delta_decay_L {valarray <double> (1.0, 6)};
	valarray <double> v_delta_decay_R {valarray <double> (1.0, 6)};
};

} // namespace suzumushi
