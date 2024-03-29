#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/*
 * This is the "data store" module for Mush.
 * It maintains a mapping from variable names to values.
 * The values of variables are stored as strings.
 * However, the module provides functions for setting and retrieving
 * the value of a variable as an integer.  Setting a variable to
 * an integer value causes the value of the variable to be set to
 * a string representation of that integer.  Retrieving the value of
 * a variable as an integer is possible if the current value of the
 * variable is the string representation of an integer.
 */

typedef struct store
{
    struct store *next;
    struct store *prev;
    char name[100];
    char string[100];
    long number;
} STORE;

STORE start;

void initStore(STORE *start)
{
    start->next = start;
    start->prev = start;
}

/**
 * @brief  Get the current value of a variable as a string.
 * @details  This function retrieves the current value of a variable
 * as a string.  If the variable has no value, then NULL is returned.
 * Any string returned remains "owned" by the data store module;
 * the caller should not attempt to free the string or to use it
 * after any subsequent call that would modify the value of the variable
 * whose value was retrieved.  If the caller needs to use the string for
 * an indefinite period, a copy should be made immediately.
 *
 * @param  var  The variable whose value is to be retrieved.
 * @return  A string that is the current value of the variable, if any,
 * otherwise NULL.
 */
char *store_get_string(char *var)
{
    if (!start.next)
    {
        return NULL;
    }
    STORE *current = start.next;
    while (current != &start)
    {
        if (strcmp(current->name, var) == 0)
        {
            if (current->string)
                return current->string;
            else
                return NULL;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief  Get the current value of a variable as an integer.
 * @details  This retrieves the current value of a variable and
 * attempts to interpret it as an integer.  If this is possible,
 * then the integer value is stored at the pointer provided by
 * the caller.
 *
 * @param  var  The variable whose value is to be retrieved.
 * @param  valp  Pointer at which the returned value is to be stored.
 * @return  If the specified variable has no value or the value
 * cannot be interpreted as an integer, then -1 is returned,
 * otherwise 0 is returned.
 */
int store_get_int(char *var, long *valp)
{
    if (!start.next)
    {
        return -1;
    }
    STORE *current = start.next;
    while (current != &start)
    {
        if (strcmp(current->name, var) == 0)
        {
            if (current->number)
            {
                *valp = current->number;
                return 0;
            }
            else
            {
                return -1;
            }
        }
        current = current->next;
    }
    return -1;
}

/**
 * @brief  Set the value of a variable as a string.
 * @details  This function sets the current value of a specified
 * variable to be a specified string.  If the variable already
 * has a value, then that value is replaced.  If the specified
 * value is NULL, then any existing value of the variable is removed
 * and the variable becomes un-set.  Ownership of the variable and
 * the value strings is not transferred to the data store module as
 * a result of this call; the data store module makes such copies of
 * these strings as it may require.
 *
 * @param  var  The variable whose value is to be set.
 * @param  val  The value to set, or NULL if the variable is to become
 * un-set.
 */
int store_set_string(char *var, char *val)
{
    if (!start.next)
    {
        initStore(&start);
    }

    STORE *current = start.next;
    while (current != &start)
    {
        if (strcmp(current->name, var) == 0)
        {
            if (val == NULL)
            {
                current->prev->next = current->next;
                current->next->prev = current->prev;
                free(current);
                return 0;
            }
            strcpy(current->string, val);
            char *endptr;
            long value = strtol(val, &endptr, 10);
            if (!errno)
                current->number = value;
            return 0;
        }
        current = current->next;
    }

    STORE *insert = (STORE *)malloc(sizeof(STORE));
    strcpy(insert->name, var);
    strcpy(insert->string, val);

    char *endptr;
    long value = strtol(val, &endptr, 10);
    if (!errno)
        insert->number = value;

    start.prev->next = insert;
    insert->prev = start.prev;
    start.prev = insert;
    insert->next = &start;
    return 0;
}

/**
 * @brief  Set the value of a variable as an integer.
 * @details  This function sets the current value of a specified
 * variable to be a specified integer.  If the variable already
 * has a value, then that value is replaced.  Ownership of the variable
 * string is not transferred to the data store module as a result of
 * this call; the data store module makes such copies of this string
 * as it may require.
 *
 * @param  var  The variable whose value is to be set.
 * @param  val  The value to set.
 */
int store_set_int(char *var, long val)
{
    if (!start.next)
    {
        initStore(&start);
    }

    STORE *current = start.next;
    while (current != &start)
    {
        if (strcmp(current->name, var) == 0)
        {
            current->number = val;
            return 0;
        }
        current = current->next;
    }

    STORE *insert = (STORE *)malloc(sizeof(STORE));
    strcpy(insert->name, var);
    insert->number = val;
    start.prev->next = insert;
    insert->prev = start.prev;
    start.prev = insert;
    insert->next = &start;
    return 0;
}

/**
 * @brief  Print the current contents of the data store.
 * @details  This function prints the current contents of the data store
 * to the specified output stream.  The format is not specified; this
 * function is intended to be used for debugging purposes.
 *
 * @param f  The stream to which the store contents are to be printed.
 */
void store_show(FILE *f)
{
    if (!start.next)
    {
        fprintf(f, "{}");
        return;
    }
    fprintf(f, "{");
    STORE *current = start.next;
    while (current != &start)
    {
        if (current->number || current->number == 0)
            fprintf(f, "%s=%ld", current->name, current->number);
        else if (current->string)
            fprintf(f, "%s=%s", current->name, current->string);
        if (current->next != &start)
            fprintf(f, ",");
        current = current->next;
    }
    fprintf(f, "}");
}
