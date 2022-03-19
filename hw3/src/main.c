#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    size_t sz_x = sizeof(double) * 8, sz_y = sizeof(int);
	void *x = sf_malloc(sz_x);
	void *y = sf_realloc(x, sz_y);
    y = y;

    sf_show_heap();

    return EXIT_SUCCESS;
}
