#pragma once

#include "TaskDialog.h"
#include <atldlgs.h>

class CPrivilegeNewDlg : public CTaskDialogImpl<CPrivilegeNewDlg>
{
public:
    CPrivilegeNewDlg();

    void OnDialogConstructed();
    bool OnButtonClicked(int buttonId);
	int DoModal(HWND hWnd);

private:
	enum
	{
		Button_Elevate = 101,
		Button_Cancel
	};

};
