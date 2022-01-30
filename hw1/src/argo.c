#include <stdlib.h>
#include <stdio.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

int charPos;
int level;

int argo_write_object(ARGO_OBJECT *o, FILE *f);

void link(ARGO_VALUE a, ARGO_VALUE b)
{
    a.prev = &b;
    b.next = &a;
}

void charCounter(char c)
{
    argo_chars_read++;
    charPos++;
    if (c == ARGO_LF)
    {
        argo_lines_read++;
        charPos = 0;
    }
}

int argo_read_object(ARGO_OBJECT *o, FILE *f)
{
    o->member_list = argo_value_storage;
    argo_value_storage->type = ARGO_NO_TYPE;
    argo_next_value++;
    link(*(argo_value_storage + argo_next_value), *(argo_value_storage + (argo_next_value - 1)));

    char c;
    while (((c = fgetc(f)) != ARGO_RBRACE) && (c != EOF))
    {
        charCounter(c);

        if (c == ARGO_QUOTE)
        {
            (*(argo_value_storage + argo_next_value)).type = ARGO_STRING_TYPE;
            if ((argo_read_string(&((*(argo_value_storage + argo_next_value)).content.string), f)) != 0)
            {
                fprintf(stderr, "Invalid string format at [%d,%d]\n", argo_lines_read, charPos);
                return -1;
            }
            argo_next_value++;
            link(*(argo_value_storage + argo_next_value), *(argo_value_storage + (argo_next_value - 1)));
        }
        if (argo_is_digit(c) || (c == ARGO_MINUS))
        {
            if (c == ARGO_MINUS)
            {
                if (!(argo_is_digit(c = fgetc(f))))
                {
                    fprintf(stderr, "Invalid number format at [%d,%d]\n", argo_lines_read, argo_chars_read);
                    return -1;
                }
                ungetc(c, f);
            }

            (argo_value_storage + argo_next_value)->type = ARGO_NUMBER_TYPE;
            if ((argo_read_number(&((*(argo_value_storage + argo_next_value)).content.number), f)) != 0)
            {
                fprintf(stderr, "Invalid number format at [%d,%d]\n", argo_lines_read, argo_chars_read);
                return -1;
            }
            argo_next_value++;
            link(*(argo_value_storage + argo_next_value), *(argo_value_storage + (argo_next_value - 1)));
        }
    }
    if (c == ARGO_RBRACE)
        return 0;
    return -1;
}

int argo_read_array(ARGO_ARRAY *a, FILE *f)
{
    return 0;
}

int argo_read_basic(ARGO_BASIC *b, FILE *f)
{
    return 0;
}

/**
 * @brief  Read JSON input from a specified input stream, parse it,
 * and return a data structure representing the corresponding value.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON value,
 * according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  See the assignment handout for
 * information on the JSON syntax standard and how parsing can be
 * accomplished.  As discussed in the assignment handout, the returned
 * pointer must be to one of the elements of the argo_value_storage
 * array that is defined in the const.h header file.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  A valid pointer if the operation is completely successful,
 * NULL if there is any error.
 */
// ARGO_VALUE *argo_read_value(FILE *f)
// {
//     char c;
//     while ((c = fgetc(f)) != EOF)
//     {
//         charCounter(c);

//         if (c == ARGO_LBRACE)
//         {
//             ARGO_VALUE temp = *(argo_value_storage + argo_next_value);
//             (*(argo_value_storage + argo_next_value)).type = ARGO_OBJECT_TYPE;
//             argo_append_char(&((*(argo_value_storage + argo_next_value)).content.string), ARGO_LBRACE);
//             if ((argo_read_object(&((*(argo_value_storage + argo_next_value)).content.object), f)) != 0)
//             {
//                 fprintf(stderr, "Invalid object format at [%d,%d]\n", argo_lines_read, charPos);
//                 return NULL;
//             }
//             argo_append_char(&((*(argo_value_storage + argo_next_value)).content.string), ARGO_RBRACE);
//             argo_next_value++;
//             link(*(argo_value_storage + argo_next_value), temp);
//         }
//         if (argo_is_digit(c) || (c == ARGO_MINUS))
//         {
//             if (c == ARGO_MINUS)
//             {
//                 if (!(argo_is_digit(c = fgetc(f))))
//                 {
//                     fprintf(stderr, "Invalid number format at [%d,%d]\n", argo_lines_read, argo_chars_read);
//                     return NULL;
//                 }
//                 ungetc(c, f);
//             }

