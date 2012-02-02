#ifndef CURVEDSALSA_H
#define CURVEDSALSA_H

#include <cstdio>
#include <stdlib.h>
#include <inttypes.h>

bool
generate_key ( uint8_t out[32], const std::string &passphrase,
	       const std::string &hispublic = std::string("") );

std::string
b64encode ( uint8_t in[32] );

void 
b64decode ( uint8_t out[32], const std::string in );

bool
encryptFile ( const std::string &outfile, const std::string &infile,
	      const std::string &passphrase, const std::string &publicKey,
	      bool quiet = false );

bool
decryptFile (  const std::string &outfile, const std::string &infile,
	       const std::string &passphrase, bool quiet = false );

int
run_tests ();

#endif //CURVEDSALSA_H
