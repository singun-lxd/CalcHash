static char *rcsid = "$Id: aeroipad.cpp,v 1.7 2007/05/20 13:33:40 cvs Exp $";
/*
*
* $RCSfile: aeroipad.cpp,v $
* $Source: /cvs/common/aeroipad.cpp,v $
* $Author: cvs $
* $Revision: 1.7 $
* $Date: 2007/05/20 13:33:40 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aeroipad.cpp,v $
* Revision 1.7  2007/05/20 13:33:40  cvs
* Added comments and some more debug code and improved robustness
*
* Revision 1.6  2007/05/20 10:59:40  cvs
* Added comments and some more debug code and improved robustness
*
* Revision 1.5  2007/05/18 22:15:53  cvs
* Added ncpaint handler drawing a border around the control
*
* Revision 1.4  2007/05/18 21:54:50  cvs
* Removed separate edit control subclassing code
*
* Revision 1.3  2007/05/18 16:24:05  cvs
* Corrected subclassing order
*
* Revision 1.2  2007/05/18 15:58:26  cvs
* Improved edit child rendering
*
* Revision 1.1  2007/04/09 18:17:47  cvs
* Added IpAddress control code
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



typedef struct tagHDC_COUNT
{
    HDC m_hdcPaint;
    DWORD m_dwCount; // we draw the dots at the right side of the control, so we don't
                     // do it for the last edit control
}
HDC_COUNT, *PHDC_COUNT;



BOOL CALLBACK DrawIPEditCtrlDots(
  HWND hwnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
    PHDC_COUNT phdcCount = (PHDC_COUNT)lParam;
    
    if(phdcCount->m_dwCount)
    {    
        RECT rc;
        VERIFY(GetClientRect(hwnd, &rc));
        ClientToScreen(hwnd, &rc);
        ScreenToClient(GetParent(hwnd), &rc);
    
        rc.left = rc.right;
        rc.right+=2;
    
        HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, NULL);
        if(hFont)
        {
            HFONT hOldFont = (HFONT)SelectObject(phdcCount->m_hdcPaint, hFont);
            TEXTMETRIC tm;
            VERIFY(GetTextMetrics(phdcCount->m_hdcPaint, &tm));
            rc.bottom = rc.top + tm.tmAscent + tm.tmInternalLeading;
            VERIFY(hFont == SelectObject(phdcCount->m_hdcPaint, hOldFont));
        }
    

        rc.top = rc.bottom-2;
    
        FillRect(&rc, phdcCount->m_hdcPaint, Color(0xFF000000));
    }
    phdcCount->m_dwCount++;

    return TRUE;
}




#define EDIT_CLASSNAME _T("Edit")

BOOL CALLBACK SubclassIPEditCtrls(
  HWND hwnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
    PBOOL pbSuccess = (PBOOL)lParam;
    ASSERT(pbSuccess);
    
    TCHAR szWndClassName[dimof(EDIT_CLASSNAME)];
    if(GetClassName(hwnd, szWndClassName, dimof(szWndClassName)))
    {
        if  (!_tcscmp(szWndClassName, EDIT_CLASSNAME))
            *pbSuccess = AeroSubClassEdit(hwnd);
    }

    return TRUE;
}





static LRESULT CALLBACK IPAddressCtrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
        case WM_NCPAINT:
            {
                LRESULT lRes = 0;
                lRes = CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
                DrawEditBorder(hWnd, pWndData);
                return lRes;
            }
        case WM_CTLCOLOREDIT:
            {
                PAERO_SUBCLASS_WND_DATA pCtrlData = (PAERO_SUBCLASS_WND_DATA)GetProp((HWND)lParam, WINDOW_DATA_STRING);
                if(pCtrlData)
                    pCtrlData->m_dwFlags|=WD_IN_PAINT_CONTROL;
                PostMessage((HWND)lParam, pWndData->m_uiRedrawMsg, 0, NULL);
            }
            break;
        case WM_PAINT:
            {
                LRESULT lRes = CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
                
                
                HDC hdc = GetDC(hWnd);
                if(hdc)
                {
                    RECT rcClient;
                    VERIFY(GetClientRect(hWnd, &rcClient));

                    HDC hdcPaint = NULL;
                    BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                    params.dwFlags        = BPPF_ERASE;
                    HPAINTBUFFER hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, 
                        &params, &hdcPaint);
                    if (hdcPaint)
                    {
                        VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), WHITENESS));
                
                        HDC_COUNT hdcCount;
                        ZeroMemory(&hdcCount, sizeof(hdcCount));
                        hdcCount.m_hdcPaint = hdcPaint;
                        VERIFY(EnumChildWindows(hWnd, DrawIPEditCtrlDots, (LPARAM)&hdcCount));
                        VERIFY(S_OK==pWndData->m_pUxTheme->BufferedPaintMakeOpaque_(hBufferedPaint, &rcClient));
                        VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
                    }   
                    VERIFY(1==ReleaseDC(hWnd, hdc));
                }

                return lRes;
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, IPAddressCtrlProc, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}


BOOL AeroSubClassIPAddressCtrl(HWND hwnd)
{
    BOOL bReturn = AeroSubClassControl(hwnd, IPAddressCtrlProc, WD_IN_PAINT_CONTROL);
    if(bReturn)
        EnumChildWindows(hwnd, SubclassIPEditCtrls, (LPARAM)&bReturn);
    return bReturn;
}

