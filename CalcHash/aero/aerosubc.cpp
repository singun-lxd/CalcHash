static char *rcsid = "$Id: aerosubc.cpp,v 1.27 2007/05/20 13:37:26 cvs Exp $";
/*
*
* $RCSfile: aerosubc.cpp,v $
* $Source: /cvs/common/aerosubc.cpp,v $
* $Author: cvs $
* $Revision: 1.27 $
* $Date: 2007/05/20 13:37:26 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerosubc.cpp,v $
* Revision 1.27  2007/05/20 13:37:26  cvs
* Added some more debug code
*
* Revision 1.26  2007/05/20 13:33:40  cvs
* Added comments and some more debug code and improved robustness
*
* Revision 1.25  2007/05/19 14:11:26  cvs
* Added automatic linkage against gdiplus
*
* Revision 1.24  2007/05/18 22:22:15  cvs
* more defensive programming
*
* Revision 1.23  2007/05/18 21:20:28  cvs
* Added DrawFilledWndRectOnParent
*
* Revision 1.22  2007/05/18 16:05:52  cvs
* Did some code clean-up
*
* Revision 1.21  2007/05/18 11:24:18  cvs
* Added progress control subclassing
*
* Revision 1.20  2007/05/11 20:52:12  cvs
* Added AeroSubclassCtrl for easier per-control subclassing
*
* Revision 1.19  2007/05/06 14:25:30  cvs
* x64 compatibility changes
*
* Revision 1.18  2007/05/04 15:39:05  cvs
* Added VERIFY
*
* Revision 1.17  2007/05/04 11:39:14  cvs
* Drawing the border now in the edit border color
*
* Revision 1.16  2007/04/22 18:55:04  cvs
* Added DrawEditBorder
*
* Revision 1.15  2007/04/22 12:29:22  cvs
* Preventing double subclassing and thus memory leaks and infinite loops
*
* Revision 1.14  2007/04/15 17:49:55  cvs
* Added disfunctional comboboxex routines
*
* Revision 1.13  2007/04/15 15:04:55  cvs
* Added selective subclassing
*
* Revision 1.12  2007/04/15 13:36:24  cvs
* fixed initialization probs if desktop composition is initially turned off
*
* Revision 1.11  2007/04/15 13:25:33  cvs
* removed monthcal subclassing code
*
* Revision 1.10  2007/04/10 17:35:56  cvs
* Added tree control
*
* Revision 1.9  2007/04/10 09:17:29  cvs
* Added date time pick control
*
* Revision 1.8  2007/04/09 18:17:47  cvs
* Added IpAddress control code
*
* Revision 1.7  2007/04/09 16:15:40  cvs
* Added spin control code
*
* Revision 1.6  2007/04/09 15:37:47  cvs
* Added selection code for slider control
*
* Revision 1.5  2007/04/09 11:59:08  cvs
* Added new file for slider controls
*
* Revision 1.4  2007/04/09 11:52:36  cvs
* Tightened code somewhat and based it on tables
*
* Revision 1.3  2007/04/08 19:52:10  cvs
* Added vertical slider code
*
* Revision 1.2  2007/04/08 13:37:51  cvs
* Added standard header
* 
*/

#include "aerosubc.h" 
#include "aeroglss.h" 
#include "safassrt.h" 
#include "aaeroint.h" 
#include <tchar.h>
#include <stdio.h>

#pragma comment(lib, "gdiplus.lib")


#define REDRAWSTRING _T("AeroAutoSubclassRedraw")



void __cdecl TRACE(LPCTSTR lpszFormat, ...)
{
#ifdef _DEBUG
	int nBuf;
	TCHAR szBuffer[512];
	va_list args;
	va_start(args, lpszFormat);
	nBuf = _vsntprintf_s(szBuffer, dimof(szBuffer), lpszFormat, args);
	ASSERT(nBuf >= 0); // was there an error? was the expanded string too long?
    OutputDebugString(szBuffer);
	va_end(args);
#else
    lpszFormat;
#endif /// _DEBUG
}




