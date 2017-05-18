static char *rcsid = "$Id: aerocmbo.cpp,v 1.8 2007/05/20 09:57:23 cvs Exp $";
/*
*
* $RCSfile: aerocmbo.cpp,v $
* $Source: /cvs/common/aerocmbo.cpp,v $
* $Author: cvs $
* $Revision: 1.8 $
* $Date: 2007/05/20 09:57:23 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerocmbo.cpp,v $
* Revision 1.8  2007/05/20 09:57:23  cvs
* More defensive coding added
*
* Revision 1.7  2007/05/18 22:21:25  cvs
* Removed dead code
*
* Revision 1.6  2007/05/18 21:59:21  cvs
* Removed separate edit control subclassing code
*
* Revision 1.5  2007/05/18 15:57:25  cvs
* Improved edit child rendering
*
* Revision 1.4  2007/04/22 12:30:05  cvs
* Added code for comboboxex edit controls to work
*
* Revision 1.3  2007/04/15 17:49:55  cvs
* Added disfunctional comboboxex routines
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


LRESULT CALLBACK ComboProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                if (hdc)
                    PaintControl(hWnd, hdc, &ps.rcPaint, (pWndData->m_dwFlags & WD_DRAW_BORDER)!=0);
                EndPaint(hWnd, &ps);
                return 1;
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, ComboProc, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ComboProcEx(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
        case WM_CTLCOLOREDIT:
            {
                PAERO_SUBCLASS_WND_DATA pCtrlData = (PAERO_SUBCLASS_WND_DATA)GetProp((HWND)lParam, WINDOW_DATA_STRING);
                if(pCtrlData)
                    pCtrlData->m_dwFlags|=WD_IN_PAINT_CONTROL;
                PostMessage((HWND)lParam, pWndData->m_uiRedrawMsg, 0, NULL);
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            VERIFY(UnsubclassControl(hWnd, ComboProcEx, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}





#define EDIT_CLASSNAME _T("Edit")

BOOL CALLBACK SubclassEditCtrls(
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





BOOL AeroSubClassComboBox(HWND hwnd)
{
    BOOL bSuccess = FALSE;
    bSuccess = AeroSubClassControl(hwnd, ComboProc, WD_IN_PAINT_CONTROL);
    if(bSuccess)
        EnumChildWindows(hwnd, SubclassEditCtrls, (LPARAM)&bSuccess);
    return bSuccess;
}


#define COMBOBOX_CLASSNAME _T("ComboBox")

BOOL CALLBACK SubclassComboBoxCtrls(
  HWND hwnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
    PBOOL pbSuccess = (PBOOL)lParam;
    ASSERT(pbSuccess);
    
    TCHAR szWndClassName[dimof(COMBOBOX_CLASSNAME)];
    if(GetClassName(hwnd, szWndClassName, dimof(szWndClassName)))
    {
        if  (!_tcscmp(szWndClassName, COMBOBOX_CLASSNAME))
            *pbSuccess = AeroSubClassComboBox(hwnd);
    }

    return TRUE;
}




BOOL AeroSubClassComboBoxEx(HWND hwnd)
{
    BOOL bSuccess = FALSE;
    bSuccess = AeroSubClassControl(hwnd, ComboProcEx, 0);
    if(bSuccess)
        EnumChildWindows(hwnd, SubclassComboBoxCtrls, (LPARAM)&bSuccess);
    return bSuccess;
}
