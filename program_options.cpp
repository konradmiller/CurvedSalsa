#include <iostream>
#include <vector>
#include <string>
#include <cstdio>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "program_options.h"

// file local variables
static po::options_description opt_desc("Options"),
				opt_desc_hidden;
static po::positional_options_description pos_opts;
static std::vector<std::string> unknown_options;


bool init_program_options( int argc, char** argv, po::variables_map *var_map )
{
	// define named options
	opt_desc.add_options()
		("encrypt,e", "Encrypt")
		("decrypt,d", "Decrypt")
		("input,i", po::value<std::string>(), "Use <arg> as input. Will use stdin if omitted.")
		("output,o", po::value<std::string>(), "Use <arg> as output. Will use stdout if omitted.")
		("create-key,c", "Create new private/public key-pair for Diffie-Hellman key exchange.")
		("pass,p", po::value<std::string>(), "Take private key from command line.")
		("key,k", po::value<std::string>(), "Take public key from command line.")
		("quiet,q", "Suppress unneeded output.")
		("help,h", "print this help message")
		;

	// define hidden options, i.e. options that don't appear in the usage message
	opt_desc_hidden.add_options()
		("test,t", "")
		("catch_all_positional", po::value<std::vector<std::string> >(), "")  // catch surplus positionals
		;

	// define positional options, i.e. options that don't need to be
	// specified using the options name:  ./my_program a.jpg
	pos_opts.add("catch_all_positional", -1); // catch surplus positionals

	po::options_description opt_all;
	opt_all.add(opt_desc).add(opt_desc_hidden);

	// parse options
	// if unknown options are detected, return a vector of unrecognized options
	// otherwise store options and parameters in var_map
	po::parsed_options parsed = po::command_line_parser(argc, argv).
					options(opt_all).positional(pos_opts).allow_unregistered().run();
	unknown_options = collect_unrecognized(parsed.options, po::exclude_positional);

	// display error and usage message if unknown options were given
	if (unknown_options.size() > 0) {
		std::cout << "Unknown option: " << unknown_options[0] << std::endl << std::endl;
		print_usage(argv[0]);
		exit(1);
	}

	// store parameter in var_map
	po::store(parsed, *var_map);
	po::notify(*var_map);

	// print usage message if --help was specified on the command line
	if (var_map->count("help")) {
		print_usage(argv[0]);
		exit(0);
	}

	return true;
}

/*
 * display a usage message to the user
 */
void print_usage(const char *program_name)
{
	printf( "%s is a  program capable of encrypting/decrypting files with\n"
		"Salsa20 via a shared key calculated from a Diffie-Hellman-Key-Exchange\n"
		"using djb's curve25519.\n\n", program_name);

	// output option descriptions
	std::cout << opt_desc << std::endl;

	printf ( "Example:\n"
		" # curvedsalsa -c -p foo\n"
		" Public key: 0n3tOdXQscOUwA7rCKvWJgA+D3JsgztMmevi9ujLdzw=\n"
		"\n"
		" # curvedsalsa -c -p bar\n"
		" Public key: U2+sw6djHDik8zFCRWT9Jj/uMUOGsGmqbh26LfMsU1Q=\n"
		"\n"
		" # curvedsalsa -e -i clear.txt -o crypt.txt -p foo \\\n"
		"               -k U2+sw6djHDik8zFCRWT9Jj/uMUOGsGmqbh26LfMsU1Q=\n"
		"\n"
		" # curvedsalsa -d -i crypt.txt -o clearagain.txt -p bar\n"
		" Decrypted using public key: 0n3tOdXQscOUwA7rCKvWJgA+D3JsgztMmevi9ujLdzw=\n"
		"\n"
		"\n"
		"Example2:\n"
		" # curvedsalsa -e -i clear.txt -o crypt.txt \\\n"
		"               -k U2+sw6djHDik8zFCRWT9Jj/uMUOGsGmqbh26LfMsU1Q=\n"
		" Public Key: deMIMWRbJy8M1+BH64Q1C0QeW7CTahjPgdx5pGvlZmA=\n"
		"\n"
		" # curvedsalsa -d -i crypt.txt -o clearagain.txt -p bar\n"
		" Decrypted using public key: deMIMWRbJy8M1+BH64Q1C0QeW7CTahjPgdx5pGvlZmA=\n");
}