void PaintControl(HWND hWnd, HDC hdc, RECT* prc, bool bDrawBorder)
{
    HDC hdcPaint = NULL;
    PAERO_SUBCLASS_WND_DATA pwd = (PAERO_SUBCLASS_WND_DATA)GetProp(hWnd, WINDOW_DATA_STRING);
    ASSERT(pwd);
    
    if(bDrawBorder)
        VERIFY(InflateRect(prc, 1, 1));
    HPAINTBUFFER hBufferedPaint = pwd->m_pUxTheme->BeginBufferedPaint(hdc, prc, BPBF_TOPDOWNDIB, NULL, &hdcPaint);
    if (hdcPaint)
    {
        RECT rc;
        VERIFY(GetWindowRect(hWnd, &rc));
        
        PatBlt(hdcPaint, 0, 0, RECTWIDTH(rc), RECTHEIGHT(rc), BLACKNESS);
        VERIFY(S_OK==pwd->m_pUxTheme->BufferedPaintSetAlpha(hBufferedPaint, &rc, 0x00));
        
        /// 
        /// first blit white so list ctrls don't look ugly:
        /// 
        VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rc), RECTHEIGHT(rc), WHITENESS));
        
        if(bDrawBorder)
            VERIFY(InflateRect(prc, -1, -1));
        // Tell the control to paint itself in our memory buffer
        SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM) hdcPaint, PRF_CLIENT|PRF_ERASEBKGND |PRF_NONCLIENT|PRF_CHECKVISIBLE);
        
        if(bDrawBorder)
        {
            VERIFY(InflateRect(prc, 1, 1));
            VERIFY(FrameRect(hdcPaint, prc, (HBRUSH)GetStockObject(BLACK_BRUSH)));
        }
        

        // Make every pixel opaque
        VERIFY(S_OK==pwd->m_pUxTheme->BufferedPaintMakeOpaque_(hBufferedPaint, prc));
        VERIFY(S_OK==pwd->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));
    }
}


BOOL PaintBufferedRect(HDC hdc, LPRECT lprc, CUxThemeAeroImpl *pUx)
{
    HDC hdcPaint = NULL;
    HPAINTBUFFER hBufferedPaint = pUx->BeginBufferedPaint(hdc, lprc, BPBF_TOPDOWNDIB, NULL, &hdcPaint);
    if (hdcPaint)
    {
        VERIFY(FrameRect(hdcPaint, lprc, (HBRUSH)GetStockObject(BLACK_BRUSH)));
        // Make every pixel opaque
        VERIFY(S_OK==pUx->BufferedPaintMakeOpaque_(hBufferedPaint, lprc));
        VERIFY(S_OK==pUx->EndBufferedPaint(hBufferedPaint, TRUE));
    }
    
    return TRUE;
}












#define LONGEST_CLASS_NAME dimof(_T("msctls_trackbar32"))


BOOL CALLBACK SubclassChildWindows(
  HWND hwnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
    PERROR_PARENT_AERO_SUBCLASS_WND_DATA pErrParentAeroData = (PERROR_PARENT_AERO_SUBCLASS_WND_DATA)lParam;

    if(ERROR_SUCCESS != (*pErrParentAeroData->m_pdwError) || pErrParentAeroData->m_hWndParent!=GetParent(hwnd))
        return TRUE; // previous calls resulted in an error, don't let a failing EnumChildWindows mask our error

    ASSERT(pErrParentAeroData);
    TCHAR szWndClassName[LONGEST_CLASS_NAME];
    if(GetClassName(hwnd, szWndClassName, dimof(szWndClassName)))
    {
        if  (!_tcscmp(szWndClassName, _T("ComboBox")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_COMBOBOX)
                VERIFY(AeroSubClassComboBox(hwnd));
            goto CLEANUP;
        }

        if  (!_tcscmp(szWndClassName, _T("ComboBoxEx32")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_COMBOBOX)
                VERIFY(AeroSubClassComboBoxEx(hwnd));
            goto CLEANUP;
        }

        if  (!_tcscmp(szWndClassName, _T("SysAnimate32")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_ANIMATION)
                VERIFY(AeroSubClassAnimation(hwnd));
            goto CLEANUP;
        }

        if  (!_tcscmp(szWndClassName, _T("Edit")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_EDIT)
                VERIFY(AeroSubClassEdit(hwnd));
            goto CLEANUP;
        }


        if(!_tcscmp(szWndClassName, _T("ListBox")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_LISTBOX)
                VERIFY(AeroSubClassListBox(hwnd));
            goto CLEANUP;
        }
            

        if(!_tcscmp(szWndClassName, _T("Static")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_STATIC)
                VERIFY(AeroSubClassStatic(hwnd));
            goto CLEANUP;
        }

        if(!_tcscmp(szWndClassName, _T("SysListView32")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_LISTCTRL)
                VERIFY(AeroSubClassListCtrl(hwnd));
            goto CLEANUP;
        }

        if(!_tcscmp(szWndClassName, _T("SysHeader32")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_HDRCTRL)
                VERIFY(AeroSubClassHeaderCtrl(hwnd));
            goto CLEANUP;
        }

        if(!_tcscmp(szWndClassName, _T("Button")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_BUTTON)
                VERIFY(AeroSubClassButton(hwnd));
            goto CLEANUP;
        }

        if(!_tcscmp(szWndClassName, _T("msctls_trackbar32")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_SLIDER)
                VERIFY(AeroSubClassSlider(hwnd));
            goto CLEANUP;
        }

        if(!_tcscmp(szWndClassName, _T("msctls_updown32")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_SPINCTRL)
                VERIFY(AeroSubClassSpinCtrl(hwnd));
            goto CLEANUP;
        }

        if(!_tcscmp(szWndClassName, _T("SysIPAddress32")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_IPADRCTRL)
                VERIFY(AeroSubClassIPAddressCtrl(hwnd));
            goto CLEANUP;
        }

        if(!_tcscmp(szWndClassName, _T("SysDateTimePick32")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_DATETIMEP)
                VERIFY(AeroSubClassDateTimePick(hwnd));
            goto CLEANUP;
        }


        if(!_tcscmp(szWndClassName, _T("SysTreeView32")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_TREECTRL)
                VERIFY(AeroSubClassTreeCtrl(hwnd));
            goto CLEANUP;
        }


        if(!_tcscmp(szWndClassName, _T("msctls_progress32")))
        {
            if(pErrParentAeroData->m_dwFlags&ASC_SUBCLASS_PRGSCTRL)
                VERIFY(AeroSubClassProgressCtrl(hwnd));
            goto CLEANUP;
        }
    }

CLEANUP:

    return TRUE;
}


