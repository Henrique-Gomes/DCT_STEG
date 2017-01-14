#include "dct_steg.h"
#include <stdio.h>
#include <iostream>

using namespace std;

int main() {

	cout << "Henrique Gomes de Jesus [versao -1]" <<
		"\n(c) 2017 EMRIQ CORPORATION. Nenhum direito reservado.\n" << endl;

	string	input;
	string	output;
	string	msg;

	FILE *	infile;
	FILE *	outfile;

	while (true) {

		cout << "Insira G para gravar ou L para ler" << endl;
		cin >> msg;

		if ((msg == "L") || (msg == "l")) {

			do {
				cout << "\nInsira a entrada" << endl;
				cin >> input;
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			cout << read_steg_message(infile) << endl;

		}

		else if ((msg == "G") || (msg == "g")) {

			do {
				cout << "\nInsira a entrada" << endl;
				cin >> input;
				errno = 0;
				if ((infile = fopen(input.c_str(), "rb")) == NULL) {
					printf("Erro %d ao abrir %s \n", errno, input.c_str());
				}
			} while (infile == NULL);

			cin.ignore(); // limpa o buffer
			cout << "\nInsira a mensagem a ser escondida" << endl;
			getline(cin, msg);
			//cin >> msg;

			do {
				cout << "\nInsira a saida" << endl;
				cin >> output;
				errno = 0;
				if ((outfile = fopen(output.c_str(), "wb")) == NULL) {
					printf("Erro %d ao tentar gravar %s \n", errno, output.c_str());
				}
			} while (outfile == NULL);

			write_steg_message(infile, outfile, msg);
		}
		else
			cout << "\nopcao invalida" << endl;

		cout << "\n " << endl;
		system("pause");
		cout << "\n " << endl;
	}
	return 0;
}