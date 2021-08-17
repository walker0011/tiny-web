/*
	功能：对线程同步机制进行封装，包含信号量/条件变量/互斥量
	作者：hwd
	时间：2021.8.17
	参考：<Linux高性能服务器编程>
*/
#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

// 信号量类的封装
class sem
{
public:
	// 创建并初始化信号量
	sem()
	{
		
		if(sem_init(&m_sem, 0,0) != 0)
		{
			throw std::exception();
		}

	}
	
	sem(int num)
	{
		if(sem_init(&m_sem, 0, num) != 0)
		{
			throw std::exception();
		}
	}

	~sem()
	{
		sem_destroy(&m_sem);
	}

	bool wait()
	{
		return sem_wait(&m_sem) == 0;
	}

	bool post()
	{
		return sem_post(&m_sem) == 0;
	}
private:
	sem_t m_sem;
};

//  互斥量的封装
class locker
{
public:
	locker()
	{
		if(pthread_mutex_init(&m_mutex, NULL) != 0)
			throw std::expcetion();

	}
	~locker()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	bool lock()
	{
		return  pthread_mutex_lock(&m_mutex) == 0;
	}
	bool unlock()
	{
		return pthread_mutex_unlock(&m_mutex) == 0;
	}
	pthread_mutex * get()
	{
		return &m_mutex;
	}
private:
	pthread_mutex_t m_mutex;
};

// 条件变量的封装
class cond
{
public:
	cond()
	{
		if(pthread_cond_init(&m_cond, NULL) != 0)
			throw_std::exception();
	}
	~cond()
	{
		pthread_cond_destroy(&m_cond);
	}
	// wait和timedwait没有进行加锁和解锁操作，
	// 说明传进去的锁就是加锁的，使用之后在外部解锁
	bool wait(pthread_mutex_t *m_mutex)
	{
		int ret = 0;
		ret = pthread_cond_wait(&m_cond, m_mutex);
		return ret == 0;
	}

	bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
	{
		int ret = 0;
		ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
		return ret == 0;
	}

	bool signal()
	{
		return pthread_cond_signal(&m_cond) == 0;
	}

	bool broadcast()
	{
		return pthread_cond_broadcast(&m_cond) == 0;
	}
private:
	pthread_cond_t m_cond;
};
#endif

