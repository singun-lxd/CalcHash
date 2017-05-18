static char *rcsid = "$Id: aerostat.cpp,v 1.4 2007/05/20 13:06:06 cvs Exp $";
/*
*
* $RCSfile: aerostat.cpp,v $
* $Source: /cvs/common/aerostat.cpp,v $
* $Author: cvs $
* $Revision: 1.4 $
* $Date: 2007/05/20 13:06:06 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerostat.cpp,v $
* Revision 1.4  2007/05/20 13:06:06  cvs
* More defensive coding and debug code added
*
* Revision 1.3  2007/05/06 14:25:30  cvs
* x64 compatibility changes
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


static LRESULT CALLBACK StaticProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    HDC hdcPaint = NULL;
                    RECT rcClient;
                    VERIFY(GetClientRect(hWnd, &rcClient));
                
                    LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
                    LONG_PTR dwStyleEx = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
                
                    HTHEME hTheme = pWndData->m_pUxTheme->OpenThemeData(NULL, L"ControlPanelStyle"); 
                
                
                    if(hTheme)
                    {
                        
                        HPAINTBUFFER hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, NULL, &hdcPaint);
                        if (hdcPaint)
                        {
                            VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));
                            VERIFY(S_OK==pWndData->m_pUxTheme->BufferedPaintSetAlpha(hBufferedPaint, &ps.rcPaint, 0x00));
                            LONG_PTR dwStaticStyle = dwStyle&0x1F;


                            if(dwStaticStyle==SS_ICON || dwStaticStyle==SS_BITMAP)
                            {
                                bool bIcon = dwStaticStyle==SS_ICON;
                                HANDLE hBmpIco = (HANDLE)SendMessage(hWnd, STM_GETIMAGE, bIcon ? IMAGE_ICON:IMAGE_BITMAP, NULL);
                                if(hBmpIco)
                                {
                                    Bitmap *pBmp = bIcon ? new Bitmap((HICON)hBmpIco) : new Bitmap((HBITMAP)hBmpIco, NULL);
                                    if(pBmp)
                                    {
                                        Graphics*    myGraphics = new Graphics(hdcPaint);
                                        if(myGraphics)
                                        {
                                            CachedBitmap *pcbmp = new CachedBitmap(pBmp, myGraphics);
                                            if(pcbmp)
                                            {
                                                VERIFY(Ok==myGraphics->DrawCachedBitmap(pcbmp, 0,0));
                                                delete pcbmp;
                                            }
                                            delete myGraphics;
                                        }

                                        delete pBmp;
                                    }
                                }
                            }
                            else if(SS_BLACKRECT==dwStaticStyle || SS_GRAYRECT==dwStaticStyle || SS_WHITERECT==dwStaticStyle)
                            {
                                ARGB argb = 0L;
                                switch (dwStaticStyle)
                                {
                                    case SS_BLACKRECT:
                                        argb = 0xFF000000;
                                        break;
                                    case SS_GRAYRECT:
                                        argb = 0xFF808080;
                                        break;
                                    case SS_WHITERECT:
                                        argb = 0xFFFFFFFF;
                                        break;
                                    default:
                                        ASSERT(0);
                                        break;
                                }
                                Color clr(argb);

                                FillRect(&rcClient, hdcPaint, clr);
                            }
                            else if(SS_BLACKFRAME==dwStaticStyle || SS_GRAYFRAME==dwStaticStyle || SS_WHITEFRAME==dwStaticStyle)
                            {
                                ARGB argb = 0L;
                                switch (dwStaticStyle)
                                {
                                    case SS_BLACKFRAME:
                                        argb = 0xFF000000;
                                        break;
                                    case SS_GRAYFRAME:
                                        argb = 0xFF808080;
                                        break;
                                    case SS_WHITEFRAME:
                                        argb = 0xFFFFFFFF;
                                        break;
                                    default:
                                        ASSERT(0);
                                        break;
                                }
                                Color clr(argb);

                                DrawRect(&rcClient, hdcPaint, DashStyleSolid, clr, 1.0);
                            }
                            else
                            {
                                DTTOPTS DttOpts = {sizeof(DTTOPTS)};
                                DttOpts.dwFlags = DTT_COMPOSITED;
                                DttOpts.crText   = RGB(255, 255, 255);
                                DttOpts.dwFlags |= DTT_GLOWSIZE;
                                DttOpts.iGlowSize = 12; // Default value
            
                                VERIFY(pWndData->m_pUxTheme->DetermineGlowSize(&DttOpts.iGlowSize));            

                                HFONT hFontOld = (HFONT)SendMessage(hWnd, WM_GETFONT, 0L, NULL);
                                if(hFontOld) 
                                    hFontOld = (HFONT) SelectObject(hdcPaint, hFontOld);
                                int iLen = GetWindowTextLength(hWnd);

                                if(iLen)
                                {
                                    iLen+=5; // 1 for terminating zero, 4 for DT_MODIFYSTRING
                                    LPWSTR szText = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*iLen);
                                    if(szText)
                                    {
                                        iLen = GetWindowTextW(hWnd, szText, iLen);
                                        if(iLen)
                                        {
                                            DWORD dwFlags = DT_WORDBREAK;
                                        
                                            switch (dwStaticStyle)
                                            {
                                                case SS_CENTER:
                                                    dwFlags |= DT_CENTER;
                                                    break;
                                                case SS_RIGHT:
                                                    dwFlags |= DT_RIGHT;
                                                    break;
                                                case SS_LEFTNOWORDWRAP:
                                                    dwFlags &= ~DT_WORDBREAK;
                                                    break;

                                            }

                                            if(dwStyle & SS_CENTERIMAGE)
                                            {
                                                dwFlags |= DT_VCENTER;
                                                dwFlags &= ~DT_WORDBREAK;
                                            }


                                            if(dwStyle & SS_ENDELLIPSIS)
                                                dwFlags |= DT_END_ELLIPSIS|DT_MODIFYSTRING;
                                            else if(dwStyle & SS_PATHELLIPSIS)
                                                dwFlags |= DT_PATH_ELLIPSIS|DT_MODIFYSTRING;
                                            else if(dwStyle & SS_WORDELLIPSIS)
                                                dwFlags |= DT_WORD_ELLIPSIS|DT_MODIFYSTRING;


                                            if (dwStyleEx&WS_EX_RIGHT)
                                                dwFlags |= DT_RIGHT;

                                            if(dwStyle & SS_NOPREFIX)
                                                dwFlags |= DT_NOPREFIX;
                                        
                                            VERIFY(S_OK==pWndData->m_pUxTheme->DrawThemeTextEx(hTheme, hdcPaint, 0, 0, 
                                                szText, -1, dwFlags, &rcClient, &DttOpts));

                                            if(dwStyle&SS_SUNKEN || dwStyle&WS_BORDER)
                                                DrawRect(&rcClient, hdcPaint, DashStyleSolid, Color(0xFF, 0,0,0), 1.0);
                                        }

                                        VERIFY(!LocalFree(szText));
                                    }
                                }

                                if (hFontOld)
                                {                        
                                    SelectObject(hdcPaint, hFontOld);
                                    hFontOld    = NULL;
                                }
                            }

                            VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
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
            VERIFY(UnsubclassControl(hWnd, StaticProc, pWndData));
            break;
    }
    
    LRESULT lRet = CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);

    return lRet;
}





BOOL AeroSubClassStatic(HWND hwnd)
{
    return AeroSubClassControl(hwnd, StaticProc, WD_IN_PAINT_CONTROL);
}
