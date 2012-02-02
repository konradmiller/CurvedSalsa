#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// for stat
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "cubehash.h"
#include "salsa.h"
#include "random.h"
#include "curvedsalsa.h"
#include "pstdint.h"

// those 4 are used to propagate the state to other threads (i.e. progress bar in gui)
long long progress        = 0,
	  progress_max    = 1;
bool 	  progress_cancel = false;

const long long CHUNK = 4096;


extern "C"
{
	extern void curve25519_donna( uint8_t *output, const uint8_t *secret, const uint8_t *bp );
}


static void
die ( const char* msg )
{
	std::cerr << msg << std::endl;
	exit( 1 );
}


// this here _only_ works for 32 byte inputs!
std::string
b64encode ( uint8_t in[32] )
{
	std::string out;
	static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";	
	int sixbit = 5, curnum = 0;

	for( int byte=0; byte<32; ++byte )
	{
		for( int bit=7; bit>=0; --bit )
		{
			if( sixbit == -1 )
			{
				out += (curnum > 63) ? '=' : encoding[curnum];
				curnum = 0;
				sixbit = 5;
			}

			curnum += ((in[byte] >> bit) & 1) ? (1<<sixbit) : 0;
			sixbit--;
		}
	}

	out += (curnum > 63) ? '=' : encoding[curnum];
	out += "=\n"; // one '=' for every 0 fillbit at end
	return out;
}


// this here _only_ works for 32 byte outputs!
void
b64decode ( uint8_t out[32], const std::string in )
{
	static const char decoding[] =
	{
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
		-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
		-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
	};

	memset( out, 0, 32 );

	std::string::const_iterator ch = in.begin();
	// we have exactly 11 4 byte blocks for 32 byte output
	for( int i=0; i<11; ++i )
	{
		out[i*3    ] |= (decoding[(int)*ch++] << 2) & ~0x3;
		out[i*3    ] |= (decoding[(int)*ch]   >> 4) &  0x3;
		out[i*3 + 1] |= (decoding[(int)*ch++] << 4) & ~0xF;
		out[i*3 + 1] |= (decoding[(int)*ch]   >> 2) &  0xF;

		if( i != 10 )
		{
			out[i*3 + 2] |= (decoding[(int)*ch++] << 6) & ~0x3F;
			out[i*3 + 2] |= (decoding[(int)*ch++]) & 0x3F; 
		}
	}
}


// generates a public key to a passphrase if hispublic is not given
// generates a shared secret if other partys public key is supplied via hispublic
bool
generate_key ( uint8_t out[32], const std::string &passphrase, const std::string &hispublic )
{
	const uint8_t basepoint[32] = { 9 };

	uint8_t  digest[32];
	CubeHash h( 32 );
	h.Update( (uint8_t*)passphrase.c_str(), passphrase.length() );
	h.Final( digest );

	digest[0]  &= 248;
	digest[31] &= 127;
	digest[31] |=  64;

	if( hispublic == "" )	// generate public key
	{
		curve25519_donna( out, digest, basepoint );
	}
	else 			// generate shared secret
	{
		uint8_t hisPublicKey[64];
		memset( hisPublicKey, 0, sizeof hisPublicKey );
		b64decode( hisPublicKey, hispublic );

		curve25519_donna( out, digest, hisPublicKey );
	}

	return true;
}


long long
getFileSize ( int fd )
{
	struct stat s;

	if( fstat(fd, &s) == -1 )
	{
		perror( "fstat" );
		return -1;
	}
	
	return s.st_size;
}


