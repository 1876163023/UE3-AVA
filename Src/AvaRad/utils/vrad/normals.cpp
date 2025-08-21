#include "vrad.h"
#include <map>

using namespace std;

namespace std
{
	template <>
	struct less<Vector> : binary_function<Vector,Vector,bool>
	{
		bool operator () ( const Vector& a, const Vector& b ) const
		{
			if (a.x < b.x) return true;
			if (a.x > b.x) return false;

			if (a.y < b.y) return true;
			if (a.y > b.y) return false;

			if (a.z < b.z) return true;
			return false;
		}
	};
}

static map<Vector,int> normals;

int availNormalIndex = 0;

int FindOrCreateNormal( const Vector& n )
{
	map<Vector, int>::const_iterator it = normals.find( n );

	if (it != normals.end())
		return it->second;

	normals.insert( make_pair( n, availNormalIndex++ ) );

	return availNormalIndex - 1;
}