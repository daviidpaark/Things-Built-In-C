#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

int parseInt(char *value)
{
    int number = 0;
    for (int i = 0; *value != '\0'; ++i)
    {
        if ((*value >= '0') && (*value <= '9'))
        {
            number = (number * 10) + (*value - '0');
            value++;
        }
        else
            return -1;
    }
    return number;
}

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
        if ((*(p + 1) != '\0') || *(argv + 2) != NULL)
            return -1;
        global_options = 0x40000000;
        return 0;
    case 'c':
        if (*(p + 1) != '\0')
            return -1;
        if (*(argv + 2) == NULL)
        {
            global_options = 0x20000000;
            return 0;
        }

        q = *(argv + 2);
        if ((*q != '-') || (*(q + 1) != 'p') || (*(q + 2) != '\0'))
            return -1;
        if (*(argv + 3) == NULL)
        {
            global_options = 0x30000004;
            return 0;
        }

        q = *(argv + 3);
        if (*q == '-')
            return -1;
        indent_level = parseInt(q);
        if (indent_level < 0 || indent_level > 127)
            return -1;
        global_options = 0x30000000 + indent_level;
        return 0;
    default:
        return -1;
    }
}