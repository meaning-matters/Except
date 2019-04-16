/*
 *      Alloc.h - memory allocation module header
 *
 *  DESCRIPTION
 *      This header contains macros that replace the standard C memory 
 *      allocation routines.  They throw an exception when there is not
 *      enough memory.  The macros use functions defined in "Alloc.c".
 *      The handy new() macro is added; it only needs the type name.
 *
 *      Using macros allows the file name and line number information supplied
 *      by the preprocessor, to be available for error reporting.
 *
 *  COPYRIGHT
 *      You are free to use, copy or modify this software at your own risk.
 *
 *  AUTHOR
 *      Cornelis van der Bent.  Please let me know if you have comments or find 
 *      flaws: cg_vanderbent@mail.com.  Enjoy!
 *
 *  MODIFICATION HISTORY
 *      1999/04/12 vdbent       Thorough test and debugging; beta release.
 *      1998/12/18 vdbent       Conception.
 */

#ifndef _ALLOC_H
#define _ALLOC_H

#include <stdlib.h>
#include "Except.h"


#define new(type)       AllocCalloc(pC, 1, sizeof(type), __FILE__, __LINE__)

#define calloc(n, size) AllocCalloc(pC, n, size, __FILE__, __LINE__)

#define malloc(size)    AllocMalloc(pC, size, __FILE__, __LINE__)

#define realloc(p,size) AllocRealloc(pC, p, size, __FILE__, __LINE__)


extern
void * AllocCalloc(
    Context *   pC,             /* pointer to thread exception context */
    int         number,         /* number of elements */
    int         size,           /* size of one element */
    char *      file,           /* name of source file where invoked */
    int         line);          /* source file line number */

extern
void * AllocMalloc(
    Context *   pC,             /* pointer to thread exception context */
    int         size,           /* size of one element */
    char *      file,           /* name of source file where invoked */
    int         line);          /* source file line number */

extern
void * AllocRealloc(
    Context *   pC,             /* pointer to thread exception context */
    void *      p,              /* pointer to original block */
    int         size,           /* size of one element */
    char *      file,           /* name of source file where invoked */
    int         line);          /* source file line number */


#endif  /* _ALLOC_H */
