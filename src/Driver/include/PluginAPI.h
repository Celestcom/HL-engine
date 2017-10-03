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


	typedef uint32_t nsvr_node_id;
	typedef uint32_t nsvr_device_id;


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

	typedef enum nsvr_device_event_type {
		nsvr_device_event_unknown = 0,
		nsvr_device_event_device_connected,
		nsvr_device_event_device_disconnected,
	} nsvr_device_event_type;

	NSVR_CORE_RETURN(int) nsvr_device_event_raise(nsvr_core* core, nsvr_device_event_type type, nsvr_device_id id);
	
	
	/////////////
	// Logging //
	/////////////

	// To log a message to the core, call nsvr_log with a given severity level and component name.
	// The message will be formatted generally as [Timestamp][Severity][PluginName][PluginComponent] Message

	typedef enum nsvr_severity {
		nsvr_severity_trace,
		nsvr_severity_info,
		nsvr_severity_warning,
		nsvr_severity_error,
		nsvr_severity_fatal
	} nsvr_severity;

	NSVR_CORE_RETURN(int) nsvr_log(nsvr_core* core, nsvr_severity level, const char* component, const char* message);


	////////////////
	// Filesystem //
	////////////////

	// If your plugin needs to access files that were distributed with the plugin, then the following call will retrieve that directory.
	typedef struct nsvr_directory {
		char path[1024];
	} nsvr_directory;
	NSVR_CORE_RETURN(int) nsvr_filesystem_getdatadirectory(nsvr_core* core, nsvr_directory* outDir);


	////////////////////////
	// Device Enumeration //
	////////////////////////

	typedef struct nsvr_node_ids {
		nsvr_node_id ids[128];
		unsigned int node_count;
	} nsvr_node_ids;

	typedef enum nsvr_node_type {
		nsvr_node_type_unknown = 0,
		nsvr_node_type_haptic,
		nsvr_node_type_led,
		nsvr_node_type_inertial_tracker,
		nsvr_node_type_absolute_tracker
	} nsvr_node_type;

	typedef struct nsvr_node_info {
		nsvr_node_id id;
		nsvr_node_type type;
		char name[512];
	} nsvr_node_info;


	typedef struct nsvr_device_ids {
		nsvr_device_id ids[128];
		unsigned int device_count;
	} nsvr_device_ids;
	
	typedef enum nsvr_device_concept {
		nsvr_device_concept_unknown,
		nsvr_device_concept_suit,
		nsvr_device_concept_controller,
		nsvr_device_concept_headwear,
		nsvr_device_concept_gun,
		nsvr_device_concept_sword
	} nsvr_device_concept;

	typedef struct nsvr_device_info {
		//todo: is the id field necessary?
		nsvr_device_id id;
		char name[512];
		nsvr_device_concept concept;
	} nsvr_device_info;


	typedef struct nsvr_plugin_device_api {
		typedef void(*nsvr_device_enumeratedevices)(nsvr_device_ids*, void*);
		typedef void(*nsvr_device_enumeratenodes)(nsvr_device_id device_id, nsvr_node_ids*, void*);
		typedef void(*nsvr_device_getnodeinfo)(nsvr_node_id node_id, nsvr_node_info* info, void*);
		typedef void(*nsvr_device_getdeviceinfo)(nsvr_device_id device_id, nsvr_device_info* info, void*);

		nsvr_device_enumeratenodes enumeratenodes_handler;
		nsvr_device_enumeratedevices enumeratedevices_handler;
		nsvr_device_getdeviceinfo getdeviceinfo_handler;
		nsvr_device_getnodeinfo getnodeinfo_handler;
		void* client_data;
	} nsvr_plugin_device_api;

	NSVR_CORE_RETURN(int) nsvr_register_device_api(nsvr_core* core, nsvr_plugin_device_api* api);
	
	////////////////
	// Updateloop //
	////////////////

	// If your plugin does not require advanced threading, you may hook into an update loop instead of spawning a background thread.
	// The delta time between updates will be provided in the callback.

	typedef struct nsvr_plugin_updateloop_api {
		typedef void(*nsvr_updateloop)(uint64_t delta_time_ms, void* cd);
		nsvr_updateloop update_handler;
		void* client_data;
	} nsvr_plugin_updateloop_api;

	NSVR_CORE_RETURN(int) nsvr_register_updateloop_api(nsvr_core* core, nsvr_plugin_updateloop_api* api);

	
	//////////////////////
	// Buffered Haptics //
	//////////////////////

	// If you have a buffered-style API with calls similar to SubmitHapticData(void* amplitudes, int length), register this interface

	typedef struct nsvr_plugin_buffered_api {
		typedef void(*nsvr_buffered_submit)(uint64_t request_id, nsvr_node_id node_id, double* amplitudes, uint32_t count, void*);
		typedef void(*nsvr_buffered_getsampleduration)(double* outSampleDuration, void*);
		typedef void(*nsvr_buffered_getmaxsamples)(uint32_t* outMaxSamples, void*);
		nsvr_buffered_submit submit_handler;
		nsvr_buffered_getmaxsamples getmaxsamples_handler;
		nsvr_buffered_getsampleduration getsampleduration_handler;
		void* client_data;
	} nsvr_plugin_buffered_api;

	NSVR_CORE_RETURN(int) nsvr_register_buffered_api(nsvr_core* core, nsvr_plugin_buffered_api* api);


	//////////////////////
	// Waveform Haptics //
	//////////////////////

	// If your device has preset or canned effects, register this interface. 
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
		typedef void(*nsvr_waveform_activate_handler)(uint64_t request_id, nsvr_node_id node_id, nsvr_waveform* waveform, void* cd);
		nsvr_waveform_activate_handler activate_handler;
		void* client_data;
	} nsvr_plugin_preset_waveform_api;
	
	NSVR_CORE_RETURN(int) nsvr_register_waveform_api(nsvr_core* core, nsvr_plugin_waveform_api* api);


	//////////////////////////////
	// Haptic Playback Controls //
	//////////////////////////////

	// In order to combine haptic effects, it must be possible to pause, unpause, and cancel effects that are already in flight.

	typedef struct nsvr_plugin_playback_api {
		typedef void(*nsvr_playback_pause)(uint64_t request_id, nsvr_node_id node_id, void* client_data);
		typedef void(*nsvr_playback_unpause)(uint64_t request_id, nsvr_node_id node_id, void* client_data);
		typedef void(*nsvr_playback_cancel)(uint64_t request_id, nsvr_node_id node_id, void* client_data);

		nsvr_playback_pause pause_handler;
		nsvr_playback_unpause unpause_handler;
		nsvr_playback_cancel cancel_handler;
		void* client_data;
	} nsvr_plugin_playback_api;

	NSVR_CORE_RETURN(int) nsvr_register_playback_api(nsvr_core* core, nsvr_plugin_playback_api* api);

	//////////////
	// Tracking //
	//////////////
	typedef struct nsvr_tracking_stream nsvr_tracking_stream;

	typedef struct nsvr_quaternion {
		float w;
		float x;
		float y;
		float z;
	} nsvr_quaternion;

	typedef struct nsvr_plugin_tracking_api {
		typedef void(*nsvr_tracking_beginstreaming)(nsvr_tracking_stream* stream, nsvr_node_id node_id, void* client_data);
		typedef void(*nsvr_tracking_endstreaming)(nsvr_node_id node_id, void* client_data);

		nsvr_tracking_beginstreaming beginstreaming_handler;
		nsvr_tracking_endstreaming endstreaming_handler;
		void* client_data;
	} nsvr_plugin_tracking_api;

	NSVR_CORE_RETURN(int) nsvr_register_tracking_api(nsvr_core* core, nsvr_plugin_tracking_api* api);
	NSVR_CORE_RETURN(int) nsvr_tracking_stream_push(nsvr_tracking_stream* stream, nsvr_quaternion* quaternion);



	/////////////////////////
	// Bodygraph / Spatial //
	/////////////////////////

	typedef enum nsvr_bodypart {
		nsvr_bodypart_unknown = 0,
		nsvr_bodypart_upperarm_left ,
		nsvr_bodypart_lowerarm_left,
		nsvr_bodypart_palm_left,
		nsvr_bodypart_upperleg_left,
		nsvr_bodypart_lowerleg_left,

		nsvr_bodypart_upperarm_right ,
		nsvr_bodypart_lowerarm_right,
		nsvr_bodypart_palm_right ,
		nsvr_bodypart_upperleg_right,
		nsvr_bodypart_lowerleg_right,

		nsvr_bodypart_hips,
		nsvr_bodypart_torso,
		nsvr_bodypart_neck,
		nsvr_bodypart_head

	} nsvr_bodypart;

	static const double nsvr_location_lowest = 0.0;
	static const double nsvr_location_innermost = 0.0;
	static const double nsvr_location_highest = 1.0;
	static const double nsvr_location_outermost = 1.0;
	static const double nsvr_location_middle = 0.5;

	typedef struct nsvr_bodygraph_region nsvr_bodygraph_region;
	typedef struct nsvr_bodygraph nsvr_bodygraph;
	
	NSVR_CORE_RETURN(int) nsvr_bodygraph_region_create(nsvr_bodygraph_region** region);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_region_destroy(nsvr_bodygraph_region** region);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_region_setboundingboxdimensions(nsvr_bodygraph_region* region, double width_cm, double height_cm);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_region_setlocation(nsvr_bodygraph_region* region, nsvr_bodypart bodypart, double segment_ratio, double rotation);

	NSVR_CORE_RETURN(int) nsvr_bodygraph_createnode(nsvr_bodygraph* graph, const char* name, nsvr_bodygraph_region* region);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_connect(nsvr_bodygraph* body, const char* nodeA, const char* nodeB);

	NSVR_CORE_RETURN(int) nsvr_bodygraph_associate(nsvr_bodygraph* body, const char* node, nsvr_node_id node_id);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_unassociate(nsvr_bodygraph* body, const char* node, nsvr_node_id node_id);
	NSVR_CORE_RETURN(int) nsvr_bodygraph_clearassociations(nsvr_bodygraph* body, nsvr_node_id node_id);

	typedef struct nsvr_plugin_bodygraph_api {
		typedef void(*nsvr_bodygraph_setup)(nsvr_bodygraph* graph, void* cd);
		nsvr_bodygraph_setup setup_handler;
		void* client_data;
	} nsvr_plugin_bodygraph_api;

	NSVR_CORE_RETURN(int) nsvr_register_bodygraph_api(nsvr_core* core, nsvr_plugin_bodygraph_api* api);

	/////////////////////////
	// Rawcommand / Debug  //
	/////////////////////////

	// This is temporary. Do not rely upon this. 

	typedef struct nsvr_plugin_rawcommand_api {
		typedef void(*nsvr_rawcommand_send)(uint8_t* bytes, unsigned int length, void* client_data);
		nsvr_rawcommand_send send_handler;
		void* client_data;
	} nsvr_plugin_rawcommand_api;

	NSVR_CORE_RETURN(int) nsvr_register_rawcommand_api(nsvr_core* core, nsvr_plugin_rawcommand_api* api);





#ifdef __cplusplus
}
#endif


