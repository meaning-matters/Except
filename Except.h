/*
 *      Except.h - exception handling module header
 *
 *  DESCRIPTION
 *      This header contains the (type) definitions and more importantly, the
 *      exeption handling support macros.  It also defines return() as a macro.
 *
 *      The 'try', 'catch' and 'finally' macro code has been nicely indented.
 *      A 'try' starts a while-loop that end in its mandatory 'finally'.  This
 *      loop encloses zero or more 'catches'.  When no 'catch' checking is
 *      performed (i.e., when DEBUG is not defined), this loop is executed
 *      twice.  In the first pass the <throwBuf> and <finalBuf> longjmp()
 *      destinations of the current exception object <pC->pEx> are saved; no
 *      user code is executed.  This is controlled by the flag <pC->pEx->ready> 
 *      which is initially zero and is set to one at the end of the first pass.
 *      When DEBUG is defined an early pass is added which checks the 'catch' 
 *      conditions; nothing else happens during this pass.
 *
 *      In the final pass (i.e., the second or third, depending on DEBUG), the
 *      user code in the 'try' block is executed (<pC->pEx->ready> is 1 by
 *      then).  If no exception occurs the 'catch' clauses are fully skipped
 *      and the 'break' halfway 'finally' is executed.  If however an exception
 *      occurred, a longjmp() to <pC->pEx->throwBuf> is performed (by the code
 *      in "Except.c").  Because "setjmp(pC->pEx->throwBuf) == 0" in 'try' is
 *      false, the "else if()" statement for each 'catch' are executed one by
 *      one until a match takes place, causing ExceptCatch() to return 1.
 *      After catching, the 'break' is also executed.
 *
 *      Finally, the two nested while-statements are executed.  At the start,
 *      <pC->pEx->ready> is still 1 and thus greater than 0, so the inner
 *      while-statement will be excecuted; ExceptFinally() is not invoked yet.
 *      In the inner while-statement <pC->pEx->ready> is found to be greater 
 *      than 0, so the user [compound] statement below will be executed; but
 *      first <pC->pEx->ready> is decremented and becomes 0.  After the user 
 *      code is executed, the condition of the outer while-statement is again
 *      evaluated.  This time ExceptFinally() is invoked and always returns 0,
 *      this ends the current exception handling level.
 *
 *  REMARKS
 *      The "do { } while (0);" are placed around the 'try' and 'catch' user
 *      code to allow the user code contain 'break' and 'continue' statements.
 *      Without it, the exception handling "while (1)" main loop would other-
 *      wise be breaked or continued.
 *
 *      For the same reason the while-loop that encloses the 'finally' code
 *      has been split up in two to make sure that ExceptFinally() is always
 *      invoked, even when the 'finally' user code does a 'break'.  The reason
 *      for checking "ready > 0" instead of "ready != 0" as is done in 'try'
 *      and 'catch', is that <ready> becomes -1 when the user code does a 
 *      'continue'.
 *
 *      Most conditions in the macro code depend on ANSI C's left-to-right
 *      lazy boolean expression evaluation.
 *
 *      The reason why the setjmp() calls need to be placed in macros, is that
 *      (according to the C standard) a longjmp() may only be done to a still
 *      existing stack frame.   
 *
 *      K&R (2nd edition) on page 254 states that: "Accessible objects have the
 *      values they had when longjmp() was called, except that non-volatile
 *      automatic variables in the function calling setjmp() become undefined
 *      if they were changed after the setjmp() call.".  In our case this only
 *      applies for <pC>, which is not modified, and so does not need to be
 *      declared volatile.  Be warned that this limitation does still exist for
 *      application variables!
 *
 *  COPYRIGHT
 *      You are free to use, copy or modify this software at your own risk.
 *
 *  AUTHOR
 *      Cornelis van der Bent.  Please let me know if you have comments or find
 *      flaws: cg_vanderbent@mail.com.  Enjoy!
 *
 *  MODIFICATION HISTORY
 *      2000/03/23 vdbent       Added 'pending'.
 *      1999/04/12 vdbent       Thorough test and debugging; beta release.
 *      1998/12/18 vdbent       Conception.
 */

