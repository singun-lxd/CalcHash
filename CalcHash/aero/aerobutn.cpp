static char *rcsid = "$Id: aerobutn.cpp,v 1.6 2007/05/20 09:05:25 cvs Exp $";
/*
*
* $RCSfile: aerobutn.cpp,v $
* $Source: /cvs/common/aerobutn.cpp,v $
* $Author: cvs $
* $Revision: 1.6 $
* $Date: 2007/05/20 09:05:25 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerobutn.cpp,v $
* Revision 1.6  2007/05/20 09:05:25  cvs
* Added comments and some more debug code
*
* Revision 1.5  2007/05/06 14:25:30  cvs
* x64 compatibility changes
*
* Revision 1.4  2007/05/04 11:39:14  cvs
* Drawing the border now in the edit border color
*
* Revision 1.3  2007/05/04 11:04:29  cvs
* Made interior of group box drawing independent from z-Order of controls
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






static LRESULT CALLBACK ButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
                    LONG_PTR dwButtonStyle = LOWORD(dwStyle);
                    LONG_PTR dwButtonType = dwButtonStyle&0xF;
                    RECT rcClient;
                    VERIFY(GetClientRect(hWnd, &rcClient));
                
                    if((dwButtonType&BS_GROUPBOX)==BS_GROUPBOX)
                    {
                        /// 
                        /// it must be a group box  
                        /// 
                        HTHEME hTheme = pWndData->m_pUxTheme->OpenThemeData(hWnd, L"Button");
                        if(hTheme)
                        {
                            HDC hdcPaint = NULL;
                            BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                            params.dwFlags        = BPPF_ERASE;

                            RECT rcExclusion = rcClient;
                            params.prcExclude = &rcExclusion;

                            /// 
                            /// We have to calculate the exclusion rect and therefore 
                            /// calculate the font height. We select the control's font
                            /// into the DC and fake a drawing operation:
                            /// 
                            HFONT hFontOld = (HFONT)SendMessage(hWnd, WM_GETFONT, 0L, NULL);
                            if(hFontOld) 
                                hFontOld = (HFONT) SelectObject(hdc, hFontOld);

                            RECT rcDraw = rcClient;
                            DWORD dwFlags = DT_SINGLELINE;
                        
                            /// 
                            /// we use uppercase A to determine the height of text, so we 
                            /// can draw the upper line of the groupbox:
                            /// 
                            DrawTextW(hdc, L"A", -1,  &rcDraw, dwFlags|DT_CALCRECT);
                        
                            if (hFontOld)
                            {                        
                                SelectObject(hdc, hFontOld);
                                hFontOld    = NULL;
                            }



                            VERIFY(InflateRect(&rcExclusion, -1, -1*RECTHEIGHT(rcDraw)));
                        
                            HPAINTBUFFER hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, 
                                &params, &hdcPaint);
                            if (hdcPaint)
                            {
                                /// 
                                /// now we again retrieve the font, but this time we select it into 
                                /// the buffered DC:
                                /// 
                                hFontOld = (HFONT)SendMessage(hWnd, WM_GETFONT, 0L, NULL);
                                if(hFontOld) 
                                    hFontOld = (HFONT) SelectObject(hdcPaint, hFontOld);

                            
                                VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));

                                VERIFY(S_OK==pWndData->m_pUxTheme->BufferedPaintSetAlpha(hBufferedPaint, &ps.rcPaint, 0x00));
                                int iState = GBS_NORMAL;
                                int iPartId = BP_GROUPBOX;

                                iState = GetStateFromBtnState(dwStyle, FALSE, FALSE, 0L, iPartId, FALSE);

                                DTTOPTS DttOpts = {sizeof(DTTOPTS)};
                                DttOpts.dwFlags = DTT_COMPOSITED;
                                DttOpts.crText   = RGB(255, 255, 255);
                                DttOpts.dwFlags |= DTT_GLOWSIZE;
                                DttOpts.iGlowSize = 12; // Default value

                                VERIFY(pWndData->m_pUxTheme->DetermineGlowSize(&DttOpts.iGlowSize));            

                                Pen*         myPen;
                                Graphics*    myGraphics;
                                COLORREF cr = RGB(0x00, 0x00, 0x00);
                                VERIFY(GetEditBorderColor(hWnd, pWndData->m_pUxTheme, &cr));
                            
                                /// 
                                /// add the alpha value:
                                /// 
                                cr |= 0xff000000;

                                myPen = new Pen(Color(cr), 1);
                                if(myPen)
                                {
                                    myGraphics = new Graphics(hdcPaint);
                                    if(myGraphics)
                                    {
                                        int iY = RECTHEIGHT(rcDraw)/2;
                                        myGraphics->DrawRectangle(myPen, rcClient.left, rcClient.top + iY, 
                                            RECTWIDTH(rcClient)-1, RECTHEIGHT(rcClient) - iY-1);
                                        delete myGraphics;
                                    }
                                    delete myPen;
                                }

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
                                            int iX = RECTWIDTH(rcDraw);
                                            rcDraw = rcClient;
                                            rcDraw.left += iX;
                                            DrawTextW(hdcPaint, szText, -1,  &rcDraw, dwFlags|DT_CALCRECT);
                                            VERIFY(PatBlt(hdcPaint, rcDraw.left, rcDraw.top , RECTWIDTH(rcDraw) + 3, RECTHEIGHT(rcDraw), BLACKNESS));
                                            rcDraw.left++;
                                            rcDraw.right++;
                                            VERIFY(S_OK==pWndData->m_pUxTheme->DrawThemeTextEx(hTheme, hdcPaint, iPartId, iState, szText, -1, 
                                                dwFlags, &rcDraw, &DttOpts));

                                        }

                                        VERIFY(!LocalFree(szText));
                                    }
                                }
                            
                                if (hFontOld)
                                {                        
                                    SelectObject(hdcPaint, hFontOld);
                                    hFontOld    = NULL;
                                }

                                VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
                        
                            }                        

                            VERIFY(S_OK==pWndData->m_pUxTheme->CloseThemeData(hTheme));
                        }
                    }

                    else if(dwButtonType==BS_CHECKBOX || dwButtonType==BS_AUTOCHECKBOX ||
                        dwButtonType==BS_3STATE || dwButtonType==BS_AUTO3STATE || dwButtonType==BS_RADIOBUTTON || dwButtonType==BS_AUTORADIOBUTTON)
                    {
                        HTHEME hTheme = pWndData->m_pUxTheme->OpenThemeData(hWnd, L"Button");
                        if(hTheme)
                        {
                            HDC hdcPaint = NULL;
                            BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                            params.dwFlags        = BPPF_ERASE;
                            HPAINTBUFFER hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                            if (hdcPaint)
                            {
                                VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));

                                VERIFY(S_OK==pWndData->m_pUxTheme->BufferedPaintSetAlpha(hBufferedPaint, &ps.rcPaint, 0x00));
                                int iState = CBS_UNCHECKEDNORMAL;

                                LRESULT dwCheckState = SendMessage(hWnd, BM_GETCHECK, 0, NULL);
                                POINT pt;
                                RECT rc;
                                GetWindowRect(hWnd, &rc);
                                GetCursorPos(&pt);
                                BOOL bHot = PtInRect(&rc, pt);
                                BOOL bFocus = GetFocus()==hWnd;
                                int iPartId = BP_CHECKBOX;

                            

                                if(dwButtonType==BS_RADIOBUTTON || dwButtonType==BS_AUTORADIOBUTTON)
                                    iPartId = BP_RADIOBUTTON;

                                iState = GetStateFromBtnState(dwStyle, bHot, bFocus, dwCheckState, iPartId, FALSE);

                            
                                HBITMAP hbmp = NULL;
                                VERIFY(S_OK==pWndData->m_pUxTheme->GetThemeBitmap(hTheme, iPartId, iState, 0, 
                                    GBF_DIRECT, &hbmp));
                                SIZE st;
                                VERIFY(GetBitmapDimensionEx(hbmp, &st));
                                BITMAP bm;
                                GetObject(hbmp, sizeof(BITMAP), &bm);
                            
                            
                                UINT uiHalfWidth = (RECTWIDTH(rcClient) - bm.bmWidth)/2;
                            
                                /// 
                                /// we have to use the whole client area, otherwise we get only partially
                                /// drawn areas:
                                /// 
                                RECT rcPaint = rcClient;
                            
                                if(dwButtonStyle & BS_LEFTTEXT)
                                {
                                    rcPaint.left += uiHalfWidth;
                                    rcPaint.right += uiHalfWidth;
                                }
                                else
                                {
                                    rcPaint.left -= uiHalfWidth;
                                    rcPaint.right -= uiHalfWidth;
                                }

                            
                                /// 
                                /// we assume that bm.bmWidth is both the horizontal and the vertical
                                /// dimension of the control bitmap and that it is square. bm.bmHeight
                                /// seems to be the height of a striped bitmap because it is an absurdly
                                /// high dimension value
                                /// 
                                if((dwButtonStyle&BS_VCENTER)==BS_VCENTER) /// BS_VCENTER is BS_TOP|BS_BOTTOM
                                {
                                    /// nothing to do, verticallly centered
                                }
                                else if(dwButtonStyle&BS_TOP)
                                {
                                    rcPaint.bottom = rcPaint.top + bm.bmWidth;
                                }
                                else if(dwButtonStyle&BS_BOTTOM)
                                {
                                    rcPaint.top =  rcPaint.bottom - bm.bmWidth;
                                }


                                VERIFY(S_OK==pWndData->m_pUxTheme->DrawThemeBackground(hTheme, hdcPaint, iPartId, iState, &rcPaint, NULL));
                                rcPaint = rcClient;
                            
                            
                                VERIFY(S_OK==pWndData->m_pUxTheme->GetThemeBackgroundContentRect(hTheme, hdcPaint, iPartId, iState, &rcPaint, &rc));

                                if(dwButtonStyle & BS_LEFTTEXT)
                                    rc.right -= bm.bmWidth+(bm.bmWidth>>1);
                                else
                                    rc.left += bm.bmWidth+(bm.bmWidth>>1);
                            
                            
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
                                            DWORD dwFlags = DT_SINGLELINE /*|DT_VCENTER*/;
                                            if(dwButtonStyle&BS_MULTILINE)
                                            {
                                                dwFlags|=DT_WORDBREAK;
                                                dwFlags&= ~(DT_SINGLELINE |DT_VCENTER);
                                            }

                                            if((dwButtonStyle&BS_CENTER)==BS_CENTER) /// BS_CENTER is BS_LEFT|BS_RIGHT
                                                dwFlags|=DT_CENTER;
                                            else if(dwButtonStyle&BS_LEFT)
                                                dwFlags|=DT_LEFT;
                                            else if(dwButtonStyle&BS_RIGHT)
                                                dwFlags|=DT_RIGHT;


                                            if((dwButtonStyle&BS_VCENTER)==BS_VCENTER) /// BS_VCENTER is BS_TOP|BS_BOTTOM
                                                dwFlags|=DT_VCENTER;
                                            else if(dwButtonStyle&BS_TOP)
                                                dwFlags|=DT_TOP;
                                            else if(dwButtonStyle&BS_BOTTOM)
                                                dwFlags|=DT_BOTTOM;
                                            else
                                                dwFlags|=DT_VCENTER;

                                            VERIFY(S_OK==pWndData->m_pUxTheme->DrawThemeTextEx(hTheme, hdcPaint, iPartId, 
                                                iState, szText, -1, dwFlags, &rc, &DttOpts));

                                            /// 
                                            /// if our control has the focus, we also have to draw the focus rectangle:
                                            /// 
                                            if(bFocus)
                                            {
                                                /// 
                                                /// now calculate the text size:
                                                /// 
                                                RECT rcDraw = rc;

                                                /// 
                                                /// we use GDI's good old DrawText, because it returns much more
                                                /// accurate data than DrawThemeTextEx, which takes the glow
                                                /// into account which we don't want:
                                                /// 
                                                VERIFY(DrawTextW(hdcPaint, szText, -1,  &rcDraw, dwFlags|DT_CALCRECT));
                                                if(dwFlags&DT_SINGLELINE)
                                                {
                                                    dwFlags &= ~DT_VCENTER;
                                                    RECT rcDrawTop;
                                                    DrawTextW(hdcPaint, szText, -1,  &rcDrawTop, dwFlags|DT_CALCRECT);
                                                    rcDraw.top = rcDraw.bottom - RECTHEIGHT(rcDrawTop);
                                                }

                                                if(dwFlags & DT_RIGHT)
                                                {
                                                    int iWidth = RECTWIDTH(rcDraw);
                                                    rcDraw.right = rc.right;
                                                    rcDraw.left = rcDraw.right - iWidth;
                                                }

                                                RECT rcFocus;
                                                VERIFY(IntersectRect(&rcFocus, &rc, &rcDraw));

                                                DrawFocusRect(&rcFocus, hdcPaint);
                                            }
                                        }

                                        VERIFY(!LocalFree(szText));
                                    }
                                }

                        


                                if (hFontOld)
                                {                        
                                    SelectObject(hdcPaint, hFontOld);
                                    hFontOld    = NULL;
                                }
                            
                                VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
                            }                        
                            VERIFY(S_OK==pWndData->m_pUxTheme->CloseThemeData(hTheme));
                        }


                    }
                    else if(BS_PUSHBUTTON==dwButtonType || BS_DEFPUSHBUTTON==dwButtonType)
                    {
                        /// 
                        /// it is a push button
                        /// 
                        HTHEME hTheme = pWndData->m_pUxTheme->OpenThemeData(hWnd, L"Button");
                        if(hTheme)
                        {
                            HDC hdcPaint = NULL;
                            BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                            params.dwFlags        = BPPF_ERASE;
                            HPAINTBUFFER hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                            if (hdcPaint)
                            {
                                VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));

                                VERIFY(S_OK==pWndData->m_pUxTheme->BufferedPaintSetAlpha(hBufferedPaint, &ps.rcPaint, 0x00));
                                int iState = CBS_UNCHECKEDNORMAL;

                                LRESULT dwCheckState = SendMessage(hWnd, BM_GETCHECK, 0, NULL);
                                POINT pt;
                                RECT rc;
                                GetWindowRect(hWnd, &rc);
                                GetCursorPos(&pt);
                                BOOL bHot = PtInRect(&rc, pt);
                                BOOL bFocus = GetFocus()==hWnd;
                                int iPartId = BP_PUSHBUTTON;

                                if(dwButtonStyle==BS_RADIOBUTTON || dwButtonStyle==BS_AUTORADIOBUTTON)
                                    iPartId = BP_RADIOBUTTON;


                                iState = GetStateFromBtnState(dwStyle, bHot, bFocus, dwCheckState, iPartId, GetCapture()==hWnd);

                                /// 
                                /// we have to use the whole client area, otherwise we get only partially
                                /// drawn areas:
                                /// 
                                RECT rcPaint = rcClient;
                                VERIFY(S_OK==pWndData->m_pUxTheme->DrawThemeBackground(hTheme, hdcPaint, iPartId, iState, &rcPaint, NULL));

                            
                                VERIFY(S_OK==pWndData->m_pUxTheme->GetThemeBackgroundContentRect(hTheme, hdcPaint, iPartId, iState, &rcPaint, &rc));
                            

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
                                            DWORD dwFlags = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
                                            VERIFY(S_OK==pWndData->m_pUxTheme->DrawThemeTextEx(hTheme, hdcPaint, 
                                                iPartId, iState, szText, -1, dwFlags, &rc, &DttOpts));

                                            /// 
                                            /// if our control has the focus, we also have to draw the focus rectangle:
                                            /// 
                                            if(bFocus)
                                            {
                                                RECT rcDraw = rcClient;
                                                VERIFY(InflateRect(&rcDraw, -3, -3));
                                                DrawFocusRect(&rcDraw, hdcPaint);
                                            }
                                        
                                        }

                                        VERIFY(!LocalFree(szText));
                                    }
                                }

                                if (hFontOld)
                                {                        
                                    SelectObject(hdcPaint, hFontOld);
                                    hFontOld    = NULL;
                                }
                            
                                VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
                            }                        
                            VERIFY(S_OK==pWndData->m_pUxTheme->CloseThemeData(hTheme));
                        }
                    }
                    else
                        PaintControl(hWnd, hdc, &ps.rcPaint, (pWndData->m_dwFlags & WD_DRAW_BORDER)!=0);
                
                
                }
                
                
                EndPaint(hWnd, &ps);
                return 1;
            }
            break;
        case WM_DESTROY:
        case WM_NCDESTROY:
            VERIFY(UnsubclassControl(hWnd, ButtonProc, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}



BOOL AeroSubClassButton(HWND hwnd)
{
    return AeroSubClassControl(hwnd, ButtonProc, WD_IN_PAINT_CONTROL);
}