BOOL AeroSubClassChild(HWND hwnd, WNDPROC pWndProc, DWORD dwAddFlags, PAERO_SUBCLASS_WND_DATA pWndParentAeroData)
{
    if(!hwnd || !pWndProc || !pWndParentAeroData)
    {
        SetLastError(ERROR_INVALID_PARAMETER);        
        return FALSE;
    }


    PAERO_SUBCLASS_WND_DATA pControlSubclassWndData = (PAERO_SUBCLASS_WND_DATA)GetProp(hwnd, WINDOW_DATA_STRING);
    if(pControlSubclassWndData)
    {
        SetLastError(ERROR_ALREADY_INITIALIZED);
        return FALSE;
    }



    pControlSubclassWndData = (PAERO_SUBCLASS_WND_DATA)LocalAlloc(LPTR, sizeof(AERO_SUBCLASS_WND_DATA));
    if(!pControlSubclassWndData)
        return FALSE;   // use the LocalAlloc last error

    memcpy(pControlSubclassWndData, pWndParentAeroData, sizeof(AERO_SUBCLASS_WND_DATA));
    pControlSubclassWndData->m_dwFlags |= dwAddFlags;

    pControlSubclassWndData->m_dwSelFirst = pControlSubclassWndData->m_dwSelLast = (DWORD) -1;


    CLocalFreeData ctrlData(pControlSubclassWndData);

    if(!SetProp(hwnd, WINDOW_DATA_STRING, pControlSubclassWndData))
        return FALSE;   // use the SetProp last error

    pControlSubclassWndData->m_oldWndProc = (WNDPROC) SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) pWndProc);
    
    if(!pControlSubclassWndData->m_oldWndProc)
    {
        DWORD dwLastError = GetLastError();
        VERIFY(pControlSubclassWndData==RemoveProp(hwnd, WINDOW_DATA_STRING));
        SetLastError(dwLastError);
        return FALSE;
    }

    ctrlData.Clear();
    
    return TRUE;
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAERO_SUBCLASS_WND_DATA pWndData = (PAERO_SUBCLASS_WND_DATA)GetProp(hWnd, WINDOW_DATA_STRING);
    ASSERT(pWndData);
    ASSERT(pWndData->m_pDwmApiImpl);
    WNDPROC pOldProc = pWndData->m_oldWndProc;
    ASSERT(pOldProc);
    BOOL bCompositionEnabled = pWndData->m_pDwmApiImpl->IsDwmCompositionEnabled();

    /// 
    /// if aero glass is turned off and if we are not in destruction code, 
    /// just call the original wnd proc we had prior to subclassing:
    /// 
    if(WM_COMMAND!=uMsg && WM_DWMCOMPOSITIONCHANGED!=uMsg && WM_DESTROY!=uMsg && WM_NCDESTROY!=uMsg && !bCompositionEnabled)
        return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);

    if (uMsg == pWndData->m_uiRedrawMsg)
    {
        HWND hControl = (HWND)lParam;
        ASSERT(hControl);
        ASSERT(::IsWindow(hControl));

        
        PAERO_SUBCLASS_WND_DATA pCtrlData = (PAERO_SUBCLASS_WND_DATA)GetProp(hControl, WINDOW_DATA_STRING);
        if(pCtrlData && pCtrlData->m_dwFlags & WD_IN_PAINT_CONTROL)
        {
            HDC hdc = GetDC(hControl);
            if(hdc)
            {
                RECT rc;
                VERIFY(GetWindowRect(hControl, &rc));

                VERIFY(MapWindowPoints(NULL, hControl, (LPPOINT) &rc, 2));

                PaintControl(hControl, hdc, &rc, (pCtrlData->m_dwFlags & WD_DRAW_BORDER)!=0);
                VERIFY(1==ReleaseDC(hControl, hdc));
            }
            pCtrlData->m_dwFlags &= ~WD_IN_PAINT_CONTROL;
            return 0;
        }
    }



    switch(uMsg)
    {
        case WM_CTLCOLORSTATIC:
            {
                HWND hControl = (HWND)lParam;
                ASSERT(hControl);
                ASSERT(IsWindow(hControl));
                PAERO_SUBCLASS_WND_DATA pCtrlData = (PAERO_SUBCLASS_WND_DATA)GetProp(hControl, WINDOW_DATA_STRING);
                if(pCtrlData)
                {
                    if(pCtrlData->m_dwFlags&WD_RETURN_BLACK_BRUSH)
                        return (LRESULT)GetStockObject(BLACK_BRUSH);

                    else if(pCtrlData->m_dwFlags&WD_RETURN_WHITE_BRUSH)
                        return (LRESULT)GetStockObject(WHITE_BRUSH);
                    else
                    {
                        pCtrlData->m_dwFlags|=WD_IN_PAINT_CONTROL;
                        PostMessage((HWND)lParam, pWndData->m_uiRedrawMsg, 0, NULL);
                    }
                }
            }

            break;
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
                if(!IsIconic(hWnd) && !(pWndData->m_dwFlags&WD_NO_FRAME_EXTEND))
                {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hWnd, &ps);

                    /// 
                    /// we have to paint the *entire* client area in black, not only the
                    /// paint area inside ps, because otherwise we get ugly areas of white
                    /// if we partially move the window out of the desktop and back in again:
                    /// 
                    /// 
                    MARGINS marGlassInset = {-1, -1, -1, -1}; // -1 means the whole window
                    if(hdc && pWndData->m_pDwmApiImpl->IsDwmCompositionEnabled() 
                        && SUCCEEDED(pWndData->m_pDwmApiImpl->DwmExtendFrameIntoClientArea(hWnd, &marGlassInset)))
                    {
                        RECT rcClient;
                        VERIFY(GetClientRect(hWnd, &rcClient));
                        VERIFY(PatBlt(hdc, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS));
                    }

                    EndPaint(hWnd, &ps);
                    return 1;
                }
            }
            break;
        case WM_COMMAND:
            {
                if(bCompositionEnabled)
                {
                    if(LBN_SELCHANGE==HIWORD(wParam))
                    {
                        InvalidateRgn((HWND)lParam, NULL, TRUE);
                        VERIFY(UpdateWindow((HWND)lParam));
                    }
                }

                if(ACN_STOP==HIWORD(wParam))
                {
                    /// 
                    /// if it is an animation control that has just stopped playing,
                    /// reset the WD_PLAY flag so it repaints itself properly again if
                    /// e.g. Aero is turned off an on again:
                    /// 
                    PAERO_SUBCLASS_WND_DATA pCtrlData = (PAERO_SUBCLASS_WND_DATA)GetProp((HWND)lParam, WINDOW_DATA_STRING);                    
                    if(pCtrlData)
                    {
                        pCtrlData->m_dwFlags&=~WD_PLAY;
                    }
                }
                if(ACN_START==HIWORD(wParam))
                {
                    PAERO_SUBCLASS_WND_DATA pCtrlData = (PAERO_SUBCLASS_WND_DATA)GetProp((HWND)lParam, WINDOW_DATA_STRING);                    
                    if(pCtrlData)
                    {
                        pCtrlData->m_dwFlags|=WD_PLAY;
                    }
                }

            }

            break;
        case WM_NOTIFY:
            {
                
                LPNMHDR lpnmh = (LPNMHDR)lParam;

                if(lpnmh)
                {
                    ASSERT(lpnmh->hwndFrom);
                    ASSERT(IsWindow(lpnmh->hwndFrom));

                    switch(lpnmh->code)
                    {
                        case LVN_ITEMCHANGED:
                        {
                            /// 
                            /// if we select and deselect list view items,
                            /// its header control sometimes vanishes so
                            /// we have to force a redraw:
                            /// 
                            HWND hHeader = ListView_GetHeader(lpnmh->hwndFrom);
                            if(hHeader)
                            {
                                InvalidateRgn(hHeader, NULL, TRUE);
                                VERIFY(UpdateWindow(hHeader));
                            }
                         }
                            break;
                         default:
                            break;
                    }
                }
            }

            break;
        case WM_DESTROY:
            ASSERT(pWndData->m_pUxTheme);
            ASSERT(pWndData->m_pDwmApiImpl);
            VERIFY(SUCCEEDED(pWndData->m_pUxTheme->BufferedPaintUnInit()));
            delete pWndData->m_pUxTheme;
            delete pWndData->m_pDwmApiImpl;
            if(WndProc==(WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC))
            {
                VERIFY(WndProc==(WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR) pWndData->m_oldWndProc));
                VERIFY(pWndData==RemoveProp(hWnd, WINDOW_DATA_STRING)); 
                VERIFY(!LocalFree(pWndData));
            }
            break;
        case WM_NCDESTROY:
            if(WndProc==(WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC))
            {
                VERIFY(WndProc==(WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR) pWndData->m_oldWndProc));
                VERIFY(pWndData==RemoveProp(hWnd, WINDOW_DATA_STRING)); 
                VERIFY(!LocalFree(pWndData));
            }
            break;
                
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}






