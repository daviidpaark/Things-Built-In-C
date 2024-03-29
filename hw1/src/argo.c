#include <stdlib.h>
#include <stdio.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

int charPos;
int level;

int argo_write_object(ARGO_OBJECT *o, FILE *f);
int argo_read_object(ARGO_OBJECT *o, FILE *f);

void link(ARGO_VALUE *a, ARGO_VALUE *b)
{
    a->prev = b;
    b->next = a;
}

void charCounter(char c)
{
    if (c == ARGO_LF)
    {
        argo_lines_read++;
        charPos = 0;
    }
    else
    {
        argo_chars_read++;
        charPos++;
    }
}

int argo_read_basic(ARGO_BASIC *b, FILE *f)
{
    char c;
    switch (c = fgetc(f))
    {
    case ('n'):
        if ((c = fgetc(f)) == 'u')
        {
            if ((c = fgetc(f)) == 'l')
            {
                if ((c = fgetc(f)) == 'l')
                {
                    *b = ARGO_NULL;
                    return 0;
                }
            }
        }
        return -1;
    case ('t'):
        if ((c = fgetc(f)) == 'r')
        {
            if ((c = fgetc(f)) == 'u')
            {
                if ((c = fgetc(f)) == 'e')
                {
                    *b = ARGO_TRUE;
                    return 0;
                }
            }
        }
        return -1;
    case ('f'):
        if ((c = fgetc(f)) == 'a')
        {
            if ((c = fgetc(f)) == 'l')
            {
                if ((c = fgetc(f)) == 's')
                {
                    if ((c = fgetc(f)) == 'e')
                    {
                        *b = ARGO_FALSE;
                        return 0;
                    }
                }
            }
        }
        return -1;
    default:
        return -1;
    }
}

int argo_read_array(ARGO_ARRAY *a, FILE *f)
{
    argo_next_value++;
    ARGO_VALUE *sent = a->element_list;
    a->element_list->type = ARGO_NO_TYPE;
    char c;
    c = fgetc(f);
    charCounter(c);
    if (c == ARGO_RBRACK)
    {
        link(sent, sent);
        return 0;
    }
    argo_next_value++;
    link((argo_value_storage + argo_next_value), sent);
    ARGO_VALUE *last = (argo_value_storage + argo_next_value);
    while (c != EOF)
    {
        charCounter(c);
        last = (argo_value_storage + argo_next_value);
        if (c == ARGO_RBRACK)
        {
            link(sent, last);
            return 0;
        }
        if (c == ARGO_LBRACK)
        {
            (*(argo_value_storage + argo_next_value)).type = ARGO_ARRAY_TYPE;
            (*(argo_value_storage + argo_next_value)).content.array.element_list = &(*(argo_value_storage + (argo_next_value + 1)));
            if ((argo_read_array(&((*(argo_value_storage + argo_next_value)).content.array), f)) != 0)
            {
                fprintf(stderr, "Invalid array format at [%d,%d]\n", argo_lines_read, charPos);
                return -1;
            }
            if (((c = fgetc(f)) == ARGO_RBRACK))
            {
                link(sent, last);
                return 0;
            }
            else
                ungetc(c, f);
            argo_next_value++;
            link((argo_value_storage + argo_next_value), last);
        }
        if (c == ARGO_LBRACE)
        {
            (*(argo_value_storage + argo_next_value)).type = ARGO_OBJECT_TYPE;
            (*(argo_value_storage + argo_next_value)).content.object.member_list = &(*(argo_value_storage + (argo_next_value + 1)));
            if ((argo_read_object(&((*(argo_value_storage + argo_next_value)).content.object), f)) != 0)
            {
                fprintf(stderr, "Invalid object format at [%d,%d]\n", argo_lines_read, charPos);
                return -1;
            }
            if (((c = fgetc(f)) == ARGO_RBRACK))
            {
                link(sent, last);
                return 0;
            }
            else
                ungetc(c, f);
            argo_next_value++;
            link((argo_value_storage + argo_next_value), last);
        }
        if (c == ARGO_QUOTE)
        {
            (*(argo_value_storage + argo_next_value)).type = ARGO_STRING_TYPE;
            if ((argo_read_string(&((*(argo_value_storage + argo_next_value)).content.string), f)) != 0)
            {
                fprintf(stderr, "Invalid string format at [%d,%d]\n", argo_lines_read, charPos);
                return -1;
            }
            if (((c = fgetc(f)) == ARGO_RBRACK))
            {
                link(sent, last);
                return 0;
            }
            else
                ungetc(c, f);
            argo_next_value++;
            link((argo_value_storage + argo_next_value), (argo_value_storage + (argo_next_value - 1)));
        }
        if (argo_is_digit(c) || (c == ARGO_MINUS))
        {
            ungetc(c, f);
            (*(argo_value_storage + argo_next_value)).type = ARGO_NUMBER_TYPE;
            if ((argo_read_number(&((*(argo_value_storage + argo_next_value)).content.number), f)) != 0)
            {
                fprintf(stderr, "Invalid number format at [%d,%d]\n", argo_lines_read, argo_chars_read);
                return -1;
            }
            if (((c = fgetc(f)) == ARGO_RBRACK))
            {
                link(sent, last);
                return 0;
            }
            else
                ungetc(c, f);
            argo_next_value++;
            link((argo_value_storage + argo_next_value), (argo_value_storage + (argo_next_value - 1)));
        }
        if ((c == 'n') || (c == 't') || (c == 'f'))
        {
            ungetc(c, f);
            (*(argo_value_storage + argo_next_value)).type = ARGO_BASIC_TYPE;
            if ((argo_read_basic(&((*(argo_value_storage + argo_next_value)).content.basic), f)) != 0)
            {
                fprintf(stderr, "Invalid basic format at [%d,%d]\n", argo_lines_read, argo_chars_read);
                return -1;
            }
            if (((c = fgetc(f)) == ARGO_RBRACK))
            {
                link(sent, last);
                return 0;
            }
            else
                ungetc(c, f);
            argo_next_value++;
            link((argo_value_storage + argo_next_value), (argo_value_storage + (argo_next_value - 1)));
        }
        c = fgetc(f);
    }
    return -1;
}

