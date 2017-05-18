static char *rcsid = "$Id: aerospin.cpp,v 1.3 2007/05/20 13:01:39 cvs Exp $";
/*
*
* $RCSfile: aerospin.cpp,v $
* $Source: /cvs/common/aerospin.cpp,v $
* $Author: cvs $
* $Revision: 1.3 $
* $Date: 2007/05/20 13:01:39 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerospin.cpp,v $
* Revision 1.3  2007/05/20 13:01:39  cvs
* More defensive coding and debug code added
*
* Revision 1.2  2007/04/15 15:03:57  cvs
* Renamed subclass function
*
* Revision 1.1  2007/04/09 16:16:28  cvs
* Added spin control code
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



static LRESULT CALLBACK SpinnerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    RECT rcClient;
                    VERIFY(GetClientRect(hWnd, &rcClient));
                    HTHEME hTheme = pWndData->m_pUxTheme->OpenThemeData(hWnd, L"Spin");
                    if(hTheme)
                    {
                        HDC hdcPaint = NULL;
                        BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                        params.dwFlags        = BPPF_ERASE;
                        HPAINTBUFFER hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, 
                            &params, &hdcPaint);
                        if (hdcPaint)
                        {
                            VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));
                            SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM) hdcPaint, PRF_CLIENT|PRF_CHECKVISIBLE);
                            VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
                        }   
                    
                        RECT rcBorder = rcClient;
                        rcBorder.right = rcBorder.left+1;

                        hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcBorder, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                        if (hdcPaint)
                        {
                            VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));
                            VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
                        }   
                    
                        rcBorder = rcClient;
                        rcBorder.left = rcBorder.right-1;

                        hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcBorder, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                        if (hdcPaint)
                        {
                            VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));
                            VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
                        }   
                    
                    
                        rcBorder = rcClient;
                        rcBorder.bottom = rcBorder.top+1;

                        hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcBorder, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                        if (hdcPaint)
                        {
                            VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));
                            VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
                        }   
                    
                        /// 
                        /// horizontal spin controls don't have a 1 Pixel border at the lower edge:
                        /// 
                        if (!(GetWindowLongPtr(hWnd, GWL_STYLE)&UDS_HORZ))
                        {
                            rcBorder = rcClient;
                            rcBorder.top = rcBorder.bottom-1;

                            hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcBorder, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                            if (hdcPaint)
                            {
                                VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));
                                VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
                            }   
                        }

                        VERIFY(S_OK==pWndData->m_pUxTheme->CloseThemeData(hTheme));
                    }
                }


                EndPaint(hWnd, &ps);
                return 1;
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, SpinnerProc, pWndData));
            break;
    }
    
    return  CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}


BOOL AeroSubClassSpinCtrl(HWND hwnd)
{
    return AeroSubClassControl(hwnd, SpinnerProc, WD_IN_PAINT_CONTROL);
}
