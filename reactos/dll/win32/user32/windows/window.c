/* $Id$
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS user32.dll
 * FILE:            lib/user32/windows/window.c
 * PURPOSE:         Window management
 * PROGRAMMER:      Casper S. Hornstrup (chorns@users.sourceforge.net)
 * UPDATE HISTORY:
 *      06-06-2001  CSH  Created
 */

/* INCLUDES ******************************************************************/
#define DEBUG
#include <user32.h>

#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(user32);

LRESULT DefWndNCPaint(HWND hWnd, HRGN hRgn, BOOL Active);
void MDI_CalcDefaultChildPos( HWND hwndClient, INT total, LPPOINT lpPos, INT delta, UINT *id );

#define CW_USEDEFAULT16 0x00008000

/* FUNCTIONS *****************************************************************/


NTSTATUS STDCALL
User32CallSendAsyncProcForKernel(PVOID Arguments, ULONG ArgumentLength)
{
  PSENDASYNCPROC_CALLBACK_ARGUMENTS CallbackArgs;

  TRACE("User32CallSendAsyncProcKernel()\n");
  CallbackArgs = (PSENDASYNCPROC_CALLBACK_ARGUMENTS)Arguments;
  if (ArgumentLength != sizeof(WINDOWPROC_CALLBACK_ARGUMENTS))
    {
      return(STATUS_INFO_LENGTH_MISMATCH);
    }
  CallbackArgs->Callback(CallbackArgs->Wnd, CallbackArgs->Msg,
			 CallbackArgs->Context, CallbackArgs->Result);
  return(STATUS_SUCCESS);
}


/*
 * @unimplemented
 */
BOOL STDCALL
AllowSetForegroundWindow(DWORD dwProcessId)
{
  UNIMPLEMENTED;
  return(FALSE);
}


/*
 * @unimplemented
 */
HDWP STDCALL
BeginDeferWindowPos(int nNumWindows)
{
#if 0
  UNIMPLEMENTED;
  return (HDWP)0;
#else
  return (HDWP)1;
#endif
}


/*
 * @implemented
 */
BOOL STDCALL
BringWindowToTop(HWND hWnd)
{
    return NtUserSetWindowPos( hWnd,
                               HWND_TOP,
                               0,
                               0,
                               0,
                               0,
                               SWP_NOSIZE | SWP_NOMOVE );
}


/*
 * @unimplemented
 */
/*
WORD STDCALL
CascadeWindows(HWND hwndParent,
	       UINT wHow,
	       CONST RECT *lpRect,
	       UINT cKids,
	       const HWND *lpKids)
{
  UNIMPLEMENTED;
  return 0;
}
*/

VOID
STDCALL
SwitchToThisWindow ( HWND hwnd, BOOL fUnknown )
{
  ShowWindow ( hwnd, SW_SHOW );
}

/*
 * @implemented
 */
HWND STDCALL
ChildWindowFromPoint(HWND hWndParent,
		     POINT Point)
{
  return (HWND) NtUserChildWindowFromPointEx(hWndParent, Point.x, Point.y, 0);
}


/*
 * @implemented
 */
HWND STDCALL
ChildWindowFromPointEx(HWND hwndParent,
		       POINT pt,
		       UINT uFlags)
{
  return (HWND) NtUserChildWindowFromPointEx(hwndParent, pt.x, pt.y, uFlags);
}


/*
 * @implemented
 */
BOOL STDCALL
CloseWindow(HWND hWnd)
{
    SendMessageA(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);

    return (BOOL)(hWnd);
}


HWND STDCALL
User32CreateWindowEx(DWORD dwExStyle,
		LPCSTR lpClassName,
		LPCSTR lpWindowName,
		DWORD dwStyle,
		int x,
		int y,
		int nWidth,
		int nHeight,
		HWND hWndParent,
		HMENU hMenu,
		HINSTANCE hInstance,
		LPVOID lpParam,
		BOOL Unicode)
{
  UNICODE_STRING WindowName;
  UNICODE_STRING ClassName;
  WNDCLASSEXA wceA;
  WNDCLASSEXW wceW;
  HWND Handle;

#if 0
  DbgPrint("[window] User32CreateWindowEx style %d, exstyle %d, parent %d\n", dwStyle, dwExStyle, hWndParent);
#endif

  if (IS_ATOM(lpClassName))
    {
      RtlInitUnicodeString(&ClassName, NULL);
      ClassName.Buffer = (LPWSTR)lpClassName;
    }
  else
    {
       if(Unicode)
           RtlInitUnicodeString(&ClassName, (PCWSTR)lpClassName);
       else
       {
          if (!RtlCreateUnicodeStringFromAsciiz(&(ClassName), (PCSZ)lpClassName))
          {
	     SetLastError(ERROR_OUTOFMEMORY);
	     return (HWND)0;
	  }
       }
    }

  if (Unicode)
    RtlInitUnicodeString(&WindowName, (PCWSTR)lpWindowName);
  else
  {
    if (!RtlCreateUnicodeStringFromAsciiz(&WindowName, (PCSZ)lpWindowName))
      {
        if (!IS_ATOM(lpClassName))
	  {
	    RtlFreeUnicodeString(&ClassName);
	  }
        SetLastError(ERROR_OUTOFMEMORY);
        return (HWND)0;
      }
  }

  if(!hMenu && (dwStyle & (WS_OVERLAPPEDWINDOW | WS_POPUP)))
  {
    if(Unicode)
    {
       wceW.cbSize = sizeof(WNDCLASSEXW);
       if(GetClassInfoExW(hInstance, (LPCWSTR)lpClassName, &wceW) && wceW.lpszMenuName)
       {
       hMenu = LoadMenuW(hInstance, wceW.lpszMenuName);
       }
    }
    else
    {
       wceA.cbSize = sizeof(WNDCLASSEXA);
       if(GetClassInfoExA(hInstance, lpClassName, &wceA) && wceA.lpszMenuName)
       {
         hMenu = LoadMenuA(hInstance, wceA.lpszMenuName);
       }
    }
  }

  Handle = NtUserCreateWindowEx(dwExStyle,
				&ClassName,
				&WindowName,
				dwStyle,
				x,
				y,
				nWidth,
				nHeight,
				hWndParent,
				hMenu,
				hInstance,
				lpParam,
				SW_SHOW,
				FALSE,
				0);

#if 0
  DbgPrint("[window] NtUserCreateWindowEx() == %d\n", Handle);
#endif

  if(!Unicode)
  {
    RtlFreeUnicodeString(&WindowName);

    if (!IS_ATOM(lpClassName))
      {
        RtlFreeUnicodeString(&ClassName);
      }
  }
  return Handle;
}


