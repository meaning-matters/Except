/*
 *      Except.c - exception handling module
 *
 *  DESCRIPTION
 *      This module contains private routines that are exclusively used by the
 *      exception handling package.  The user must not call any of these 
 *      routines directly.
 *
 *      One of the only two global variables is <pC>.  It is a pointer to a
 *      per thread exception handling context; it will always be NULL.  The
 *      'try' macro also defines a <pC>, but as a local variable of the block 
 *      that encloses all 'catch' clauses; at the start of the 'try' macro it 
 *      will be made to point to the actual exception handling context that
 *      is returned by ExceptGetContext().  Using this local <pC> avoids having
 *      to call ExceptGetContext() at every point it is used in the 'try' and
 *      'catch' macros.
 *      The 'finally', 'throw' and 'return' macros also need a <pC>, but have
 *      to invoke the relatively slow (compared to direct <pC> use)
 *      ExceptGetContext().  But because ExceptGetContext() is passed <pC> and
 *      immediately returns it again when not NULL, these macros will spend
 *      no time looking up (in hash table when multi-threading) the current
 *      context when inside a 'try' or 'catch' block; indirectly they will use
 *      the local <pC> mentioned above.  When outside such a block the global
 *      <pC> will be referenced and ExceptGetContext() will perform the needed
 *      lookup.
 *      In other words, ExceptGetContext() will either use the global <pC>
 *      with value NULL which will require context lookup, or will use a local
 *      <pC> that already points to the current context (no lookup needed).
 *
 *      For single-threaded applications the context data is kept in the
 *      static <defaultContext> so that a look up by ExceptGetContext() is
 *      simply taking the address of it.
 *      For multi-threading each thread (using exception handling) will get
 *      its private context.  In that case these contexts are kept in a
 *      hash table for fast lookup; the thread ID is used as key.
 *
 *      Each 'try' statement is associated with an exception object which
 *      keeps both the state of the 'try' statement and the description of
 *      the exception when occurred in the scope of this 'try' statement.  To
 *      allow nesting, these objects are stored in a LIFO buffer (i.e., stack).
 *      This stack is part of the exception handling context mentioned above.
 *      A 'try' will create such an exception object and will push it on the 
 *      stack.  The matching 'finally' will pop the stack and, when the current
 *      exception object contains a pending exception, it will be handled
 *      (either rethrow or default action), subsequently it will be freed.
 *      For performance reasons the context holds a reference to the top 
 *      of the stack (i.e., the current exception object).
 *
 *      In a multi-threading environment the threads either share the signal
 *      handlers (like on Solaris or Windows NT) or each have a private set
 *      (like on the real-time OS VxWorks).  When shared, the first 'try'
 *      executed by one of the threads, will save the default (or previously 
 *      installed by the application) handler in static variables; the last
 *      'finally' executed by one of the threads, will restore them again.
 *      But with private handlers, they will be stored in the context in the 
 *      outermost 'try' of each thread and restored during the matching
 *      'finally'.
 *
 *      There are three different longjmp() destinations which are all kept
 *      by the exception object.  Two of them <throwBuf> and <finalBuf> are
 *      recorded, (using setjmp()) by the 'try' and 'finally' macros
 *      respectively, even before the user 'try' block is being executed.
 *      The third destination <returnBuf> is recorded by the return() macro.
 *      Having these destinations, routines in this module (that are called
 *      from the macros) can jump to either destination:
 *
 *              throwBuf  - starts 'catch' evaluation; jumped to by
 *                          ExceptThrow() on a 'throw' inside 'try' block
 *              finalBuf  - start 'finally' block execution; jumped to by 
 *                          ExceptThrow() on a 'throw' inside 'catch' or 
 *                          'finally' block (the 'finally' block will not be 
 *                          executed again); or jumped to by ExceptReturn()
 *                          which is invoked from the 'return' macro
 *              returnBuf - execute native return(); jumped to by the 'finally'
 *                          cleanup routine ExceptFinally(); this destination
 *                          is dynamically allocated and is propagated using
 *                          the <pData> exception context member
 *                          
 *
 *      First, the macro code is responsible for the control flow of both
 *      the user and its own code (it supplies all necessary control flow
 *      statement).  Then there is an intertwined cooperation between the
 *      macro code and the routines of this module: each macro has a matching
 *      routine which is called from it, and the routines jump back into the
 *      macro code using destinations recorded by the macros.  Finally, the
 *      responsibility of the routines is performing the exception handling
 *      administration and in a way logistics.
 *
 *  INCLUDE FILES
 *      Except.h
 *
 *  COPYRIGHT
 *      You are free to use, copy or modify this software at your own risk.
 *
 *  AUTHOR
 *      Cornelis van der Bent.  Please let me know if you have comments or find
 *      flaws: cg_vanderbent@mail.com.  Enjoy!
 *
 *  TODO
 *      ### What happens when SIGSEGV in finally?  Is numThreadsTry-- in correct place?
 *      ### When no try-catch in our thread but elsewhere, and run-time exception
 *          occors: then e.g. BusError lost is printed continuously.  In Java
 *          an unhandled exception is passed to ThreadGroup, we don't have this.
 *          One solution is to exit() the application.  Another would be to
 *          introduce a kind of ThreadGroup.uncaughtException() handler routine.
 *      ### Maybe it's not allowed to call pthread_self in a signal handler?
 *
 *  MODIFICATION HISTORY
 *      1999/05/25 vdbent       Fixed return() jmp_buf propagation.
 *      1999/04/20 vdbent       Added '&' before <jmp_buf> memcpy() arguments.
 *      1999/04/12 vdbent       Thorough test and debugging; beta release.
 *      1998/12/18 vdbent       Conception.
 */

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#ifdef  EXCEPT_THREAD_POSIX
#include <pthread.h>
#endif
#include "Except.h"
#include "Assert.h"
#include "Hash.h"
#include "List.h"

