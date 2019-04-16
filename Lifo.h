/*
 *      Lifo.h - LIFO buffer library header
 *
 *  DESCRIPTION
 *      This header belongs to "Lifo.c" and must be included by every module
 *      that uses LIFO buffers.
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

#ifndef _LIFO_H
#define _LIFO_H

typedef struct                  /* LIFO buffer */
{
    void **     pObjects;       /* user object 'array' */
    int         size;           /* size of object 'array' */
    int         pointer;        /* stack pointer points to free 'array' item */
} Lifo;


extern
Lifo * LifoCreate(void);

extern
void LifoDestroy(
    Lifo *      pLifo);         /* pointer to LIFO buffer */

extern
void LifoDestroyData(
    Lifo *      pLifo);         /* pointer to LIFO buffer */

extern
void LifoPush(
    Lifo *      pLifo,          /* pointer to LIFO buffer */
    void *      pObject);       /* object being added */

extern
void * LifoPop(
    Lifo *      pLifo);         /* pointer to LIFO buffer */

extern
void * LifoPeek(
    Lifo *      pLifo,          /* pointer to LIFO buffer */
    int         number);        /* object number to get (0: top) */

extern
int LifoCount(
    Lifo *      pLifo);         /* pointer to LIFO buffer */


#endif  /* _LIFO_H */
