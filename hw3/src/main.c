#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    // double *ptr = sf_malloc(sizeof(double));
    // *ptr = 320320320;

    // double *ptr2 = sf_malloc(sizeof(double));
    // *ptr2 = 320320320;

    // double *ptr3 = sf_malloc(sizeof(double));
    // *ptr3 = 320320320;
    // double *ptr4 = sf_malloc(sizeof(double));
    // *ptr4 = 320320320;
    // double *ptr5 = sf_malloc(sizeof(double));
    // *ptr5 = 320320320;
    // double *ptr6 = sf_malloc(sizeof(double));
    // *ptr6 = 320320320;

    // sf_free(ptr);
    // sf_free(ptr2);
    // sf_free(ptr3);
    // sf_free(ptr4);
    // sf_free(ptr5);
    // sf_free(ptr6);

    // sf_free(ptr6);
    // sf_free(ptr5);
    // sf_free(ptr4);
    // sf_free(ptr3);
    // sf_free(ptr2);
    // sf_free(ptr);

    size_t sz_u = 200, sz_v = 150, sz_w = 50, sz_x = 150, sz_y = 200, sz_z = 250;
	void *u = sf_malloc(sz_u);
	/* void *v = */ sf_malloc(sz_v);
	void *w = sf_malloc(sz_w);
	/* void *x = */ sf_malloc(sz_x);
	void *y = sf_malloc(sz_y);
	/* void *z = */ sf_malloc(sz_z);

    sf_free(u);
	sf_free(w);
	sf_free(y);

    sf_show_heap();


    return EXIT_SUCCESS;
}
