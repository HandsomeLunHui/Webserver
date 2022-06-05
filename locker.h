#ifndef LOCKER_H
#define LOCKER_H

#include <exception>   //异常
#include <pthread.h>   //线程
#include <semaphore.h>  //信号量

// 线程同步机制封装类

// 互斥锁类
class locker {
public:
    locker() {
        if(pthread_mutex_init(&m_mutex, NULL) != 0) {   //初始化互斥量 ！=0 则证明初始化失败
            throw std::exception();
        }
    }

    ~locker() { 
        pthread_mutex_destroy(&m_mutex);  //析构函数 释放信号量
    }

    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;  //加锁 ==0 则证明加锁解锁成功
    }

    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;   //解锁
    }

    pthread_mutex_t *get()  //获取当前信号量
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;   //成员函数
};


// 条件变量类 等待某一条件发生
/*pthread_cond_init()函数               功能：初始化一个条件变量

pthread_cond_wait()函数             功能：阻塞等待一个条件变量

pthread_cond_timedwait()函数    功能：限时等待一个条件变量

pthread_cond_signal()函数          功能：唤醒至少一个阻塞在条件变量上的线程

pthread_cond_broadcast()函数    功能：唤醒全部阻塞在条件变量上的线程

pthread_cond_destroy()函数        功能：销毁一个条件变量

以上6 个函数的返回值都是：成功返回0， 失败直接返回错误号。

pthread_cond_t 类型，其本质是一个结构体。为简化理解，应用时可忽略其实现细节，简单当成整数看待。如：

pthread_cond_t  cond; 变量cond只有两种取值1、0。*/

class cond {
public:
    cond(){
        if (pthread_cond_init(&m_cond, NULL) != 0) {   //初始化条件变量
            throw std::exception();
        }
    }
    ~cond() {
        pthread_cond_destroy(&m_cond);   //析构函数 释放信号量
    }

    bool wait(pthread_mutex_t *m_mutex) {
        int ret = 0;
        ret = pthread_cond_wait(&m_cond, m_mutex);   //无条件等待
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t) {
        int ret = 0;
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);   //限时等待
        return ret == 0;
    }
    bool signal() {
        return pthread_cond_signal(&m_cond) == 0;    //唤醒至少一个阻塞的线程
    }
    bool broadcast() {
        return pthread_cond_broadcast(&m_cond) == 0;   //唤醒全部的线程
    }

private:
    pthread_cond_t m_cond;
};


// 信号量类
class sem {
public:
    sem() {
        if( sem_init( &m_sem, 0, 0 ) != 0 ) {
            throw std::exception();
        }
    }
    sem(int num) {
        if( sem_init( &m_sem, 0, num ) != 0 ) {
            throw std::exception();
        }
    }
    ~sem() {
        sem_destroy( &m_sem );
    }
    // 等待信号量
    bool wait() {
        return sem_wait( &m_sem ) == 0;   //wait一次sem-1
    }
    // 增加信号量
    bool post() {
        return sem_post( &m_sem ) == 0;   //post一次sem+1
    }
private:
    sem_t m_sem;
};

#endif