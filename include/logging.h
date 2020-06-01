/* Reference : https://gist.github.com/xiongjia/e23b9572d3fc3d677e3d */
#pragma once

#include <boost/log/expressions.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/attributes/mutable_constant.hpp>

namespace attrs   = boost::log::attributes;
namespace expr    = boost::log::expressions;
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

#define INFO  BOOST_LOG_SEV(my_logger::get(), boost::log::trivial::info)
#define WARN  BOOST_LOG_SEV(my_logger::get(), boost::log::trivial::warning)
#define ERROR BOOST_LOG_SEV(my_logger::get(), boost::log::trivial::error)

//Narrow-char thread-safe logger.
typedef boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger_t;

//declares a global logger with a custom initialization
BOOST_LOG_GLOBAL_LOGGER(my_logger, logger_t)
class Logger {
    public:
        static void setIp(std::string ip) {
            std::string fmtIp = "[IP: " + ip + "]"; 
            logging::attribute_cast<attrs::mutable_constant<std::string>>(logging::core::get()->get_global_attributes()["clientIp"]).set(fmtIp);
        }

        static void setPath(std::string path) {
            std::string fmtPath = "[Request_Path: " + path + "]"; 
            logging::attribute_cast<attrs::mutable_constant<std::string>>(logging::core::get()->get_global_attributes()["reqPath"]).set(fmtPath);
        }
        static void setStatus(std::string status) {
            std::string fmtStatus = "[Response_Code: " + status + "]"; 
            logging::attribute_cast<attrs::mutable_constant<std::string>>(logging::core::get()->get_global_attributes()["resCode"]).set(fmtStatus);
        }
        static void reset() {
            logging::attribute_cast<attrs::mutable_constant<std::string>>(logging::core::get()->get_global_attributes()["reqPath"]).set("");
            logging::attribute_cast<attrs::mutable_constant<std::string>>(logging::core::get()->get_global_attributes()["resCode"]).set("");
        }
        static void resetIp() {
             logging::attribute_cast<attrs::mutable_constant<std::string>>(logging::core::get()->get_global_attributes()["clientIp"]).set("");
        }
};