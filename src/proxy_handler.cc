#include "proxy_handler.h"
#include "status_handler.h"

Response ProxyHandler::handleRequest(const Request& request) {
    StatusHandler status_handler;
    Response res;
    INFO << "FILE NOT FOUND " << request.uri_;
    res.code_ = res.not_found;
    res.headers_["Content-type"] = "text/plain";
    std::string msg = "Error: File Not Found";
    msg += "\nserve_addr: " + serve_addr + "\nproxy_addr: " + proxy_addr;
    res.headers_["Content-length"] = std::to_string(msg.size());
    res.body_ = msg;
    res.headers_["Connection"] = "close";
    res.src_type = 0;
    status_handler.addRecord(request.uri_, "ProxyHandler", Response::not_found);
    return res;
}

void ProxyHandler::setLocation(std::string location_path, std::string proxy_pass) {
    serve_addr = boost::algorithm::trim_copy_if(location_path, [] (char c) {return c == '"';});
    proxy_addr = boost::algorithm::trim_copy_if(proxy_pass, [] (char c) {return c == '"';});
}

RequestHandler* ProxyHandler::Init(const std::string& location_path, const NginxConfig& config)
{
    std::string proxy_pass;
    // Need to grab proxy_pass's contents
    for (const std::shared_ptr<NginxConfigStatement> stmt : config.statements_) {
        if (stmt->tokens_[0] == "proxy_pass" && stmt->tokens_.size() >= 2) {
            proxy_pass = stmt->tokens_[1];
            break;
        }
    }

    ProxyHandler* p_handler = new ProxyHandler;
    p_handler->setLocation(location_path, proxy_pass);
    return p_handler;
}
