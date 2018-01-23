#include "stdafx.h"
#include "runtime_include/NSDriverApi.h"
#include "Driver.h"
#include "Locator.h"
#include "EnumTranslator.h"
#include "logging_initialization.h"

#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)



HVR_RETURN(int) hvr_platform_create(hvr_platform** platform)
{
	Locator::initialize();
	Locator::provide(new EnumTranslator());
	
	initialize_logging();

	try {
		*platform = AS_TYPE(hvr_platform, new Driver());
		return 1;

	}
	catch (const boost::interprocess::interprocess_exception&) {
		return -1;
	}
}

HVR_RETURN(void) hvr_platform_destroy(hvr_platform** ptr)
{
	delete AS_TYPE(Driver, *ptr);
	*ptr = nullptr;
}

HVR_RETURN(int) hvr_platform_shutdown(hvr_platform * ptr)
{
	return AS_TYPE(Driver, ptr)->Shutdown();
}

HVR_RETURN(int) hvr_platform_startup(hvr_platform * ptr)
{
	return AS_TYPE(Driver, ptr)->StartThread();
}

HVR_RETURN(unsigned int) hvr_platform_getversion(void)
{
	return HVR_PLATFORM_API_VERSION;
}


HVR_RETURN(int) hvr_platform_enumerateplugins(hvr_platform* platform, hvr_plugin_list * outPlugins)
{
	return AS_TYPE(Driver, platform)->EnumeratePlugins(outPlugins);
}

HVR_RETURN(int) hvr_platform_getplugininfo(hvr_platform* platform, hvr_plugin_id id, hvr_plugin_info* outInfo)
{
	return AS_TYPE(Driver, platform)->GetPluginInfo(id, outInfo);

}

HVR_RETURN(int) hvr_platform_setupdiagnostics(hvr_platform* ptr, hvr_diagnostics_ui * api)
{
	AS_TYPE(Driver, ptr)->ProvideRenderingApi(api);
	return 1;
}

HVR_RETURN(int) hvr_platform_updatediagnostics(hvr_platform* ptr, hvr_plugin_id id)
{
	AS_TYPE(Driver, ptr)->DrawDiagnostics(id);
	return 1;
}

HVR_RETURN(int) hvr_platform_getcurrentsuitstate(hvr_platform * ptr, int * outState, uint64_t* outError)
{
	AS_TYPE(Driver, ptr)->GetCurrentSuitState(outState, outError);
	return 1;
}


