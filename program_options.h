#ifndef _PROGRAM_OPTIONS_H_
#define _PROGRAM_OPTIONS_H_

#include "boost/program_options.hpp"

bool init_program_options(int argc, char** argv, boost::program_options::variables_map *var_map);
void print_usage(const char *program_name = "curvedsalsa");

#endif /* _PROGRAM_OPTIONS_H_ */