Context *       pC = NULL;
Class           Throwable = { 1, NULL, "Throwable" };

except_class_define(Exception,           Throwable);
except_class_define(OutOfMemoryError,    Exception);
except_class_define(FailedAssertion,     Exception);
except_class_define(RuntimeException,    Exception);
except_class_define(AbnormalTermination, RuntimeException);  /* SIGABRT */
except_class_define(ArithmeticException, RuntimeException);  /* SIGFPE */
except_class_define(IllegalInstruction,  RuntimeException);  /* SIGILL */
except_class_define(SegmentationFault,   RuntimeException);  /* SIGSEGV */
except_class_define(BusError,            RuntimeException);  /* SIGBUS */

#if     defined(EXCEPT_MT_SHARED) || defined(EXCEPT_MT_PRIVATE)
#define MULTI_THREADING 1
#ifdef  EXCEPT_THREAD_POSIX
#define EXCEPT_THREAD_ID_FUNC           (int)pthread_self
#define EXCEPT_THREAD_MUTEX_FUNC        ExceptMutex
#else
extern  int EXCEPT_THREAD_ID_FUNC(void);
extern  int EXCEPT_THREAD_MUTEX_FUNC(int mode);
#endif
#else
#define MULTI_THREADING 0
#define EXCEPT_THREAD_MUTEX_FUNC(mode)
#endif

#ifdef  EXCEPT_MT_SHARED
#define SHARE_HANDLERS  1
#else
#define SHARE_HANDLERS  0
#endif

static Class            ReturnEvent = { 1, NULL, "ReturnEvent" };
static Context          defaultContext; /* used when single-threaded */
static volatile Hash *  pContextHash;   /* thread context hash-table */
static volatile int     numThreadsTry;  /* number of threads in 'try' stmt. */
static Handler          sharedSigAbrtHandler;
static Handler          sharedSigFpeHandler;
static Handler          sharedSigIllHandler;
static Handler          sharedSigSegvHandler;
static Handler          sharedSigBusHandler;


/******************************************************************************
 *
 *      ExceptMutex - lock/unlock for thread shared data access
 *
 *  DESCRIPTION
 *      This routine is the POSIX threads implementation of the function that
 *      locks/unlocks threads before/after accessing shared data.  The <mode>
 *      parameter selects between lock (1) and unlock (0).
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      N/A.
 */

#ifdef  EXCEPT_THREAD_POSIX
static void ExceptMutex(
    int         mode)           /* 1: lock, 0 unlock */
{
    static volatile pthread_mutex_t mutex;
    static volatile sig_atomic_t    initialized;
    static volatile sig_atomic_t    ready;
    static volatile int             count;
    static volatile pthread_t       tid;

    if (!initialized && !initialized++)
    {
        pthread_mutex_init(&mutex, NULL);
        ready = 1;
    }
    else
    {
        while (!ready)
            ;
    }
    
    if (mode == 1)
    {
        if (tid == pthread_self())
        {
            count++;
        }
        else
        {
            pthread_mutex_lock(&mutex);
            tid = pthread_self();
            count = 1;
        }
    }
    else if (mode == 0)
    {
        if (tid == pthread_self())
        {
            if (--count == 0)
            {
                tid = 0;
                pthread_mutex_unlock(&mutex);
            }
        }
        else if (tid != 0 && tid != pthread_self())
        {
            fprintf(stderr, "Except internal error: thread attempts to unlock"
                            " without holding lock\n");
        }
    }
}
#endif


