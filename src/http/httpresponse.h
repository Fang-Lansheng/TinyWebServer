//
// Created by Jifan Zhang on 2022/4/13.
//

#ifndef TINYWEBSERVER_SRC_HTTP_HTTPRESPONSE_H
#define TINYWEBSERVER_SRC_HTTP_HTTPRESPONSE_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unordered_map>

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse {
public:
    HttpResponse();

    ~HttpResponse();

    void Init(const std::string& src_dir, std::string& path,
              bool is_keep_alive = false, int code = -1);

    void MakeResponse(Buffer& buffer);

    void UnmapFile();

    char* File();

    size_t FileLength() const;

    void ErrorContent(Buffer& buffer, std::string message);

    int Code() const;

private:
    void AddStateLine(Buffer& buffer);

    void AddHeader(Buffer& buffer);

    void AddContent(Buffer& buffer);

    void ErrorHtml();

    std::string GetFileType();

    int code_;
    bool is_keep_alive_;
    std::string path_;
    std::string src_dir_;
    char* mm_file_;
    struct stat mm_file_stat_{};

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int ,std::string> CODE_PATH;
};


#endif //TINYWEBSERVER_SRC_HTTP_HTTPRESPONSE_H
