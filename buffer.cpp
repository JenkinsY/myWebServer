// encode UTF-8

#include "buffer.h"

Buffer::Buffer(int initBuffersize):buffer_(initBuffersize),readPos_(0),writePos_(0){}

size_t Buffer::readableBytes() const
{
    return writePos_-readPos_;
}

size_t Buffer::writeableBytes() const
{
    return buffer_.size()-writePos_;
}

size_t Buffer::readBytes() const
{
    return readPos_;
}

const char* Buffer::curReadPtr() const
{
    return BeginPtr_()+readPos_;
}

const char* Buffer::curWritePtrConst() const
{
    return BeginPtr_()+writePos_;
}

char* Buffer::curWritePtr()
{
    return BeginPtr_()+writePos_;
}

void Buffer::updateReadPtr(size_t len)
{
    assert(len<=readableBytes());
    readPos_+=len;
}

void Buffer::updateReadPtrUntilEnd(const char* end)
{
    assert(end>=curReadPtr());
    updateReadPtr(end-curReadPtr());
}

void Buffer::updateWritePtr(size_t len)
{
    assert(len<=writeableBytes());
    writePos_+=len;
}

void Buffer::initPtr()
{
    bzero(&buffer_[0],buffer_.size());
    readPos_=0;
    writePos_=0;
}

void Buffer::allocateSpace(size_t len)
{
    //如果buffer_里面剩余的空间有len就进行调整，否则需要申请空间。
    //剩余空间包括write指针之前的空间和可写的空间
    if(writeableBytes()+readBytes()<len)
    {
        buffer_.resize(writePos_+len+1);
    }
    else{
        //将读指针置为0,调整一下
        size_t readable=readableBytes();
        std::copy(BeginPtr_()+readPos_,BeginPtr_()+writePos_,BeginPtr_());
        readPos_=0;
        writePos_=readable;
        assert(readable==readableBytes());
    }
}

void Buffer::ensureWriteable(size_t len)
{
    if(writeableBytes()<len)
    {
        allocateSpace(len);
    }
    assert(writeableBytes()>=len);
}

void Buffer::append(const char* str,size_t len)
{
    assert(str);
    ensureWriteable(len);
    std::copy(str,str+len,curWritePtr());
    updateWritePtr(len);
}

void Buffer::append(const std::string& str)
{
    append(str.data(),str.length());
}

void Buffer::append(const void* data,size_t len)
{
    assert(data);
    append(static_cast<const char*>(data),len);
}

void Buffer::append(const Buffer& buffer)
{
    append(buffer.curReadPtr(),buffer.readableBytes());
}

ssize_t Buffer::readFd(int fd,int* Errno)
{
    char buff[65535];//暂时的缓冲区
    struct iovec iov[2];
    const size_t writable=writeableBytes();

    // buff 为暂存区，buffer写满后会往buff中写
    iov[0].iov_base=BeginPtr_()+writePos_;
    iov[0].iov_len=writable;
    iov[1].iov_base=buff;
    iov[1].iov_len=sizeof(buff);

    // readv会将读取的数据往iov中填充，iov[0]填满后往iov[1]继续填充
    const ssize_t len=readv(fd,iov,2);
    if(len<0)
    {
        //std::cout<<"从fd读取数据失败！"<<std::endl;
        *Errno=errno;
    }
    else if(static_cast<size_t>(len)<=writable)
    {
        writePos_+=len;
    }
    // 此时writePos_指向buffer尾部，将buff中数据写进buffer中会导致扩容
    else{
        writePos_=buffer_.size();
        append(buff,len-writable);
    }
    return len;
}

ssize_t Buffer::writeFd(int fd,int* Errno)
{
    size_t readSize=readableBytes();
    ssize_t len=write(fd,curReadPtr(),readSize);
    if(len<0)
    {
        //std::cout<<"往fd写入数据失败！"<<std::endl;
        *Errno=errno;
        return len;
    }
    readPos_+=len;
    return len;
}

std::string Buffer::AlltoStr()
{
    std::string str(curReadPtr(),readableBytes());
    initPtr();
    return str;
}

char* Buffer::BeginPtr_()
{
    return &*buffer_.begin();
}

// 区分迭代器和指针的区别
const char* Buffer::BeginPtr_() const
{
    return &*buffer_.begin();
}