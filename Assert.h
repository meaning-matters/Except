/* 
 *      Assert.h - assertion check module header
 *
 *  DESCRIPTION
 *      This header file defines the assert(), validate() and check() macros.
 *      The assert() macro replaces the one defined in the standard C header
 *      "assert.h".
 *
 *      When DEBUG is defined, all three macros will invoke AssertAction()
 *      which performs failed assertion processing.  Having a routine instead
 *      of only a macro (like the standard C assert() macro), allows placing a
 *      debugger breakpoint.
 *
 *      When DEBUG is not defined then assert() is an empty macro; validate()
 *      and check() will still perform the test, but won't call AssertAction()
 *      on failure.  Instead, validate() will let the current routine return
 *      its second parameter.  When this routine returns void, NOTHING must be
 *      used as second parameter (NOTHING is empty, and it will satisfy the 
 *      preprocessor).  And on failure, the check() macro will throw an excep-
 *      tion with its second parameter as exception number.
 *
 *      The validate() and check() macros can be used in situations where a 
 *      failure is severe enough to cause a failed assertion during DEBUG, and
 *      where a defined action (either returning or throwing) is required in
 *      the production version (preventing (immediate) application failure).
 *
 *      In contrary with standard C, this assert() will not invoke abort() by
 *      default; so the application will not stop on the first failure, which
 *      seems the preferred choice during DEBUG.  Defining ASSERT_ABORT will
 *      enable aborting.
 *
 *      The shortest way of defining assert(e) is:
 *
 *              if (!(e))
 *                  AssertAction(pC, DO_ABORT, #e, __FILE__, __LINE__);
 *
 *      but this would lead to erroneous code when used in a construct like:
 *
 *              if (...)
 *                  assert(...);
 *              else
 *              {
 *              }
 *
 *      because the preprocessor output would be (indented as the compiler
 *      sees it):
 *
 *              if (...)
 *                  if (!(...))
 *                      AssertAction(pC, 0, "...", "file,c", 123);
 *                  else
 *                  {
 *                  }
 *
 *      The 'else' becomes part of the assertion check instead of the user 'if'
 *      statement!  To prevent situations like this: "dangling 'if'", the
 *      'if' statement in the macros below all have an 'else' clause.
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

#ifndef _ASSERT_H
#define _ASSERT_H

#include "Except.h"

#ifdef  ASSERT_ABORT
#define DO_ABORT        1
#else
#define DO_ABORT        0
#endif

#undef  assert

#ifdef  DEBUG
#define assert(e)                                                       \
        if (e)                                                          \
        {                                                               \
            /* prevent dangling 'if' */                                 \
        }                                                               \
        else                                                            \
        {                                                               \
            AssertAction(pC, DO_ABORT, #e, __FILE__, __LINE__);         \
        }
#else   /* DEBUG */
#define assert(e)
#endif  /* DEBUG */

#define NOTHING         /* 'void' validate return value */

#define validate(e, r)                                                  \
        if (e)                                                          \
        {                                                               \
            /* prevent dangling 'if' */                                 \
        }                                                               \
        else                                                            \
        {                                                               \
            assert(e);                                                  \
            return r;                                                   \
        }

#define check(e, n)                                                     \
        if (e)                                                          \
        {                                                               \
            /* prevent dangling 'if' */                                 \
        }                                                               \
        else                                                            \
        {                                                               \
            assert(e);                                                  \
            throw(n, NULL);                                             \
        }


extern
void AssertAction(
    Context *   pC,             /* pointer to thread exception context */
    int         doAbort,        /* flag if abort() must be invoked */
    char *      expr,           /* failed expression string */
    char *      file,           /* name of source file where failed */
    int         line);          /* source file line number */


#endif  /* _ASSERT_H */

