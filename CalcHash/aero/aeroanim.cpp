static char *rcsid = "$Id: aeroanim.cpp,v 1.10 2007/05/20 08:46:45 cvs Exp $";
/*
*
* $RCSfile: aeroanim.cpp,v $
* $Source: /cvs/common/aeroanim.cpp,v $
* $Author: cvs $
* $Revision: 1.10 $
* $Date: 2007/05/20 08:46:45 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aeroanim.cpp,v $
* Revision 1.10  2007/05/20 08:46:45  cvs
* Added comments and some more debug code
*
* Revision 1.9  2007/05/13 12:19:34  cvs
* x64 compat changes
*
* Revision 1.8  2007/05/13 11:21:09  cvs
* Added compat changes for VS2005
*
* Revision 1.7  2007/05/12 15:47:01  cvs
* Removed conditional compilation
*
* Revision 1.6  2007/05/12 15:43:45  cvs
* Added deletion of temp file
*
* Revision 1.5  2007/05/12 15:14:38  cvs
* Added multiple open option
*
* Revision 1.4  2007/05/11 20:52:52  cvs
* Added the faster and flicker-free animation control code in draft state
*
* Revision 1.3  2007/05/06 14:25:30  cvs
* x64 compatibility changes
*
* Revision 1.2  2007/04/08 13:37:51  cvs
* Added standard header
* 
*/

#include <windows.h>
#include <tchar.h>
#include <vfw.h>
#include "safassrt.h"
#include "aaeroint.h"
#include "aerosubc.h"
#include "aeroglss.h"
#include <process.h>
#include <gdiplus.h>
using namespace Gdiplus;

#pragma comment(lib, "vfw32.lib")



#define AVI_REDRAW_FALLBACK       0x1
#define AVI_REDRAW_STOP           0x2
#define AVI_REDRAW_PLAY           0x4
#define AVI_REDRAW_PLAYNOADVANCE  0x8
#define AVI_DESTROY_VFW_TYPES    0x10


static HANDLE CreateTempFile(LPCTSTR szPrefix, LPTSTR szFileName) 
{
    ASSERT(szFileName);    
    // Get temp dir.
    TCHAR szDir[MAX_PATH];
    if (GetTempPath(sizeof(szDir)/sizeof(TCHAR), szDir) == 0)
        return NULL;

    if (!GetTempFileName(szDir, szPrefix, 0, szFileName))
        return NULL;

    // Open temp file.
    HANDLE hTemp = CreateFile(szFileName,
        GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ,      
        NULL,   // Default security descriptor
        CREATE_ALWAYS,   
        FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | 
        FILE_ATTRIBUTE_TEMPORARY,
        NULL);

    return hTemp == INVALID_HANDLE_VALUE ? NULL : hTemp;
}
 

typedef struct tagAVI_CTRL_DATA
{
    HDRAWDIB m_hDrawDib;
    PAVISTREAM m_pAS;
    PGETFRAME m_pGF;
    DWORD m_dwTimerInterval;
    AVISTREAMINFO m_asi;
    COLORREF m_crTransparent;
    HANDLE m_hThread;
    HANDLE m_hThreadStopEvent;
    HWND m_hWnd;
    DWORD m_dwFrames;
    DWORD m_dwCurrentFrame;
    DWORD m_dwCurrentRepeat;
    UINT m_uiMsg;
    UINT m_wFrom; 
    UINT m_wTo; 
    UINT m_cRepeat;
    TCHAR m_szFileName[MAX_PATH];
}
AVI_CTRL_DATA, *PAVI_CTRL_DATA;

unsigned __stdcall AnimationControlThread(LPVOID lpv)
{
    ASSERT(lpv);
    PAVI_CTRL_DATA pacd = (PAVI_CTRL_DATA)lpv;

    if(!pacd->m_hThreadStopEvent)
        return ERROR_INVALID_PARAMETER;


    while (WAIT_TIMEOUT==WaitForSingleObject(pacd->m_hThreadStopEvent, pacd->m_dwTimerInterval))
        PostMessage(pacd->m_hWnd, pacd->m_uiMsg, AVI_REDRAW_PLAY, NULL);
    
    return 0L;
} 


