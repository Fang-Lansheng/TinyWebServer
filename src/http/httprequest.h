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

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };
    
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

    HttpRequest();

    ~HttpRequest();

    void Init();

    bool Parse(Buffer& buffer);

    std::string Path() const;
    std::string& Path();

    std::string Method() const;

    std::string Version() const;

    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;

    //// TODO:
    // void ParseFromData();
    // void ParseJson();

private:
    bool ParseRequestLine(const std::string& line);

    void ParseHeader(const std::string& line);

    void ParseBody(const std::string& line);

    void ParsePath();

    void ParsePost();

    void ParseFormUrlencoded();

    static bool UserVerify(const std::string& username,
                           const std::string& passwd,
                           bool is_login);

    static int ConvertHex(char c);

    PARSE_STATE parse_state_ = REQUEST_LINE;
    std::string method_;
    std::string path_;
    std::string version_;
    std::string body_;
    std::unordered_map<std::string, std::string> header_;

    std::unordered_map<std::string, std::string> post_;
    static const std::vector<std::string> DEFAULT_HTML_;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG_;
};


#endif //TINYWEBSERVER_SRC_HTTP_HTTPREQUEST_H