/******************************************************************************
 *
 *      ExceptPrintDebug - print routine name for debugging
 *
 *  DESCRIPTION
 *      This routine is invoked in most of the routines in this module.  It 
 *      prints <pName> to stderr indented with spaces up to the 'try' nesting
 *      level.
 *
 *      Due to the use of longjmp() and the 'tricky' macros, the program flow
 *      may seem obscure.  Having a list of indented routine invocations
 *      seems a minimal aid for finding bugs, or understanding operation.
 *
 *      When DEBUG_EXCEPT is not defined ExceptPrintDebug() ends up being an 
 *      empty macro.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      N/A.
 */

#ifdef  EXCEPT_DEBUG
static void ExceptPrintDebug(
    Context *   pC,             /* pointer to exception context */
    char *      pName)          /* routine name being printed */
{
    int n;

    if (pC == NULL)
        pC = ExceptGetContext(NULL);

    for (n = (pC && pC->exStack) ? LifoCount(pC->exStack) : 0; n != 0; n--)
        fputs(" ", stderr);

    fputs(pName, stderr);
    fputc('\n', stderr);
}
#else
#define ExceptPrintDebug(pC, pName)
#endif


/******************************************************************************
 *
 *      ExceptGetScope - get exception block scope
 *
 *  DESCRIPTION
 *      This routine simply returns the exception handling code scope (OUTSIDE,
 *      INTERNAL, TRY, CATCH or FINALLY).
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      The current scope.
 */

Scope ExceptGetScope(
    Context *   pC)             /* pointer to thread exception context */
{
    Scope       scope;

    if (pC == NULL)
        pC = ExceptGetContext(NULL);

    ExceptPrintDebug(pC, "ExceptGetScope");

    if (pC == NULL || pC->pEx == NULL)
        scope = OUTSIDE;
    else
        scope = ((Except *)LifoPeek(pC->exStack, 1))->scope;

    return scope;
}


/******************************************************************************
 *
 *      ExceptGetContext - get exception handling context of current thread
 *
 *  DESCRIPTION
 *      This routine looks up the exception handling context of the current
 *      thread.  For performance reasons the static <defaultContext> is used
 *      for single-threading; for multi-threading, the context is
 *      either retrieved from the hash table <pContextHash> or, when not there
 *      yet, will be created on the fly, and added to the hash table.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Pointer to current exception handling context.
 */

Context * ExceptGetContext(
    Context *   pC)             /* pointer to thread exception context */
{
#if     MULTI_THREADING
    EXCEPT_THREAD_MUTEX_FUNC(1);
    if (pC == NULL && pContextHash != NULL)
        pC = HashLookup(pContextHash, EXCEPT_THREAD_ID_FUNC());
    EXCEPT_THREAD_MUTEX_FUNC(0);
    
    return pC;
#else
    return &defaultContext;
#endif
}


/******************************************************************************
 *
 *      ExceptCreateContext - create exception handling context for thread
 *
 *  DESCRIPTION
 *      This routine creates and stores the exception handling context for
 *      the current thread.
 *
 *  SIDE EFFECTS
 *      Adds created context to hash table.
 *
 *  RETURNS
 *      Pointer to new exception handling context.
 */

#if     MULTI_THREADING
static Context * ExceptCreateContext(void)
{
    Context *   pC;

    pC = calloc(1, sizeof(Context));
    if (pC == NULL)
        fprintf(stderr, "Except internal error: out of memory.\n");
    EXCEPT_THREAD_MUTEX_FUNC(1);
    HashAdd(pContextHash, EXCEPT_THREAD_ID_FUNC(), pC);
    EXCEPT_THREAD_MUTEX_FUNC(0);
    
    ExceptPrintDebug(pC, "ExceptCreateContext");
    
    return pC;
}
#else
#define ExceptCreateContext()   NULL
#endif


/******************************************************************************
 *
 *      ExceptGetMessage - get current exception description string
 *
 *  DESCRIPTION
 *      This routine composes a descriptive string of the current exception 
 *      <pEx>.  A pointer to this routine is stored in the <getMessage> member
 *      of <pEx>, so a user can invoke it inside a 'catch' block.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Address of static description string which is overwritten by each call.
 */

static char * ExceptGetMessage(void)
{
    Context *   pC = ExceptGetContext(NULL);

    ExceptPrintDebug(pC, "ExceptGetMessage");

    sprintf(pC->message, "%s: file \"%s\", line %d.",
            pC->pEx->class->name, pC->pEx->file, pC->pEx->line);

    return pC->message;
}


/******************************************************************************
 *
 *      ExceptGetClass - get current exception class
 *
 *  DESCRIPTION
 *      This routine returns the <class> member of the current exception.  A
 *      pointer to this routine is stored in the <getMask> member of <pEx>,
 *      so a user can invoke it inside a 'catch' block.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Current exception class.
 */

static ClassRef ExceptGetClass(void)
{
    Context *   pC = ExceptGetContext(NULL);

    ExceptPrintDebug(pC, "ExceptGetClass");

    return pC->pEx->class;
}


