static char *rcsid = "$Id: aerotree.cpp,v 1.3 2007/05/20 13:08:41 cvs Exp $";
/*
*
* $RCSfile: aerotree.cpp,v $
* $Source: /cvs/common/aerotree.cpp,v $
* $Author: cvs $
* $Revision: 1.3 $
* $Date: 2007/05/20 13:08:41 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerotree.cpp,v $
* Revision 1.3  2007/05/20 13:08:41  cvs
* More defensive coding and debug code added
*
* Revision 1.2  2007/04/22 18:55:04  cvs
* Added DrawEditBorder
*
* Revision 1.1  2007/04/10 17:35:56  cvs
* Added tree control
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




static LRESULT CALLBACK TreeCtrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                LRESULT lRes = CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);

                RECT rcClient;
                VERIFY(GetClientRect(hWnd, &rcClient));
                HDC hdc=GetDC(hWnd);
                if(hdc)
                {
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

                    VERIFY(1==ReleaseDC(hWnd, hdc));
                }

                DrawEditBorder(hWnd, pWndData);
                return lRes;
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, TreeCtrlProc, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}





BOOL AeroSubClassTreeCtrl(HWND hwnd)
{
    return AeroSubClassControl(hwnd, TreeCtrlProc, 0);
}

