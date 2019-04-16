/*
 *      Hash.h - hash table library header
 *
 *  DESCRIPTION
 *      This header belongs to "Hash.c" and must be included by every module
 *      that uses hash tables.
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

#ifndef _HASH_H
#define _HASH_H

#include "List.h"

typedef struct _Hash    Hash;   /* hash table */
struct _Hash            
{
    List **     pNodeLists;     /* node lists */
    int         count;          /* number of stored nodes */
};


extern
Hash * HashCreate(void);

extern
void HashDestroy(
    Hash *      pHash);         /* pointer to hash table */

extern
void HashDestroyData(
    Hash *      pHash);         /* pointer to hash table */

extern
void * HashLookup(
    Hash *      pHash,          /* pointer to hash table */
    int         key);           /* key number */

extern
void HashAdd(
    Hash *      pHash,          /* pointer to hash table */
    int         key,            /* key number */
    void *      pData);         /* user data to be stored */

extern
void * HashRemove(
    Hash *      pHash,          /* pointer to hash table */
    int         key);           /* key number */

extern
int HashCount(
    Hash *      pHash);         /* pointer to hash table */


#endif  /* _HASH_H */
