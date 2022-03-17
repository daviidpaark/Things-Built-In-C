#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    double *ptr = sf_malloc(sizeof(double));
    *ptr = 320320320;

    double *ptr2 = sf_malloc(sizeof(double));
    *ptr2 = 320320320;

    double *ptr3 = sf_malloc(sizeof(double));
    *ptr3 = 320320320;



    sf_show_heap();

    return EXIT_SUCCESS;
}