int argo_read_object(ARGO_OBJECT *o, FILE *f)
{
    ARGO_VALUE *sent = o->member_list;
    argo_next_value++;
    *(argo_value_storage + argo_next_value) = *sent;
    o->member_list->type = ARGO_NO_TYPE;
    char c;
    c = fgetc(f);
    charCounter(c);
    if (c == ARGO_RBRACE)
    {
        link(sent, sent);
        return 0;
    }
    argo_next_value++;
    link((argo_value_storage + argo_next_value), sent);
    ARGO_VALUE *last = (argo_value_storage + argo_next_value);
    while (c != EOF)
    {
        charCounter(c);
        last = (argo_value_storage + argo_next_value);
        if (c == ARGO_RBRACE)
        {
            link(sent, last);
            return 0;
        }

        if (c == ARGO_QUOTE)
        {
            if ((argo_read_string(&((*(argo_value_storage + argo_next_value)).name), f)) != 0)
            {
                fprintf(stderr, "Invalid string format at [%d,%d]\n", argo_lines_read, charPos);
                return -1;
            }
            c = fgetc(f);
            while (c != EOF)
            {
                if (c == ARGO_LBRACE)
                {
                    (*(argo_value_storage + argo_next_value)).type = ARGO_OBJECT_TYPE;
                    (*(argo_value_storage + argo_next_value)).content.object.member_list = &(*(argo_value_storage + (argo_next_value + 1)));
                    if ((argo_read_object(&((*(argo_value_storage + argo_next_value)).content.object), f)) != 0)
                    {
                        fprintf(stderr, "Invalid object format at [%d,%d]\n", argo_lines_read, charPos);
                        return -1;
                    }
                    if (((c = fgetc(f)) == ARGO_RBRACE))
                    {
                        link(sent, last);
                        return 0;
                    }
                    else
                        ungetc(c, f);
                    argo_next_value++;
                    link((argo_value_storage + argo_next_value), last);
                    break;
                }
                if (c == ARGO_LBRACK)
                {
                    (*(argo_value_storage + argo_next_value)).type = ARGO_ARRAY_TYPE;
                    (*(argo_value_storage + argo_next_value)).content.array.element_list = &(*(argo_value_storage + (argo_next_value + 1)));
                    if ((argo_read_array(&((*(argo_value_storage + argo_next_value)).content.array), f)) != 0)
                    {
                        fprintf(stderr, "Invalid array format at [%d,%d]\n", argo_lines_read, charPos);
                        return -1;
                    }
                    if (((c = fgetc(f)) == ARGO_RBRACE))
                    {
                        link(sent, last);
                        return 0;
                    }
                    else
                        ungetc(c, f);
                    argo_next_value++;
                    link((argo_value_storage + argo_next_value), last);
                    break;
                }
                if (c == ARGO_QUOTE)
                {
                    (*(argo_value_storage + argo_next_value)).type = ARGO_STRING_TYPE;
                    if ((argo_read_string(&((*(argo_value_storage + argo_next_value)).content.string), f)) != 0)
                    {
                        fprintf(stderr, "Invalid string format at [%d,%d]\n", argo_lines_read, charPos);
                        return -1;
                    }
                    c = fgetc(f);
                    while (argo_is_whitespace(c))
                        c = fgetc(f);
                    if (c == ARGO_RBRACE)
                    {
                        link(sent, last);
                        return 0;
                    }
                    else
                        ungetc(c, f);
                    argo_next_value++;
                    link((argo_value_storage + argo_next_value), (argo_value_storage + (argo_next_value - 1)));
                    break;
                }
                if (argo_is_digit(c) || (c == ARGO_MINUS))
                {
                    ungetc(c, f);
                    (*(argo_value_storage + argo_next_value)).type = ARGO_NUMBER_TYPE;
                    if ((argo_read_number(&((*(argo_value_storage + argo_next_value)).content.number), f)) != 0)
                    {
                        fprintf(stderr, "Invalid number format at [%d,%d]\n", argo_lines_read, argo_chars_read);
                        return -1;
                    }
                    c = fgetc(f);
                    while (argo_is_whitespace(c))
                        c = fgetc(f);
                    if (c == ARGO_RBRACE)
                    {
                        link(sent, last);
                        return 0;
                    }
                    else
                        ungetc(c, f);
                    argo_next_value++;
                    link((argo_value_storage + argo_next_value), (argo_value_storage + (argo_next_value - 1)));
                    break;
                }
                if ((c == 'n') || (c == 't') || (c == 'f'))
                {
                    ungetc(c, f);
                    (*(argo_value_storage + argo_next_value)).type = ARGO_BASIC_TYPE;
                    if ((argo_read_basic(&((*(argo_value_storage + argo_next_value)).content.basic), f)) != 0)
                    {
                        fprintf(stderr, "Invalid basic format at [%d,%d]\n", argo_lines_read, argo_chars_read);
                        return -1;
                    }
                    c = fgetc(f);
                    while (argo_is_whitespace(c))
                        c = fgetc(f);
                    if (c == ARGO_RBRACE)
                    {
                        link(sent, last);
                        return 0;
                    }
                    else
                        ungetc(c, f);
                    argo_next_value++;
                    link((argo_value_storage + argo_next_value), (argo_value_storage + (argo_next_value - 1)));
                    break;
                }
                c = fgetc(f);
            }
        }
        c = fgetc(f);
    }
    return -1;
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
ARGO_VALUE *argo_read_value(FILE *f)
{
    char c;
    c = fgetc(f);
    while (c != EOF)
    {
        charCounter(c);
        if (c == ARGO_LBRACE)
        {
            (*(argo_value_storage + argo_next_value)).type = ARGO_OBJECT_TYPE;
            (*(argo_value_storage + argo_next_value)).content.object.member_list = &(*(argo_value_storage + (argo_next_value + 1)));
            if ((argo_read_object(&((*(argo_value_storage + argo_next_value)).content.object), f)) != 0)
            {
                fprintf(stderr, "Invalid object format at [%d,%d]\n", argo_lines_read, charPos);
                return NULL;
            }
        }
        if (c == ARGO_LBRACK)
        {
            (*(argo_value_storage + argo_next_value)).type = ARGO_ARRAY_TYPE;
            (*(argo_value_storage + argo_next_value)).content.array.element_list = &(*(argo_value_storage + (argo_next_value + 1)));
            if ((argo_read_array(&((*(argo_value_storage + argo_next_value)).content.array), f)) != 0)
            {
                fprintf(stderr, "Invalid array format at [%d,%d]\n", argo_lines_read, charPos);
                return NULL;
            }
        }
        if (c == ARGO_QUOTE)
        {
            (*(argo_value_storage + argo_next_value)).type = ARGO_STRING_TYPE;
            if ((argo_read_string(&((*(argo_value_storage + argo_next_value)).content.string), f)) != 0)
            {
                fprintf(stderr, "Invalid string format at [%d,%d]\n", argo_lines_read, charPos);
                return NULL;
            }
        }
        if (argo_is_digit(c) || (c == ARGO_MINUS))
        {
            ungetc(c, f);
            (*(argo_value_storage + argo_next_value)).type = ARGO_NUMBER_TYPE;
            if ((argo_read_number(&((*(argo_value_storage + argo_next_value)).content.number), f)) != 0)
            {
                fprintf(stderr, "Invalid number format at [%d,%d]\n", argo_lines_read, argo_chars_read);
                return NULL;
            }
        }
        if ((c == 'n') || (c == 't') || (c == 'f'))
        {
            ungetc(c, f);
            (*(argo_value_storage + argo_next_value)).type = ARGO_BASIC_TYPE;
            if ((argo_read_basic(&((*(argo_value_storage + argo_next_value)).content.basic), f)) != 0)
            {
                fprintf(stderr, "Invalid basic format at [%d,%d]\n", argo_lines_read, argo_chars_read);
                return NULL;
            }
            c = fgetc(f);
            charCounter(c);
            if (!argo_is_whitespace(c))
            {
                fprintf(stderr, "Invalid basic format at [%d,%d]\n", argo_lines_read, argo_chars_read);
                return NULL;
            }
        }
        c = fgetc(f);
    }
    return argo_value_storage;
}

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
    char c;
    c = fgetc(f);
    while (c != EOF)
    {
        charCounter(c);
        if (c == ARGO_QUOTE)
        {
            return 0;
        }
        if (c == ARGO_BSLASH)
        {
            c = fgetc(f);
            int i = 0;
            if (c == ARGO_U)
            {
                c = fgetc(f);
                int decimal = 0;
                int base = 4096;
                while (argo_is_hex(c))
                {
                    if (c >= '0' && c <= '9')
                    {
                        decimal += ((int)c - 48) * base;
                        base /= 16;
                    }
                    else if (c >= 'A' && c <= 'F')
                    {
                        decimal += (c - 55) * base;
                        base /= 16;
                    }
                    else if (c >= 'a' && c <= 'f')
                    {
                        decimal += (c - 87) * base;
                        base /= 16;
                    }
                    c = fgetc(f);
                }
                argo_append_char(s, decimal);
                ungetc(c, f);
            }
            else if (c == ARGO_B)
            {
                argo_append_char(s, ARGO_BS);
            }
            else if (c == ARGO_F)
            {
                argo_append_char(s, ARGO_FF);
            }
            else if (c == ARGO_N)
            {
                argo_append_char(s, ARGO_LF);
            }
            else if (c == ARGO_R)
            {
                argo_append_char(s, ARGO_CR);
            }
            else if (c == ARGO_T)
            {
                argo_append_char(s, ARGO_HT);
            }
            else if (c == ARGO_QUOTE)
            {
                argo_append_char(s, ARGO_QUOTE);
            }
            else if (c == ARGO_BSLASH)
            {
                argo_append_char(s, ARGO_BSLASH);
            }
            else
            {
                return -1;
            }
        }
        else
        {
            argo_append_char(s, c);
        }
        c = fgetc(f);
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
    n->valid_string = 1;
    n->valid_int = 1;
    n->valid_float = 1;
    int sign = 1;
    int expSign = 1;
    int exponent = 0;
    long v = 0;
    double d = 0.0;
    char c;
    c = fgetc(f);
    while (c != EOF)
    {
        charCounter(c);
        if (c == ARGO_MINUS)
        {
            argo_append_char(&n->string_value, c);
            sign = -1;
            c = fgetc(f);
            charCounter(c);
        }
        if (c == ARGO_PERIOD)
        {
            argo_append_char(&n->string_value, c);
            d = (double)v;
            int i = 1;
            c = fgetc(f);
            charCounter(c);
            while (argo_is_digit(c))
            {
                argo_append_char(&n->string_value, c);
                int j = i;
                int div = 10;
                while (j > 1)
                {
                    div = div * 10;
                    j--;
                }
                d = d + ((double)(c - '0')) / div;
                i++;
                c = fgetc(f);
                charCounter(c);
            }
            if (argo_is_exponent(c))
            {
                argo_append_char(&n->string_value, c);
                c = fgetc(f);
                charCounter(c);
                if (c == ARGO_MINUS)
                {
                    argo_append_char(&n->string_value, c);
                    expSign = -1;
                    c = fgetc(f);
                    charCounter(c);
                }
                while (argo_is_digit(c))
                {
                    argo_append_char(&n->string_value, c);
                    exponent = (exponent * 10) + (c - '0');
                    c = fgetc(f);
                    charCounter(c);
                }
            }
            n->valid_float = 1;
            n->valid_int = 0;
        }
        if (argo_is_exponent(c))
        {
            argo_append_char(&n->string_value, c);
            d = (double)v;
            c = fgetc(f);
            charCounter(c);
            if (c == ARGO_MINUS)
            {
                argo_append_char(&n->string_value, c);
                expSign = -1;
                c = fgetc(f);
                charCounter(c);
            }
            while (argo_is_digit(c))
            {
                argo_append_char(&n->string_value, c);
                exponent = (exponent * 10) + (c - '0');
                c = fgetc(f);
                charCounter(c);
            }
            n->valid_float = 1;
            n->valid_int = 0;
            break;
        }
        if (!argo_is_digit(c))
        {
            ungetc(c, f);
            break;
        }
        argo_append_char(&n->string_value, c);
        v = (v * 10) + (c - '0');
        c = fgetc(f);
    }
    if (exponent > 0)
    {
        while (exponent > 0)
        {
            if (n->valid_int == 0)
            {
                if (expSign == 1)
                {
                    d = d * 10;
                    exponent--;
                }
                else
                {
                    d = d / 10;
                    exponent--;
                }
            }
            else
            {
                if (expSign == 1)
                {
                    v = v * 10;
                    d = d * 10;
                    exponent--;
                }
                else
                {
                    v = v / 10;
                    d = d * 10;
                    exponent--;
                }
            }
        }
    }
    n->float_value = d * sign;
    n->int_value = v * sign;
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
            fprintf(f, "%c", ARGO_COLON);
            if (global_options != CANONICALIZE_OPTION)
            {
                fprintf(f, "%c", ARGO_SPACE);
            }

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
            fprintf(f, "%c", ARGO_COLON);
            if (global_options != CANONICALIZE_OPTION)
            {
                fprintf(f, "%c", ARGO_SPACE);
            }

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
            fprintf(f, "%c", ARGO_COLON);
            if (global_options != CANONICALIZE_OPTION)
            {
                fprintf(f, "%c", ARGO_SPACE);
            }

            argo_write_string(&head->content.string, f);
        }
        if (head->type == ARGO_NUMBER_TYPE)
        {
            argo_write_string(&head->name, f);
            fprintf(f, "%c", ARGO_COLON);
            if (global_options != CANONICALIZE_OPTION)
            {
                fprintf(f, "%c", ARGO_SPACE);
            }

            argo_write_number(&head->content.number, f);
        }
        if (head->type == ARGO_BASIC_TYPE)
        {
            argo_write_string(&head->name, f);
            fprintf(f, "%c", ARGO_COLON);
            if (global_options != CANONICALIZE_OPTION)
            {
                fprintf(f, "%c", ARGO_SPACE);
            }

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
    }
    if (v->type == ARGO_ARRAY_TYPE)
    {
        fprintf(f, "%c", ARGO_LBRACK);
        indent(f);
        argo_write_array(&v->content.array, f);
        fprintf(f, "%c", ARGO_RBRACK);
    }
    if (v->type == ARGO_NUMBER_TYPE)
    {
        argo_write_number(&v->content.number, f);
    }
    if (v->type == ARGO_STRING_TYPE)
    {
        argo_write_string(&v->content.string, f);
    }
    if (v->type == ARGO_BASIC_TYPE)
    {
        argo_write_basic(&v->content.basic, f);
    }
    if (global_options != CANONICALIZE_OPTION)
    {
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
        if (*(s->content + i) < 32 || *(s->content + i) > 255)
        {
            if (*(s->content + i) == ARGO_BS)
            {
                fprintf(f, "\\b");
            }
            else if (*(s->content + i) == ARGO_FF)
            {
                fprintf(f, "\\f");
            }
            else if (*(s->content + i) == ARGO_LF)
            {
                fprintf(f, "\\n");
            }
            else if (*(s->content + i) == ARGO_CR)
            {
                fprintf(f, "\\r");
            }
            else if (*(s->content + i) == ARGO_HT)
            {
                fprintf(f, "\\t");
            }
            else
            {
                fprintf(f, "\\u%04x", *(s->content + i));
            }
        }
        else if (*(s->content + i) == 92)
        {
            fprintf(f, "\\\\");
        }
        else if (*(s->content + i) == 34)
        {
            fprintf(f, "\\\"");
        }
        else if (*(s->content + i) >= 32)
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
        if (n->int_value <= __LONG_MAX__)
        {
            fprintf(f, "%ld", n->int_value);
            return 0;
        }
        double value = n->int_value;
        int exponent = 0;
        if (value > 0)
        {
            if (value > 1)
            {
                while (value >= 1)
                {
                    exponent++;
                    value = value / 10;
                }
                fprintf(f, "%.*fe%d", ARGO_PRECISION, value, exponent);
                return 0;
            }
            else if (value < 1)
            {
                while (value <= 0.1)
                {
                    exponent++;
                    value = value * 10;
                }
                if (exponent == 1)
                {
                    fprintf(f, "%.*f", ARGO_PRECISION, n->float_value);
                    return 0;
                }
                fprintf(f, "%.*fe-%d", ARGO_PRECISION, value, exponent);
                return 0;
            }
            fprintf(f, "1.0");
            return 0;
        }
        if (value < 0)
        {
            if (value < -1)
            {
                while (value <= -1)
                {
                    exponent++;
                    value = value / 10;
                }
                fprintf(f, "%.*fe%d", ARGO_PRECISION, value, exponent);
                return 0;
            }
            else if (value > -1)
            {
                while (value >= -0.1)
                {
                    exponent++;
                    value = value * 10;
                }
                if (exponent == 1)
                {
                    fprintf(f, "%.*f", ARGO_PRECISION, n->float_value);
                    return 0;
                }
                fprintf(f, "%.*fe-%d", ARGO_PRECISION, value, exponent);
                return 0;
            }
            fprintf(f, "-1.0");
            return 0;
        }
        else
        {
            if (value == 0.0)
            {
                fprintf(f, "0.0");
                return 0;
            }
        }
    }
    if (n->valid_float)
    {
        double value = n->float_value;
        int exponent = 0;
        if (value > 0)
        {
            if (value > 1)
            {
                while (value >= 1)
                {
                    exponent++;
                    value = value / 10;
                }
                fprintf(f, "%.*fe%d", ARGO_PRECISION, value, exponent);
                return 0;
            }
            else if (value < 1)
            {
                while (value < 0.1)
                {
                    exponent++;
                    value = value * 10;
                }
                if (exponent == 1)
                {
                    fprintf(f, "%.*f", ARGO_PRECISION, n->float_value);
                    return 0;
                }
                fprintf(f, "%.*fe-%d", ARGO_PRECISION, value, exponent);
                return 0;
            }
            fprintf(f, "1.0");
            return 0;
        }
        if (value < 0)
        {
            if (value < -1)
            {
                while (value <= -1)
                {
                    exponent++;
                    value = value / 10;
                }
                fprintf(f, "%.*fe%d", ARGO_PRECISION, value, exponent);
                return 0;
            }
            else if (value > -1)
            {
                while (value >= -0.1)
                {
                    exponent++;
                    value = value * 10;
                }
                if (exponent == 1)
                {
                    fprintf(f, "%.*f", ARGO_PRECISION, n->float_value);
                    return 0;
                }
                fprintf(f, "%.*fe-%d", ARGO_PRECISION, value, exponent);
                return 0;
            }
            fprintf(f, "-1.0");
            return 0;
        }
        else
        {
            if (value == 0.0)
            {
                fprintf(f, "0.0");
                return 0;
            }
        }
    }
    if (n->valid_string)
    {
        argo_write_string(&n->string_value, f);
        return 0;
    }
    return -1;
}