/******************************************************************************
 *
 *      ExceptGetData - get current exception associated data
 *
 *  DESCRIPTION
 *      This routine returns the <pData> member of the current exception.  A
 *      pointer to this routine is stored in the <getData> member of <pEx>,
 *      so a user can invoke it inside a 'catch' block.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      Current exception associated data.
 */

static void * ExceptGetData(void)
{
    Context *   pC = ExceptGetContext(NULL);

    ExceptPrintDebug(pC, "ExceptGetData");

    return pC->pEx->pData;
}


/******************************************************************************
 *
 *      ExceptPrintTryTrace - prints the nested 'try' trace
 *
 *  DESCRIPTION
 *      This routine prints the source file name and line number of the 'try'
 *      statement in which the exception occurred and of all its enclosing
 *      'try' statements.
 *
 *      Unless the <pFile> argument is not NULL, it prints to stderr.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      N/A.
 */

static void ExceptPrintTryTrace(
   FILE *       pFile)          /* stream to which is printed or NULL */
{
    Context *   pC = ExceptGetContext(NULL);
    int         n;
    
    ExceptPrintDebug(pC, "ExceptGetData");

    if (pFile == NULL)
        pFile = stderr;

#if     MULTI_THREADING
    fprintf(pFile, "%s occurred in thread %d:\n", pC->pEx->class->name,
            EXCEPT_THREAD_ID_FUNC());
#else
    fprintf(pFile, "%s occurred:\n", pC->pEx->class->name);
#endif

    for (n = 1; n <= LifoCount(pC->exStack); n++)
    {
        Except *    pEx = LifoPeek(pC->exStack, n);
        
        fprintf(pFile, "        in 'try' at %s:%d\n", pEx->tryFile, pEx->tryLine);
    }
}


/******************************************************************************
 *
 *      ExceptThrowSignal - 'throw' exception caused by signal
 *
 *  DESCRIPTION
 *      This routine is used as the signal handler for SIGABRT, SIGFPE, SIGILL,
 *      SIGSEGV and SIGBUS.  Only traps (the (ANSI C) signals that deal with
 *      illegal/abnormal/erroneous conditions) are supported.  Installing this
 *      routine as signal handler is done in ExceptTry().
 *
 *      It uses ExceptThrow() to perform the actual 'throw'.
 *
 *      Some OSs (e.g. Solaris) first set the signal's disposition to SIG_DFL
 *      before executing the signal handler.  Therefore this routine is instal-
 *      led as signal handler again each time it is invoked.
 *
 *  SIDE EFFECTS
 *      ExceptThrow() is invoked, so this routine will not return.
 *
 *  RETURNS
 *      N/A.
 */

static void ExceptThrowSignal(
    int         number)         /* signal number */
{
    ClassRef    class;

    ExceptPrintDebug(pC, "ExceptThrowSignal");

    switch (number)
    {
    case SIGABRT: class = AbnormalTermination; break;
    case SIGFPE:  class = ArithmeticException; break;
    case SIGILL:  class = IllegalInstruction;  break;
    case SIGSEGV: class = SegmentationFault;   break;
#ifdef  SIGBUS
    case SIGBUS:  class = BusError;            break;
#endif
    }

    signal(number, ExceptThrowSignal);

    class->signalNumber = number;       /* redundant after first time */
        
    ExceptThrow(NULL, class, NULL, "?", 0);
}


/******************************************************************************
 *
 *      ExceptInstallHandlers - install signal/trap handlers if needed
 *
 *  DESCRIPTION
 *      This routine installs a handler for the traps SIGABRT, SIGFPE, SIGILL,
 *      SIGSEGV and (depending on the platform) SIGBUS.  The previously
 *      installed handlers are stored to be placed back later (when leaving
 *      the outermost 'try' statement) by ExceptResetTrapHandlers().
 *
 *      If only install the handlers once when needed.
 *
 *  SIDE EFFECTS
 *      Increments <numThreadsTry> when shared handlers are restored.
 *
 *  RETURNS
 *      When stored 1, otherwise 0.
 */

