
#pragma once
#ifndef __AVA_TEMPLATE__
#define __AVA_TEMPLATE__

namespace ava
{
#define SHELLSORT_MAX 32
	typedef UINT		size_t;

	template<class _SwapType> inline
	void swap( _SwapType& _lhs, _SwapType& _rhs )
	{
		//_SwapType _tmp = _lhs;
		//_lhs = _rhs;
		//_rhs = _tmp;
		appMemswap( &_lhs, &_rhs, sizeof(_SwapType) );
	}

	template<class _Iter> inline
	void iter_swap( _Iter _lhs, _Iter _rhs )
	{
		swap( *_lhs, *_rhs );
	}
	
	template<class _Type1, class _Type2>
	struct pair
	{
		typedef pair<_Type1, _Type2> _MyType;
		typedef _Type1 first_type;
		typedef _Type2 second_type;

		pair() : first( _Type1() ) , second( _Type2() ) {}
		pair( const _Type1& _Value1, const _Type2& _Value2 ) : first(_Value1) , second(_Value2) {}

		template< class _Type1, class _Type2 >
		pair( const pair< _Type1, _Type2 >& _rhs ) : first(_rhs.first) , second(_rhs.second) {}

		void swap( _MyType& _rhs )
		{
			swap( first, _rhs.first );
			swap( second, _rhs.second );
		}

		_Type1 first;
		_Type2 second;
	};

	// according to the wikepedia. keyword 'quicksort'
	template<class _RandomIterator>
	_RandomIterator _InPlace_QuickSort_Partition( _RandomIterator _First, _RandomIterator _Last )
	{
		_RandomIterator _Pivot = _First + (_Last - _First - 1) / 2;
		iter_swap( _Pivot , _Last - 1 );
		_Pivot = _Last - 1;
		
		_RandomIterator _Elem = _First;
		_RandomIterator _Stored = _First;
		for( ; 1 < (_Last - _Elem)  ; ++_Elem )
			if( !(*_Pivot < *_Elem) )
				iter_swap( _Elem, _Stored ), ++_Stored;

		if( !(*_Stored < *(_Last - 1)) )
		{
			iter_swap( _Last - 1, _Stored );
			return _Stored;
		}
		else
			return _Last - 1;
	}

	// according to the wikepedia. keyword 'quicksort'
	template<class _RandomIterator, class _BinaryPredicate>
		_RandomIterator _InPlace_QuickSort_Partition( _RandomIterator _First, _RandomIterator _Last, _BinaryPredicate _Pred )
	{
		_RandomIterator _Pivot = _First + (_Last - _First - 1) / 2;
		iter_swap( _Pivot , _Last - 1 );
		_Pivot = _Last - 1;

		_RandomIterator _Elem = _First;
		_RandomIterator _Stored = _First;
		for( ; 1 < (_Last - _Elem)  ; ++_Elem )
			if( !_Pred( *_Pivot, *_Elem ) )
				iter_swap( _Elem, _Stored ), ++_Stored;

		if( !_Pred( *_Stored ,*(_Last - 1)) )
		{
			iter_swap( _Last - 1, _Stored );
			return _Stored;
		}
		else
			return _Last - 1;
	}

	template<class _RandomIterator, class _BinaryPredicate>
	void _ShellSort( _RandomIterator _First, _RandomIterator _Last )
	{
		size_t _Incr = (_Last - _First) / 2;
		while( _Incr > 0 )
		{
			for( _RandomIterator _Elem = _First ; _Elem != _Last ; ++_Elem )
			{
				size_t _Diff = _Elem - _First;
				_RandomIterator _Sel = _Elem;
				while( (_Diff >= _Incr) &&  (*_Sel < *(_Sel - _Incr) ) )
				{
					iter_swap( _Sel, (_Sel - _Incr) );
					_Sel -= _Incr;
					_Diff -= _Incr;
				}
			}
			_Incr /= 2;
		}
	}

	template<class _RandomIterator, class _BinaryPredicate>
	void _ShellSort( _RandomIterator _First, _RandomIterator _Last, _BinaryPredicate _Pred )
	{
		size_t _Incr = (_Last - _First) / 2;
		while( _Incr > 0 )
		{
			for( _RandomIterator _Elem = _First ; _Elem != _Last ; ++_Elem )
			{
				size_t _Diff = _Elem - _First;
				_RandomIterator _Sel = _Elem;
				while( (_Diff >= _Incr) && _Pred( *_Sel , *(_Sel - _Incr) ) )
				{
					iter_swap( _Sel, (_Sel - _Incr) );
					_Sel -= _Incr;
					_Diff -= _Incr;
				}
			}
			_Incr /= 2;
		}
	}


	template<class _RandomIterator> inline
	void sort(_RandomIterator _First, _RandomIterator _Last)
	{
		if( 1 < (_Last - _First) )
		{
			_RandomIterator _NewPivot = _InPlace_QuickSort_Partition( _First, _Last );
			sort( _First, _NewPivot);
			sort( (_NewPivot + 1), _Last);
		}
	}

	template<class _RandomIterator, class _BinaryPredicate> inline
	void sort(_RandomIterator _First, _RandomIterator _Last, _BinaryPredicate _Pred)
	{
		if( 1 < (_Last - _First) )
		{
			_RandomIterator _NewPivot = _InPlace_QuickSort_Partition( _First, _Last, _Pred );
			sort( _First, _NewPivot, _Pred );
			sort( (_NewPivot + 1), _Last, _Pred );
		}
	}
}

#endif