#include "static_file_handler.h"

int StaticFileHandler::configParser(std::string http_body, std::string &response)
{
    // regex for whole line, should include a GET request, then optional file, then http version
    // std::regex r("([A-Z]+ )\\/([a-zA-Z]+(\\.(txt|png|html|jpg|ico))*)*( HTTP\\/[1-2]\\.[0-2])(\\r\\n)");
    // if (std::regex_match(http_body.begin(), http_body.end(), r))
    // {
    // regex for each type of file that can be found
    // if not one of these, return 0
    std::regex txt_html("\\/([a-zA-Z]+\\.(txt|html))");
    std::regex jpg_png("\\/([a-zA-Z]+\\.(png|jpg))");
    std::regex favicon("\\/[a-zA-Z]+\\.ico");
    std::regex file("\\/[a-zA-Z]+");
    std::smatch m;
    if (std::regex_search(http_body, m, favicon))
    {
        response = m[0];
        return 0;
    }
    else if (std::regex_search(http_body, m, txt_html))
    {
        response = m[0];
        return 1;
    }
    else if (std::regex_search(http_body, m, jpg_png))
    {
        response = m[0];
        return 2;
    }
    else if (std::regex_search(http_body, m, file))
    {
        response = m[0];
        return 3;
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
    for (int i = 1; i < location.size(); i++)
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
    std::string image_header = "HTTP/1.1 200 OK\r\n"
                               "Content-type: image/png\r\n\r\n";
    std::string file_header = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: application/octet-stream\r\n\r\n";

    std::string bad_request = "HTTP/1.0 400 Bad Request\r\n\r\n";
    std::string not_found = "HTTP/1.0 404 Not Found\r\n\r\n";
    std::string good_request = "HTTP/1.0 200 OK\r\n\r\n";

    std::string return_str = "",
                http_response = "";

    // config_type is the int for which file is being requested
    // return_str is the name of the file
    char ch;
    int config_type = configParser(http_request, return_str);
    bool found = parseAbsoluteRoot(return_str, configLocation);

    std::string current_path = boost::filesystem::current_path().string();
    return_str = "/" + current_path + return_str;

    if (!found)
    {
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
        return text_header;
    }

    // if config is html or txt
    else if (config_type == 1)
    {
        if (return_str[0] == '/')
        {
            return_str = return_str.substr(1);
        }

        boost::filesystem::path my_path{return_str};

        if (boost::filesystem::exists(my_path)) // only run if file is opened correctly
        {
            boost::filesystem::fstream fin(my_path, std::ios::in | std::ios::binary);
            http_response = text_header;

            // int size = fin.tellg();
            // char *image = new char[size];
            // fin.seekg(0, std::ios::beg);
            // fin.read(image, size);
            std::string line;
            while (std::getline(fin, line))
            {
                http_response += line;
            }
            fin.close();
            // http_response += image;
            // delete[] image;
        }
        else
        {
            return not_found;
        }
        return http_response;
    }

    // if config is jpg or png
    else if (config_type == 2)
    {
        if (return_str[0] == '/')
        {
            return_str = return_str.substr(1);
        }
        http_response = image_header;
        std::fstream fin(return_str, std::ios::in | std::ios::binary | std::ios::ate); // opens image to be read
        if (!fin.is_open())
        {
            return not_found;
        }
        else
        {
            std::streampos size = fin.tellg();
            char *image = new char[size];
            fin.seekg(0, std::ios::beg);
            fin.read(image, size);
            fin.close();
            http_response += image;
            delete[] image;
            return http_response;
        }
    }

    // if config is a filename
    else if (config_type == 3)
    {
        // similar implementation as above, but use file_header instead of image
        if (return_str[0] == '/')
        {
            return_str = return_str.substr(1);
        }
        http_response = file_header;
        std::fstream fin(return_str, std::ios::in | std::ios::binary | std::ios::ate);
        if (!fin.is_open())
        {
            return not_found;
        }
        else
        {
            std::streampos size = fin.tellg();
            char *file = new char[size];
            fin.seekg(0, std::ios::beg);
            fin.read(file, size);
            fin.close();
            http_response += file;
            delete[] file;

            return http_response;
        }
    }
    return http_response;
}