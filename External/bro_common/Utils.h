#pragma once
#include <set>
#include "SyncObjects.h"

#include "CustomAllocator.h"

//////////////////////////////////////////////////////////////////////////
template <typename T>
class NumberPoolT
{
public:
	NumberPoolT(T initialNumber = 1) : m_initialNumber(initialNumber) { }

	typename typedef std::set<T, std::less<T>, BroCustomAllocator<T> > TPool;
	typename typedef TPool::iterator TPoolIT;
	T GenerateNumber();
	void ReturnToPool(T value);

private:
	T					m_initialNumber;
	CCriticalSection	m_csKey;
	TPool				m_pool;
};

template <typename T>
T NumberPoolT<T>::GenerateNumber()
{
	TLock lo(m_csKey);

	T number;
	if (m_pool.empty())
	{
		number = m_initialNumber++;
	}
	else
	{
		TPoolIT it = m_pool.begin();
		number = *it;
		m_pool.erase(it);
	}
	return number;
}

template <typename T>
void NumberPoolT<T>::ReturnToPool(T value)
{
	TLock lo(m_csKey);

	m_pool.insert(value);
}
//////////////////////////////////////////////////////////////////////////
