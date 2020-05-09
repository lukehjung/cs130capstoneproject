#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <unordered_map> 

class Response {
	// HTTP response codes
    enum StatusCode {
      ok = 200,
      created = 201,
      accepted = 202,
      no_content = 204,
      moved_permanently = 301,
      moved_temporarily = 302,
      not_modified = 304,
      bad_request = 400,
      unauthorized = 401,
      forbidden = 403,
      not_found = 404,
      internal_server_error = 500,
      not_implemented = 501,
      bad_gateway = 502,
      service_unavailable = 503
    };

	// An HTML code indicating success/failure of processing
  	StatusCode code_;

  	// A map of headers, for convenient lookup ("Content-Type", "Cookie", etc)
  	std::unordered_map<std::string, std::string> headers_;

  	// The content of the response
  	std::string body_;

	// Media type of response content
	// MimeType content_type_;
};

#endif // RESPONSE_H