/*
 * @implemented
 */
HWND STDCALL
CreateWindowExA(DWORD dwExStyle,
		LPCSTR lpClassName,
		LPCSTR lpWindowName,
		DWORD dwStyle,
		int x,
		int y,
		int nWidth,
		int nHeight,
		HWND hWndParent,
		HMENU hMenu,
		HINSTANCE hInstance,
		LPVOID lpParam)
{
    MDICREATESTRUCTA mdi;
    HWND hwnd;

    if (dwExStyle & WS_EX_MDICHILD)
    {
        POINT mPos[2];
        UINT id = 0;
        HWND top_child;

        /* lpParams of WM_[NC]CREATE is different for MDI children.
        * MDICREATESTRUCT members have the originally passed values.
        */
        mdi.szClass = lpClassName;
        mdi.szTitle = lpWindowName;
        mdi.hOwner = hInstance;
        mdi.x = x;
        mdi.y = y;
        mdi.cx = nWidth;
        mdi.cy = nHeight;
        mdi.style = dwStyle;
        mdi.lParam = (LPARAM)lpParam;

        lpParam = (LPVOID)&mdi;

        if (GetWindowLongW(hWndParent, GWL_STYLE) & MDIS_ALLCHILDSTYLES)
        {
            if (dwStyle & WS_POPUP)
            {
                WARN("WS_POPUP with MDIS_ALLCHILDSTYLES is not allowed\n");
                return(0);
            }
            dwStyle |= (WS_CHILD | WS_CLIPSIBLINGS);
        }
        else
        {
            dwStyle &= ~WS_POPUP;
            dwStyle |= (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CAPTION |
                WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        }

        top_child = GetWindow(hWndParent, GW_CHILD);

        if (top_child)
        {
            /* Restore current maximized child */
            if((dwStyle & WS_VISIBLE) && IsZoomed(top_child))
            {
                TRACE("Restoring current maximized child %p\n", top_child);
                SendMessageW( top_child, WM_SETREDRAW, FALSE, 0 );
                ShowWindow(top_child, SW_RESTORE);
                SendMessageW( top_child, WM_SETREDRAW, TRUE, 0 );
            }
        }

        MDI_CalcDefaultChildPos(hWndParent, -1, mPos, 0, &id);

        if (!(dwStyle & WS_POPUP)) hMenu = (HMENU)id;

        if (dwStyle & (WS_CHILD | WS_POPUP))
        {
            if (x == CW_USEDEFAULT || x == CW_USEDEFAULT16)
            {
                x = mPos[0].x;
                y = mPos[0].y;
            }
            if (nWidth == CW_USEDEFAULT || nWidth == CW_USEDEFAULT16 || !nWidth)
                nWidth = mPos[1].x;
            if (nHeight == CW_USEDEFAULT || nHeight == CW_USEDEFAULT16 || !nHeight)
                nHeight = mPos[1].y;
        }
    }

    hwnd = User32CreateWindowEx(dwExStyle,
                                lpClassName,
                                lpWindowName,
                                dwStyle,
                                x,
                                y,
                                nWidth,
                                nHeight,
                                hWndParent,
                                hMenu,
                                hInstance,
                                lpParam,
                                FALSE);
    return hwnd;
}


/*
 * @implemented
 */
HWND STDCALL
CreateWindowExW(DWORD dwExStyle,
		LPCWSTR lpClassName,
		LPCWSTR lpWindowName,
		DWORD dwStyle,
		int x,
		int y,
		int nWidth,
		int nHeight,
		HWND hWndParent,
		HMENU hMenu,
		HINSTANCE hInstance,
		LPVOID lpParam)
{
    MDICREATESTRUCTW mdi;
    HWND hwnd;

    if (dwExStyle & WS_EX_MDICHILD)
    {
        POINT mPos[2];
        UINT id = 0;
        HWND top_child;

        /* lpParams of WM_[NC]CREATE is different for MDI children.
        * MDICREATESTRUCT members have the originally passed values.
        */
        mdi.szClass = lpClassName;
        mdi.szTitle = lpWindowName;
        mdi.hOwner = hInstance;
        mdi.x = x;
        mdi.y = y;
        mdi.cx = nWidth;
        mdi.cy = nHeight;
        mdi.style = dwStyle;
        mdi.lParam = (LPARAM)lpParam;

        lpParam = (LPVOID)&mdi;

        if (GetWindowLongW(hWndParent, GWL_STYLE) & MDIS_ALLCHILDSTYLES)
        {
            if (dwStyle & WS_POPUP)
            {
                WARN("WS_POPUP with MDIS_ALLCHILDSTYLES is not allowed\n");
                return(0);
            }
            dwStyle |= (WS_CHILD | WS_CLIPSIBLINGS);
        }
        else
        {
            dwStyle &= ~WS_POPUP;
            dwStyle |= (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CAPTION |
                WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        }

        top_child = GetWindow(hWndParent, GW_CHILD);

        if (top_child)
        {
            /* Restore current maximized child */
            if((dwStyle & WS_VISIBLE) && IsZoomed(top_child))
            {
                TRACE("Restoring current maximized child %p\n", top_child);
                SendMessageW( top_child, WM_SETREDRAW, FALSE, 0 );
                ShowWindow(top_child, SW_RESTORE);
                SendMessageW( top_child, WM_SETREDRAW, TRUE, 0 );
            }
        }

        MDI_CalcDefaultChildPos(hWndParent, -1, mPos, 0, &id);

        if (!(dwStyle & WS_POPUP)) hMenu = (HMENU)id;

        if (dwStyle & (WS_CHILD | WS_POPUP))
        {
            if (x == CW_USEDEFAULT || x == CW_USEDEFAULT16)
            {
                x = mPos[0].x;
                y = mPos[0].y;
            }
            if (nWidth == CW_USEDEFAULT || nWidth == CW_USEDEFAULT16 || !nWidth)
                nWidth = mPos[1].x;
            if (nHeight == CW_USEDEFAULT || nHeight == CW_USEDEFAULT16 || !nHeight)
                nHeight = mPos[1].y;
        }
    }

    hwnd = User32CreateWindowEx(dwExStyle,
                                (LPCSTR) lpClassName,
                                (LPCSTR) lpWindowName,
                                dwStyle,
                                x,
                                y,
                                nWidth,
                                nHeight,
                                hWndParent,
                                hMenu,
                                hInstance,
                                lpParam,
                                TRUE);
    return hwnd;
}

/*
 * @unimplemented
 */
HDWP STDCALL
DeferWindowPos(HDWP hWinPosInfo,
	       HWND hWnd,
	       HWND hWndInsertAfter,
	       int x,
	       int y,
	       int cx,
	       int cy,
	       UINT uFlags)
{
#if 0
  return NtUserDeferWindowPos(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
#else
  SetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
  return hWinPosInfo;
#endif
}


/*
 * @implemented
 */
BOOL STDCALL
DestroyWindow(HWND hWnd)
{
  return NtUserDestroyWindow(hWnd);
}


/*
 * @unimplemented
 */
BOOL STDCALL
EndDeferWindowPos(HDWP hWinPosInfo)
{
#if 0
  UNIMPLEMENTED;
  return FALSE;
#else
  return TRUE;
#endif
}


/*
 * @implemented
 */
HWND STDCALL
GetDesktopWindow(VOID)
{
    PWINDOW Wnd;
    HWND Ret = NULL;

    _SEH_TRY
    {
        Wnd = GetThreadDesktopWnd();
        if (Wnd != NULL)
            Ret = UserHMGetHandle(Wnd);
    }
    _SEH_HANDLE
    {
        /* Do nothing */
    }
    _SEH_END;

    return Ret;
}


/*
 * @unimplemented
 */
HWND STDCALL
GetForegroundWindow(VOID)
{
   return NtUserGetForegroundWindow();
}

static
BOOL
User32EnumWindows (
	HDESK hDesktop,
	HWND hWndparent,
	WNDENUMPROC lpfn,
	LPARAM lParam,
	DWORD dwThreadId,
	BOOL bChildren )
{
  DWORD i, dwCount = 0;
  HWND* pHwnd = NULL;
  HANDLE hHeap;

  if ( !lpfn )
    {
      SetLastError ( ERROR_INVALID_PARAMETER );
      return FALSE;
    }

  /* FIXME instead of always making two calls, should we use some
     sort of persistent buffer and only grow it ( requiring a 2nd
     call ) when the buffer wasn't already big enough? */
  /* first get how many window entries there are */
  SetLastError(0);
  dwCount = NtUserBuildHwndList (
    hDesktop, hWndparent, bChildren, dwThreadId, lParam, NULL, 0 );
  if ( !dwCount || GetLastError() )
    return FALSE;

  /* allocate buffer to receive HWND handles */
  hHeap = GetProcessHeap();
  pHwnd = HeapAlloc ( hHeap, 0, sizeof(HWND)*(dwCount+1) );
  if ( !pHwnd )
    {
      SetLastError ( ERROR_NOT_ENOUGH_MEMORY );
      return FALSE;
    }

  /* now call kernel again to fill the buffer this time */
  dwCount = NtUserBuildHwndList (
    hDesktop, hWndparent, bChildren, dwThreadId, lParam, pHwnd, dwCount );
  if ( !dwCount || GetLastError() )
    {
      if ( pHwnd )
	HeapFree ( hHeap, 0, pHwnd );
      return FALSE;
    }

  /* call the user's callback function until we're done or
     they tell us to quit */
  for ( i = 0; i < dwCount; i++ )
  {
    /* FIXME I'm only getting NULLs from Thread Enumeration, and it's
     * probably because I'm not doing it right in NtUserBuildHwndList.
     * Once that's fixed, we shouldn't have to check for a NULL HWND
     * here
     */
    if ( !(ULONG)pHwnd[i] ) /* don't enumerate a NULL HWND */
      continue;
    if ( !(*lpfn)( pHwnd[i], lParam ) )
    {
      HeapFree ( hHeap, 0, pHwnd );
      return FALSE;
    }
  }
  if ( pHwnd )
    HeapFree ( hHeap, 0, pHwnd );
  return TRUE;
}


/*
 * @implemented
 */
BOOL
STDCALL
EnumChildWindows(
	HWND hWndParent,
	WNDENUMPROC lpEnumFunc,
	LPARAM lParam)
{
  if ( !hWndParent )
  {
    return EnumWindows(lpEnumFunc, lParam);
  }
  return User32EnumWindows ( NULL, hWndParent, lpEnumFunc, lParam, 0, TRUE );
}


/*
 * @implemented
 */
BOOL
STDCALL
EnumThreadWindows(DWORD dwThreadId,
		  WNDENUMPROC lpfn,
		  LPARAM lParam)
{
  if ( !dwThreadId )
    dwThreadId = GetCurrentThreadId();
  return User32EnumWindows ( NULL, NULL, lpfn, lParam, dwThreadId, FALSE );
}


/*
 * @implemented
 */
BOOL STDCALL
EnumWindows(WNDENUMPROC lpEnumFunc,
	    LPARAM lParam)
{
  return User32EnumWindows ( NULL, NULL, lpEnumFunc, lParam, 0, FALSE );
}


/*
 * @implemented
 */
BOOL
STDCALL
EnumDesktopWindows(
	HDESK hDesktop,
	WNDENUMPROC lpfn,
	LPARAM lParam)
{
  return User32EnumWindows ( hDesktop, NULL, lpfn, lParam, 0, FALSE );
}


/*
 * @implemented
 */
HWND STDCALL
FindWindowExA(HWND hwndParent,
	      HWND hwndChildAfter,
	      LPCSTR lpszClass,
	      LPCSTR lpszWindow)
{
   UNICODE_STRING ucClassName, *pucClassName = NULL;
   UNICODE_STRING ucWindowName, *pucWindowName = NULL;
   HWND Result;

   if (IS_ATOM(lpszClass))
   {
      ucClassName.Buffer = (LPWSTR)lpszClass;
      ucClassName.Length = 0;
      pucClassName = &ucClassName;
   }
   else if (lpszClass != NULL)
   {
      if (!RtlCreateUnicodeStringFromAsciiz(&ucClassName,
                                            (LPSTR)lpszClass))
      {
         SetLastError(ERROR_NOT_ENOUGH_MEMORY);
         return NULL;
      }
      pucClassName = &ucClassName;
   }

   if (lpszWindow != NULL)
   {
      if (!RtlCreateUnicodeStringFromAsciiz(&ucWindowName,
                                            (LPSTR)lpszWindow))
      {
         if (!IS_ATOM(lpszClass) && lpszClass != NULL)
            RtlFreeUnicodeString(&ucWindowName);

         SetLastError(ERROR_NOT_ENOUGH_MEMORY);
         return NULL;
      }

      pucWindowName = &ucWindowName;
   }

   Result = NtUserFindWindowEx(hwndParent,
                               hwndChildAfter,
                               pucClassName,
                               pucWindowName,
                               0);

   if (!IS_ATOM(lpszClass) && lpszClass != NULL)
      RtlFreeUnicodeString(&ucClassName);
   if (lpszWindow != NULL)
      RtlFreeUnicodeString(&ucWindowName);

   return Result;
}


/*
 * @implemented
 */
HWND STDCALL
FindWindowExW(HWND hwndParent,
	      HWND hwndChildAfter,
	      LPCWSTR lpszClass,
	      LPCWSTR lpszWindow)
{
   UNICODE_STRING ucClassName, *pucClassName = NULL;
   UNICODE_STRING ucWindowName, *pucWindowName = NULL;

   if (IS_ATOM(lpszClass))
   {
      ucClassName.Length = 0;
      ucClassName.Buffer = (LPWSTR)lpszClass;
      pucClassName = &ucClassName;
   }
   else if (lpszClass != NULL)
   {
      RtlInitUnicodeString(&ucClassName,
                           lpszClass);
      pucClassName = &ucClassName;
   }

   if (lpszWindow != NULL)
   {
      RtlInitUnicodeString(&ucWindowName,
                           lpszWindow);
      pucWindowName = &ucWindowName;
   }

   return NtUserFindWindowEx(hwndParent,
                             hwndChildAfter,
                             pucClassName,
                             pucWindowName,
                             0);
}


/*
 * @implemented
 */
HWND STDCALL
FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName)
{
  //FIXME: FindWindow does not search children, but FindWindowEx does.
  //       what should we do about this?
  return FindWindowExA (NULL, NULL, lpClassName, lpWindowName);
}


/*
 * @implemented
 */
HWND STDCALL
FindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName)
{
  /*

  There was a FIXME here earlier, but I think it is just a documentation unclarity.

  FindWindow only searches top level windows. What they mean is that child
  windows of other windows than the desktop can be searched.
  FindWindowExW never does a recursive search.

	/ Joakim
  */

  return FindWindowExW (NULL, NULL, lpClassName, lpWindowName);
}



/*
 * @unimplemented
 */
BOOL STDCALL
GetAltTabInfoA(HWND hwnd,
	       int iItem,
	       PALTTABINFO pati,
	       LPSTR pszItemText,
	       UINT cchItemText)
{
  UNIMPLEMENTED;
  return FALSE;
}


/*
 * @unimplemented
 */
BOOL STDCALL
GetAltTabInfoW(HWND hwnd,
	       int iItem,
	       PALTTABINFO pati,
	       LPWSTR pszItemText,
	       UINT cchItemText)
{
  UNIMPLEMENTED;
  return FALSE;
}


/*
 * @implemented
 */
HWND STDCALL
GetAncestor(HWND hwnd, UINT gaFlags)
{
    HWND Ret = NULL;
    PWINDOW Ancestor, Wnd;
    
    Wnd = ValidateHwnd(hwnd);
    if (!Wnd)
        return NULL;

    _SEH_TRY
    {
        Ancestor = NULL;
        switch (gaFlags)
        {
            case GA_PARENT:
                if (Wnd->Parent != NULL)
                    Ancestor = DesktopPtrToUser(Wnd->Parent);
                break;

            default:
                /* FIXME: Call win32k for now */
                Wnd = NULL;
                break;
        }

        if (Ancestor != NULL)
            Ret = UserHMGetHandle(Ancestor);
    }
    _SEH_HANDLE
    {
        /* Do nothing */
    }
    _SEH_END;

    if (!Wnd) /* Fall back */
        Ret = NtUserGetAncestor(hwnd, gaFlags);

    return Ret;
}


/*
 * @implemented
 */
BOOL STDCALL
GetClientRect(HWND hWnd, LPRECT lpRect)
{
    PWINDOW Wnd = ValidateHwnd(hWnd);

    if (Wnd != NULL)
    {
        lpRect->left = lpRect->top = 0;
        lpRect->right = Wnd->ClientRect.right - Wnd->ClientRect.left;
        lpRect->bottom = Wnd->ClientRect.bottom - Wnd->ClientRect.top;
        return TRUE;
    }

    return FALSE;
}


/*
 * @implemented
 */
BOOL STDCALL
GetGUIThreadInfo(DWORD idThread,
		 LPGUITHREADINFO lpgui)
{
  return (BOOL)NtUserGetGUIThreadInfo(idThread, lpgui);
}


/*
 * @unimplemented
 */
HWND STDCALL
GetLastActivePopup(HWND hWnd)
{
  UNIMPLEMENTED;
  return 0;
}


/*
 * @implemented
 */
HWND STDCALL
GetParent(HWND hWnd)
{
    PWINDOW Wnd, WndParent;
    HWND Ret = NULL;

    Wnd = ValidateHwnd(hWnd);
    if (Wnd != NULL)
    {
        _SEH_TRY
        {
            WndParent = NULL;
            if (Wnd->Style & WS_CHILD)
            {
                if (Wnd->Parent != NULL)
                    WndParent = DesktopPtrToUser(Wnd->Parent);
            }
            else if (Wnd->Style & WS_POPUP)
            {
                if (Wnd->Owner != NULL)
                    WndParent = DesktopPtrToUser(Wnd->Owner);
            }

            if (WndParent != NULL)
                Ret = UserHMGetHandle(WndParent);
        }
        _SEH_HANDLE
        {
            /* Do nothing */
        }
        _SEH_END;
    }

    return Ret;
}


/*
 * @unimplemented
 */
BOOL STDCALL
GetProcessDefaultLayout(DWORD *pdwDefaultLayout)
{
  UNIMPLEMENTED;
  return FALSE;
}


/*
 * @unimplemented
 */
BOOL STDCALL
GetTitleBarInfo(HWND hwnd,
		PTITLEBARINFO pti)
{
  UNIMPLEMENTED;
  return FALSE;
}


/*
 * @implemented
 */
HWND STDCALL
GetWindow(HWND hWnd,
	  UINT uCmd)
{
    PWINDOW Wnd, FoundWnd;
    HWND Ret = NULL;

    Wnd = ValidateHwnd(hWnd);
    if (!Wnd)
        return NULL;

    _SEH_TRY
    {
        FoundWnd = NULL;
        switch (uCmd)
        {
            case GW_OWNER:
                if (Wnd->Owner != NULL)
                    FoundWnd = DesktopPtrToUser(Wnd->Owner);
                break;

            default:
                /* FIXME: Optimize! Fall back to NtUserGetWindow for now... */
                Wnd = NULL;
                break;
        }

        if (FoundWnd != NULL)
            Ret = UserHMGetHandle(FoundWnd);
    }
    _SEH_HANDLE
    {
        /* Do nothing */
    }
    _SEH_END;

    if (!Wnd) /* Fall back to win32k... */
        Ret = NtUserGetWindow(hWnd, uCmd);

    return Ret;
}


/*
 * @implemented
 */
HWND STDCALL
GetTopWindow(HWND hWnd)
{
  if (!hWnd) hWnd = GetDesktopWindow();
  return GetWindow( hWnd, GW_CHILD );
}


/*
 * @implemented
 */
BOOL STDCALL
GetWindowInfo(HWND hwnd,
	      PWINDOWINFO pwi)
{
  return NtUserGetWindowInfo(hwnd, pwi);
}


/*
 * @implemented
 */
UINT STDCALL
GetWindowModuleFileNameA(HWND hwnd,
			 LPSTR lpszFileName,
			 UINT cchFileNameMax)
{
  HINSTANCE hWndInst;

  if(!(hWndInst = NtUserGetWindowInstance(hwnd)))
  {
    return 0;
  }

  return GetModuleFileNameA(hWndInst, lpszFileName, cchFileNameMax);
}


/*
 * @implemented
 */
UINT STDCALL
GetWindowModuleFileNameW(HWND hwnd,
			 LPWSTR lpszFileName,
			 UINT cchFileNameMax)
{
  HINSTANCE hWndInst;

  if(!(hWndInst = NtUserGetWindowInstance(hwnd)))
  {
    return 0;
  }

  return GetModuleFileNameW(hWndInst, lpszFileName, cchFileNameMax);
}


/*
 * @implemented
 */
BOOL STDCALL
GetWindowPlacement(HWND hWnd,
                   WINDOWPLACEMENT *lpwndpl)
{
  return (BOOL)NtUserGetWindowPlacement(hWnd, lpwndpl);
}


/*
 * @implemented
 */
BOOL STDCALL
GetWindowRect(HWND hWnd,
	      LPRECT lpRect)
{
    PWINDOW Wnd = ValidateHwnd(hWnd);

    if (Wnd != NULL)
    {
        *lpRect = Wnd->WindowRect;
        return TRUE;
    }

    return FALSE;
}


/*
 * @implemented
 */
int STDCALL
GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount)
{
    PWINDOW Wnd;
    PCWSTR Buffer;
    INT Length = 0;

    if (lpString == NULL)
        return 0;

    Wnd = ValidateHwnd(hWnd);
    if (!Wnd)
        return 0;

    _SEH_TRY
    {
        if (Wnd->pi != g_kpi)
        {
            if (nMaxCount > 0)
            {
                /* do not send WM_GETTEXT messages to other processes */
                Length = Wnd->WindowName.Length / sizeof(WCHAR);
                if (Length != 0)
                {
                    Buffer = DesktopPtrToUser(Wnd->WindowName.Buffer);
                    if (Buffer != NULL)
                    {
                        if (!WideCharToMultiByte(CP_ACP,
                                               0,
                                               Buffer,
                                               Length + 1,
                                               lpString,
                                               nMaxCount,
                                               NULL,
                                               NULL))
                        {
                            lpString[nMaxCount - 1] = '\0';
                        }
                    }
                    else
                    {
                        Length = 0;
                        lpString[0] = '\0';
                    }
                }
                else
                    lpString[0] = '\0';
            }

            Wnd = NULL; /* Don't send a message */
        }
    }
    _SEH_HANDLE
    {
        lpString[0] = '\0';
        Length = 0;
        Wnd = NULL; /* Don't send a message */
    }
    _SEH_END;

    if (Wnd != NULL)
        Length = SendMessageA(hWnd, WM_GETTEXT, nMaxCount, (LPARAM)lpString);

    return Length;
}


