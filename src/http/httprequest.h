//
// Created by Jifan Zhang on 2022/4/13.
//

#ifndef TINYWEBSERVER_SRC_HTTP_HTTPREQUEST_H
#define TINYWEBSERVER_SRC_HTTP_HTTPREQUEST_H

#include <regex>
#include <string>
#include <cerrno>
#include <vector>
#include <unordered_map>
#include <mysql/mysql.h>

#include "../log/log.h"
#include "../buffer/buffer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnraii.h"

// Parse HTTP request message
class HttpRequest {
public:
    // A collection of parsing status.
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    // A collection of HTTP request code.
    enum HTTP_CODE {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESPONSE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    // ctor
    HttpRequest();

    // dtor
    ~HttpRequest();

    // HTTP requester initialization.
    void Init();

    // Parse the HTTP request from the client.
    bool Parse(Buffer& buffer);

    // Get the path of requested resource.
    std::string Path() const;
    // Get the path of requested resource [overloaded].
    std::string& Path();

    // Get the request method of the HTTP request.
    std::string Method() const;

    // Get the HTTP version of the HTTP request.
    std::string Version() const;

    // Get the content of the HTTP request message.
    std::string GetPost(const std::string& key) const;
    // Get the content of the HTTP request message [overloaded].
    std::string GetPost(const char* key) const;

    // Check if is HTTP keep-alive.
    bool IsKeepAlive() const;

    // TODO:
    // void ParseFromData();
    // void ParseJson();

private:
    // Parse the HTTP Request-Line. Get the request method, request
    // url and the HTTP version (for example, "GET / HTTP/1.1").
    bool ParseRequestLine(const std::string& line);

    // Parse the HTTP header.
    void ParseHeader(const std::string& line);

    // Parse the HTTP body.
    void ParseBody(const std::string& line);

private:
    // Parse the HTTP request URL
    void ParsePath();

    // Parse the content of the HTTP body.
    void ParsePost();

    // Parse the HTTP request path from the encoded URL
    void ParseFormUrlencoded();

    // Find the database for user authentication, used to achieve the login function.
    static bool UserVerify(const std::string& username,
                           const std::string& passwd,
                           bool is_login);

    // Convert a number from hexadecimal to decimal.
    static int ConvertHex(char c);

private:
    // The current parsing status.
    PARSE_STATE parse_state_ = REQUEST_LINE;

    // Request method, such as GET, HEAD, POST.
    std::string method_;

    // Request url, the resource upon which to apply the request.
    std::string path_;

    // The HTTP version
    std::string version_;

    // The HTTP body, the actual content of the message.
    std::string body_;

    // The HTTP header, providing the recipient with information
    // about the message, the sender, and the way in which the
    // sender wants to communicate with the recipient.
    std::unordered_map<std::string, std::string> header_;

    // A dictionary of request message.
    std::unordered_map<std::string, std::string> post_;

    // A collection of default web pages.
    static const std::vector<std::string> DEFAULT_HTML_;

    // A mapping from HTML file to tag.
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG_;
};


#endif //TINYWEBSERVER_SRC_HTTP_HTTPREQUEST_H