BOOL AeroAutoSubclass(HWND hWnd, DWORD dwFlags, DWORD dwReserved)
{
    if(!hWnd || !IsWindow(hWnd) || 0L!=dwReserved)  
    {
        SetLastError(ERROR_INVALID_PARAMETER);        
        return FALSE;
    }

    UINT uiRedrawMsg = RegisterWindowMessage(REDRAWSTRING);
    if(!uiRedrawMsg)
        return FALSE; // use the RegisterWindowMessage last error

    CDwmApiImpl *pDwm = NULL;
    CUxThemeAeroImpl *pUxTheme = NULL;
    PAERO_SUBCLASS_WND_DATA pWndData = NULL;
    DWORD dwLastError = ERROR_SUCCESS;
    MARGINS marGlassInset = {-1,-1,-1,-1};
    HRESULT hRes = S_OK;
    bool bBufferedPaintInitialized = false;
    ERROR_PARENT_AERO_SUBCLASS_WND_DATA errParentAeroData;
    ZeroMemory(&errParentAeroData, sizeof(errParentAeroData));
    errParentAeroData.m_dwFlags = dwFlags;
    errParentAeroData.m_hWndParent = hWnd;

    try
    {
        pDwm = new CDwmApiImpl;
    }
    catch (...)
    {
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
    }

    if(ERROR_SUCCESS!=dwLastError)
        goto CLEANUP;

    try
    {
        pUxTheme = new CUxThemeAeroImpl;
    }
    catch (...)
    {
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
    }

    if(ERROR_SUCCESS!=dwLastError)
        goto CLEANUP;

    pWndData = (PAERO_SUBCLASS_WND_DATA)LocalAlloc(LPTR, sizeof(AERO_SUBCLASS_WND_DATA));
    if(!pWndData)
    {
        dwLastError = GetLastError();
        goto CLEANUP;
    }

    if(!pDwm->Initialize())
    {
        dwLastError = GetLastError();
        goto CLEANUP;
    }

    if(!pUxTheme->Initialize())
    {
        dwLastError = GetLastError();
        goto CLEANUP;
    }

    if(pDwm->IsDwmCompositionEnabled() && !(dwFlags&ASC_NO_FRAME_EXTENSION))
    {
        /// 
        /// we do not evaluate the return value of pDwm->DwmExtendFrameIntoClientArea, because
        /// if composition is turned off in the tiny little race condition after the previous call
        /// to IsDwmCompositionEnabled and before the following call to DwmExtendFrameIntoClientArea,
        /// we would see DwmExtendFrameIntoClientArea fail. However, if composition is turned on again 
        /// aterwards, the UI can display composition again without problems:
        /// 
        pDwm->DwmExtendFrameIntoClientArea(hWnd, &marGlassInset);
    }

    hRes = pUxTheme->BufferedPaintInit();
    if(FAILED(hRes))
    {
        dwLastError = hRes;
        goto CLEANUP;
    }
    bBufferedPaintInitialized = true;

    if(!SetProp(hWnd, WINDOW_DATA_STRING, pWndData))
    {
        dwLastError = hRes;
        goto CLEANUP;
    }

    errParentAeroData.m_pdwError = &dwLastError;
    errParentAeroData.m_pWndParentAeroData = pWndData;
    pWndData->m_pDwmApiImpl = pDwm;
    pWndData->m_pUxTheme = pUxTheme;
    pWndData->m_uiRedrawMsg = uiRedrawMsg;

    if(dwFlags&ASC_NO_FRAME_EXTENSION)
        pWndData->m_dwFlags |= WD_NO_FRAME_EXTEND;

    if(!EnumChildWindows(hWnd, SubclassChildWindows, (LPARAM)&errParentAeroData))
    {
        if(ERROR_SUCCESS==dwLastError)
            dwLastError = GetLastError();
        goto CLEANUP;
    }
    
    
    if(ERROR_SUCCESS!=dwLastError)
        goto CLEANUP;


    pWndData->m_oldWndProc = (WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR) WndProc);
    if(!pWndData->m_oldWndProc)
    {
        dwLastError = GetLastError();
        goto CLEANUP;
    }




