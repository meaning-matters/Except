/*
 *      Alloc.c - memory allocation module
 *
 *  DESCRIPTION
 *      This module contains routines that are used by the macros defined in
 *      "Alloc.h".  None of these routines must be directly called by the user.
 *
 *  INCLUDE FILES
 *      Alloc.h
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

#include <stdlib.h>
#include "Except.h"


/******************************************************************************
 *
 *      AllocCalloc - allocate a cleared chunk of memory
 *
 *  DESCRIPTION
 *      This routine invokes the calloc() C library function for allocating
 *      a chunk of cleared memory.
 *
 *  SIDE EFFECTS
 *      An EX_MEMORY exception is thrown when there's not enough memory.
 *
 *  RETURNS
 *      Pointer to allocated memory.
 */

void * AllocCalloc(
    Context *   pC,             /* pointer to thread exception context */
    int         number,         /* number of elements */
    int         size,           /* size of one element */
    char *      file,           /* name of source file where invoked */
    int         line)           /* source file line number */
{
    void *      pMem;

    pMem = calloc(number, size);
    if (pMem == NULL)
        ExceptThrow(pC, OutOfMemoryError, NULL, file, line);

    return pMem;
}


/******************************************************************************
 *
 *      AllocMalloc - allocate chunk of memory
 *
 *  DESCRIPTION
 *      This routine invokes the malloc() C library function for allocating
 *      a chunk of memory.
 *
 *  SIDE EFFECTS
 *      An EX_MEMORY exception is thrown when there's not enough memory.
 *
 *  RETURNS
 *      Pointer to allocated memory.
 */

void * AllocMalloc(
    Context *   pC,             /* pointer to thread exception context */
    int         size,           /* size of one element */
    char *      file,           /* name of source file where invoked */
    int         line)           /* source file line number */
{
    void *      pMem;

    pMem = malloc(size);
    if (pMem == NULL)
        ExceptThrow(pC, OutOfMemoryError, NULL, file, line);

    return pMem;
}


/******************************************************************************
 *
 *      AllocRealloc - change size of allocated memory chunk
 *
 *  DESCRIPTION
 *      This routine invokes the realloc() C library function for changing
 *      the size of <p>.
 *
 *  SIDE EFFECTS
 *      An EX_MEMORY exception is thrown when there's not enough memory.
 *
 *  RETURNS
 *      Pointer to allocated memory.
 */

void * AllocRealloc(
    Context *   pC,             /* pointer to thread exception context */
    void *      p,              /* pointer to original block */
    int         size,           /* size of one element */
    char *      file,           /* name of source file where invoked */
    int         line)           /* source file line number */
{
    void *      pMem;

    pMem = realloc(p, size);
    if (pMem == NULL)
        ExceptThrow(pC, OutOfMemoryError, NULL, file, line);

    return pMem;
}


/* end of Alloc.c */
