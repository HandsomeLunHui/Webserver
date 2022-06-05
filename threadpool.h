#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>   //链表实现一个等待队列
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

using namespace std;

// 线程池类，将它定义为模板类是为了代码复用，模板参数T是任务类
template<typename T>
class threadpool {
public:
    /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();   //线程池析构函数 将线程池数组析构并结束线程
    bool append(T* request);    //添加到队列尾部

private:
    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    static void* worker(void* arg);
    void run();

private:
    // 线程的数量
    int m_thread_number;  
    
    // 描述线程池的数组，大小为m_thread_number    
    pthread_t * m_threads;

    // 请求队列中最多允许的、等待处理的请求的数量  最大请求数量
    int m_max_requests; 
    
    // 请求队列
    std::list< T* > m_workqueue;  //使用一个链表来存储请求队列(也可以叫等待队列)

    // 保护请求队列的互斥锁
    locker m_queuelocker;   

    // 是否有任务需要处理
    sem m_queuestat;

    // 是否结束线程 true 为结束线程
    bool m_stop;
};

//构造函数 初始化线程数量 最大请求数
template< typename T >
threadpool< T >::threadpool(int thread_number, int max_requests) : 
        m_thread_number(thread_number), m_max_requests(max_requests), 
        m_stop(false), m_threads(NULL) {

    if((thread_number <= 0) || (max_requests <= 0) ) {
        throw std::exception();   //判断输入请求的线程数量与最大请求数是否合法
    }
    //合法就在堆中申请number个线程
    m_threads = new pthread_t[m_thread_number];  
    if(!m_threads) {  //判断是否申请失败
        throw std::exception();
    }

    // 创建thread_number 个线程，并将他们设置为脱离线程。
    for ( int i = 0; i < thread_number; ++i ) {
        printf( "create the %dth thread\n", i);
        if(pthread_create(m_threads + i, NULL, worker, this ) != 0) {
            delete [] m_threads;
            throw std::exception();
        }
        
        if( pthread_detach( m_threads[i] )!=0 ) {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template< typename T >
threadpool< T >::~threadpool() {
    delete [] m_threads;   //释放申请的内存
    m_stop = true;   //true终止线程
}

template< typename T >
bool threadpool< T >::append( T* request )   //向队列中添加一个请求
{
    // 操作工作队列时一定要加锁，因为它被所有线程共享。
    m_queuelocker.lock();
    if ( m_workqueue.size() > m_max_requests ) {   //如果队列的长度大于了最大请求数
        m_queuelocker.unlock();  //解锁并返回
        return false;
    }
    //否则将新请求添加到队列中
    m_workqueue.push_back(request);
    m_queuelocker.unlock();   //解锁
    m_queuestat.post();    //信号量+1
    return true;  //返回成功
}

template< typename T >
void* threadpool< T >::worker( void* arg )   //工作函数
{
    threadpool* pool = ( threadpool* )arg;   //线程执行运行函数 
    pool->run();
    return pool;
}

template< typename T >
void threadpool< T >::run() {
    //如果m_stop不为真
    while (!m_stop) {
        m_queuestat.wait();  //信号量-1 
        m_queuelocker.lock();
        if ( m_workqueue.empty() ) {
            m_queuelocker.unlock();
            continue;
        }
        //不为空就将队列第一位取出
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if ( !request ) {
            continue;
        }
        request->process();
    }

}

#endif
