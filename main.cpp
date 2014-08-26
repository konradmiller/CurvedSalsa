#include <iostream>
#include <cstdio>
#include <stdlib.h>

#include "getpass.h"
#include "program_options.h"
#include "curvedsalsa.h"

namespace po = boost::program_options;
bool quiet;

std::string prompt_password( const std::string &what, bool hidden = false )
{
	std::string pw;
	std::cerr << "Please enter " << what << ": ";
	if( hidden == true )
	{
		char *pass = getpass( "" );
		pw = pass;
		free( pass );
	}
	else
	{
		getline( std::cin, pw, '\n' );
	}

	return pw;
}

int main( int argc, char *argv[] )
{
	std::string inputfile  = "";
	std::string outputfile = "";
	std::string passphrase = "";
	std::string publicKey  = "";
	bool encrypt = false,
	     decrypt = false,
	     keygen  = false;

	// set up command line options and
	// check for help requests and usage errors
	po::variables_map var_map;

	try
	{
		init_program_options(argc, argv, &var_map);
	}
	catch( po::multiple_occurrences m )
	{
		std::cerr << "Error: Please use each options at most once."
			  << std::endl << std::endl;
		print_usage( argv[0] );
		exit( 2 );
	}
	catch( ... )
	{
		std::cerr << "Error: unknown error in command line options."
			  << std::endl << std::endl;
		print_usage( argv[0] );
		exit( 2 );
	}

	// no options -> print usage message
	if( argc == 1 )
	{
		print_usage( argv[0] );
		exit( 1 );
	}

	// check command line options
	if( var_map.count("test") > 0 )
	{
		exit( run_tests() );
	}

	encrypt = (var_map.count("encrypt")    > 0);
	decrypt = (var_map.count("decrypt")    > 0);
	keygen  = (var_map.count("create-key") > 0);

	// check that the user only request one out of decrypt/encrypt/keygen
	if( (int)decrypt + (int)encrypt + (int)keygen > 1 )
	{
		std::cerr << "You need to chose one of encrypt, decrypt and key-gen."
			  << std::endl << std::endl;
		print_usage( argv[0] );
		exit( 2 );
	}

	if (var_map.count("pass")   > 0) passphrase = var_map["pass"].as<std::string>();
	if (var_map.count("input")  > 0) inputfile  = var_map["input"].as<std::string>();
	if (var_map.count("output") > 0) outputfile = var_map["output"].as<std::string>();
	if (var_map.count("key")    > 0) publicKey  = var_map["key"].as<std::string>();
	quiet = (var_map.count("quiet") > 0);

	// here comes the program logic
	if( keygen )
	{
		if( passphrase == "" )
			passphrase = prompt_password( "passphrase", true );
	
		uint8_t publicKey[32];
		if( generate_key( publicKey, passphrase ) )
			std::cerr << "Public key: " << b64encode( publicKey );
		else
			std::cerr << "Failed to generate public key" << std::endl;
	}
	else if( encrypt )
	{
		if( inputfile == "" && publicKey == "" )
		{
			std::cerr << "Error: Public key needs to be given on "
				  << "commandline when encrypting from stdin" << std::endl;
			exit( 1 );
		}

		if( publicKey == "" )
			publicKey  = prompt_password( "public key" );

		encryptFile( outputfile, inputfile, passphrase, publicKey );
	}
	else if( decrypt )
	{
		if( inputfile == "" && passphrase == "" )
		{
			std::cerr << "Error: passphrase needs to be given on "
				  << "commandline when decrypting from stdin" << std::endl;
			exit( 1 );
		}

		if( passphrase == "" )
			passphrase = prompt_password( "passphrase", true );

		decryptFile( outputfile, inputfile, passphrase );
	}
	else
	{
		print_usage( argv[0] );
		exit( 0 );
	}

	return 0;
}



