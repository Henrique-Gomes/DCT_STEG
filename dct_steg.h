#pragma once
#include <string>
#include <stdio.h>

using namespace std;

void write_steg_message(FILE * infile, FILE * outfile, string msg);

string read_steg_message(FILE * infile);