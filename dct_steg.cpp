#include <stdio.h>
#include <errno.h>
#include <jpeglib.h>
#include <iostream>
#include <string>
#include <math.h>

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
	fclose(outfile);
	fclose(infile);

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

	fclose(infile);

	return msg;
}

pair<long float, long int> read_chi(FILE * infile) {

	float		z		= 0;											// frequência esperada
	long float	chi		= 0;											// qui-quadrado
	long int*	f		= (long int *)malloc(65536*sizeof(long int));	// frequência encontrada
	long int	tam		= 0;

	struct	jpeg_decompress_struct	srcinfo;
	struct	jpeg_error_mgr			srcerr;

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

	for (int i = 0; i < 32768; i++) {
		z = (f[2*i] + f[2*i + 1]) / 2;
		if (z > 0.1)
			chi += pow((f[2 * i] - z),2) / z;
	}

	jpeg_finish_decompress(&srcinfo);

	jpeg_destroy_decompress(&srcinfo);

	fclose(infile);

	return make_pair(chi, tam);
}