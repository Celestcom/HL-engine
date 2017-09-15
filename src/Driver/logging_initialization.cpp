#include "stdafx.h"
#include "logging_initialization.h"

#include "PluginAPI.h"


#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include "logger.h"
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", nsvr_loglevel)


std::ostream& operator<< (std::ostream& strm, nsvr_loglevel level)
{
	static const char* strings[] =
	{
		"trace",
		"infodd",
		"warning",
		"error",
		"fatal"
	};

	if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
		strm << strings[level];
	else
		strm << static_cast< int >(level);

	return strm;
}



void initialize_logging() {

	boost::filesystem::path::imbue(std::locale("C"));

	namespace logging = boost::log;
	namespace sinks = boost::log::sinks;
	namespace src = boost::log::sources;
	namespace expr = boost::log::expressions;
	namespace attrs = boost::log::attributes;
	namespace keywords = boost::log::keywords;


	//We need two sinks. One sink writes everything to a file,
	//one sink writes only important things to the console. We also want a commmon formatter.

	//Here's the common formatter
	logging::formatter fmt = expr::stream
		<< std::setw(6) << std::setfill('0') <<
		expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S")
		<<std::setfill(' ') << ": <" << severity << ">\t" << expr::smessage;
		
	logging::core::get()->reset_filter();
	//Here's the console sink
	using text_sink = sinks::synchronous_sink<sinks::text_ostream_backend>;



	boost::shared_ptr<std::ostream> stream(&std::clog, boost::null_deleter());
	boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();
	sink->locked_backend()->add_stream(stream);
	sink->set_formatter(fmt);
	sink->set_filter(severity >= nsvr_loglevel_trace);

	logging::core::get()->add_sink(sink);

	sink = boost::make_shared<text_sink>();

	sink->locked_backend()->add_stream(boost::make_shared<std::ofstream>("hardlight_service.log"));
	sink->set_formatter(fmt);
	sink->locked_backend()->auto_flush(true);
	
	logging::core::get()->add_sink(sink);

	logging::add_common_attributes();

	my_logger& lg = sclogger::get();
	BOOST_LOG_SEV(lg, nsvr_loglevel_info) << "Logger initialized";

}