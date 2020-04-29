#include "static_file_handler.h"
#include "logging.h"

int StaticFileHandler::configParser(std::string http_body)
{
    // regex for whole line, should include a GET request, then optional file, then http version
    // std::regex r("([A-Z]+ )\\/([a-zA-Z]+(\\.(txt|png|html|jpg|ico))*)*( HTTP\\/[1-2]\\.[0-2])(\\r\\n)");
    // if (std::regex_match(http_body.begin(), http_body.end(), r))
    // {
    // regex for each type of file that can be found
    // if not one of these, return 0
    std::regex txt_html("\\/([a-zA-Z]+\\.(txt|html))");
    std::regex jpg("\\/[a-zA-Z]+\\.jpg");
    std::regex png("\\/[a-zA-Z]+\\.png");
    std::regex favicon("\\/[a-zA-Z]+\\.ico");
    std::regex file("\\/[a-zA-Z]+");
    std::smatch m;
    if (std::regex_search(http_body, m, favicon))
    {
        return 0;
    }
    else if (std::regex_search(http_body, m, txt_html))
    {
        return 1;
    }
    else if (std::regex_search(http_body, m, png))
    {
        return 2;
    }
    else if (std::regex_search(http_body, m, jpg))
    {
        return 3;
    }
    else if (std::regex_search(http_body, m, file))
    {
        return 4;
    }
    else
    {
        return 0;
    }
    // }
    // // if http request is malformed, return -1
    // else
    // {
    //     return -1;
    // }
}

bool StaticFileHandler::parseAbsoluteRoot(std::string &location, std::vector<std::string> configLocation)
{
    std::string first_part = "/";
    std::string second_part = "";
    bool flag = false;
    for (int i = 1; i < location.length(); i++)
    {
        char c = location[i];
        if (c == '/')
        {
            first_part += c;
            second_part = location.substr(i);
            flag = true;
            break;
        }

        first_part += c;
    }
    if (!flag)
    {
        first_part = "/";
        second_part = location;
    }

    // iterate by 2 each time so add one after increasing it once

    for (int i = 0; i < configLocation.size(); i += 2)
    {
        // if found in configLocation, return true and set location variable to new prefix and append path
        if (first_part == configLocation[i])
        {
            location = configLocation[i + 1] + second_part;
            return true;
        }
    }

    // if couldn't find value in configLocation, return false
    return false;
}

std::string StaticFileHandler::getResponse(std::string http_request, std::vector<std::string> configLocation)
{
    // HTTP Headers used for each type of file
    std::string text_header = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n\r\n";
    // std::string image_header = "HTTP/1.1 200 OK\r\n"
    //                            "Content-type: image/png\r\n\r\n";
    std::string file_header = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: application/octet-stream\r\n\r\n";

    std::string bad_request = "HTTP/1.0 400 Bad Request\r\n\r\n";
    std::string not_found = "HTTP/1.0 404 Not Found\r\n\r\n";
    std::string good_request = "HTTP/1.0 200 OK\r\n\r\n";

    std::string return_str = http_request,
                http_response = "";

    // config_type is the int for which file is being requested
    // return_str is the name of the file
    char ch;
    int config_type = configParser(http_request);
    bool found = parseAbsoluteRoot(return_str, configLocation);
    std::string body = "";

    std::string current_path = boost::filesystem::current_path().string();
    return_str = current_path + return_str;

    while (return_str[0] == '/' && return_str[1] == '/')
    {
        return_str = return_str.substr(1);
    }

    INFO << "CALLING FILE:" << return_str;

    if (!found)
    {
        INFO << "FILE NOT FOUND" << return_str;
        return "Error: not found";
    }

    // if config is malformed, return bad request
    // if (config_type == -1)
    // {
    //     return "Error: malformed request";
    // }

    // if config is a GET request but no file
    else if (config_type == 0)
    {
        http_response = text_header;
        http_response += "Content-Length: 36\r\n\r\n";
        http_response += "Hello World! This is the index page.";
        return http_response;
    }

    // if config is html or txt
    else if (config_type == 1)
    {

        boost::filesystem::path my_path{return_str};

        if (boost::filesystem::exists(my_path)) // only run if file is opened correctly
        {
            boost::filesystem::fstream fin(my_path, std::ios::in | std::ios::binary);
            http_response = text_header;

            body = "";
            std::string line;
            while (std::getline(fin, line))
            {
                body += line;
           }
           http_response += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
           http_response += body;

            fin.close();
            // http_response += image;
            // delete[] image;
        }
        else
        {
            INFO << "ERROR: " << return_str << " not found.";
            return not_found;
        }
        return http_response;
    }

    // // if config is jpg or png
    // else if (config_type == 2)
    // {
    //     if (return_str[0] == '/')
    //     {
    //         return_str = return_str.substr(1);
    //     }
    //     http_response = image_header;
    //     std::fstream fin(return_str, std::ios::in | std::ios::binary | std::ios::ate); // opens image to be read
    //     if (!fin.is_open())
    //     {
    //         return not_found;
    //     }
    //     else
    //     {
    //         std::streampos size = fin.tellg();
    //         char *image = new char[size];
    //         fin.seekg(0, std::ios::beg);
    //         fin.read(image, size);
    //         fin.close();
    //         http_response += image;
    //         delete[] image;
    //         return http_response;
    //     }
    // }
    //
    // // if config is a filename
    // else if (config_type == 3)
    // {
    //     // similar implementation as above, but use file_header instead of image
    //     if (return_str[0] == '/')
    //     {
    //         return_str = return_str.substr(1);
    //     }
    //     http_response = file_header;
    //     std::fstream fin(return_str, std::ios::in | std::ios::binary | std::ios::ate);
    //     if (!fin.is_open())
    //     {
    //         return not_found;
    //     }
    //     else
    //     {
    //         std::streampos size = fin.tellg();
    //         char *file = new char[size];
    //         fin.seekg(0, std::ios::beg);
    //         fin.read(file, size);
    //         fin.close();
    //         http_response += file;
    //         delete[] file;
    //
    //         return http_response;
    //     }
    // }
    // return http_response;
}