/*
 * @implemented
 */
int STDCALL
GetWindowTextLengthA(HWND hWnd)
{
    return(SendMessageA(hWnd, WM_GETTEXTLENGTH, 0, 0));
}


/*
 * @implemented
 */
int STDCALL
GetWindowTextLengthW(HWND hWnd)
{
    return(SendMessageW(hWnd, WM_GETTEXTLENGTH, 0, 0));
}


/*
 * @implemented
 */
int STDCALL
GetWindowTextW(HWND hWnd, LPWSTR lpString, int nMaxCount)
{
    PWINDOW Wnd;
    PCWSTR Buffer;
    INT Length = 0;

    if (lpString == NULL)
        return 0;

    Wnd = ValidateHwnd(hWnd);
    if (!Wnd)
        return 0;

    _SEH_TRY
    {
        if (Wnd->pi != g_kpi)
        {
            if (nMaxCount > 0)
            {
                /* do not send WM_GETTEXT messages to other processes */
                Length = Wnd->WindowName.Length / sizeof(WCHAR);
                if (Length != 0)
                {
                    Buffer = DesktopPtrToUser(Wnd->WindowName.Buffer);
                    if (Buffer != NULL)
                    {
                        RtlCopyMemory(lpString,
                                      Buffer,
                                      (Length + 1) * sizeof(WCHAR));
                    }
                    else
                    {
                        Length = 0;
                        lpString[0] = '\0';
                    }
                }
                else
                    lpString[0] = '\0';
            }

            Wnd = NULL; /* Don't send a message */
        }
    }
    _SEH_HANDLE
    {
        lpString[0] = '\0';
        Length = 0;
        Wnd = NULL; /* Don't send a message */
    }
    _SEH_END;

    if (Wnd != NULL)
        Length = SendMessageW(hWnd, WM_GETTEXT, nMaxCount, (LPARAM)lpString);

    return Length;
}