bool
encryptFile ( const std::string &outfile, const std::string &infile,
	      const std::string &passphrase, const std::string &publicKey,
	      bool quiet )
{
	std::string secretPassphrase;
	uint8_t iv[Salsa::IV_LENGTH],
		buf[CHUNK];
	long long filesize = 0;
	int n;

	if( passphrase == "" )
	{
		uint8_t randomPw[32];
		Random::getInstance()->getBytes( randomPw, 32 );
		secretPassphrase = b64encode( randomPw );
	}
	else
	{
		secretPassphrase = passphrase;
	}

	uint8_t sharedKey[32];
	if( ! generate_key(sharedKey, secretPassphrase, publicKey) )
		die( "Failed to generate shared secret" );
	
	uint8_t myPublicKey[32];
	if( ! generate_key(myPublicKey, secretPassphrase) )
		die( "Error: Failed to generate public key" );

	if( passphrase == "" && quiet == false ) // password is generated
		std::cerr << "Public Key for Verification is: " << b64encode( myPublicKey );


	// check if infile exists:
	FILE *inFd = NULL;
	if( infile != "" )
	{
		if( (inFd = fopen( infile.c_str(), "r" )) == NULL )
			die( "Cannot open input file" );
		else
			filesize = getFileSize( fileno(inFd) );
	}
	else
	{
		if( (inFd = fdopen( 0, "r" )) == NULL )
			die( "Cannot open stdin" );
	}

	// open outfile or stdout
	FILE *outFd = NULL;
	if( outfile == "" )
		outFd = fdopen( 1, "w" ); // stdout
	else
		outFd = fopen( outfile.c_str(), "w" );


	// set random IV
	Random::getInstance()->getBytes( iv, Salsa::IV_LENGTH );

	// write our random iv to file	
	int len = Salsa::IV_LENGTH;
	while( (n = fwrite(iv, sizeof(uint8_t), len, outFd)) > 0 )
		len -= n;

	if( len > 0 )
		die( "cannot write IV" );


	// write our public key to file
	len = 32;
	while( (n = fwrite(myPublicKey, sizeof(uint8_t), len, outFd)) > 0 )
		len -= n;

	if( len > 0 )
		die( "cannot write my public key" );


	// everything else we write from here on will be encrypted
	Salsa s( sharedKey, iv );


	// prepend a CubeHash digest of the iv to make a
	// plausability check of password possible
	uint8_t ivdigest[32];
	CubeHash h( sizeof ivdigest );
	h.Update( (uint8_t*)iv, Salsa::IV_LENGTH );
	h.Final( ivdigest );
	s.encrypt( ivdigest, sizeof ivdigest );

	len = sizeof ivdigest;
	while( (n = fwrite(ivdigest, sizeof(uint8_t), len, outFd)) > 0 )
		len -= n;

	if( len > 0 )
		die( "cannot write IV-Hash" );


	// now encrypt the actual file
	if( quiet || (infile == "") || (outfile == "") )
	{
		while( (n = fread(buf, sizeof(uint8_t), sizeof buf, inFd)) > 0 )
		{
			s.encrypt( buf, n );
			len = n;
			while( (n = fwrite(buf, sizeof(uint8_t), len, outFd)) > 0 )
				len -= n;

			if( len > 0 )
				die( "cannot write data" );
		}
	}
	else
	{
		long long kbsize = filesize / 1024;
		progress_max = filesize;
		progress     = 0;
		
		fprintf( stderr, "\rEncrypted %8.0lld/%lld kB", 0ll, kbsize );

		while( (n = fread(buf, sizeof(uint8_t), sizeof buf, inFd)) > 0 )
		{
			s.encrypt( buf, n );
			progress += n;

			len = n;
			while( (n = fwrite(buf, sizeof(uint8_t), len, outFd)) > 0 )
				len -= n;

			if( len > 0 )
				die( "cannot write data" );

			fprintf( stderr, "\rEncrypted %8.0lld/%lld kB", progress/1024, kbsize );

			if( progress_cancel )
				break;
		}

		if( progress_cancel )
		{
			progress_cancel = false;
			return false;
		}

		progress = progress_max;

		if( progress_max < 1024 )
			fprintf( stderr, "\rEncrypted %8.0lld/%lld Bytes\n", filesize, filesize );
		else
			fprintf( stderr, "\rEncrypted %8.0lld/%lld kB\n", kbsize, kbsize );
	}
	
	fclose( inFd );
	fclose( outFd );

	return true;
}


