#ifndef SALSA_H
#define SALSA_H

#include <inttypes.h>

class Salsa
{

public:
	static const int IV_LENGTH  = 8;
	static const int KEY_LENGTH = 32;

	Salsa ( const uint8_t *key, const uint8_t iv[IV_LENGTH] = 0 );
	void init ( const uint8_t *key, const uint8_t iv[IV_LENGTH] = 0 );
	void keySetup ( const uint8_t *key );
	void ivSetup ( const uint8_t iv[IV_LENGTH] );
	void encrypt ( uint8_t *m, const uint32_t bytes ); 
	void decrypt ( uint8_t *m, const uint32_t bytes ); 

private:
	void wordToByte ( uint32_t input[16] );

	uint32_t m_state[16];
	uint8_t  m_output[64];
};

#endif
