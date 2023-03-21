//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"
#include <math.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;
	
	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}
	
	t = tok;
	
	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));
		
		if(c == '#') ungetc(c, file);
	}
	
	*t = 0;
	
	return tok;
}


long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);
				
				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;
				
				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;
	
	// Abre o ficheiro
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}
		
		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}
		
		fclose(file);
	}
	else
	{
		#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
		#endif
	}
	
	return image;
}


int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;
	
	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;
			
			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);
			
			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);
		
			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				return 0;
			}
		}
		
		fclose(file);

		return 1;
	}
	
	return 0;
}

int vc_gray_negative(IVC *srcdst)
{
	unsigned char *data = (unsigned char *) srcdst->data;
	int width = srcdst->width;
	int heigth = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x,y;
	long int pos;

	if((srcdst->width<=0) || (srcdst->height<=0) || (srcdst->data == NULL)) return 0;
	if(channels =! 1) return 0;

	for(y=0; y<heigth; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = y*bytesperline + x*channels;
			data[pos] = 255-data[pos];

		}
	}
	return 1;


}

int vc_rgb_negative(IVC *srcdst)
{
	unsigned char *data = (unsigned char *) srcdst->data;
	int width = srcdst->width;
	int heigth = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x,y;
	long int pos;

	if((srcdst->width<=0) || (srcdst->height<=0) || (srcdst->data == NULL)) return 0;
	if(channels =! 3) return 0;

	for(y=0; y<heigth; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = y*bytesperline + x*channels;
			data[pos] = 255-data[pos];
			data[pos+1] = 255-data[pos+1];
			data[pos+2] = 255-data[pos+2];
		}
	}
	return 1;


}

int vc_rgb_to_gray(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char*) src->data;
	int bytesperline_src = src->width *src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*) dst->data;
	int bytesperline_dst = dst->width *dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x,y;
	long int pos_src, pos_dst;
	float rf, gf, bf;


	if((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if((src->width != dst->width) || (src->height != dst->height)) return 0;
	if((src->channels != 3) || (dst->channels != 1)) return 0;

	for(y = 0; y < height; y++)
	{
		for(x=0; x< width; x++)
		{
			pos_src = y*bytesperline_src + x*channels_src;
			pos_dst = y*bytesperline_dst + x*channels_dst;
			while(x < 2)
			{
				printf("%ld", pos_src);
			}
			rf = (float) datasrc[pos_src];
			gf = (float) datasrc[pos_src+1];
			bf = (float) datasrc[pos_src+2];

			datadst[pos_dst] = (unsigned char) ((rf* 0.299) + (gf* 0.587) + (bf*0.144));
		}



	}

	return 1;
}

//Converte imagem de rgb para hsv
int vc_rgb_to_hsv(IVC *src, IVC *dst)
{
    unsigned char *data = (unsigned char *) src->data;
    int bytesperline = src->width * src->channels;
    int channels = src->channels;
	unsigned char *datadst = (unsigned char *) dst->data;
	int bytesperlinedst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
    int height = src->height;
    int x, y;
    long int pos, pos_dst;
	float MAX = 0; 	//Valor Máximo
	float V;		//Valor
	float MIN = 0; 	//Valor Mínimo
	float S = 0;  	//Valor Saturação
	float H = 0; 	//Componente Matiz

	//Atribuir dimensões para segunda imagem
	//dst->width = width;
	//dst->height = height;

    //Verificação de erros
    if ((src->width <=0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 3)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			pos = y * bytesperline + x * channels;
			pos_dst = y * bytesperlinedst + x * channels_dst;
			MAX = data[pos];
			MIN = data[pos];


			//Verificar maior das três componentes RGB
			if (data[pos + 1] > MAX )
			{
				MAX = data[pos + 1];
			}

			if (data[pos + 2] > MAX)
			{
				MAX = data[pos + 2];
			}

			//Atribuir Valor	
			V = MAX;
			// Verificar menor das Três componentes RGB
			if (data[pos + 1] < MIN )
			{
				MIN = data[pos + 1];
			}

			if (data[pos + 2] < MIN)
			{
				MIN = data[pos + 2];
			}

			if (V == 0 || MAX == MIN)
			{
				S = 0;
				H = 0;
			} else
			{
				S = (MAX - MIN) / V;
				if (MAX == data[pos] && data[pos + 1] >= data[pos + 2])
				{
					H = 60 * (data[pos + 1] - data[pos + 2]) / (MAX - MIN);
				} else if (MAX == data[pos] && data[pos + 2] > data[pos + 1])
				{
					H = 360 + 60 * (data[pos + 1] - data[pos + 2]) / (MAX - MIN);
				} else if (MAX  == data[pos + 1])
				{
					H = 120 + 60 * (data[pos + 2] - data[pos]) / (MAX - MIN);
				} else if (MAX == data[pos + 2])
				{
					H = 240 + 60 * (data[pos] - data[pos + 1]) / (MAX - MIN);
				}
			}

			datadst[pos_dst] = (unsigned char)(H / 2);
			datadst[pos_dst + 1] = (unsigned char)(S * 255);
			datadst[pos_dst + 2] = V;
		}
	}

    return 1;
}


int vc_hsv_segmentation(IVC*src, IVC*dst, int hmin,int hmax, int smin, int smax, int vmax, int vmin)
{
	unsigned char *data = (unsigned char *) src->data;
    int bytesperline = src->width * src->channels;
    int channels = src->channels;
	unsigned char *datadst = (unsigned char *) dst->data;
	int bytesperlinedst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
    int height = src->height;
    int x, y;
    long int pos, pos_dst;

	





























}