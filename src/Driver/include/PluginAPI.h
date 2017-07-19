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
#define NSVR_PLUGIN_API
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

	typedef struct nsvr_quaternion {
		float w;
		float x;
		float y;
		float z;
	} nsvr_quaternion;

	

	
	
	
	// The API requirements are divided into 3 levels: Basic, Standard, and Extended.
	// 
	// The goal is to get you up and running as soon as possible, adding additional complexity
	// when it is desired or available in your stack.
	//
	// If you implement the >>Basic API<<, we will command your hardware with the simplest primitives
	// available. This is appropriate for devices like controllers, or if your API only supports buffered or 
	// preset haptic capabilities.
	// 
	// If you implement the >>Standard API<<, we will pass off more complex events to your software for more precise
	// processing that we cannot perform. For example, the transition from a long lived-effect to a short effect requires
	// specific knowledge of your hardware and firmware. Additionally, your software will provide an interface to query the status
	// of the hardware, which can enable richer features such as emulation modes for developers. 
	//
	// If you implement the >>Extended API<<, your software promises to deliver complex playback capabilities, 
	// such as starting, stopping, and canceling effects. Additionally, this opens the door for ducking, layering,
	// and compositing operations.


	// For communicating with the core, create a device_event and call event_raise on it. 
	// See documentation specific to each of these event types.
	enum nsvr_device_event_type {
		nsvr_device_event_unknown = 0,
		nsvr_device_event_device_connected = 1,
		nsvr_device_event_device_disconnected = 2,
		nsvr_device_event_tracking_update = 3
	};

	NSVR_CORE_RETURN(int) nsvr_device_event_create(nsvr_device_event** event, nsvr_device_event_type type);
	NSVR_CORE_RETURN(int) nsvr_device_event_raise(nsvr_core* core, nsvr_device_event* event);
	NSVR_CORE_RETURN(int) nsvr_device_event_destroy(nsvr_device_event** event);

	
	//////////////////////////
	// Basic Implementation //
	//////////////////////////

	// You must first implement these three functions in your DLL:

	// Perform initialization routines such as instantiating an entry point class, and returning it
	NSVR_PLUGIN_RETURN(int) NSVR_Plugin_Init(nsvr_plugin** plugin);

	// Opportunity to register various callbacks with the core
	NSVR_PLUGIN_RETURN(int) NSVR_Plugin_Configure(nsvr_plugin* plugin, nsvr_core* core);

	// Cleanup and destroy the plugin
	NSVR_PLUGIN_RETURN(int) NSVR_Plugin_Free(nsvr_plugin** plugin);


	// If you have a buffered-style API with calls similar to SubmitHapticData(void* amplitudes, int length),
	// implement the nsvr_buffered_request interface by calling nsvr_register_buffered_handler with your callback.  

	typedef struct nsvr_buffered_request nsvr_buffered_request;
	typedef void(*nsvr_buffered_handler)(nsvr_buffered_request* request, void* client_data);

	// Registers your handler with the core
	NSVR_CORE_RETURN(int) nsvr_register_buffered_handler(nsvr_core* core, nsvr_buffered_handler handler, void* client_data);


	// If you have a "preset"-style API with calls similar to TriggerSpecialEffect() or TriggerPulse(int microseconds),
	// implement the nsvr_preset_request interface by calling nsvr_register_preset_handler with your callback.

	typedef struct nsvr_preset_request nsvr_preset_request;
	typedef void(*nsvr_preset_handler)(nsvr_preset_request* req, void* client_data);

	// Registers your handler with the core
	NSVR_CORE_RETURN(int) nsvr_register_preset_handler(nsvr_core* core, nsvr_preset_handler handler, void* client_data);
	
	// To retrieve information about a received preset_request, use the following functions.

	// A preset family specifies a certain feeling which your haptic device produces. 
	typedef enum nsvr_preset_family {
		nsvr_preset_family_unknown = 0,
		nsvr_preset_family_bump = 1,
		nsvr_preset_family_click = 2
	} nsvr_preset_family;


	// outFamily is the family of the given preset
	NSVR_CORE_RETURN(int) nsvr_preset_request_getfamily(nsvr_preset_request* req, nsvr_preset_family* outFamily);
	
	// outStrength is the strength of the given preset
	NSVR_CORE_RETURN(int) nsvr_preset_request_getstrength(nsvr_preset_request* req, float* outStrength);

	
	

	/////////////////////////////
	// Standard Implementation //
	/////////////////////////////

	// We request your software to perform complex haptic behavior through the following event types:
	enum nsvr_request_type {
		nsvr_request_type_unknown = 0,
		nsvr_request_type_brief_haptic = 1,
		nsvr_request_type_lasting_haptic = 2,
	};

	
	typedef struct nsvr_request nsvr_request;
	typedef void(*nsvr_basic_request_handler)(nsvr_request* request, void* client_data);

	// Register a your request handler for a specific nsvr_request_type with the core
	NSVR_CORE_RETURN(int) nsvr_register_request_handler(nsvr_core* core, nsvr_request_type type, nsvr_basic_request_handler handler, void* client_data);
	

	// Possible future APIs (unstable) 
	typedef struct nsvr_node nsvr_node;
	NSVR_CORE_RETURN(int) nsvr_node_create(nsvr_node** node);
	NSVR_CORE_RETURN(int) nsvr_node_setdisplayname(nsvr_node* node, const char* name);
	NSVR_CORE_RETURN(int) nsvr_node_destroy(nsvr_node** node);


	typedef struct nsvr_querystate nsvr_querystate;
	NSVR_CORE_RETURN(int) nsvr_querystate_create(nsvr_querystate** querystate);
	NSVR_CORE_RETURN(int) nsvr_querystate_register(nsvr_querystate* querystate, nsvr_core* core);
	NSVR_CORE_RETURN(int) nsvr_querystate_addnode(nsvr_querystate* state, nsvr_node* node, float x, float y, float z);
	NSVR_CORE_RETURN(int) nsvr_querystate_updatenode(nsvr_node* node, bool active);



	/////////////////////////////
	// Extended Implementation //
	/////////////////////////////

	typedef struct nsvr_playback_handle nsvr_playback_handle;
	NSVR_CORE_RETURN(int) nsvr_playback_handle_getid(nsvr_playback_handle* handle, uint64_t* outId);
	
	typedef void(*nsvr_playback_pause)(nsvr_playback_handle* handle, void* client_data);
	typedef void(*nsvr_playback_unpause)(nsvr_playback_handle* handle, void* client_data);
	typedef void(*nsvr_playback_cancel)(nsvr_playback_handle* handle, void* client_data);

	struct nsvr_playback_requirements {
		

		nsvr_playback_pause pause_fn;
		nsvr_playback_unpause unpause_fn;
		nsvr_playback_cancel cancel_fn;


		
	};

	typedef void(*nsvr_advanced_request_handler)(nsvr_playback_handle* handle, nsvr_request* request, void* client_data);
	nsvr_advanced_request_handler request_fn;


	typedef struct nsvr_device_event nsvr_device_event;

	NSVR_CORE_RETURN(int) nsvr_request_gettype(nsvr_request* cevent, nsvr_request_type* outType);
	NSVR_CORE_RETURN(int) nsvr_request_briefhaptic_geteffect(nsvr_request* cevent, uint32_t* outEffect);
	NSVR_CORE_RETURN(int) nsvr_request_briefhaptic_getstrength(nsvr_request* cevent, float* outStrength);
	NSVR_CORE_RETURN(int) nsvr_request_briefhaptic_getregion(nsvr_request* cevent, char* outRegion);




	NSVR_CORE_RETURN(int) nsvr_request_lastinghaptic_getid(nsvr_request* cevent, uint64_t* outId);
	NSVR_CORE_RETURN(int) nsvr_request_lastinghaptic_geteffect(nsvr_request* cevent, uint32_t* outEffect);
	NSVR_CORE_RETURN(int) nsvr_request_lastinghaptic_getstrength(nsvr_request* cevent, float* outStrength);
	NSVR_CORE_RETURN(int) nsvr_request_lastinghaptic_getduration(nsvr_request* cevent, float* outDuration);
	NSVR_CORE_RETURN(int) nsvr_request_lastinghaptic_getregion(nsvr_request* cevent, char* outRegion);





	




#ifdef __cplusplus
}
#endif


