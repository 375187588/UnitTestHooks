// ApiHook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ApiHook.h"
int WINAPI Hook_MessageBoxA(HWND hWnd, PCSTR pText, PCSTR pCaption, UINT type);

typedef int (WINAPI *pfnMessageBoxA)(HWND, PCSTR, PCSTR, UINT);

ApiHook *g_pMessageBoxA = NULL;

int WINAPI Hook_MessageBoxA(HWND hWnd, PCSTR pText, PCSTR pCaption, UINT type)
{
  return ((pfnMessageBoxA)(PROC)*g_pMessageBoxA)(hWnd, pText, "Consider MessageBoxA, Hooked!", type);
}

int _tmain(int argc, _TCHAR* argv[])
{
  g_pMessageBoxA = new ApiHook("User32.dll", "MessageBoxA", (PROC)Hook_MessageBoxA);

  // While the API is hooked, the caption will be replaced.
  MessageBoxA(NULL, "Testing the ApiHook functionality", "This is the caption", MB_OK);

  delete g_pMessageBoxA;
  g_pMessageBoxA = NULL;

  MessageBoxA(NULL, "Testing the ApiHook functionality", "This is the caption", MB_OK);

	return 0;
}

