#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */

int validargs(int argc, char **argv)
{
    if ((argc == 1) | (argc > 4))
        return -1;

    char *p, *q;
    p = *(argv + 1);
    if (*p != '-')
        return -1;
    p++;

    switch (*p)
    {
    case 'h':
        if (*(p + 1) != '\0')
            return -1;
        global_options = 0x80000000;
        return 0;
    case 'v':
        if (*(argv + 2) != NULL)
            return -1;
        global_options = 0x40000000;
        return 0;
    case 'c':
        if (*(argv + 2) != NULL)
        {
            q = *(argv + 2);
            if ((*q != '-') || (*(q + 1) != 'p'))
                return -1;
            q++;
            if (*(argv + 3) == NULL)
            {
                global_options = 0x30000000;
                return 0;
            }
            q = *(argv + 3);
            global_options = 0x30000000; // TO-DO Parse string to int and add to global_options
            return 0;
        }
        global_options = 0x20000000;
        return 0;
    default:
        return -1;
    }
}