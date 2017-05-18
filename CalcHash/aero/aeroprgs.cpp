static char *rcsid = "$Id: aeroprgs.cpp,v 1.2 2007/05/20 12:48:48 cvs Exp $";
/*
*
* $RCSfile: aeroprgs.cpp,v $
* $Source: /cvs/common/aeroprgs.cpp,v $
* $Author: cvs $
* $Revision: 1.2 $
* $Date: 2007/05/20 12:48:48 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aeroprgs.cpp,v $
* Revision 1.2  2007/05/20 12:48:48  cvs
* Added some more debug code
*
* Revision 1.1  2007/05/18 11:24:36  cvs
* Added progress control subclassing
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




static LRESULT CALLBACK ProgressCtrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                RECT rc;
                VERIFY(GetWindowRect(hWnd, &rc));
                VERIFY(MapWindowPoints(NULL, hWnd, (LPPOINT) &rc, 2));

                if(hdc)
                {
                    PaintControl(hWnd, hdc, &rc, false);
                
                    /// 
                    /// we remove the ugly light gray spots in the 
                    /// four corners of the control by painting a 
                    /// black pixel on them. Even if you do custom
                    /// painting for the control, using the "PROGRESS"
                    /// theme and DrawThemeBackground, these flashes
                    /// still appear, so we definitely have to cheat 
                    /// and paint black pixels:
                    /// 
                    BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                    params.dwFlags        = 0L;
                    HDC hdcPaint = NULL;
                    params.dwFlags        = 0L;
                    HPAINTBUFFER hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rc, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                    if (hdcPaint)
                    {
                        COLORREF cr = RGB(0x00, 0x00, 0x00);
                        SetPixel(hdcPaint, 0, 0, cr);
                        SetPixel(hdcPaint, 0, RECTHEIGHT(rc) - 1, cr);
                        SetPixel(hdcPaint, RECTWIDTH(rc) - 1, 0, cr);
                        SetPixel(hdcPaint, RECTWIDTH(rc) - 1, RECTHEIGHT(rc) - 1, cr);

                        VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));    
                    }
                }
                
                EndPaint(hWnd, &ps);
                return 1;
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, ProgressCtrlProc, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}




BOOL AeroSubClassProgressCtrl(HWND hwnd)
{
    return AeroSubClassControl(hwnd, ProgressCtrlProc, 0);
}