static void RedrawStop(PAVI_CTRL_DATA pacd)
{
    ASSERT(pacd);
    if(pacd->m_hThreadStopEvent)
        VERIFY(SetEvent(pacd->m_hThreadStopEvent));    

    if(pacd->m_hThread)
    {
        VERIFY(WAIT_OBJECT_0==WaitForSingleObject(pacd->m_hThread, INFINITE));
        VERIFY(CloseHandle(pacd->m_hThread));
        pacd->m_hThread = NULL;
    }

    if(pacd->m_hThreadStopEvent)
    {
        VERIFY(CloseHandle(pacd->m_hThreadStopEvent));    
        pacd->m_hThreadStopEvent = NULL;
    }
}

void DestroyResources(PAVI_CTRL_DATA pacd)
{
    ASSERT(pacd);
    if (pacd->m_pGF)
    {
	    VERIFY(AVIStreamGetFrameClose(pacd->m_pGF) == NOERROR);
	    pacd->m_pGF = NULL;
    }

    // release avi stream
    if (pacd->m_pAS)
    {
	    VERIFY(0==AVIStreamRelease(pacd->m_pAS));
        pacd->m_pAS = NULL;
    }

    /// 
    /// delete the file, if any:
    /// 
    if(pacd->m_szFileName[0])
    {
        VERIFY(DeleteFile(pacd->m_szFileName));
        pacd->m_szFileName[0] = _T('\0');
    }
}


