#pragma once
#include "PluginAPI.h"

#include <unordered_map>
#include <memory>
#include <functional>

#include "better_enum.h"
///
/// The purpose of this file is to wrap C callbacks with ones that we can more easily call from
/// C++. Basically, we wrap each function pointer with some syntactic sugar so that we can call it just like a normal 
/// method. The way that it is done may not be the best, and should probably be upgraded, but it works.


template<typename FnPtr, typename... Arguments>
struct callback {
	FnPtr handler;
	void* user_data;
	
	callback() : user_data(nullptr) {
	
		handler = [](auto...) {
		};

		spy = [](auto...) {
		};

		cpp_fn = [](auto...) {

		};
	}
	std::function<void(Arguments...)> spy;
	std::function<void(Arguments...)> cpp_fn;

	callback(FnPtr handler, void* ud);

	// Invoke the callback
	void operator()(Arguments... arguments);
};



// Constructor takes the function pointer and a user_data void pointer.
template<typename FnPtr, typename ...Arguments>
inline callback<FnPtr, Arguments...>::callback(FnPtr handler, void * ud)
	: handler(handler)
	, user_data(ud)
	, cpp_fn([](Arguments...) {}) 
	, spy([](Arguments...) {}) {}


// Here's where we pass in the user_data
template<typename FnPtr, typename ...Arguments>
inline void callback<FnPtr, Arguments...>::operator()(Arguments ...argument)
{
	handler(std::forward<Arguments>(argument)..., user_data);
	spy(std::forward<Arguments>(argument)...);
	cpp_fn(std::forward<Arguments>(argument)...);
}


// We use a common base class so that the apis can be stored in a hashtable
// Please, do replace with a better solution if found
class plugin_api {
public:
	virtual ~plugin_api() {}
};

// Each time you add a new API, add a new entry to the enum
BETTER_ENUM(Apis, uint32_t, 
	Unknown = 0,
	Buffered,
	Preset,
	Request,
	Playback,
	Sampling,
	Device,
	RawCommand,
	Tracking,
	BodyGraph,
	Waveform,
	Updateloop,
	Diagnostics
);



struct buffered_api : public plugin_api {
	buffered_api(nsvr_plugin_buffered_api* api) 
		: submit_buffer { api->submit_handler, api->client_data }
		, submit_getmaxsamples {api->getmaxsamples_handler, api->client_data}
		, submit_getsampleduration {api->getsampleduration_handler, api->client_data}
	{}
	buffered_api() = default;
	callback<
		nsvr_plugin_buffered_api::nsvr_buffered_submit,
		uint64_t,
		nsvr_node_id,
		double*,
		uint32_t
	> submit_buffer;


	callback<
		nsvr_plugin_buffered_api::nsvr_buffered_getmaxsamples,
		uint32_t*
	> submit_getmaxsamples;

	callback<
		nsvr_plugin_buffered_api::nsvr_buffered_getsampleduration,
		double*
	> submit_getsampleduration;


	static Apis getApiType() { return Apis::Buffered; }
};


struct playback_api : public plugin_api {
	playback_api(nsvr_plugin_playback_api* api)
		: submit_pause{ api->pause_handler, api->client_data }
		, submit_cancel{ api->cancel_handler, api->client_data }
		, submit_unpause{ api->unpause_handler, api->client_data } {}
	playback_api()
		: submit_pause()
		, submit_cancel()
		, submit_unpause()
	{}
	callback<
		nsvr_plugin_playback_api::nsvr_playback_cancel, 
		uint64_t,
		nsvr_node_id
	> submit_cancel;

	callback<
		nsvr_plugin_playback_api::nsvr_playback_pause, 
		uint64_t,
		nsvr_node_id
	> submit_pause;

	callback<
		nsvr_plugin_playback_api::nsvr_playback_unpause,
		uint64_t,
		nsvr_node_id
	> submit_unpause;

	static  Apis getApiType() { return Apis::Playback; }
};




struct device_api : public plugin_api {
	device_api(nsvr_plugin_device_api* api)
		: submit_enumeratenodes{ api->enumeratenodes_handler, api->client_data }
		, submit_enumeratedevices{ api->enumeratedevices_handler, api->client_data }
		, submit_getdeviceinfo{ api->getdeviceinfo_handler, api->client_data }
		, submit_getnodeinfo{ api->getnodeinfo_handler, api->client_data }
	{}

	device_api() :
		submit_enumeratedevices(),
		submit_enumeratenodes(),
		submit_getdeviceinfo(),
		submit_getnodeinfo()
	{
	}

	callback<
		nsvr_plugin_device_api::nsvr_device_enumeratenodes,
		nsvr_device_id,
		nsvr_node_ids*
	> submit_enumeratenodes;

	callback<
		nsvr_plugin_device_api::nsvr_device_enumeratedevices,
		nsvr_device_ids*
	> submit_enumeratedevices;

	callback<
		nsvr_plugin_device_api::nsvr_device_getdeviceinfo,
		nsvr_device_id,
		nsvr_device_info*
	> submit_getdeviceinfo;