//             (argo_value_storage + argo_next_value)->type = ARGO_NUMBER_TYPE;
//             if ((argo_read_number(&((*(argo_value_storage + argo_next_value)).content.number), f)) != 0)
//             {
//                 fprintf(stderr, "Invalid number format at [%d,%d]\n", argo_lines_read, argo_chars_read);
//                 return NULL;
//             }
//             argo_next_value++;
//             link(*(argo_value_storage + argo_next_value), *(argo_value_storage + (argo_next_value - 1)));
//         }
//     }
//     return argo_value_storage;
// }

/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON string literal, and return a data structure
 * representing the corresponding string.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON string
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_string(ARGO_STRING *s, FILE *f)
{
    argo_append_char(s, ARGO_QUOTE);
    char c;
    while (((c = fgetc(f)) != ARGO_QUOTE) && (c != EOF))
    {
        charCounter(c);
        argo_append_char(s, c);
    }
    if (c == ARGO_QUOTE)
    {
        argo_append_char(s, ARGO_QUOTE);
        return 0;
    }
    return -1;
}

/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON number, and return a data structure representing
 * the corresponding number.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON numeric
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  The returned value must contain
 * (1) a string consisting of the actual sequence of characters read from
 * the input stream; (2) a floating point representation of the corresponding
 * value; and (3) an integer representation of the corresponding value,
 * in case the input literal did not contain any fraction or exponent parts.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_number(ARGO_NUMBER *n, FILE *f)
{
    char c;
    while (((argo_is_digit(c = fgetc(f))) || (argo_is_exponent(c)) || (c == ARGO_PERIOD)) && (c != EOF))
    {
        charCounter(c);
    }
    return 0;
}

void indent(FILE *f)
{
    if (global_options == CANONICALIZE_OPTION)
    {
        return;
    }
    else
    {
        fprintf(f, "%c", ARGO_LF);
        for (int j = 0; j <= level; j++)
        {
            for (int i = 0; i < indent_level; i++)
            {
                fprintf(f, " ");
            }
        }
    }
}

int argo_write_basic(ARGO_BASIC *b, FILE *f)
{
    switch (*b)
    {
    case (0):
        fprintf(f, "%s", ARGO_NULL_TOKEN);
        return 0;
    case (1):
        fprintf(f, "%s", ARGO_TRUE_TOKEN);
        return 0;
    case (2):
        fprintf(f, "%s", ARGO_FALSE_TOKEN);
        return 0;
    }
    return -1;
}

int argo_write_array(ARGO_ARRAY *a, FILE *f)
{
    ARGO_VALUE *head = a->element_list->next;
    while (head->type != ARGO_NO_TYPE)
    {
        if (head->type == ARGO_OBJECT_TYPE)
        {
            fprintf(f, "%c", ARGO_LBRACE);
            level++;
            indent(f);
            argo_write_object(&head->content.object, f);
            level--;
            indent(f);
            fprintf(f, "%c", ARGO_RBRACE);
        }
        if (head->type == ARGO_ARRAY_TYPE)
        {
            fprintf(f, "%c", ARGO_LBRACK);
            level++;
            indent(f);
            argo_write_array(&head->content.array, f);
            level--;
            indent(f);
            fprintf(f, "%c", ARGO_RBRACK);
        }
        if (head->type == ARGO_STRING_TYPE)
        {
            argo_write_string(&head->content.string, f);
        }
        if (head->type == ARGO_NUMBER_TYPE)
        {
            argo_write_number(&head->content.number, f);
        }
        if (head->type == ARGO_BASIC_TYPE)
        {
            argo_write_basic(&head->content.basic, f);
        }
        if (head->next->type != ARGO_NO_TYPE)
        {
            fprintf(f, "%c", ARGO_COMMA);
            indent(f);
        }
        else if (global_options != CANONICALIZE_OPTION && level == 0)
        {
            fprintf(f, "%c", ARGO_LF);
        }
        head = head->next;
    }
    return 0;
}

