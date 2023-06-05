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

int vc_scale_gray_to_rgb(IVC* src, IVC* dst)
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


	if((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if((src->width != dst->width) || (src->height != dst->height)) return 0;
	if((src->channels != 1) || (dst->channels != 3)) return 0;

	for(y=0; y<height;y++)
	{
		for(x=0; x<width; x++)
		{
			pos = y * bytesperline + x * channels;
			pos_dst = y * bytesperlinedst + x * channels_dst;


			if(data[pos] < 64)
			{
				datadst[pos_dst] = 0;
				datadst[pos_dst+1] = data[pos]*4;
				datadst[pos_dst+2] = 255;
			}

			else if(data[pos] < 128)
			{
				datadst[pos_dst] = 0;
				datadst[pos_dst+1] = 255;
				datadst[pos_dst+2] = 255-((data[pos]-64)*4);
			}
			else if(data[pos]< 192)
			{
				datadst[pos_dst] = ((data[pos]-128)*4);
				datadst[pos_dst+1] = 255;
				datadst[pos_dst+2] = 0;
			}
			else
			{
				datadst[pos_dst] = 255;
				datadst[pos_dst+1] = 255 -((data[pos]-64)*4);
				datadst[pos_dst+2] = 0;
			}
		}
	}
	return 0;

}


int vc_gray_to_binary(IVC *src,IVC *dst, int threshold)
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



	if((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if((src->width != dst->width) || (src->height != dst->height)) return 0;
	if((src->channels != 1) || (dst->channels != 1)) return 0;

	for(y=0; y<height;y++)
	{
		for(x=0; x<width; x++)
		{
			pos = y * bytesperline + x * channels;
			pos_dst = y * bytesperlinedst + x * channels_dst;

		
			
			if(data[pos] > threshold)
				datadst[pos_dst] = 255;
			else
				datadst[pos_dst] = 0;

			
		
		
		
		
		
		
		
		
		}	
	}


}

int vc_gray_to_binary_global_mean(IVC *src,IVC *dst)
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
	float media=0;



	if((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if((src->width != dst->width) || (src->height != dst->height)) return 0;
	if((src->channels != 1) || (dst->channels != 1)) return 0;

	for(y=0; y<height;y++)
	{
		for(x=0; x<width; x++)
		{
			pos = y * bytesperline + x * channels;

			media += data[pos];


		
		
		
		}	
	}
	media = media/(width*height);


	
	for(y=0; y<height;y++)
	{
		for(x=0; x<width; x++)
		{
			pos = y * bytesperline + x * channels;
			pos_dst = y * bytesperlinedst + x * channels_dst;

			if(data[pos] > media)
				datadst[pos_dst] = 255;
			else
				datadst[pos_dst] = 0;


		
		
		
		}	
	}

}
int vc_gray_to_binary_automatic_binary(IVC *src, IVC *dst)
{
    unsigned char *data = (unsigned char *)src->data;
    int bytesperline = src->width * src->channels;
    int channels = src->channels;
    unsigned char *datadst = (unsigned char *)dst->data;
    int bytesperlinedst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, i, j;
    long int pos, pos_dst;
    int Vmax, Vmin;
    int threshold;
    int neighbor_size = 25;
    int half_neighbor = neighbor_size / 2;

    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
        return 0;
    if ((src->width != dst->width) || (src->height != dst->height))
        return 0;
    if ((src->channels != 1) || (dst->channels != 1))
        return 0;

    // Thresholding using T = (1/2) * (Vmax + Vmin) for each neighborhood
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            Vmax = 0;
            Vmin = 255;

            // Finding Vmax and Vmin in the neighborhood
            for (i = -half_neighbor; i <= half_neighbor; i++)
            {
                for (j = -half_neighbor; j <= half_neighbor; j++)
                {
                    int cur_x = x + j;
                    int cur_y = y + i;

                    // Check if coordinates are within the image bounds
                    if (cur_x >= 0 && cur_x < width && cur_y >= 0 && cur_y < height)
                    {
                        pos = cur_y * bytesperline + cur_x * channels;
                        if (data[pos] > Vmax)
                            Vmax = data[pos];
                        if (data[pos] < Vmin)
                            Vmin = data[pos];
                    }
                }
            }

            // Calculate threshold for the current neighborhood
            threshold = (Vmax + Vmin) / 2;

            pos = y * bytesperline + x * channels;
            pos_dst = y * bytesperlinedst + x * channels_dst;

            if (data[pos] < threshold)
                datadst[pos_dst] = 0;
            else
                datadst[pos_dst] = 255;
        }
    }

    return 1;
}


int vc_gray_to_binary_automatic_binary_bernsen(IVC *src, IVC *dst)
{
    unsigned char *data = (unsigned char *)src->data;
    int bytesperline = src->width * src->channels;
    int channels = src->channels;
    unsigned char *datadst = (unsigned char *)dst->data;
    int bytesperlinedst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y, i, j;
    long int pos, pos_dst;
    int Vmax, Vmin;
	int threshold;
    int neighbor_size = 25;
    int half_neighbor = neighbor_size / 2;
    int Cmin = 15;

    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
        return 0;
    if ((src->width != dst->width) || (src->height != dst->height))
        return 0;
    if ((src->channels != 1) || (dst->channels != 1))
        return 0;

    // Thresholding using Bernsen's method for each neighborhood
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            Vmax = 0;
            Vmin = 255;

            // Finding Vmax and Vmin in the neighborhood
            for (i = -half_neighbor; i <= half_neighbor; i++)
            {
                for (j = -half_neighbor; j <= half_neighbor; j++)
                {
                    int cur_x = x + j;
                    int cur_y = y + i;

                    // Check if coordinates are within the image bounds
                    if (cur_x >= 0 && cur_x < width && cur_y >= 0 && cur_y < height)
                    {
                        pos = cur_y * bytesperline + cur_x * channels;
                        if (data[pos] > Vmax)
                            Vmax = data[pos];
                        if (data[pos] < Vmin)
                            Vmin = data[pos];
                    }
                }
            }

            // Calculate threshold for the current neighborhood using Bernsen's method
            if ((Vmax - Vmin) < 15)
                threshold = 122;
            else
                threshold = (Vmax + Vmin) / 2;

            pos = y * bytesperline + x * channels;
            pos_dst = y * bytesperlinedst + x * channels_dst;

            if (data[pos] < threshold)
                datadst[pos_dst] = 0;
            else
                datadst[pos_dst] = 255;
        }
    }

    return 1;
}

