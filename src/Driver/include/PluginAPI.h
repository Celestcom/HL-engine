#pragma once

#include <stdint.h>

#ifdef _WIN32
#ifdef NSVR_BUILDING_CORE
#define NSVR_CORE_API __declspec(dllexport)
#define NSVR_PLUGIN_API 
#else  
#define NSVR_CORE_API __declspec(dllimport) 
#define NSVR_PLUGIN_API __declspec(dllexport)
#endif  
#else
#define NSVR_CORE_API
#define nsvr_plugin_API
#endif


#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)

#define NSVR_CORE_RETURN(ReturnType) NSVR_CORE_API ReturnType __cdecl
#define NSVR_PLUGIN_RETURN(ReturnType) NSVR_PLUGIN_API ReturnType __cdecl


#ifdef __cplusplus
extern "C" {
#endif

	// Opaque type representing your hardware plugin
	typedef struct nsvr_plugin nsvr_plugin;

	// Opaque type representing the core API provided to you
	typedef struct nsvr_core nsvr_core;

	typedef enum nsvr_region { test = 0 } nsvr_region; //todo: deperecate


	// As a plugin author, you may implement three levels of functionality.
	 
	// If you implement the Basic Requirements, we will command your hardware with the simplest primitives
	// available. This is appropriate for devices like controllers, or if your API only supports buffered or 
	// "preset" haptic capabilities.
	
	// If you implement the Standard Requirements, we will pass more complex events to your plugin for processing 
	// that we cannot perform. For example, the haptic transition from a long-lived effect to a short, abrupt effect
	// requires hardware and firmware-specific knowledge. 

	// If you implement the Extended Requirements, your plugin will deliver complex playback capabilities, 
	// such as starting, stopping, and canceling effects. Additionally, this will allow the core to perform ducking, 
	// layering, and compositing operations.



	/////////////////////////
	// Plugin Registration //
	/////////////////////////

	typedef struct nsvr_plugin_api {
		typedef int(*nsvr_plugin_init)(nsvr_plugin** plugin);
		typedef int(*nsvr_plugin_configure)(nsvr_plugin* plugin, nsvr_core* core);
		typedef int(*nsvr_plugin_free)(nsvr_plugin* plugin);

		nsvr_plugin_init init;
		nsvr_plugin_configure configure;
		nsvr_plugin_free free;
	} nsvr_plugin_api;


	// This is the only function that your plugin must export.
	NSVR_PLUGIN_RETURN(int) nsvr_plugin_register(nsvr_plugin_api* api);

	
	
	///////////////////////////
	// Raising device events //
	///////////////////////////

	// Communication with the core is achieved by raising events.
	// For instance, when a device is connected, you may raise the device_connected event.

	typedef struct nsvr_device_event nsvr_device_event;

	enum nsvr_device_event_type {
		nsvr_device_event_unknown = 0,
		nsvr_device_event_device_connected,
		nsvr_device_event_device_disconnected,
	}; 

	NSVR_CORE_RETURN(int) nsvr_device_event_raise(nsvr_core* core, nsvr_device_event_type type, uint64_t id);
	


	typedef struct nsvr_quaternion {
		float w;
		float x;
		float y;
		float z;
	} nsvr_quaternion;

	

	typedef struct nsvr_device_ids {
		uint64_t ids[128];
		unsigned int device_count;
	} nsvr_device_ids;

	typedef enum nsvr_device_capability {
		nsvr_device_capability_none = 0,
		nsvr_device_capability_preset = 1 << 0,
		nsvr_device_capability_buffered = 1 << 1,
		nsvr_device_capability_dynamic = 1 << 2
	} nsvr_device_capability;

	typedef enum nsvr_device_type {
		nsvr_device_type_unknown = 0,
		nsvr_device_type_haptic,
		nsvr_device_type_led
	} nsvr_device_type;

	typedef struct nsvr_device_basic_info {
		uint64_t id;
		uint32_t type;
		uint32_t capabilities;
		char name[512];
	} nsvr_device_basic_info;

	typedef struct nsvr_device_request nsvr_device_request;
	typedef struct nsvr_plugin_device_api {
		typedef void(*nsvr_device_enumerateids)(nsvr_device_ids*, void*);
		typedef void(*nsvr_device_getinfo)(uint64_t id, nsvr_device_basic_info* info, void*);
		nsvr_device_enumerateids enumerateids_handler;
		nsvr_device_getinfo getinfo_handler;
		void* client_data;
	} nsvr_plugin_device_api;

	NSVR_CORE_RETURN(int) nsvr_register_device_api(nsvr_core* core, nsvr_plugin_device_api* api);
	
	
	//////////////////////////
	// Basic Implementation //
	//////////////////////////

	

	// If you have a buffered-style API with calls similar to SubmitHapticData(void* amplitudes, int length),
	// implement the buffer_api interface


	typedef struct nsvr_plugin_buffered_api {
		typedef void(*nsvr_buffered_submit)(uint64_t device_id, double* amplitudes, uint32_t count, void*);
		typedef void(*nsvr_buffered_getsampleduration)(double* outSampleDuration, void*);
		typedef void(*nsvr_buffered_getmaxsamples)(uint32_t* outMaxSamples, void*);
		nsvr_buffered_submit submit_handler;
		nsvr_buffered_getmaxsamples getmaxsamples_handler;
		nsvr_buffered_getsampleduration getsampleduration_handler;
		void* client_data;
	} nsvr_plugin_buffered_api;

	NSVR_CORE_RETURN(int) nsvr_register_buffered_api(nsvr_core* core, nsvr_plugin_buffered_api* api);

	// If you have a "preset"-style API with calls similar to TriggerSpecialEffect() or TriggerPulse(int microseconds),
	// implement the preset_api interface

	typedef struct nsvr_preset nsvr_preset;

	typedef struct nsvr_plugin_preset_api {
		typedef void(*nsvr_preset_handler)(uint64_t device_id, nsvr_preset*, void*);
		nsvr_preset_handler preset_handler;
		void* client_data;
	} nsvr_plugin_preset_api;

	NSVR_CORE_RETURN(int) nsvr_register_preset_api(nsvr_core* core, nsvr_plugin_preset_api* api);
	

	// A preset family specifies a certain feeling which your haptic device produces. 
	typedef enum nsvr_default_waveform {
		nsvr_preset_family_unknown = 0,
		nsvr_preset_family_bump = 1,
		nsvr_preset_family_buzz = 2,
		nsvr_preset_family_click = 3,
		nsvr_preset_family_double_click = 4,
		nsvr_preset_family_fuzz = 5,
		nsvr_preset_family_hum = 6,
		nsvr_preset_family_pulse = 8,
		nsvr_preset_family_tick = 11,
		nsvr_preset_family_triple_click = 16
	} nsvr_default_waveform;


	typedef struct nsvr_waveform nsvr_waveform;


	NSVR_CORE_RETURN(int) nsvr_waveform_getname(nsvr_waveform* req, nsvr_default_waveform* outWaveform);
	
	NSVR_CORE_RETURN(int) nsvr_waveform_getstrength(nsvr_waveform* req, float* outStrength);

	NSVR_CORE_RETURN(int) nsvr_waveform_getrepetitions(nsvr_waveform* req, uint32_t* outRepetitions);

	
	


	typedef struct nsvr_plugin_waveform_api {
		typedef void(*nsvr_waveform_activate_handler)(uint64_t request_id, uint64_t device_id, nsvr_waveform* waveform, void* cd);
		nsvr_waveform_activate_handler activate_handler;
		void* client_data;
	} nsvr_plugin_preset_waveform_api;
	
	NSVR_CORE_RETURN(int) nsvr_register_waveform_api(nsvr_core* core, nsvr_plugin_waveform_api* api);


	/////////////////////////////
	// Standard Implementation //
	/////////////////////////////




	
	//Question: do we want to sample a device or sample a region?

	typedef struct nsvr_sampling_sample {
		float data_0;
		float data_1;
		float data_2;
		float intensity;
	} nsvr_sampling_sample;
	typedef struct nsvr_plugin_sampling_api {
		typedef void(*nsvr_sampling_querystate)(nsvr_region region, nsvr_sampling_sample* outSample, void* client_data);
		nsvr_sampling_querystate query_handler;
		void* client_data;
	} nsvr_plugin_sampling_api;
	NSVR_CORE_RETURN(int) nsvr_register_sampling_api(nsvr_core* core, nsvr_plugin_sampling_api* api);


	/////////////////////////////
	// Extended Implementation //
	/////////////////////////////


	
	typedef struct nsvr_plugin_playback_api {
		typedef void(*nsvr_playback_pause)(uint64_t request_id, void* client_data);
		typedef void(*nsvr_playback_unpause)(uint64_t request_id, void* client_data);
		typedef void(*nsvr_playback_cancel)(uint64_t request_id, void* client_data);

		nsvr_playback_pause pause_handler;
		nsvr_playback_unpause unpause_handler;
		nsvr_playback_cancel cancel_handler;
		void* client_data;
	} nsvr_plugin_playback_api;

	NSVR_CORE_RETURN(int) nsvr_register_playback_api(nsvr_core* core, nsvr_plugin_playback_api* api);



	
	typedef struct nsvr_plugin_rawcommand_api {
		typedef void(*nsvr_rawcommand_send)(uint8_t* bytes, unsigned int length, void* client_data);
		nsvr_rawcommand_send send_handler;
		void* client_data;
	} nsvr_plugin_rawcommand_api;

	NSVR_CORE_RETURN(int) nsvr_register_rawcommand_api(nsvr_core* core, nsvr_plugin_rawcommand_api* api);


	typedef struct nsvr_tracking_stream nsvr_tracking_stream;


	typedef struct nsvr_plugin_tracking_api {
		typedef void(*nsvr_tracking_beginstreaming)(nsvr_tracking_stream* stream, nsvr_region region, void* client_data);
		typedef void(*nsvr_tracking_endstreaming)(nsvr_region region, void* client_data);

		nsvr_tracking_beginstreaming beginstreaming_handler;
		nsvr_tracking_endstreaming endstreaming_handler;
		void* client_data;
	} nsvr_plugin_tracking_api;
	
	NSVR_CORE_RETURN(int) nsvr_register_tracking_api(nsvr_core* core, nsvr_plugin_tracking_api* api);
	NSVR_CORE_RETURN(int) nsvr_tracking_stream_push(nsvr_tracking_stream* stream, nsvr_quaternion* quaternion);





	typedef enum nsvr_bodypart {
		nsvr_bodypart_unknown = 0,
		nsvr_bodypart_upperarm_left ,
		nsvr_bodypart_forearm_left,
		nsvr_bodypart_palm_left,
		nsvr_bodypart_upperleg_left,
		nsvr_bodypart_lowerleg_left,

		nsvr_bodypart_upperarm_right ,
		nsvr_bodypart_forearm_right,
		nsvr_bodypart_palm_right ,
		nsvr_bodypart_upperleg_right,
		nsvr_bodypart_lowerleg_right,

		nsvr_bodypart_hips,
		nsvr_bodypart_torso,
		nsvr_bodypart_neck,
		nsvr_bodypart_head


	} nsvr_bodypart;



	typedef enum nsvr_bodypart_rotation {
		nsvr_bodypart_rotation_back = -180,
		nsvr_bodypart_rotation_front = 0,
		nsvr_bodypart_rotation_left = -90,
		nsvr_bodypart_rotation_right = 90
		 
	} nsvr_bodypart_rotation;

	typedef enum nsvr_region_relation {
		nsvr_region_relation_unknown = 0,
		nsvr_region_relation_above,
		nsvr_region_relation_below,
		nsvr_region_relation_left,
		nsvr_region_relation_right
	} nsvr_region_relation;


	static const double nsvr_location_lowest = 0.0;
	static const double nsvr_location_innermost = 0.0;
	static const double nsvr_location_highest = 1.0;
	static const double nsvr_location_outermost = 1.0;
	static const double nsvr_location_middle = 0.5;

	

	typedef struct nsvr_bodygraph_region nsvr_bodygraph_region;
	typedef struct nsvr_bodygraph nsvr_bodygraph;
	NSVR_CORE_RETURN(int) nsvr_bodygraph_region_create(nsvr_bodygraph_region** region);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_region_destroy(nsvr_bodygraph_region** region);

	NSVR_CORE_RETURN(int) nsvr_bodygraph_region_setwidthcm(nsvr_bodygraph_region* region, double centimeters);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_region_setheightcm(nsvr_bodygraph_region* region, double centimeters);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_region_setboundingboxdimensions(nsvr_bodygraph_region* region, double width_cm, double height_cm);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_createnode_absolute(nsvr_bodygraph* graph, const char* name, nsvr_bodygraph_region* region);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_connect(nsvr_bodygraph* body, const char* nodeA, const char* nodeB);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_region_setlocation(nsvr_bodygraph_region* region, nsvr_bodypart bodypart, double segment_ratio, double rotation);

	NSVR_CORE_RETURN(int) nsvr_bodygraph_associate(nsvr_bodygraph* body, const char* node, uint64_t device_id);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_unassociate(nsvr_bodygraph* body, const char* node, uint64_t device_id);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_clearassociations(nsvr_bodygraph* body, uint64_t device_id);
	typedef struct nsvr_plugin_bodygraph_api {
		typedef void(*nsvr_bodygraph_setup)(nsvr_bodygraph* graph, void* cd);
		nsvr_bodygraph_setup setup_handler;
		void* client_data;
	} nsvr_plugin_bodygraph_api;
	NSVR_CORE_RETURN(int) nsvr_register_bodygraph_api(nsvr_core* core, nsvr_plugin_bodygraph_api* api);




#ifdef __cplusplus
}
#endif


