//
// Created by Jifan Zhang on 2022/4/13.
//

#include "httprequest.h"

// FIX IT: Initialization with static storage duration
const std::vector<std::string> HttpRequest::DEFAULT_HTML_{
        "/index", "/register", "/login", "/welcome",
        "/video", "/picture", };

// FIX IT: Initialization with static storage duration
const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG_{
        {"register.html", 0}, {"/login.html", 1}, };

HttpRequest::HttpRequest() {
    Init();
}

void HttpRequest::Init() {
    method_ = path_ = version_ = body_ = "";
    parse_state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HttpRequest::Parse(Buffer &buffer) {
    const char CRLF[] = "\r\n";
    if (buffer.ReadableBytes() <= 0) {
        return false;
    }
    while (buffer.ReadableBytes() && parse_state_ != FINISH) {
        const char* LINE_END = std::search(
                buffer.Peek(), buffer.BeginWriteConst(), CRLF, CRLF + 2);
        std::string line(buffer.Peek(), LINE_END);

        switch (parse_state_) {
            case REQUEST_LINE:
                if (!ParseRequestLine(line)) {
                    return false;
                }
                ParsePath();
                break;
            case HEADERS:
                ParseHeader(line);
                if (buffer.ReadableBytes() <= 2) {
                    parse_state_ = FINISH;
                }
                break;
            case BODY:
                ParseBody(line);
                break;
            default:
                break;
        }

        if (LINE_END == buffer.BeginWrite()) {
            break;
        }

        buffer.RetrieveUntil(LINE_END + 2);
    }

    LOG_DEBUG("[%s], [%s], [%s]",
              method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

std::string HttpRequest::Path() const {
    return path_;
}

std::string& HttpRequest::Path() {
    return path_;
}

std::string HttpRequest::Method() const {
    return method_;
}

std::string HttpRequest::Version() const {
    return version_;
}

std::string HttpRequest::GetPost(const std::string& key) const {
    assert(!key.empty());
    if (post_.count(key)) {
        return post_.find(key)->second;
    } else {
        return "";
    }
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key);
    if (post_.count(key)) {
        return post_.find(key)->second;
    } else {
        return "";
    }
}

bool HttpRequest::IsKeepAlive() const {
    if (header_.count("Connection")) {
        return header_.find("Connection")->second == "keep-alive" &&
            version_ == "1.1";
    } else {
        return false;
    }
}

bool HttpRequest::ParseRequestLine(const std::string &line) {
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch sub_match;

    if (std::regex_match(line, sub_match, pattern)) {
        method_ = sub_match[1];
        path_ = sub_match[2];
        version_ = sub_match[3];
        parse_state_ = HEADERS;
        return true;
    } else {
        LOG_ERROR("HTTP request line error");
        return false;
    }
}

void HttpRequest::ParseHeader(const std::string &line) {
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch sub_match;

    if (std::regex_match(line, sub_match, pattern)) {
        header_[sub_match[1]] = sub_match[2];
    } else {
        parse_state_ = BODY;
    }
}

void HttpRequest::ParseBody(const std::string &line) {
    body_ = line;
    ParsePost();
    parse_state_ = FINISH;
    LOG_DEBUG("Body: %s, length: %d", line.c_str(), line.size());
}

void HttpRequest::ParsePath() {
    if (path_ == "/") {
        path_ = "/index.html";
    } else {
        for (const auto& item: DEFAULT_HTML_) {
            if (item == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

void HttpRequest::ParsePost() {
    if (method_ == "POST" &&
        header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFormUrlencoded();
        if (DEFAULT_HTML_TAG_.count(path_)) {
            int tag = DEFAULT_HTML_TAG_.find(path_)->second;
            LOG_DEBUG("Tag: %d", tag);
            if (tag == 0 || tag == 1) {
                bool is_login = (tag == 1);
                if (UserVerify(post_["username"],
                               post_["password"], is_login)) {
                    path_ = "/welcome.html";
                } else {
                    path_ = "/error.html";
                }
            }
        }
    }
}

void HttpRequest::ParseFormUrlencoded() {
    if (body_.empty()) {
        return;
    }

    std::string key, value;
    int num = 0;
    size_t n = body_.size();
    size_t i = 0, j = 0;

    for (; i < n; ++i) {
        char c = body_[i];
        switch (c) {
            case '=':
                key = body_.substr(j, i - j);
                j = i + 1;
                break;
            case '+':
                body_[i] = ' ';
                break;
            case '%':
                num = ConvertHex(body_[i + 1]) * 16 + ConvertHex(body_[i + 2]);
                // FIX IT: Narrowing conversion from 'int' to 'char'
                body_[i + 2] = num % 10 + '0';
                // FIX IT: Narrowing conversion from 'int' to 'char'
                body_[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                value = body_.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
    }

    assert(j <= i);
    if (!post_.count(key) && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

bool HttpRequest::UserVerify(const std::string &username,
                             const std::string &passwd, bool is_login) {
    if (username.empty() || passwd.empty()) {
        // Invalid username or password
        return false;
    }

    LOG_INFO("Verify user: %s", username.c_str());
    MYSQL* sql;
    SqlConnRaii(&sql, SqlConnPool::Instance());
    assert(sql);

    bool flag = false;
    char order[256] = {};
    MYSQL_RES* res = nullptr;

    if (!is_login) {
        flag = true;
    }

    snprintf(order, 256, "SELECT username, password "
                         "FROM user WHERE USERNAME='%s' LIMIT 1",
                         username.c_str());
    LOG_DEBUG("%s", order);

    if (mysql_query(sql, order)) {
        mysql_free_result(res);
        return false;
    }

    res = mysql_store_result(sql);

//    auto j = mysql_num_fields(res);
//    auto field = mysql_fetch_field(res);

    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        std::string stored_passwd = row[1];
        if (is_login) {
            if (passwd == stored_passwd) {
                flag = true;
            } else {
                flag = false;
                LOG_DEBUG("The password does not match the username.");
            }
        } else {
            flag = false;
            LOG_DEBUG("The username was already used.");
        }
    }
    mysql_free_result(res);

    if (!is_login && flag) {
        LOG_DEBUG("New user registration.");
        bzero(order, 256);
        snprintf(order, 256, "INSERT INTO user(username, password) "
                             "VALUES('%s', '%s')",
                             username.c_str(), passwd.c_str());
        LOG_DEBUG("%s", order);
        if (mysql_query(sql, order)) {
            LOG_DEBUG("MYSQL insertion error!");
            flag = false;
        } else {
            flag = true;
        }
    }
    SqlConnPool::Instance()->FreeConn(sql);
    LOG_DEBUG("Successfully verified user: %s", username.c_str());
    return flag;
}

int HttpRequest::ConvertHex(char c) {
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else {
        return c;
    }
}

HttpRequest::~HttpRequest() = default;




