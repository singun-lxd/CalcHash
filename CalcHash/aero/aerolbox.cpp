static char *rcsid = "$Id: aerolbox.cpp,v 1.4 2007/05/20 12:41:48 cvs Exp $";
/*
*
* $RCSfile: aerolbox.cpp,v $
* $Source: /cvs/common/aerolbox.cpp,v $
* $Author: cvs $
* $Revision: 1.4 $
* $Date: 2007/05/20 12:41:48 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerolbox.cpp,v $
* Revision 1.4  2007/05/20 12:41:48  cvs
* More defensive coding and debug code added
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


LRESULT CALLBACK ListBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    PaintControl(hWnd, hdc, &ps.rcPaint, false);
                    DrawEditBorder(hWnd, pWndData);
                }
                EndPaint(hWnd, &ps);
                return 1;
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, ListBoxProc, pWndData));
            break;
    }
    
    LRESULT lRet = CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
    

    switch(uMsg)
    {
        case WM_NCPAINT:
        case WM_VSCROLL:
            InvalidateRgn(hWnd, NULL, TRUE);
            VERIFY(UpdateWindow(hWnd));
            break;
    }

    return lRet;
}


BOOL AeroSubClassListBox(HWND hwnd)
{
    return AeroSubClassControl(hwnd, ListBoxProc, WD_IN_PAINT_CONTROL);
}