	//note: should probably also take the device id
	callback<
		nsvr_plugin_device_api::nsvr_device_getnodeinfo,
		nsvr_device_id,
		nsvr_node_id,
		nsvr_node_info*
	> submit_getnodeinfo;

	static  Apis getApiType() { return Apis::Device; }

};

struct waveform_api : public plugin_api {
	waveform_api(nsvr_plugin_waveform_api* api)
		: submit_activate{ api->activate_handler, api->client_data } {}

	waveform_api() = default;
	callback<
		nsvr_plugin_waveform_api::nsvr_waveform_activate_handler,
		uint64_t,
		nsvr_node_id,
		nsvr_waveform*
	> submit_activate;
	static Apis getApiType() { return Apis::Waveform; }
};

struct rawcommand_api : public plugin_api {
	rawcommand_api(nsvr_plugin_rawcommand_api* api)
		: submit_rawcommand{ api->send_handler, api->client_data } {}

	callback<
		nsvr_plugin_rawcommand_api::nsvr_rawcommand_send,
		uint8_t*,
		unsigned int
	> submit_rawcommand;

	static Apis getApiType() { return Apis::RawCommand; }
};

struct bodygraph_api : public plugin_api {
	bodygraph_api() = default;
	bodygraph_api(nsvr_plugin_bodygraph_api* api)
		: submit_setup{ api->setup_handler, api->client_data } {}
	callback<
		nsvr_plugin_bodygraph_api::nsvr_bodygraph_setup,
		nsvr_bodygraph*
	> submit_setup;

	static Apis getApiType() { return Apis::BodyGraph; }
};
struct tracking_api : public plugin_api {
	tracking_api(nsvr_plugin_tracking_api* api)
		: submit_beginstreaming{ api->beginstreaming_handler, api->client_data }
		, submit_endstreaming{ api->endstreaming_handler, api->client_data } {}

	tracking_api() = default;
	callback<
		nsvr_plugin_tracking_api::nsvr_tracking_beginstreaming,
		nsvr_tracking_stream*,
		nsvr_node_id
	> submit_beginstreaming;

	callback<
		nsvr_plugin_tracking_api::nsvr_tracking_endstreaming,
		nsvr_node_id
	> submit_endstreaming;

	static Apis getApiType() { return Apis::Tracking; }
};

struct updateloop_api : public plugin_api {
	updateloop_api(nsvr_plugin_updateloop_api* api)
		: submit_update{ api->update_handler, api->client_data } {}

	callback<
		nsvr_plugin_updateloop_api::nsvr_updateloop,
		uint64_t
	> submit_update;

	static Apis getApiType() { return Apis::Updateloop; }
};

struct diagnostics_api : public plugin_api {
	diagnostics_api(nsvr_plugin_diagnostics_api* api)
		: submit_updatemenu{ api->updatemenu_handler, api->client_data } {}
	callback<nsvr_plugin_diagnostics_api::nsvr_diagnostics_updatemenu,
		nsvr_diagnostics_ui*
	> submit_updatemenu;

	static Apis getApiType() { return Apis::Diagnostics; }
};

// Represents the capabilities of a particular plugin, e.g. a plugin supports
// the buffered api, the preset api, and the playback api.
// A plugin may register these apis indirectly using Register, 
// and we can retrieve and use it internally by calling GetApi<T>()
class PluginApis {
public:

	template<typename InternalApi, typename ExternalApi>
	void Register(ExternalApi* api);
	
	template<typename T>
	T* GetApi();

	bool Supports(Apis name) const;

	template<typename T>
	bool Supports() const;

	void Each(std::function<void(Apis, plugin_api*)>);

	template<typename InternalApi>
	InternalApi* ConstructDefault();
private:
	std::unordered_map<Apis::_enumerated, std::unique_ptr<plugin_api>> m_apis;
};





template<typename InternalApi, typename ExternalApi>
inline void PluginApis::Register(ExternalApi * api)
{
	auto x = std::unique_ptr<plugin_api>(new InternalApi(api));
	m_apis.emplace(std::make_pair(InternalApi::getApiType(), std::move(x)));
}

// Returns nullptr if the api identified by name is not found, or if the api is 
// found but the supplied type is wrong.
template<typename T>
inline T* PluginApis::GetApi()
{
	if (m_apis.find(T::getApiType()) != m_apis.end()) {
		auto ptr = m_apis.at(T::getApiType()).get();
		if (T* derived_ptr = dynamic_cast<T*>(ptr)) {
			return derived_ptr;
		}
	}
	return nullptr;
}

template<typename T>
inline bool PluginApis::Supports() const
{
	return m_apis.find(T::getApiType()) != m_apis.end();
}

template<typename InternalApi>
inline InternalApi* PluginApis::ConstructDefault()
{
	auto x = std::make_unique<InternalApi>();
	auto ptr = x.get();
	m_apis.emplace(std::make_pair(InternalApi::getApiType(), std::move(x)));
	return ptr;
}
