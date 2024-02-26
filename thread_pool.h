#pragma once                                                                                                                                                 
                                                                                                                                                             
#include <future>                                                                                                                                            
#include <functional>                                                                                                                                        
                                                                                                                                                             
#include <iostream>                                                                                                                                          
#include <queue>                                                                                                                                             
#include <set>                                                                                                                                               
#include <vector>                                                                                                                                            
#include <mutex>                                                                                                                                             
#include <utility>                                                                                                                                           
#include <string>                                                                                                                                            
#include <memory>                                                                                                                                            
                                                                                                                                                             
namespace nio {                                                                                                                                              
                                                                                                                                                             
class ThreadPool {                                                                                                                                           
protected:                                                                                                                                                   
    struct TaskFunc {                                                                                                                                        
        TaskFunc(uint64_t expireTime) : _expireTime(expireTime) {                                                                                            
        }                                                                                                                                                    
        std::function<void()> _func;                                                                                                                         
        uint64_t _expireTime = 0;                                                                                                                            
    };                                                                                                                                                       
                                                                                                                                                             
    typedef std::shared_ptr<TaskFunc> TaskFuncPtr;                                                                                                           
public:                                                                                                                                                      
    ThreadPool();                                                                                                                                            
    virtual ~ThreadPool();                                                                                                                                   
    void init(size_t num);                                                                                                                                   
    size_t getThreadNum() {                                                                                                                                  
        std::unique_lock<std::mutex> lock(_mutex);                                                                                                           
        return _threads.size();                                                                                                                              
    }                                                                                                                                                        
                                                                                                                                                             
    size_t getJobNum() {                                                                                                                                     
        std::unique_lock<std::mutex> lock(_mutex);                                                                                                           
        return _tasks.size();                                                                                                                                
    }                                                                                                                                                        
                                                                                                                                                             
    void stop();                                                                                                                                             
                                                                                                                                                             
    void start();                                                                                                                                            
                                                                                                                                                             
    template<class F, class... Args>                                                                                                                         
    auto exec(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {                                                                                  
        return exec(0, f, args...);     
    }

    template<class F, class... Args>
    auto exec(int64_t timeoutMs, F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        int expireTime = timeoutMs == 0 ? 0 : timeoutMs;
        using RetType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        TaskFuncPtr fPtr = std::make_shared<TaskFunc>(expireTime);

        fPtr->_func = [task](){(*task)();};
        std::unique_lock<std::mutex> lock(_mutex);
        _tasks.push(fPtr);
        _condition.notify_one();

        return task->get_future();
    }

    bool waitForAllDone(int millsecond = -1);

protected:
    bool get(TaskFuncPtr& task);
    bool isTerminate() {return _bTerminate;}
    void run();
protected:
    std::queue<TaskFuncPtr> _tasks;
    std::vector<std::thread*> _threads;
    std::mutex _mutex;
    std::condition_variable _condition;
    size_t _threadNum;
    bool _bTerminate;
    std::atomic<int> _atomic{0};
};

class ThreadPoolHash {
public:
    ThreadPoolHash();
    virtual ~ThreadPoolHash();

    ThreadPoolHash(const ThreadPoolHash&) = delete;
    ThreadPoolHash& operator = (const ThreadPoolHash&) = delete;

    void init(size_t num);

    void stop();

    void start();
    template<class F, class... Args>
    auto exec(const std::string& hashKey, F&& f, Args&&... args)->std::future<decltype(f(args...))> {
        return exec(hashKey, 0, f, args...);
    }

    template<class F, class... Args>
    auto exec(const std::string& hashkey,int64_t timeoutMs, F&& f, Args&&... args)->std::future<decltype(f(args...))> {
        ThreadPool* thread = selectThread(hashkey);
        if (thread) {
            return thread->exec(timeoutMs,f, args...);
        } else {
        }
    }

protected:
    ThreadPool* selectThread(const std::string& hashkey);
private:
    std::vector<ThreadPool*> _pools;
};

}
