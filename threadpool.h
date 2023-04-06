// encode UTF-8

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<thread>
#include<condition_variable>
#include<mutex>
#include<vector>
#include<queue>
#include<future>

class ThreadPool{
private:
    bool m_stop;
    std::vector<std::thread>m_thread;
    std::queue<std::function<void()>>tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv;

public:
    explicit ThreadPool(size_t threadNumber):m_stop(false){
        for(size_t i=0;i<threadNumber;++i)
        {
            // 所有创建的线程都被加入到成员变量 m_thread 中
            m_thread.emplace_back(
                [this](){
                    for(;;)
                    {
                        std::function<void()>task;
                        {
                            // unique_lock 被用来在条件变量上等待和保护临界区
                            std::unique_lock<std::mutex>lk(m_mutex);
                            // 等待的条件是当 m_stop 为 true 时或者任务队列 tasks 不为空时，线程就可以被唤醒
                            // 阻塞时自动释放锁，当被唤醒时重新获得锁，在结束此轮循环时 lk 析构解锁
                            m_cv.wait(lk,[this](){ return m_stop||!tasks.empty();});
                            // 线程会退出死循环
                            if(m_stop&&tasks.empty()) return;
                            // 线程会从任务队列中取出一个任务并执行
                            task=std::move(tasks.front());
                            tasks.pop();
                        }
                        task();
                    }
                }
            );
        }
    }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;

    ThreadPool & operator=(const ThreadPool &) = delete;
    ThreadPool & operator=(ThreadPool &&) = delete;

    ~ThreadPool(){
        // 先加锁，确保其他线程无法修改 m_stop 标志位
        {
            std::unique_lock<std::mutex>lk(m_mutex);
            m_stop=true;
        }
        // 通知所有线程 m_stop 标志位已经置为 true，让所有线程退出循环
        m_cv.notify_all();
        // 等待所有线程结束
        for(auto& threads:m_thread)
        {
            threads.join();
        }
    }
    /* submit函数用于向线程池提交一个任务 */
    template<typename F,typename... Args>
    // 用 decltype 推导出函数 f 的返回值类型，并返回一个 std::future 对象, 用于异步地获取函数执行的结果
    auto submit(F&& f,Args&&... args)->std::future<decltype(f(args...))>{
        // std::make_shared 用于在堆上分配一个指向 std::packaged_task 对象的智能指针 taskPtr，
        // 以便在返回 std::future 之后继续保持该对象的生命周期
        auto taskPtr=std::make_shared<std::packaged_task<decltype(f(args...))()>>(
            // 使用 std::bind 将可变数量的参数传递给函数 f
            std::bind(std::forward<F>(f),std::forward<Args>(args)...)
        );
        {
            std::unique_lock<std::mutex>lk(m_mutex);
            if(m_stop) throw std::runtime_error("submit on stopped ThreadPool");
            tasks.emplace([taskPtr](){ (*taskPtr)(); });
        }
        // 唤醒一个等待中的线程，以便执行新提交的任务。
        m_cv.notify_one();
        return taskPtr->get_future();

    }
};

#endif //THREADPOOL_H

/* how to use */

// ThreadPool pool(4);
// std::vector<std::future<int>> results;
// for(int i = 0; i < 8; ++i) {
//     results.emplace_back(pool.submit([]() {
//         // computing task and return result
//     }));
// }
// for(auto && result: results)
//     std::cout << result.get() << ' ';
