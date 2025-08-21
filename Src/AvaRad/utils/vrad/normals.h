#pragma once

int FindOrCreateNormal( const Vector& n );

struct NVector : Vector
{
	NVector()
		: index( -1 )
	{
	}

	NVector( const Vector& v )
		: Vector( v ), index( -1 )
	{
	}

	NVector( const NVector& n )
		: Vector( n ), index( n.index )
	{
	}

	int getIndex() const
	{
		if (index < 0) 
			index = FindOrCreateNormal( *this );

		return index;
	}

	mutable int index;
};