static int ExceptInstallHandlers(
    Context *   pC)             /* pointer to thread exception context */
{
    int stored = 0;
    
    if (pC->exStack == NULL)
    {
        pC->exStack = LifoCreate();

        EXCEPT_THREAD_MUTEX_FUNC(1);
        if (MULTI_THREADING && SHARE_HANDLERS && numThreadsTry++ == 0)
        {
            sharedSigAbrtHandler = signal(SIGABRT, ExceptThrowSignal);
            sharedSigFpeHandler  = signal(SIGFPE,  ExceptThrowSignal);
            sharedSigIllHandler  = signal(SIGILL,  ExceptThrowSignal);
            sharedSigSegvHandler = signal(SIGSEGV, ExceptThrowSignal);
#ifdef  SIGBUS
            sharedSigBusHandler  = signal(SIGBUS,  ExceptThrowSignal);
#endif

            stored = 1;
        }
        else if (!MULTI_THREADING || !SHARE_HANDLERS)
        {
            pC->sigAbrtHandler = signal(SIGABRT, ExceptThrowSignal);
            pC->sigFpeHandler  = signal(SIGFPE,  ExceptThrowSignal);
            pC->sigIllHandler  = signal(SIGILL,  ExceptThrowSignal);
            pC->sigSegvHandler = signal(SIGSEGV, ExceptThrowSignal);
#ifdef  SIGBUS
            pC->sigBusHandler  = signal(SIGBUS,  ExceptThrowSignal);
#endif

            stored = 1;
        }
        EXCEPT_THREAD_MUTEX_FUNC(0);
    }

    return stored;
}
    

/******************************************************************************
 *
 *      ExceptRestoreHandlers - restores signal/trap handlers if needed
 *
 *  DESCRIPTION
 *      This routine restores the application original handlers for SIGABRT,
 *      SIGFPE, SIGILL, SIGSEGV and (depending on the platform) SIGBUS.  The
 *      The original handlers are stored by ExceptInstallHandlers().
 *
 *      It only restored the handlers once when needed.
 *
 *  SIDE EFFECTS
 *      Decrements <numThreadsTry> when shared handlers are restored.
 *
 *  RETURNS
 *      When restored 1, otherwise 0.
 */

static int ExceptRestoreHandlers(
    Context *   pC)             /* pointer to thread exception context */
{
    int restored = 0;

    EXCEPT_THREAD_MUTEX_FUNC(1);
    if (MULTI_THREADING && SHARE_HANDLERS && --numThreadsTry == 0)
    {
        signal(SIGABRT, sharedSigAbrtHandler);
        signal(SIGFPE,  sharedSigFpeHandler);
        signal(SIGILL,  sharedSigIllHandler);
        signal(SIGSEGV, sharedSigSegvHandler);
#ifdef  SIGBUS
        signal(SIGBUS,  sharedSigBusHandler);
#endif

        restored = 1;
    }
    else if (!MULTI_THREADING || !SHARE_HANDLERS)
    {
        signal(SIGABRT, pC->sigAbrtHandler);
        signal(SIGFPE,  pC->sigFpeHandler);
        signal(SIGILL,  pC->sigIllHandler);
        signal(SIGSEGV, pC->sigSegvHandler);
#ifdef  SIGBUS
        signal(SIGBUS,  pC->sigBusHandler);
#endif

        restored = 1;
    }
    EXCEPT_THREAD_MUTEX_FUNC(0);
    
    return restored;
}


/******************************************************************************
 *
 *      ExceptThreadCleanup - cleanup exception handling for ceased thread
 *
 *  DESCRIPTION
 *      This routine, which is used by the except_thread_cleanup() macro,
 *      removes the exception context of the <threadId> thread from
 *      <pContextHash> and frees it.
 *
 *      It must be used after the specified thread has ceased to exist and
 *      when there is reason to assume that this thread did not perform a
 *      cleanup itself (done automatically by the outermost 'finally'); for
 *      example when it was killed.
 *
 *      In a multi-threading environment that recycles IDs, this routine must
 *      be called as soon as possible after the specified thread was stopped
 *      without executing the outermost 'fianlly'.  To prevent hazardous
 *      situations, care must be taken that no new threads are created in
 *      between stopping a thread and cleaning it up using this routine.
 *
 *  SIDE EFFECTS
 *      May remove a context from <pContextHash> and free it.
 *
 *  RETURNS
 *      N/A.
 */

void ExceptThreadCleanup(
    int         threadId)       /* ID of ceased thread or -1 for self */
{
#if     MULTI_THREADING
    validate(threadId != EXCEPT_THREAD_ID_FUNC(), NOTHING);
    if (threadId == -1)
        threadId = EXCEPT_THREAD_ID_FUNC();

    EXCEPT_THREAD_MUTEX_FUNC(1);
    if (pContextHash != NULL)
    {
        Context * pC;
        
        pC = HashLookup(pContextHash, threadId);
        if (pC != NULL)
        {
            ExceptRestoreHandlers(pC);
            LifoDestroyData(pC->exStack);
            if (pC->pEx->checkList != NULL)
                ListDestroyData(pC->pEx->checkList);
            free(HashRemove(pContextHash, threadId));
        }
    }    
    EXCEPT_THREAD_MUTEX_FUNC(0);
#endif
}


