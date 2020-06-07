#include "cache_handler.h"

RequestHandler* CacheHandler::Init(const std::string& location_path, const NginxConfig& config) {
    RequestHandler* cache_handler = new CacheHandler();
    return cache_handler;
}

Response CacheHandler::handleRequest(const Request& request) {
    // request uri_ is a full url of a webpage
    // Request rq = request;
    // rq.uri_ = getUrl(request.uri_);
    // return proxy_helper.handleRequest(rq);
    ProxyHandler proxy_helper;
    return proxy_helper.handleRequest(request);
}

// std::string CacheHandler::getUrl(std::string uri) {
//     std::string target = "/cache/";
//     std::string url = uri.replace(uri.find(target), target.length(), "");
//     std::cout << "url is " << url << std::endl;
//     return url;
// }