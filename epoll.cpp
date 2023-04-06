// encode UTF-8

#include "epoller.h"

// epoll_create() 在内核中创建 epoll 实例并返回一个 epoll 文件描述符
Epoller::Epoller(int maxEvent):epollerFd_(epoll_create(512)), events_(maxEvent){
    assert(epollerFd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() {
    close(epollerFd_);
}

bool Epoller::addFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollerFd_, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::modFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollerFd_, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::delFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollerFd_, EPOLL_CTL_DEL, fd, &ev);
}

/*
当 timeout 为 0 时，epoll_wait 永远会立即返回。
而 timeout 为 -1 时，epoll_wait 会一直阻塞直到任一已注册的事件变为就绪。
当 timeout 为一正整数时，epoll 会阻塞直到计时 timeout 毫秒终了或已注册的事件变为就绪。
因为内核调度延迟，阻塞的时间可能会略微超过 timeout 毫秒。
*/
int Epoller::wait(int timeoutMs) {
    return epoll_wait(epollerFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

int Epoller::getEventFd(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

uint32_t Epoller::getEvents(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}