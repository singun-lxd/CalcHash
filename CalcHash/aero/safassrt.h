#ifndef __SAFASSRT_H__
#define __SAFASSRT_H__

/*
*
* $RCSfile: safassrt.h,v $
* $Source: /cvs/common/safassrt.h,v $
* $Author: cvs $
* $Revision: 1.5 $
* $Date: 2005/09/11 08:36:27 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: safassrt.h,v $
* Revision 1.5  2005/09/11 08:36:27  cvs
* Added ASSERT as a define on __ASSERT, same with VERIFY
*
* Revision 1.4  2005/05/13 08:52:23  cvs
* Changed a lot of things to get rid of two different ASSERTs and allow for ASSERT MessageBoxes to even appear from services running under distinct user accounts. ALL ASSERTs are now actually inlined macros like in the MSVCRT.
*
* Revision 1.3  2004/05/07 15:07:05  cvs
* Added a dummy function for release builds of __VERIFY so it doesn't throw warnings anymore
*
* Revision 1.2  2003/10/17 15:54:11  cvs
* Now printing out the correct line and file information
*
* Revision 1.1  2003/08/23 14:20:14  cvs
* no message
* 
*/


#ifdef _DEBUG
#include <assert.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

void SafeAssert(const char *szExpr, const char * szFile, unsigned lineno, int iErrMode);

#ifdef __cplusplus
}
#endif


#define __ASSERT(exp) (void)((exp) || (SafeAssert("__ASSERT failed for\n" #exp,__FILE__, __LINE__, _OUT_TO_MSGBOX),0))
#define __VERIFY(exp)   (void)((exp) || (SafeAssert("__VERIFY failed for\n" #exp,__FILE__, __LINE__, _OUT_TO_MSGBOX),0))
#define __ASSERT_EX(exp, comment) (void)((exp) || (SafeAssert("__ASSERT failed for\n" #exp "\n" comment,__FILE__, __LINE__, _OUT_TO_MSGBOX),0))
#define __VERIFY_EX(exp, comment)   (void)((exp) || (SafeAssert("__VERIFY failed for\n" #exp "\n" comment,__FILE__, __LINE__, _OUT_TO_MSGBOX),0))

#else //// _DEBUG

//#define TURNOFFWARNING (a) {#pragma warning (disable 4553) a}

#define __ASSERT(a)  ((void)0)
#define __VERIFY(a)  ((void)(a))
#define __ASSERT_EX(a, b)  ((void)0)
#define __VERIFY_EX(a, b)  ((void)(a))
#endif


#ifndef ASSERT 
#define  ASSERT __ASSERT
#endif // ASSERT 

#ifndef VERIFY 
#define  VERIFY __VERIFY
#endif // VERIFY 




#endif
