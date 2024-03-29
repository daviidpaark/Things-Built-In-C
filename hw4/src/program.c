#include <stdlib.h>
#include <stdio.h>

#include "mush.h"
#include "debug.h"

/*
 * This is the "program store" module for Mush.
 * It maintains a set of numbered statements, along with a "program counter"
 * that indicates the current point of execution, which is either before all
 * statements, after all statements, or in between two statements.
 * There should be no fixed limit on the number of statements that the program
 * store can hold.
 */

typedef struct prog
{
    struct prog *next;
    struct prog *prev;
    STMT *stmt;
} PROG;

PROG head;

int counter;
int length;

void initProg(PROG *head)
{
    head->next = head;
    head->prev = head;
}

/**
 * @brief  Output a listing of the current contents of the program store.
 * @details  This function outputs a listing of the current contents of the
 * program store.  Statements are listed in increasing order of their line
 * number.  The current position of the program counter is indicated by
 * a line containing only the string "-->" at the current program counter
 * position.
 *
 * @param out  The stream to which to output the listing.
 * @return  0 if successful, -1 if any error occurred.
 */
int prog_list(FILE *out)
{
    if (!head.next)
        return -1;
    PROG *current = head.next;
    int i = 1;
    if (counter < length)
    {
        while (current != &head)
        {
            show_stmt(out, current->stmt);
            if (i == counter)
                fprintf(out, "-->\n");
            current = current->next;
            i++;
        }
    }
    else
    {
        while (current != &head)
        {
            show_stmt(out, current->stmt);
            current = current->next;
            i++;
        }
        fprintf(out, "-->\n");
    }
    return 0;
}

/**
 * @brief  Insert a new statement into the program store.
 * @details  This function inserts a new statement into the program store.
 * The statement must have a line number.  If the line number is the same as
 * that of an existing statement, that statement is replaced.
 * The program store assumes the responsibility for ultimately freeing any
 * statement that is inserted using this function.
 * Insertion of new statements preserves the value of the program counter:
 * if the position of the program counter was just before a particular statement
 * before insertion of a new statement, it will still be before that statement
 * after insertion, and if the position of the program counter was after all
 * statements before insertion of a new statement, then it will still be after
 * all statements after insertion.
 *
 * @param stmt  The statement to be inserted.
 * @return  0 if successful, -1 if any error occurred.
 */
int prog_insert(STMT *stmt)
{
    if (!head.next)
    {
        initProg(&head);
    }
    PROG *insert = (PROG *)malloc(sizeof(PROG));
    insert->stmt = stmt;

    PROG *current = head.next;
    while (current != &head)
    {
        if (current->stmt->lineno == insert->stmt->lineno){
            insert->next = current->next;
            insert->prev = current->prev;
            current->next->prev = insert;
            current->prev->next = insert;
            free_stmt(current->stmt);
            free(current);
            return 0;
        }
        current = current->next;
    }

    head.prev->next = insert;
    insert->prev = head.prev;
    head.prev = insert;
    insert->next = &head;
    length++;
    counter++;
    return 0;
}

/**
 * @brief  Delete statements from the program store.
 * @details  This function deletes from the program store statements whose
 * line numbers fall in a specified range.  Any deleted statements are freed.
 * Deletion of statements preserves the value of the program counter:
 * if before deletion the program counter pointed to a position just before
 * a statement that was not among those to be deleted, then after deletion the
 * program counter will still point the position just before that same statement.
 * If before deletion the program counter pointed to a position just before
 * a statement that was among those to be deleted, then after deletion the
 * program counter will point to the first statement beyond those deleted,
 * if such a statement exists, otherwise the program counter will point to
 * the end of the program.
 *
 * @param min  Lower end of the range of line numbers to be deleted.
 * @param max  Upper end of the range of line numbers to be deleted.
 */
int prog_delete(int min, int max)
{
    if (!head.next)
        return -1;
    PROG *current = head.next;
    while (current != &head)
    {
        if (current->stmt->lineno >= min && current->stmt->lineno <= max)
        {
            PROG *tmp = current->next;
            current->prev->next = current->next;
            current->next->prev = current->prev;
            free_stmt(current->stmt);
            free(current);
            current = tmp;
            length--;
        }
        else
            current = current->next;
    }
    return 0;
}

/**
 * @brief  Reset the program counter to the beginning of the program.
 * @details  This function resets the program counter to point just
 * before the first statement in the program.
 */
void prog_reset(void)
{
    counter = 0;
}

/**
 * @brief  Fetch the next program statement.
 * @details  This function fetches and returns the first program
 * statement after the current program counter position.  The program
 * counter position is not modified.  The returned pointer should not
 * be used after any subsequent call to prog_delete that deletes the
 * statement from the program store.
 *
 * @return  The first program statement after the current program
 * counter position, if any, otherwise NULL.
 */
STMT *prog_fetch(void)
{
    if (!head.next)
        return NULL;
    PROG *current = head.next;
    int i = 1;
    while (current != &head)
    {
        if (i > counter)
        {
            return current->stmt;
        }
        current = current->next;
        i++;
    }
    return NULL;
}

/**
 * @brief  Advance the program counter to the next existing statement.
 * @details  This function advances the program counter by one statement
 * from its original position and returns the statement just after the
 * new position.  The returned pointer should not be used after any
 * subsequent call to prog_delete that deletes the statement from the
 * program store.
 *
 * @return The first program statement after the new program counter
 * position, if any, otherwise NULL.
 */
STMT *prog_next()
{
    if (!head.next)
        return NULL;
    counter++;
    PROG *current = head.next;
    int i = 1;
    while (current != &head)
    {
        if (i > counter)
        {
            return current->stmt;
        }
        current = current->next;
        i++;
    }
    return NULL;
}

/**
 * @brief  Perform a "go to" operation on the program store.
 * @details  This function performs a "go to" operation on the program
 * store, by resetting the program counter to point to the position just
 * before the statement with the specified line number.
 * The statement pointed at by the new program counter is returned.
 * If there is no statement with the specified line number, then no
 * change is made to the program counter and NULL is returned.
 * Any returned statement should only be regarded as valid as long
 * as no calls to prog_delete are made that delete that statement from
 * the program store.
 *
 * @return  The statement having the specified line number, if such a
 * statement exists, otherwise NULL.
 */
STMT *prog_goto(int lineno)
{
    if (!head.next)
        return NULL;
    PROG *current = head.next;
    int i = 0;
    while (current != &head)
    {
        if (current->stmt->lineno == lineno)
        {
            counter = i;
            return current->stmt;
        }
        current = current->next;
        i++;
    }
    return NULL;
}