DWORD STDCALL
GetWindowThreadProcessId(HWND hWnd,
			 LPDWORD lpdwProcessId)
{
   return NtUserGetWindowThreadProcessId(hWnd, lpdwProcessId);
}


/*
 * @implemented
 */
BOOL STDCALL
IsChild(HWND hWndParent,
	HWND hWnd)
{
    PWINDOW WndParent, Wnd;
    BOOL Ret = FALSE;

    WndParent = ValidateHwnd(hWndParent);
    if (!WndParent)
        return FALSE;
    Wnd = ValidateHwnd(hWnd);
    if (!Wnd)
        return FALSE;

    _SEH_TRY
    {
        while (Wnd != NULL)
        {
            if (Wnd->Parent != NULL)
            {
                Wnd = DesktopPtrToUser(Wnd->Parent);
                if (Wnd == WndParent)
                {
                    Ret = TRUE;
                    break;
                }
            }
            else
                break;
        }
    }
    _SEH_HANDLE
    {
        /* Do nothing */
    }
    _SEH_END;

    return Ret;
}


/*
 * @implemented
 */
BOOL STDCALL
IsIconic(HWND hWnd)
{
    PWINDOW Wnd = ValidateHwnd(hWnd);

    if (Wnd != NULL)
        return (Wnd->Style & WS_MINIMIZE) != 0;

    return FALSE;
}


