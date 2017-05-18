static char *rcsid = "$Id: aeroslid.cpp,v 1.5 2007/05/20 12:57:04 cvs Exp $";
/*
*
* $RCSfile: aeroslid.cpp,v $
* $Source: /cvs/common/aeroslid.cpp,v $
* $Author: cvs $
* $Revision: 1.5 $
* $Date: 2007/05/20 12:57:04 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aeroslid.cpp,v $
* Revision 1.5  2007/05/20 12:57:04  cvs
* More defensive coding and debug code added
*
* Revision 1.4  2007/05/06 14:25:30  cvs
* x64 compatibility changes
*
* Revision 1.3  2007/04/09 18:17:47  cvs
* Added IpAddress control code
*
* Revision 1.2  2007/04/09 15:37:47  cvs
* Added selection code for slider control
*
* Revision 1.1  2007/04/09 11:58:45  cvs
* Added new file for slider controls
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


#ifndef dimof
#define dimof(a) (sizeof(a)/sizeof(a[0]))
#endif ///  dimof


const int g_iPart[2][3] ={ 
                          {TKP_THUMB, TKP_THUMBTOP, TKP_THUMBBOTTOM},
                          {TKP_THUMBVERT, TKP_THUMBLEFT, TKP_THUMBRIGHT}
                         };
                        
const int g_iThumbStates[2][3][4] = 
{
    {
        {TUS_NORMAL, TUS_FOCUSED, TUS_HOT, TUS_DISABLED},
        {TUTS_NORMAL, TUTS_FOCUSED, TUTS_HOT, TUTS_DISABLED},
        {TUBS_NORMAL, TUBS_FOCUSED, TUBS_HOT, TUBS_DISABLED}
    },
    {
        {TUVS_NORMAL, TUVS_FOCUSED, TUVS_HOT, TUVS_DISABLED},
        {TUVLS_NORMAL, TUVLS_FOCUSED, TUVLS_HOT, TUVLS_DISABLED},
        {TUVRS_NORMAL, TUVRS_FOCUSED, TUVRS_HOT, TUVRS_DISABLED}
    }
};

const int g_iTicPart[] = {TKP_TICS, TKP_TICSVERT};
const int g_iTicState[] = {TSS_NORMAL, TSVS_NORMAL};

const int g_iChannelPart[] = {TKP_TRACK, TKP_TRACKVERT};
const int g_iChannelState[] = {TRS_NORMAL, TRVS_NORMAL};

void FlipRectCoordinates(LPRECT lprc)
{
    RECT rcTmp = *lprc;
    lprc->left = rcTmp.top;
    lprc->right = rcTmp.bottom;
    lprc->top = rcTmp.left;
    lprc->bottom = rcTmp.right;
}



static LRESULT CALLBACK SliderProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    RECT rcClient;
                    GetClientRect(hWnd, &rcClient);
                
                    HTHEME hTheme = pWndData->m_pUxTheme->OpenThemeData(hWnd, L"TRACKBAR");
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
                            bool bFocus = GetFocus()==hWnd;
                            bool bDisabled = (dwStyle&WS_DISABLED)!=0L;
                            RECT rc;
                            VERIFY(GetWindowRect(hWnd, &rc));
                            int iSelMin = 0;
                            int iSelMax = 0;
                            
                            if(bFocus)
                                DrawFocusRect(&rcClient, hdcPaint);

                            SIZE siz; 
                        
                            int iVertical = (dwStyle&TBS_VERT)!=0?1:0;


                            rc = rcClient;
                            VERIFY(S_OK==pWndData->m_pUxTheme->GetThemePartSize(hTheme, hdcPaint, 
                                g_iChannelPart[iVertical], g_iChannelState[iVertical], &rc, TS_TRUE, &siz));
                            SIZE sizThumb;
                            SIZE sizMarker = {0,0};

                            /// 
                            /// when using GetThemeColor(hTheme, TKP_TICS, TSS_NORMAL, TMT_EDGEFILLCOLOR..)
                            /// we always get a very very light color which simply cannot be the right one,
                            /// so we simply use GetSysColor for the proper color:
                            /// 
                            COLORREF cr = GetSysColor(bDisabled?COLOR_3DLIGHT:COLOR_3DSHADOW);
                            Color col;
                            col.SetFromCOLORREF(cr);
                        
                            DWORD dwMoveMarkerMove = 1L;
                        
                        
                            if(!(dwStyle&TBS_NOTICKS))
                            {
                                VERIFY(S_OK==pWndData->m_pUxTheme->GetThemePartSize(hTheme, hdcPaint, 
                                    g_iTicPart[iVertical], g_iTicState[iVertical], NULL, TS_TRUE, &sizMarker));
                                    
                                /// 
                                /// the leftmost and rightmost marker are pixel higher, between focus and
                                /// markers are 4 pixels:
                                /// 
                                if(dwStyle&TBS_BOTH || dwStyle&TBS_TOP)
                                    dwMoveMarkerMove = (iVertical?sizMarker.cx:sizMarker.cy) + 2 + 4;  
                            }

                            ::SendMessage(hWnd, TBM_GETTHUMBRECT, 0, (LPARAM)&rc);
                            sizThumb.cx = RECTWIDTH(rc);
                            sizThumb.cy = RECTHEIGHT(rc);
                        
                            ::SendMessage(hWnd, TBM_GETCHANNELRECT, 0, (LPARAM)&rc);
                            RECT rcSelection = rc;

                            if(dwStyle&TBS_ENABLESELRANGE)
                            {
                        	    int nSelMin = (int)::SendMessage(hWnd, TBM_GETSELSTART, 0, 0L);
	                            int nSelMax = (int)::SendMessage(hWnd, TBM_GETSELEND, 0, 0L);
                        	    int nRangeMin = (int)::SendMessage(hWnd, TBM_GETRANGEMIN, 0, 0L);
	                            int nRangeMax = (int)::SendMessage(hWnd, TBM_GETRANGEMAX, 0, 0L);

                                rcSelection.top+=3;
                                rcSelection.bottom-=2;

                                if(nSelMin>=nRangeMin && nSelMin<nRangeMax && nSelMax<=nRangeMax && nSelMax>nRangeMin)
                                {
                                    ///
                                    /// we first try to calculate the selection range:
                                    ///
                                    rcSelection.left+=RECTWIDTH(rc)*(nSelMin-nRangeMin)/(nRangeMax-nRangeMin);
                                    rcSelection.right = rc.left + RECTWIDTH(rc)*(nSelMax-nRangeMin)/(nRangeMax-nRangeMin);

                                    ///
                                    /// if autoticks are specified, we can use the positions of autoticks
                                    /// which are much more accurate and in sync with the tick markers:
                                    ///
                                    if(dwStyle&TBS_AUTOTICKS)
                                    {
                                        DWORD* pdwTicks = (DWORD*) ::SendMessage(hWnd, TBM_GETPTICS, 0, 0l);
                                        if(pdwTicks)
                                        {
                                            UINT dwTicks = (UINT) ::SendMessage(hWnd, TBM_GETNUMTICS, 0, 0l);
                                            if (dwTicks>2)
                                            {
                                                dwTicks-=2;
                                                DWORD dwIdx = 0L;
                                                for(;dwIdx<dwTicks;dwIdx++)
                                                {
                                                    if(pdwTicks[dwIdx]==(DWORD)nSelMin)
                                                    {
                                                        int iPos = (int)::SendMessage(hWnd, TBM_GETTICPOS, dwIdx, 0L);
                                                        if(-1!=iPos)
                                                            rcSelection.left = rcClient.left+iPos;        
                                                    }
                                                    if(pdwTicks[dwIdx]==(DWORD)nSelMax)
                                                    {
                                                        int iPos = (int)::SendMessage(hWnd, TBM_GETTICPOS, dwIdx, 0L);
                                                        if(-1!=iPos)
                                                            rcSelection.right = rcClient.left+iPos;        
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    iSelMin = rcSelection.left;
                                    iSelMax = rcSelection.right;
                                    VERIFY(OffsetRect(&rcSelection, 1,0));
                                }
                            }

                            if(iVertical)
                            {
                                FlipRectCoordinates(&rc);
                                FlipRectCoordinates(&rcSelection);
                            }
                        
                            /// 
                            /// draw the channel:
                            /// 
                            VERIFY(S_OK==pWndData->m_pUxTheme->DrawThemeBackground(hTheme, hdcPaint, 
                                g_iChannelPart[iVertical], g_iChannelState[iVertical], &rc, NULL));
                        
                            if(dwStyle&TBS_ENABLESELRANGE)
                                FillRect(&rcSelection, hdcPaint, Color(0xff0000ff));

                            int iRange = 0;
                        
                            if (iVertical)
                            {
                                rc.left = rcClient.left + dwMoveMarkerMove + 1; // 1 for at least 1 pixel distance between marker/focus and thumb
                                rc.right = rc.left + sizThumb.cx;
                                iRange = RECTHEIGHT(rcClient)-3*sizThumb.cy;
                            }
                            else
                            {
                                rc.top = rcClient.top + dwMoveMarkerMove + 1; // 1 for at least 1 pixel distance between marker/focus and thumb
                                rc.bottom = rc.top + sizThumb.cy;
                                iRange = RECTWIDTH(rcClient)-3*sizThumb.cx;
                            }
                        
                            if(!(dwStyle&TBS_NOTICKS))
                            {
                                int iShift[] = {0,0};
                                size_t stId = 0;
                                size_t stShifts = dimof(iShift);
                                iShift[1] = (iVertical?(sizThumb.cx + sizMarker.cx):(sizThumb.cy + sizMarker.cy)) + 3;
                            
                                if(dwStyle&TBS_TOP)
                                    stShifts = 1;
                                else if(!(dwStyle&TBS_BOTH))
                                    stId = 1;

                                
                                for (;stId<stShifts;stId++)
                                {
                                    RECT rcMark;
                                    int iXMove = 0;
                                    int iYMove = 0;
                                
                                    if(iVertical)
                                    {
                                        rcMark.top = rc.top + sizThumb.cy/2;
                                        rcMark.bottom = rcMark.top + 2;
                                        rcMark.right = rc.left + iShift[stId];
                                        rcMark.left = rcMark.right - sizMarker.cx - 2;
                                        iYMove = RECTHEIGHT(rc) - sizThumb.cy;
                                    }
                                    else
                                    {
                                        rcMark.left = rc.left + sizThumb.cx/2;
                                        rcMark.right = rcMark.left + 2;
                                        rcMark.bottom = rc.top + iShift[stId];
                                        rcMark.top = rcMark.bottom - sizMarker.cy - 2;
                                        iXMove = RECTWIDTH(rc) - sizThumb.cx;
                                    }


                                    /// 
                                    /// paint the two ticks at the lower and higher end of the position range
                                    /// 
                                    FillRect(&rcMark, hdcPaint, col);
                                    VERIFY(OffsetRect(&rcMark, iXMove, iYMove));
                                    FillRect(&rcMark, hdcPaint, col);
                                    VERIFY(OffsetRect(&rcMark, -1*iXMove, -1*iYMove));

                                    /// 
                                    /// now paint the tick marks inbetween:
                                    /// 
                                    if(dwStyle&TBS_AUTOTICKS)
                                    {
                                        /// 
                                        /// make the intermediate markers one pixel smaller:
                                        /// 
                                        if(iVertical)
                                        {
                                            if(stId)
                                                rcMark.right--;
                                            else
                                                rcMark.left++;
                                        }
                                        else
                                        {
                                            if(stId)
                                                rcMark.bottom--;
                                            else
                                                rcMark.top++;
                                        }

                                        int iNumTicks = (int)::SendMessage(hWnd, TBM_GETNUMTICS, 0, 0l) - 2; 
                                        int iId = 0;

                                        for (;iId<iNumTicks;iId++)    
                                        {
                                            int iMove = (int)::SendMessage(hWnd, TBM_GETTICPOS, iId, 0L);
                                            if(iVertical)
                                            {
                                                rcMark.top = iMove;
                                                rcMark.bottom = rcMark.top+2;
                                            }
                                            else
                                            {
                                                rcMark.left = iMove;
                                                rcMark.right = rcMark.left+2;
                                            }
                                        
                                            FillRect(&rcMark, hdcPaint, col);
                                        }
                                    }

                                    /// 
                                    /// paint the selection marks (triangles)
                                    /// 
                                    if(dwStyle&TBS_ENABLESELRANGE)
                                    {
                                        Point pts[3];
                                        Color colBlack(0xff000000);
                                    
                                        if(iVertical)
                                        {
                                            pts[0].X = rcMark.right - 2;
                                            pts[0].Y = iSelMin;

                                            pts[1].X = pts[0].X - 2;
                                            pts[1].Y = iSelMin;
                                    
                                            Draw1PixelSolidLine(pts[0], pts[1], hdcPaint, colBlack);
                                    
                                            pts[2].Y = pts[1].Y - 2;
                                    
                                            if(stId)
                                            {
                                                pts[2].X = pts[0].X;
                                                Draw1PixelSolidLine(pts[0], pts[2], hdcPaint, colBlack);
                                                Draw1PixelSolidLine(pts[2], pts[1], hdcPaint, colBlack);
                                            }
                                            else
                                            {
                                                pts[2].X = pts[1].X;
                                                Draw1PixelSolidLine(pts[1], pts[2], hdcPaint, colBlack);
                                                Draw1PixelSolidLine(pts[2], pts[0], hdcPaint, colBlack);
                                            }
                                        
                                            pts[0].Y = iSelMax;
                                            pts[1].Y = iSelMax;
                                    
                                            Draw1PixelSolidLine(pts[0], pts[1], hdcPaint, colBlack);
                                    
                                            pts[2].Y = pts[1].Y + 2;
                                    
                                            if(stId)
                                            {
                                                pts[2].X = pts[0].X;
                                                Draw1PixelSolidLine(pts[0], pts[2], hdcPaint, colBlack);
                                                Draw1PixelSolidLine(pts[2], pts[1], hdcPaint, colBlack);
                                            }
                                            else
                                            {
                                                pts[2].X = pts[1].X;
                                                Draw1PixelSolidLine(pts[1], pts[2], hdcPaint, colBlack);
                                                Draw1PixelSolidLine(pts[2], pts[0], hdcPaint, colBlack);
                                            }
                                        }
                                        else
                                        {
                                            pts[0].X = iSelMin;
                                            pts[0].Y = rcMark.bottom - 2;

                                            pts[1].X = iSelMin;
                                            pts[1].Y = pts[0].Y - 2;
                                    
                                            Draw1PixelSolidLine(pts[0], pts[1], hdcPaint, colBlack);
                                    
                                    
                                            pts[2].X = pts[1].X - 2;
                                    
                                            if(stId)
                                            {
                                                pts[2].Y = pts[0].Y;
                                                Draw1PixelSolidLine(pts[0], pts[2], hdcPaint, colBlack);
                                                Draw1PixelSolidLine(pts[2], pts[1], hdcPaint, colBlack);
                                            }
                                            else
                                            {
                                                pts[2].Y = pts[1].Y;
                                                Draw1PixelSolidLine(pts[1], pts[2], hdcPaint, colBlack);
                                                Draw1PixelSolidLine(pts[2], pts[0], hdcPaint, colBlack);
                                            }

                                            pts[0].X = iSelMax;
                                            pts[1].X = iSelMax;
                                    
                                            Draw1PixelSolidLine(pts[0], pts[1], hdcPaint, colBlack);
                                    
                                            pts[2].X = pts[1].X + 2;
                                    
                                            if(stId)
                                            {
                                                pts[2].Y = pts[0].Y;
                                                Draw1PixelSolidLine(pts[0], pts[2], hdcPaint, colBlack);
                                                Draw1PixelSolidLine(pts[2], pts[1], hdcPaint, colBlack);
                                            }
                                            else
                                            {
                                                pts[2].Y = pts[1].Y;
                                                Draw1PixelSolidLine(pts[1], pts[2], hdcPaint, colBlack);
                                                Draw1PixelSolidLine(pts[2], pts[0], hdcPaint, colBlack);
                                            }
                                        }
                                    }
                                }
                            }

                            /// 
                            /// slider controls only appear as hot if 
                            /// mouse is moved directly above the thumb:
                            /// 
                            POINT pt;
                            VERIFY(GetCursorPos(&pt));
                            VERIFY(ScreenToClient(hWnd, &pt));
                            ::SendMessage(hWnd, TBM_GETTHUMBRECT, 0, (LPARAM)&rc);
                            bool bHot = PtInRect(&rc, pt)!=FALSE;

                            int iStyleIndex = 0;
                            if(dwStyle&TBS_BOTH)
                                iStyleIndex = 0;
                            else if(dwStyle&TBS_TOP)
                                iStyleIndex = 1;
                            else
                                iStyleIndex = 2;

                            int iStateIndex = 0;
                            if(bFocus)
                                iStateIndex = 1;
                            if(bHot)
                                iStateIndex = 2;
                            if(bDisabled)
                                iStateIndex = 3;

                            VERIFY(S_OK==pWndData->m_pUxTheme->DrawThemeBackground(hTheme, hdcPaint, 
                                g_iPart[iVertical][iStyleIndex], g_iThumbStates[iVertical][iStyleIndex][iStateIndex], &rc, NULL));
                            VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
                        }                        
                        VERIFY(S_OK==pWndData->m_pUxTheme->CloseThemeData(hTheme));
                    }                
                }
                
                EndPaint(hWnd, &ps);
                return 1;
            }
            break;
        case WM_LBUTTONDOWN:
            pWndData->m_iCurrentSliderPos = (int)SendMessage(hWnd, TBM_GETPOS, 0, NULL);
        case WM_LBUTTONUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            LRESULT lRet = CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
            InvalidateRgn(hWnd, NULL, TRUE);
            VERIFY(UpdateWindow(hWnd));
            return lRet;
        }
            break;
        case WM_MOUSEMOVE:
            if(GetCapture()==hWnd)
            {
                LRESULT lRet = CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
                /// 
                /// compare current position with last stored position, update only
                /// if both are different thus reducing flicker significantly:
                /// 
                int iPos = (int)SendMessage(hWnd, TBM_GETPOS, 0, NULL);
                if(iPos!=pWndData->m_iCurrentSliderPos)
                {
                    pWndData->m_iCurrentSliderPos = iPos;
                    InvalidateRgn(hWnd, NULL, TRUE);
                    VERIFY(UpdateWindow(hWnd));
                }
                return lRet;
            }

            break;
        case WM_NCPAINT:
            {
                LONG_PTR dwExStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
                if(WS_EX_CLIENTEDGE&dwExStyle)
                {
                    HWND hParent = GetParent(hWnd);
                    if(hParent)
                    {
                        HDC hdc = GetDC(hParent);
                        if(hdc)
                        {
                            LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
                            COLORREF cr = GetSysColor(dwStyle&WS_DISABLED?COLOR_3DLIGHT:COLOR_3DSHADOW);
                            Color col;
                            col.SetFromCOLORREF(cr);
                            RECT rcWnd;
                            VERIFY(GetWindowRect(hWnd, &rcWnd));
                            ScreenToClient(hParent, &rcWnd);
                            DrawRect(&rcWnd, hdc, DashStyleSolid, col, 1.0);
                            VERIFY(1==ReleaseDC(hParent, hdc));
                        }
                    }
                }    
            }
            
            return 1;
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, SliderProc, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}


BOOL AeroSubClassSlider(HWND hwnd)
{
    return AeroSubClassControl(hwnd, SliderProc, 0L);
}
