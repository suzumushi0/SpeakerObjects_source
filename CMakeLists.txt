cmake_minimum_required(VERSION 3.14.0)

# suzumushi
option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)
unset(SMTG_CREATE_PLUGIN_LINK)
set (SMTG_CXX_STANDARD "20")

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")

set(vst3sdk_SOURCE_DIR "D:/VST/VST_SDK/vst3sdk")
if(NOT vst3sdk_SOURCE_DIR)
	message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(SpeakerObjects
	# This is your plug-in version number. Change it here only.
	# Version number symbols usable in C++ can be found in
	# source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
	VERSION 1.1.1.0 
	DESCRIPTION "SpeakerObjects"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(SpeakerObjects
	source/Aachen_KEMAR/IR_TBL_L_44.h
	source/Aachen_KEMAR/IR_TBL_R_44.h
	source/Aachen_KEMAR/IR_TBL_L_48.h
	source/Aachen_KEMAR/IR_TBL_R_48.h
	source/Aachen_KEMAR/IR_TBL_L_96.h
	source/Aachen_KEMAR/IR_TBL_R_96.h
	source/York_KEMAR/IR_TBL_L_44.h
	source/York_KEMAR/IR_TBL_R_44.h
	source/York_KEMAR/IR_TBL_L_48.h
	source/York_KEMAR/IR_TBL_R_48.h
	source/York_KEMAR/IR_TBL_L_96.h
	source/York_KEMAR/IR_TBL_R_96.h
	source/York_KU100/IR_TBL_L_44.h
	source/York_KU100/IR_TBL_R_44.h
	source/York_KU100/IR_TBL_L_48.h
	source/York_KU100/IR_TBL_R_48.h
	source/York_KU100/IR_TBL_L_96.h
	source/York_KU100/IR_TBL_R_96.h
	source/SPcids.h
	source/SPcontroller.h
	source/SPcontroller.cpp
	source/SPDSPparam.h
	source/SPDSPparam.cpp
	source/SPentry.cpp
	source/SPprocessor.h
	source/SPprocessor.cpp
	source/SO2ndordIIRfilters.h
	source/SOconfig.h
	source/SODDL.h
	source/SOextparam.h
	source/SOextparam.cpp
	source/SOLPF.h
	source/SOparam.h
	source/SOpinna.h
	source/SOTextEdit.h
	source/SOTextEdit.cpp
	source/SOTextEditcreator.cpp
	source/SOudsampling.h
	source/SOvst3editor.h
	source/SOvst3editor.cpp
	source/SOXYPad.h
	source/SOXYPad.cpp
	source/SOXYPadcreator.cpp
	source/version.h
)

#- VSTGUI Wanted ----
if(SMTG_ENABLE_VSTGUI_SUPPORT)
	target_sources(SpeakerObjects
		PRIVATE
			resource/SpeakerObjects.uidesc
	)
	target_link_libraries(SpeakerObjects
		PRIVATE
			vstgui_support
	)
	smtg_target_add_plugin_resources(SpeakerObjects
		RESOURCES
			"resource/SpeakerObjects.uidesc"
	)
endif(SMTG_ENABLE_VSTGUI_SUPPORT)
# -------------------

smtg_target_add_plugin_snapshots (SpeakerObjects
	RESOURCES
		resource/822E8DBB607552E28CD8B2DC4261C8D7_snapshot.png
		resource/822E8DBB607552E28CD8B2DC4261C8D7_snapshot_2.0x.png
)

target_link_libraries(SpeakerObjects
	PRIVATE
		sdk
)

smtg_target_configure_version_file(SpeakerObjects)

if(SMTG_MAC)
	smtg_target_set_bundle(SpeakerObjects
		BUNDLE_IDENTIFIER foobar
		COMPANY_NAME "suzumushi"
	)
	smtg_target_set_debug_executable(SpeakerObjects
		"/Applications/VST3PluginTestHost.app"
		"--pluginfolder;$(BUILT_PRODUCTS_DIR)"
	)
elseif(SMTG_WIN)
	target_sources(SpeakerObjects PRIVATE 
		resource/win32resource.rc
	)
	if(MSVC)
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SpeakerObjects)

		smtg_target_set_debug_executable(SpeakerObjects
			"$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
			"--pluginfolder \"$(OutDir)/\""
		)
	endif()
endif(SMTG_MAC)
