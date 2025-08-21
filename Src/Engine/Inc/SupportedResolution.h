#ifndef __SUPPORTEDRESOLUTION_H__
#define __SUPPORTEDRESOLUTION_H__

struct FScreenResolution
{
	INT Width, Height;

	static INT GCD(INT a, INT b)
	{		
		for (;a>0 && b>0;)
		{
			if (b > a)
			{
				appMemswap( &a, &b, sizeof(INT) );
			} 
			else if (b == 0)
			{			
				return a;
			} 
			else
			{
				INT r = a%b;
				a = b;
				b = r;
			}			
		}

		return a;
	}

	void Factor( const FScreenResolution& Raw )
	{
		INT gcd = GCD( Raw.Width, Raw.Height );

		Width = Raw.Width / gcd;
		Height = Raw.Height / gcd;
	}	

	UBOOL operator == ( const FScreenResolution& A ) const
	{
		return Width == A.Width && Height == A.Height;
	}

	friend DWORD GetTypeHash(const FScreenResolution& Key )
	{
		return Key.Width + Key.Height * 7919/* Magic prime number */;
	}
};

void GetSupportedResolutions( TMultiMap< FScreenResolution, FScreenResolution >& Result );

#endif
