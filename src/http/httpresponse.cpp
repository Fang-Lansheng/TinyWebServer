//
// Created by Jifan Zhang on 2022/4/13.
//

#include "httpresponse.h"

// FIX IT: Initialization with static storage duration
const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
        { ".html",  "text/html" },
        { ".xml",   "text/xml" },
        { ".xhtml", "application/xhtml+xml" },
        { ".txt",   "text/plain" },
        { ".rtf",   "application/rtf" },
        { ".pdf",   "application/pdf" },
        { ".word",  "application/nsword" },
        { ".png",   "image/png" },
        { ".gif",   "image/gif" },
        { ".jpg",   "image/jpeg" },
        { ".jpeg",  "image/jpeg" },
        { ".au",    "audio/basic" },
        { ".mpeg",  "video/mpeg" },
        { ".mpg",   "video/mpeg" },
        { ".avi",   "video/x-msvideo" },
        { ".gz",    "application/x-gzip" },
        { ".tar",   "application/x-tar" },
        { ".css",   "text/css "},
        { ".js",    "text/javascript "}, };

// FIX IT: Initialization with static storage duration
const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
        { 200, "OK" },
        { 400, "Bad Request" },
        { 403, "Forbidden" },
        { 404, "Not Found" }, };

// FIX IT: Initialization with static storage duration
const std::unordered_map<int, std::string> HttpResponse::CODE_PATH = {
        { 400, "/400.html" },
        { 403, "/403.html" },
        { 404, "/404.html" }, };

HttpResponse::HttpResponse() {
    code_ = -1;
    path_ = "";
    src_dir_ = "";
    is_keep_alive_ = false;
    mm_file_ = nullptr;
    mm_file_stat_ = { 0 };
}

HttpResponse::~HttpResponse() {
    UnmapFile();
}

void HttpResponse::Init(const std::string &src_dir, std::string &path,
                        bool is_keep_alive, int code) {
    assert(!src_dir.empty());
    if (mm_file_) {
        UnmapFile();
    }

    code_ = code;
    path_ = path;
    is_keep_alive_ = is_keep_alive;
    src_dir_ = src_dir;
}

void HttpResponse::MakeResponse(Buffer &buffer) {
    // Determine the requested file
    if (stat((src_dir_ + path_).data(), &mm_file_stat_) < 0 ||
            S_ISDIR(mm_file_stat_.st_mode)) {
        LOG_WARN("File %s not found!", (src_dir_ + path_).c_str());
        code_ = 404;
    } else if (!(mm_file_stat_.st_mode & S_IROTH)) {
        LOG_WARN("Cannot access to %s now!", (src_dir_ + path_).c_str());
        code_ = 403;
    } else if (code_ == -1) {
        code_ = 200;
    }

    ErrorHtml();
    AddStateLine(buffer);
    AddHeader(buffer);
    AddContent(buffer);
}

void HttpResponse::UnmapFile() {
    if (mm_file_) {
        munmap(mm_file_, mm_file_stat_.st_size);
        mm_file_ = nullptr;
    }
}

char* HttpResponse::File() {
    return mm_file_;
}

size_t HttpResponse::FileLength() const {
    return mm_file_stat_.st_size;
}

void HttpResponse::ErrorContent(Buffer &buffer, std::string message) {
    std::string body;
    std::string status;

    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(code_)) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }

    body += std::to_string(code_) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buffer.Append("Content-length: " + std::to_string(body.size()) +
        "\r\n\r\n");
    buffer.Append(body);
}

int HttpResponse::Code() const {
    return code_;
}

void HttpResponse::AddStateLine(Buffer &buffer) {
    std::string status;
    if (CODE_STATUS.count(code_)) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        code_ = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buffer.Append("HTTP/1.1 " + std::to_string(code_) +
        " " + status + "\r\n");
}

void HttpResponse::AddHeader(Buffer &buffer) {
    const std::string CRLF = "\r\n";
    buffer.Append("Connection: ");
    if (is_keep_alive_) {
        buffer.Append("keep-alive" + CRLF);
        buffer.Append("keep-alive: max=6, timeout=120" + CRLF);
    } else {
        buffer.Append("close" + CRLF);
    }
    buffer.Append("Content-type: " + GetFileType() + CRLF);
}

void HttpResponse::AddContent(Buffer &buffer) {
    int src_fd = open((src_dir_ + path_).data(), O_RDONLY);
    if (src_fd < 0) {
        ErrorContent(buffer, "Requested file not found!");
        return;
    }

    // FIX IT!
    // Map the file to memory
    LOG_DEBUG("file path %s", (src_dir_ + path_).data());
    auto mm_ret = mmap(nullptr, mm_file_stat_.st_size,
                       PROT_READ, MAP_PRIVATE, src_fd, 0);
    if (mm_ret == MAP_FAILED) {
        ErrorContent(buffer, "Requested file not found!");
    }
    mm_file_ = static_cast<char*>(mm_ret);
    close(src_fd);
    buffer.Append("Content-length: " + std::to_string(mm_file_stat_.st_size)
        + "\r\n\r\n");
}

void HttpResponse::ErrorHtml() {
    if (CODE_PATH.count(code_)) {
        path_ = CODE_PATH.find(code_)->second;
        stat((src_dir_ + path_).data(), &mm_file_stat_);
    }
}

std::string HttpResponse::GetFileType() {
    std::string::size_type idx = path_.find_last_of('.');
    if (idx == std::string::npos) {
        return "text/plain";
    }
    std::string suffix = path_.substr(idx);
    if (SUFFIX_TYPE.count(suffix)) {
        return SUFFIX_TYPE.find(suffix)->second;
    } else {
        return "text/plain";
    }
}


