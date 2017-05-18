/* Win7ShellApi.h -- Window 7 Shell API */

#ifndef __Win7ShellApi_h__
#define __Win7ShellApi_h__

#include <ole2.h>

#ifndef MSGFLT_ADD
#define MSGFLT_ADD 1
#endif


#ifndef __ITaskbarList3_INTERFACE_DEFINED__
#define __ITaskbarList3_INTERFACE_DEFINED__

EXTERN_C const CLSID CLSID_TaskbarList;

EXTERN_C const IID IID_ITaskbarList3Vtbl;

typedef enum _THUMBBUTTONFLAGS {
	THBF_ENABLED	= 0,
	THBF_DISABLED	= 0x1,
	THBF_DISMISSONCLICK	= 0x2,
	THBF_NOBACKGROUND	= 0x4,
	THBF_HIDDEN	= 0x8,
	THBF_NONINTERACTIVE	= 0x10
} THUMBBUTTONFLAGS;

typedef enum _THUMBBUTTONMASK {
    THB_BITMAP	= 0x1,
	THB_ICON	= 0x2,
	THB_TOOLTIP	= 0x4,
	THB_FLAGS	= 0x8
} THUMBBUTTONMASK;

#include <pshpack8.h>
typedef struct _THUMBBUTTON
{
    THUMBBUTTONMASK dwMask;
    UINT iId;
    UINT iBitmap;
    HICON hIcon;
    WCHAR szTip[ 260 ];
    THUMBBUTTONFLAGS dwFlags;
} THUMBBUTTON, *LPTHUMBBUTTON;

#include <poppack.h>
#define THBN_CLICKED        0x1800

typedef enum _TBPFLAG {
	TBPF_NOPROGRESS	= 0,
	TBPF_INDETERMINATE	= 0x1,
	TBPF_NORMAL	= 0x2,
	TBPF_ERROR	= 0x4,
	TBPF_PAUSED	= 0x8
} TBPFLAG;

//typedef _COM_interface ITaskbarList3Vtbl ITaskbarList3Vtbl;

