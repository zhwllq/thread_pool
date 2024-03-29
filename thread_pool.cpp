#include "thread_pool.h"                                                                                                                                     
                                                                                                                                                             
#include <iostream>                                                                                                                                          
                                                                                                                                                             
namespace nio {                                                                                                                                              
                                                                                                                                                             
ThreadPool::ThreadPool() : _threadNum(1), _bTerminate(false) {                                                                                               
}                                                                                                                                                            
                                                                                                                                                             
ThreadPool::~ThreadPool() {                                                                                                                                  
    stop();                                                                                                                                                  
}                                                                                                                                                            
                                                                                                                                                             
void ThreadPool::init(size_t num) {                                                                                                                          
    std::unique_lock<std::mutex> lock(_mutex);                                                                                                               
    if (!_threads.empty()) {                                                                                                                                 
    }                                                                                                                                                        
                                                                                                                                                             
    _threadNum = num;                                                                                                                                        
}                                                                                                                                                            
                                                                                                                                                             
void ThreadPool::stop() {                                                                                                                                    
    {                                                                                                                                                        
        std::unique_lock<std::mutex> lock(_mutex);                                                                                                           
        _bTerminate = true;                                                                                                                                  
        _condition.notify_all();                                                                                                                             
    }                                                                                                                                                        
    
    for(size_t i = 0; i < _threads.size(); i++) {                                                                                                            
        if(_threads[i]->joinable()) {                                                                                                                        
            _threads[i]->join();                                                                                                                             
        }                                                                                                                                                    
        delete _threads[i];                                                                                                                                  
        _threads[i] = nullptr;                                                                                                                               
    }                                                                                                                                                        
                                                                                                                                                             
    std::unique_lock<std::mutex> lock(_mutex);                                                                                                               
    _threads.clear();                                                                                                                                        
}                                                                                                                                                            
                                                                                                                                                             
void ThreadPool::start() {                                                                                                                                   
    std::unique_lock<std::mutex> lock(_mutex);                                                                                                               
    if (!_threads.empty()) {                                                                                                                                 
    }                                                                                                                                                        
                                                                                                                                                             
    for(size_t i = 0; i < _threadNum; i++) {                                                                                                                 
        _threads.push_back(new std::thread(&ThreadPool::run, this));
    }
}

bool ThreadPool::get(TaskFuncPtr& task) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_tasks.empty()) {
        _condition.wait(lock, [this] { return _bTerminate || !_tasks.empty(); });
    }

    if (_bTerminate) return false;

    if (!_tasks.empty()) {
        task = std::move(_tasks.front());
        _tasks.pop();
        return true;
    }

    return false;
}

void ThreadPool::run() {
    while(!isTerminate()) {
        TaskFuncPtr task;
        bool ok = get(task);
        if (ok) {
            ++_atomic;
            try {
                task->_func();
            } catch(...) {
            }

            --_atomic;

            std::unique_lock<std::mutex> lock(_mutex);

            if (_atomic == 0 && _tasks.empty()) _condition.notify_all();
        }
    }
}

bool ThreadPool::waitForAllDone(int millsecond) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_tasks.empty()) return true;
    if (millsecond < 0) {
        _condition.wait(lock, [this] { return _tasks.empty(); });
        return true;
    } else {
        return _condition.wait_for(lock, std::chrono::milliseconds(millsecond), [this] { return _tasks.empty(); });
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ThreadPoolHash::ThreadPoolHash() {
}

ThreadPoolHash::~ThreadPoolHash() {
}

void ThreadPoolHash::init(size_t num) {
    for(size_t i = 0 ;i < num;i++) {
        ThreadPool* p = new ThreadPool();
        p->init(1);
        _pools.push_back(p);
    }
}

ThreadPool* ThreadPoolHash::selectThread(const std::string& hashkey) {
    if (_pools.empty()) return nullptr;

    std::hash<std::string> hash_fu;
    size_t pos = hash_fu(hashkey) % _pools.size();
    return _pools[pos];
}

void ThreadPoolHash::stop() {
    for (size_t i = 0; i < _pools.size(); i++) {
        _pools[i]->stop();
        delete _pools[i];
    }
    _pools.clear();
}

void ThreadPoolHash::start() {
    for (size_t i = 0; i < _pools.size() ;i ++) {
        _pools[i]->start();
    }
}

}
