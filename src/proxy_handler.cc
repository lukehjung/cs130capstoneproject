#include "proxy_handler.h"
#include "status_handler.h"
#include <curl/curl.h>
#include <unordered_map>
#include "logging.h"

Response ProxyHandler::handleRequest(const Request& request) {
    StatusHandler status_handler;
    Response res;
    CURL* curl;
    CURLcode curle_code;
    curl = curl_easy_init();
    std::string req_uri = request.uri_;
    std::string dest;
    std::string uri_suffix;
    INFO << "REQ URI: " << req_uri;
    
    //parse request.uri_ and resolve 
    uri_suffix = req_uri.replace(req_uri.find(serve_addr), serve_addr.length(), "");
    if(uri_suffix[0] != '/' && proxy_addr[proxy_addr.length()-1] != '/')
        uri_suffix = "/" + uri_suffix;

    //required uri  
    dest = proxy_addr + uri_suffix;
    INFO << "DESTINATION: " << dest; 
    INFO << "PROXY ADDR: " <<  proxy_addr;
    
    if (curl) {
        curle_code = curl_easy_setopt(curl, CURLOPT_URL, dest.c_str());
        if (curle_code != CURLE_OK)
            ERROR << "curl could not set proxy address";
            return_500(); 
        curle_code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        if (curle_code != CURLE_OK)
            ERROR << "curl could not set redirect follow-on";
        curle_code = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20L); //set max redirects to 20
        if (curle_code != CURLE_OK)
            ERROR << "curl could not set max redirects";
        curle_code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
        if (curle_code != CURLE_OK)
            ERROR << "curl could not set timeout for requests";
        
        long response_code;
        std::unordered_map<std::string, std::string> header_data;
        std::string msg;
        
        // receive the HTTP response body
        curle_code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        if (curle_code != CURLE_OK)
            ERROR << "curl could not set response body callback"; 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &msg);
        
        // receive the HTTP headers
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_data);
        
        //send request and obtain status code
        curle_code = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        if (curle_code != CURLE_OK)
            ERROR << "FAILURE: Curl request was not made";
        
        res.body_ = msg;
        res.code_ = static_cast<Response::StatusCode>(response_code);
        
        for (auto it : header_data)
        {
            res.headers_[it.first] = it.second;
        }
        
        curl_easy_cleanup(curl);
    }
    else
    {
        ERROR << "CURL ERROR";
        return_500();
    }
    
    res.src_type = 0;
    status_handler.addRecord(request.uri_, "ProxyHandler", res.code_);
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

std::size_t write_callback(const char* ptr, std::size_t size, std::size_t byte_count, std::string* msg)
{
    const std::size_t totalBytes = size*byte_count;
    msg->append(ptr, totalBytes);
    return totalBytes;
}

std::size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    size_t total_length = size*nitems;
    size_t index=0;
    while(index < total_length)
    {
        char* curr_header = (char*)buffer + index;
        if((curr_header[0] == '\r') || (curr_header[0] == '\n'))
            break;
        index++;
    }
    
    std::string hdr((char*)buffer, (char*)buffer + index);
    std::unordered_map<std::string, std::string>* header_map = (std::unordered_map<std::string, std::string>*)userdata;
    
    size_t colon_pos = hdr.find(":");
    if(colon_pos != std::string::npos)
        header_map->insert(std::make_pair<std::string, std::string>(hdr.substr(0, colon_pos), hdr.substr(colon_pos+2)));
    
    return nitems;
}

Response return_500() {
  Response res; 
  res.code_ = Response::internal_server_error;
  return res; 
}