/*
 * @implemented
 */
BOOL STDCALL
IsWindow(HWND hWnd)
{
    PWINDOW Wnd = ValidateHwndNoErr(hWnd);
    if (Wnd != NULL)
    {
        /* FIXME: If window is being destroyed return FALSE! */
        return TRUE;
    }

    return FALSE;
}


/*
 * @implemented
 */
BOOL STDCALL
IsWindowUnicode(HWND hWnd)
{
    PWINDOW Wnd = ValidateHwnd(hWnd);

    if (Wnd != NULL)
        return Wnd->Unicode;

    return FALSE;
}


/*
 * @implemented
 */
BOOL STDCALL
IsWindowVisible(HWND hWnd)
{
    BOOL Ret = FALSE;
    PWINDOW Wnd = ValidateHwnd(hWnd);

    if (Wnd != NULL)
    {
        _SEH_TRY
        {
            Ret = TRUE;

            do
            {
                if (!(Wnd->Style & WS_VISIBLE))
                {
                    Ret = FALSE;
                    break;
                }

                if (Wnd->Parent != NULL)
                    Wnd = DesktopPtrToUser(Wnd->Parent);
                else
                    break;

            } while (Wnd != NULL);
        }
        _SEH_HANDLE
        {
            Ret = FALSE;
        }
        _SEH_END;
    }

    return Ret;
}


