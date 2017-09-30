#pragma once
#include <string>
#include <stdio.h>
#include <utility>

using namespace std;

void write_steg_message(FILE * infile, FILE * outfile, string msg);

string read_steg_message(FILE * infile);

pair<long float, long int> read_chi(FILE * infile);

std::tuple<int, int, int, int> read_RS(FILE * infile);

string leitura_Completa(FILE * infile, string input);

float media(FILE * infile);

float variancia(FILE * infile, string input);

float curtose(FILE * infile);

float obliquidade(FILE * infile);