#ifndef _EXCEPT_H
#define _EXCEPT_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include "Lifo.h"
#include "List.h"

#define SETJMP(env)             sigsetjmp(env, 1)
#define LONGJMP(env, val)       siglongjmp(env, val)
#define JMP_BUF                 sigjmp_buf


typedef void (* Handler)(int);

typedef struct _Class *ClassRef;        /* exception class reference */
struct _Class
{
    int         notRethrown;            /* always 1 (used by throw()) */
    ClassRef    parent;                 /* parent class */
    char *      name;                   /* this class name string */
    int         signalNumber;           /* optional signal number */
};

typedef struct _Class Class[1];         /* exception class */

typedef enum _Scope                     /* exception handling scope */
{
    OUTSIDE = -1,                       /* outside any 'try' */
    INTERNAL,                           /* exception handling internal */
    TRY,                                /* in 'try' (across routine calls) */
    CATCH,                              /* in 'catch' (idem.) */
    FINALLY                             /* in 'finally' (idem.) */
} Scope;

typedef enum _State                     /* exception handling state */
{
    EMPTY,                              /* no exception occurred */
    PENDING,                            /* exception occurred but not caught */
    CAUGHT                              /* occurred exception caught */
} State;

typedef struct _Except                  /* exception handle */
{
    int         notRethrown;            /* always 0 (used by throw()) */
    State       state;                  /* current state of this handle */
    JMP_BUF     throwBuf;               /* start-'catching' destination */
    JMP_BUF     finalBuf;               /* perform-'finally' destination */
    ClassRef    class;                  /* occurred exception class */
    void *      pData;                  /* exception associated (user) data */
    char *      file;                   /* exception file name */
    int         line;                   /* exception line number */
    int         ready;                  /* macro code control flow flag */
    Scope       scope;                  /* exception handling scope */
    int         first;                  /* flag if first try in function */
    List *      checkList;              /* list used by 'catch' checking */
    char*       tryFile;                /* source file name of 'try' */
    int         tryLine;                /* source line number of 'try' */

    ClassRef    (*getClass)(void);      /* method returning class reference */
    char *      (*getMessage)(void);    /* method getting description */
    void *      (*getData)(void);       /* method getting application data */
    void        (*printTryTrace)(FILE*);/* method printing nested trace */
} Except;

typedef struct _Context                 /* exception context per thread */
{
    Except *    pEx;                    /* current exception handle */
    Lifo *      exStack;                /* exception handle stack */
    char        message[1024];          /* used by ExceptGetMessage() */
    Handler     sigAbrtHandler;         /* default SIGABRT handler */
    Handler     sigFpeHandler;          /* default SIGFPE handler */
    Handler     sigIllHandler;          /* default SIGILL handler */
    Handler     sigSegvHandler;         /* default SIGSEGV handler */
    Handler     sigBusHandler;          /* default SIGBUS handler */
} Context;

extern Context *        pC;
extern Class            Throwable;

#define except_class_declare(child, parent) extern Class child
#define except_class_define(child, parent)  Class child = { 1, parent, #child }

except_class_declare(Exception,           Throwable);
except_class_declare(OutOfMemoryError,    Exception);
except_class_declare(FailedAssertion,     Exception);
except_class_declare(RuntimeException,    Exception);
except_class_declare(AbnormalTermination, RuntimeException);  /* SIGABRT */
except_class_declare(ArithmeticException, RuntimeException);  /* SIGFPE */
except_class_declare(IllegalInstruction,  RuntimeException);  /* SIGILL */
except_class_declare(SegmentationFault,   RuntimeException);  /* SIGSEGV */
except_class_declare(BusError,            RuntimeException);  /* SIGBUS */


