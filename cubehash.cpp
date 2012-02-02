// this is basically djb's reference implementation from
// http://cubehash.cr.yp.to/software.html wrapped in a class

#include "cubehash.h"

#define CUBEHASH_ROUNDS 8
#define ROTATE(a,b) (((a) << (b)) | ((a) >> (32 - b)))

CubeHash::CubeHash ()
	: m_hashByteLen( 64 )
{
	init();
}


CubeHash::CubeHash ( int outputByteLen )
	: m_hashByteLen( outputByteLen )
{
	init();
}


void CubeHash::init ()
{
#ifdef __SSE2__

	for( int i = 0; i < 8; ++i )
		m_x[i] = _mm_set_epi32( 0, 0, 0, 0 );

	m_x[0] = _mm_set_epi32( 0, CUBEHASH_ROUNDS, 1, m_hashByteLen );

#else

	for( int i = 0; i < 32; ++i )
		m_x[i] = 0;

	m_x[0] = m_hashByteLen;
	m_x[1] = 1;
	m_x[2] = CUBEHASH_ROUNDS;

#endif

	transform( 10 * CUBEHASH_ROUNDS );
}


bool CubeHash::setByteLength ( int byteLength )
{
	if( (byteLength == 0) || (byteLength > 64) )
		return false;

	m_hashByteLen = byteLength;
	init();

	return true;
}


void CubeHash::transform ( int r )
{
#ifdef __SSE2__

	__m128i x0, x1, x2, x3, x4, x5, x6, x7;
	__m128i y0, y1, y2, y3;

	x0 = m_x[0];   x1 = m_x[1];   x2 = m_x[2];   x3 = m_x[3];
	x4 = m_x[4];   x5 = m_x[5];   x6 = m_x[6];   x7 = m_x[7];

	for( ; r > 0; --r )
	{
		x4 = _mm_add_epi32( x0, x4 );
		x5 = _mm_add_epi32( x1, x5 );
		x6 = _mm_add_epi32( x2, x6 );
		x7 = _mm_add_epi32( x3, x7 );

		y0 = x2;
		y1 = x3;
		y2 = x0;
		y3 = x1;

		x0 = _mm_xor_si128( _mm_slli_epi32(y0,7), _mm_srli_epi32(y0,25) );
		x1 = _mm_xor_si128( _mm_slli_epi32(y1,7), _mm_srli_epi32(y1,25) );
		x2 = _mm_xor_si128( _mm_slli_epi32(y2,7), _mm_srli_epi32(y2,25) );
		x3 = _mm_xor_si128( _mm_slli_epi32(y3,7), _mm_srli_epi32(y3,25) );

		x0 = _mm_xor_si128( x0, x4 );
		x1 = _mm_xor_si128( x1, x5 );
		x2 = _mm_xor_si128( x2, x6 );
		x3 = _mm_xor_si128( x3, x7 );

		x4 = _mm_shuffle_epi32( x4, 0x4e );
		x5 = _mm_shuffle_epi32( x5, 0x4e );
		x6 = _mm_shuffle_epi32( x6, 0x4e );
		x7 = _mm_shuffle_epi32( x7, 0x4e );

		x4 = _mm_add_epi32( x0, x4 );
		x5 = _mm_add_epi32( x1, x5 );
		x6 = _mm_add_epi32( x2, x6 );
		x7 = _mm_add_epi32( x3, x7 );

		y0 = x1;
		y1 = x0;
		y2 = x3;
		y3 = x2;

		x0 = _mm_xor_si128( _mm_slli_epi32(y0,11), _mm_srli_epi32(y0,21) );
		x1 = _mm_xor_si128( _mm_slli_epi32(y1,11), _mm_srli_epi32(y1,21) );
		x2 = _mm_xor_si128( _mm_slli_epi32(y2,11), _mm_srli_epi32(y2,21) );
		x3 = _mm_xor_si128( _mm_slli_epi32(y3,11), _mm_srli_epi32(y3,21) );

		x0 = _mm_xor_si128( x0, x4 );
		x1 = _mm_xor_si128( x1, x5 );
		x2 = _mm_xor_si128( x2, x6 );
		x3 = _mm_xor_si128( x3, x7 );

		x4 = _mm_shuffle_epi32( x4, 0xb1 );
		x5 = _mm_shuffle_epi32( x5, 0xb1 );
		x6 = _mm_shuffle_epi32( x6, 0xb1 );
		x7 = _mm_shuffle_epi32( x7, 0xb1 );
	}

	m_x[0] = x0;   m_x[1] = x1;   m_x[2] = x2;   m_x[3] = x3;
	m_x[4] = x4;   m_x[5] = x5;   m_x[6] = x6;   m_x[7] = x7;

#else // non SSE2

	int i;
	uint32_t y[16];

	for( ; r > 0; --r )
	{
		for( i = 0; i < 16; ++i )  m_x[i + 16] += m_x[i];
		for( i = 0; i < 16; ++i )  y[i ^ 8] = m_x[i];
		for( i = 0; i < 16; ++i )  m_x[i] = ROTATE(y[i],7);
		for( i = 0; i < 16; ++i )  m_x[i] ^= m_x[i + 16];
		for( i = 0; i < 16; ++i )  y[i ^ 2] = m_x[i + 16];
		for( i = 0; i < 16; ++i )  m_x[i + 16] = y[i];
		for( i = 0; i < 16; ++i )  m_x[i + 16] += m_x[i];
		for( i = 0; i < 16; ++i )  y[i ^ 4] = m_x[i];
		for( i = 0; i < 16; ++i )  m_x[i] = ROTATE(y[i],11);
		for( i = 0; i < 16; ++i )  m_x[i] ^= m_x[i + 16];
		for( i = 0; i < 16; ++i )  y[i ^ 1] = m_x[i + 16];
		for( i = 0; i < 16; ++i )  m_x[i + 16] = y[i];
	}

#endif
}


void CubeHash::Update ( const uint8_t *data, int dataByteLen )
{

	while( dataByteLen >= 1 )
	{
#ifdef __SSE2__
		m_x[0] = _mm_xor_si128( m_x[0],_mm_set_epi32(0,0,0, (int) (unsigned int) *data) );
#else
		m_x[0] ^= (uint32_t) *data;
#endif
		++data;
		--dataByteLen;

		transform( CUBEHASH_ROUNDS );
	}
}


void CubeHash::Final ( uint8_t *hashval )
{
	int i;

#ifdef __SSE2__

	((uint8_t*) m_x)[0] ^= (uint32_t) 128;
	transform( CUBEHASH_ROUNDS );
	m_x[7] = _mm_xor_si128( m_x[7], _mm_set_epi32(1,0,0,0) );

	transform( 10 * CUBEHASH_ROUNDS );

	for( i = 0; i < m_hashByteLen; ++i )
		hashval[i] = ((uint8_t*)m_x)[i];

#else // non SSE2

	m_x[0] ^= (uint32_t) 128;
	transform( CUBEHASH_ROUNDS );
	m_x[31] ^= 1;

	transform( 10 * CUBEHASH_ROUNDS );

	for( i = 0; i < m_hashByteLen; ++i )
		hashval[i] = m_x[i / 4] >> (8 * (i % 4));

#endif
}


#undef CUBEHASH_ROUNDS
#undef ROTATE
