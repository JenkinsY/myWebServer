// encode UTF-8

#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>  //open
#include <unistd.h> //close
#include <sys/stat.h> //stat
#include <sys/mman.h> //mmap,munmap
#include <assert.h>

#include "buffer.h"

class HTTPresponse
{
public:
    HTTPresponse();
    ~HTTPresponse();

    void init(const std::string& srcDir,std::string& path,bool isKeepAlive=false,int code=-1);
    void makeResponse(Buffer& buffer);
    void unmapFile_();
    char* file();
    size_t fileLen() const;
    void errorContent(Buffer& buffer,std::string message);
    int code() const {return code_;}


private:
    void addStateLine_(Buffer& buffer);
    void addResponseHeader_(Buffer& buffer);
    void addResponseContent_(Buffer& buffer);

    // 针对 4XX 的状态码
    void errorHTML_();
    std::string getFileType_();

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;

    // 使用了共享内存
    char* mmFile_;
    struct  stat mmFileStat_;

    // SUFFIX_TYPE 表示后缀名到文件类型的映射关系，CODE_STATUS 表示状态码到相应状态 (字符串类型) 的映射。
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};

#endif //HTTP_RESPONSE_H

/* response example
HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
ETag: "34aa387-d-1568eb00"
Accept-Ranges: bytes
Content-Length: 51
Vary: Accept-Encoding
Content-Type: text/plain
*/