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
    double *ptr4 = sf_malloc(sizeof(double));
    *ptr4 = 320320320;
    double *ptr5 = sf_malloc(sizeof(double));
    *ptr5 = 320320320;
    double *ptr6 = sf_malloc(sizeof(double));
    *ptr6 = 320320320;

    // sf_show_heap();

    sf_free(ptr);
    sf_free(ptr2);
    sf_free(ptr3);   
    sf_free(ptr4);
    sf_free(ptr5);
    sf_free(ptr6);

    // sf_free(ptr6);
    // sf_free(ptr5);
    // sf_free(ptr4);
    // sf_free(ptr3);
    // sf_free(ptr2);
    // sf_free(ptr);

    sf_show_heap();

    return EXIT_SUCCESS;
}
