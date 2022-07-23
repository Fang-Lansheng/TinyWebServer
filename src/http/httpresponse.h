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

// Parse HTTP response message
class HttpResponse {
public:
    // ctor
    HttpResponse();

    // dtor
    ~HttpResponse();

    // HTTP responser initialization
    void Init(const std::string& src_dir, std::string& path,
              bool is_keep_alive = false, int code = -1);

    // Generate the HTTP response message.
    void MakeResponse(Buffer& buffer);

    // Deallocate the mapping from the file to the memory.
    void UnmapFile();

    // Get the file.
    char* File();

    // Get the file length.
    size_t FileLength() const;

    // Generate an error response when a client or server error occurs.
    void ErrorContent(Buffer& buffer, const std::string& message) const;

    // Return the status code.
    int Code() const;

private:
    // Generate the Status-Line of the HTTP response, including
    // the HTTP version, the status code and the reason phrase
    // (for example, "HTTP/1.1 200 OK").
    void AddStateLine(Buffer& buffer);

    // Generate the header fields of the HTTP response.
    void AddHeader(Buffer& buffer);

    // Generate the message body of the HTTP response.
    void AddContent(Buffer& buffer);

    // Redirect to the error page if a client or server error occurs.
    void ErrorHtml();

    // Get the content type.
    std::string GetFileType();

private:
    // The status code.
    int code_;

    // HTTP keep-alive
    bool is_keep_alive_;

    // The path of the file.
    std::string path_;

    // The root directory of the resources.
    std::string src_dir_;

    // The pointer to the mapped region in memory.
    char* mm_file_;

    // The information about the file.
    struct stat mm_file_stat_{};

    // The mapping from the file extension to the content type.
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;

    // The mapping from the status code to the reason phrase.
    static const std::unordered_map<int, std::string> CODE_STATUS;

    // The mapping from the status code (indicating client/server error)
    // to the path of corresponding HTML file.
    static const std::unordered_map<int ,std::string> CODE_PATH;
};


#endif //TINYWEBSERVER_SRC_HTTP_HTTPRESPONSE_H
