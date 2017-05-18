static char *rcsid = "$Id: aerohdr.cpp,v 1.4 2007/05/20 10:41:03 cvs Exp $";
/*
*
* $RCSfile: aerohdr.cpp,v $
* $Source: /cvs/common/aerohdr.cpp,v $
* $Author: cvs $
* $Revision: 1.4 $
* $Date: 2007/05/20 10:41:03 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerohdr.cpp,v $
* Revision 1.4  2007/05/20 10:41:03  cvs
* Simplified code somewhat
*
* Revision 1.3  2007/05/20 10:39:52  cvs
* More defensive coding added
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


LRESULT CALLBACK HeaderCtrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    PaintControl(hWnd, hdc, &ps.rcPaint, (pWndData->m_dwFlags & WD_DRAW_BORDER)!=0);
                EndPaint(hWnd, &ps);
                return 1;
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, HeaderCtrlProc, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}


BOOL AeroSubClassHeaderCtrl(HWND hwnd)
{
    return AeroSubClassControl(hwnd, HeaderCtrlProc, WD_IN_PAINT_CONTROL);
}
