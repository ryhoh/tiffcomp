#ifndef TIFFCOMP_H
#define TIFFCOMP_H

#include <iostream>
#include "../src/compositeProcess.h"

ryhoh_tiff::CompositeProcess *get_opt(int arg_c, char *argv[]);
void print_help();
void print_version();

#endif