/*
 * @implemented
 */
BOOL
STDCALL
IsWindowEnabled(
  HWND hWnd)
{
    // AG: I don't know if child windows are affected if the parent is
    // disabled. I think they stop processing messages but stay appearing
    // as enabled.

    return !(GetWindowLongW(hWnd, GWL_STYLE) & WS_DISABLED);
}


/*
 * @implemented
 */
BOOL STDCALL
IsZoomed(HWND hWnd)
{
    return (GetWindowLongW(hWnd, GWL_STYLE) & WS_MAXIMIZE) != 0;
}


/*
 * @unimplemented
 */
BOOL STDCALL
LockSetForegroundWindow(UINT uLockCode)
{
  UNIMPLEMENTED;
  return FALSE;
}


/*
 * @implemented
 */
BOOL STDCALL
MoveWindow(HWND hWnd,
	   int X,
	   int Y,
	   int nWidth,
	   int nHeight,
	   BOOL bRepaint)
{
  return NtUserMoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);
}


/*
 * @implemented
 */
BOOL STDCALL
AnimateWindow(HWND hwnd,
	      DWORD dwTime,
	      DWORD dwFlags)
{
  /* FIXME Add animation code */

  /* If trying to show/hide and it's already   *
   * shown/hidden or invalid window, fail with *
   * invalid parameter                         */

  BOOL visible;
  visible = IsWindowVisible(hwnd);
  if(!IsWindow(hwnd) ||
    (visible && !(dwFlags & AW_HIDE)) ||
    (!visible && (dwFlags & AW_HIDE)))
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }

  ShowWindow(hwnd, (dwFlags & AW_HIDE) ? SW_HIDE : ((dwFlags & AW_ACTIVATE) ? SW_SHOW : SW_SHOWNA));

  return TRUE;
}


