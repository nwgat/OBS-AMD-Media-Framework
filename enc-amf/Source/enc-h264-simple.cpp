/*
MIT License

Copyright (c) 2016 Michael Fabian Dirks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////
#include "enc-h264-simple.h"

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////
#define AMF_TEXT_H264(x) (AMF_TEXT("H264Simple." ## x))
#define AMF_TEXT_H264_T(x) obs_module_text(AMF_TEXT_H264(x))

#define AMF_VCE_H264_NAME												AMF_TEXT_H264("Name")
#define AMF_VCE_H264_PRESET												AMF_TEXT_H264("Preset")
#define AMF_VCE_H264_PRESET_RECORDING									AMF_TEXT_H264("Preset.Recording")
#define AMF_VCE_H264_PRESET_TWITCH										AMF_TEXT_H264("Preset.Twitch")
#define AMF_VCE_H264_PRESET_YOUTUBE										AMF_TEXT_H264("Preset.YouTube")
#define AMF_VCE_H264_KEYFRAME_INTERVAL									AMF_TEXT_H264("KeyframeInterval")
#define AMF_VCE_H264_QUALITY_PRESET										AMF_TEXT_H264("QualityPreset")
#define AMF_VCE_H264_QUALITY_PRESET_SPEED								AMF_TEXT_H264("QualityPreset.Speed")
#define AMF_VCE_H264_QUALITY_PRESET_BALANCED							AMF_TEXT_H264("QualityPreset.Balanced")
#define AMF_VCE_H264_QUALITY_PRESET_QUALITY								AMF_TEXT_H264("QualityPreset.Quality")
#define AMF_VCE_H264_PROFILE											AMF_TEXT_H264("Profile")
#define AMF_VCE_H264_PROFILE_BASELINE									AMF_TEXT_H264("Profile.Baseline")
#define AMF_VCE_H264_PROFILE_MAIN										AMF_TEXT_H264("Profile.Main")
#define AMF_VCE_H264_PROFILE_HIGH										AMF_TEXT_H264("Profile.High")
#define AMF_VCE_H264_PROFILE_LEVEL										AMF_TEXT_H264("Profile.Level")
#define AMF_VCE_H264_PROFILE_LEVEL2(x)									AMF_TEXT_H264("Profile.Level." vstr(x))
#define AMF_VCE_H264_RATECONTROL										AMF_TEXT_H264("RateControl")
#define AMF_VCE_H264_RATECONTROL_CQP									AMF_TEXT_H264("RateControl.CQP")
#define AMF_VCE_H264_RATECONTROL_CBR									AMF_TEXT_H264("RateControl.CBR")
#define AMF_VCE_H264_RATECONTROL_VBR_PEAK_CONSTRAINED					AMF_TEXT_H264("RateControl.VBR.Peak")
#define AMF_VCE_H264_RATECONTROL_VBR_LATENCY_CONSTRAINED				AMF_TEXT_H264("RateControl.VBR.Latency")
#define AMF_VCE_H264_BITRATE_TARGET										AMF_TEXT_H264("Bitrate.Target")
#define AMF_VCE_H264_BITRATE_PEAK										AMF_TEXT_H264("Bitrate.Peak")
#define AMF_VCE_H264_QP_MINIMUM											AMF_TEXT_H264("QP.Minimum")
#define AMF_VCE_H264_QP_MAXIMUM											AMF_TEXT_H264("QP.Maximum")
#define AMF_VCE_H264_QP_IFRAME											AMF_TEXT_H264("QP.IFrame")
#define AMF_VCE_H264_QP_PFRAME											AMF_TEXT_H264("QP.PFrame")
#define AMF_VCE_H264_QP_BFRAME											AMF_TEXT_H264("QP.BFrame")
#define AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE								AMF_TEXT_H264("UseCustomBufferSize")
#define AMF_VCE_H264_CUSTOM_BUFFER_SIZE									AMF_TEXT_H264("CustomBufferSize")
#define AMF_VCE_H264_FRAME_SKIPPING										AMF_TEXT_H264("FrameSkipping")

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin;
using namespace Plugin::AMD;
using namespace Plugin::Interface;

void Plugin::Interface::H264SimpleInterface::encoder_register() {
	static obs_encoder_info* encoder_info = new obs_encoder_info();
	std::memset(encoder_info, 0, sizeof(obs_encoder_info));
	encoder_info->id = "amd_amf_h264_simple";
	encoder_info->type = obs_encoder_type::OBS_ENCODER_VIDEO;
	encoder_info->codec = "h264";

	// Functions
	encoder_info->get_name = &get_name;
	encoder_info->get_defaults = &get_defaults;
	encoder_info->get_properties = &get_properties;
	encoder_info->create = &create;
	encoder_info->destroy = &destroy;
	encoder_info->encode = &encode;
	encoder_info->update = &update;
	encoder_info->get_video_info = &get_video_info;
	encoder_info->get_extra_data = &get_extra_data;

	obs_register_encoder(encoder_info);
}

const char* Plugin::Interface::H264SimpleInterface::get_name(void* type_data) {
	return AMF_TEXT_H264_T("Name");
}

void Plugin::Interface::H264SimpleInterface::get_defaults(obs_data_t *settings) {
	// Main Properties
	/// Preset
	obs_data_set_default_int(settings, AMF_VCE_H264_PRESET, 0);
	/// Keyframe Interval
	obs_data_set_default_int(settings, AMF_VCE_H264_KEYFRAME_INTERVAL, 2);
	/// Quality Preset
	obs_data_set_default_int(settings, AMF_VCE_H264_QUALITY_PRESET, VCEQualityPreset_Balanced);
	/// Profile
	obs_data_set_default_int(settings, AMF_VCE_H264_PROFILE, VCEProfile_Main);
	obs_data_set_default_int(settings, AMF_VCE_H264_PROFILE_LEVEL, VCEProfileLevel_41);
	/// Rate Control
	obs_data_set_default_int(settings, AMF_VCE_H264_RATECONTROL, VCERateControlMethod_CBR);
	obs_data_set_default_int(settings, AMF_VCE_H264_QP_MINIMUM, 18);
	obs_data_set_default_int(settings, AMF_VCE_H264_QP_MAXIMUM, 51);
	/// Rate Control: CBR, VBR
	obs_data_set_default_int(settings, AMF_VCE_H264_BITRATE_TARGET, 2500);
	obs_data_set_default_int(settings, AMF_VCE_H264_BITRATE_PEAK, 2500);
	/// Rate Control: Constant QP
	obs_data_set_default_int(settings, AMF_VCE_H264_QP_IFRAME, 22);
	obs_data_set_default_int(settings, AMF_VCE_H264_QP_PFRAME, 22);
	obs_data_set_default_int(settings, AMF_VCE_H264_QP_BFRAME, 22);
	/// VBV Buffer
	obs_data_set_default_bool(settings, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE, false);
	obs_data_set_default_int(settings, AMF_VCE_H264_CUSTOM_BUFFER_SIZE, 2500);
	/// Frame Skipping
	obs_data_set_default_bool(settings, AMF_VCE_H264_FRAME_SKIPPING, false);
}

obs_properties_t* Plugin::Interface::H264SimpleInterface::get_properties(void* data) {
	obs_properties* props = obs_properties_create();
	obs_property_t* list;
	obs_property_t* p;

	// Main Properties
	/// Preset
	list = obs_properties_add_list(props, AMF_VCE_H264_PRESET, obs_module_text(AMF_VCE_H264_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "", -1);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PRESET_RECORDING), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PRESET_TWITCH), 1);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PRESET_YOUTUBE), 2);
	obs_property_set_modified_callback(list, &preset_modified);

	/// Keyframe Interval
	obs_properties_add_int(props, AMF_VCE_H264_KEYFRAME_INTERVAL, obs_module_text(AMF_VCE_H264_KEYFRAME_INTERVAL), 1, 60, 1);

	/// Quality Preset
	list = obs_properties_add_list(props, AMF_VCE_H264_QUALITY_PRESET, obs_module_text(AMF_VCE_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_SPEED), Plugin::AMD::VCEQualityPreset::VCEQualityPreset_Speed);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_BALANCED), Plugin::AMD::VCEQualityPreset::VCEQualityPreset_Balanced);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_QUALITY), Plugin::AMD::VCEQualityPreset::VCEQualityPreset_Quality);

	/// Profile
	list = obs_properties_add_list(props, AMF_VCE_H264_PROFILE, obs_module_text(AMF_VCE_H264_PROFILE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_BASELINE), Plugin::AMD::VCEProfile::VCEProfile_Baseline);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_MAIN), Plugin::AMD::VCEProfile::VCEProfile_Main);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_HIGH), Plugin::AMD::VCEProfile::VCEProfile_High);

	/// Profile Level
	list = obs_properties_add_list(props, AMF_VCE_H264_PROFILE_LEVEL, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	switch (VCECapabilities::getInstance()->getEncoderCaps(VCEEncoderType_AVC)->maxProfileLevel) {
		case 52:
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(52)), VCEProfileLevel_52);
		case 51:
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(51)), VCEProfileLevel_51);
		case 50:
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(50)), VCEProfileLevel_50);
		case 42: // Some VCE 2.0 Cards.
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(42)), VCEProfileLevel_42);
		case 41: // Some APUs and VCE 1.0 Cards.
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(41)), VCEProfileLevel_41);
		default: // These should in theory be supported by all VCE 1.0 devices and APUs.
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(40)), VCEProfileLevel_40);
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(32)), VCEProfileLevel_32);
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(31)), VCEProfileLevel_31);
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(30)), VCEProfileLevel_30);
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(22)), VCEProfileLevel_22);
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(21)), VCEProfileLevel_21);
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(20)), VCEProfileLevel_20);
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(13)), VCEProfileLevel_13);
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(12)), VCEProfileLevel_12);
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(11)), VCEProfileLevel_11);
			obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2(10)), VCEProfileLevel_10);
			break;
	}

	/// Rate Control
	list = obs_properties_add_list(props, AMF_VCE_H264_RATECONTROL, obs_module_text(AMF_VCE_H264_RATECONTROL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_CQP), Plugin::AMD::H264RateControlMethod::VCERateControlMethod_CQP);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_CBR), Plugin::AMD::H264RateControlMethod::VCERateControlMethod_CBR);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_VBR_PEAK_CONSTRAINED), Plugin::AMD::H264RateControlMethod::VCERateControlMethod_VBR);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_VBR_LATENCY_CONSTRAINED), Plugin::AMD::H264RateControlMethod::VCERateControlMethod_VBR_LAT);
	obs_property_set_modified_callback(list, &ratecontrolmethod_modified);

	/// Rate Control: CBR, VBR
	obs_properties_add_int(props, AMF_VCE_H264_BITRATE_TARGET, obs_module_text(AMF_VCE_H264_BITRATE_TARGET), 10, VCECapabilities::getInstance()->getEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);
	obs_properties_add_int(props, AMF_VCE_H264_BITRATE_PEAK, obs_module_text(AMF_VCE_H264_BITRATE_PEAK), 10, VCECapabilities::getInstance()->getEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);

	/// Rate Control: Constrained QP
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_MINIMUM, obs_module_text(AMF_VCE_H264_QP_MINIMUM), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_MAXIMUM, obs_module_text(AMF_VCE_H264_QP_MAXIMUM), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_IFRAME, obs_module_text(AMF_VCE_H264_QP_IFRAME), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_PFRAME, obs_module_text(AMF_VCE_H264_QP_PFRAME), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_BFRAME, obs_module_text(AMF_VCE_H264_QP_BFRAME), 0, 51, 1);

	/// VBV Buffer
	p = obs_properties_add_bool(props, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE, obs_module_text(AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE));
	obs_properties_add_int(props, AMF_VCE_H264_CUSTOM_BUFFER_SIZE, obs_module_text(AMF_VCE_H264_CUSTOM_BUFFER_SIZE), 1, 100000, 1);
	obs_property_set_modified_callback(p, &custombuffer_modified);

	/// Frame Skipping
	obs_properties_add_bool(props, AMF_VCE_H264_FRAME_SKIPPING, obs_module_text(AMF_VCE_H264_FRAME_SKIPPING));

	return props;
}

bool Plugin::Interface::H264SimpleInterface::preset_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	switch (obs_data_get_int(settings, AMF_VCE_H264_PRESET)) {
		case 0: // Recording
			obs_data_set_int(settings, AMF_VCE_H264_KEYFRAME_INTERVAL, 1);
			obs_data_set_int(settings, AMF_VCE_H264_QUALITY_PRESET, VCEQualityPreset_Balanced);
			obs_data_set_int(settings, AMF_VCE_H264_PROFILE, VCEProfile_High);
			obs_data_set_int(settings, AMF_VCE_H264_PROFILE_LEVEL, VCECapabilities::getInstance()->getEncoderCaps(VCEEncoderType_AVC)->maxProfileLevel);
			obs_data_set_int(settings, AMF_VCE_H264_RATECONTROL, VCERateControlMethod_VBR_LAT);
			obs_data_set_int(settings, AMF_VCE_H264_BITRATE_TARGET, 10000);
			obs_data_set_int(settings, AMF_VCE_H264_BITRATE_PEAK, VCECapabilities::getInstance()->getEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000);
			obs_data_set_int(settings, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE, false);
			obs_data_set_int(settings, AMF_VCE_H264_FRAME_SKIPPING, false);
			break;
		case 1: // Twitch
			obs_data_set_int(settings, AMF_VCE_H264_KEYFRAME_INTERVAL, 2);
			obs_data_set_int(settings, AMF_VCE_H264_QUALITY_PRESET, VCEQualityPreset_Speed);
			obs_data_set_int(settings, AMF_VCE_H264_PROFILE, VCEProfile_Main);
			obs_data_set_int(settings, AMF_VCE_H264_PROFILE_LEVEL, VCEProfileLevel_41);
			obs_data_set_int(settings, AMF_VCE_H264_RATECONTROL, VCERateControlMethod_CBR);
			obs_data_set_int(settings, AMF_VCE_H264_BITRATE_TARGET, 2500);
			obs_data_set_int(settings, AMF_VCE_H264_BITRATE_PEAK, 3500);
			obs_data_set_int(settings, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE, true);
			obs_data_set_int(settings, AMF_VCE_H264_CUSTOM_BUFFER_SIZE, 3000);
			obs_data_set_int(settings, AMF_VCE_H264_FRAME_SKIPPING, true);
			break;
		case 2: // YouTube
			obs_data_set_int(settings, AMF_VCE_H264_KEYFRAME_INTERVAL, 2);
			obs_data_set_int(settings, AMF_VCE_H264_QUALITY_PRESET, VCEQualityPreset_Speed);
			obs_data_set_int(settings, AMF_VCE_H264_PROFILE, VCEProfile_High);
			obs_data_set_int(settings, AMF_VCE_H264_PROFILE_LEVEL, VCEProfileLevel_42);
			obs_data_set_int(settings, AMF_VCE_H264_RATECONTROL, VCERateControlMethod_VBR);
			obs_data_set_int(settings, AMF_VCE_H264_BITRATE_TARGET, 3000);
			obs_data_set_int(settings, AMF_VCE_H264_BITRATE_PEAK, 6000);
			obs_data_set_int(settings, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE, true);
			obs_data_set_int(settings, AMF_VCE_H264_CUSTOM_BUFFER_SIZE, 3000);
			obs_data_set_int(settings, AMF_VCE_H264_FRAME_SKIPPING, true);
			break;
	}
	obs_data_set_int(settings, AMF_VCE_H264_PRESET, -1);
	return true;
}

bool Plugin::Interface::H264SimpleInterface::ratecontrolmethod_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	// Reset State
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_PEAK), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_TARGET), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_IFRAME), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_PFRAME), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_BFRAME), false);

	switch (obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL)) {
		case Plugin::AMD::H264RateControlMethod::VCERateControlMethod_CQP:
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_IFRAME), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_PFRAME), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_BFRAME), true);
			break;
		case Plugin::AMD::H264RateControlMethod::VCERateControlMethod_CBR:
		case Plugin::AMD::H264RateControlMethod::VCERateControlMethod_VBR:
		case Plugin::AMD::H264RateControlMethod::VCERateControlMethod_VBR_LAT:
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_PEAK), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_TARGET), true);
			break;
	}
	return true;
}

bool Plugin::Interface::H264SimpleInterface::custombuffer_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_CUSTOM_BUFFER_SIZE), false);
	if (obs_data_get_bool(settings, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE))
		obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_CUSTOM_BUFFER_SIZE), true);

	return true;
}

void* Plugin::Interface::H264SimpleInterface::create(obs_data_t* settings, obs_encoder_t* encoder) {
	try {
		return new Plugin::Interface::H264SimpleInterface(settings, encoder);
	} catch (std::exception e) {
		return NULL;
	}
}

void Plugin::Interface::H264SimpleInterface::destroy(void* data) {
	delete (static_cast<Plugin::Interface::H264SimpleInterface*>(data));
	data = nullptr;
}

bool Plugin::Interface::H264SimpleInterface::update(void *data, obs_data_t *settings) {
	return static_cast<Plugin::Interface::H264SimpleInterface*>(data)->update(settings);
}

bool Plugin::Interface::H264SimpleInterface::encode(void *data, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	return static_cast<Plugin::Interface::H264SimpleInterface*>(data)->encode(frame, packet, received_packet);
}

void Plugin::Interface::H264SimpleInterface::get_video_info(void *data, struct video_scale_info *info) {
	static_cast<Plugin::Interface::H264SimpleInterface*>(data)->get_video_info(info);
}

bool Plugin::Interface::H264SimpleInterface::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {
	return static_cast<Plugin::Interface::H264SimpleInterface*>(data)->get_extra_data(extra_data, size);
}

//////////////////////////////////////////////////////////////////////////
// Module Code
//////////////////////////////////////////////////////////////////////////
Plugin::Interface::H264SimpleInterface::H264SimpleInterface(obs_data_t* settings, obs_encoder_t* encoder) {
	int32_t width, height, fpsNum, fpsDen;

	// OBS Settings
	width = obs_encoder_get_width(encoder);
	height = obs_encoder_get_height(encoder);
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);
	fpsNum = voi->fps_num;
	fpsDen = voi->fps_den;

	// Encoder Static Parameters
	m_VideoEncoder = new Plugin::AMD::VCEEncoder(VCEEncoderType_AVC, VCEMemoryType_Host);
	switch (voi->format) {
		case VIDEO_FORMAT_NV12:
			m_VideoEncoder->SetInputSurfaceFormat(VCESurfaceFormat_NV12);
			m_VideoEncoder->SetOutputSurfaceFormat(VCESurfaceFormat_NV12);
			break;
		case VIDEO_FORMAT_I420:
			m_VideoEncoder->SetInputSurfaceFormat(VCESurfaceFormat_I420);
			m_VideoEncoder->SetOutputSurfaceFormat(VCESurfaceFormat_I420);
			break;
		case VIDEO_FORMAT_RGBA:
			m_VideoEncoder->SetInputSurfaceFormat(VCESurfaceFormat_RGBA);
			m_VideoEncoder->SetOutputSurfaceFormat(VCESurfaceFormat_RGBA);
			break;
	}

	/// Encoder Static Parameters
	m_VideoEncoder->SetUsage(VCEUsage_Transcoding);
	m_VideoEncoder->SetProfile((VCEProfile)obs_data_get_int(settings, AMF_VCE_H264_PROFILE));
	if ((height > 1080) || (width > 1920)) {
		switch (Plugin::AMD::VCECapabilities::getInstance()->getEncoderCaps(VCEEncoderType_AVC)->maxProfileLevel) {
			case 50:
				m_VideoEncoder->SetProfileLevel(VCEProfileLevel_50);
				break;
			case 51:
				m_VideoEncoder->SetProfileLevel(VCEProfileLevel_51);
				break;
			case 52:
				m_VideoEncoder->SetProfileLevel(VCEProfileLevel_52);
				break;
		}
	}

	/// Encoder Resolution Parameters
	m_VideoEncoder->SetFrameSize(width, height);
	m_VideoEncoder->SetFrameRate(fpsNum, fpsDen);

	/// Encoder Rate Control
	//m_VideoEncoder->SetTargetBitrate((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_BITRATE_TARGET) * 1000);
	//m_VideoEncoder->SetPeakBitrate((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_BITRATE_PEAK) * 1000);
	m_VideoEncoder->SetRateControlMethod((H264RateControlMethod)obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL));
	m_VideoEncoder->SetRateControlSkipFrameEnabled(obs_data_get_bool(settings, AMF_VCE_H264_FRAME_SKIPPING));
	if (obs_data_get_bool(settings, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE))
		m_VideoEncoder->SetVBVBufferSize((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_CUSTOM_BUFFER_SIZE) * 1000);
	m_VideoEncoder->SetInitialVBVBufferFullness(1.0);
	m_VideoEncoder->SetMinimumQP((uint8_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM));
	m_VideoEncoder->SetMaximumQP((uint8_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MAXIMUM));
	if (obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL) != VCERateControlMethod_CQP) {
		m_VideoEncoder->SetTargetBitrate((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_BITRATE_TARGET) * 1000);
		m_VideoEncoder->SetPeakBitrate((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_BITRATE_PEAK) * 1000);
	}
	if (obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL) == VCERateControlMethod_CQP) {
		m_VideoEncoder->SetIFrameQP((uint8_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM));
		m_VideoEncoder->SetPFrameQP((uint8_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM));
		m_VideoEncoder->SetBFrameQP((uint8_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM));
	}
	//m_VideoEncoder->SetEnforceHRDRestrictionsEnabled(false);
	m_VideoEncoder->SetFillerDataEnabled(true);

	/// Encoder Picture Control Parameters
	//m_VideoEncoder->SetHeaderInsertionSpacing((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_KEYFRAME_INTERVAL) * (uint32_t)((double_t)fpsNum / (double_t)fpsDen));
	m_VideoEncoder->SetIDRPeriod((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_KEYFRAME_INTERVAL) * (uint32_t)((double_t)fpsNum / (double_t)fpsDen));
	//m_VideoEncoder->SetDeBlockingFilterEnabled(false);


	/// Encoder Miscellaneous Parameters
	//m_VideoEncoder->SetScanType(H264ScanType_Progressive);
	m_VideoEncoder->SetQualityPreset((VCEQualityPreset)obs_data_get_int(settings, AMF_VCE_H264_QUALITY_PRESET));

	/// Encoder Motion Estimation Parameters
	m_VideoEncoder->SetHalfPixelMotionEstimationEnabled(true);
	m_VideoEncoder->SetQuarterPixelMotionEstimationEnabled(true);

	/// Encoder SVC Parameters (Only Webcam Usage)

	//////////////////////////////////////////////////////////////////////////
	// Verify
	try {
		AMF_LOG_INFO("Verify Settings:");
		/// Internal
		try {
			AMF_LOG_INFO("	Surface Format (Input): %d", m_VideoEncoder->GetInputSurfaceFormat());
			AMF_LOG_INFO("	Surface Format (Output): %d", m_VideoEncoder->GetOutputSurfaceFormat());
		} catch (...) {}
		/// Encoder Static Parameters
		try {
			AMF_LOG_INFO("	Usage: %d", m_VideoEncoder->GetUsage());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Profile: %d", m_VideoEncoder->GetProfile());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Profile Level: %d", m_VideoEncoder->GetProfileLevel());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Max LTR Frames: %d", m_VideoEncoder->GetMaxLTRFrames());
		} catch (...) {}
		/// Encoder Resolution Parameters
		try {
			AMF_LOG_INFO("	Frame Size: %dx%d", m_VideoEncoder->GetFrameSize().first, m_VideoEncoder->GetFrameSize().second);
		} catch (...) {}
		/// Encoder Rate Control
		try {
			AMF_LOG_INFO("	Target Bitrate: %d", m_VideoEncoder->GetTargetBitrate());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Peak Bitrate: %d", m_VideoEncoder->GetPeakBitrate());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Rate Control Method: %d", m_VideoEncoder->GetRateControlMethod());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Frame Skipping: %s", m_VideoEncoder->IsRateControlSkipFrameEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Min QP: %d", m_VideoEncoder->GetMinimumQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Max QP: %d", m_VideoEncoder->GetMaximumQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	I-QP: %d", m_VideoEncoder->GetIFrameQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	P-QP: %d", m_VideoEncoder->GetPFrameQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	B-QP: %d", m_VideoEncoder->GetBFrameQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Frame Rate: %d/%d", m_VideoEncoder->GetFrameRate().first, m_VideoEncoder->GetFrameRate().second);
		} catch (...) {}
		try {
			AMF_LOG_INFO("	VBV Buffer Size: %d", m_VideoEncoder->GetVBVBufferSize());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	VBV Buffer Fullness: %f", m_VideoEncoder->GetInitialVBVBufferFullness());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Enforce HRD: %s", m_VideoEncoder->IsEnforceHRDRestrictionsEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Filler Data: %s", m_VideoEncoder->IsFillerDataEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Max AU Size: %d", m_VideoEncoder->GetMaximumAccessUnitSize());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	BPic Delta QP: %d", m_VideoEncoder->GetBPictureDeltaQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Ref BPic Delta QP: %d", m_VideoEncoder->GetReferenceBPictureDeltaQP());
		} catch (...) {}
		/// Encoder Picture Control Parameters
		try {
			AMF_LOG_INFO("	Header Insertion Spacing: %d", m_VideoEncoder->GetHeaderInsertionSpacing());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	IDR Period: %d", m_VideoEncoder->GetIDRPeriod());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Deblocking: %s", m_VideoEncoder->IsDeBlockingFilterEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Intra-REfresh MBs: %d", m_VideoEncoder->GetIntraRefreshMBsNumberPerSlot());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Slices per Frame: %d", m_VideoEncoder->GetSlicesPerFrame());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	BPic Count: %d", m_VideoEncoder->GetBPicturesPattern());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	B-Frame Reference: %s", m_VideoEncoder->IsBReferenceEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		/// Encoder Miscellaneos Parameters
		try {
			AMF_LOG_INFO("	Scan Type: %d", m_VideoEncoder->GetScanType());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Quality Preset: %d", m_VideoEncoder->GetQualityPreset());
		} catch (...) {}
		/// Encoder Motion Estimation Parameters
		try {
			AMF_LOG_INFO("	Half-Pixel Motion: %s", m_VideoEncoder->IsHalfPixelMotionEstimationEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Quarter-Pixel Motion: %s", m_VideoEncoder->IsQuarterPixelMotionEstimationEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		/// Encoder SVC Parameters (Only Webcam Usage)
		try {
			AMF_LOG_INFO("	Temporal Enhancement Layers: %d", m_VideoEncoder->GetNumberOfTemporalEnhancementLayers());
		} catch (...) {}


		/*try {
			AMF_LOG_INFO("	GOP Size: %d", m_VideoEncoder->GetGOPSize());
		} catch (...) {}*/
	} catch (...) {}

	m_VideoEncoder->Start();
}

Plugin::Interface::H264SimpleInterface::~H264SimpleInterface() {
	m_VideoEncoder->Stop();
	delete m_VideoEncoder;
}

bool Plugin::Interface::H264SimpleInterface::update(obs_data_t* settings) {
	return false;
}

bool Plugin::Interface::H264SimpleInterface::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	bool retVal = true;
	retVal = m_VideoEncoder->SendInput(frame);
	m_VideoEncoder->GetOutput(packet, received_packet);
	return retVal;
}

void Plugin::Interface::H264SimpleInterface::get_video_info(struct video_scale_info* info) {
	m_VideoEncoder->GetVideoInfo(info);
}

bool Plugin::Interface::H264SimpleInterface::get_extra_data(uint8_t** extra_data, size_t* size) {
	return m_VideoEncoder->GetExtraData(extra_data, size);
}