/******************************************************************************
 *
 *      ExceptTry - prepare for 'try'
 *
 *  DESCRIPTION
 *      This routine creates and initializes the exception handling context
 *      (for the current thread) if not there yet, installs ExceptThrowSignal() 
 *      as the signal handler for SIGABRT, SIGFPE, SIGILL, SIGSEGV and SIGBUS,
 *      and finally stores an empty/cleared handle on the exception nesting
 *      stack.
 *
 *      When <pC> is NULL, this is the first try in a routine.  This condition
 *      is stored to enable a ReturnEvent thrown (by the return() macro) from
 *      a nested level, to propagate upto this first routine level.  In this
 *      way all finally blocks can be executed even when returned from a nested
 *      level.
 *
 *      This routine is invoked as the first action of the 'try' macro.
 *
 *  SIDE EFFECTS
 *      Signal/trap handlers are installed.
 *
 *  RETURNS
 *      N/A.
 */

void ExceptTry(
    Context *   pC,             /* pointer to thread exception context */
    char *      file,           /* source file name */
    int         line)           /* source line number */
{
    int first;
    
#if     MULTI_THREADING
    EXCEPT_THREAD_MUTEX_FUNC(1);
    if (pContextHash == NULL)
        pContextHash = HashCreate();
    EXCEPT_THREAD_MUTEX_FUNC(0);
#endif
  
    if (first = (pC == NULL))
        pC = ExceptGetContext(NULL);
    if (pC == NULL)                     /* not needed for single-threading */
        pC = ExceptCreateContext();
  
    ExceptInstallHandlers(pC);

    LifoPush(pC->exStack, pC->pEx = calloc(sizeof(Except), 1));
    pC->pEx->first = first; 
    pC->pEx->tryFile = file;
    pC->pEx->tryLine = line;
    
    ExceptPrintDebug(pC, "ExceptTry");
}


/******************************************************************************
 *
 *      ExceptThrow - dispatch exception 'throw'
 *
 *  DESCRIPTION
 *      This routine processes a thrown exception.
 *
 *      Throwing an exception involves copying the five arguments into the
 *      current exception handle <pEx> and subsequently jumping back to the
 *      user/macro source code for further processing.  The longjmp() depends
 *      on the context (the inner most exception block type) in which the
 *      throw occurred.  When inside a 'try' block we jump into the try() macro
 *      code in order to go through the catch() macros code.  When inside
 *      a 'catch' block or a 'finally' block a jump is done into the finally()
 *      macro code.
 *
 *      When this routine is invoked outside exception scope, it prints a
 *      message on <stderr> telling in full detail that an exception was lost.
 *
 *  SIDE EFFECTS
 *      When called from exception scope it never returns because of longjmp().
 *
 *  RETURNS
 *      N/A.
 */

void ExceptThrow(
    Context *   pC,             /* pointer to thread exception context */
    void *      pExceptOrClass, /* rethrown exception OR class to be catched */
    void *      pData,          /* pointer to associated data or NULL */
    char *      file,           /* name of source file where invoked */
    int         line)           /* source file line number */
{
    ExceptPrintDebug(pC, "ExceptThrow");

    if (pC == NULL)
        pC = ExceptGetContext(NULL);

    if (pC == NULL || pC->exStack == NULL || LifoCount(pC->exStack) == 0)
    {
        fprintf(stderr, "%s lost: file \"%s\", line %d.\n",
                ((ClassRef)pExceptOrClass)->name, file, line);
    
        return;
    }
    
    if (((ClassRef)pExceptOrClass)->notRethrown)
    {
        pC->pEx->class         = (ClassRef)pExceptOrClass;
        pC->pEx->pData         = pData;
        pC->pEx->file          = file;
        pC->pEx->line          = line;
        pC->pEx->getMessage    = ExceptGetMessage;
        pC->pEx->getData       = ExceptGetData;
        pC->pEx->printTryTrace = ExceptPrintTryTrace;
    }
    pC->pEx->state = PENDING;   /* in case of throw() inside 'catch' */

    switch (pC->pEx->scope)
    {
    case TRY:
        ExceptPrintDebug(pC, "longjmp(throwBuf)");
        LONGJMP(pC->pEx->throwBuf, 1);

    case CATCH:
        ExceptPrintDebug(pC, "longjmp(finalBuf)");
        LONGJMP(pC->pEx->finalBuf, 1);

    case FINALLY:
        ExceptPrintDebug(pC, "longjmp(finalBuf)");
        LONGJMP(pC->pEx->finalBuf, 1);
    }
}


/******************************************************************************
 *
 *      ExceptIsDerived - determine if class is derived or identical
 *
 *  DESCRIPTION
 *      This routine determines if <class> is derived from <base> or is
 *      identical.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      One when derived or identical, otherwise zero.
 */

static int ExceptIsDerived(
    ClassRef    class,  /* class being considered */
    ClassRef    base)   /* base class */
{
    while (class->parent != NULL && class != base)
        class = class->parent;

    return class == base;
}


