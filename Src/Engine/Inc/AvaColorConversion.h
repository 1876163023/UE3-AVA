#ifndef __AVA_COLOR_CONVERSION__
#define __AVA_COLOR_CONVERSION__

//////////////////////////////////////////////////////////////////////////
//
//	AvaColorConversion.h	
//
//  [2006/11/30 ���½�]
//
//  ����ȯ�� �ʿ��� �Լ����� ��Ƶξ���.
//
//  ���� 
//	RGB <=> HSL ��ȯ ����
//	R16B16G16A16F�� ���� Float -> HalfFloat ��ȯ ����
//
//////////////////////////////////////////////////////////////////////////


/* ��ȯ�޼ҵ��� ����(���� ��� R,G,B,H,S,L)�� ��� [0.0f ... 1.0f]�� ������ ���´� */
class AvaColorConversion
{
public:

	static void RGBtoHSL(FLOAT& R, FLOAT& G, FLOAT& B)
	{
		float h,s,l;

		float	maxRGB = Max(R,Max(G,B)),
				minRGB = Min(R,Min(G,B));

		l = (maxRGB + minRGB) / 2.0f;
		if( maxRGB == minRGB )
		{
			s = 0.0f;
			h = -1.0f;	//HSL_UNDEFINED
		}
		else
		{
			s = (l <= 0.5f) ? (maxRGB-minRGB) / (maxRGB + minRGB) : (maxRGB - minRGB) /(2.0f - maxRGB - minRGB) ;

			float delta = maxRGB - minRGB;
			if( delta == 0.0f )
				delta = 1.0f;

			if( R == maxRGB )
				h = (G - B) / delta;
			else if ( G == maxRGB )
				h = 2.0f + (B - R) / delta;
			else
				h = 4.0f + (R - G) / delta;

			h /= 6.0f;
			if( h < 0.0 )
				h += 1.0f;
		}
		R = h;G = s; B = l;
	}

	static void HSLtoRGB(FLOAT &H, FLOAT &S, FLOAT &L)
	{
		float R,G,B;

		if( S == 0 )
			R = G = B = L;
		else
		{
			float m1, m2;
			m2 = L <= 0.5f ? L * (1.0f + S) : L + S - L * S;
			m1 = 2.0f * L - m2;

			R = GetHSLValue(m1,m2,H * 6.0f + 2.0f);
			G = GetHSLValue(m1,m2,H * 6.0f);
			B = GetHSLValue(m1,m2,H * 6.0f - 2.0f);	
		}

		H = R; S = G; L = B;
	}

	static FLOAT GetHSLValue(FLOAT n1, FLOAT n2, FLOAT hue)
	{
		hue = hue > 6.0f ? hue - 6.0f : (hue < 0.0f ? hue + 6.0f : hue);
		return hue < 1.0f ? n1 + ( n2 - n1 ) * hue : ( hue < 3.0f ? n2 : (hue < 4.0f ? n1 + (n2 - n1) * (4.0f - hue) : n1 ) );
	}

	/**	@brief Float(32bit Float)�κ��� HalfFloat(16bit Float)�� ���� �Լ�
	 *	
	 *	Ư������(�μ� f�� 0.0f, 1.0f ������ ���� ����)������ ����ϹǷ� 
	 *	32bit�� ����(exponent)�� 16bit�� �ٲٴ� ���� �Ұ����ϴ��� ����ó���� ���� ����.
	 */
	static USHORT GetHalfFloat( FLOAT f )
	{
		UINT sign = (*((UINT*)&f) & 0x80000000) >> (31);
		UINT exponent = (*((UINT*)&f) & 0x7f800000) >> (23);
		UINT mantissa =  *((UINT*)&f) & 0x007fffff;

		sign = sign << 15;
		exponent = (Clamp((INT)exponent - 127,-15,16) + 15) << 10;
		mantissa = mantissa >> (23-10);

		return ( sign | exponent  | mantissa );
	}
};


#endif