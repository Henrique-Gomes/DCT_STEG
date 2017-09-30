#include "dct_steg.h"
#include "text_source.h"
#include <stdio.h>
#include <iostream>
#include <utility>
#include <tuple>
#include <string>
//#include <filesystem>
//#include <afx.h>
#include <dirent.h>
//#include <cstdlib>
#include <fstream>


using std::cout;
using std::pair;

bool isJPG(string filename) {
	for (int i = 0; i < filename.length(); i++)
		filename[i] = toupper(filename[i]);

	return ((filename.find(".JPG") != 4294967295) || (filename.find(".JPEG") != 4294967295));
}

void batch_write(string diretorio) {

	string arquivo;
	DIR *dir = 0;
	struct dirent *entrada = 0;
	unsigned char isDir = 0x4;
	unsigned char isFile = 0x8;
	int cont = 0;
	FILE *	infile;
	FILE *	outfile;
	string output;

	dir = opendir(diretorio.c_str());

	if (dir == 0) {
		std::cerr << "Nao foi possivel abrir diretorio." << std::endl;
		exit(1);
	}

	while (entrada = readdir(dir)) {
		arquivo = diretorio + "\\" + entrada->d_name;
		if (isJPG(arquivo))/*((arquivo.find(".jpg") != 4294967295) || (arquivo.find(".jpeg") != 4294967295))*/ {

			if ((infile = fopen(arquivo.c_str(), "rb")) != NULL) {

				output = diretorio + "\\STEG\\" + entrada->d_name/*to_string(cont) + ".jpg"*/;

				if ((outfile = fopen(output.c_str(), "wb")) != NULL) {

					write_steg_message(infile, outfile, getString());

					cont++;

					fclose(outfile);
				} else
					printf("Erro %d ao escrever %s \n", errno, output.c_str());
				fclose(infile);
			}
		}
	}

	closedir(dir);
}

