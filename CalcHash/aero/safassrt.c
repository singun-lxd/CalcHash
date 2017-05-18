static char *rcsid = "$Id: safassrt.c,v 1.4 2006/07/30 11:56:11 cvs Exp $";
/*
*
* $RCSfile: safassrt.c,v $
* $Source: /cvs/common/safassrt.c,v $
* $Author: cvs $
* $Revision: 1.4 $
* $Date: 2006/07/30 11:56:11 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: safassrt.c,v $
* Revision 1.4  2006/07/30 11:56:11  cvs
* added deprecation macro for VS2005 builds
*
* Revision 1.3  2005/05/13 08:52:23  cvs
* Changed a lot of things to get rid of two different ASSERTs and allow for ASSERT MessageBoxes to even appear from services running under distinct user accounts. ALL ASSERTs are now actually inlined macros like in the MSVCRT.
*
* Revision 1.2  2003/10/17 15:54:11  cvs
* Now printing out the correct line and file information
*
* Revision 1.1  2003/08/23 14:20:14  cvs
* no message
* 
*/
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning (disable:4115 4305)
#include <windows.h>
#pragma warning (default:4115 4305)


#include "safassrt.h"



#ifdef _DEBUG
#include <stdio.h>
#include <signal.h>


#if defined (_M_IX86)
#define _DbgBreak() __asm { int 3 }
#elif defined (_M_ALPHA)
void _BPT();
#pragma intrinsic(_BPT)
#define _DbgBreak() _BPT()
#else  /* defined (_M_ALPHA) */
#define _DbgBreak() DebugBreak()
#endif  /* defined (_M_ALPHA) */


// anybuf _IOYOURBUF: stolen from the runtime's file2.h header file: 
#define anybuf(s)       ((s)->_flag & (_IOMYBUF|_IONBF|_IOYOURBUF))
#define _IOYOURBUF      0x0100

#ifndef MB_SERVICE_NOTIFICATION
#define MB_SERVICE_NOTIFICATION    0x00040000L
#endif //MB_SERVICE_NOTIFICATION


/*
 * assertion format string for use with output to stderr
 */
static char _assertstring[] = "Assertion failed: %s, file %s, line %d\n";
static char * dblnewline = "\n\n";
static char * dotdotdot = "...";
static char * newline = "\n";


#define MAXLINELEN  60 /* max length for line in message box */
#define ASSERTBUFSZ (MAXLINELEN * 9) /* 9 lines in message box */
#define BOXINTRO    "Assertion failed!"
#define PROGINTRO   "Program: "
#define FILEINTRO   "File: "
#define LINEINTRO   "Line: "
#define EXPRINTRO   "Expression: "
#define INFOINTRO   "For information on how your program can cause an assertion\n" \
                    "failure, see the Visual C++ documentation on asserts"
#define HELPINTRO   "(Press Retry to debug the application - JIT must be enabled)"
#define NEWLINESZ   1
#define DOTDOTDOTSZ 3
#define DBLNEWLINESZ   2

/// local prototypes:
static int __cdecl ___crtMessageBoxA(LPCSTR lpText, LPCSTR lpCaption, UINT uType);


/***
*_assert() - Display a message and abort
*
*Purpose:
*       The assert macro calls this routine if the assert expression is
*       true.  By placing the assert code in a subroutine instead of within
*       the body of the macro, programs that call assert multiple times will
*       save space.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

/*static void __cdecl __assert (
        void *expr,
        void *filename,
        unsigned lineno
        )*/
