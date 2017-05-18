static char *rcsid = "$Id: aerolctr.cpp,v 1.6 2007/05/20 13:33:40 cvs Exp $";
/*
*
* $RCSfile: aerolctr.cpp,v $
* $Source: /cvs/common/aerolctr.cpp,v $
* $Author: cvs $
* $Revision: 1.6 $
* $Date: 2007/05/20 13:33:40 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerolctr.cpp,v $
* Revision 1.6  2007/05/20 13:33:40  cvs
* Added comments and some more debug code and improved robustness
*
* Revision 1.5  2007/05/20 12:46:10  cvs
* More defensive coding and debug code added
*
* Revision 1.4  2007/05/04 15:38:39  cvs
* Removed macros and empty lines
*
* Revision 1.3  2007/05/04 11:39:14  cvs
* Drawing the border now in the edit border color
*
* Revision 1.2  2007/04/08 13:37:52  cvs
* Added standard header
* 
*/

#include <windows.h>
#include <tchar.h>
#include "safassrt.h"
#include "aaeroint.h"
#include "aerosubc.h"
#include "aeroglss.h"
#include <gdiplus.h>
using namespace Gdiplus;



LRESULT CALLBACK ListCtrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAERO_SUBCLASS_WND_DATA pWndData = (PAERO_SUBCLASS_WND_DATA)GetProp(hWnd, WINDOW_DATA_STRING);
    ASSERT(pWndData);
    ASSERT(pWndData->m_pDwmApiImpl);
    WNDPROC pOldProc = pWndData->m_oldWndProc;
    ASSERT(pOldProc);
    PAERO_SUBCLASS_WND_DATA pWndDataParent = (PAERO_SUBCLASS_WND_DATA)GetProp(GetParent(hWnd), WINDOW_DATA_STRING);

    /// 
    /// if aero glass is turned off and if we are not in destruction code, 
    /// just call the original wnd proc we had prior to subclassing:
    /// 
    if(WM_DESTROY!=uMsg && WM_NCDESTROY!=uMsg && pWndDataParent && !pWndData->m_pDwmApiImpl->IsDwmCompositionEnabled())
        return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);

    switch(uMsg)
    {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                if(hdc)
                {
                    RECT rc;
                    VERIFY(GetWindowRect(hWnd, &rc));
                    VERIFY(MapWindowPoints(NULL, hWnd, (LPPOINT) &rc, 2));

                    PaintControl(hWnd, hdc, &ps.rcPaint, (pWndData->m_dwFlags & WD_DRAW_BORDER)!=0);
                }
                EndPaint(hWnd, &ps);

                /// 
                /// now draw the 
                /// border around this control:
                /// 
                DrawEditBorder(hWnd, pWndData);
                return 1;
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, ListCtrlProc, pWndData));
            break;
    }
    
    LRESULT lRet = CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);

    switch(uMsg)
    {
        case WM_VSCROLL:
        case WM_HSCROLL:
        case WM_NCPAINT:
            {
                HWND hHeader = ListView_GetHeader(hWnd);
                if(hHeader)
                {
                    InvalidateRgn(hHeader, NULL, TRUE);
                    VERIFY(UpdateWindow(hHeader));
                }
                
            }
            break;
    }
    
    return lRet;
}






#define HDR_CLASSNAME _T("SysHeader32")

BOOL CALLBACK SubclassHeaderCtrl(
  HWND hwnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
    PBOOL pbSuccess = (PBOOL)lParam;
    ASSERT(pbSuccess);
    
    TCHAR szWndClassName[dimof(HDR_CLASSNAME)];
    if(GetClassName(hwnd, szWndClassName, dimof(szWndClassName)))
    {
        if  (!_tcscmp(szWndClassName, HDR_CLASSNAME))
            *pbSuccess = AeroSubClassHeaderCtrl(hwnd);
    }

    return TRUE;
}







BOOL AeroSubClassListCtrl(HWND hwnd)
{
    BOOL bReturn = AeroSubClassControl(hwnd, ListCtrlProc, WD_IN_PAINT_CONTROL);
    if(bReturn)
        EnumChildWindows(hwnd, SubclassHeaderCtrl, (LPARAM)&bReturn);
    return bReturn;
}
