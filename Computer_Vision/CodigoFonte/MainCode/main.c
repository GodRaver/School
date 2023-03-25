#include "../vc.h"
#include <stdio.h>
#include <math.h>


int main()
{
    IVC *image, *image2;

    image = vc_read_image("Images/ImagensTESTE/cells.pgm");
    if (image == NULL)
    {
        printf("ERROR -> vc_image_new():\n\tOut of memory!\n");
        getchar();
        return 0;
    }
    
    image2 = vc_image_new(image->width,image->height,1,image->levels);
    vc_gray_to_binary_global_mean(image,image2);

    
    vc_write_image("teste.pgm", image2);
    vc_image_free(image2);
    vc_image_free(image);

    
    return 0;


}