CLEANUP:

    if(ERROR_SUCCESS!=dwLastError)
    {
        RemoveProp(hWnd, WINDOW_DATA_STRING); // don't care if this fails


        if(pDwm)
            delete pDwm;
        if(pUxTheme)
        {
            if(bBufferedPaintInitialized)
                pUxTheme->BufferedPaintUnInit();
                
            delete pUxTheme;
        }
        if(pWndData)
            VERIFY(!LocalFree(pWndData));

        SetLastError(dwLastError);
        return FALSE;
    }

    return TRUE;
}

void DrawFocusRect(LPRECT prcFocus, HDC hdcPaint)
{
    DrawRect(prcFocus, hdcPaint, DashStyleDot, Color(0xFF, 0,0,0), 1.0);    
};

void DrawRect(LPRECT prc, HDC hdcPaint, DashStyle dashStyle, Color clr, REAL width)
{
    Pen*         myPen;
    Graphics*    myGraphics;
    myPen = new Pen(clr, width);
    if(myPen)
    {
        myPen->SetDashStyle(dashStyle);
        myGraphics = new Graphics(hdcPaint);
        if(myGraphics)
        {
            myGraphics->DrawRectangle(myPen, prc->left, prc->top, 
                prc->right - 1 - prc->left, prc->bottom - 1 - prc->top);
            delete myGraphics;
        }
        delete myPen;
    }
}

