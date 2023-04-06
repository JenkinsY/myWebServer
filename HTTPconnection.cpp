// encode UTF-8

#include "HTTPconnection.h"

const char* HTTPconnection::srcDir;
std::atomic<int> HTTPconnection::userCount;
bool HTTPconnection::isET;

HTTPconnection::HTTPconnection() { 
    fd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
};

HTTPconnection::~HTTPconnection() { 
    closeHTTPConn(); 
};

void HTTPconnection::initHTTPConn(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuffer_.initPtr();
    readBuffer_.initPtr();
    isClose_ = false;
}

void HTTPconnection::closeHTTPConn() {
    response_.unmapFile_();
    if(isClose_ == false){
        isClose_ = true; 
        userCount--;
        close(fd_);
    }
}

int HTTPconnection::getFd() const {
    return fd_;
};

struct sockaddr_in HTTPconnection::getAddr() const {
    return addr_;
}

const char* HTTPconnection::getIP() const {
    return inet_ntoa(addr_.sin_addr);
}

int HTTPconnection::getPort() const {
    return addr_.sin_port;
}

/* 读方法，ET模式会将缓存读空 */
// 返回最后一次读取的长度，以及错误类型
ssize_t HTTPconnection::readBuffer(int* saveErrno) {
    // 最后一次读取的长度
    ssize_t len = -1;
    do {
        len = readBuffer_.readFd(fd_, saveErrno);
        //std::cout<<fd_<<" read bytes:"<<len<<std::endl;
        if (len <= 0) {
            break;
        }
    } while (isET);
    return len;
}

/* 写方法,响应头和响应体是分开的，要用iov实现写操作 */
ssize_t HTTPconnection::writeBuffer(int* saveErrno) {
    // 最后一次写入的长度
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        // 缓存为空，传输完成
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) { break; } /* 传输结束 */
        // 响应头已经写完，开始将iov_[1]中的文件内容写入fd
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {
            // 更新响应体传输起点和长度
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            // 响应头不再需要传输
            if(iov_[0].iov_len) {
                // 响应头保存在写缓存中，全部回收即可
                writeBuffer_.initPtr();
                iov_[0].iov_len = 0;
            }
        }
        // 响应头还没传输完成
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len; 
            writeBuffer_.updateReadPtr(len);
        }
    } while(isET || writeBytes() > 10240);  // 一次最多传输 10KB 数据
    return len;
}

/* 处理方法：解析读缓存内的请求报文，判断是否完整 */
// 不完整返回false，完整在写缓存内写入响应头，并获取响应体内容（文件）
bool HTTPconnection::handleHTTPConn() {
    request_.init();
    if(readBuffer_.readableBytes() <= 0) {
        //std::cout<<"readBuffer is empty!"<<std::endl;
        return false;
    }
    else if(request_.parse(readBuffer_)) {
        response_.init(srcDir, request_.path(), request_.isKeepAlive(), 200);
    }else {
        std::cout<<"400!"<<std::endl;
        //readBuffer_.printContent();
        response_.init(srcDir, request_.path(), false, 400);
    }

    response_.makeResponse(writeBuffer_);
    /* 响应头 */
    iov_[0].iov_base = const_cast<char*>(writeBuffer_.curReadPtr());
    iov_[0].iov_len = writeBuffer_.readableBytes();
    iovCnt_ = 1;

    /* 响应体 文件 */
    if(response_.fileLen() > 0  && response_.file()) {
        iov_[1].iov_base = response_.file();
        iov_[1].iov_len = response_.fileLen();
        iovCnt_ = 2;
    }
    return true;
}
