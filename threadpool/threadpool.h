#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio.h>
#include <exception>
#include <pthread.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T>
class threadpool
{
public:
	// thread_number：表示线程池中的线程数量，max_request表示请求队列
	//中最多允许的，得到处理的连接请求数量
	threadpool(int actor_model, connection_pool * connPool, 
				int thread_number = 8, int max_requests = 10000);
	~threadpool();	

	bool append(T * request, int state);
	bool append_p(T * request);
private:
	static void *work(void *arg);
	void run();
private: 
	int m_thread_number;          // 线程池中线程数量
	int m_max_requests;           // 请求队列中允许的最大请求数
	pthread_t * m_threads;        // 描述线程池的数组
	
	std::list<T*> m_workqueue;    // 请求队列
	locker m_queuelocker;         // 保护请求队列的互斥锁
	sem m_queuestat;              // 是否有任务需要处理
	
	connection_pool *m_connPool;  // 数据库
	int m_actor_model;            //模型切换

};

/* Summary:线程池的构造函数
** parameter:
** Return 

*/
template <typename T>
threadpool<T>:threadpool(int actor_model, connection_pool *connPool
						, int thread_number, int max_requests)
						: m_actor_model(actor_mode)
						, m_thread_number(thread_number)
						, m_max_requests(max_requests)
						, m_threads(NULL), m_connPool(connPool)
{
	if(thread_numebr <= 0 || max_requests <= 0)
		throw std::exception();

	// 创建线程标识数组
	m_threads = new pthread_t[m_thread_number];
	if( !m_thread)
		throw std::exception();
	for(int i = 0; i < thread_number; ++ i)
	{
		// 创建线程
		if(pthread_create(mthread + i, NULL, work, this) != 0)
		{
			delete[] m_threads;
			throw std::exception();
		}
		// 设置线程脱离，从而不需要在主线程中回收
		if(pthread_detach(mthread[i])
		{
			delete[] m_threads;
			throw std::exception();
		}
	}
}

// 线程池析构函数
template <typename T>
threadpool<T>::~threadpool()
{
	delete[] m_threads;
}


// 向请求队列中添加请求
template <typename T>
bool threadpool<T>::append(T* request, int state)
{

	m_queuelocker.lock();  // 对请求队列加锁
	if(m_workqueue.size() >= m_max_requests)
	{
		m_queuelocker.unlock();
		return false;
	}
	request->m_state = state;
	m_workqueue.push_back(request);
	m_queuelocker.unlock();
	m_queuestat.post();
	return true;
}


// 向请求队列中添加请求
template <typename T>
bool threadpool<T>::append_p(T* request)
{

	m_queuelocker.lock();  // 对请求队列加锁
	if(m_workqueue.size() >= m_max_requests)
	{
		m_queuelocker.unlock();
		return false;
	}
	request->m_state = state;
	m_workqueue.push_back(request);
	m_queuelocker.unlock();
	m_queuestat.post();
	return true;
}

// 线程回调函数，静态
template <typename T>
void * threadpool<T>::worker(void *arg)
{
	threadpool *pool = (threadpool*)arg;
	pool->run();
	return pool;
}

template <typename T>
void threadpool<T>::run()
{
	while(true)
	{	
		// 工作线程等待请求队列中有任务
		m_queuestat.wait();
		m_queuelocker.lock();
		if(m_workqueue.empty())
		{
			m_queuelocker.unlock();
			continue;
		}
		
		T * request = m_workqueue.front();
		m_workqueue.pop_front();
		m_queuelock.unlock();

		if(!request)
			continue;
		if(1 == m_actor_model)
		{
			if(0 == request->state)
			{
				if(request->read_once())
				{

					request->improv = 1;
					connectionRAII mysqlcon(&request->mysql, m_connPool);
					request->process();
				}
				else
				{
					request->improv = 1;
					request->timer_flag = 1;
				}
			}
			else
			{
				if(request->write())
				{
					request->improv = 1;
				}
				else
				{
					request->improv = 1;
					request->timer_flag = 1;
				}
			}

		}
		else
		{
			connectionRAII mysqlcon(&request->mysql, m_connPool);
			retuest->process();
		}
	}
}

#endif