/*
 * @implemented
 */
BOOL STDCALL
OpenIcon(HWND hWnd)
{
    if (!(GetWindowLongW(hWnd, GWL_STYLE) & WS_MINIMIZE))
        return FALSE;

    ShowWindow(hWnd,SW_RESTORE);
    return TRUE;
}


/*
 * @unimplemented
 */
HWND STDCALL
RealChildWindowFromPoint(HWND hwndParent,
			 POINT ptParentClientCoords)
{
  UNIMPLEMENTED;
  return (HWND)0;
}

/*
 * @unimplemented
 */
BOOL STDCALL
SetForegroundWindow(HWND hWnd)
{
   return NtUserCallHwndLock(hWnd, HWNDLOCK_ROUTINE_SETFOREGROUNDWINDOW);
}


/*
 * @unimplemented
 */
BOOL STDCALL
SetLayeredWindowAttributes(HWND hwnd,
			   COLORREF crKey,
			   BYTE bAlpha,
			   DWORD dwFlags)
{
  UNIMPLEMENTED;
  return FALSE;
}


/*
 * @implemented
 */
HWND STDCALL
SetParent(HWND hWndChild,
	  HWND hWndNewParent)
{
  return NtUserSetParent(hWndChild, hWndNewParent);
}


/*
 * @unimplemented
 */
BOOL STDCALL
SetProcessDefaultLayout(DWORD dwDefaultLayout)
{
  UNIMPLEMENTED;
  return FALSE;
}


/*
 * @unimplemented
 */
BOOL STDCALL
SetWindowPlacement(HWND hWnd,
		   CONST WINDOWPLACEMENT *lpwndpl)
{
  return (BOOL)NtUserSetWindowPlacement(hWnd, (WINDOWPLACEMENT *)lpwndpl);
}


/*
 * @implemented
 */
BOOL STDCALL
SetWindowPos(HWND hWnd,
	     HWND hWndInsertAfter,
	     int X,
	     int Y,
	     int cx,
	     int cy,
	     UINT uFlags)
{
  return NtUserSetWindowPos(hWnd,hWndInsertAfter, X, Y, cx, cy, uFlags);
}


/*
 * @implemented
 */
