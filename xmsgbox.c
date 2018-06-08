/* XMsgBox code by TMouse (TMouse at NOSPAM bigfoot dot com)
 * Based loosely on Visual Basic code found at VBNet:
 * http://www.mvps.org/vbnet/index.html?code/hooks/messageboxhooktimerapi.htm
 * Feel free to distribute. */

#include <windows.h>
#include "xmsgbox.h"

/* The id of the static control in a message box dialog that holds the message text. */
#define IDPROMPT  0xFFFF

/* We use Windows memory allocators to avoid the C library. */
#define malloc(size) HeapAlloc(GetProcessHeap(), 0, size)
#define free(lpvoid) HeapFree(GetProcessHeap(), 0, lpvoid)

/* Thread data structure. We need to pass parameters between the hook procedure,
 * the Timer procedure and the XMsgBox function. We do this in this structure.
 * For thread safety, the structure is allocated for each call and its address
 * passed around using thread local storage */
typedef struct tagTHREAD_SPECIFIC_DATA {
	HHOOK    hHook;            // The handle of the hook we set.
	UINT_PTR dwTimerId;        // The id of the timer we set.
	HWND     hwndMsgBox;       // The hwnd of the message box.
	DWORD    dwTimerCountDown; // Number of intervals that the msgbox has been visible.
	X_MSG_BOX_PARAMS * xmbParams;  // A pointer to the caller provided XMsgBox parameters.
} THREAD_SPECIFIC_DATA, * LPTHREAD_SPECIFIC_DATA;

/* File level variables. These save the allocated TLS index and
 * whether a TLS index has been allocated. */
static DWORD m_dwTlsIndex;
static BOOL  m_bTlsInitialized;


// ========================================================================
static BOOL SetThreadSpecificData(LPTHREAD_SPECIFIC_DATA lpThreadData) {

	/* This function takes a pointer to thread data and stores
	 * it in thread local storage. A TLS index is allocated if required. */

	if ( !m_bTlsInitialized ) {
		m_dwTlsIndex = TlsAlloc();
		if (m_dwTlsIndex == TLS_OUT_OF_INDEXES) return FALSE;
		m_bTlsInitialized = TRUE;
	}

	return TlsSetValue(m_dwTlsIndex, lpThreadData);
}


// ========================================================================
static LPTHREAD_SPECIFIC_DATA GetThreadSpecificData() {

	// This function returns a pointer to thread data from TLS.

	if ( !m_bTlsInitialized ) return NULL;

	return TlsGetValue(m_dwTlsIndex);
}


// ========================================================================
static LRESULT CALLBACK MsgBoxHookProc(LONG uMsg, WPARAM wParam,LPARAM lParam) {

	LPTHREAD_SPECIFIC_DATA lpThreadData;

	lpThreadData = GetThreadSpecificData();
	if (lpThreadData == NULL) return FALSE;

	if (uMsg == HCBT_ACTIVATE) {
		LRESULT lr;

		/* When the message box is about to be shown
		 * change the button captions. */

		/* In a HCBT_ACTIVATE message, wParam holds
		 * the handle to the messagebox - save that in a thread specific
		 * variable so we can use it in the timer procedure... */
		lpThreadData->hwndMsgBox = (HWND) wParam;

		/* The ID's of the buttons on the message box
		 * correspond exactly to the values they return,
		 * so the same values can be used to identify
		 * specific buttons in a SetDlgItemText call. */
		if (lpThreadData->xmbParams->dwCountButtons == 1) {
			SetDlgItemText(lpThreadData->hwndMsgBox, IDOK, lpThreadData->xmbParams->szButtonCaptions[0]);
		}
		else if (lpThreadData->xmbParams->dwCountButtons == 2) {
			SetDlgItemText(lpThreadData->hwndMsgBox, IDYES, lpThreadData->xmbParams->szButtonCaptions[0]);
			SetDlgItemText(lpThreadData->hwndMsgBox, IDNO,  lpThreadData->xmbParams->szButtonCaptions[1]);
		}
		else if (lpThreadData->xmbParams->dwCountButtons != 0) { // Three buttons
			SetDlgItemText(lpThreadData->hwndMsgBox, IDABORT,  lpThreadData->xmbParams->szButtonCaptions[0]);
			SetDlgItemText(lpThreadData->hwndMsgBox, IDRETRY,  lpThreadData->xmbParams->szButtonCaptions[1]);
			SetDlgItemText(lpThreadData->hwndMsgBox, IDIGNORE, lpThreadData->xmbParams->szButtonCaptions[2]);
		}

		lr = CallNextHookEx(lpThreadData->hHook, uMsg, wParam, lParam);

		// We're done with the dialog, so release the hook...
		UnhookWindowsHookEx(lpThreadData->hHook);
		lpThreadData->hHook = NULL;

		return lr;
	}

	// Let normal hook processing continue...
	return CallNextHookEx(lpThreadData->hHook, uMsg, wParam, lParam);
}


