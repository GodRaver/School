#include "../vc.h"
#include <stdio.h>
#include <math.h>


int main()
{
    IVC *image, *image2;
    int x, y;
    long int pos;

    image = vc_read_image("ImagensHSV/HSVTestImage01.ppm");
    if (image == NULL)
    {
        printf("ERROR -> vc_image_new():\n\tOut of memory!\n");
        getchar();
        return 0;
    }
    image2 = vc_image_new(image->width,image->height,1,image->levels);

    vc_rgb_to_hsv(image,image2);

    


    
    vc_write_image("teste.ppm", image2);
    vc_image_free(image2);
    vc_image_free(image);

    
    printf("Press any key to exit...\n");
    getchar();
    return 0;


}