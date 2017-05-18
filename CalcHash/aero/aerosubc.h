#ifndef AEROSUBC_H__
#define AEROSUBC_H__

/*
*
* $RCSfile: aerosubc.h,v $
* $Source: /cvs/common/aerosubc.h,v $
* $Author: cvs $
* $Revision: 1.11 $
* $Date: 2007/05/18 11:24:18 $
* $State: Exp $
* Copyright (c) Stefan Kuhr
*
*
*
* $Log: aerosubc.h,v $
* Revision 1.11  2007/05/18 11:24:18  cvs
* Added progress control subclassing
*
* Revision 1.10  2007/05/11 20:52:13  cvs
* Added AeroSubclassCtrl for easier per-control subclassing
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
* Revision 1.6  2007/04/10 09:17:29  cvs
* Added date time pick control
*
* Revision 1.5  2007/04/09 18:17:47  cvs
* Added IpAddress control code
*
* Revision 1.4  2007/04/09 16:15:57  cvs
* Added spin control code
*
* Revision 1.3  2007/04/09 12:02:58  cvs
* Added new file for slider controls
*
* Revision 1.2  2007/04/08 13:37:51  cvs
* Added standard header
* 
*/


#include <windows.h>

#define ASC_NO_FRAME_EXTENSION 0x0001
#define ASC_SUBCLASS_STATIC    0x0002
#define ASC_SUBCLASS_BUTTON    0x0004
#define ASC_SUBCLASS_LISTBOX   0x0008
#define ASC_SUBCLASS_LISTCTRL  0x0010
#define ASC_SUBCLASS_HDRCTRL   0x0020
#define ASC_SUBCLASS_EDIT      0x0040
#define ASC_SUBCLASS_COMBOBOX  0x0080
#define ASC_SUBCLASS_ANIMATION 0x0100
#define ASC_SUBCLASS_SLIDER    0x0200
#define ASC_SUBCLASS_SPINCTRL  0x0400
#define ASC_SUBCLASS_IPADRCTRL 0x0800
#define ASC_SUBCLASS_DATETIMEP 0x1000
#define ASC_SUBCLASS_TREECTRL  0x2000
#define ASC_SUBCLASS_PRGSCTRL  0x4000

#define ASC_SUBCLASS_ALL_CONTROLS (ASC_SUBCLASS_STATIC|ASC_SUBCLASS_BUTTON|ASC_SUBCLASS_LISTBOX|ASC_SUBCLASS_LISTCTRL|\
ASC_SUBCLASS_HDRCTRL|ASC_SUBCLASS_EDIT|ASC_SUBCLASS_COMBOBOX|ASC_SUBCLASS_ANIMATION|ASC_SUBCLASS_SLIDER|\
ASC_SUBCLASS_SPINCTRL|ASC_SUBCLASS_IPADRCTRL|ASC_SUBCLASS_DATETIMEP|ASC_SUBCLASS_TREECTRL|ASC_SUBCLASS_PRGSCTRL)


#ifdef __cplusplus
extern "C" {
#endif /// __cplusplus

BOOL AeroAutoSubclass(HWND hWnd, DWORD dwFlags, DWORD dwReserved);
BOOL AeroSubClassStatic(HWND hwnd);
BOOL AeroSubClassButton(HWND hwnd);
BOOL AeroSubClassListBox(HWND hwnd);
BOOL AeroSubClassListCtrl(HWND hwnd);
BOOL AeroSubClassHeaderCtrl(HWND hwnd);
BOOL AeroSubClassEdit(HWND hwnd);
BOOL AeroSubClassComboBox(HWND hwnd);
BOOL AeroSubClassComboBoxEx(HWND hwnd);
BOOL AeroSubClassAnimation(HWND hwnd);
BOOL AeroSubClassSlider(HWND hwnd);
BOOL AeroSubClassSpinCtrl(HWND hwnd);
BOOL AeroSubClassIPAddressCtrl(HWND hwnd);
BOOL AeroSubClassDateTimePick(HWND hwnd);
BOOL AeroSubClassTreeCtrl(HWND hwnd);
BOOL AeroSubClassCtrl(HWND hwnd);
BOOL AeroSubClassProgressCtrl(HWND hwnd);


#ifdef __cplusplus
}
#endif /// __cplusplus


#endif // AEROSUBC_H__