static LRESULT CALLBACK AnimBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
    if(WM_DESTROY!=uMsg && WM_NCDESTROY!=uMsg && /*WM_PAINT!=uMsg &&*/ ACM_PLAY!=uMsg && ACM_STOP!=uMsg &&
        pWndDataParent && !pWndData->m_pDwmApiImpl->IsDwmCompositionEnabled())
        return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);

    if(uMsg==pWndData->m_uiRedrawMsg) 
    {

        switch (wParam)
        {
            case AVI_DESTROY_VFW_TYPES:
            {
                ASSERT(pWndData->m_pParam);
                PAVI_CTRL_DATA pacd = (PAVI_CTRL_DATA)pWndData->m_pParam;
                DestroyResources(pacd);
            }
                break;
            case AVI_REDRAW_STOP:
            {
                ASSERT(pWndData->m_pParam);
                PAVI_CTRL_DATA pacd = (PAVI_CTRL_DATA)pWndData->m_pParam;
                RedrawStop(pacd);
            }
                break;
            case AVI_REDRAW_PLAY:
            case AVI_REDRAW_PLAYNOADVANCE:
            {
                ASSERT(pWndData->m_pParam);
                PAVI_CTRL_DATA pacd = (PAVI_CTRL_DATA)pWndData->m_pParam;


                if(pacd->m_dwCurrentFrame==pacd->m_dwFrames || pacd->m_dwCurrentFrame>pacd->m_wTo)
                {
                    pacd->m_dwCurrentFrame = pacd->m_wFrom;
                    pacd->m_dwCurrentRepeat++;
                }

                if(pacd->m_dwCurrentRepeat>pacd->m_cRepeat)
                {
                    SendMessage(hWnd, ACM_STOP, 0L, NULL);
                    pacd->m_dwCurrentFrame = pacd->m_wFrom;
                    pacd->m_dwCurrentRepeat = 0L;
                    break;
                }

                HDC hdc = NULL;
                RECT rcClient;

                VERIFY(GetClientRect(hWnd, &rcClient));
                hdc = GetDC(hWnd);
                if(hdc)
                {
	                LPBITMAPINFOHEADER lpBi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pacd->m_pGF, (LONG)pacd->m_dwCurrentFrame);
	                ASSERT(lpBi);
                    if (lpBi)
	                {
                        Bitmap *pBmp = NULL;

                        HDC hdcCompat = CreateCompatibleDC(hdc);
                        if(hdcCompat)
                        {
                            HBITMAP hBmp = CreateCompatibleBitmap(hdc, RECTWIDTH(pacd->m_asi.rcFrame),  RECTHEIGHT(pacd->m_asi.rcFrame));
                            if(hBmp)
                            {
                        
                                HBITMAP hBmpOld = (HBITMAP)SelectObject(hdcCompat, hBmp);
                                if(hBmpOld)
                                {
                                    /// 
                                    VERIFY(DrawDibDraw(pacd->m_hDrawDib, hdcCompat, 
                                        rcClient.left, rcClient.top,
						                -1, -1, lpBi, NULL,
						                0, 0, -1, -1, 0));

                                    pBmp = new Bitmap(hBmp, NULL);
                                    VERIFY(hBmp==SelectObject(hdcCompat, hBmpOld));
                                }
                                VERIFY(DeleteObject(hBmp));
                            }

                            VERIFY(DeleteDC(hdcCompat));
                        }

                        if(pBmp)
                        {
                            /// 
                            /// replace all transparent colors with a transparent alpha channel
                            /// 
                            Bitmap *pAlphaBmp = pBmp->Clone(0, 0, pBmp->GetWidth(), pBmp->GetHeight(), PixelFormat32bppARGB);

                            if(pAlphaBmp)
                            {
                                LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
                        
                                if(dwStyle & ACS_TRANSPARENT)
                                {
                                    INT iX, iY;
                                    for (iX = 0;iX<(INT)pBmp->GetWidth();iX++)
                                    {
                                        for (iY = 0;iY<(INT)pBmp->GetHeight();iY++)
                                        {
                                            Color col;
                                            VERIFY(Ok==pBmp->GetPixel(iX, iY, &col));
                                            BYTE r = col.GetR();
                                            BYTE g = col.GetG();
                                            BYTE b = col.GetB();
                                            if (pacd->m_crTransparent == RGB(r, g, b))
                                            {
                                                Color colnew(0x00, r, g, b);
										        VERIFY(Ok==pAlphaBmp->SetPixel(iX, iY, colnew));
                                            }
                                        }
                                    }
                                }

                                /// 
                                /// release pBmp as early as possible:
                                /// 
                                delete pBmp;
                                pBmp = NULL;


                                /// 
                                /// now draw the transparent bitmap:
                                /// 
                                BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                                params.dwFlags        = BPPF_ERASE;


                                HDC hdcPaint = NULL;
                                HPAINTBUFFER hBufferedPaint = NULL;

                                hdcPaint = NULL;
                                int iXPos = 0;
                                int iYPos = 0;

                                if(dwStyle & ACS_CENTER)
                                {
                                    iXPos = (RECTWIDTH(rcClient) - RECTWIDTH(pacd->m_asi.rcFrame))/2; 
                                    iYPos = (RECTHEIGHT(rcClient) - RECTHEIGHT(pacd->m_asi.rcFrame))/2; 
                                }

                                hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                                if (hdcPaint)
                                {
                                    Graphics*    myGraphics = new Graphics(hdcPaint);
                                    if(myGraphics)
                                    {
                                        CachedBitmap *pcbmp = new CachedBitmap(pAlphaBmp, myGraphics);

                                        if(pcbmp)
                                        {
                                            VERIFY(Ok==myGraphics->DrawCachedBitmap(pcbmp, iXPos, iYPos));
                                            delete pcbmp;
                                        }
                                        delete myGraphics;
                                    }


                                    VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));    
                                }

                                delete pAlphaBmp;

                                DrawEditBorder(hWnd, pWndData);

                            }

                            if(pBmp)
                                delete pBmp;
                        }

                    }

                    VERIFY(1==ReleaseDC(hWnd, hdc));
                }
        
                if(AVI_REDRAW_PLAY==wParam)
                    pacd->m_dwCurrentFrame++;
            }
        }       
    }


    if(uMsg==WM_TIMER || uMsg==pWndData->m_uiRedrawMsg)
    {
        if(pWndData->m_pParam) 
        {
            PAVI_CTRL_DATA pacd = (PAVI_CTRL_DATA)pWndData->m_pParam;
            if(pacd->m_dwTimerInterval)
                return 1;
        }
        
        /// 
        /// this is the cpu intensive and (sometimes, on low-end machines) flickering fallback code:
        /// 
        LRESULT lRet = 1;
        HDC hdc = NULL;
        RECT rcClient;
        
        VERIFY(GetClientRect(hWnd, &rcClient));
        hdc = GetDC(hWnd);
        if(hdc)
        {
            BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
            params.dwFlags        = BPPF_ERASE;


            HDC hdcPaint = NULL;
            HPAINTBUFFER hBufferedPaint = NULL;
            Bitmap *pAlphaBmp = NULL;

            HDC hdcCompat = CreateCompatibleDC(hdc);
            if(hdcCompat)
            {
                HBITMAP hBmp = CreateCompatibleBitmap(hdc, RECTWIDTH(rcClient),  RECTHEIGHT(rcClient));
                if(hBmp)
                {
                    HBITMAP hBmpOld = (HBITMAP)SelectObject(hdcCompat, hBmp);
                    if(hBmpOld)
                    {
                        /// 
                        /// we send WM_PRINTCLIENT twice, the first time we enforce a black background,
                        /// the second time we enforce a white background. By setting those pixels to 
                        /// transparent (alpha=0x00) that are different, we not only make those parts 
                        /// of the client area transparent that would otherwise get an ugly battleship 
                        /// grey background but for free also those that should be transparent if the
                        /// ACS_TRANSPARENT style bit is set for the control:
                        /// 
                        pWndData->m_dwFlags|=WD_RETURN_BLACK_BRUSH;
                        SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM)hdcCompat, PRF_CLIENT/*|PRF_ERASEBKGND*/);                                
                        Bitmap *pBmpBlack = new Bitmap(hBmp, NULL);
                        pWndData->m_dwFlags&=~WD_RETURN_BLACK_BRUSH;

                        pWndData->m_dwFlags|=WD_RETURN_WHITE_BRUSH;
                        SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM)hdcCompat, PRF_CLIENT/*|PRF_ERASEBKGND*/);                                
                        pWndData->m_dwFlags&=~WD_RETURN_WHITE_BRUSH;

                        Bitmap *pBmpWhite = new Bitmap(hBmp, NULL);
                        

                        if(pBmpBlack && pBmpWhite)
                        {
                            pAlphaBmp = pBmpBlack->Clone(0, 0, pBmpBlack->GetWidth(), pBmpBlack->GetHeight(), PixelFormat32bppARGB);

                            if(pAlphaBmp)
                            {
                                INT iX, iY;
                                for (iX = 0;iX<(INT)pBmpBlack->GetWidth();iX++)
                                {
                                    for (iY = 0;iY<(INT)pBmpBlack->GetHeight();iY++)
                                    {
                                        Color colBlk;
                                        VERIFY(Ok==pBmpBlack->GetPixel(iX, iY, &colBlk));
                                        Color colWht;
                                        VERIFY(Ok==pBmpWhite->GetPixel(iX, iY, &colWht));
                                        

                                        BYTE rb = colBlk.GetR();
                                        BYTE gb = colBlk.GetG();
                                        BYTE bb = colBlk.GetB();
                                        BYTE rw = colWht.GetR();
                                        BYTE gw = colWht.GetG();
                                        BYTE bw = colWht.GetB();

                                        if(rb!=rw || gb!=gw || bb!=bw)
                                        {
                                            ///  
                                            /// if pixels are different, then this must be the 
                                            /// background color from WM_CTLCOLORSTATIC, it 
                                            /// must be transparent in any case:
                                            /// 
                                            Color colnew(0x00, rb, gb, bb);
											VERIFY(Ok==pAlphaBmp->SetPixel(iX, iY, colnew));
                                        }
                                    }
                                }
                            }
                        }

                        if(pBmpBlack)
                            delete pBmpBlack;
                        
                        if(pBmpWhite)
                            delete pBmpWhite;
                        
                        
                        VERIFY(hBmp==SelectObject(hdcCompat, hBmpOld));
                    }
                    VERIFY(DeleteObject(hBmp));
                }

                VERIFY(DeleteDC(hdcCompat));
            }


            /// 
            /// call the original implementation of the WM_TIMER handler. 
            /// If we don't do so, the animation will not advance:
            /// 
            lRet = CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);

            
            hdcPaint = NULL;
            
            hBufferedPaint = pWndData->m_pUxTheme->BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, &params, &hdcPaint);
            if (hdcPaint)
            {
                if(pAlphaBmp)
                {
                    Graphics*    myGraphics = new Graphics(hdcPaint);
                    if(myGraphics)
                    {
                        CachedBitmap *pcbmp = new CachedBitmap(pAlphaBmp, myGraphics);

                        if(pcbmp)
                        {
                            VERIFY(Ok==myGraphics->DrawCachedBitmap(pcbmp, 0,0));
                            delete pcbmp;
                        }
                        delete myGraphics;
                    }

                    delete pAlphaBmp;
                }

                VERIFY(S_OK==pWndData->m_pUxTheme->EndBufferedPaint(hBufferedPaint, TRUE));    
            }

            VERIFY(1==ReleaseDC(hWnd, hdc));
        }

        return lRet;
    }

    switch(uMsg)
    {
        case WM_NCPAINT:
            return 1;
        
        case ACM_PLAY:
            
            if(pWndData->m_pParam)
            {
                SendMessage(hWnd, pWndData->m_uiRedrawMsg, AVI_REDRAW_STOP, NULL);
                PAVI_CTRL_DATA pacd = (PAVI_CTRL_DATA)pWndData->m_pParam;
                
                pacd->m_wFrom = LOWORD(lParam);
                pacd->m_dwCurrentFrame = pacd->m_wFrom;
                pacd->m_wTo = HIWORD(lParam);
                pacd->m_cRepeat = (UINT)wParam;
                
                pacd->m_hThreadStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                
                if(pacd->m_hThreadStopEvent)
                    pacd->m_hThread = (HANDLE)_beginthreadex(NULL, 0L, AnimationControlThread, pacd, 0L, NULL);
            }
            pWndData->m_dwFlags|=WD_PLAY;
            break;
        case ACM_STOP:
            
            if(pWndData->m_pParam)    
                SendMessage(hWnd, pWndData->m_uiRedrawMsg, AVI_REDRAW_STOP, NULL);
            pWndData->m_dwFlags&=~WD_PLAY;
            break;

        case ACM_OPEN:
        {
            /// 
            /// we have to be prepared that someone does a 
            /// SendMessage(hWnd, ACM_OPEN multiple times.
            /// In that case we first have to stop the old 
            /// animation thread and close all sync objects 
            /// plus the file.
            /// 
            if(!(pWndData->m_dwFlags&WD_AVI_INIT_DONE))
            {
                AVIFileInit();
                pWndData->m_dwFlags|=WD_AVI_INIT_DONE;
            }

            if(!pWndData->m_pParam)
                pWndData->m_pParam = LocalAlloc(LPTR, sizeof(AVI_CTRL_DATA));
            else
            {
                /// 
                /// If we get here, someone sent ACM_OPEN twice:
                /// 
                SendMessage(hWnd, pWndData->m_uiRedrawMsg, AVI_REDRAW_STOP, NULL);
                SendMessage(hWnd, pWndData->m_uiRedrawMsg, AVI_DESTROY_VFW_TYPES, NULL);
            }
                


            if(pWndData->m_pParam)
            {
                PAVI_CTRL_DATA pacd = (PAVI_CTRL_DATA)pWndData->m_pParam;


                ASSERT(!pacd->m_hThread);
                ASSERT(!pacd->m_hThreadStopEvent);

                pacd->m_hWnd = hWnd;
                pacd->m_uiMsg = pWndData->m_uiRedrawMsg;
                
                if(!pacd->m_hDrawDib)
                    pacd->m_hDrawDib = DrawDibOpen();
                
                if(pacd->m_hDrawDib)
                {
                    LPCTSTR szFile = NULL;
                    HANDLE hFile = NULL;
                    if(HIWORD(lParam)==0)
                    {
                        /// 
                        /// it is a resource id. Create a temp file from the resource binary
                        /// 
                        HINSTANCE hInst = (HINSTANCE) wParam;
                  	    HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(LOWORD(lParam)), _T("AVI"));
                        if(hRes)
                        {
                            HGLOBAL hMem = LoadResource(hInst, hRes);
                            if(hMem)
                            {
                                DWORD dwSizeRes = SizeofResource(hInst, hRes);
                                LPBYTE lpData = (LPBYTE)LockResource(hMem);
                                if(lpData)
                                {
                                    hFile = CreateTempFile(_T("aer"), pacd->m_szFileName);
                            
                                    if(hFile) // test must be for non-NULL!!
                                    {
                                        DWORD dwWritten = 0L;
                                        if(WriteFile(hFile, lpData, dwSizeRes, &dwWritten, NULL))
                                        {
                                            ASSERT(GetFileSize(hFile, NULL)==dwSizeRes);
                                            szFile = pacd->m_szFileName;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        /// 
                        /// it is a file name 
                        /// 
                        szFile = (LPCTSTR)lParam;
                    }

                    if(szFile)
                    {
                        // open AVI file:
                        HRESULT hr = AVIStreamOpenFromFile(&pacd->m_pAS, szFile, streamtypeVIDEO,
                            0, OF_READ|OF_SHARE_EXCLUSIVE, NULL);
                    
                        if (FAILED(hr))
                            pacd->m_pAS = NULL;
                        else
                        {
                            ASSERT(pacd->m_pAS);
                            pacd->m_pGF = AVIStreamGetFrameOpen(pacd->m_pAS, NULL);
	                        
                            // get number of frames
	                        LONG lFrames = AVIStreamLength(pacd->m_pAS);
	                        ASSERT(-1!=lFrames);

	                        // calculate timer delay
	                        LONG lLTime = AVIStreamEndTime(pacd->m_pAS);
	                        ASSERT(-1!=lLTime);

                            if(-1!=lFrames && -1!=lLTime)
                            {
                                if(lFrames)
                                {
                              	    pacd->m_dwTimerInterval = (UINT)(lLTime / lFrames);
                                    pacd->m_dwFrames = (DWORD)lFrames;
                                }
                            }

                            /// 
                            /// get avi information:
                            /// 
	                        VERIFY(!AVIStreamInfo(pacd->m_pAS, &pacd->m_asi, sizeof(AVISTREAMINFO)));

	                        LPBITMAPINFO lpbi;

                            /// 
	                        /// fetch first frame
                            /// 
	                        lpbi = (LPBITMAPINFO)AVIStreamGetFrame(pacd->m_pGF, 0);
	                        ASSERT(lpbi);

                            /// 
	                        /// currently works only with 8bit-BMPs
                            /// 
	                        ASSERT(lpbi && 8==lpbi->bmiHeader.biBitCount);
	                        
                            /// 
	                        /// get first pixel and use as index in the color table
                            /// 
	                        LPBYTE lpbyIndex = (LPBYTE)((DWORD_PTR)lpbi + lpbi->bmiHeader.biSize + (lpbi->bmiHeader.biClrUsed*sizeof(RGBQUAD)));
	                        RGBQUAD* pRGBFirst = (RGBQUAD*)(&lpbi->bmiColors[*lpbyIndex]);

	                        pacd->m_crTransparent = RGB(pRGBFirst->rgbRed, pRGBFirst->rgbGreen, pRGBFirst->rgbBlue);
                            LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
                            if(dwStyle &ACS_AUTOPLAY)
                                SendMessage(hWnd, ACM_PLAY, (WPARAM)-1, MAKELONG(0, -1));
                            else
                                SendMessage(hWnd, ACM_PLAY, 0, MAKELONG(0, 0));
                        }
                    }
                    else
                        pacd->m_szFileName[0] = _T('\0'); // zero terminate it to length 0

                    if(hFile)
                        VERIFY(CloseHandle(hFile));
                }
            }
        }
            break;
        case WM_PAINT:
            {
                LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
				PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                hdc;

                if(pWndData->m_pParam)
                    PostMessage(hWnd, pWndData->m_uiRedrawMsg, AVI_REDRAW_PLAYNOADVANCE, NULL);
                else
                {
                    /// 
                    /// fallback branch:
                    /// 
                    if(!(dwStyle&ACS_AUTOPLAY) && !(pWndData->m_dwFlags&WD_PLAY))
                        SendMessage(hWnd, pWndData->m_uiRedrawMsg, 0, NULL);
                }
                
                EndPaint(hWnd, &ps);
				return 1;
            }
            break;
        case WM_NCDESTROY:
        case WM_DESTROY:
            
            if(pWndData->m_pParam)
            {
                PAVI_CTRL_DATA pacd = (PAVI_CTRL_DATA)pWndData->m_pParam;
                
                if(pacd)
                {
                    RedrawStop(pacd);
                    DestroyResources(pacd);

                    if (pacd->m_hDrawDib)
                    {
                        VERIFY(DrawDibClose(pacd->m_hDrawDib));
                        pacd->m_hDrawDib = NULL;
                    }

                    VERIFY(!LocalFree(pacd));
                    pWndData->m_pParam = NULL;
                }
            }

            if(pWndData->m_dwFlags&WD_AVI_INIT_DONE)
            {
                AVIFileExit();
                pWndData->m_dwFlags &= ~WD_AVI_INIT_DONE;
            }
            
            VERIFY(UnsubclassControl(hWnd, AnimBoxProc, pWndData));
            break;
    }
    
    return CallWindowProc(pOldProc, hWnd, uMsg, wParam, lParam);
}


BOOL AeroSubClassAnimation(HWND hwnd)
{
    return AeroSubClassControl(hwnd, AnimBoxProc, 0L);
}