#ifdef  DEBUG

#define CHECKED                                                         \
        static int checked

#define CHECK_BEGIN(pC, pChecked, file, line)                           \
            ExceptCheckBegin(pC, pChecked, file, line)

#define CHECK(pC, pChecked, class, file, line)                          \
                 ExceptCheck(pC, pChecked, class, file, line)

#define CHECK_END                                                       \
            !checked

#else   /* DEBUG */

#define CHECKED
#define CHECK_BEGIN(pC, pChecked, file, line)           1
#define CHECK(pC, pChecked, class, file, line)          1
#define CHECK_END                                       0

#endif  /* DEBUG */


#define except_thread_cleanup(id)       ExceptThreadCleanup(id)

#define try                                                             \
    ExceptTry(pC, __FILE__, __LINE__);                                  \
    while (1)                                                           \
    {                                                                   \
        Context *       pTmpC = ExceptGetContext(pC);                   \
        Context *       pC = pTmpC;                                     \
        CHECKED;                                                        \
                                                                        \
        if (CHECK_BEGIN(pC, &checked, __FILE__, __LINE__) &&            \
            pC->pEx->ready && SETJMP(pC->pEx->throwBuf) == 0)           \
        {                                                               \
            pC->pEx->scope = TRY;                                       \
            do                                                          \
            {

#define catch(class, e)                                                 \
            }                                                           \
            while (0);                                                  \
        }                                                               \
        else if (CHECK(pC, &checked, class, __FILE__, __LINE__) &&      \
                 pC->pEx->ready && ExceptCatch(pC, class))              \
        {                                                               \
            Except *e = LifoPeek(pC->exStack, 1);                       \
            pC->pEx->scope = CATCH;                                     \
            do                                                          \
            {

#define finally                                                         \
            }                                                           \
            while (0);                                                  \
        }                                                               \
        if (CHECK_END)                                                  \
            continue;                                                   \
        if (!pC->pEx->ready && SETJMP(pC->pEx->finalBuf) == 0)          \
            pC->pEx->ready = 1;                                         \
        else                                                            \
            break;                                                      \
    }                                                                   \
    ExceptGetContext(pC)->pEx->scope = FINALLY;                         \
    while (ExceptGetContext(pC)->pEx->ready > 0 || ExceptFinally(pC))   \
        while (ExceptGetContext(pC)->pEx->ready-- > 0)

#define throw(pExceptOrClass, pData)                                    \
    ExceptThrow(pC, (ClassRef)pExceptOrClass, pData, __FILE__, __LINE__)

#define return(x)                                                       \
    {                                                                   \
        if (ExceptGetScope(pC) != OUTSIDE)                              \
        {                                                               \
            void *      pData = malloc(sizeof(JMP_BUF));                \
            ExceptGetContext(pC)->pEx->pData = pData;                   \
            if (SETJMP(*(JMP_BUF *)pData) == 0)                         \
                ExceptReturn(pC);                                       \
            else                                                        \
                free(pData);                                            \
        }                                                               \
        return x;                                                       \
    }

#define pending                                                         \
    (ExceptGetContext(pC)->pEx->state == PENDING)

extern Scope    ExceptGetScope(Context *pC);
extern Context *ExceptGetContext(Context *pC);
extern void     ExceptThreadCleanup(int threadId);
extern void     ExceptTry(Context *pC, char *file, int line);
extern void     ExceptThrow(Context *pC, void * pExceptOrClass,
                            void *pData, char *file, int line);
extern int      ExceptCatch(Context *pC, ClassRef class);
extern int      ExceptFinally(Context *pC);
extern void     ExceptReturn(Context *pC);
extern int      ExceptCheckBegin(Context *pC, int *pChecked,
                                 char *file, int line);
extern int      ExceptCheck(Context *pC, int *pChecked, ClassRef class,
                            char *file, int line);
        

#endif  /* _EXCEPT_H */
