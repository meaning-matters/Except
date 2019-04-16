    /*
     *      List.c - doubly linked list library
     *
     *  DESCRIPTION
     *      This module contains routines to create and maintain doubly linked
     *      lists of data objects.  A list can be used for storing data object
     *      pointers or integer values (except zero).
     *
     *      The application using this library only has to deal with a single list
     *      pointer and its data objects.  It does not have to deal with list nodes
     *      as is often seen, neither do the the data objects need to supply space
     *      for list pointers (often called <pNext> and <pPrev>).  This list type
     *      is generally called to be 'non-intrusive'.
     *      The price paid for this convenience is that nodes can not be accessed
     *      randomly, which means that deleting a node may require a linear search.
     *      The list does however keeps a pointer to the last accessed list node;
     *      the supplied set of operations relative to this node still makes this
     *      kind of list very useful for many applications without (much) perfor-
     *      mance loss compared to 'more traditional' linked lists in C.
     *
     *  NOTES
     *      Doing something that is not allowed, or entering a condition that is
     *      regarded as an error, will result in a 'failed assertion', when this
     *      module has been built with DEBUG defined.  The routine descriptions
     *      tell what to watch out for.
     *
     *  INTERNAL
     *      The idea of using a dummy node was taken from "Obfuscated C and Other
     *      Mysteries" by Don Libes, John Wiley & Sons - 1993, chapter 11.  It
     *      results in simpler list operation code.
     *
     *                                              down-->           <--up
     *                                             after-->           <--before
     *
     *                                   +-------------------- - --------------+
     *                                   |                                     |
     *      +--------------+             V         Head Node        Tail Node  |
     *      |              |         +-------+     +-------+        +-------+  |
     *      |        pHead---------->|/pNext------>| pNext---- - -->| pNext----+
     *      |              |         |///////|     |       |        |       |
     *      |    pNodeLast--->?   +----pPrev/|<------pPrev |<- - -----pPrev |
     *      |              |      |  |///////|     |       |        |       |
     *      |        count |      |  |/pData/|     | pData |     +->| pData |
     *      |              |      |  +-- | --+     +-- | --+     |  +-- | --+
     *      +--------------+      |      |             |         |      |
     *                            |      V             V         |      V
     *                            |     ###           ###        |     ###
     *        //// = Dummy Node   |     ###           ###        |     ###
     *                            |                              |
     *         ### = User Data    +--------------------------- - +
     *
     *      Notice that pList->pHead->pPrev points to the tail of the list; this
     *      is used a number of times in the code below.
     *
     *      For efficiency some short code fragments show up a number of times
     *      in different routines, instead of nesting the routines.
     *
     *  INCLUDE FILES
     *      List.h
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
     *      1999/03/09 kees         Composed.
     *
     *****************************************************************************/

    #include <stdlib.h>
    #include "List.h"
    #include "Assert.h"     /* includes "Except.h" which defines return() macro */


    /******************************************************************************
     *
     *      ListCreate - create empty list
     *
     *  DESCRIPTION
     *      This routine creates an empty list.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Pointer to empty list.
     */

    List * ListCreate(void)
    {
        List *      pList;

        pList = malloc(sizeof(List));

        pList->pHead = malloc(sizeof(ListNode));

        pList->pHead->pNext = pList->pHead->pPrev = pList->pHead;

        pList->pNodeLast = NULL;
        pList->count = 0;

        return pList;
    }


    /******************************************************************************
     *
     *      ListDestroy - free list but not user data
     *
     *  DESCRIPTION
     *      This routine frees the list handle and the nodes, but does not free 
     *      the user data.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      N/A.
     */

    void ListDestroy(
        List *      pList)          /* pointer to list */
    {
        ListNode *  pNode;

        assert(pList != NULL);

        pNode = pList->pHead->pNext;
        while (pNode != pList->pHead)
        {
            ListNode *      pNext;

            pNext = pNode->pNext;
            free(pNode);
            pNode = pNext;
        }

        free(pList->pHead);
        free(pList);
    }


    /******************************************************************************
     *
     *      ListDestroyData - free list including user data
     *
     *  DESCRIPTION
     *      This routine frees the list handle and the nodes, and does also free 
     *      the user data using free(); the caller is responsible that all of this 
     *      user data was allocated with routines compatible with free().
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      N/A.
     */

    void ListDestroyData(
        List *      pList)          /* pointer to list */
    {
        ListNode *  pNode;

        assert(pList != NULL);

        pNode = pList->pHead->pNext;
        while (pNode != pList->pHead)
        {
            ListNode *      pNext;

            pNext = pNode->pNext;
            free(pNode->pData);
            free(pNode);
            pNode = pNext;
        }

        free(pList->pHead);
        free(pList);
    }


    /******************************************************************************
     *
     *      ListAddHead - add node to head of list
     *
     *  DESCRIPTION
     *      This routine adds the specified data object value at the head of the 
     *      specified list.  The last accessed list node is set to the added
     *      node.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      N/A.
     */

    void ListAddHead(
        List *      pList,          /* pointer to list */
        void *      pData)          /* data object value */
    {
        ListNode *  pNode;

        assert(pList != NULL);

        pNode = malloc(sizeof(ListNode));

        pNode->pData = pData;
        (pNode->pNext = pList->pHead->pNext)->pPrev = pNode;
        (pList->pHead->pNext = pNode)->pPrev = pList->pHead;

        pList->pNodeLast = pNode;
        pList->count++;
    }


    /******************************************************************************
     *
     *      ListAddTail - add node to tail of list
     *
     *  DESCRIPTION
     *      This routine adds the specified data object value at the tail of the 
     *      specified list.  The last accessed list node is set to the added
     *      node.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      N/A.
     */

    void ListAddTail(
        List *      pList,          /* pointer to list */
        void *      pData)          /* data object value */
    {
        ListNode *  pNode;

        assert(pList != NULL);

        pNode = malloc(sizeof(ListNode));

        pNode->pData = pData;
        (pNode->pPrev = pList->pHead->pPrev)->pNext = pNode;
        (pList->pHead->pPrev = pNode)->pNext = pList->pHead;

        pList->pNodeLast = pNode;
        pList->count++;
    }


    /******************************************************************************
     *
     *      ListAddBefore - add node before last accessed node
     *
     *  DESCRIPTION
     *      This routine adds the specified data object value in the specified
     *      list just before the node that was last accessed by one of the 
     *      routines from this library that set it.  'Before' means towards the
     *      head of the list.  The last accessed list node is set to the added
     *      node.
     *
     *      Nothing happens when the last accessed list node is not set; this
     *      causes a failed assertion when DEBUG defined.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      N/A.
     */

    void ListAddBefore(
        List *      pList,          /* pointer to list */
        void *      pData)          /* data object value */
    {
        ListNode *  pNode;

        assert(pList != NULL);
        validate(pList->pNodeLast != NULL, NOTHING);

        pNode = malloc(sizeof(ListNode));

        pNode->pData = pData;
        (pNode->pPrev = pList->pNodeLast->pPrev)->pNext = pNode;
        (pList->pNodeLast->pPrev = pNode)->pNext = pList->pNodeLast;

        pList->pNodeLast = pNode;
        pList->count++;
    }


    /******************************************************************************
     *
     *      ListAddAfter - add node after last accessed node
     *
     *  DESCRIPTION
     *      This routine adds the specified data object value in the specified
     *      list right after the node that was last accessed by one of the
     *      routines from this library that set it.  'After' means towards the
     *      tail of the list.  The last accessed list node is set to the added
     *      node.
     *
     *      Nothing happens when the last accessed list node is not set; this
     *      causes a failed assertion when DEBUG defined.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      N/A.
     */

    void ListAddAfter(
        List *      pList,          /* pointer to list */
        void *      pData)          /* data object value */
    {
        ListNode *  pNode;

        assert(pList != NULL);
        validate(pList->pNodeLast != NULL, NOTHING);

        pNode = malloc(sizeof(ListNode));

        pNode->pData = pData;
        (pNode->pNext = pList->pNodeLast->pNext)->pPrev = pNode;
        (pList->pNodeLast->pNext = pNode)->pPrev = pList->pNodeLast;

        pList->pNodeLast = pNode;
        pList->count++;
    }


    /******************************************************************************
     *
     *      ListRemoveHead - remove head node from list
     *
     *  DESCRIPTION
     *      This routine removes the head list node from the specified list.  The
     *      last accessed list node is reset.
     *
     *      It is not allowed to pass this routine an empty list.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Removed data object value.
     */

    void * ListRemoveHead(
        List *      pList)          /* pointer to list */
    {
        void *      pData;
        ListNode *  pNode;

        assert(pList != NULL);
        validate(pList->count > 0, NULL);

        pNode = pList->pHead->pNext;

        pData = pNode->pData;
        (pList->pHead->pNext = pNode->pNext)->pPrev = pList->pHead;
        free(pNode);

        pList->pNodeLast = NULL;
        pList->count--;

        return pData;
    }


    /******************************************************************************
     *
     *      ListRemoveTail - remove tail node from list
     *
     *  DESCRIPTION
     *      This routine removes the tail list node from the specified list.  The
     *      last accessed list node is reset.
     *
     *      It is not allowed to pass this routine an empty list.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Removed data object value.
     */

    void * ListRemoveTail(
        List *      pList)          /* pointer to list */
    {
        void *      pData;
        ListNode *  pNode;

        assert(pList != NULL);
        validate(pList->count > 0, NULL);

        pNode = pList->pHead->pPrev;

        pData = pNode->pData;
        (pList->pHead->pPrev = pNode->pPrev)->pNext = pList->pHead;
        free(pNode);

        pList->pNodeLast = NULL;
        pList->count--;

        return pData;
    }


    /******************************************************************************
     *
     *      ListRemove - remove specified node from list
     *
     *  DESCRIPTION
     *      This routine removes the node with the specified data object value
     *      The last accessed list node is reset.
     *
     *      It is an error if the specified node is not in the list.  It is not
     *      allowed to pass this routine an empty list.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Removed data object value.
     */

    void * ListRemove(
        List *      pList,          /* pointer to list */
        void *      pData)          /* data object value */
    {
        ListNode *  pNode;

        assert(pList != NULL);
        validate(pList->count > 0, NULL);

        pNode = pList->pHead->pNext;
        while (pNode != pList->pHead && pNode->pData != pData)
            pNode = pNode->pNext;
        validate(pNode->pData == pData, NULL);

        pNode->pNext->pPrev = pNode->pPrev;
        pNode->pPrev->pNext = pNode->pNext;
        free(pNode);

        pList->pNodeLast = NULL;
        pList->count--;

        return pData;
    }


    /******************************************************************************
     *
     *      ListRemoveLast - remove last accessed node from list
     *
     *  DESCRIPTION
     *      This routine removes the node which was last accessed by one of the
     *      routines in this library that set it.  Subsequently the last accessed
     *      list node is set to the next node for convenience.
     *
     *      It is an error if the last accessed node was not set by one of the
     *      routines in this library.  It is not allowed to pass this routine an
     *      empty list.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Removed data object value.
     */

    void * ListRemoveLast(
        List *      pList)          /* pointer to list */
    {
        void *      pData;
        ListNode *  pNext;

        assert(pList != NULL);
        validate(pList->pNodeLast != NULL, NULL);

        pData = pList->pNodeLast->pData;

        pNext = pList->pNodeLast->pNext;

        pList->pNodeLast->pNext->pPrev = pList->pNodeLast->pPrev;
        pList->pNodeLast->pPrev->pNext = pList->pNodeLast->pNext;
        free(pList->pNodeLast);

        pList->pNodeLast = pNext;
        pList->count--;

        return pData;
    }


    /******************************************************************************
     *
     *      ListHead - get head data object value
     *
     *  DESCRIPTION
     *      This routine returns the user data object value of the head node of
     *      the specified list.  The last accessed list node is reset if the list
     *      is empty, otherwise it is set to the list head.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Head data object value, or NULL if empty.
     */

    void * ListHead(
        List *      pList)          /* pointer to list */
    {
        assert(pList != NULL);

        if (pList->count == 0)
            return NULL;
        else
            return 0, (pList->pNodeLast = pList->pHead->pNext)->pData;
    }


    /******************************************************************************
     *
     *      ListTail - get tail data object value
     *
     *  DESCRIPTION
     *      This routine returns the user data object value of the tail node of
     *      the specified list.  The last accessed list node is reset if the list
     *      is empty, otherwise it is set to the list tail.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Head data object value, or NULL if empty.
     */

    void * ListTail(
        List *      pList)          /* pointer to list */
    {
        assert(pList != NULL);

        if (pList->count == 0)
            return NULL;
        else
            return 0, (pList->pNodeLast = pList->pHead->pPrev)->pData;
    }


    /******************************************************************************
     *
     *      ListLast - get last accessed data object value
     *
     *  DESCRIPTION
     *      This routine returns the user data object value of the last accessed
     *      node of the specified list.  The last accessed list node is not
     *      affected.
     *
     *      When the last accessed list node is not set, which is also the case
     *      when the list is empty, NULL is returned.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Last accessed data object value, or NULL if not set.
     */

    void * ListLast(
        List *      pList)          /* pointer to list */
    {
        assert(pList != NULL);

        if (pList->pNodeLast == NULL)
            return NULL;
        else
            return pList->pNodeLast->pData;
    }


    /******************************************************************************
     *
     *      ListNext - get next data object value
     *
     *  DESCRIPTION
     *      This routine returns the user data object value of the next node
     *      with respect to the last accessed list node.  The last accessed list
     *      node is set to the next node, or is reset if the tail is passed.
     *
     *      It is an error if the last accessed list node is not set; which is
     *      also the case when the list is empty.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Next data object value, or NULL if already at tail.
     */

    void * ListNext(
        List *      pList)          /* pointer to list */
    {
        assert(pList != NULL);
        validate(pList->pNodeLast != NULL, NULL);

        if ((pList->pNodeLast = pList->pNodeLast->pNext) == pList->pHead)
        {
            pList->pNodeLast = NULL;
            return NULL;
        }
        else        
            return pList->pNodeLast->pData;
    }


    /******************************************************************************
     *
     *      ListPrev - get previous data object value
     *
     *  DESCRIPTION
     *      This routine returns the user data object value of the previous node
     *      with respect to the last accessed list node.  The last accessed list 
     *      node is set to the previous node, or is reset if the head is passed.
     *
     *      It is an error if the last accessed list node is not set; which is
     *      also the case when the list is empty.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Next data object value, or NULL if already at head.
     */

    void * ListPrev(
        List *      pList)          /* pointer to list */
    {
        assert(pList != NULL);
        validate(pList->pNodeLast != NULL, NULL);

        if ((pList->pNodeLast = pList->pNodeLast->pPrev) == pList->pHead)
        {
            pList->pNodeLast = NULL;
            return NULL;
        }
        else        
            return pList->pNodeLast->pData;
    }


    /******************************************************************************
     *
     *      ListCount - report number of nodes in list
     *
     *  DESCRIPTION
     *      This routine returns the number of nodes in the specified list.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Number of nodes in list.
     */

    int ListCount(
        List *      pList)          /* pointer to list */
    {
        assert(pList != NULL);

        return pList->count;
    }


    /******************************************************************************
     *
     *      ListFind - find list node
     *
     *  DESCRIPTION
     *      This routine finds the node with the specified data object value.  If
     *      nothing was found the last accessed list node is not affected, 
     *      otherwise it is set to the found node.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Found data object value, or NULL if not found or empty list.
     */

    void * ListFind(
        List *      pList,          /* pointer to list */
        void *      pData)          /* data object value */
    {
        ListNode *  pNode;

        assert(pList != NULL);

        pNode = pList->pHead->pNext;
        while (pNode != pList->pHead && pNode->pData != pData)
            pNode = pNode->pNext;

        if (pNode->pData == pData)
        {
            pList->pNodeLast = pNode;
            return pData;
        }
        else
            return NULL;
    }


    /******************************************************************************
     *
     *      ListSplitBefore - split list just before last node
     *
     *  DESCRIPTION
     *      This routine splits up the specified list in two parts.  The split is
     *      made just before the last accessed list node.  A new list is created
     *      for the part before the last accessed node.  The last accessed list
     *      node for the original list is not affected.  And the last accessed
     *      list node for the new list is reset.
     *
     *      It is not allowed to pass this routine an empty list.  If the list
     *      contains only one node, the new list will be empty.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Pointer to new list containing nodes before original last accessed
     *      list node.
     */

    List * ListSplitBefore(
        List *      pListOrg)       /* pointer to original list */
    {
        List *      pListNew;
        ListNode *  pNodeOrg;

        assert(pListOrg != NULL);
        validate(pListOrg->count > 0, NULL);
        validate(pListOrg->pNodeLast != NULL, NULL);

        pListNew = malloc(sizeof(List));

        pListNew->pHead = malloc(sizeof(ListNode));
        pListNew->pHead->pData = NULL;
        pListNew->count = 0;

        pNodeOrg = pListOrg->pHead->pNext;
        while (pNodeOrg != pListOrg->pNodeLast)
        {
            pListOrg->count--;
            pListNew->count++;

            pNodeOrg = pNodeOrg->pNext;
        }

        if (pListNew->count == 0)
        {
            pListNew->pHead->pPrev = pListNew->pHead->pNext = pListNew->pHead;
        }
        else
        {
            /* connect list part to new list */
            pListNew->pHead->pNext = pListOrg->pHead->pNext;
            pListNew->pHead->pNext->pPrev = pListNew->pHead;
            pListNew->pHead->pPrev = pListOrg->pNodeLast->pPrev;
            pListNew->pHead->pPrev->pNext = pListNew->pHead;

            /* bind last accessed node and original dummy node together */
            pListOrg->pHead->pNext = pListOrg->pNodeLast;
            pListOrg->pNodeLast->pPrev = pListOrg->pHead;
        }

        pListNew->pNodeLast = NULL;

        return pListNew;
    }


    /******************************************************************************
     *
     *      ListSplitAfter - split list just after last node
     *
     *  DESCRIPTION
     *      This routine splits up the specified list in two parts.  The split is
     *      made just after the last accessed list node.  A new list is created
     *      for the part after the last accessed node.  The last accessed list
     *      node for the original list is not affected.  And the last accessed
     *      list node for the new list is reset.
     *
     *      It is not allowed to pass this routine an empty list.  If the list
     *      contains only one node, the new list will be empty.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      Pointer to new list containing nodes after original last accessed
     *      list node.
     */

    List * ListSplitAfter(
        List *      pListOrg)       /* pointer to original list */
    {
        List *      pListNew;
        ListNode *  pNodeOrg;

        assert(pListOrg != NULL);
        validate(pListOrg->count > 0, NULL);
        validate(pListOrg->pNodeLast != NULL, NULL);

        pListNew = malloc(sizeof(List));

        pListNew->pHead = malloc(sizeof(ListNode));
        pListNew->pHead->pData = NULL;
        pListNew->count = 0;

        pNodeOrg = pListOrg->pNodeLast->pNext;
        while (pNodeOrg != pListOrg->pHead)
        {
            pListOrg->count--;
            pListNew->count++;

            pNodeOrg = pNodeOrg->pNext;
        }

        if (pListNew->count == 0)
        {
            pListNew->pHead->pPrev = pListNew->pHead->pNext = pListNew->pHead;
        }
        else
        {
            /* connect list part to new list */
            pListNew->pHead->pNext = pListOrg->pNodeLast->pNext;
            pListNew->pHead->pNext->pPrev = pListNew->pHead;
            pListNew->pHead->pPrev = pListOrg->pHead->pPrev;
            pListNew->pHead->pPrev->pNext = pListNew->pHead;

            /* bind last accessed node and original dummy node together */
            pListOrg->pNodeLast->pNext = pListOrg->pHead;
            pListOrg->pHead->pPrev = pListOrg->pNodeLast;
        }

        pListNew->pNodeLast = NULL;

        return pListNew;
    }


    /******************************************************************************
     *
     *      ListConcat - concatenate two lists
     *
     *  DESCRIPTION
     *      This routine concatenates the second list <pListAdd> to the tail of
     *      the first list <pListDst>.  Either list (or both) may be empty.  After
     *      the operation the <pListAdd> handle is destroyed.  The last accessed 
     *      list node of the resulting list is reset.
     *
     *  SIDE EFFECTS
     *      None.
     *
     *  RETURNS
     *      List pointer <pListDst> containing the nodes of both lists.
     */

    List * ListConcat(
        List *      pListDst,       /* pointer to destination list */
        List *      pListAdd)       /* pointer to list to be added at tail */
    {
        assert(pListDst != NULL);
        assert(pListAdd != NULL);

        switch (((pListAdd->count > 0) << 1) | (pListDst->count > 0))
        {
        case 0:
            /* both lists empty */

            break;

        case 1:
            /* destination list not empty and add list empty */

            break;

        case 2:
            /* destination list empty and add list not empty */

            pListDst->pHead->pNext = pListAdd->pHead->pNext;
            pListDst->pHead->pNext->pPrev = pListDst->pHead;
            pListDst->pHead->pPrev = pListAdd->pHead->pPrev;
            pListDst->pHead->pPrev->pNext = pListDst->pHead;
            break;

        case 3:
            /* both lists not empty */

            pListAdd->pHead->pPrev->pNext = pListDst->pHead;
            pListDst->pHead->pPrev->pNext = pListAdd->pHead->pNext;
            pListAdd->pHead->pNext->pPrev = pListDst->pHead->pPrev;
            pListDst->pHead->pPrev = pListAdd->pHead->pPrev;
            break;
        }

        pListDst->pNodeLast = NULL;
        pListDst->count += pListAdd->count;

        free(pListAdd->pHead);
        free(pListAdd);

        return pListDst;
    } 


    /* end of List.c */
