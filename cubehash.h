// this is basically djb's reference implementation from
// http://cubehash.cr.yp.to/software.html just wrapped as a class

#ifndef CUBEHASH_H
#define CUBEHASH_H

#include <inttypes.h>

#ifdef __SSE2__
	#include <emmintrin.h>
#endif

class CubeHash
{
public:
	CubeHash ();
	CubeHash ( int outputByteLen );
	void Update ( const uint8_t *data, int dataByteLen );
	void Final ( uint8_t *hashval );
	bool setByteLength ( int byteLength );
	void init ();

private:
	void transform ( int r );
	int m_hashByteLen;

#ifdef __SSE2__
	__m128i m_x[8];
#else
	uint32_t m_x[32];
#endif

};

#endif // CUBEHASH_H