void DrawPolygon(const Point* points, IN INT count, HDC hdcPaint, DashStyle dashStyle, Color clr, REAL width)
{
    Pen*         myPen;
    Graphics*    myGraphics;
    myPen = new Pen(clr, width);
    if(myPen)
    {
        myPen->SetDashStyle(dashStyle);
        myGraphics = new Graphics(hdcPaint);
        if(myGraphics)
        {
            myGraphics->DrawPolygon(myPen, points, count);
            delete myGraphics;
        }
        delete myPen;
    }
}

void DrawLine(const Point &pt1, const Point &pt2, HDC hdcPaint, DashStyle dashStyle, Color clr, REAL width)
{
    Pen*         myPen;
    Graphics*    myGraphics;
    myPen = new Pen(clr, width);
    if(myPen)
    {
        myPen->SetDashStyle(dashStyle);
        myGraphics = new Graphics(hdcPaint);
        if(myGraphics)
        {
            myGraphics->DrawLine(myPen, pt1, pt2);
            delete myGraphics;
        }
        delete myPen;
    }
}




void FillRect(LPRECT prc, HDC hdcPaint, Color clr)
{
    Graphics*    myGraphics;
    SolidBrush *pBrush = new SolidBrush(clr);

    if(pBrush)
    {
        myGraphics = new Graphics(hdcPaint);
        if(myGraphics)
        {
            myGraphics->FillRectangle(pBrush, prc->left, prc->top, 
                prc->right - 1 - prc->left, prc->bottom - 1 - prc->top);

            delete myGraphics;
        }
        delete pBrush;
    }
}