bool decryptFile (  const std::string &outfile, const std::string &infile, const std::string &passphrase, bool quiet )
{
	long long filesize = 0;
	uint8_t iv[Salsa::IV_LENGTH],
		publicKey[32],
		sharedKey[32],
		buf[CHUNK];
	int n, len;

	memset( iv, 0, sizeof iv );

	FILE *inFd = NULL;
	if( infile != "" )
	{
		if( (inFd = fopen( infile.c_str(), "r" )) == NULL )
			die( "Cannot open input file" );
		else
			filesize = getFileSize( fileno(inFd) );
	}
	else
	{
		if( (inFd = fdopen( 0, "r" )) == NULL )
			die( "Cannot open stdin" );
	}

	// read iv
	len = Salsa::IV_LENGTH;
	while( (n = fread(iv, sizeof(uint8_t), len, inFd)) > 0 )
		len -= n;

	if( len > 0 )
		die( "cannot read IV" );


	// read public key
	len = 32;
	while( (n = fread(publicKey, sizeof(uint8_t), len, inFd)) > 0 )
		len -= n;

	if( len > 0 )
		die( "cannot read public key" );

	if( ! generate_key( sharedKey, passphrase, b64encode(publicKey) ) )
		die( "Failed to generate shared secret" );

	Salsa s( sharedKey, iv );

	// open outfile or stdout
	FILE *outFd = NULL;
	if( outfile == "" )
		outFd = fdopen( 1, "w" ); // stdout
	else
		outFd = fopen( outfile.c_str(), "w" );


	// check the prepended CubeHash of the iv
	// if incorrect -> assume that pw was incorrect
	uint8_t ivDigest[32];
	uint8_t calculatedIvDigest[32];

	CubeHash h( 32 );
	h.Update( (uint8_t*)iv, Salsa::IV_LENGTH );
	h.Final( calculatedIvDigest );

	// read hashed iv
	len = 32;
	while( (n = fread(ivDigest, sizeof(uint8_t), len, inFd)) > 0 )
		len -= n;

	if( len > 0 )
		die( "cannot read hased IV" );

	s.decrypt( ivDigest, sizeof ivDigest );

	if( memcmp(ivDigest, calculatedIvDigest, sizeof ivDigest) != 0 )
	{
		std::cerr << "Wrong password!" << std::endl;
		progress_max = -1;
		return false;
	}


	if( quiet || (infile == "") || (outfile == "") )
	{
		while( (n = fread(buf, sizeof(uint8_t), sizeof buf, inFd)) > 0 )
		{
			s.decrypt( buf, n );
			len = n;
			while( (n = fwrite(buf, sizeof(uint8_t), len, outFd)) > 0 )
				len -= n;

			if( len > 0 )
				die( "cannot write data" );
		}
	}
	else
	{
		long long kbsize = filesize / 1024;
		progress_max = filesize;
		progress     = 0;
		
		fprintf( stderr, "\rDecrypted %8.0lld/%lld kB", 0ll, kbsize );

		while( (n = fread(buf, sizeof(uint8_t), sizeof buf, inFd)) > 0 )
		{
			s.encrypt( buf, n );
			progress += n;

			len = n;
			while( (n = fwrite(buf, sizeof(uint8_t), len, outFd)) > 0 )
				len -= n;

			if( len > 0 )
				die( "cannot write data" );

			fprintf( stderr, "\rDecrypted %8.0lld/%lld kB", progress/1024, kbsize );

			if( progress_cancel )
				break;
		}

		if( progress_cancel )
		{
			progress_cancel = false;
			return false;
		}

		progress = progress_max;

		if( progress_max < 1024 )
			fprintf( stderr, "\rDecrypted %8.0lld/%lld Bytes\n", filesize, filesize );
		else
			fprintf( stderr, "\rDecrypted %8.0lld/%lld kB\n", kbsize, kbsize );
	}

	if( quiet == false )
		std::cerr << "Signed with public key: " << b64encode( publicKey );

	fclose( inFd );
	fclose( outFd );

	return true;
}


int run_tests ()
{
	std::string passphrase1 = "test1",
		    passphrase2 = "test2";

	std::cout << " +++ passphrase 1: " << passphrase1 << std::endl
		  << " +++ passphrase 2: " << passphrase2 << std::endl;

	uint8_t secretKey1[32],
		secretKey2[32];

	CubeHash h( 32 );
	h.Update( (uint8_t*)passphrase1.c_str(), passphrase1.length() );
	h.Final( secretKey1 );

	h.init();
	h.Update( (uint8_t*)passphrase2.c_str(), passphrase2.length() );
	h.Final( secretKey2 );

	std::cout << " +++ secretKey 1: " << b64encode( secretKey1 )
		  << " +++ secretKey 2: " << b64encode( secretKey2 );

	uint8_t publicKey1[32],
		publicKey2[32];
	generate_key( publicKey1, passphrase1 );
	generate_key( publicKey2, passphrase2 );
	std::cout << " +++ publicKey 1: " << b64encode( publicKey1 )
		  << " +++ publicKey 2: " << b64encode( publicKey2 );

	uint8_t sharedSecret1[32],
		sharedSecret2[32];
	generate_key( sharedSecret1, passphrase1, b64encode(publicKey2) );
	generate_key( sharedSecret2, passphrase2, b64encode(publicKey1) );
	std::cout << " +++ shared secret1: " << b64encode( sharedSecret1 )
		  << " +++ shared secret2: " << b64encode( sharedSecret2 );

	std::cout << " --------> ";
	if( b64encode(sharedSecret1) == b64encode(sharedSecret2) )
	{
		std::cout << "PASSED" << std::endl;
		return 0;
	}
	else
	{
		std::cout << "FAILED" << std::endl;
		return 1;
	}
}
