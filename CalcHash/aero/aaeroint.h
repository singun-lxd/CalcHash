#ifndef AAEROINT_H__
#define AAEROINT_H__

/*
*
* $RCSfile: aaeroint.h,v $
* $Source: /cvs/common/aaeroint.h,v $
* $Author: cvs $
* $Revision: 1.18 $
* $Date: 2007/05/20 13:33:40 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aaeroint.h,v $
* Revision 1.18  2007/05/20 13:33:40  cvs
* Added comments and some more debug code and improved robustness
*
* Revision 1.17  2007/05/18 21:20:28  cvs
* Added DrawFilledWndRectOnParent
*
* Revision 1.16  2007/05/13 10:39:05  cvs
* Added compat changes for VS2005
*
* Revision 1.15  2007/05/11 20:52:52  cvs
* Added the faster and flicker-free animation control code in draft state
*
* Revision 1.14  2007/05/06 14:25:29  cvs
* x64 compatibility changes
*
* Revision 1.13  2007/05/04 15:37:56  cvs
* #ifdeffed TRACE
*
* Revision 1.12  2007/05/04 11:39:13  cvs
* Drawing the border now in the edit border color
*
* Revision 1.11  2007/04/22 18:55:04  cvs
* Added DrawEditBorder
*
* Revision 1.10  2007/04/22 15:42:04  cvs
* Now painting multiline edit control background properly if disabled
*
* Revision 1.9  2007/04/15 17:49:55  cvs
* Added disfunctional comboboxex routines
*
* Revision 1.8  2007/04/15 15:04:55  cvs
* Added selective subclassing
*
* Revision 1.7  2007/04/15 13:25:33  cvs
* removed monthcal subclassing code
*
* Revision 1.6  2007/04/10 17:35:56  cvs
* Added tree control
*
* Revision 1.5  2007/04/09 18:17:47  cvs
* Added IpAddress control code
*
* Revision 1.4  2007/04/09 15:37:47  cvs
* Added selection code for slider control
*
* Revision 1.3  2007/04/09 12:02:58  cvs
* Added new file for slider controls
*
* Revision 1.2  2007/04/08 13:37:51  cvs
* Added standard header
* 
*/


#include <windows.h>
#include <tchar.h>
#include <gdiplus.h>
using namespace Gdiplus;


/// 
/// internal data types, functions and macros for autoaero implementation
/// 

class CDwmApiImpl;
class CUxThemeAeroImpl;

typedef struct tagAERO_SUBCLASS_WND_DATA
{
    DWORD m_dwFlags;
    WNDPROC m_oldWndProc;
    CDwmApiImpl *m_pDwmApiImpl;
    CUxThemeAeroImpl *m_pUxTheme;
    UINT m_uiRedrawMsg;
    union
    {
        DWORD m_dwSelFirst;
        int m_iCurrentSliderPos;
    };
    DWORD m_dwSelLast;
    LPVOID m_pParam;
}
AERO_SUBCLASS_WND_DATA, *PAERO_SUBCLASS_WND_DATA;

typedef struct tagERROR_PARENT_AERO_SUBCLASS_WND_DATA
{
    PDWORD m_pdwError;
    DWORD m_dwFlags;
    HWND m_hWndParent;
    PAERO_SUBCLASS_WND_DATA m_pWndParentAeroData;
}
ERROR_PARENT_AERO_SUBCLASS_WND_DATA, *PERROR_PARENT_AERO_SUBCLASS_WND_DATA;

class CLocalFreeData
{
    LPVOID m_lpData;

    public:
        CLocalFreeData(LPVOID lpData) : m_lpData(lpData)
        {

        }

        ~CLocalFreeData(void)
        {
            DWORD dwLastError = GetLastError();
            VERIFY(!LocalFree(m_lpData));
           SetLastError(dwLastError);
        }
        void Clear(void)
        {
            m_lpData = NULL;
        }
};

BOOL AeroAutoSubClassChild(HWND hwnd, WNDPROC pWndProc, PDWORD pdwError, DWORD dwAddFlags);

void DrawFocusRect(LPRECT prcFocus, HDC hdcPaint);
void DrawRect(LPRECT prc, HDC hdcPaint, DashStyle dashStyle, Color clr, REAL width);
void FillRect(LPRECT prc, HDC hdcPaint, Color clr);
void DrawPolygon(const Point* points, IN INT count, HDC hdcPaint, DashStyle dashStyle, Color clr, REAL width);
void DrawLine(const Point &pt1, const Point &pt2, HDC hdcPaint, DashStyle dashStyle, Color clr, REAL width);


#define WINDOW_DATA_STRING _T("AeroAutoSubclass")

#ifndef RECTWIDTH
#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#endif

#ifndef RECTHEIGHT
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)
#endif

#define WD_IN_PAINT_CONTROL   0x1
#define WD_DRAW_BORDER        0x2 
#define WD_RETURN_BLACK_BRUSH 0x4
#define WD_RETURN_WHITE_BRUSH 0x8
#define WD_PLAY               0x10
#define WD_NO_FRAME_EXTEND    0x20
#define WD_CARET_HIDDEN       0x40
#define WD_AVI_INIT_DONE      0x80


#ifndef dimof
#define dimof(a) (sizeof(a)/sizeof(a[0]))
#endif ///  dimof

#ifndef WM_DWMCOMPOSITIONCHANGED         
#define WM_DWMCOMPOSITIONCHANGED        0x031E
#endif /// WM_DWMCOMPOSITIONCHANGED         



BOOL UnsubclassControl(HWND hWnd, WNDPROC Proc, PAERO_SUBCLASS_WND_DATA pWndData);
int GetStateFromBtnState(LONG_PTR dwStyle, BOOL bHot, BOOL bFocus, LRESULT dwCheckState, int iPartId, BOOL bHasMouseCapture);
BOOL AeroSubClassControl(HWND hwnd, WNDPROC pWndProc, DWORD dwAddFlags);
void PaintControl(HWND hWnd, HDC hdc, RECT* prc, bool bDrawBorder);
BOOL PaintBufferedRect(HDC hdc, LPRECT lprc, CUxThemeAeroImpl *pUx);

#ifdef _DEBUG
#ifndef TRACE 
void __cdecl TRACE(LPCTSTR lpszFormat, ...);
#endif // TRACE
#endif // _DEBUG

void ScreenToClient(HWND hWnd, LPRECT lprc);
void ClientToScreen(HWND hWnd, LPRECT lprc);
void Draw1PixelSolidLine(Point pt1, Point pt2, HDC hdcPaint, Color col);
void DrawSolidWndRectOnParent(HWND hWnd, Color clr);
void DrawFilledWndRectOnParent(HWND hWnd, Color clr);
void DrawEditBorder(HWND hWnd, PAERO_SUBCLASS_WND_DATA pWndData);
BOOL GetEditBorderColor(HWND hWnd, CUxThemeAeroImpl *pUxTheme, COLORREF *pClr);


#endif // AAEROINT_H__
