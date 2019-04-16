/*
 *      Lifo.c - LIFO buffer library
 *
 *  DESCRIPTION
 *      This module contains routines for managing LIFO buffers (i.e., stacks).
 *      The size of a LIFO buffer is increased automatically when needed, so
 *      it never becomes full.  The size is however never decreased.
 *
 *  INCLUDE FILES
 *      Lifo.h
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
#include "Lifo.h"
#include "Assert.h"     /* includes "Except.h" which defines return() macro */

#define INIT_SIZE       32      /* initial size */
#define INCR_SIZE       32      /* size increment when not enough space */


/******************************************************************************
 *
 *      LifoCreate - create LIFO buffer of unlimited size
 *
 *  DESCRIPTION
 *      This routine creates an empty LIFO buffer.  The size is increased when
 *      needed.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Pointer to LIFO buffer.
 */

Lifo * LifoCreate(void)
{
    Lifo *      pLifo;

    pLifo = malloc(sizeof(Lifo));
    pLifo->pObjects = malloc(INIT_SIZE * sizeof(void *));
    pLifo->size = INIT_SIZE;
    pLifo->pointer = 0;
    return pLifo;
}


/******************************************************************************
 *
 *      LifoDestroy - free LIFO buffer
 *
 *  DESCRIPTION
 *      This routine frees the memory that is occupied by the LIFO buffer.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      N/A.
 */

void LifoDestroy(
    Lifo *      pLifo)          /* pointer to LIFO buffer */
{
    assert(pLifo != NULL);

    free(pLifo->pObjects);
    free(pLifo);
}


/******************************************************************************
 *
 *      LifoDestroyData - free LIFO buffer including user data
 *
 *  DESCRIPTION
 *      This routine frees the memory that is occupied by the LIFO buffer.  The 
 *      user data objects are also freed using free(); the caller is responsi-
 *      ble that all objects were allocated using routines compatible with 
 *      free().
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      N/A.
 */

void LifoDestroyData(
    Lifo *      pLifo)          /* pointer to LIFO buffer */
{
    assert(pLifo != NULL);

    while(pLifo->pointer > 0)
        free(pLifo->pObjects[pLifo->pointer--]);

    free(pLifo->pObjects);
    free(pLifo);
}


/******************************************************************************
 *
 *      LifoPush - push object to LIFO buffer
 *
 *  DESCRIPTION
 *      This routine adds an object to the top of specified LIFO buffer.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      N/A.
 */

void LifoPush(
    Lifo *      pLifo,          /* pointer to LIFO buffer */
    void *      pObject)        /* object being added */
{
    assert(pLifo != NULL && pObject != NULL);

    if (pLifo->pointer == pLifo->size) {
        pLifo->size += INCR_SIZE;
        pLifo->pObjects = realloc(pLifo->pObjects,
                                  pLifo->size * sizeof(void *));
    }
    pLifo->pObjects[pLifo->pointer++] = pObject;
}


/******************************************************************************
 *
 *      LifoPop - pop object from LIFO buffer
 *
 *  DESCRIPTION
 *      This routine removes an object from the top of the specified LIFO
 *      buffer.
 *
 *      It is illegal to perform this operation on an empty LIFO buffer (will
 *      result in failed assertion when DEBUG defined, otherwise returns NULL).
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Removed object, or NULL if LIFO buffer is empty.
 */

void * LifoPop(
    Lifo *      pLifo)          /* pointer to LIFO buffer */
{
    assert(pLifo != NULL);
    validate(pLifo->pointer > 0, NULL);

    return pLifo->pObjects[--pLifo->pointer];
}


/******************************************************************************
 *
 *      LifoPeek - get specified LIFO buffer object
 *
 *  DESCRIPTION
 *      This routine returns the N-th object from the specified LIFO buffer
 *      counting from the top (i.e., the first/top object is retrieved with
 *      number equal to 1).  The object is not removed.
 *
 *      It is illegal to perform this operation on an empty LIFO buffer (will
 *      result in failed assertion when DEBUG defined, otherwise returns NULL).
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Specified object, or NULL if LIFO buffer is empty.
 */

void * LifoPeek(
    Lifo *      pLifo,          /* pointer to LIFO buffer */
    int         number)         /* object number to get (1: top) */
{
    assert(pLifo != NULL);
    validate(pLifo->pointer > 0, NULL);
    validate(number > 0 && number <= pLifo->pointer, NULL);

    return pLifo->pObjects[pLifo->pointer - number];
}


/******************************************************************************
 *
 *      LifoCount - get number of objects in LIFO buffer
 *
 *  DESCRIPTION
 *      This routine returns the number of objects in the specified LIFO
 *      buffer.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Number of objects in LIFO buffer.
 */

int LifoCount(
    Lifo *      pLifo)          /* pointer to LIFO buffer */
{
    assert(pLifo != NULL);

    return pLifo->pointer;
}


/* end of Lifo.c */
