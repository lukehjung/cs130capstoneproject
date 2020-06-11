#include "proxy_handler.h"
#include "status_handler.h"
#include <curl/curl.h>
#include <unordered_map>
#include <cstring>
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

    // act accordingly based on the speicifed cache-control directives
    std::map<std::string, int> cache_hdrs = parse_cache_hdrs(request.headers_);
    bool should_cache = true; // decide if the response should be cached
    bool can_use = false;     // decide if the cache should be used
    bool cache_only = false;  // return only the cached version
    bool must_validate = cache_hdrs["no-cache"] ? true : false; // re-fetch the response
    cache_control(cache_hdrs, req_uri, must_validate, can_use, cache_only, should_cache);

    // the cache is valid and the client wants it
    if(can_use)
    {
      return use_cache(req_uri);
    }

    // the client only wants the cache, but we dont have it
    else if(cache_only)
    {
      return no_cache();
    }


    if (request.uri_.rfind("http", 0) == 0)
    {
        dest = request.uri_;
    }
    else
    {
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

    // cache the response, expire in 1 minutes by default
    if(should_cache && res.code_ == Response::ok)
    {
        store_cache(request.uri_, res);
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

/* convert a cache-control directives string to a map. */
// this funciton needs testing, just need to check if "dir1, dir2, .." is converted to "{dir1, dir2,..}" correctly
std::map<std::string, int> ProxyHandler::parse_cache_hdrs(std::map<std::string, std::string> req_hdrs)
{
  std::string cache_dirs = req_hdrs["Cache-Control"]; // e.g. "dir1, dir2, ..."
  std::map<std::string, int> cache_hdrs;  // e.g. "(dir, 1), ..."
  std::string delimiter = ",";
  std::string token;
  int pos = 0;

  // convert the cache-control directives string into a map
  while ((pos = cache_dirs.find(delimiter)) != std::string::npos)
  {
    int val = 1;
    token = cache_dirs.substr(0, pos);
    boost::trim(token);

    // retrieve the specified value for these dirs
    // these dirs are in this format: e.g. (max-age=100)
    // we want to map 'max-age' to its specified value, 100.
    if(token.find("max-age") != std::string::npos
      || token.find("max-stale") != std::string::npos
      || token.find("min-fresh") != std::string::npos)
    {
      int token_pos = token.find("=");
      bool isNum = true;
      // check if the given value is a number
      std::string given_val = token.substr(token_pos + 1);
      for(int i = 0; i < given_val.length(); i++)
      {
        isNum = isdigit(given_val[i]) ? true : false;
      }

      val = isNum ? std::stoi(given_val) : -1;
      token = token.substr(0, token_pos);
    }

    cache_hdrs[token] = val;
    cache_dirs.erase(0, pos + delimiter.length());
  }

  // retrieve the specified value for the last directive, if any
  boost::trim(cache_dirs);
  if(cache_dirs.find("max-age") != std::string::npos
    || cache_dirs.find("max-stale") != std::string::npos
    || cache_dirs.find("min-fresh") != std::string::npos)
  {
    int token_pos = cache_dirs.find("=");
    bool isNum = true;
    std::string given_val = cache_dirs.substr(token_pos + 1);
    for(int i = 0; i < given_val.length(); i++)
    {
      isNum = isdigit(given_val[i]) ? true : false;
    }

    cache_dirs = cache_dirs.substr(0, token_pos);
    cache_hdrs[cache_dirs] = isNum ? std::stoi(given_val) : -1;
  }

  else
  {
    cache_hdrs[cache_dirs] = 1;
  }

  return cache_hdrs;
}

/*
 This function needs testing, especially if the bool reference are set correctly
 determine how to deal with the cache based on the cache-control directives in the request.
 @cache_hdrs: the cache-control directives in map format
 @req_uri: the request uri
 @must_validate: if this is true, must validate/re-fetch the response even if we have the cache
 @can_use: the cache is good to use
 @cache_only: the client only accepts cached response
 @should_cache: the response should be cached
*/
bool ProxyHandler::cache_control(std::map<std::string, int> cache_hdrs, const std::string req_uri,
  const bool must_validate, bool& can_use, bool& cache_only, bool& should_cache)
{
  // provides a return type for testing
  bool res = true;
  // the cached version is present
  if(!must_validate && cached_pages.count(req_uri) > 0)
  {
    can_use = true;
    if(cache_hdrs["max-age"] && cache_hdrs["max-stale"])
    {
      int max_age = cache_hdrs["max-age"];
      int max_stale = cache_hdrs["max-stale"];
      bool stale_ok = max_age > max_stale ? false : true;

      if(!(stale_ok && cached_pages.at(req_uri).couldStale(max_stale)))
      {
        INFO << "The cache exceeds the given max-stale.";
        can_use = false;
      }

      else if (cached_pages.at(req_uri).isExpired(max_age))
      {
        INFO << "The cache exceeds the given max-age.";
        can_use = false;
      }
    }

    else if(cache_hdrs["max-age"])
    {
      int max_age = cache_hdrs["max-age"];
      can_use = !cached_pages.at(req_uri).isExpired(max_age);

      if(!can_use)
      {
        INFO << "The cache exceeds the given max-age.";
        res = false; // if can't use cache, return false
      }
    }

    else if(cache_hdrs["max-stale"])
    {
      int max_stale = cache_hdrs["max-stale"];
      can_use = cached_pages.at(req_uri).couldStale(max_stale);

      if(!can_use)
      {
        INFO << "The cache exceeds the given max-stale.";
        res = false;
      }
    }

    if(cache_hdrs["min-fresh"])
    {
      int min_fresh = cache_hdrs["min-fresh"];
      can_use = cached_pages.at(req_uri).isFresh(min_fresh);

      if(!can_use)
      {
        INFO << "The cache is not at least fresh for the given time.";
        res = false; // if can't use cache, return false
      }
    }

    if(cache_hdrs["no-store"])
    {
      can_use = false;
      cached_pages.erase(req_uri);
      res = true;
    }
  }

  else
  {
    // the client only wants the cached version
    if(!must_validate && cache_hdrs["only-if-cached"])
    {
      cache_only = true;
    }

    if(cache_hdrs["no-store"])
    {
      should_cache = false;
    }
    res = true; // of cache works throughout, return true
  }
  return res;
}

// This function needs testing, e.g. expected to be called when a cache is present for the request
Response ProxyHandler::use_cache(std::string req_uri)
{
  StatusHandler status_handler;
  INFO << "Cache Hit With URI: " << req_uri;
  status_handler.addRecord(req_uri, "ProxyHandler", Response::not_modified);
  return cached_pages.at(req_uri).get_cache();
}

// This function needs testing, e.g. expected to be called when there is no cache,
// when we have the only-if-cached directive
Response ProxyHandler::no_cache()
{
  INFO << "No cache present for the given request.";
  Response response;
  response.code_ = Response::gateway_timeout;
  response.src_type = 0;
  response.headers_["Content-type"] = "text/plain";
  response.headers_["Connection"] = "close";
  return response;
}

// This function needs testing, e.g. expected to be called when we cache a response
void ProxyHandler::store_cache(std::string req_uri, Response res)
{
  INFO << "Cache Miss With URI: " << req_uri;
  cached_page page(res);
  cached_pages.insert({req_uri, page});
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

std::map<std::string, cached_page> ProxyHandler::get_cached_pages()
{
    return cached_pages;
}