int GetStateFromBtnState(LONG_PTR dwStyle, BOOL bHot, BOOL bFocus, LRESULT dwCheckState, int iPartId, BOOL bHasMouseCapture)
{
    int iState = 0;
    switch (iPartId)
    {
        case BP_PUSHBUTTON:
            iState = PBS_NORMAL;
            if (dwStyle&WS_DISABLED)
                iState = PBS_DISABLED;
            else
            {
                if(dwStyle&BS_DEFPUSHBUTTON)
                    iState = PBS_DEFAULTED;

                if(bHasMouseCapture && bHot)
                    iState = PBS_PRESSED;
                else if (bHasMouseCapture || bHot)
                    iState = PBS_HOT;
            }
            break;
        case BP_GROUPBOX:
            iState = (dwStyle & WS_DISABLED)?GBS_DISABLED:GBS_NORMAL;
            break;

        case BP_RADIOBUTTON:
            iState = RBS_UNCHECKEDNORMAL;
            switch(dwCheckState)
            {
                case BST_CHECKED:
                    if (dwStyle&WS_DISABLED)
                        iState = RBS_CHECKEDDISABLED;
                    else if(bFocus)
                        iState = RBS_CHECKEDPRESSED;
                    else if(bHot)
                        iState = RBS_CHECKEDHOT;
                    else
                        iState = RBS_CHECKEDNORMAL;       
                    break;
                case BST_UNCHECKED:
                    if (dwStyle&WS_DISABLED)
                        iState = RBS_UNCHECKEDDISABLED;
                    else if(bFocus)
                        iState = RBS_UNCHECKEDPRESSED;
                    else if(bHot)
                        iState = RBS_UNCHECKEDHOT;
                    else 
                        iState = RBS_UNCHECKEDNORMAL;       
                    break;
            }
            break;

        case BP_CHECKBOX:
            switch(dwCheckState)
            {
                case BST_CHECKED:
                    if (dwStyle&WS_DISABLED)
                        iState = CBS_CHECKEDDISABLED;
                    else if(bFocus)
                        iState = CBS_CHECKEDPRESSED;
                    else if(bHot)
                        iState = CBS_CHECKEDHOT;
                    else
                        iState = CBS_CHECKEDNORMAL;       
                    break;
                case BST_INDETERMINATE:
                    if (dwStyle&WS_DISABLED)
                        iState = CBS_MIXEDDISABLED;
                    else if(bFocus)
                        iState = CBS_MIXEDPRESSED;
                    else if(bHot)
                        iState = CBS_MIXEDHOT;
                    else 
                        iState = CBS_MIXEDNORMAL;       
                    break;
                case BST_UNCHECKED:
                    if (dwStyle&WS_DISABLED)
                        iState = CBS_UNCHECKEDDISABLED;
                    else if(bFocus)
                        iState = CBS_UNCHECKEDPRESSED;
                    else if(bHot)
                        iState = CBS_UNCHECKEDHOT;
                    else 
                        iState = CBS_UNCHECKEDNORMAL;       
                    break;
            }
            break;
        default:
            ASSERT(0);
            break;

    }

    return iState;
}


BOOL UnsubclassControl(HWND hWnd, WNDPROC Proc, PAERO_SUBCLASS_WND_DATA pWndData)
{
    ASSERT(pWndData->m_pUxTheme);
    ASSERT(pWndData->m_pDwmApiImpl);
    if(Proc==(WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC))
    {
        VERIFY(Proc==(WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR) pWndData->m_oldWndProc));
        VERIFY(pWndData==RemoveProp(hWnd, WINDOW_DATA_STRING)); 
        VERIFY(!LocalFree(pWndData));
        return TRUE;
    }

    return FALSE;
}

