/*
 *      Hash.c - hash table library for integer keys
 *
 *  DESCRIPTION
 *      This module contains routines for managing hash tables that use
 *      integer numbers as key.
 *
 *  INCLUDE FILES
 *      Hash.h
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
#include <string.h>
#include "List.h"
#include "Hash.h"
#include "Assert.h"     /* includes "Except.h" which defines return() macro */

#define HASH_SIZE       256     /* hash table size (power of 2 << HASH_SIZE) */

typedef struct                  /* hash table node */
{
    void *      pData;          /* pointer to the user data */
    int         key;            /* key number */
} HashNode;


/******************************************************************************
 *
 *      HashCreate - create hash table
 *
 *  DESCRIPTION
 *      This routine creates an empty hash table.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Pointer to hash table.
 */

Hash * HashCreate(void)
{
    Hash *      pHash;
    int         n;

    pHash = malloc(sizeof(Hash));

    pHash->count = 0;

    pHash->pNodeLists = malloc(HASH_SIZE * sizeof(List *));
    for (n = 0; n < HASH_SIZE; n++)
        pHash->pNodeLists[n] = ListCreate();

    return pHash;
} 


/******************************************************************************
 *
 *      HashDestroy - free hash table
 *
 *  DESCRIPTION
 *      This routine frees the memory that is occupied by the hash table.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      N/A.
 */

void HashDestroy(
    Hash *      pHash)          /* pointer to hash table */
{
    int         n;

    assert(pHash != NULL);

    for (n = 0; n < HASH_SIZE; n++)
        ListDestroyData(pHash->pNodeLists[n]);

    free(pHash->pNodeLists);
    free(pHash);
}


/******************************************************************************
 *
 *      HashDestroyData - free hash table including user data
 *
 *  DESCRIPTION
 *      This routine frees the memory that is occupied by the hash table.  The 
 *      user data in each entry is also freed using free(); the caller is 
 *      responsible that all of this user data was allocated using routines 
 *      compatible with free().
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      N/A.
 */

void HashDestroyData(
    Hash *      pHash)          /* pointer to hash table */
{
    int         n;

    assert(pHash != NULL);

    for (n = 0; n < HASH_SIZE; n++)
    {
        HashNode *      pNode;

        pNode = ListHead(pHash->pNodeLists[n]);
        while (pNode != NULL)
        {
            free(pNode->pData);

            pNode = ListNext(pHash->pNodeLists[n]);
        }

        ListDestroyData(pHash->pNodeLists[n]);
    }

    free(pHash->pNodeLists);
    free(pHash);
}


/******************************************************************************
 *
 *      HashValue - calculate hash value
 *
 *  DESCRIPTION
 *      This routine calculates the hash value of integral key <key> for
 *      the specified hash table.
 *
 *  INTERNAL
 *      We use the 'multiplication method' as described in "Introduction to
 *      Algorithms" by Thomas H. Cormen, Charles E. Leiserson, and Ronald L.
 *      Rivest, MIT Press - 7th printing 1996, page 228.
 *
 *      To avoid floating point operations and gain speed, the calculations are
 *      done with integers by scaling with a factor 2^16.  For 'A' we used the
 *      golden ration as Knuth suggested.  The modulo operation is implemented 
 *      with bitwise AND, and the division with right shift.
 *
 *      Tests showed that the used constants indeed give a nice even distri-
 *      bution of the hash values (no matter if <key> is incremented with 1, 2,
 *      4, 8 or other values).
 *
 *      The HASH_SIZE must be much lower than the scale factor.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Hash table index value.
 */

static int HashValue(
    Hash *      pHash,          /* pointer to hash table */
    unsigned    key)            /* key number */
{
    int value;

    assert(pHash != NULL);

    value = (HASH_SIZE * ((key * 40503) & 65535)) >> 16;
    
    if (value >= HASH_SIZE || value < 0)
        fprintf(stderr, "INVALID HashValue: %d\n", value);

    return value;
}


/******************************************************************************
 *
 *      HashLookup - find hash node of key
 *
 *  DESCRIPTION
 *      This routine looks up the hash node value belonging to <pKey> in the
 *      specified hash table.  If two or more nodes with the same key exist,
 *      the most recent added to the hash table is returned.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Found hash node value, or NULL if not found.
 */

void * HashLookup(
    Hash *      pHash,          /* pointer to hash table */
    int         key)            /* key number */
{
    List *      nodeList;
    HashNode *  pNode;

    assert(pHash != NULL);

    nodeList = pHash->pNodeLists[HashValue(pHash, key)];

    pNode = ListHead(nodeList);
    while (pNode != NULL)
    {
        if (pNode->key == key)
        {
            return pNode->pData;
        }

        pNode = ListNext(nodeList);
    }

    return NULL;
}


/******************************************************************************
 *
 *      HashAdd - add node to hash table
 *
 *  DESCRIPTION
 *      This routine adds a node to the specified hash table.  The node has
 *      key <key> and will store <pData>.
 *
 *      The <pData> value may not be zero (because HashLookup() uses it as
 *      'not found' return value).
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      N/A.
 */

void HashAdd(
    Hash *      pHash,          /* pointer to hash table */
    int         key,            /* key number */
    void *      pData)          /* user data to be stored */
{
    HashNode *  pNode;

    assert(pHash != NULL);
    validate(pData != 0, NOTHING);

    pNode = malloc(sizeof(HashNode));
    pNode->key   = key;
    pNode->pData = pData;

    ListAddHead(pHash->pNodeLists[HashValue(pHash, key)], pNode);
    pHash->count++;
}


/******************************************************************************
 *
 *      HashRemove - remove node from hash table
 *
 *  DESCRIPTION
 *      This routine removes the node defined by <pKey> from the specified
 *      hash table.
 *
 *      Identical keys stored in the hash table are removed in FIFO order; this
 *      means that the most recent added one is removed.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Removed node value, or NULL if not found.
 */

void * HashRemove(
    Hash *      pHash,          /* pointer to hash table */
    int         key)            /* key number */
{
    List *      nodeList;
    HashNode *  pNode;
    void *      pData;

    assert(pHash != NULL);

    nodeList = pHash->pNodeLists[HashValue(pHash, key)];

    pNode = ListHead(nodeList);
    while (pNode != NULL)
    {
        if (pNode->key == key)
        {
            ListRemoveLast(nodeList);
            pData = pNode->pData;
            free(pNode);
            pHash->count--;

            return pData;
        }

        pNode = ListNext(nodeList);
    }

    return NULL;
}


/******************************************************************************
 *
 *      HashCount - get number of nodes in hash table
 *
 *  DESCRIPTION
 *      This routine returns the number of nodes in the specified hash table.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Number of nodes in hash table.
 */

int HashCount(
    Hash *      pHash)          /* pointer to hash table */
{
    assert(pHash != NULL);

    return pHash->count;
}


/* end of Hash.c */
