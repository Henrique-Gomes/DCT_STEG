#include <stdio.h>
#include <errno.h>
#include <jpeglib.h>
#include <iostream>
#include <string>
#include <math.h>
#include <tuple>

using namespace std;

JBLOCKARRAY rowPtrs[MAX_COMPONENTS];

void write_steg(jpeg_decompress_struct srcinfo, jvirt_barray_ptr * src_coef_arrays, string msg) {
	long int		cont = 0;

	//cout << "Started reading DCT" << endl;
	for (JDIMENSION compNum = 0; compNum < srcinfo.num_components; compNum++) {
		size_t blockRowSize = (size_t) sizeof(JCOEF) * DCTSIZE2 * srcinfo.comp_info[compNum].width_in_blocks;
		for (JDIMENSION rowNum = 0; rowNum < srcinfo.comp_info[compNum].height_in_blocks; rowNum++) {
			// A pointer to the virtual array of dct values
			rowPtrs[compNum] = ((&srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[compNum], rowNum, (JDIMENSION)1, FALSE);
			// Loop through the blocks to get the dct values
			for (JDIMENSION blockNum = 0; blockNum < srcinfo.comp_info[compNum].width_in_blocks; blockNum++) {
				//...iterate over DCT coefficients
				for (JDIMENSION i = 0; i < DCTSIZE2; i++) {
					// Se a mensagem não acabou
					if (cont < (msg.size()+1)*8) {

						if ((unsigned char)(msg[(int)(cont / 8)] << (cont % 8)) > 0x7f) // se bit == 1
							rowPtrs[compNum][0][blockNum][i] = (JCOEF)(rowPtrs[compNum][0][blockNum][i] | 0x0001);
						else
							rowPtrs[compNum][0][blockNum][i] = (JCOEF)(rowPtrs[compNum][0][blockNum][i] & 0xfffe);

						cont++;
					}
				}
			}
		}
	}
}

void write_steg_message(FILE * infile, FILE * outfile, string msg) {

	struct	jpeg_decompress_struct	srcinfo;
	struct	jpeg_error_mgr			srcerr;

	struct	jpeg_compress_struct	destinfo;
	struct	jpeg_error_mgr			desterr;

	srcinfo.err = jpeg_std_error(&srcerr);
	jpeg_create_decompress(&srcinfo);
	jpeg_stdio_src(&srcinfo, infile);
	(void)jpeg_read_header(&srcinfo, FALSE);

	//coefficients
	jvirt_barray_ptr * src_coef_arrays = jpeg_read_coefficients(&srcinfo);
	write_steg(srcinfo, src_coef_arrays, msg);

	destinfo.err = jpeg_std_error(&desterr);
	jpeg_create_compress(&destinfo);
	jpeg_stdio_dest(&destinfo, outfile);
	jpeg_copy_critical_parameters(&srcinfo, &destinfo);
	destinfo.in_color_space = srcinfo.out_color_space;
	jpeg_write_coefficients(&destinfo, src_coef_arrays);

	jpeg_finish_compress(&destinfo);
	jpeg_finish_decompress(&srcinfo);

	jpeg_destroy_compress(&destinfo);
	jpeg_destroy_decompress(&srcinfo);
	std::fclose(outfile);
	std::fclose(infile);

	return;
}

string read_steg(jpeg_decompress_struct srcinfo, jvirt_barray_ptr * src_coef_arrays) {
	long int		cont = 0;
	string	msg = "";

	//cout << "Started reading DCT" << endl;
	for (JDIMENSION compNum = 0; compNum < srcinfo.num_components; compNum++) {
		size_t blockRowSize = (size_t) sizeof(JCOEF) * DCTSIZE2 * srcinfo.comp_info[compNum].width_in_blocks;
		for (JDIMENSION rowNum = 0; rowNum < srcinfo.comp_info[compNum].height_in_blocks; rowNum++) {
			// A pointer to the virtual array of dct values
			rowPtrs[compNum] = ((&srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[compNum], rowNum, (JDIMENSION)1, FALSE);
			// Loop through the blocks to get the dct values
			for (JDIMENSION blockNum = 0; blockNum < srcinfo.comp_info[compNum].width_in_blocks; blockNum++) {
				//...iterate over DCT coefficients
				for (JDIMENSION i = 0; i < DCTSIZE2; i++) {

					if ((cont > 7) && (msg[(int)(cont / 8)-1] == '\0'))
						return msg;

					if ((cont % 8) == 0) {
						msg.push_back((char)(0x00));
					}
					
					if ((rowPtrs[compNum][0][blockNum][i] & 0x0001) == 0x0001) {
						msg[(int)(cont / 8)] += pow(2, (7 - (cont % 8)));
					}

					cont++;
				}
			}
		}
	}

	return msg;
}

string read_steg_message(FILE * infile) {

	struct	jpeg_decompress_struct	srcinfo;
	struct	jpeg_error_mgr			srcerr;

	srcinfo.err = jpeg_std_error(&srcerr);
	jpeg_create_decompress(&srcinfo);
	jpeg_stdio_src(&srcinfo, infile);
	(void)jpeg_read_header(&srcinfo, FALSE);


	//coefficients
	jvirt_barray_ptr * src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	string	msg = read_steg(srcinfo, src_coef_arrays);

	jpeg_finish_decompress(&srcinfo);

	jpeg_destroy_decompress(&srcinfo);

	std::fclose(infile);

	return msg;
}

pair<long float, long int> read_chi(FILE * infile) {

	float		z		= 0;											// frequência esperada
	long float	chi		= 0;											// qui-quadrado
	long int*	f		= (long int *)malloc(65536*sizeof(long int));	// frequência encontrada
	long int	tam		= 0;

	struct	jpeg_decompress_struct	srcinfo;
	struct	jpeg_error_mgr			srcerr;

	// para cada valor possível com 2 bytes
	for (int i = 0; i < 65536; i++)
		f[i] = 0;

	srcinfo.err = jpeg_std_error(&srcerr);
	jpeg_create_decompress(&srcinfo);
	jpeg_stdio_src(&srcinfo, infile);
	(void)jpeg_read_header(&srcinfo, FALSE);

	//coefficients
	jvirt_barray_ptr * src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	//cout << "Started reading DCT" << endl;
	for (JDIMENSION compNum = 0; compNum < srcinfo.num_components; compNum++) {
		size_t blockRowSize = (size_t) sizeof(JCOEF) * DCTSIZE2 * srcinfo.comp_info[compNum].width_in_blocks;
		for (JDIMENSION rowNum = 0; rowNum < srcinfo.comp_info[compNum].height_in_blocks; rowNum++) {
			// A pointer to the virtual array of dct values
			rowPtrs[compNum] = ((&srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[compNum], rowNum, (JDIMENSION)1, FALSE);
			// Loop through the blocks to get the dct values
			for (JDIMENSION blockNum = 0; blockNum < srcinfo.comp_info[compNum].width_in_blocks; blockNum++) {
				//...iterate over DCT coefficients
				for (JDIMENSION i = 0; i < DCTSIZE2; i++) {

					f[(unsigned short)rowPtrs[compNum][0][blockNum][i]]++;
					tam++;
				}
			}
		}
	}
	
	// para cada par (65536/2)
	for (int i = 0; i < 32768; i++) {
		z = (f[2*i] + f[2*i + 1]) / 2;
		if (z > 0.1)
			chi += pow((f[2 * i] - z),2) / z;
	}

	jpeg_finish_decompress(&srcinfo);

	jpeg_destroy_decompress(&srcinfo);

	std::fclose(infile);

	return make_pair(chi, tam);
}

std::tuple<int, int, int> RS_Flip(std::tuple<int, int, int> G, int function) {

	// máscara = {0,1,0}

	if (get<1>(G) % 2 == 0)
		return std::make_tuple(get<0>(G), get<1>(G) + function, get<2>(G));
	else
		return std::make_tuple(get<0>(G), get<1>(G) - function, get<2>(G));
}

float RS_noise(std::tuple<int, int, int> G) {

	return (abs(get<1>(G) - get<0>(G)) + abs(get<2>(G) - get<1>(G))) / 2;
}

std::tuple<int, int, int, int> read_RS(FILE * infile) {

	std::tuple<int, int, int> trio = make_tuple(0,0,0);
	std::tuple<int, int, int, int> resultado = make_tuple(0, 0, 0, 0);
	int R = 0;		// contador de grupos regulares
	int S = 0;		// contador de grupos singulares
	int Rn = 0;		// contador de grupos regulares com máscara negativa
	int Sn = 0;		// contador de grupos singulares com máscara negativa

	float ffg, fg;	// valores de f(F(G)) e f(G)

	int cont = 0;

	struct	jpeg_decompress_struct	srcinfo;
	struct	jpeg_error_mgr			srcerr;

	srcinfo.err = jpeg_std_error(&srcerr);
	jpeg_create_decompress(&srcinfo);
	jpeg_stdio_src(&srcinfo, infile);
	(void)jpeg_read_header(&srcinfo, FALSE);

	//coefficients
	jvirt_barray_ptr * src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	//cout << "Started reading DCT" << endl;
	for (JDIMENSION compNum = 0; compNum < srcinfo.num_components; compNum++) {
		size_t blockRowSize = (size_t) sizeof(JCOEF) * DCTSIZE2 * srcinfo.comp_info[compNum].width_in_blocks;
		for (JDIMENSION rowNum = 0; rowNum < srcinfo.comp_info[compNum].height_in_blocks; rowNum++) {
			// A pointer to the virtual array of dct values
			rowPtrs[compNum] = ((&srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[compNum], rowNum, (JDIMENSION)1, FALSE);
			// Loop through the blocks to get the dct values
			for (JDIMENSION blockNum = 0; blockNum < srcinfo.comp_info[compNum].width_in_blocks; blockNum++) {
				//...iterate over DCT coefficients
				for (JDIMENSION i = 0; i < DCTSIZE2; i++) {

					switch (cont % 3) {
					case 0:
						get<0>(trio) = rowPtrs[compNum][0][blockNum][i]; break;
					case 1:
						get<1>(trio) = rowPtrs[compNum][0][blockNum][i]; break;
					case 2:
						get<2>(trio) = rowPtrs[compNum][0][blockNum][i];
					}

					if (cont % 3 == 2) {

						fg = RS_noise(trio);
						ffg = RS_noise(RS_Flip(trio,1));

						if (ffg > fg)
							R++;
						else if (ffg < fg)
							S++;

						ffg = RS_noise(RS_Flip(trio, -1));

						if (ffg > fg)
							Rn++;
						else if (ffg < fg)
							Sn++;
					}

					cont++;
				}
			}
		}
	}

	jpeg_finish_decompress(&srcinfo);

	jpeg_destroy_decompress(&srcinfo);

	std::fclose(infile);

	get<0>(resultado) = R;
	get<1>(resultado) = S;
	get<2>(resultado) = Rn;
	get<3>(resultado) = Sn;

	return resultado;
}

float media(FILE * infile) {

	long int	soma = 0, tam = 0;

	struct	jpeg_decompress_struct	srcinfo;
	struct	jpeg_error_mgr			srcerr;

	srcinfo.err = jpeg_std_error(&srcerr);
	jpeg_create_decompress(&srcinfo);
	jpeg_stdio_src(&srcinfo, infile);
	(void)jpeg_read_header(&srcinfo, FALSE);

	//coefficients
	jvirt_barray_ptr * src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	//cout << "Started reading DCT" << endl;
	for (JDIMENSION compNum = 0; compNum < srcinfo.num_components; compNum++) {
		size_t blockRowSize = (size_t) sizeof(JCOEF) * DCTSIZE2 * srcinfo.comp_info[compNum].width_in_blocks;
		for (JDIMENSION rowNum = 0; rowNum < srcinfo.comp_info[compNum].height_in_blocks; rowNum++) {
			// A pointer to the virtual array of dct values
			rowPtrs[compNum] = ((&srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[compNum], rowNum, (JDIMENSION)1, FALSE);
			// Loop through the blocks to get the dct values
			for (JDIMENSION blockNum = 0; blockNum < srcinfo.comp_info[compNum].width_in_blocks; blockNum++) {
				//...iterate over DCT coefficients
				for (JDIMENSION i = 0; i < DCTSIZE2; i++) {
					soma += rowPtrs[compNum][0][blockNum][i] & 0x0001;
					tam++;
				}
			}
		}
	}

	jpeg_finish_decompress(&srcinfo);

	jpeg_destroy_decompress(&srcinfo);

	std::fclose(infile);

	return soma*1.0/tam;
}

float variancia(FILE * infile, float media) {

	struct	jpeg_decompress_struct	srcinfo;
	struct	jpeg_error_mgr			srcerr;

	srcinfo.err = jpeg_std_error(&srcerr);
	jpeg_create_decompress(&srcinfo);
	jpeg_stdio_src(&srcinfo, infile);
	(void)jpeg_read_header(&srcinfo, FALSE);

	long int	tam = 0;
	double		var = 0;

	//coefficients
	jvirt_barray_ptr * src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	//cout << "Started reading DCT" << endl;
	for (JDIMENSION compNum = 0; compNum < srcinfo.num_components; compNum++) {
		size_t blockRowSize = (size_t) sizeof(JCOEF) * DCTSIZE2 * srcinfo.comp_info[compNum].width_in_blocks;
		for (JDIMENSION rowNum = 0; rowNum < srcinfo.comp_info[compNum].height_in_blocks; rowNum++) {
			// A pointer to the virtual array of dct values
			rowPtrs[compNum] = ((&srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[compNum], rowNum, (JDIMENSION)1, FALSE);
			// Loop through the blocks to get the dct values
			for (JDIMENSION blockNum = 0; blockNum < srcinfo.comp_info[compNum].width_in_blocks; blockNum++) {
				//...iterate over DCT coefficients
				for (JDIMENSION i = 0; i < DCTSIZE2; i++) {
					var += pow((rowPtrs[compNum][0][blockNum][i] & 0x0001) - media, 2);
					tam++;
				}
			}
		}
	}

	var = (var / (tam - 1));

	jpeg_finish_decompress(&srcinfo);

	jpeg_destroy_decompress(&srcinfo);

	std::fclose(infile);

	return var;
}

float variancia(FILE * infile, string input) {

	float	med	=	media(infile);

	if ((infile = fopen(input.c_str(), "rb")) == NULL)
		printf("Erro %d ao abrir %s \n", errno, input.c_str());

	return variancia(infile, med);

}

float curtose(FILE * infile) {

	long long n, n1;
	double M1, M2, M3, M4;
	double delta, delta_n, delta_n2, term1;

	n = n1 = 0;
	M1 = M2 = M3 = M4 = 0.0;

	struct	jpeg_decompress_struct	srcinfo;
	struct	jpeg_error_mgr			srcerr;

	srcinfo.err = jpeg_std_error(&srcerr);
	jpeg_create_decompress(&srcinfo);
	jpeg_stdio_src(&srcinfo, infile);
	(void)jpeg_read_header(&srcinfo, FALSE);

	//coefficients
	jvirt_barray_ptr * src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	//cout << "Started reading DCT" << endl;
	for (JDIMENSION compNum = 0; compNum < srcinfo.num_components; compNum++) {
		size_t blockRowSize = (size_t) sizeof(JCOEF) * DCTSIZE2 * srcinfo.comp_info[compNum].width_in_blocks;
		for (JDIMENSION rowNum = 0; rowNum < srcinfo.comp_info[compNum].height_in_blocks; rowNum++) {
			// A pointer to the virtual array of dct values
			rowPtrs[compNum] = ((&srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[compNum], rowNum, (JDIMENSION)1, FALSE);
			// Loop through the blocks to get the dct values
			for (JDIMENSION blockNum = 0; blockNum < srcinfo.comp_info[compNum].width_in_blocks; blockNum++) {
				//...iterate over DCT coefficients
				for (JDIMENSION i = 0; i < DCTSIZE2; i++) {

					n1 = n;
					n++;
					delta = rowPtrs[compNum][0][blockNum][i] - M1;
					delta_n = delta / n;
					delta_n2 = delta_n * delta_n;
					term1 = delta * delta_n * n1;
					M1 += delta_n;
					M4 += term1 * delta_n2 * (n*n - 3 * n + 3) + 6 * delta_n2 * M2 - 4 * delta_n * M3;
					M3 += term1 * delta_n * (n - 2) - 3 * delta_n * M2;
					M2 += term1;
				}
			}
		}
	}

	jpeg_finish_decompress(&srcinfo);

	jpeg_destroy_decompress(&srcinfo);

	std::fclose(infile);

	return (double)((n)*M4 / (M2*M2) - 3.0);
}

float obliquidade(FILE * infile) {

	long long n, n1;
	double M1, M2, M3, M4;
	double delta, delta_n, delta_n2, term1;

	n = n1 = 0;
	M1 = M2 = M3 = M4 = 0.0;

	struct	jpeg_decompress_struct	srcinfo;
	struct	jpeg_error_mgr			srcerr;

	srcinfo.err = jpeg_std_error(&srcerr);
	jpeg_create_decompress(&srcinfo);
	jpeg_stdio_src(&srcinfo, infile);
	(void)jpeg_read_header(&srcinfo, FALSE);

	//coefficients
	jvirt_barray_ptr * src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	//cout << "Started reading DCT" << endl;
	for (JDIMENSION compNum = 0; compNum < srcinfo.num_components; compNum++) {
		size_t blockRowSize = (size_t) sizeof(JCOEF) * DCTSIZE2 * srcinfo.comp_info[compNum].width_in_blocks;
		for (JDIMENSION rowNum = 0; rowNum < srcinfo.comp_info[compNum].height_in_blocks; rowNum++) {
			// A pointer to the virtual array of dct values
			rowPtrs[compNum] = ((&srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[compNum], rowNum, (JDIMENSION)1, FALSE);
			// Loop through the blocks to get the dct values
			for (JDIMENSION blockNum = 0; blockNum < srcinfo.comp_info[compNum].width_in_blocks; blockNum++) {
				//...iterate over DCT coefficients
				for (JDIMENSION i = 0; i < DCTSIZE2; i++) {

					n1 = n;
					n++;
					delta = rowPtrs[compNum][0][blockNum][i] - M1;
					delta_n = delta / n;
					delta_n2 = delta_n * delta_n;
					term1 = delta * delta_n * n1;
					M1 += delta_n;
					M4 += term1 * delta_n2 * (n*n - 3 * n + 3) + 6 * delta_n2 * M2 - 4 * delta_n * M3;
					M3 += term1 * delta_n * (n - 2) - 3 * delta_n * M2;
					M2 += term1;
				}
			}
		}
	}

	jpeg_finish_decompress(&srcinfo);

	jpeg_destroy_decompress(&srcinfo);

	std::fclose(infile);

	return sqrt((double)n) * M3 / pow(M2, 1.5);
}

string leitura_Completa(FILE * infile, string input) {

	struct	jpeg_decompress_struct	srcinfo;
	struct	jpeg_error_mgr			srcerr;

	// qui quadrado
	float		z = 0;											// frequência esperada
	long float	chi = 0;											// qui-quadrado
	long int*	f = (long int *)malloc(65536 * sizeof(long int));	// frequência encontrada
	for (int i = 0; i < 65536; i++)
		f[i] = 0;

	// RS
	std::tuple<int, int, int> trio = make_tuple(0, 0, 0);
	int R = 0;		// contador de grupos regulares
	int S = 0;		// contador de grupos singulares
	int Rn = 0;		// contador de grupos regulares com máscara negativa
	int Sn = 0;		// contador de grupos singulares com máscara negativa
	float ffg, fg;	// valores de f(F(G)) e f(G)

	long int cont = 0;
	long int soma = 0;

	// curtose e obliquidade
	long long n, n1;
	double M1, M2, M3, M4;
	double delta, delta_n, delta_n2, term1;
	n = n1 = 0;
	M1 = M2 = M3 = M4 = 0.0;

	// variancia
	float var;

	srcinfo.err = jpeg_std_error(&srcerr);
	jpeg_create_decompress(&srcinfo);
	jpeg_stdio_src(&srcinfo, infile);
	(void)jpeg_read_header(&srcinfo, FALSE);

	//coefficients
	jvirt_barray_ptr * src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	//cout << "Started reading DCT" << endl;
	for (JDIMENSION compNum = 0; compNum < srcinfo.num_components; compNum++) {
		size_t blockRowSize = (size_t) sizeof(JCOEF) * DCTSIZE2 * srcinfo.comp_info[compNum].width_in_blocks;
		for (JDIMENSION rowNum = 0; rowNum < srcinfo.comp_info[compNum].height_in_blocks; rowNum++) {
			// A pointer to the virtual array of dct values
			rowPtrs[compNum] = ((&srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[compNum], rowNum, (JDIMENSION)1, FALSE);
			// Loop through the blocks to get the dct values
			for (JDIMENSION blockNum = 0; blockNum < srcinfo.comp_info[compNum].width_in_blocks; blockNum++) {
				//...iterate over DCT coefficients
				for (JDIMENSION i = 0; i < DCTSIZE2; i++) {

					f[(unsigned short)rowPtrs[compNum][0][blockNum][i]]++;

					switch (cont % 3) {
					case 0:
						get<0>(trio) = rowPtrs[compNum][0][blockNum][i]; break;
					case 1:
						get<1>(trio) = rowPtrs[compNum][0][blockNum][i]; break;
					case 2:
						get<2>(trio) = rowPtrs[compNum][0][blockNum][i];
					}

					if (cont % 3 == 2) {

						fg = RS_noise(trio);
						ffg = RS_noise(RS_Flip(trio, 1));

						if (ffg > fg)
							R++;
						else if (ffg < fg)
							S++;

						ffg = RS_noise(RS_Flip(trio, -1));

						if (ffg > fg)
							Rn++;
						else if (ffg < fg)
							Sn++;
					}

					cont++;
					soma += rowPtrs[compNum][0][blockNum][i] & 0x0001;

					n1 = n;
					n++;
					delta = rowPtrs[compNum][0][blockNum][i] - M1;
					delta_n = delta / n;
					delta_n2 = delta_n * delta_n;
					term1 = delta * delta_n * n1;
					M1 += delta_n;
					M4 += term1 * delta_n2 * (n*n - 3 * n + 3) + 6 * delta_n2 * M2 - 4 * delta_n * M3;
					M3 += term1 * delta_n * (n - 2) - 3 * delta_n * M2;
					M2 += term1;
				}
			}
		}
	}

	// para cada par (65536/2)
	for (int i = 0; i < 32768; i++) {
		z = (f[2 * i] + f[2 * i + 1]) / 2;
		if (z > 0.1)
			chi += pow((f[2 * i] - z), 2) / z;
	}

	jpeg_finish_decompress(&srcinfo);

	jpeg_destroy_decompress(&srcinfo);

	std::fclose(infile);

	if ((infile = fopen(input.c_str(), "rb")) == NULL)
		printf("Erro %d ao abrir %s \n", errno, input.c_str());

	var = variancia(infile, 1.0*soma / cont);

	return 
		to_string(cont) + "," +
		to_string(chi) + "," +
		to_string(R) + "," +
		to_string(S) + "," +
		to_string(Rn) + "," +
		to_string(Sn) + "," +
		to_string(1.0*soma/cont) + "," +
		to_string(var) + "," +
		to_string((double)((n)*M4 / (M2*M2) - 3.0)) + "," +
		to_string(sqrt((double)n) * M3 / pow(M2, 1.5));
	
	// Tamanho, Qui Quadrado, RS Rm, RS Sm, RS R-m, RS R-s, Media, Variancia, curtose, obliquidade
}

/*int RS_Flip(int num, int function) {
		    kmk
	if (num % 2 == 0)
		return num + function;
	else
		return num - function;
}*/