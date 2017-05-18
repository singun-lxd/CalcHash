static char *rcsid = "$Id: aeroedit.cpp,v 1.12 2007/05/20 10:38:25 cvs Exp $";
/*
*
* $RCSfile: aeroedit.cpp,v $
* $Source: /cvs/common/aeroedit.cpp,v $
* $Author: cvs $
* $Revision: 1.12 $
* $Date: 2007/05/20 10:38:25 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aeroedit.cpp,v $
* Revision 1.12  2007/05/20 10:38:25  cvs
* Added some more debug code
*
* Revision 1.11  2007/05/19 13:54:58  cvs
* Removed warning in x64 build
*
* Revision 1.10  2007/05/18 22:09:22  cvs
* Fixed orphaned caret bug
*
* Revision 1.9  2007/05/18 21:52:19  cvs
* Removed the multiline edit subclassing code
*
* Revision 1.8  2007/05/18 21:30:09  cvs
* Rewrote and simplified subclassing for single line edit controls
*
* Revision 1.7  2007/05/06 14:25:30  cvs
* x64 compatibility changes
*
* Revision 1.6  2007/05/04 15:38:16  cvs
* Removed macros
*
* Revision 1.5  2007/04/22 18:55:04  cvs
* Added DrawEditBorder
*
* Revision 1.4  2007/04/22 15:42:04  cvs
* Now painting multiline edit control background properly if disabled
*
* Revision 1.3  2007/04/22 12:34:33  cvs
* Removed TRACE statements
*
* Revision 1.2  2007/04/08 13:37:51  cvs
* Added standard header
* 
*/

#include <windows.h>
#include <tchar.h>
#include "safassrt.h"
#include "aaeroint.h"
#include "aerosubc.h"
#include "aeroglss.h"
#include <windowsx.h>
#include <gdiplus.h>
using namespace Gdiplus;

static void UpdateIfSelChanged(HWND hWnd, PAERO_SUBCLASS_WND_DATA pWndData)
{
    DWORD dwFirst, dwLast;
    SendMessage(hWnd, EM_GETSEL, (WPARAM)&dwFirst, (LPARAM)&dwLast);
    if(dwFirst!=pWndData->m_dwSelFirst || dwLast!=pWndData->m_dwSelLast)
    {
        pWndData->m_dwSelFirst = dwFirst;
        pWndData->m_dwSelLast = dwLast;
        VERIFY(InvalidateRect(hWnd, NULL, TRUE));
        VERIFY(UpdateWindow(hWnd));
    }
}


static LRESULT CALLBACK EditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
    if(WM_DESTROY!=uMsg && WM_NCDESTROY!=uMsg && WM_DWMCOMPOSITIONCHANGED!=uMsg && pWndDataParent && !pWndData->m_pDwmApiImpl->IsDwmCompositionEnabled())
        return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);



    if(pWndData->m_uiRedrawMsg==uMsg && pWndData->m_dwFlags & WD_IN_PAINT_CONTROL)
    {
        HDC hdc = GetDC(hWnd);
        hdc = GetDC(hWnd);
        if(hdc)
        {
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
            params.dwFlags        = 0L;//BPPF_ERASE;
            HDC hdcPaint = NULL;
            HPAINTBUFFER hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, &params, &hdcPaint);
            if (hdcPaint)
            {
                LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
                DWORD_PTR dwSyscolorIdx = (dwStyle&WS_DISABLED || dwStyle&ES_READONLY)?COLOR_3DFACE:COLOR_WINDOW;
                VERIFY(FillRect(hdcPaint, &rcClient, (HBRUSH)(dwSyscolorIdx+1)));
                
                SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM) hdcPaint, PRF_CLIENT|PRF_CHECKVISIBLE);
                
                /// Make every pixel opaque
                VERIFY(S_OK==pWndData->m_pUxTheme->BufferedPaintMakeOpaque_(hBufferedPaint, &rcClient));
                VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));    
            }

            VERIFY(1==ReleaseDC(hWnd, hdc));
            pWndData->m_dwFlags &= ~WD_IN_PAINT_CONTROL;
        }

        return 1;
    }

    switch(uMsg)
    {
        case WM_KEYDOWN:
        {    
            LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
            if(dwStyle&WS_VSCROLL || dwStyle&ES_MULTILINE)
            {
                if(!(pWndData->m_dwFlags&WD_CARET_HIDDEN))
                {
                    HideCaret(hWnd);
                    pWndData->m_dwFlags|=WD_CARET_HIDDEN;
                }
            }
        }
            break;
        case WM_KEYUP:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MOUSELEAVE:
        {
            LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
            if(dwStyle&WS_VSCROLL || dwStyle&ES_MULTILINE)
            {
                if(pWndData->m_dwFlags&WD_CARET_HIDDEN)
                {
                    ShowCaret(hWnd);
                    pWndData->m_dwFlags&=~WD_CARET_HIDDEN;
                }
            
                UpdateIfSelChanged(hWnd, pWndData);
            }
        }
            break;
        case WM_NCPAINT:
            {
                LRESULT lRes = 0;
                lRes = CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
                DrawEditBorder(hWnd, pWndData);
                return lRes;
            }
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, EditProc, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}




BOOL AeroSubClassEdit(HWND hwnd)
{
    return AeroSubClassControl(hwnd, EditProc, WD_IN_PAINT_CONTROL);
}


