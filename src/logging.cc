#include "logging.h"

//Defines a global logger initialization routine
BOOST_LOG_GLOBAL_LOGGER_INIT(my_logger, logger_t)
{
    logger_t lg;

    logging::add_common_attributes();

    // logging::core::get()->add_global_attribute("Scope", attrs::named_scope());

    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::trace
    );

    /* log formatter:
     * [TimeStamp][ClientIp] [ThreadId] [Severity Level] Log message
     */
    auto fmtTimeStamp = expr::
        format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");

    auto fmtThreadId = expr::
        attr<attrs::current_thread_id::value_type>("ThreadID");

    auto fmtSeverity = expr::
        attr<logging::trivial::severity_level>("Severity");

    // auto fmtScope = expr::format_named_scope("Scope",
    //     boost::log::keywords::format = "%n(%f:%l)",
    //     boost::log::keywords::iteration = boost::log::expressions::reverse,
    //     boost::log::keywords::depth = 2);

    auto fmtClient = expr::attr<std::string>("ClientIp");

    logging::formatter logFmt =
        logging::expressions::format("[%1%] %2% (%3%) [%4%] %5%")
        % fmtTimeStamp % fmtClient % fmtThreadId % fmtSeverity
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
