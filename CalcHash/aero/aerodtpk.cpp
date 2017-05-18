static char *rcsid = "$Id: aerodtpk.cpp,v 1.4 2007/05/20 10:33:49 cvs Exp $";
/*
*
* $RCSfile: aerodtpk.cpp,v $
* $Source: /cvs/common/aerodtpk.cpp,v $
* $Author: cvs $
* $Revision: 1.4 $
* $Date: 2007/05/20 10:33:49 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerodtpk.cpp,v $
* Revision 1.4  2007/05/20 10:33:49  cvs
* More defensive coding added
*
* Revision 1.3  2007/05/20 10:23:54  cvs
* More defensive coding added
*
* Revision 1.2  2007/05/18 22:21:41  cvs
* Removed dead code
*
* Revision 1.1  2007/04/10 09:17:29  cvs
* Added date time pick control
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




LRESULT CALLBACK DateTimePickProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                
                    BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                    params.dwFlags        = BPPF_ERASE;

                    HDC hdcPaint = NULL;
                    HPAINTBUFFER hBufferedPaint = NULL;
                    hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                    if (hdcPaint)
                    {
                        VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));
                        SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM)hdcPaint, PRF_CLIENT|PRF_ERASEBKGND |PRF_NONCLIENT|PRF_CHECKVISIBLE);

                        VERIFY(S_OK==pWndData->m_pUxTheme->BufferedPaintMakeOpaque_(hBufferedPaint, &rcClient));
                        VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));    
                    }
                }
                EndPaint(hWnd, &ps);
                return 1;
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, DateTimePickProc, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}


BOOL AeroSubClassDateTimePick(HWND hwnd)
{
    return AeroSubClassControl(hwnd, DateTimePickProc, WD_IN_PAINT_CONTROL);
}

