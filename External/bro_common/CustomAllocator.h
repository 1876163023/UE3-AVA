/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: ComDef

	Name: BroCustomAllocator.h

	Description: Custom memory allocator for stl containers.

***/

#pragma once

#include <cstdlib>
#include <xmemory>


#ifndef _FARQ	/* specify standard memory model */
 #define _FARQ
 #define _PDFT	ptrdiff_t
 #define _SIZT	size_t
#endif /* _FARQ */



class IbroMemAlloc
{
public:
	virtual void* Alloc(unsigned long size) = 0;
	virtual void Free(void *mem) = 0;
};

class CbroDefaultMemAlloc : public IbroMemAlloc
{
public:
	void* Alloc(unsigned long size)
	{
		return operator new(size);
	}
	void Free(void *mem)
	{
		::operator delete(mem);
	}
};


class CbroMemAlloc
{
public:
	static void Init(IbroMemAlloc *alloc)
	{
		pAlloc = alloc;
	}

	template<class _Ty> static void* Alloc(_SIZT count, _Ty _FARQ *) throw()
	{
		if (pAlloc)
			return pAlloc->Alloc(count * sizeof(_Ty));
		else
			return 0;
	}

	template<class _Ty> static void Free(_Ty _FARQ *mem)
	{
		if (pAlloc)
			pAlloc->Free((void*)mem);
	}

	static CbroDefaultMemAlloc defaultAlloc;
	static IbroMemAlloc *pAlloc;
};



template<class _Ty>
class BroCustomAllocator
{
public:
	typedef _Ty value_type;
	typedef value_type _FARQ *pointer;
	typedef value_type _FARQ& reference;
	typedef const value_type _FARQ *const_pointer;
	typedef const value_type _FARQ& const_reference;

	typedef _SIZT size_type;
	typedef _PDFT difference_type;

	template<class _Other>
	struct rebind
	{
		typedef BroCustomAllocator<_Other> other;
	};

	pointer address(reference _Val) const
	{	// return address of mutable _Val
		return (&_Val);
	}

	const_pointer address(const_reference _Val) const
	{	// return address of nonmutable _Val
		return (&_Val);
	}

	BroCustomAllocator() throw()
	{	// construct default allocator (do nothing)
	}

	BroCustomAllocator(const BroCustomAllocator<_Ty>&) throw()
	{	// construct by copying (do nothing)
	}

	template<class _Other>
	BroCustomAllocator(const BroCustomAllocator<_Other>&) throw()
	{	// construct from a related allocator (do nothing)
	}

	template<class _Other>
	BroCustomAllocator<_Ty>& operator=(const BroCustomAllocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	void deallocate(pointer _Ptr, size_type)
	{	// deallocate object at _Ptr, ignore size
		CbroMemAlloc::Free(_Ptr);
	}

	pointer allocate(size_type _Count)
	{	// allocate array of _Count elements
		return static_cast<pointer>(CbroMemAlloc::Alloc(_Count, (pointer)0));
	}

	pointer allocate(size_type _Count, const void _FARQ *)
	{	// allocate array of _Count elements, ignore hint
		return (allocate(_Count));
	}

	void construct(pointer _Ptr, const _Ty& _Val)
	{	// construct object at _Ptr with value _Val
		std::_Construct(_Ptr, _Val);
	}

	void destroy(pointer _Ptr)
	{	// destroy object at _Ptr
		std::_Destroy(_Ptr);
	}

	_SIZT max_size() const throw()
	{	// estimate maximum array size
		_SIZT _Count = (_SIZT)(-1) / sizeof (_Ty);
		return (0 < _Count ? _Count : 1);
	}
};




// allocator TEMPLATE OPERATORS
template<class _Ty, class _Other>
inline bool operator==(const BroCustomAllocator<_Ty>&, const BroCustomAllocator<_Other>&) throw()
{	// test for allocator equality (always true)
	return (true);
}

template<class _Ty, class _Other>
inline bool operator!=(const BroCustomAllocator<_Ty>&, const BroCustomAllocator<_Other>&) throw()
{	// test for allocator inequality (always false)
	return (false);
}
