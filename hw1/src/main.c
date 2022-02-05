#include <stdio.h>
#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if (validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if (global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);

    if (global_options == VALIDATE_OPTION)
    {
        if (argo_read_value(stdin) != NULL)
            return EXIT_SUCCESS;
    }
    else if (global_options == CANONICALIZE_OPTION)
    {
        ARGO_VALUE *head = argo_read_value(stdin);
        if (head != NULL)
        {
            if (argo_write_value(head, stdout) == 0)
                return EXIT_SUCCESS;
        }
    }
    else if (global_options >= (CANONICALIZE_OPTION | PRETTY_PRINT_OPTION))
    {
        ARGO_VALUE *head = argo_read_value(stdin);
        if (head != NULL)
        {
            if (argo_write_value(head, stdout) == 0)
                return EXIT_SUCCESS;
        }
    }
    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
