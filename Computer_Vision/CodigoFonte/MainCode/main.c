#include "../vc.h"
#include <stdio.h>
#include <math.h>


int main()
{
    IVC *image, *image2, *image3;

    image = vc_read_image("Images/FLIR/flir-01.pgm");
    if (image == NULL)
    {
        printf("ERROR -> vc_image_new():\n\tOut of memory!\n");
        getchar();
        return 0;
    }

    image2 = vc_image_new(image->width,image->height,image->channels,1);
    if (image2 == NULL)
    {
        printf("ERROR -> vc_image_new():\n\tOut of memory!\n");
        getchar();
        return 0;
    }
    image3 = vc_image_new(image2->width,image2->height,image2->channels,1);


    vc_gray_to_binary(image,image2,127);
    vc_binary_dilate(image2,image3,3);


    vc_write_image("teste2.pgm", image3);
    vc_image_free(image2);
    vc_image_free(image);
    vc_image_free(image3);
    
    return 0;


}