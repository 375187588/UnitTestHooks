API Hook
========

This object provides a mechanism to override system and runtime APIs for use with unit-testing.  
This object allows APIs to be selectively overridden. Therefore, the feature can be enabled and disabled in the same executable.  
This is a valuable ability when a test could not normally be performed otherwise.  
  
Examples:  
  -Tests that relate to time  
  -Tests that open files  
  -Tests that require network communications  
  -Tests that require system permissions not accessible to the test machine.  

However, there is no reason this class could not be used to hook API calls for other purposes.

Example
=======

`#include "ApiHook.h"`
``
`//  Forward Declarations `
`typedef int (WINAPI *pfnMessageBoxA)(HWND, PCSTR, PCSTR, UINT);`
``
`ApiHook *g_pMessageBoxA = NULL;`
``
`// An override that will be called when the target API is hooked.`
`int WINAPI Hook_MessageBoxA(HWND hWnd, PCSTR pText, PCSTR pCaption, UINT type)`
`{`
`  return ((pfnMessageBoxA)(PROC)*g_pMessageBoxA)(hWnd, pText, "Consider MessageBoxA, Hooked!", type);`
`}`
``
`int _tmain(int argc, _TCHAR* argv[])`
`{`
`  g_pMessageBoxA = new ApiHook("User32.dll", "MessageBoxA", (PROC)Hook_MessageBoxA);`
``
`  // While the API is hooked, the caption will be replaced.`
`  MessageBoxA(NULL, "Testing the ApiHook functionality", "This is the caption", MB_OK);`
``
`  delete g_pMessageBoxA;`
`  g_pMessageBoxA = NULL;`
``
`  MessageBoxA(NULL, "Testing the ApiHook functionality", "This is the caption", MB_OK);``
``
`	return 0;`
}`
  
Future
======
I would like to extend this mechanism to be portible to *nix based systems.  
I am aware of LD_PRELOAD, dl_open and dl_sym. I am investigating other methods I have seen used. 