// ========================================================================
static LPTSTR ReplaceTimer(LPCTSTR szString, DWORD dwTime) {

	/* This function returns a copy of szString
	 * with all instances of "%T" replaced with dwTime.
	 * The result must be free()ed. */

	DWORD dwNumInstances   = 0;
	LPTSTR szTemp          = (LPTSTR) szString;
	LPTSTR szReturn        = NULL;
	TCHAR szTime[32];

	if (szString == NULL) return NULL;

	// Convert the time to a string...
	wsprintf(szTime, TEXT("%d"), dwTime);

	// Count instances of "%T" in the input string...
	while (*szTemp) {
		if (*szTemp == TEXT('%') && *(szTemp + 1) == TEXT('T')) dwNumInstances++;
		szTemp++;
	}

	// Allocate memory for the return string...
	szReturn = malloc( (lstrlen(szString) + (dwNumInstances * (lstrlen(szTime) - 2)) + 1) * sizeof(TCHAR));
	if (szReturn == NULL) {
		SetLastError(ERROR_OUTOFMEMORY);
		return NULL;
	}

	*szReturn = TEXT('\0');

	szTemp = szReturn;

	// Copy input string into return string while replacing "%t"...
	while(*szString) {

		if (*szString == TEXT('%') && *(szString + 1) == TEXT('T')) {

			// Copy in time string...
			lstrcat(szTemp, szTime);
			// Advance szTemp to null terminator...
			szTemp += lstrlen(szTime);
			// Skip "%T" in input string...
			szString += 2;
		}
		else {
			// Copy character...
			*szTemp = *szString;
			// Add null terminator...
			*(szTemp + 1) = TEXT('\0');
			// Advance szTemp to null terminator...
			szTemp++;
			// Move to next character in input string...
			szString++;
		}
	}

	return szReturn;
}


// ========================================================================
static VOID CALLBACK TimerProc(HWND hwnd, LONG uMsg,LONG idEvent,DWORD dwTime) {

	/* This procedure is called while the message box is displayed.
	 * We use it to update the message box text and dismiss the message box
	 * if the timeout has expired. */

	LPTHREAD_SPECIFIC_DATA lpThreadData;

	lpThreadData = GetThreadSpecificData();
	if (lpThreadData == NULL) return;

	/* Assure that there is messagebox to update.
	 * lpThreadData->hwndMsgBox is saved in the hook procedure. */
	if (lpThreadData->hwndMsgBox != NULL) {

		LPTSTR szUpdatedPrompt;

		// Increment the interval counter. This contains how many
		// intervals the message box has been visible.
		lpThreadData->dwTimerCountDown++;

		// Update the prompt message with the countdown value...
		szUpdatedPrompt = ReplaceTimer(lpThreadData->xmbParams->szPrompt,
					       lpThreadData->xmbParams->dwTimerDuration - lpThreadData->dwTimerCountDown);

		if (szUpdatedPrompt != NULL) {
			// Send the new text to the message box...
			SetDlgItemText(lpThreadData->hwndMsgBox, IDPROMPT, szUpdatedPrompt);

			// Free the string returned by ReplaceTimer...
			free(szUpdatedPrompt);
		}

		// If the timer has 'expired' (the count==duration), we need to
		// get the message box to exit...
		if (lpThreadData->dwTimerCountDown == lpThreadData->xmbParams->dwTimerDuration) {

			// Nothing more to do, so we can kill this timer...
			KillTimer(NULL, lpThreadData->dwTimerId);
			lpThreadData->dwTimerId = 0;

			// This will make MessageBox return immediately...
			PostQuitMessage(0);
		}
	} 
}


