/*********************/
/* par.c             */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


#include "errmsg.h"
#include "buffer.h"    /* Also includes <stddef.h>. */
#include "reformat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <getopt.h>

#undef NULL
#define NULL ((void *) 0)


const char * const progname = "par";
const char * const version = "3.20";

static int digtoint(char c)

/* Returns the value represented by the digit c,   */
/* or -1 if c is not a digit. Does not use errmsg. */
{
  return c == '0' ? 0 :
         c == '1' ? 1 :
         c == '2' ? 2 :
         c == '3' ? 3 :
         c == '4' ? 4 :
         c == '5' ? 5 :
         c == '6' ? 6 :
         c == '7' ? 7 :
         c == '8' ? 8 :
         c == '9' ? 9 :
         -1;

  /* We can't simply return c - '0' because this is ANSI  */
  /* C code, so it has to work for any character set, not */
  /* just ones which put the digits together in order.    */
}


static int strtoudec(const char *s, int *pn)

/* Puts the decimal value of the string s into *pn, returning */
/* 1 on success. If s is empty, or contains non-digits,       */
/* or represents an integer greater than 9999, then *pn       */
/* is not changed and 0 is returned. Does not use errmsg.     */
{
  int n = 0;

  if (!*s) return 0;

  do {
    if (n >= 1000 || !isdigit(*s)) return 0;
    n = 10 * n + digtoint(*s);
  } while (*++s);

  *pn = n;

  return 1;
}


static void parseopt(
  const char *opt, int *pwidth, int *pprefix,
  int *psuffix, int *phang, int *plast, int *pmin
)
/* Parses the single option in opt, setting *pwidth, *pprefix,     */
/* *psuffix, *phang, *plast, or *pmin as appropriate. Uses errmsg. */
{
  const char *saveopt = opt;
  char oc;
  int n, r;
  char * errmsg = (char *)malloc(200);

  if (*opt == '-') ++opt;

  if (!strcmp(opt, "version")) {
    sprintf(errmsg, "%s %s\n", progname, version);
    set_error(errmsg);
    free(errmsg);
    return;
  }

  oc = *opt;

  if (isdigit(oc)) {
    if (!strtoudec(opt, &n)) goto badopt;
    if (n <= 8) *pprefix = n;
    else *pwidth = n;
  }
  else {
    if (!oc) goto badopt;
    n = 1;
    r = strtoudec(opt + 1, &n);
    if (opt[1] && !r) goto badopt;

    if (oc == 'w' || oc == 'p' || oc == 's') {
      if (!r) goto badopt;
      if      (oc == 'w') *pwidth  = n;
      else if (oc == 'p') *pprefix = n;
      else                *psuffix = n;
    }
    else if (oc == 'h') *phang = n;
    else if (n <= 1) {
      if      (oc == 'l') *plast = n;
      else if (oc == 'm') *pmin = n;
    }
    else goto badopt;
  }

  clear_error();
  free(errmsg);
  return;

badopt:
  sprintf(errmsg, "Bad option: %.149s\n", saveopt);
  set_error(errmsg);
  free(errmsg);
}


static char **readlines(void)

/* Reads lines from stdin until EOF, or until a blank line is encountered, */
/* in which case the newline is pushed back onto the input stream. Returns */
/* a NULL-terminated array of pointers to individual lines, stripped of    */
/* their newline characters. Uses errmsg, and returns NULL on failure.     */
{
  struct buffer *cbuf = NULL, *pbuf = NULL;
  int c, blank;
  char ch, *ln, *nullline = NULL, nullchar = '\0', **lines = NULL;

  cbuf = newbuffer(sizeof (char));
  if (is_error()) goto rlcleanup;
  pbuf = newbuffer(sizeof (char *));
  if (is_error()) goto rlcleanup;

  for (blank = 1;  ; ) {
    c = getchar();
    if (c == EOF) break;
    if (c == '\n') {
      if (blank) {
        ungetc(c,stdin);
        break;
      }
      additem(cbuf, &nullchar);
      if (is_error()) goto rlcleanup;
      ln = copyitems(cbuf);
      if (is_error()) goto rlcleanup;
      additem(pbuf, &ln);
      if (is_error()) goto rlcleanup;
      clearbuffer(cbuf);
      blank = 1;
    }
    else {
      if (!isspace(c)) blank = 0;
      ch = c;
      additem(cbuf, &ch);
      if (is_error()) goto rlcleanup;
    }
  }

  if (!blank) {
    additem(cbuf, &nullchar);
    if (is_error()) goto rlcleanup;
    ln = copyitems(cbuf);
    if (is_error()) goto rlcleanup;
    additem(pbuf, &ln);
    if (is_error()) goto rlcleanup;
  }

  additem(pbuf, &nullline);
  if (is_error()) goto rlcleanup;
  lines = copyitems(pbuf);

rlcleanup:

  if (cbuf) freebuffer(cbuf);
  if (pbuf) {
    if (!lines)
      for (;;) {
        lines = nextitem(pbuf);
        if (!lines) break;
        free(*lines);
      }
    freebuffer(pbuf);
  }

  return lines;
}