/******************************************************************************
 *
 *      ExceptCatch - check if exception can be caught
 *
 *  DESCRIPTION
 *      This routine checks if the currently occurred exception <pC->pEx>
 *      matches <id>.  It is invoked for each subsequent 'catch' clause.
 *
 *      It is called from the 'catch' macro.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      When the current exception was matched 1, or otherwise 0.
 */

int ExceptCatch(
    Context *   pC,             /* pointer to thread exception context */
    ClassRef    class)          /* pointer to occurred exception */
{
    ExceptPrintDebug(pC, "ExceptCatch");

    if (pC == NULL)
        pC = ExceptGetContext(NULL);

    if (pC->pEx->state == PENDING && ExceptIsDerived(pC->pEx->class, class))
        pC->pEx->state = CAUGHT;

    return pC->pEx->state == CAUGHT;
}


/******************************************************************************
 *
 *      ExceptFinally - resolve at end of 'finally'
 *
 *  DESCRIPTION
 *      This routine is called as the final action of an exception level; it
 *      excutes even after the user code in the 'finally' block.
 *
 *      The exception nesting stack (of the current thread) is popped.  When
 *      this stack becomes empty, the default signal handlers may be installed
 *      (when conditions described at the beginning of this file and in the
 *      "README" are met).  If the just popped exception was not caught, the
 *      default action (i.e., outside this package) is performed when the stack
 *      became empty; when the stack is not empty yet, the exception is propa-
 *      gated by doing a new trow.
 *
 *      When there is a pending 'return', a longjmp() is done to the macro code
 *      that performs the actual return.
 *
 *      In all cases the popped exception handle is freed.  For multi-threading
 *      The exception context of the current thread is removed from the hash
 *      table <pContextHash> and freed, when this is the outermost 'finally'
 *      (i.e., when <pC->exStack> became empty).
 *
 *      The return value of this routine is used as stop condition in one of
 *      the while-loops of the 'finally' macro code, and must always be zero. 
 *
 *  SIDE EFFECTS
 *      May never return because of longjmp() for 'return'.
 *
 *  RETURNS
 *      Always 0.
 */

int ExceptFinally(
    Context *   pC)             /* pointer to thread exception context */
{
    Except *    pEx;
    Except      ex;

    if (pC == NULL)
        pC = ExceptGetContext(NULL);

    ex = *(pEx = LifoPop(pC->exStack));
    free(pEx);
    pC->pEx = LifoCount(pC->exStack) ? LifoPeek(pC->exStack, 1) : NULL;

    if (LifoCount(pC->exStack) == 0)
    {
        /* outermost level - default action */

        int     restored = ExceptRestoreHandlers(pC);

        if (ex.state == PENDING)
        {
            if (ex.class == FailedAssertion)
            {
                AssertAction(pC, DO_ABORT, ex.pData, ex.file, ex.line);
            }
            else if (ExceptIsDerived(ex.class, RuntimeException) && restored)
            {
                LifoDestroy(pC->exStack);
                if (MULTI_THREADING)
                {
                    EXCEPT_THREAD_MUTEX_FUNC(1);
                    free(HashRemove(pContextHash, EXCEPT_THREAD_ID_FUNC()));
                    EXCEPT_THREAD_MUTEX_FUNC(0);
                }
                else
                {
                    pC->exStack = NULL;
                }
             
                raise(ex.class->signalNumber);
            }
            else if (ex.class == ReturnEvent)
            {
                LifoDestroy(pC->exStack);
                if (MULTI_THREADING)
                {
                    EXCEPT_THREAD_MUTEX_FUNC(1);
                    free(HashRemove(pContextHash, EXCEPT_THREAD_ID_FUNC()));
                    EXCEPT_THREAD_MUTEX_FUNC(0);
                }
                else
                {
                    pC->exStack = NULL;
                }
                
                LONGJMP(*(JMP_BUF *)ex.pData, 1);
            }
            else
                fprintf(stderr, "%s lost: file \"%s\", line %d.\n",
                        ex.class->name, ex.file, ex.line);
        }
        LifoDestroy(pC->exStack);
        if (MULTI_THREADING)
        {
            EXCEPT_THREAD_MUTEX_FUNC(1);
            free(HashRemove(pContextHash, EXCEPT_THREAD_ID_FUNC()));
            EXCEPT_THREAD_MUTEX_FUNC(0);
        }
        else
        {
            pC->exStack = NULL;
        }
    }
    else     
    {
        /* inner level - propagate */

        if (ex.state == PENDING)
        {
            if (ex.class == ReturnEvent && ex.first)
            {            
                LONGJMP(*(JMP_BUF *)ex.pData, 1);    /* pData is returnBuf */
            }
            else
            {
                ExceptThrow(pC, ex.class, ex.pData, ex.file, ex.line);
            }
        }
    }
            
    return 0;
}