int argo_write_object(ARGO_OBJECT *o, FILE *f)
{
    ARGO_VALUE *head = o->member_list->next;
    while (head->type != ARGO_NO_TYPE)
    {
        if (head->type == ARGO_OBJECT_TYPE)
        {
            argo_write_string(&head->name, f);
            fprintf(f, "%c ", ARGO_COLON);

            fprintf(f, "%c", ARGO_LBRACE);
            level++;
            indent(f);
            argo_write_object(&head->content.object, f);
            level--;
            indent(f);
            fprintf(f, "%c", ARGO_RBRACE);
        }
        if (head->type == ARGO_ARRAY_TYPE)
        {
            argo_write_string(&head->name, f);
            fprintf(f, "%c ", ARGO_COLON);

            fprintf(f, "%c", ARGO_LBRACK);
            level++;
            indent(f);
            argo_write_array(&head->content.array, f);
            level--;
            indent(f);
            fprintf(f, "%c", ARGO_RBRACK);
        }
        if (head->type == ARGO_STRING_TYPE)
        {
            argo_write_string(&head->name, f);
            fprintf(f, "%c ", ARGO_COLON);

            argo_write_string(&head->content.string, f);
        }
        if (head->type == ARGO_NUMBER_TYPE)
        {
            argo_write_string(&head->name, f);
            fprintf(f, "%c ", ARGO_COLON);

            argo_write_number(&head->content.number, f);
        }
        if (head->type == ARGO_BASIC_TYPE)
        {
            argo_write_string(&head->name, f);
            fprintf(f, "%c ", ARGO_COLON);

            argo_write_basic(&head->content.basic, f);
        }
        if (head->next->type != ARGO_NO_TYPE)
        {
            fprintf(f, "%c", ARGO_COMMA);
            indent(f);
        }
        else if (global_options != CANONICALIZE_OPTION && level == 0)
        {
            fprintf(f, "%c", ARGO_LF);
        }
        head = head->next;
    }

    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified value to
 * a specified output stream.
 * @details  Write canonical JSON representing a specified value
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.
 *
 * @param v  Data structure representing a value.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_value(ARGO_VALUE *v, FILE *f)
{
    if (v->type == ARGO_OBJECT_TYPE)
    {
        fprintf(f, "%c", ARGO_LBRACE);
        indent(f);
        argo_write_object(&v->content.object, f);
        fprintf(f, "%c", ARGO_RBRACE);
        indent(f);
    }
    if (v->type == ARGO_ARRAY_TYPE)
    {
        fprintf(f, "%c", ARGO_LBRACK);
        indent(f);
        argo_write_array(&v->content.array, f);
        fprintf(f, "%c", ARGO_RBRACK);
        indent(f);
    }
    if (v->type == ARGO_NUMBER_TYPE)
    {
        argo_write_number(&v->content.number, f);
        fprintf(f, "\n");
    }
    if (v->type == ARGO_STRING_TYPE)
    {
        argo_write_string(&v->content.string, f);
        fprintf(f, "\n");
    }
    if (v->type == ARGO_BASIC_TYPE)
    {
        argo_write_basic(&v->content.basic, f);
        fprintf(f, "\n");
    }
    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified string
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified string
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument string may contain any sequence of
 * Unicode code points and the output is a JSON string literal,
 * represented using only 8-bit bytes.  Therefore, any Unicode code
 * with a value greater than or equal to U+00FF cannot appear directly
 * in the output and must be represented by an escape sequence.
 * There are other requirements on the use of escape sequences;
 * see the assignment handout for details.
 *
 * @param v  Data structure representing a string (a sequence of
 * Unicode code points).
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_string(ARGO_STRING *s, FILE *f)
{
    fprintf(f, "%c", ARGO_QUOTE);
    for (int i = 0; i < s->length; i++)
    {
        if (*(s->content + i) == 92)
        {
            fprintf(f, "\\\\");
        }
        else if (*(s->content + i) == 34)
        {
            fprintf(f, "\\\"");
        }
        else
        {
            fprintf(f, "%c", *(s->content + i));
        }
    }
    fprintf(f, "%c", ARGO_QUOTE);
    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified number
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified number
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument number may contain representations
 * of the number as any or all of: string conforming to the
 * specification for a JSON number (but not necessarily canonical),
 * integer value, or floating point value.  This function should
 * be able to work properly regardless of which subset of these
 * representations is present.
 *
 * @param v  Data structure representing a number.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_number(ARGO_NUMBER *n, FILE *f)
{
    if (n->valid_int)
    {
        fprintf(f, "%ld", n->int_value);
        return 0;
    }
    if (n->valid_float)
    {
        fprintf(f, "%1.e", n->float_value);
        return 0;
    }
    if (n->valid_string)
    {
        argo_write_string(&n->string_value, f);
        return 0;
    }
    return -1;
}