int vc_binary_erode(IVC *src, IVC *dst, int size)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	int xk, yk;
	int i, j;
	long int pos, posk;
	int s1, s2;
	unsigned char pixel;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	s2 = (size - 1) / 2;
	s1 = -(s2);

	memcpy(datadst, datasrc, bytesperline * height);

	// Cálculo da erosão
	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			pos = y * bytesperline + x * channels;

			pixel = datasrc[pos];

			for (yk = s1; yk <= s2; yk++)
			{
				j = y + yk;

				if ((j < 0) || (j >= height)) continue;

				for (xk = s1; xk <= s2; xk++)
				{
					i = x + xk;

					if ((i < 0) || (i >= width)) continue;

					posk = j * bytesperline + i * channels;

					pixel &= datasrc[posk];
				}
			}

			// Se um qualquer pixel da vizinhança, na imagem de origem, for de plano de fundo, então o pixel central
			// na imagem de destino é também definido como plano de fundo.
			if (pixel == 0) datadst[pos] = 0;
		}
	}

	return 1;
}

int vc_binary_dilate(IVC *src, IVC *dst, int size)
{
    unsigned char *datasrc = (unsigned char*)src->data;
    unsigned char *datadst = (unsigned char*)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    int x, y;
    int xk, yk;
    int i, j;
    long int pos, posk;
    int s1, s2;
    unsigned char pixel;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if (channels != 1) return 0;

    s2 = (size - 1) / 2;
    s1 = -(s2);

    memcpy(datadst, datasrc, bytesperline * height);

    // Cálculo da dilatação
    for (y = 0; y<height; y++)
    {
        for (x = 0; x<width; x++)
        {
            pos = y * bytesperline + x * channels;

            pixel = datasrc[pos];

            for (yk = s1; yk <= s2; yk++)
            {
                j = y + yk;

                if ((j < 0) || (j >= height)) continue;

                for (xk = s1; xk <= s2; xk++)
                {
                    i = x + xk;

                    if ((i < 0) || (i >= width)) continue;

                    posk = j * bytesperline + i * channels;

                    pixel &= datasrc[posk];
                }
            }

            // Se um qualquer pixel da vizinhança, na imagem de origem, for de plano de fundo, então o pixel central
            // na imagem de destino é também definido como plano de fundo.
            if (pixel == 255) datadst[pos] = 255;
        }
    }

    return 1;
}

int vc_binary_open(IVC *src, IVC *dst, int kernel)
{

}
int vc_binary_close(IVC *src, IVC *dst, int kernel)
{

}

// Etiquetagem de blobs
// src		: Imagem bin�ria de entrada
// dst		: Imagem grayscale (ir� conter as etiquetas)
// nlabels	: Endere�o de mem�ria de uma vari�vel, onde ser� armazenado o n�mero de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. � necess�rio libertar posteriormente esta mem�ria.
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que ser� retornado desta fun��o.

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem bin�ria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pix�is de plano de fundo devem obrigat�riamente ter valor 0
	// Todos os pix�is de primeiro plano devem obrigat�riamente ter valor 255
	// Ser�o atribu�das etiquetas no intervalo [1,254]
	// Este algoritmo est� assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem bin�ria
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est� marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do n�mero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}


int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 1) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta �rea de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

}

int vc_gray_histogram_show(IVC *src, IVC *dst)
{

}


int vc_gray_histogram_equalization(IVC *src, IVC *dst)
{

}

