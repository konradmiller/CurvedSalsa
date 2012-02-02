#include <cstdio>
#include <cstdlib>
#include <sys/time.h>

#include "random.h"
#include "cubehash.h"


Random* Random::getInstance ()
{
	static Random instance;
	return &instance;
}


inline Random::Random ()
{
#if defined(__linux__) || defined(__FreeBSD__)
	m_randfile = fopen( "/dev/urandom", "r" );

	if( ! m_randfile )
	{
		fprintf( stderr, "cannot open /dev/urandom - bailing out!\n" );
		exit( 1 );
	}
#else
	timeval tv;
	timezone tz;
	gettimeofday( &tv, &tz );
	srand( tv.tv_usec );
#endif
}


inline Random::~Random()
{
#if defined(__linux__) || defined(__FreeBSD__)
	fclose( m_randfile );
#endif
}


void Random::getBytes ( uint8_t *dest, int32_t num )
{
#if defined(__linux__) || defined(__FreeBSD__)
	int n;
	while( (n = fread(dest, 1, num, m_randfile)) > 0 )
		num -= n;
#else
	while( --num >= 0 )
		*(dest+num) = (uint8_t) rand();
#endif
}


inline Random::Random ( const Random & )
{ }