// ========================================================================
INT XMsgBox(X_MSG_BOX_PARAMS * xmbParams) {

	LPTHREAD_SPECIFIC_DATA lpThreadData;
	DWORD   dwStyle;
	LPTSTR  szUpdatedPrompt;
	BOOL    bCustomButtons = TRUE;
	int     iReturnValue;
	MSG     msg;

	lpThreadData = malloc(sizeof(THREAD_SPECIFIC_DATA));
	if (lpThreadData == NULL) {
		SetLastError(ERROR_OUTOFMEMORY);
		return XMB_ERROR;
	}

	/* We store a pointer to the data in TLS so it can be used by
	 * the hook procedure and the timer procedure... */
	if ( !SetThreadSpecificData(lpThreadData) ) {
		free(lpThreadData);
		return XMB_ERROR;
	}

	ZeroMemory(lpThreadData, sizeof(THREAD_SPECIFIC_DATA));

	// Store the a pointer to the caller provided parameters in the thread data...
	lpThreadData->xmbParams = xmbParams;

	/* We set up a windows hook.
	 * This hook function will be called when the message box
	 * is activated. This allows us to save the message box hwnd
	 * and provide new button captions. */
	lpThreadData->hHook = SetWindowsHookEx(WH_CBT, (HOOKPROC) MsgBoxHookProc, NULL, GetCurrentThreadId());
	if (lpThreadData->hHook == NULL) {
		free(lpThreadData);
		return XMB_ERROR;
	}

	/* If dwTimerDuration != 0, enable the timer. Because the
	 * MessageBox creates a modal dialog, control
	 * won't return to the next line until the dialog
	 * is closed. This necessitates our starting the
	 * timer before making the call.
	 *
	 * However, timer events will execute once the
	 * modal dialog is shown, allowing us to use the
	 * timer to dynamically modify the on-screen message! 
	 *
	 * This happens because the message box pumps an internal
	 * message loop which allows timer messages to be dispatched
	 * to our timer procedure. */

	if (lpThreadData->xmbParams->dwTimerDuration != 0) {

		// Replace '%T' substring in the
		// original prompt with starting duration...
		szUpdatedPrompt = ReplaceTimer(lpThreadData->xmbParams->szPrompt, lpThreadData->xmbParams->dwTimerDuration);

		// If replace succeeded, enable the timer...
		if (szUpdatedPrompt != NULL || lpThreadData->xmbParams->szPrompt == NULL) {
			lpThreadData->dwTimerId = SetTimer(NULL, 0,
                              (UINT) lpThreadData->xmbParams->dwTimerInterval,
                              (TIMERPROC) TimerProc);
		}

		if (lpThreadData->dwTimerId == 0 ||
                   (szUpdatedPrompt == NULL && lpThreadData->xmbParams->szPrompt != NULL) ) {

			UnhookWindowsHookEx(lpThreadData->hHook);
			if (szUpdatedPrompt) free(szUpdatedPrompt);
			free(lpThreadData);
			return XMB_ERROR;
		}
	}
	else szUpdatedPrompt = lpThreadData->xmbParams->szPrompt;

	/* We add to the user specified style a style that
	 * will provide the correct number of buttons... */
	switch (lpThreadData->xmbParams->dwCountButtons) {
		case 0:
			// Do not use custom buttons.
			dwStyle = lpThreadData->xmbParams->dwStyle;
			bCustomButtons = FALSE;
			break;
		case 1:
			dwStyle = lpThreadData->xmbParams->dwStyle | MB_OK;
			break;
		case 2:
			dwStyle = lpThreadData->xmbParams->dwStyle | MB_YESNO;
			break;
		default:        // Assume three custom buttons.
			dwStyle = lpThreadData->xmbParams->dwStyle | MB_ABORTRETRYIGNORE;
	}

	// Call the MessageBox API...
	iReturnValue = MessageBox(lpThreadData->xmbParams->hwnd,
                                szUpdatedPrompt,
                                lpThreadData->xmbParams->szTitle,
                                dwStyle);

	// So that any lagging WM_TIMER calls can not try to access
	// lpThreadData params after they are freed...
	SetThreadSpecificData(NULL);

	// In case the timer event didn't suspend the timer, do it now...
	if (lpThreadData->dwTimerId != 0) KillTimer(NULL, lpThreadData->dwTimerId);

	// Free string returned by ReplaceTimer...
	if (lpThreadData->xmbParams->dwTimerDuration != 0) free(szUpdatedPrompt);

	// In case the hook procedure didn't unhook itself...
	if (lpThreadData->hHook != NULL) UnhookWindowsHookEx(lpThreadData->hHook);

	free(lpThreadData);

       /*
        *  See if there is a WM_QUIT message in the queue. If so,
        *  then you timed out. Eat the message so you don't quit the
        *  entire application.
        */ 
       if (PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE)) {

           /*
            *  If you timed out, then return minus 1.
            */ 
           return -1;
       }

	// If no custom buttons, return normal return value from MessageBox...
	if (!bCustomButtons) return iReturnValue;

	// Else Return the position of the button pressed...
	switch (iReturnValue) {
		case IDOK:     return 1;
		case IDYES:    return 1;
		case IDNO:     return 2;
		case IDABORT:  return 1;
		case IDRETRY:  return 2;
		case IDIGNORE: return 3;
	}

	// We shouldn't get here unless an error has occurred...
	return XMB_ERROR;
}


// =================================================================
static DWORD WINAPI XMsgBoxThreadProc(LPVOID lpParameter) {
	// New Thread procedure. Call XMsgBox...
	return XMsgBox(lpParameter);
}


// =================================================================
BOOL XMsgBoxAsync(X_MSG_BOX_PARAMS * xmbParams, LPHANDLE lphThread, LPDWORD lpdwThreadId) {

	/* This function provides a version of XMsgBox that returns immediately. It does
	 * this by executing XMsgBox() in a new thread. xmbParams and its contents must
	 * be valid for the lifetime of the message box. */

	DWORD dwThreadId;
	HANDLE hThread;

	hThread = CreateThread(NULL, 0, XMsgBoxThreadProc, xmbParams, 0, &dwThreadId);

	// Return thread handle to caller if requested...
	if (lphThread != NULL) *lphThread = hThread;
	else CloseHandle(hThread);

	// Return thread id to caller if requested...
	if (lpdwThreadId != NULL) *lpdwThreadId = dwThreadId;

	return (hThread != NULL ? TRUE : FALSE);
}