void SafeAssert(const char *expr, const char * filename, unsigned lineno, int __error_mode)
{
        /*
         * Build the assertion message, then write it out. The exact form
         * depends on whether it is to be written out via stderr or the
         * MessageBox API.
         */
        if ( (__error_mode == _OUT_TO_STDERR) || ((__error_mode ==
               _OUT_TO_DEFAULT) /*&& (__app_type == _CONSOLE_APP)*/) )
        {
            /*
             * Build message and write it out to stderr. It will be of the
             * form:
             *        Assertion failed: <expr>, file <filename>, line <lineno>
             */
            if ( !anybuf(stderr) )
            /*
             * stderr is unused, hence unbuffered, as yet. set it to
             * single character buffering (to avoid a malloc() of a
             * stream buffer).
             */
             (void) setvbuf(stderr, NULL, _IONBF, 0);

            fprintf(stderr, _assertstring, expr, filename, lineno);
            fflush(stderr);
        }
        else {
            int nCode;
            char * pch;
            char assertbuf[ASSERTBUFSZ];
            char progname[MAX_PATH];

            /*
             * Line 1: box intro line
             */
            strcpy( assertbuf, BOXINTRO );
            strcat( assertbuf, dblnewline );

            /*
             * Line 2: program line
             */
            strcat( assertbuf, PROGINTRO );

            if ( !GetModuleFileNameA( NULL, progname, MAX_PATH ))
                strcpy( progname, "<program name unknown>");

            pch = (char *)progname;

            /* sizeof(PROGINTRO) includes the NULL terminator */
            if ( sizeof(PROGINTRO) + strlen(progname) + NEWLINESZ > MAXLINELEN )
            {
                pch += (sizeof(PROGINTRO) + strlen(progname) + NEWLINESZ) - MAXLINELEN;
                strncpy( pch, dotdotdot, DOTDOTDOTSZ );
            }

            strcat( assertbuf, pch );
            strcat( assertbuf, newline );

            /*
             * Line 3: file line
             */
            strcat( assertbuf, FILEINTRO );

            /* sizeof(FILEINTRO) includes the NULL terminator */
            if ( sizeof(FILEINTRO) + strlen(filename) + NEWLINESZ >
                 MAXLINELEN )
            {
                /* too long. use only the first part of the filename string */
                strncat( assertbuf, filename, MAXLINELEN - sizeof(FILEINTRO)
                         - DOTDOTDOTSZ - NEWLINESZ );
                /* append trailing "..." */
                strcat( assertbuf, dotdotdot );
            }
            else
                /* plenty of room on the line, just append the filename */
                strcat( assertbuf, filename );

            strcat( assertbuf, newline );

            /*
             * Line 4: line line
             */
            strcat( assertbuf, LINEINTRO );
            _itoa( lineno, assertbuf + strlen(assertbuf), 10 );
            strcat( assertbuf, dblnewline );

            /*
             * Line 5: message line
             */
            strcat( assertbuf, EXPRINTRO );

            /* sizeof(HELPINTRO) includes the NULL terminator */

            if (    strlen(assertbuf) +
                    strlen(expr) +
                    2*DBLNEWLINESZ +
                    sizeof(INFOINTRO)-1 +
                    sizeof(HELPINTRO) > ASSERTBUFSZ )
            {
                strncat( assertbuf, expr,
                    ASSERTBUFSZ -
                    (strlen(assertbuf) +
                    DOTDOTDOTSZ +
                    2*DBLNEWLINESZ +
                    sizeof(INFOINTRO)-1 +
                    sizeof(HELPINTRO)) );
                strcat( assertbuf, dotdotdot );
            }
            else
                strcat( assertbuf, expr );

            strcat( assertbuf, dblnewline );

            /*
             * Line 6, 7: info line
             */

            strcat(assertbuf, INFOINTRO);
            strcat( assertbuf, dblnewline );

            /*
             * Line 8: help line
             */
            strcat(assertbuf, HELPINTRO);

            /*
             * Write out via MessageBox
             */

            nCode = ___crtMessageBoxA(assertbuf,
                "Microsoft Visual C++ Runtime Library",
                MB_DEFAULT_DESKTOP_ONLY|MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

            /* Abort: abort the program */
            if (nCode == IDABORT)
            {
                /* raise abort signal */
                raise(SIGABRT);

                /* We usually won't get here, but it's possible that
                   SIGABRT was ignored.  So exit the program anyway. */

                _exit(3);
            }

            /* Retry: call the debugger */
            if (nCode == IDRETRY)
            {
                _DbgBreak();
                /* return to user code */
                return;
            }

            /* Ignore: continue execution */
            if (nCode == IDIGNORE)
                return;
        }

        abort();
}



/***
*__crtMessageBox - call MessageBoxA dynamically.
*
*Purpose:
*       Avoid static link with user32.dll. Only load it when actually needed.
*
*Entry:
*       see MessageBoxA docs.
*
*Exit:
*       see MessageBoxA docs.
*
*Exceptions:
*
*******************************************************************************/
static int __cdecl ___crtMessageBoxA(
        LPCSTR lpText,
        LPCSTR lpCaption,
        UINT uType
        )
{
        static int (APIENTRY *pfnMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT) = NULL;
        static HWND (APIENTRY *pfnGetActiveWindow)(void) = NULL;
        static HWND (APIENTRY *pfnGetLastActivePopup)(HWND) = NULL;

        HWND hWndParent = NULL;

        if (NULL == pfnMessageBoxA)
        {
            HANDLE hlib = LoadLibraryA("user32.dll");

            if (NULL == hlib || NULL == (pfnMessageBoxA =
                        (int (APIENTRY *)(HWND, LPCSTR, LPCSTR, UINT))
                        GetProcAddress(hlib, "MessageBoxA")))
                return 0;

            pfnGetActiveWindow = (HWND (APIENTRY *)(void))
                        GetProcAddress(hlib, "GetActiveWindow");

            pfnGetLastActivePopup = (HWND (APIENTRY *)(HWND))
                        GetProcAddress(hlib, "GetLastActivePopup");
        }

        if (pfnGetActiveWindow)
            hWndParent = (*pfnGetActiveWindow)();

        if (hWndParent != NULL && pfnGetLastActivePopup)
            hWndParent = (*pfnGetLastActivePopup)(hWndParent);

        return (*pfnMessageBoxA)(hWndParent, lpText, lpCaption, uType);
}



#endif

