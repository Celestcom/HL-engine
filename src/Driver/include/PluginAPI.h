#pragma once

#include <stdint.h>
//NOTE: We will support 32 bit only for now.

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

#define NSVR_CORE_EVENT(name, version) \
struct nsvr_cevent_##name##_v##version 


#define NSVR_CORE_EVENT_LATEST(name, version) \
const unsigned int nsvr_cevent_##name##_latest = version; \
typedef struct nsvr_cevent_##name##_v##version nsvr_cevent_##name

#ifdef __cplusplus
extern "C" {
#endif


	enum NSVR_Region { NSVR_Region_None, NSVR_Region_LeftChest, NSVR_Region_RightChest };

	typedef struct NSVR_Core_Quaternion_t {
		float w;
		float x;
		float y;
		float z;
	} NSVR_Core_Quaternion;


	typedef struct NSVR_Provider_t NSVR_Provider;
	typedef struct NSVR_Plugin_t NSVR_Plugin;
	typedef struct NSVR_Core_Ctx_s nsvr_core_ctx;









	enum nsvr_pevent_type {
		nsvr_pevent_unknown = 0,
		nsvr_pevent_device_connected = 1,
		nsvr_pevent_device_disconnected = 2,
		nsvr_pevent_tracking_update = 3

	};

	enum nsvr_cevent_type {
		nsvr_cevent_type_unknown = 0,
		nsvr_cevent_type_brief_haptic = 1,
		nsvr_cevent_type_lasting_haptic = 2,
		nsvr_cevent_type_playback_statechange = 3,
		nsvr_cevent_type_realtime_request = 4
	

	};
	
	//Broadcast problem
	//The old plugins need to be able to interpret new event versions correctly
	//Because say we broadcast BriefTaxelv1. Then service upgrades and now we broadcast BriefTaxelv2.
	//Oops, old plugin behavior changed: sure, maybe they don't crash, simply ignore it. But they stop
	//working. 
	/*
	
	Principles:

	never change a struct once its in the wild
	provide backwards compatibility in the coreDLL, by sending old struct types to plugins
	who don't know about the new types
	need a translator component F(targetVersionOfStruct, latestStructArgs) -> oldStruct 
	within the plugin. So the service always intends to send the latest version, but it gets
	translated down as necessary. 
	
	
	*/

	


	typedef struct nsvr_pevent nsvr_pevent;


	typedef struct nsvr_cevent nsvr_cevent;












	NSVR_CORE_EVENT(brief_haptic, 1) {
		uint32_t effect;
		float strength;
		const char* region;
	};

	NSVR_CORE_EVENT_LATEST(brief_haptic, 1);


	NSVR_CORE_EVENT(lasting_haptic, 1) {
		uint64_t id;
		uint32_t effect;
		float strength;
		float duration;
		const char* region;
	};
	
	NSVR_CORE_EVENT_LATEST(lasting_haptic, 1);




	enum nsvr_playback_statechange_command {
		nsvr_playback_statechange_unknown = 0,
		nsvr_playback_statechange_pause = 1,
		nsvr_playback_statechange_unpause = 2,
		nsvr_playback_statechange_cancel = 3
	};


	NSVR_CORE_EVENT(playback_statechange, 1) {
		uint64_t effect_id;
		enum nsvr_playback_statechange_command command;
	};

	NSVR_CORE_EVENT_LATEST(playback_statechange, 1);



	NSVR_CORE_                                                                                             


	NSVR_PLUGIN_RETURN(int) NSVR_Configure(NSVR_Plugin* pluginPtr, nsvr_core_ctx* core);


	typedef void(*nsvr_cevent_handler)(void* event, nsvr_cevent_type type, void* user_data);

	typedef struct nsvr_cevent_callback {
		nsvr_cevent_handler handler;
		void* user_data;
	} nsvr_cevent_callback;

	typedef void(*nsvr_pevent_handler)(void* event, nsvr_pevent_type, void* user_data);
	typedef struct nsvr_pevent_callback {
		nsvr_pevent_handler handler;
		void* user_data;
	};
	/******/
	//For plugin to receive data from core
	NSVR_CORE_RETURN(int) nsvr_register_cevent_hook(nsvr_core_ctx* core, nsvr_cevent_type eventType, unsigned int targetVersion, nsvr_cevent_callback cb);


	//For Core to receive data from plugin
	//NSVR_CORE_RETURN(int) nsvr_register_polling_function(
	NSVR_PLUGIN_RETURN(int) nsvr_register_pevent_hook(NSVR_Plugin**, nsvr_pevent_type eventType, nsvr_pevent_callback cb);
 	
	NSVR_CORE_RETURN(int) nsvr_pevent_create(nsvr_pevent** event, nsvr_pevent_type type);
	NSVR_CORE_RETURN(int) nsvr_pevent_destroy(nsvr_pevent** event);
	NSVR_CORE_RETURN(int) nsvr_raise_pevent(nsvr_core_ctx* core, nsvr_pevent* event);
	

	NSVR_CORE_RETURN(int) nsvr_pevent_setdeviceid(nsvr_pevent* event, uint32_t device_id);

	//Only valid on tracking type event
	NSVR_CORE_RETURN(int) nsvr_pevent_settrackingstate(nsvr_pevent* event, NSVR_Core_Quaternion* quat);
		
		
		/******/

	
	//
	NSVR_PLUGIN_RETURN(int) NSVR_Init(NSVR_Plugin** pluginPtr);


	NSVR_PLUGIN_RETURN(int) NSVR_Free(NSVR_Plugin** ptr);



#ifdef __cplusplus
}
#endif


