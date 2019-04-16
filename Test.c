/*
 *      Test.c - tests for C exception handling package (single threaded)
 *
 *  DESCRIPTION
 *      This program contains tens of test routines for the Java like
 *      exception handling package for C.  It is impossible to fully
 *      automate all the tests.  In this version of the program the
 *      result of all tests needs to be verifid by visual inspection of
 *      the output.  For each test the programs prints the expected
 *      result, which will be printed immediately after.
 *
 *  TODO
 *      Badly needs to be updated (and cleaned up) to cover all features
 *      and check all functionality boundaries.
 */

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include "Except.h"
#include "Alloc.h"
#ifndef DEBUG
#define DEBUG           /* switch on assertion checking */
#endif
#include "Assert.h"

except_class_declare(Level1Exception, Exception);
except_class_declare(Level2Exception, Level1Exception);
except_class_define(Level1Exception, Exception);
except_class_define(Level2Exception, Level1Exception);

int     testNum = 1;

int main(void);

static void TestThrow(void)
{
    printf("\nTHROW TESTS -------------------------------------------\n\n");

    /* see if exception is lost outside */
    printf("-->%2d: Lost Exception?\n", testNum++);
    throw (Exception, NULL);
    printf("\n");

    /* see if throw works and if correct catch clause is selected */
    try
    {
        printf("-->%2d: Caught Exception?\n", testNum++);
        throw (Exception, NULL);
    }
    catch (RuntimeException, e);
    catch (Exception, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    /* see if throw works properly from within catch clause */
    try
    {
        throw (Exception, NULL);
    }
    catch (Exception, e)
    {
        printf("-->%2d: Lost Level1Exception?\n", testNum++);
        throw (Level1Exception, NULL);
    }
    catch (Level1Exception, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    /* see if exception class inheritance works in the right direction */
    try
    {
        printf("-->%2d: Lost Level1Exception?\n", testNum++);
        throw (Level1Exception, NULL);
    }
    catch (Level2Exception, e);
    finally;
    printf("\n");

    /* again see if exception class inheritance works in the right direction */
    try
    {
        printf("-->%2d: Caught Level2Exception?\n", testNum++);
        throw (Level2Exception, NULL);
    }
    catch (Level1Exception, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    try;
    finally
    {
        printf("-->%2d: Lost Exception?\n", testNum++);
        throw (Exception, NULL);
    }
    printf("\n");
}

int t1(void)
{
    try
    {
        return(6);
    }
    catch (Throwable, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    return(7);
}

int t2(void)
{
    try
    {
        return(6);
    }
    finally
    {
        return(7);
    }
}

int t3(void)
{
    try
    {
        assert(0);
    }
    catch (FailedAssertion, e)
    {
        return(8);
    }
    finally;
}

int t4(void)
{
    try
    {
        assert(0);
    }
    catch (FailedAssertion, e)
    {
        return(8);
    }
    finally
    {
        return(9);
    }
}

int tX(void)
{
    try
    {
        try
        {
            try
            {
                return(1);
            }
            finally
            {
                printf("A ");
            }
        }
        finally
        {
            printf("B ");
        }
    }
    finally
    {
        printf("C ");
    }

    return 2;
}

int spell(void)
{
    if (1)
        try
            try 
                try
                    throw (Throwable, 0);
                catch (Throwable, e)
                    return(1);
                finally
                {
                    printf("A");
                    return(2);
                }
        
            catch (Throwable, e)
                printf("Magic");
            finally
                printf("B");
    
        finally
            printf("C");
    return(3);
}

static int TestReturn(void)
{
    printf("\nRETURN TESTS ------------------------------------------\n\n");

    printf("-->%2d: Returns 6?\n", testNum++);
    printf("Return value = %d\n", t1());
    printf("\n");

    printf("-->%2d: Returns 7?\n", testNum++);
    printf("Return value = %d\n", t2());
    printf("\n");

    printf("-->%2d: Returns 8?\n", testNum++);
    printf("Return value = %d\n", t3());
    printf("\n");

    printf("-->%2d: Returns 9?\n", testNum++);
    printf("Return value = %d\n", t4());
    printf("\n");

    printf("-->%2d: Prints \"ABC2\"?\n", testNum++);
    try
    {
        printf("%d\n", spell());
    }
    catch (Throwable, e)
        printf("%s\n", e->getMessage());
    finally;
    printf("\n");

    printf("-->%2d: Prints \"A B C 1\"?\n", testNum++);
    printf("%d\n", tX());
    printf("\n");    
}

static void t5(void)
{
    try
    {
        raise(SIGABRT);
    }
    catch (SegmentationFault, e);
    catch (IllegalInstruction, e);
    finally;
}

static void t6(void)
{
    try
    {
        *((int *)0) = 0;        /* may not be portable */
    }
    catch (SegmentationFault, e)
    {
        long f[] = { 1, 2, 3 };
        ((void(*)())f)();       /* may not be portable */
    }
    finally
    {
    }
}

static int t7(void)
{
#if 0
    try
    {
        *((int *)0) = 0;        /* may not be portable */
    }
    catch (SegmentationFault, e)
    {
        long f[] = { 'i', 'l', 'l', 'e', 'g', 'a', 'l' };
        ((void(*)())f)();       /* may not be portable */
    }
    finally
    {
        return(1 / strcmp("", ""));
    }
#else
    static double       n;

    return 0.0 / n;
#endif
}

static int t8(void)
{
    try
    {
    }
    finally
    {
        *((int *)0) = 0;
    }
}

static void TestSignal(void)
{
    printf("\nSIGNAL TESTS ------------------------------------------\n\n");

    try
    {
        char*   p = "segment";
        printf("-->%2d: Violates segmentation?\n", testNum++);
        
        while (1)
        {
            *p++ = ' ';
        }
        
        //*((long *)main) = 0;  /* might not be portable */
    }
    catch (SegmentationFault, e)
    {
        printf("%s\n", e->getMessage());
    }
    catch (RuntimeException, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    try
    {
        printf("-->%2d: Aborts?\n", testNum++);
        t5();
    }
    catch (AbnormalTermination, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    try
    {
        printf("-->%2d: Illegal instruction?\n", testNum++);
        t6();
    }
    catch (RuntimeException, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    try
    {
        printf("-->%2d: Arithmetic?\n", testNum++);
        t7();
    }
    catch (Throwable, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");
    
    try
    {
        printf("-->%2d: RuntimeException (in finally)?\n", testNum++);
        t8();
    }
    catch (RuntimeException, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
}

static void TestMemory(void)
{
    printf("\nMEMORY TESTS ------------------------------------------\n\n");

    try
    {
        void *  p;

        printf("-->%2d: Out of memory?\n", testNum++);
        p = malloc(INT_MAX);    /* calloc() and realloc() are similar */
    }
    catch (OutOfMemoryError, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    try
    {
        void *  p;

        printf("-->%2d: Out of memory?\n", testNum++);
        p = new(int[INT_MAX/4]);
    }
    catch (OutOfMemoryError, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    try
    {
        void *  p;

        printf("-->%2d: Not out of memory?\n", testNum++);
        p = new(int);
    }
    catch (OutOfMemoryError, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally
    {
        printf("Enough memory left.\n");
    }
    printf("\n");
}

static void TestNesting()
{
    printf("\nNESTING TESTS -----------------------------------------\n\n");

    try
    {
        try
        {
            try
            {
                printf("-->%2d: Throws Level2Exception?\n", testNum++);
                throw (Level2Exception, NULL);
            }
            catch (RuntimeException, e);
            catch (OutOfMemoryError, e);
            finally;
        }
        catch (FailedAssertion, e);
        finally;
    }
    catch (RuntimeException, e);
    catch (Exception, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    try
    {
        try
        {
            try
            {
                throw (Level2Exception, NULL);
            }
            catch (OutOfMemoryError, e);
            catch (RuntimeException, e);
            finally;
        }
        catch (Level2Exception, e)
        {
            try
            {
                printf("-->%2d: Throws Level1Exception?\n", testNum++);
                throw (Level1Exception, NULL);
            }
            catch (RuntimeException, e);
            finally;
        }
        finally;
    }
    catch (RuntimeException, e);
    catch (Level2Exception, e);
    catch (Level1Exception, e)
    {
        e->printTryTrace(0);
    }
    finally;
    printf("\n");

    try
    {
        try
        {
            try
            {
                throw (Exception, NULL);
            }
            catch (Exception, e)
            {
                throw (Level1Exception, NULL);
            }
            catch (Level1Exception, e)
            {
                throw (Throwable, NULL);
            }
            finally;
        }
        catch (Level1Exception, e)
        {
            printf("-->%2d: Throws Level2Exception?\n", testNum++);
            throw (Level2Exception, NULL);
        }
        catch (Throwable, e)
        {
            throw (Throwable, NULL);
        }
        finally;
    }
    catch (Throwable, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    try
    {
        try
        {
            throw (Exception, NULL);
        }
        finally
        {
            printf("-->%2d: Throws Level1Exception?\n", testNum++);
            throw (Level1Exception, NULL);
        }
    }
    catch (Exception, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally;
    printf("\n");

    try
    {
        printf("-->%2d: No Level1Exception caught?\n", testNum++);
        throw (Level1Exception, NULL);
    }
    catch (Level1Exception, e);
    catch (Level1Exception, e)
    {
        printf("%s\n", e->getMessage());
    }
    finally
    {
        printf("Nothing caught.\n");
    }
    printf("\n");

    try
        try
        {
            printf("-->%2d: Rethrow test: prints \"Hello\"?\n", testNum++);
            throw (Exception, "Hello");
        }
        catch (Exception, e)
            throw (e, "there!");
        finally;
    catch (Exception, e)
        printf("%s\n", e->getData());
    finally;
    printf("\n");

    printf("-->%2d: Does nothing (except for some warnings)?\n", testNum++);
    try
        try
            try
                raise(SIGABRT);
            catch (Throwable, e);
            finally;
        finally;
    finally
        printf("Nothing!\n");
    printf("\n");

    printf("-->%2d: Does nothing (except for some warnings)?\n", testNum++);
    try
        try
        finally
        {       /* required '{' */
            try
            finally;
        }
    finally
        printf("Nothing!\n");
    printf("\n");
}

static void TestAssert(void)
{
    printf("\nASSERT TESTS ------------------------------------------\n\n");

    printf("-->%2d: Failed assert line %d?\n", testNum++, __LINE__ + 1);
    assert(0);
    printf("\n");

    try
    {
        printf("-->%2d: Caught assert line %d?\n", testNum++, __LINE__ + 1);
        assert((0, 0, 0, 0, 0));
    }
    catch (FailedAssertion, e)
    {
        printf("%s -- %s\n", e->getMessage(), e->getData());
    }
    finally;
    printf("\n");

    try
    {
        assert(0);
    }
    catch (FailedAssertion, e)
    {
        printf("-->%2d: Failed assert line %d?\n", testNum++, __LINE__ + 1);
        assert(0);
    }
    finally;
    printf("\n");

    try;
    finally
    {
        printf("-->%2d: Failed assert line %d?\n", testNum++, __LINE__ + 1);
        assert(0);
    }
    printf("\n");
}

static int t9(void)
{
#undef  assert
#define assert(e)       /* as in "Assert.h" when DEBUG not defined */

    validate(0, 27);
}

static void TestValidate(void)
{
    printf("\nVALIDATE TESTS ----------------------------------------\n\n");

    printf("-->%2d: Returns 27?\n", testNum++);
    printf("Returned %d\n", t9());
    printf("\n");
}

static void TestCheck(void)
{
    printf("\nCHECK TESTS -------------------------------------------\n\n");

    printf("-->%2d: Superfluous catch Exception at line %d?\n", testNum++,
           __LINE__ + 3);
    try;
    catch (Throwable, e);
    catch (Exception, e);
    finally;
    printf("\n");

    printf("-->%2d: Superfluous catch Level2Exception at line %d?\n", testNum++,
            __LINE__ + 4);
    try;
    catch (Exception, e);
    catch (FailedAssertion, e);
    catch (Level2Exception, e);
    catch (RuntimeException, e);
    finally;
    printf("\n");

    printf("-->%2d: Two superfluous catches?\n", testNum++);
    try;
    catch (Throwable, e);
    catch (FailedAssertion, e);
    catch (Exception, e);
    finally;
    printf("\n");

    printf("-->%2d: Duplicate catch (SegmentationFault) at line %d?\n", 
           testNum++, __LINE__ + 4);
    try;
    catch (SegmentationFault, e);
    catch (FailedAssertion, e);
    catch (SegmentationFault, e);
    catch (RuntimeException, e);
    finally;
    printf("\n");

    printf("-->%2d: Warning: No catches?\n", testNum++);
    try;
    finally;
    printf("\n");
}

void recurse(int x)
{
    printf("recurse(%d)\n", x);

    try
    {
        if (x == 0)
            *((int *)0) = 0;
        recurse(x - 1);
    }
    finally
        printf("%d, ", x);
}

static void TestRecursion(void)
{
    printf("\nRECURSION TESTS ---------------------------------------\n\n");

    printf("-->%2d: Hits a run-time exception after 10 levels?\n", testNum++);
    try
        recurse(10);
    catch (RuntimeException, e)
        printf("%s\n", e->getMessage());
    finally;
    printf("\n");
}


void CheckStack(void)
{
    Context *pC = ExceptGetContext(NULL);

    if (pC != NULL && pC->exStack != NULL)
    {
        printf("LifoCount == %d != 0\n", LifoCount(pC->exStack));
    }
}


int main(void)
{
    TestThrow();
    CheckStack();

    TestReturn();
    CheckStack();

    TestMemory();
    CheckStack();

    TestNesting();
    CheckStack();

    TestAssert();
    CheckStack();

    TestValidate();
    CheckStack();

    TestCheck();
    CheckStack();

    TestRecursion();
    CheckStack();

    TestSignal();
    CheckStack();

    printf("\nREADY\n\n");
}