static void setdefaults(
  const char * const *inlines, int *pwidth, int *pprefix,
  int *psuffix, int *phang, int *plast, int *pmin
)
/* If any of *pwidth, *pprefix, *psuffix, *phang, *plast, *pmin are     */
/* less than 0, sets them to default values based on inlines, according */
/* to "par.doc". Does not use errmsg because it always succeeds.        */
{
  int numlines;
  const char *start, *end, * const *line, *p1, *p2;

  if (*pwidth < 0) *pwidth = 72;
  if (*phang < 0) *phang = 0;
  if (*plast < 0) *plast = 0;
  if (*pmin < 0) *pmin = *plast;

  for (line = inlines;  *line;  ++line);
  numlines = line - inlines;

  if (*pprefix < 0) {
    if (numlines <= *phang + 1)
      *pprefix = 0;
    else {
      start = inlines[*phang];
      for (end = start;  *end;  ++end);
      for (line = inlines + *phang + 1;  *line;  ++line) {
        for (p1 = start, p2 = *line;  p1 < end && *p1 == *p2;  ++p1, ++p2);
        end = p1;
      }
      *pprefix = end - start;
    }
  }

  if (*psuffix < 0) {
    if (numlines <= 1)
      *psuffix = 0;
    else {
      start = *inlines;
      for (end = start;  *end;  ++end);
      for (line = inlines + 1;  *line;  ++line) {
        for (p2 = *line;  *p2;  ++p2);
        for (p1 = end;
             p1 > start && p2 > *line && p1[-1] == p2[-1];
             --p1, --p2);
        start = p1;
      }
      while (end - start >= 2 && isspace(*start) && isspace(start[1])) ++start;
      *psuffix = end - start;
    }
  }
}


static void freelines(char **lines)
/* Frees the strings pointed to in the NULL-terminated array lines, then */
/* frees the array. Does not use errmsg because it always succeeds.      */
{
  char **line;

  for (line = lines;  *line;  ++line)
    free(*line);

  free(lines);
}


int original_main(int argc, char * const *argv)
{
  int width, widthbak = -1, prefix, prefixbak = -1, suffix, suffixbak = -1,
      hang, hangbak = -1, last, lastbak = -1, min, minbak = -1, c;
  char *parinit, *picopy = NULL, *opt, **inlines = NULL, **outlines = NULL,
       **line;
  const char * const whitechars = " \f\n\r\t\v";

  const char * const shortForm = "w:p:s:hlm";
  const struct option longForm[] = 
  {
    { "version", 0, NULL, 'v' },
    { "width",   1, NULL, 'w' },
    { "prefix",  1, NULL, 'p' },
    { "suffix",  1, NULL, 's' },
    { "hang",    1, NULL, 'h' },
    { "last",    0, NULL, 'L' },
    { "min",     0, NULL, 'M' },
    { "no-last", 0, NULL, 'K' },
    { "no-min",  0, NULL, 'N' },
    {  0,        0, 0,     0  }
  };
  int args;
  char * errmsg = (char *)malloc(200);

  parinit = getenv("PARINIT");
  if (parinit) {
    picopy = malloc((strlen(parinit) + 1) * sizeof (char));
    if (!picopy) {
      set_error(outofmem);
      goto parcleanup;
    }
    strcpy(picopy,parinit);
    opt = strtok(picopy,whitechars);
    while (opt) {
      parseopt(opt, &widthbak, &prefixbak,
               &suffixbak, &hangbak, &lastbak, &minbak);
      if (is_error()) goto parcleanup;
      opt = strtok(NULL,whitechars);
    }
    free(picopy);
    picopy = NULL;
  }

  do {
    args = getopt_long(argc, argv, shortForm, longForm, NULL);
    if (optarg) {
      if (args == 'l' || args == 'm') continue;
      else if (atoi(optarg) == 0) {
        sprintf(errmsg ,"bin/par: unrecognized argument '%s' for option '-%c'\n", optarg, args);
        set_error(errmsg);
        goto parcleanup;
      }
    }
    switch (args)
    {
      case 'w':
        widthbak = atoi(optarg);
        break;
      case 'p':
        prefixbak = atoi(optarg);
        break;
      case 's':
        suffixbak = atoi(optarg);
        break;
      case 'h':
        if (optarg) {
          hangbak = atoi(optarg);
        }
        else hangbak = 1;
        break;
      case 'l':
        if (optarg) {
          lastbak = atoi(optarg);
        }
        else lastbak = 1;
        break;
      case 'm':
        if (optarg) {
          minbak = atoi(optarg);
        }
        else minbak = 1;
        break;
      case 'L':
        lastbak = 1;
        break;
      case 'M':
        minbak = 1;
        break;
      case 'K':
        lastbak = 0;
        break;
      case 'N':
        minbak = 0;
        break;
      case 'v':
        sprintf(errmsg, "%s %s\n", progname, version);
        fprintf(stdout, "%s", errmsg);
        goto parcleanup;
      case '?':
        goto parcleanup;
    }
  } while (args != -1);

  if (optind < argc) {
    while (*++argv) { 
      int n = atoi(*argv);
      if (n <= 8) prefixbak = n;
      else if (n >= 9) widthbak = n;
    }
  }

  for (;;) {
    for (;;) {
      c = getchar();
      if (c == EOF) goto parcleanup;
      if (c != '\n') break;
      putchar(c);
    }
    ungetc(c,stdin);

    inlines = readlines();
    if (is_error()) goto parcleanup;
    if (!*inlines) {
      free(inlines);
      inlines = NULL;
      continue;
    }

    width = widthbak;  prefix = prefixbak;  suffix = suffixbak;
    hang = hangbak;  last = lastbak;  min = minbak;
    setdefaults((const char * const *) inlines,
                &width, &prefix, &suffix, &hang, &last, &min);

    outlines = reformat((const char * const *) inlines,
                        width, prefix, suffix, hang, last, min);
    if (is_error()) goto parcleanup;

    freelines(inlines);
    inlines = NULL;

    for (line = outlines;  *line;  ++line)
      puts(*line);

    freelines(outlines);
    outlines = NULL;
  }

parcleanup:

  if (picopy) free(picopy);
  if (inlines) freelines(inlines);
  if (outlines) freelines(outlines);
  if (errmsg) free(errmsg);

  if (is_error()) {
    report_error(stderr);
    clear_error();
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
