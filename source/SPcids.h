//
// Copyright (c) 2021-2023 suzumushi
//
// 2023-11-29		SPcids.h
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 (CC BY-NC-SA 4.0).
//
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace suzumushi {
//------------------------------------------------------------------------
static const Steinberg::FUID kSpeakerObjectsProcessorUID (0x822E8DBB, 0x607552E2, 0x8CD8B2DC, 0x4261C8D7);
static const Steinberg::FUID kSpeakerObjectsControllerUID (0x99E89CB2, 0xE7FD5A3D, 0xB8702BFA, 0x820EFCDF);

#define SpeakerObjectsVST3Category "Fx"

//------------------------------------------------------------------------
} // namespace suzumushi
