/*
 *      Assert.c - assertion check module
 *
 *  DESCRIPTION
 *      This module contains a routine that is used by the assert(), validate()
 *      and check() macros which are defined in "Assert.h".  This routine must
 *      not be called by the user.
 *
 *      Having a routine for processing failed assertions, instead of having
 *      a macro only (like the standard C assert() macro), allows placing a
 *      debugger breakpoint.
 *
 *  INCLUDE FILES
 *      Assert.h
 *
 *  COPYRIGHT
 *      You are free to use, copy or modify this software at your own risk.
 *
 *  AUTHOR
 *      Cornelis van der Bent.  Please let me know if you have comments or find
 *      flaws: cg_vanderbent@mail.com.  Enjoy!
 *
 *  MODIFICATION HISTORY
 *      1999/05/14 vdbent       Changes for version 1.0.
 *      1999/04/12 vdbent       Thorough test and debugging; beta release.
 *      1998/12/18 vdbent       Conception.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Except.h"


/******************************************************************************
 *
 *      AssertAction - process failed assertion
 *
 *  DESCRIPTION
 *      This routine is invoked when an assertion has failed.  It is used by
 *      the macros assert(), validate() and check(), which are defined in
 *      "Assert.h".
 *
 *      When in exception handling context, it throws an EX_ASSERT exception;
 *      besides the file name and the line number, the exception string is also
 *      passed for the user to be retrieved inside a 'catch' clause using the
 *      e->getData() member function.
 *
 *      When not in exception handling context, the standard message will be
 *      printed on <stderr>.  In this situation the <doAbort> flag is used to
 *      determine if abort() is to be invoked (causing 'core dump') or not.
 *
 *  SIDE EFFECTS
 *      When <doAbort> is true, there is nothing more to say when not in
 *      exception handling context.  When in exception handling context an
 *      exception is thrown.
 *
 *  RETURNS
 *      N/A.
 */

void AssertAction(
    Context *   pC,             /* pointer to thread exception context */
    int         doAbort,        /* flag if abort() must be invoked */
    char *      expr,           /* failed expression string */
    char *      file,           /* name of source file where failed */
    int         line)           /* source file line number */
{
    Scope       scope = ExceptGetScope(pC);

    if (scope == TRY || scope == CATCH || scope == FINALLY)
    {
        if (pC == NULL)
            pC = ExceptGetContext(NULL);        /* should never return NULL */

        ExceptThrow(pC, FailedAssertion, expr, file, line);
    }
    else
    {
        fprintf(stderr, "Assertion failed %s: %s, file \"%s\", line %d.\n",
                doAbort ? "" : "(no abort)", expr, file, line);

        if (doAbort)
            abort();
    }
}


/* end of Assert.c */