int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	float histmax;
	int histthreshold;
	int sumx, sumy;
	float hist[256] = { 0.0f };

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	size = width * height;

	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// PosA PosB PosC
			// PosD PosX PosE
			// PosF PosG PosH

			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 1) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			// PosA*(-1) PosB*0 PosC*(1)
			// PosD*(-1) PosX*0 PosE*(1)
			// PosF*(-1) PosG*0 PosH*(1)

			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -1;
			sumx += datasrc[posF] * -1;

			sumx += datasrc[posC] * +1;
			sumx += datasrc[posE] * +1;
			sumx += datasrc[posH] * +1;
			sumx = sumx / 3; // 3 = 1 + 1 + 1

			// PosA*(-1) PosB*(-1) PosC*(-1)
			// PosD*0    PosX*0    PosE*0
			// PosF*(1)  PosG*(1)  PosH*(1)

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -1;
			sumy += datasrc[posC] * -1;

			sumy += datasrc[posF] * +1;
			sumy += datasrc[posG] * +1;
			sumy += datasrc[posH] * +1;
			sumy = sumy / 3; // 3 = 1 + 1 + 1

			//datadst[posX] = (unsigned char)sqrt((double)(sumx*sumx + sumy*sumy));
			datadst[posX] = (unsigned char) (sqrt((double) (sumx*sumx + sumy*sumy)) / sqrt(2.0));
			// Explicação:
			// Queremos que no caso do pior cenário, em que sumx = sumy = 255, o resultado
			// da operação se mantenha no intervalo de valores admitido, isto é, entre [0, 255].
			// Se se considerar que:
			// max = 255
			// Então,
			// sqrt(pow(max,2) + pow(max,2)) * k = max <=> sqrt(2*pow(max,2)) * k = max <=> k = max / (sqrt(2) * max) <=> 
			// k = 1 / sqrt(2)
		}
	}

	// Calcular o histograma com o valor das magnitudes
	for (i = 0; i < size; i++)
	{
		hist[datadst[i]]++;
	}

	// Definir o threshold.
	// O threshold é definido pelo nível de intensidade (das magnitudes)
	// quando se atinge uma determinada percentagem de pixeis, definida pelo utilizador.
	// Por exemplo, se o parâmetro 'th' tiver valor 0.8, significa the o threshold será o 
	// nível de magnitude, abaixo do qual estão pelo menos 80% dos pixeis.
	histmax = 0.0f;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Prewitt Threshold
		if (histmax >= (((float)size) * th)) break;
	}
	histthreshold = i == 0 ? 1 : i;

	// Aplicada o threshold
	for (i = 0; i < size; i++)
	{
		if (datadst[i] >= (unsigned char) histthreshold) datadst[i] = 255;
		else datadst[i] = 0;
	}

	return 1;
}

int vc_gray_edge_sobel(IVC *src, IVC *dst, float th)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	float histmax;
	int histthreshold;
	int sumx, sumy;
	float hist[256] = { 0.0f };

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	size = width * height;

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// PosA PosB PosC
			// PosD PosX PosE
			// PosF PosG PosH

			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 1) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			// PosA*(-1) PosB*0 PosC*(1)
			// PosD*(-2) PosX*0 PosE*(2)
			// PosF*(-1) PosG*0 PosH*(1)

			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -2;
			sumx += datasrc[posF] * -1;
			sumx += datasrc[posC] * 1;
			sumx += datasrc[posE] * 2;
			sumx += datasrc[posH] * 1;
			sumx = sumx / 4; // 4 = 1 + 2 + 1

			// PosA*(-1) PosB*(-2) PosC*(-1)
			// PosD*0    PosX*0    PosE*0
			// PosF*(1)  PosG*(2)  PosH*(1)

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -2;
			sumy += datasrc[posC] * -1;
			sumy += datasrc[posF] * 1;
			sumy += datasrc[posG] * 2;
			sumy += datasrc[posH] * 1;
			sumy = sumy / 4; // 4 = 1 + 2 + 1

			datadst[posX] = (unsigned char)(sqrt((double)(sumx * sumx + sumy * sumy)) / sqrt(2.0));
		}
	}

	// Calcular o histograma com o valor das magnitudes
	for (i = 0; i < size; i++)
	{
		hist[datadst[i]]++;
	}

	// Definir o threshold.
	// O threshold é definido pelo nível de intensidade (das magnitudes)
	// quando se atinge uma determinada percentagem de pixeis, definida pelo utilizador.
	// Por exemplo, se o parâmetro 'th' tiver valor 0.8, significa the o threshold será o 
	// nível de magnitude, abaixo do qual estão pelo menos 80% dos pixeis.
	histmax = 0.0f;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Sobel Threshold
		if (histmax >= (((float)size) * th)) break;
	}
	histthreshold = i == 0 ? 1 : i;

	// Aplicada o threshold
	for (i = 0; i < size; i++)
	{
		if (datadst[i] >= (unsigned char)histthreshold) datadst[i] = 255;
		else datadst[i] = 0;
	}

	return 1;
}