/******************************************************************************
 *
 *      ExceptReturn - process 'return'
 *
 *  DESCRIPTION
 *      This routine, which is called by the return() macro when in exception
 *      handling scope, overrules any pending exception and performs a
 *      longjmp() to the 'finally' code (which on its turn, when finished, will
 *      jump back to the return() macro code in order to perform the actual 
 *      return).
 *
 *  SIDE EFFECTS
 *      Never returns because of longjmp() to 'finally' code.
 *
 *  RETURNS
 *      N/A.
 */

void ExceptReturn(
    Context *   pC)             /* pointer to thread exception context */
{
    ExceptPrintDebug(pC, "ExceptReturn");

    if (pC == NULL)
        pC = ExceptGetContext(NULL);

    pC->pEx->class = ReturnEvent;       /* may overrule pending exception */
    pC->pEx->state = PENDING;           /* in case of return() inside 'catch' */
    ExceptPrintDebug(pC, "longjmp(finalBuf)");
        
    LONGJMP(pC->pEx->finalBuf, 1);
}


/******************************************************************************
 *
 *      ExceptCheckBegin - initiate 'catch' condition checking
 *
 *  DESCRIPTION
 *      This routine is called during DEBUG for each 'try' statement, just 
 *      before and after the 'catch' conditions are checked.  When called the
 *      first time, it creates a list in which all 'catch' conditions will
 *      be stored and which will be used for checking.  The second time, this
 *      routine performs the final test: check if there were any 'catch'
 *      clauses; subsequently the list is destroyed again.
 *
 *      The return value influences the control flow of the macro code.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      First time called 0, or 1 the second time (i.e., when checked).
 */

int ExceptCheckBegin(
    Context *   pC,             /* pointer to thread exception context */
    int *       pChecked,       /* pointer to flag if finished checking */
    char *      file,           /* name of source file where invoked */
    int         line)           /* source file line number */
{
    ExceptPrintDebug(pC, "ExceptCheckBegin");

    if (pC->pEx->checkList == NULL && !*pChecked)
    {
        pC->pEx->checkList = ListCreate();
    }
    else
    {
        if (!*pChecked)
        {
            if (ListCount(pC->pEx->checkList) == 0)
            {
                fprintf(stderr,
                        "Warning: No catch clause(s): file \"%s\", line %d.\n",
                        file, line);
            }

            ListDestroyData(pC->pEx->checkList);
            pC->pEx->checkList = NULL;
            *pChecked = 1;
        }
    }

    return *pChecked;
}


/******************************************************************************
 *
 *      ExceptCheck - perform 'catch' condition check
 *
 *  DESCRIPTION
 *      This routine is called during DEBUG for each 'catch' clause and does
 *      all tests (described in "README") using the conditions of the previous
 *      'catch' clauses, that are stored in <pC->pEx->checkList>.  When a test
 *      fails a message is printed on <stderr>.  After all tests have been
 *      performed, the current 'catch' condition is added to the list.
 *
 *      It is, for the filename in the message, assumed that all 'catch'
 *      clauses belonging to a 'try' statement are all in the same file.
 *
 *      The return value influences the control flow of the macro code.
 *
 *  SIDE EFFECTS
 *      None.
 *
 *  RETURNS
 *      When testing finished 1 (refer to ExceptCheckBegin()), or 0.
 */

int ExceptCheck(
    Context *   pC,             /* pointer to thread exception context */
    int *       pChecked,       /* pointer to flag if finished checking */
    ClassRef    class,          /* next exception class to be checked */
    char *      file,           /* name of source file where invoked */
    int         line)           /* source file line number */
{
    typedef struct _Check
    {
        ClassRef        class;
        int             line;
    } Check;

    ExceptPrintDebug(pC, "ExceptCheck");

    if (!*pChecked)
    {
        Check * pCheck;

        pCheck = ListHead(pC->pEx->checkList);
        while (pCheck != NULL)                  /* breaked from */
        {
            if (class == pCheck->class)
            {
                fprintf(stderr, "Duplicate catch(%s): file \"%s\", line %d; "
                        "already caught at line %d.\n",
                        class->name, file, line, pCheck->line);
                break;
            }

            if (ExceptIsDerived(class, pCheck->class))
            {
                fprintf(stderr, "Superfluous catch(%s): file \"%s\", line %d; "
                        "already caught by %s at line %d.\n", class->name,
                        file, line, pCheck->class->name, pCheck->line);
                break;
            }

            pCheck = ListNext(pC->pEx->checkList);
        }

        if (pCheck == NULL)
        {
            pCheck = malloc(sizeof(Check));
            pCheck->class = class;
            pCheck->line  = line;
            ListAddTail(pC->pEx->checkList, pCheck);
        }
    }
    return *pChecked;
}


/* end of "Except.c" */
