#include "../vc.h"
#include <stdio.h>


int main()
{
    IVC* image;
    IVC* image2;



    
    image = vc_read_image("Images/Additional/flower.ppm");

    image2 = vc_image_new(image->width, image->height, 1, image->levels);


    vc_rgb_to_gray(image, image2);

    printf("errooooo");

    vc_write_image("ola.pgm", image2);



    vc_image_free(image);
    vc_image_free(image2);

    return 0;












}