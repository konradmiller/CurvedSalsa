#ifndef RANDOM_H
#define RANDOM_H

#include <inttypes.h>
#include <cstdio>

class Random
{
public:
	inline ~Random();
	static Random* getInstance ();

	void getBytes ( uint8_t *dest, int32_t num );

private:
	inline Random ();
	inline Random ( const Random & );

#if defined(__linux__) || defined(__FreeBSD__)
	FILE *m_randfile;
#endif
};

#endif
