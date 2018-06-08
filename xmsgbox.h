/* XMsgBox code by TMouse (TMouse at NOSPAM bigfoot dot com)
 * Based loosely on Visual Basic code found at VBNet:
 * http://www.mvps.org/vbnet/index.html?code/hooks/messageboxhooktimerapi.htm
 * Feel free to distribute. */

#ifndef XMSGBOX_H_INCLUDED
#define XMSGBOX_H_INCLUDED

#include <windows.h>

typedef struct tagX_MSG_BOX_PARAMS {
	HWND hwnd;          // Owner window. May be NULL
	DWORD dwStyle;      // Message box style. Use MB_ constants.

	DWORD dwTimerDuration;  // How many intervals until the message box times out?
	DWORD dwTimerInterval;  // The length of each interval in milliseconds.

	LPTSTR szTitle;     // The title of the message box. May be NULL.
	LPTSTR szPrompt;    // The text to use in the message box.

	DWORD dwCountButtons;        // The number of custom buttons[0 - 3].
	LPTSTR szButtonCaptions[3];  // The captions for the custom buttons.
} X_MSG_BOX_PARAMS;

/* -------------------------------------------------------------------------
    X_MSG_BOX_PARAMS Notes:
--> Set dwCountButtons to 0 if you do not want to use custom buttons. In this
case the normal buttons based on dwStyle will be used.

--> If using custom buttons, then make sure that no button styles such as MB_YESNO are
included in dwStyle. These will be added by XMsgBox to provide the correct number of buttons.

--> A maximum of three buttons can be specified in dwCountButtons. More than
this will default to three. 

--> Place the characters '%T' in the szPrompt string where you want the countdown
number to be inserted. This is case-sensitive.

--> Set dwTimerDuration to 0 if you do not want the message box to be timed. In this
case any %T in the szPrompt string will not be replaced. 

--> Buttons strings specified in szButtonsCaptions should have a maximum length
of about 12 characters. Longer than this may be truncated.
---------------------------------------------------------------------------- */

INT XMsgBox(X_MSG_BOX_PARAMS * xmbParams);

/* ---------------------------------------------------------------------------
This function will not return until the user takes action or the message box times out.
Return Values;
XMB_TIMEOUT (-1): The message box timed out.
XMB_ERROR   (0): An error occurred. Please use GetLastError() to find the error number.
If the message box is not using custom buttons(ie. dwCountButtons == 0) then the return value,
if not one of the above, will be the normal return value from MessageBox. This is usually one of
the button constants: IDOK, IDYES, etc.

If the message box is using custom buttons, the return value will be one of the constants:
XMB_BUTTON1, etc. (See below).
------------------------------------------------------------------------------- */

BOOL XMsgBoxAsync(X_MSG_BOX_PARAMS * xmbParams, LPHANDLE lphThread, LPDWORD lpdwThreadId);

/* ---------------------------------------------------------------------------
This function will return immediately. It should be strongly noted that xmbParams
and its contents MUST be valid for the lifetime of the message box.
If lphThread or lpdwThreadId are not null, they will return the handle or the id 
of the thread that the message box is launched in. Some things that can be done with these:

if (WaitForSingleObject(hThread, 0) == WAIT_OBJECT_0) - the message box has returned.

WaitForSingleObject(hThread, INFINITE) - Wait for the message box to be dismissed or timeout

GetExitCodeThread(hThread, &dwExitCode) - Get the return value of XMsgBox after the thread
has completed.

PostThreadMessage(dwThreadId, WM_QUIT,0,0) - Make the message box exit.
------------------------------------------------------------------------------ */

// Some simple constants to make code more readable.
#define XMB_BUTTON1 1
#define XMB_BUTTON2 2
#define XMB_BUTTON3 3
#define XMB_TIMEOUT -1
#define XMB_ERROR   0

#endif // XMSGBOX_H_INCLUDED