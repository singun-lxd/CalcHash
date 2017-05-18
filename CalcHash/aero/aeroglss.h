#ifndef AEROGLAS_H__
#define AEROGLAS_H__


/*
*
* $RCSfile: aeroglss.h,v $
* $Source: /cvs/common/aeroglss.h,v $
* $Author: cvs $
* $Revision: 1.6 $
* $Date: 2007/05/17 18:21:27 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aeroglss.h,v $
* Revision 1.6  2007/05/17 18:21:27  cvs
* Including sal.h now
*
* Revision 1.5  2007/05/17 13:53:57  cvs
* Added a number of SAL annotations
*
* Revision 1.4  2007/05/13 11:12:41  cvs
* Added compat changes for VS2005
*
* Revision 1.3  2007/05/06 14:28:21  cvs
* x64 compatibility changes
* 
*/

#include <specstrings.h>


#if _MSC_VER<1400
#include <sal.h>
#endif // _MSC_VER<1400

#define TMSCHEMA_H // this excludes the deprecated tmschema.h without dependency on _WIN32_WINNT macro
#include <windows.h>
#include <Uxtheme.h>
#include <vssym32.h>


class CDwmApiImpl
{
    private:
        HINSTANCE m_hDwmApiLib;
        BOOL IsInitialized(void);

    public:
        CDwmApiImpl(void);
        ~CDwmApiImpl(void);
        BOOL Initialize(void);
        HRESULT DwmExtendFrameIntoClientArea(HWND hWnd,const MARGINS* pMarInset);
        BOOL IsDwmCompositionEnabled(void);
        HRESULT DwmEnableComposition(UINT uCompositionAction);
};

/// macros stolen from dwmapi.h:
#ifndef DWM_EC_DISABLECOMPOSITION
#define DWM_EC_DISABLECOMPOSITION 0
#endif /// DWM_EC_DISABLECOMPOSITION

#ifndef DWM_EC_ENABLECOMPOSITION
#define DWM_EC_ENABLECOMPOSITION 1
#endif /// DWM_EC_ENABLECOMPOSITION


class CUxThemeAeroImpl
{
    private:
      HINSTANCE m_hUxThemeLib;
      BOOL IsInitialized(void);

    public:
        CUxThemeAeroImpl(void);
        BOOL Initialize(void);
        ~CUxThemeAeroImpl(void);
        HRESULT BufferedPaintInit(void);
        HRESULT BufferedPaintUnInit(void);
        HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
        BOOL DetermineGlowSize(int *piSize, LPCWSTR pszClassIdList = NULL);
        HRESULT CloseThemeData(HTHEME hTheme);
        HPAINTBUFFER BeginBufferedPaint(HDC hdcTarget, const RECT* prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc);
        HRESULT EndBufferedPaint(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);
        HRESULT DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS *pOptions);
        HRESULT GetThemeInt(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int *piVal);
        HRESULT GetThemeSysFont(HTHEME hTheme, int iFontId, LOGFONTW *plf);
        HRESULT BufferedPaintSetAlpha(HPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha);
        HRESULT BufferedPaintMakeOpaque_(HPAINTBUFFER hBufferedPaint, const RECT *prc);
        HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
        HRESULT GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect);
        HRESULT GetThemeBackgroundExtent(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pContentRect, LPRECT pExtentRect);
        HRESULT GetThemeBitmap(HTHEME hTheme, int iPartId, int iStateId, int iPropId, ULONG dwFlags, HBITMAP *phBitmap);
        HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc, const RECT *prc);
        BOOL IsThemeBackgroundPartiallyTransparent(HTHEME hTheme,int iPartId, int iStateId);
        HRESULT DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect);
        HRESULT GetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF *pColor);
        HRESULT GetThemePartSize(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize,SIZE *psz);
        HRESULT GetThemePosition(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint);
        HRESULT GetThemeMargins(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, MARGINS *pMargins);
        HRESULT GetThemeMetric(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int *piVal);
        HRESULT GetThemeRect(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPRECT pRect);
};


#endif /// AEROGLAS
