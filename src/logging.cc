#include "logging.h"
long lineNumber = 1;
//Defines a global logger initialization routine
BOOST_LOG_GLOBAL_LOGGER_INIT(my_logger, logger_t)
{
    logger_t lg;

    logging::add_common_attributes();

    // logging::core::get()->add_global_attribute("Scope", attrs::named_scope());

    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::trace
    );
    logging::core::get()->add_global_attribute("clientIp", attrs::mutable_constant<std::string>(""));
    logging::core::get()->add_global_attribute("reqPath", attrs::mutable_constant<std::string>(""));
    logging::core::get()->add_global_attribute("resCode", attrs::mutable_constant<std::string>(""));

    /* log formatter:
     * [TimeStamp][ClientIp] [ThreadId] [Severity Level] Log message
     */
    // auto fmtTimeStamp = expr::
    //     format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");

    auto fmtThreadId = expr::
        attr<attrs::current_thread_id::value_type>("ThreadID");

    auto fmtSeverity = expr::
        attr<logging::trivial::severity_level>("Severity");

    auto fmtClient = expr::attr<std::string>("clientIp");
    auto fmtPath = expr::attr<std::string>("reqPath");
    auto fmtCode = expr::attr<std::string>("resCode");
    auto fmtLine = expr::attr< unsigned int >("LineID");

    logging::formatter logFmt =
        logging::expressions::format("[%1%][ResponseMetrics]%2%%3%%4%[%5%] %6%")
        % fmtLine % fmtClient % fmtPath % fmtCode % fmtSeverity
        % logging::expressions::smessage;


    /* console sink */
    auto consoleSink = logging::add_console_log(std::clog);
    consoleSink->set_formatter(logFmt);

    /* fs sink */
    auto fsSink = logging::add_file_log(
        keywords::file_name = "server_%Y-%m-%d.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::min_free_space = 30 * 1024 * 1024,
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), // rotate files at midnight
        keywords::open_mode = std::ios_base::app); // append to file
    fsSink->set_formatter(logFmt);
    fsSink->locked_backend()->auto_flush(true);

    return lg;
}