BOOL AeroSubClassControl(HWND hwnd, WNDPROC pWndProc, DWORD dwAddFlags)
{
    if(!hwnd || !IsWindow(hwnd) || !pWndProc)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    HWND hWndParent = GetParent(hwnd);
    if(!hWndParent)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    PAERO_SUBCLASS_WND_DATA pWndParentAeroData = (PAERO_SUBCLASS_WND_DATA)
        GetProp(hWndParent, WINDOW_DATA_STRING);
    
    if(!pWndParentAeroData)
    {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    return AeroSubClassChild(hwnd, pWndProc, dwAddFlags, pWndParentAeroData);
}


void ScreenToClient(HWND hWnd, LPRECT lprc)
{
    POINT pt;
    pt.x = lprc->left;
    pt.y = lprc->top;
    VERIFY(ScreenToClient(hWnd, &pt));
    lprc->left = pt.x;
    lprc->top = pt.y;

    pt.x = lprc->right;
    pt.y = lprc->bottom;
    VERIFY(ScreenToClient(hWnd, &pt));
    lprc->right = pt.x;
    lprc->bottom = pt.y;
}


void ClientToScreen(HWND hWnd, LPRECT lprc)
{
    POINT pt;
    pt.x = lprc->left;
    pt.y = lprc->top;
    VERIFY(ClientToScreen(hWnd, &pt));
    lprc->left = pt.x;
    lprc->top = pt.y;

    pt.x = lprc->right;
    pt.y = lprc->bottom;
    VERIFY(ClientToScreen(hWnd, &pt));
    lprc->right = pt.x;
    lprc->bottom = pt.y;
}


void Draw1PixelSolidLine(Point pt1, Point pt2, HDC hdcPaint, Color col)
{
    DrawLine(pt1, pt2, hdcPaint, DashStyleSolid, col, 1.0);
}



void DrawWndOnParentTransparent(HWND hWnd, PAERO_SUBCLASS_WND_DATA pWndData)
{
    RECT rcWnd;
    VERIFY(GetWindowRect(hWnd, &rcWnd));
    HWND hParent = GetParent(hWnd);
    if(hParent)
    {
        ScreenToClient(hParent, &rcWnd);

        HDC hdc = GetDC(hParent);
        if(hdc)
        {
            BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
            params.dwFlags        = BPPF_ERASE;

            HDC hdcPaint = NULL;
            HPAINTBUFFER hBufferedPaint = NULL;
            hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcWnd, BPBF_TOPDOWNDIB, &params, &hdcPaint);
            if (hdcPaint)
            {
                VERIFY(PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcWnd), RECTHEIGHT(rcWnd), BLACKNESS));
                VERIFY(S_OK==pWndData->m_pUxTheme->BufferedPaintSetAlpha(hBufferedPaint, &rcWnd, 0x00));
                VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));    
            }

            VERIFY(1==ReleaseDC(hParent, hdc));
        }
    }
}




void DrawFilledWndRectOnParent(HWND hWnd, Color clr)
{
    RECT rcWnd;
    VERIFY(GetWindowRect(hWnd, &rcWnd));
    HWND hParent = GetParent(hWnd);
    if(hParent)
    {
        ScreenToClient(hParent, &rcWnd);

        HDC hdc = GetDC(hParent);
        if(hdc)
        {
            FillRect(&rcWnd, hdc, clr);
            VERIFY(1==ReleaseDC(hWnd, hdc));
        }
    }
}





void DrawSolidWndRectOnParent(HWND hWnd, Color clr)
{
    RECT rcWnd;
    VERIFY(GetWindowRect(hWnd, &rcWnd));
    HWND hParent = GetParent(hWnd);
    if(hParent)
    {
        ScreenToClient(hParent, &rcWnd);

        HDC hdc = GetDC(hParent);
        if(hdc)
        {
            DrawRect(&rcWnd, hdc, DashStyleSolid, clr, 1.0);
            VERIFY(1==ReleaseDC(hWnd, hdc));
        }
    }
}

BOOL GetEditBorderColor(HWND hWnd, CUxThemeAeroImpl *pUxTheme, COLORREF *pClr)
{
    ASSERT(pUxTheme);
    ASSERT(pClr);
    
    HTHEME hTheme = pUxTheme->OpenThemeData(hWnd, L"Edit");
    if(hTheme)
    {
        VERIFY(S_OK==pUxTheme->GetThemeColor(hTheme, EP_BACKGROUNDWITHBORDER, EBWBS_NORMAL, TMT_BORDERCOLOR, pClr));
        VERIFY(S_OK==pUxTheme->CloseThemeData(hTheme));
        return TRUE;
    }

    return FALSE;
}

void DrawEditBorder(HWND hWnd, PAERO_SUBCLASS_WND_DATA pWndData)
{
    ASSERT(pWndData);
    ASSERT(pWndData->m_pUxTheme);

    LONG_PTR dwStyleEx = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    if(!(dwStyleEx&WS_EX_CLIENTEDGE))
        return;

    COLORREF cr = RGB(0x00, 0x00, 0x00);
    VERIFY(GetEditBorderColor(hWnd, pWndData->m_pUxTheme, &cr));

    Color clr;
    clr.SetFromCOLORREF(cr);
    DrawSolidWndRectOnParent(hWnd, clr );
}



BOOL AeroSubClassCtrl(HWND hwnd)
{
    DWORD dwError = ERROR_SUCCESS;
    ERROR_PARENT_AERO_SUBCLASS_WND_DATA errParentAeroData;
    ZeroMemory(&errParentAeroData, sizeof(errParentAeroData));
    errParentAeroData.m_dwFlags = ASC_SUBCLASS_ALL_CONTROLS;
    errParentAeroData.m_pdwError = &dwError;
    errParentAeroData.m_hWndParent = ::GetParent(hwnd);

    return SubclassChildWindows(hwnd, (LPARAM)&errParentAeroData);
}


