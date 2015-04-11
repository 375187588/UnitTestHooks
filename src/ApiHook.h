/// @file   ApiHook.h 
/// 
/// API Hook library for unit-tests
///
/// This object provides a mechanism to override system and runtime APIs for 
/// use with unit-testing.  This is a valuable ability when a test could not
/// normally be performed otherwise.  
///
/// Examples:
///   Tests that relate to time
///   Tests that open files
///   Tests that require network communications
///   Tests that require system permissions not accessible to the test machine.
///
/// However, there is no reason this class could not be used to hook API
/// calls for other purposes.
///
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
/// The Windows implementation has been adapted from a class
/// developed by Jeffrey Richter and published in the book
///     "Windows Via C/C++".
///
//  ****************************************************************************
#ifndef APIHOOK_H_INCLUDED
#define APIHOOK_H_INCLUDED
//  Includes *******************************************************************
#include <vector>
#include <windows.h>

//  ****************************************************************************
/// Provides a simple mechanism to Hook single API calls exported from a library.
/// The intended primary use for this object is with Unit-testing.
///                 
class ApiHook
{
public:
  ApiHook(const char* pLibName, const char* pFnName, PROC pfnHook);
 ~ApiHook();

  operator PROC()                                 { return m_pfnOrig;}

  static 
    FARPROC WINAPI GetProcAddressRaw(HMODULE hMod, const char* pProcName);

  static 
    HMODULE GetExcludeModuleHandle();

  static 
    bool    GetModuleExclude()                    { return sm_isExclude;}

private:
  //  Typedef ******************************************************************
  typedef std::vector<ApiHook*>                   ApiHookArray;

  //  Data Members *************************************************************
  static
    ApiHookArray  sm_hooks;             ///< A static array of pointers to ApiHook 
                                        ///  objects that are currently active.                                      
  static 
    PVOID         sm_pMaxAppAddr;       ///< The maximum private memory address 
                                        ///  for this module.
  static
    bool          sm_isExclude;         ///< Indicates if the module this object
                                        ///  instance resides in should be excluded 
                                        ///  from API Hooks.
                                        
  std::string     m_libName;            ///<  Library module that contains the 
                                        ///   function to be hooked.

  std::string     m_fnName;             ///< The name of the function to be hooked.  
                                        
  PROC            m_pfnOrig;            ///< Address to the original function.
                                        
  PROC            m_pfnHook;            ///< Address to the hook function.
  
  //  Instantiate Hooks for these API related system calls. ********************
#ifdef WIN32
  static ApiHook sm_LoadLibraryA;                 
  static ApiHook sm_LoadLibraryW;
  static ApiHook sm_LoadLibraryExA;
  static ApiHook sm_LoadLibraryExW;
  static ApiHook sm_GetProcAddress;
#endif

  //  Methods ******************************************************************
  static
    void WINAPI ReplaceIATEntry(
      const char* pLibName,
      PROC        pfnOrig,
      PROC        pfnHook,
      HMODULE     hModCaller
    );

  static
    void WINAPI ReplaceIATEntryEx(
      const char* pLibName,
      PROC        pfnOrig,
      PROC        pfnHook
    );

  static
    void WINAPI ReplaceEATEntry(
      HMODULE     hMod,
      const char* pFnName,
      PROC        pfnNew
    );


  static 
    void WINAPI FixupModuleOnLoad(
      HMODULE hMod, 
      DWORD   flags
    );

  static 
    HMODULE WINAPI LoadLibraryA(
      PCSTR pszModulePath
    );

  static 
    HMODULE WINAPI LoadLibraryW(
      PCWSTR pszModulePath
    );

  static 
    HMODULE WINAPI LoadLibraryExA(
      PCSTR   pszModulePath,
      HANDLE  hFile,
      DWORD   flags
    );

  static 
    HMODULE WINAPI LoadLibraryExW(
      PCWSTR  pszModulePath,
      HANDLE  hFile,
      DWORD   flags
    );

  static 
    FARPROC WINAPI GetProcAddress(
      HMODULE     hMod,
      const char* pFnName
    );

};


#endif
