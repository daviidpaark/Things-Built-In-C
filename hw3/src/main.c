#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    double *ptr, *ptr2, *ptr3;
    ptr = sf_malloc(sizeof(double));
    ptr2 = sf_malloc(30 * sizeof(double));
    ptr3 = sf_malloc(60 * sizeof(double));

    ptr = ptr;
    ptr2 = ptr2;
    ptr3 = ptr3;

    sf_show_heap();
    printf("%f\n",sf_internal_fragmentation());
    printf("%f\n",sf_peak_utilization());

    return EXIT_SUCCESS;
}