BOOL STDCALL
SetWindowTextA(HWND hWnd,
	       LPCSTR lpString)
{
  DWORD ProcessId;
  if(!NtUserGetWindowThreadProcessId(hWnd, &ProcessId))
  {
    return FALSE;
  }

  if(ProcessId != GetCurrentProcessId())
  {
    /* do not send WM_GETTEXT messages to other processes */
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    if(lpString)
    {
      RtlInitAnsiString(&AnsiString, (LPSTR)lpString);
      RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
      NtUserDefSetText(hWnd, &UnicodeString);
      RtlFreeUnicodeString(&UnicodeString);
    }
    else
      NtUserDefSetText(hWnd, NULL);

    if ((GetWindowLongW(hWnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION)
    {
      DefWndNCPaint(hWnd, (HRGN)1, -1);
    }
    return TRUE;
  }

  return SendMessageA(hWnd, WM_SETTEXT, 0, (LPARAM)lpString);
}


/*
 * @implemented
 */
BOOL STDCALL
SetWindowTextW(HWND hWnd,
	       LPCWSTR lpString)
{
  DWORD ProcessId;
  if(!NtUserGetWindowThreadProcessId(hWnd, &ProcessId))
  {
    return FALSE;
  }

  if(ProcessId != GetCurrentProcessId())
  {
    /* do not send WM_GETTEXT messages to other processes */
    UNICODE_STRING UnicodeString;

    if(lpString)
      RtlInitUnicodeString(&UnicodeString, (LPWSTR)lpString);

    NtUserDefSetText(hWnd, (lpString ? &UnicodeString : NULL));

    if ((GetWindowLongW(hWnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION)
    {
      DefWndNCPaint(hWnd, (HRGN)1, -1);
    }
    return TRUE;
  }

  return SendMessageW(hWnd, WM_SETTEXT, 0, (LPARAM)lpString);
}


/*
 * @implemented
 */
BOOL STDCALL
ShowOwnedPopups(HWND hWnd,
		BOOL fShow)
{
  return (BOOL)NtUserCallTwoParam((DWORD)hWnd, fShow, TWOPARAM_ROUTINE_SHOWOWNEDPOPUPS);
}


/*
 * @implemented
 */
BOOL STDCALL
ShowWindow(HWND hWnd,
	   int nCmdShow)
{
  return NtUserShowWindow(hWnd, nCmdShow);
}


/*
 * @unimplemented
 */
BOOL STDCALL
ShowWindowAsync(HWND hWnd,
		int nCmdShow)
{
  return NtUserShowWindowAsync(hWnd, nCmdShow);
}


/*
 * @unimplemented
 */
/*
WORD STDCALL
TileWindows(HWND hwndParent,
	    UINT wHow,
	    CONST RECT *lpRect,
	    UINT cKids,
	    const HWND *lpKids)
{
  UNIMPLEMENTED;
  return 0;
}
*/


/*
 * @unimplemented
 */
BOOL STDCALL
UpdateLayeredWindow(HWND hwnd,
		    HDC hdcDst,
		    POINT *pptDst,
		    SIZE *psize,
		    HDC hdcSrc,
		    POINT *pptSrc,
		    COLORREF crKey,
		    BLENDFUNCTION *pblend,
		    DWORD dwFlags)
{
  UNIMPLEMENTED;
  return FALSE;
}


/*
 * @implemented
 */
HWND STDCALL
WindowFromPoint(POINT Point)
{
  //TODO: Determine what the actual parameters to
  // NtUserWindowFromPoint are.
  return NtUserWindowFromPoint(Point.x, Point.y);
}


/*
 * @implemented
 */
int STDCALL
MapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints)
{
    PWINDOW FromWnd, ToWnd;
    POINT Delta;
    UINT i;

    FromWnd = ValidateHwndOrDesk(hWndFrom);
    if (!FromWnd)
        return 0;

    ToWnd = ValidateHwndOrDesk(hWndTo);
    if (!ToWnd)
        return 0;

    Delta.x = FromWnd->ClientRect.left - ToWnd->ClientRect.left;
    Delta.y = FromWnd->ClientRect.top - ToWnd->ClientRect.top;

    for (i = 0; i != cPoints; i++)
    {
        lpPoints[i].x += Delta.x;
        lpPoints[i].y += Delta.y;
    }

    return MAKELONG(LOWORD(Delta.x), LOWORD(Delta.y));
}


/*
 * @implemented
 */
BOOL STDCALL
ScreenToClient(HWND hWnd, LPPOINT lpPoint)
{
    PWINDOW Wnd, DesktopWnd;

    Wnd = ValidateHwnd(hWnd);
    if (!Wnd)
        return FALSE;

    DesktopWnd = GetThreadDesktopWnd();

    lpPoint->x += DesktopWnd->ClientRect.left - Wnd->ClientRect.left;
    lpPoint->y += DesktopWnd->ClientRect.top - Wnd->ClientRect.top;

    return TRUE;
}


/*
 * @implemented
 */
BOOL STDCALL
ClientToScreen(HWND hWnd, LPPOINT lpPoint)
{
    PWINDOW Wnd, DesktopWnd;

    Wnd = ValidateHwnd(hWnd);
    if (!Wnd)
        return FALSE;

    DesktopWnd = GetThreadDesktopWnd();

    lpPoint->x += Wnd->ClientRect.left - DesktopWnd->ClientRect.left;
    lpPoint->y += Wnd->ClientRect.top - DesktopWnd->ClientRect.top;

    return TRUE;
}


/*
 * @implemented
 */
BOOL
STDCALL
SetWindowContextHelpId(HWND hwnd,
          DWORD dwContextHelpId)
{
  return NtUserSetWindowContextHelpId(hwnd, dwContextHelpId);
}


/*
 * @implemented
 */
DWORD
STDCALL
GetWindowContextHelpId(HWND hwnd)
{
    PWINDOW Wnd = ValidateHwnd(hwnd);
    if (Wnd != NULL)
    {
        return Wnd->ContextHelpId;
    }

    return 0;
}

/*
 * @implemented
 */
int
STDCALL
InternalGetWindowText(HWND hWnd, LPWSTR lpString, int nMaxCount)
{
    INT Ret = NtUserInternalGetWindowText(hWnd, lpString, nMaxCount);
    if (Ret == 0)
        *lpString = L'\0';
    return Ret;
}

/*
 * @implemented
 */
BOOL
STDCALL
IsHungAppWindow(HWND hwnd)
{
  return (NtUserQueryWindow(hwnd, QUERY_WINDOW_ISHUNG) != 0);
}

/*
 * @implemented
 */
VOID
STDCALL
SetLastErrorEx(DWORD dwErrCode, DWORD dwType)
{
  SetLastError(dwErrCode);
}

/*
 * @implemented
 */
HWND
STDCALL
GetFocus(VOID)
{
  return (HWND)NtUserGetThreadState(THREADSTATE_FOCUSWINDOW);
}

/*
 * @implemented
 */
HWND
STDCALL
SetTaskmanWindow(HWND hWnd)
{
    return NtUserCallHwndOpt(hWnd, HWNDOPT_ROUTINE_SETTASKMANWINDOW);
}

/*
 * @implemented
 */
HWND
STDCALL
SetProgmanWindow(HWND hWnd)
{
    return NtUserCallHwndOpt(hWnd, HWNDOPT_ROUTINE_SETPROGMANWINDOW);
}

/*
 * @implemented
 */
HWND
STDCALL
GetProgmanWindow(VOID)
{
  return (HWND)NtUserGetThreadState(THREADSTATE_PROGMANWINDOW);
}

/*
 * @implemented
 */
HWND
STDCALL
GetTaskmanWindow(VOID)
{
  return (HWND)NtUserGetThreadState(THREADSTATE_TASKMANWINDOW);
}

/*
 * @implemented
 */
BOOL STDCALL
ScrollWindow(HWND hWnd, int dx, int dy, CONST RECT *lpRect,
   CONST RECT *prcClip)
{
   return NtUserScrollWindowEx(hWnd, dx, dy, lpRect, prcClip, 0, NULL,
      (lpRect ? 0 : SW_SCROLLCHILDREN) | SW_INVALIDATE) != ERROR;
}


/*
 * @implemented
 */
INT STDCALL
ScrollWindowEx(HWND hWnd, int dx, int dy, CONST RECT *prcScroll,
   CONST RECT *prcClip, HRGN hrgnUpdate, LPRECT prcUpdate, UINT flags)
{
   return NtUserScrollWindowEx(hWnd, dx, dy, prcScroll, prcClip, hrgnUpdate,
      prcUpdate, flags);
}

/*
 * @implemented
 */
BOOL
STDCALL
AnyPopup(VOID)
{
  return NtUserAnyPopup();
}

/*
 * @implemented
 */
BOOL
STDCALL
IsWindowInDestroy(HWND hWnd)
{
  return NtUserIsWindowInDestroy(hWnd);
}

/*
 * @implemented
 */
VOID
STDCALL
DisableProcessWindowsGhosting(VOID)
{
  NtUserEnableProcessWindowGhosting(FALSE);
}

/* EOF */