int main() {

	cout << "Henrique Gomes de Jesus [versao -1]" <<
		"\n(c) 2017 EMRIQ CORPORATION. Nenhum direito reservado.\n" << endl;

	string	input;
	string	output;
	string	msg;
	string	arquivo;

	pair<long float, long int> chi; // qui quadrado ; tamanho
	std::tuple<int, int, int, int> RS; // R; S; Rn; Sn

	DIR *dir = 0;
	struct dirent *entrada = 0;
	unsigned char isDir = 0x4;
	unsigned char isFile = 0x8;

	FILE *		infile;
	FILE *		outfile;
	ofstream	CSV_file;

	while (true) {

		cout << "Opcoes" << endl;
		cout << "G: Gravar" << endl;
		cout << "L: Ler" << endl;
		cout << "C: Qui-Quadrado" << endl;
		cout << "R: RS Esteganalise" << endl;
		cout << "M: Media" << endl;
		cout << "V: Variancia" << endl;
		cout << "K: Curtose" << endl;
		cout << "O: Obliquidade" << endl;
		cout << "LC: Leitura Esteganalitica Completa" << endl;
		cout << "GL: Gravar em lotes" << endl;
		cout << "LL: Ler em lotes" << endl;
		cout << endl;
		getline(std::cin, msg);
		//cin >> msg;

		if ((msg == "L") || (msg == "l")) {

			do {
				cout << "\nInsira a entrada" << endl;
				getline(std::cin, input);
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			cout << "\nMensagem lida:\n\n" << read_steg_message(infile) << endl;

			fclose(infile);

		}

		else if ((msg == "G") || (msg == "g")) {

			do {
				cout << "\nInsira a entrada" << endl;
				getline(std::cin, input);
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			cin.ignore(); // limpa o buffer
			cout << "\nInsira a mensagem a ser escondida" << endl;
			getline(cin, msg);

			do {
				cout << "\nInsira a saida" << endl;
				getline(std::cin, output);
				errno = 0;
				if ((outfile = fopen(output.c_str(), "wb")) == NULL) {
					printf("Erro %d ao tentar gravar %s \n", errno, output.c_str());
				}
			} while (outfile == NULL);

			write_steg_message(infile, outfile, msg);

			fclose(infile);

			fclose(outfile);
		}

		else if ((msg == "C") || (msg == "c")) {

			do {
				cout << "\nInsira a entrada" << endl;
				getline(std::cin, input);
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			chi = read_chi(infile);

			cout << "\ntamanho: " << to_string(chi.second) << endl;
			cout << "qui quadrado: " << to_string(chi.first) << endl;

			fclose(infile);

		}

		else if ((msg == "R") || (msg == "r")) {

			do {
				cout << "\nInsira a entrada" << endl;
				getline(std::cin, input);
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			RS = read_RS(infile);

			cout << "\nR:  " << to_string(get<0>(RS)) << endl;
			cout << "S:  "   << to_string(get<1>(RS)) << endl;
			cout << "Rn: "   << to_string(get<2>(RS)) << endl;
			cout << "Sn: "   << to_string(get<3>(RS)) << endl;

			fclose(infile);
		}

		else if (msg == "M") {

			do {
				cout << "\nInsira a entrada" << endl;
				getline(std::cin, input);
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			cout << "\n" << to_string(media(infile)) << endl;

			fclose(infile);
		}

		else if (msg == "V") {

			do {
				cout << "\nInsira a entrada" << endl;
				getline(std::cin, input);
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			cout << "\n" << to_string(variancia(infile, input)) << endl;

			fclose(infile);
		}

		else if (msg == "K") {

			do {
				cout << "\nInsira a entrada" << endl;
				getline(std::cin, input);
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			cout << "\n" << to_string(curtose(infile)) << endl;

			fclose(infile);
		}

		else if (msg == "O") {

			do {
				cout << "\nInsira a entrada" << endl;
				getline(std::cin, input);
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			cout << "\n" << to_string(obliquidade(infile)) << endl;

			fclose(infile);
		}

		else if (msg == "LC") {

			do {
				cout << "\nInsira a entrada" << endl;
				getline(std::cin, input);
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			cout << "\n" << leitura_Completa(infile,input) << endl;

			fclose(infile);
		}

		else if (msg == "GL") {

			cout << "\nInsira o diretorio com JPEGs" << endl;
			cout << "\nO diretorio deve ser inserido sem contra barra no final." << endl;
			cout << "Alem dos JPEGs, deve ter dentro da pasta uma pasta vazia chamada STEG" << endl;
			cout << "onde as estego imagens serao gravadas." << "\n" << endl;
			getline(std::cin, input);

			dir = opendir(input.c_str());

			if (dir == 0) {
				std::cerr << "Nao foi possivel abrir diretorio." << std::endl;
				exit(1);
			}

			batch_write(input);

			/*while (entrada = readdir(dir)) {
				msg = entrada->d_name;
				if (msg.find(".jpg") != 4294967295)
					std::cout << msg << std::endl;
			}

			closedir(dir);*/

		}

		else if (msg == "LL") {

			cout << "\nInsira o diretorio com JPEGs" << endl;
			cout << "\nO diretorio deve ser inserido sem contra barra no final." << endl;
			cout << "Sera salvo um arquivo data.CSV no mesmo diretorio com os dados lidos.\n" << endl;
			getline(std::cin, input);

			dir = opendir(input.c_str());

			if (dir == 0) {
				std::cerr << "\nNao foi possivel abrir diretorio de entrada." << std::endl;
				exit(1);
			}

			CSV_file.open(input+"\\data.CSV", std::ios::app);

			if (! CSV_file.is_open())
			{
				cout << "\nO arquivo data.CSV nao pode ser criado.";
				exit(1);
			}



			while (entrada = readdir(dir)) {
				arquivo = input + "\\" + entrada->d_name;
				if (isJPG(arquivo)) {

					if ((infile = fopen(arquivo.c_str(), "rb")) != NULL) {
						CSV_file << leitura_Completa(infile, input) << "\n";
						fclose(infile);
					}
					else {
						arquivo = entrada->d_name;
						cout << "\nO arquivo " + arquivo + " nao pode ser lido e sera ignorado.";
					}
				}
			}

			closedir(dir);

			CSV_file.close();
		}

		else
			cout << "\nopcao invalida" << endl;

		cout << "\n " << endl;
		system("pause");
		cout << "\n " << endl;
	}
	return 0;
}