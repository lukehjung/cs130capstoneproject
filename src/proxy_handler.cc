#include "proxy_handler.h"
#include "status_handler.h"
#include <curl/curl.h>
#include <unordered_map>
#include "logging.h"

std::map<std::string, cached_page> ProxyHandler::cached_pages;

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

    // if we have the cached resopnse, return the cached version instead
    if(cached_pages.count(request.uri_) > 0)
    {
        INFO << "Cache Hit With URI: " << request.uri_;
        status_handler.addRecord(request.uri_, "ProxyHandler", Response::not_modified);
        return cached_pages.at(request.uri_).get_cache();
    }
    
    if (request.uri_.rfind("http", 0) == 0) {
        dest = request.uri_;
    } else {
        //parse request.uri_ and resolve
        uri_suffix = req_uri.replace(req_uri.find(serve_addr), serve_addr.length(), "");
        if (uri_suffix[0] != '/' && proxy_addr[proxy_addr.length()-1] != '/')
            uri_suffix = "/" + uri_suffix;
        
        //required uri
        dest = proxy_addr + uri_suffix;
        INFO << "DESTINATION: " << dest;
        INFO << "PROXY ADDR: " <<  proxy_addr;
    }
    
    struct curl_slist *list = NULL;
    
    res.code_ = Response::ok;
    if (curl) {
        curle_code = curl_easy_setopt(curl, CURLOPT_URL, dest.c_str());
        if (curle_code != CURLE_OK)
        {
            ERROR << "curl could not set proxy address";
            res.code_ = Response::internal_server_error;
        }
        curle_code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        if (curle_code != CURLE_OK)
        {
            ERROR << "curl could not set redirect follow-on";
            res.code_ = Response::internal_server_error;
        }
        curle_code = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20L); //set max redirects to 20
        if (curle_code != CURLE_OK)
        {
            ERROR << "curl could not set max redirects";
            res.code_ = Response::internal_server_error;
        }
        curle_code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
        if (curle_code != CURLE_OK)
        {
            ERROR << "curl could not set timeout for requests";
            res.code_ = Response::internal_server_error;
        }
        
        // If condition not true, there was some error in setting up curl
        if (res.code_ == Response::ok)
        {
            list = curl_slist_append(list, "Connection: close");
            
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
            if (curle_code == CURLE_OK)
            {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                
                std::string edited_msg;
                if(header_data.find("Content-Type") != header_data.end() && header_data["Content-Type"].find("text/html") != std::string::npos)
                {
                    edited_msg = parse_html_body(msg);
                }
                else
                {
                    edited_msg = msg;
                }
                
                res.body_ = edited_msg;
                res.code_ = static_cast<Response::StatusCode>(response_code);
                
                for (auto it : header_data)
                {
                    res.headers_[it.first] = it.second;
                }
                res.headers_["Content-Length"] = std::to_string(edited_msg.length());
                res.headers_["Connection"] = "close";
                res.headers_.erase("Transfer-Encoding");
                
            }
            else
            {
                ERROR << "FAILURE: Curl request failed";
                res.code_ = Response::internal_server_error;
            }
            
            curl_slist_free_all(list);
        }
        
        curl_easy_cleanup(curl);
    }
    else
    {
        ERROR << "CURL ERROR";
        res.code_ = Response::internal_server_error;
    }
    
    res.src_type = 0;
    status_handler.addRecord(request.uri_, "ProxyHandler", res.code_);

    // cache the response, expire in 10 minutes by default
    if(res.code_ == Response::ok)
    {
        INFO << "Cache Miss With URI: " << request.uri_;
        cached_page page(res);
        cached_pages.insert({request.uri_, page});
    }

    return res;
}

/* function to parse HTML content, prepend it with the correct server address in
 order to achieve correct reverse proxying */
std::string ProxyHandler::parse_html_body(std::string& msg)
{
    //identify and set correct address to prepend
    std::string target_addr;
    if(serve_addr[serve_addr.length()-1] == '/')
        target_addr = serve_addr.substr(0, serve_addr.length()-1);
    else
        target_addr = serve_addr;
    
    INFO << "TARGET ADDR: " << target_addr;
        
    //string to hold the edited message
    std::string edited_msg = "";
    //string that allows us to identify when to insert, i.e any time we see an address beginning with '/'
    std::string curr_target = "";
    for(auto it = msg.begin(); it != msg.end(); it++)
    {
        //if neither " nor /, just add it to edited message and flush the curr_target
        if((*it) != '"' && (*it) != '/')
        {
            edited_msg.push_back((*it));
            curr_target = "";
        }
        else if((*it) == '"')   //if the next char is / then we will insert
        {
            edited_msg.push_back((*it));
            curr_target.push_back((*it));
        }
        else if((*it) == '/')
        {
            curr_target.push_back((*it));
            if(curr_target == "\"/")    //compare the curr_target with "/
            {
                edited_msg.append(target_addr + "/");   //if they are the same, then prepend
            }
            else
            {
                edited_msg.push_back((*it));
            }
            curr_target = "";
        }
    }
    return edited_msg;
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