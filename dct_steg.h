#pragma once
#include <string>
#include <stdio.h>
#include <utility>

using namespace std;

void write_steg_message(FILE * infile, FILE * outfile, string msg);

string read_steg_message(FILE * infile);

pair<long float, long int> read_chi(FILE * infile);