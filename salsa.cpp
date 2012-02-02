#include <cstring>
#include "ecrypt-portable.h"
#include "salsa.h"

#define XOR(v,w) ((v) ^ (w))
#define PLUS(v,w) (U32V((v) + (w)))
#define PLUSONE(v) (PLUS((v),1))

#define QUARTERROUND(a,b,c,d) \
	x[a] = PLUS(x[a],x[b]); x[d] = ROTL32(XOR(x[d],x[a]),16); \
	x[c] = PLUS(x[c],x[d]); x[b] = ROTL32(XOR(x[b],x[c]),12); \
	x[a] = PLUS(x[a],x[b]); x[d] = ROTL32(XOR(x[d],x[a]), 8); \
	x[c] = PLUS(x[c],x[d]); x[b] = ROTL32(XOR(x[b],x[c]), 7);

Salsa::Salsa ( const uint8_t *key, const uint8_t iv[IV_LENGTH] )
{
	init( key, iv );
}

void Salsa::init ( const uint8_t *key, const uint8_t iv[IV_LENGTH] )
{
	keySetup( key );
	ivSetup( iv );
}


void Salsa::keySetup ( const uint8_t *key )
{
	const char *constants = "expand 32-byte k";

	m_state[4]  = U8TO32_LITTLE( key + 0  );
	m_state[5]  = U8TO32_LITTLE( key + 4  );
	m_state[6]  = U8TO32_LITTLE( key + 8  );
	m_state[7]  = U8TO32_LITTLE( key + 12 );

	key += 16;

	m_state[8]  = U8TO32_LITTLE( key + 0  );
	m_state[9]  = U8TO32_LITTLE( key + 4  );
	m_state[10] = U8TO32_LITTLE( key + 8  );
	m_state[11] = U8TO32_LITTLE( key + 12 );
	m_state[0]  = U8TO32_LITTLE( constants + 0  );
	m_state[1]  = U8TO32_LITTLE( constants + 4  );
	m_state[2]  = U8TO32_LITTLE( constants + 8  );
	m_state[3]  = U8TO32_LITTLE( constants + 12 );
}


void Salsa::ivSetup ( const uint8_t iv[IV_LENGTH] )
{
	if( iv == 0 )
	{
		uint8_t nullIv[IV_LENGTH];
		memset( nullIv, 0, sizeof nullIv );

		m_state[12] = 0;
		m_state[13] = 0;
		m_state[14] = U8TO32_LITTLE( nullIv + 0 );
		m_state[15] = U8TO32_LITTLE( nullIv + 4 );
	}
	else
	{
		m_state[12] = 0;
		m_state[13] = 0;
		m_state[14] = U8TO32_LITTLE( iv + 0 );
		m_state[15] = U8TO32_LITTLE( iv + 4 );
	}
}


void Salsa::wordToByte ( uint32_t input[16] )
{
	uint32_t x[16];
	int i;

	for( i = 0; i < 16; ++i )
		x[i] = input[i];

	for( i = 8; i > 0; i -= 2 )
	{
		QUARTERROUND( 0, 4,  8, 12 )
		QUARTERROUND( 1, 5,  9, 13 )
		QUARTERROUND( 2, 6, 10, 14 )
		QUARTERROUND( 3, 7, 11, 15 )
		QUARTERROUND( 0, 5, 10, 15 )
		QUARTERROUND( 1, 6, 11, 12 )
		QUARTERROUND( 2, 7,  8, 13 )
		QUARTERROUND( 3, 4,  9, 14 )
	}

	for( i = 0; i < 16; ++i )
		x[i] = PLUS( x[i], input[i] );

	for( i = 0; i < 16; ++i )
		U32TO8_LITTLE( m_output + 4*i, x[i] );
}


void Salsa::encrypt ( uint8_t *m, uint32_t bytes )
{
	uint32_t i;

	if ( ! bytes )
		return;

	for( ;; )
	{
		wordToByte( m_state );
		m_state[12] = PLUSONE( m_state[12] );

		if( ! m_state[12] )
			m_state[13] = PLUSONE( m_state[13] );

		if( bytes <= 64 )
		{
			for( i = 0; i < bytes; ++i )
				m[i] ^= m_output[i];

			return;
		}

		for( i = 0; i < 64; ++i )
			m[i] ^= m_output[i];

		bytes -= 64;
		m += 64;
	}
}

void Salsa::decrypt ( uint8_t *m, const uint32_t bytes )
{
	encrypt( m, bytes );
}