#define INTERFACE ITaskbarList3Vtbl
DECLARE_INTERFACE_(ITaskbarList3Vtbl,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,LPVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(HrInit)(THIS) PURE;
	STDMETHOD(AddTab)(THIS_ HWND) PURE;
	STDMETHOD(DeleteTab)(THIS_ HWND) PURE;
	STDMETHOD(ActivateTab)(THIS_ HWND) PURE;
	STDMETHOD(SetActiveAlt)(THIS_ HWND) PURE;
	STDMETHOD(MarkFullscreenWindow)(THIS_ HWND,BOOL) PURE;
	STDMETHOD(SetProgressValue)(THIS_ HWND,ULONGLONG,ULONGLONG) PURE;
	STDMETHOD(SetProgressState)(THIS_ HWND,DWORD) PURE;
	STDMETHOD(RegisterTab)(THIS_ HWND,HWND) PURE;
	STDMETHOD(UnregisterTab)(THIS_ HWND) PURE;
	STDMETHOD(SetTabOrder)(THIS_ HWND,HWND) PURE;
	STDMETHOD(SetTabActive)(THIS_ HWND,HWND,DWORD) PURE;
	STDMETHOD(ThumbBarAddButtons)(THIS_ HWND,UINT,LPTHUMBBUTTON) PURE;
	STDMETHOD(ThumbBarUpdateButtons)(THIS_ HWND,UINT,LPTHUMBBUTTON) PURE;
	STDMETHOD(ThumbBarSetImageList)(THIS_ HWND,HIMAGELIST) PURE;
	STDMETHOD(SetOverlayIcon)(THIS_ HWND,HICON,LPCWSTR) PURE;
	STDMETHOD(SetThumbnailTooltip)(THIS_ HWND,LPCWSTR) PURE;
	STDMETHOD(SetThumbnailClip)(THIS_ HWND,HICON,RECT) PURE;
};
#undef INTERFACE
typedef struct ITaskbarList3Vtbl ITaskbarList3;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define ITaskbarList3_QueryInterface(p,riid,ppvObject) (p)->lpVtbl->QueryInterface(p,riid,ppvObject)
#define ITaskbarList3_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ITaskbarList3_Release(p) (p)->lpVtbl->Release(p)
#define ITaskbarList3_HrInit(p) (p)->lpVtbl->HrInit(p)
#define ITaskbarList3_AddTab(p,hwnd) (p)->lpVtbl->AddTab(p,hwnd)
#define ITaskbarList3_DeleteTab(p,hwnd) (p)->lpVtbl->DeleteTab(p,hwnd)
#define ITaskbarList3_ActivateTab(p,hwnd) (p)->lpVtbl->ActivateTab(p,hwnd)
#define ITaskbarList3_SetActiveAlt(p,hwnd) (p)->lpVtbl->SetActiveAlt(p,hwnd)
#define ITaskbarList3_MarkFullscreenWindow(p,hwnd,fFullscreen) (p)->lpVtbl->MarkFullscreenWindow(p,hwnd,fFullscreen)
#define ITaskbarList3_SetProgressValue(p,hwnd,ullCompleted,ullTotal) (p)->lpVtbl->SetProgressValue(p,hwnd,ullCompleted,ullTotal)
#define ITaskbarList3_SetProgressState(p,hwnd,tbpFlags) (p)->lpVtbl->SetProgressState(p,hwnd,tbpFlags)
#define ITaskbarList3_RegisterTab(p,hwndTab,hwndMDI) (p)->lpVtbl->RegisterTab(p,hwndTab,hwndMDI)
#define ITaskbarList3_UnregisterTab(p,hwndTab) (p)->lpVtbl->UnregisterTab(p,hwndTab)
#define ITaskbarList3_SetTabOrder(p,hwndTab,hwndInsertBefore) (p)->lpVtbl->SetTabOrder(p,hwndTab,hwndInsertBefore)
#define ITaskbarList3_SetTabActive(p,hwndTab,hwndMDI,dwReserved) (p)->lpVtbl->SetTabActive(p,hwndTab,hwndMDI,dwReserved)
#define ITaskbarList3_ThumbBarAddButtons(p,hwnd,cButtons,pButton) (p)->lpVtbl->ThumbBarAddButtons(p,hwnd,cButtons,pButton)
#define ITaskbarList3_ThumbBarUpdateButtons(p,hwnd,cButtons,pButton) (p)->lpVtbl->ThumbBarUpdateButtons(p,hwnd,cButtons,pButton)
#define ITaskbarList3_ThumbBarSetImageList(p,hwnd,himl) (p)->lpVtbl->ThumbBarSetImageList(p,hwnd,himl)
#define ITaskbarList3_SetOverlayIcon(p,hwnd,hIcon,pszDescription) (p)->lpVtbl->SetOverlayIcon(p,hwnd,hIcon,pszDescription)
#define ITaskbarList3_SetThumbnailTooltip(p,hwnd,pszTip) (p)->lpVtbl->SetThumbnailTooltip(p,hwnd,pszTip)
#define ITaskbarList3_SetThumbnailClip(p,hwnd,prcClip) (p)->lpVtbl->SetThumbnailClip(p,hwnd,prcClip)
#else  /* !defined(__cplusplus) || defined(CINTERFACE) */
#define ITaskbarList3_QueryInterface(p,riid,ppvObject) (p)->QueryInterface(p,riid,ppvObject)
#define ITaskbarList3_AddRef(p) (p)->AddRef(p)
#define ITaskbarList3_Release(p) (p)->Release(p)
#define ITaskbarList3_HrInit(p) (p)->HrInit(p)
#define ITaskbarList3_AddTab(p,hwnd) (p)->AddTab(p,hwnd)
#define ITaskbarList3_DeleteTab(p,hwnd) (p)->DeleteTab(p,hwnd)
#define ITaskbarList3_ActivateTab(p,hwnd) (p)->ActivateTab(p,hwnd)
#define ITaskbarList3_SetActiveAlt(p,hwnd) (p)->SetActiveAlt(p,hwnd)
#define ITaskbarList3_MarkFullscreenWindow(p,hwnd,fFullscreen) (p)->MarkFullscreenWindow(p,hwnd,fFullscreen)
#define ITaskbarList3_SetProgressValue(p,hwnd,ullCompleted,ullTotal) (p)->SetProgressValue(p,hwnd,ullCompleted,ullTotal)
#define ITaskbarList3_SetProgressState(p,hwnd,tbpFlags) (p)->SetProgressState(p,hwnd,tbpFlags)
#define ITaskbarList3_RegisterTab(p,hwndTab,hwndMDI) (p)->RegisterTab(p,hwndTab,hwndMDI)
#define ITaskbarList3_UnregisterTab(p,hwndTab) (p)->UnregisterTab(p,hwndTab)
#define ITaskbarList3_SetTabOrder(p,hwndTab,hwndInsertBefore) (p)->SetTabOrder(p,hwndTab,hwndInsertBefore)
#define ITaskbarList3_SetTabActive(p,hwndTab,hwndMDI,dwReserved) (p)->SetTabActive(p,hwndTab,hwndMDI,dwReserved)
#define ITaskbarList3_ThumbBarAddButtons(p,hwnd,cButtons,pButton) (p)->ThumbBarAddButtons(p,hwnd,cButtons,pButton)
#define ITaskbarList3_ThumbBarUpdateButtons(p,hwnd,cButtons,pButton) (p)->ThumbBarUpdateButtons(p,hwnd,cButtons,pButton)
#define ITaskbarList3_ThumbBarSetImageList(p,hwnd,himl) (p)->ThumbBarSetImageList(p,hwnd,himl)
#define ITaskbarList3_SetOverlayIcon(p,hwnd,hIcon,pszDescription) (p)->SetOverlayIcon(p,hwnd,hIcon,pszDescription)
#define ITaskbarList3_SetThumbnailTooltip(p,hwnd,pszTip) (p)->SetThumbnailTooltip(p,hwnd,pszTip)
#define ITaskbarList3_SetThumbnailClip(p,hwnd,prcClip) (p)->SetThumbnailClip(p,hwnd,prcClip)
#endif  /* !defined(__cplusplus) || defined(CINTERFACE) */

