#include "../vc.h"
#include <stdio.h>
#include <math.h>


int main()
{
    IVC *image, *image2;

    image = vc_read_image("Images/FLIR/flir-02.pgm");
    if (image == NULL)
    {
        printf("ERROR -> vc_image_new():\n\tOut of memory!\n");
        getchar();
        return 0;
    }
    
    image2 = vc_image_new(image->width,image->height,3,image->levels);
    vc_scale_gray_to_rgb(image,image2);

    
    vc_write_image("teste.ppm", image2);
    vc_image_free(image2);
    vc_image_free(image);

    
    return 0;


}