#endif 	/* __ITaskbarList3_INTERFACE_DEFINED__ */


#ifndef __ICustomDestinationList_INTERFACE_DEFINED__
#define __ICustomDestinationList_INTERFACE_DEFINED__

/* interface ICustomDestinationList */
/* [unique][object][uuid] */ 

typedef /* [v1_enum] */ 
	enum KNOWNDESTCATEGORY
{	KDC_FREQUENT	= 1,
KDC_RECENT	= ( KDC_FREQUENT + 1 ) 
} 	KNOWNDESTCATEGORY;


EXTERN_C const IID IID_ICustomDestinationList;

#if defined(__cplusplus) && !defined(CINTERFACE)

MIDL_INTERFACE("6332debf-87b5-4670-90c0-5e57b408a49e")
ICustomDestinationList : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetAppID( 
		/* [string][in] */ __RPC__in_string LPCWSTR pszAppID) = 0;

	virtual HRESULT STDMETHODCALLTYPE BeginList( 
		/* [out] */ __RPC__out UINT *pcMinSlots,
		/* [in] */ __RPC__in REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out_opt void **ppv) = 0;

	virtual HRESULT STDMETHODCALLTYPE AppendCategory( 
		/* [string][in] */ __RPC__in_string LPCWSTR pszCategory,
		/* [in] */ __RPC__in_opt IObjectArray *poa) = 0;

	virtual HRESULT STDMETHODCALLTYPE AppendKnownCategory( 
		/* [in] */ KNOWNDESTCATEGORY category) = 0;

	virtual HRESULT STDMETHODCALLTYPE AddUserTasks( 
		/* [in] */ __RPC__in_opt IObjectArray *poa) = 0;

	virtual HRESULT STDMETHODCALLTYPE CommitList( void) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetRemovedDestinations( 
		/* [in] */ __RPC__in REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out_opt void **ppv) = 0;

	virtual HRESULT STDMETHODCALLTYPE DeleteList( 
		/* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszAppID) = 0;

	virtual HRESULT STDMETHODCALLTYPE AbortList( void) = 0;

};

#else 	/* C style interface */

typedef struct ICustomDestinationListVtbl
{
	BEGIN_INTERFACE

		HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
		__RPC__in ICustomDestinationList * This,
		/* [in] */ __RPC__in REFIID riid,
		/* [annotation][iid_is][out] */ 
		__RPC__deref_out  void **ppvObject);

		ULONG ( STDMETHODCALLTYPE *AddRef )( 
			__RPC__in ICustomDestinationList * This);

		ULONG ( STDMETHODCALLTYPE *Release )( 
			__RPC__in ICustomDestinationList * This);

		HRESULT ( STDMETHODCALLTYPE *SetAppID )( 
			__RPC__in ICustomDestinationList * This,
			/* [string][in] */ __RPC__in_string LPCWSTR pszAppID);

		HRESULT ( STDMETHODCALLTYPE *BeginList )( 
			__RPC__in ICustomDestinationList * This,
			/* [out] */ __RPC__out UINT *pcMinSlots,
			/* [in] */ __RPC__in REFIID riid,
			/* [iid_is][out] */ __RPC__deref_out_opt void **ppv);

		HRESULT ( STDMETHODCALLTYPE *AppendCategory )( 
			__RPC__in ICustomDestinationList * This,
			/* [string][in] */ __RPC__in_string LPCWSTR pszCategory,
			/* [in] */ __RPC__in_opt IObjectArray *poa);

		HRESULT ( STDMETHODCALLTYPE *AppendKnownCategory )( 
			__RPC__in ICustomDestinationList * This,
			/* [in] */ KNOWNDESTCATEGORY category);

		HRESULT ( STDMETHODCALLTYPE *AddUserTasks )( 
			__RPC__in ICustomDestinationList * This,
			/* [in] */ __RPC__in_opt IObjectArray *poa);

		HRESULT ( STDMETHODCALLTYPE *CommitList )( 
			__RPC__in ICustomDestinationList * This);

		HRESULT ( STDMETHODCALLTYPE *GetRemovedDestinations )( 
			__RPC__in ICustomDestinationList * This,
			/* [in] */ __RPC__in REFIID riid,
			/* [iid_is][out] */ __RPC__deref_out_opt void **ppv);

		HRESULT ( STDMETHODCALLTYPE *DeleteList )( 
			__RPC__in ICustomDestinationList * This,
			/* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszAppID);

		HRESULT ( STDMETHODCALLTYPE *AbortList )( 
			__RPC__in ICustomDestinationList * This);

	END_INTERFACE
} ICustomDestinationListVtbl;

interface ICustomDestinationList
{
	CONST_VTBL struct ICustomDestinationListVtbl *lpVtbl;
};



#ifdef COBJMACROS


#define ICustomDestinationList_QueryInterface(This,riid,ppvObject)	\
	( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICustomDestinationList_AddRef(This)	\
	( (This)->lpVtbl -> AddRef(This) ) 

#define ICustomDestinationList_Release(This)	\
	( (This)->lpVtbl -> Release(This) ) 


#define ICustomDestinationList_SetAppID(This,pszAppID)	\
	( (This)->lpVtbl -> SetAppID(This,pszAppID) ) 

#define ICustomDestinationList_BeginList(This,pcMinSlots,riid,ppv)	\
	( (This)->lpVtbl -> BeginList(This,pcMinSlots,riid,ppv) ) 

#define ICustomDestinationList_AppendCategory(This,pszCategory,poa)	\
	( (This)->lpVtbl -> AppendCategory(This,pszCategory,poa) ) 

#define ICustomDestinationList_AppendKnownCategory(This,category)	\
	( (This)->lpVtbl -> AppendKnownCategory(This,category) ) 

#define ICustomDestinationList_AddUserTasks(This,poa)	\
	( (This)->lpVtbl -> AddUserTasks(This,poa) ) 

#define ICustomDestinationList_CommitList(This)	\
	( (This)->lpVtbl -> CommitList(This) ) 

#define ICustomDestinationList_GetRemovedDestinations(This,riid,ppv)	\
	( (This)->lpVtbl -> GetRemovedDestinations(This,riid,ppv) ) 

#define ICustomDestinationList_DeleteList(This,pszAppID)	\
	( (This)->lpVtbl -> DeleteList(This,pszAppID) ) 

#define ICustomDestinationList_AbortList(This)	\
	( (This)->lpVtbl -> AbortList(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICustomDestinationList_INTERFACE_DEFINED__ */

#endif /* __Win7ShellApi_h__ */
