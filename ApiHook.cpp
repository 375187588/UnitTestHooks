/// @file   ApiHook.h 
/// 
/// API Hook library for unit-tests
///
/// The MIT License(MIT)
/// @copyright 2014 Paul M Watt
///
/// The Windows implementation has been adapted from a class
/// developed by Jeffrey Richter and published in the book
///     "Windows Via C/C++".
///
//  ****************************************************************************
//  Includes *******************************************************************
#include "ApiHook.h"
#include <algorithm>

#ifdef WIN32
# include <ImageHlp.h>
# pragma  comment(lib, "ImageHlp")

# include <TlHelp32.h>
# include <StrSafe.h>
#else
# error "An implementation to Hook API calls has not been provided for this platform."
#endif

//  Static Data Members ********************************************************
ApiHook::ApiHookArray ApiHook::sm_hooks;          ///< Declare static instance.

bool    ApiHook::sm_isExclude   = false;          ///< Exclude this module by default.
PVOID   ApiHook::sm_pMaxAppAddr = NULL;           ///< Initialize value on startup.


#ifdef WIN32
ApiHook ApiHook::sm_LoadLibraryA  ("Kernel32.dll", "LoadLibraryA",   (PROC)ApiHook::LoadLibraryA);
ApiHook ApiHook::sm_LoadLibraryW  ("Kernel32.dll", "LoadLibraryW",   (PROC)ApiHook::LoadLibraryW);
ApiHook ApiHook::sm_LoadLibraryExA("Kernel32.dll", "LoadLibraryExA", (PROC)ApiHook::LoadLibraryExA);
ApiHook ApiHook::sm_LoadLibraryExW("Kernel32.dll", "LoadLibraryExW", (PROC)ApiHook::LoadLibraryExW);
ApiHook ApiHook::sm_GetProcAddress("Kernel32.dll", "GetProcAddress", (PROC)ApiHook::GetProcAddress);
#endif

//  Forward Declarations *******************************************************
namespace // unnamed
{

HMODULE GetModuleFromAddress(PVOID pv);
bool ReplaceFunctionAddress(PROC* ppfnOrig, PROC pfnNew);

#ifdef WIN32
LONG WINAPI InvalidReadExceptionFilter(PEXCEPTION_POINTERS pep);
#endif

} // namespace anonymous

//  Implementation *************************************************************
//  ****************************************************************************
ApiHook::ApiHook(
  const char* pLibName, 
  const char* pFnName, 
  PROC pfnHook
)
  : m_libName(pLibName)
  , m_fnName(pFnName)
  , m_pfnHook(pfnHook)
{
  sm_hooks.push_back(this);

#ifdef WIN32
  // Query for the address of the original function to hook.
  HMODULE hModule = ::GetModuleHandleA(pLibName);
  m_pfnOrig       = GetProcAddressRaw(hModule, pFnName);

  // If the function does not exist, exit.
  // This usually occurs because the library is not yet loaded.
  if (!m_pfnOrig)
  {
    wchar_t pathName[MAX_PATH];
    ::GetModuleFileNameW(NULL, pathName, MAX_PATH);

    const size_t k_msgSize = 1024;
    wchar_t msg[k_msgSize];
    ::StringCchPrintfW( msg, 
                        k_msgSize, 
                        L"[%4u - %s] Impossible to find %S\r\n",
                        ::GetCurrentProcessId(), 
                        pathName, 
                        pFnName
                      );
    ::OutputDebugString(msg);
    return;
  }

#endif

  // Hook the requested function for all currently loaded modules.
  ReplaceIATEntryEx(pLibName, m_pfnOrig, m_pfnHook);
}

//  ****************************************************************************
ApiHook::~ApiHook()
{
  // Unhook this function from all modules.
  ReplaceIATEntryEx(m_libName.c_str(), m_pfnHook, m_pfnOrig);

  // Remove this object from the management container.
  ApiHookArray::iterator iter = std::find(sm_hooks.begin(), sm_hooks.end(), this);
  if (iter != sm_hooks.end())
  {
    sm_hooks.erase(iter);
  }
}

//  IMPORTANT: Do not inline this function. ************************************
FARPROC WINAPI ApiHook::GetProcAddressRaw(
  HMODULE hMod, 
  const char* pProcName
)
{
  typedef FARPROC (WINAPI *pfnGetProcAddress)(HMODULE, PCSTR);

  pfnGetProcAddress pfnProc = (pfnGetProcAddress)(PROC)sm_GetProcAddress;
  if (!pfnProc)
  {
    // This function has not yet been hooked.
    return ::GetProcAddress(hMod, pProcName);
  }

  return pfnProc(hMod, pProcName);
}

//  ****************************************************************************
HMODULE ApiHook::GetExcludeModuleHandle()
{
  return  GetModuleExclude()
          ? GetModuleFromAddress(GetExcludeModuleHandle)
          : NULL;
}

//  ****************************************************************************
void WINAPI ApiHook::ReplaceIATEntryEx( 
  const char* pLibName, 
  PROC pfnOrig, 
  PROC pfnHook 
)
{
  HMODULE hThisMod  = GetExcludeModuleHandle();

#ifdef WIN32
  // Request a list of library modules in this process.
  HANDLE hModuleSnap = 
    ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ::GetCurrentProcessId());
  if (INVALID_HANDLE_VALUE == hModuleSnap) 
  {
    return;
  }

  MODULEENTRY32 entry;
  entry.dwSize = sizeof(entry);
  BOOL isContinue = TRUE;
  for ( isContinue = ::Module32First(hModuleSnap, &entry); 
        isContinue;
        isContinue = ::Module32Next(hModuleSnap, &entry)) 
  {
    // Don't hook functions from modules that match hThisMod;
    if (entry.hModule != hThisMod)
    {
      // Hook this function in the specified module.
      ReplaceIATEntry(pLibName, pfnOrig, pfnHook, entry.hModule);
    }
  }

  ::CloseHandle(hModuleSnap);
  hModuleSnap = NULL;

#endif

}

//  ****************************************************************************
void WINAPI ApiHook::ReplaceIATEntry( 
  const char* pLibName, 
  PROC        pfnOrig, 
  PROC        pfnHook,
  HMODULE     hModCaller
)
{
  // Exceptions may occur during this call based on threading, the state of
  // library loads and unloads.  Protect with the read violation handler.
  // Get the address of the modules import section.
  ULONG                     size            = 0;
  PIMAGE_IMPORT_DESCRIPTOR  pImportDesc     = NULL;
  PIMAGE_SECTION_HEADER     pSectionHeader  = NULL;
  __try 
  {
    pImportDesc = PIMAGE_IMPORT_DESCRIPTOR(
      ::ImageDirectoryEntryToDataEx(hModCaller,
                                    TRUE,
                                    IMAGE_DIRECTORY_ENTRY_IMPORT,
                                    &size,
                                    &pSectionHeader
                                   ));
  }
  __except (InvalidReadExceptionFilter(GetExceptionInformation()))
  {
    // No current operations.
  }

  if (!pImportDesc)
  {
    // The module has no import section
    // or is no longer loaded into memory.
    return;
  }

  // Search for any references to the callee's functions
  for (; pImportDesc->Name; pImportDesc++)
  {
    // Find entries that match the requested library.
    char* pModName = (char*)((PBYTE) hModCaller + pImportDesc->Name);
    if (0 != ::lstrcmpiA(pModName, pLibName))
    { // This entry does not match.
      continue;
    }

    // Get the caller's import address table (IAT) for the lib's functions.
    PIMAGE_THUNK_DATA pThunk = PIMAGE_THUNK_DATA(
      (PBYTE) hModCaller + pImportDesc->FirstThunk);

    // Replace the current address, with the hook address.
    for (; pThunk->u1.Function; pThunk++)
    {
      // Get the address to the function pointer.
      PROC* ppfn = (PROC*) &pThunk->u1.Function;

      // Is this the function we are looking for?
      bool isFound = (**ppfn == pfnOrig);
      if (!isFound)
      { // This is not the correct function.  Skip to the next one.
        continue;
      }

      // Attempt to write the new address.
      ReplaceFunctionAddress(ppfn, pfnHook);
      return;
    }
  }
}

//  ****************************************************************************
void WINAPI ApiHook::ReplaceEATEntry(
  HMODULE     hMod,
  const char* pFnName,
  PROC        pfnNew
)
{
  // Exceptions may occur during this call based on threading, the state of
  // library loads and unloads.  Protect with the read violation handler.
  // Get the address of the modules import section.
  ULONG                     size            = 0;
  PIMAGE_EXPORT_DIRECTORY   pExportDir      = NULL;
  PIMAGE_SECTION_HEADER     pSectionHeader  = NULL;
  __try 
  {
    pExportDir = PIMAGE_EXPORT_DIRECTORY(
      ::ImageDirectoryEntryToDataEx(hMod,
                                    TRUE,
                                    IMAGE_DIRECTORY_ENTRY_EXPORT,
                                    &size,
                                    &pSectionHeader
                                   ));
  }
  __except (InvalidReadExceptionFilter(GetExceptionInformation()))
  {
    // No current operations.
  }

  if (!pExportDir)
  {
    // The module has no import section
    // or is no longer loaded into memory.
    return;
  }

  DWORD* pNamesRvas    = (DWORD*)((BYTE*)hMod + pExportDir->AddressOfNames);
  WORD*  pNameOrdinals = (WORD*) ((BYTE*)hMod + pExportDir->AddressOfNameOrdinals);
  DWORD* pFnAddresses  = (DWORD*)((BYTE*)hMod + pExportDir->AddressOfFunctions);

  // Walk the array of this modules functions.
  for (DWORD index = 0; index < pExportDir->NumberOfNames; ++index)
  {
    // Find entries that match the requested function.
    char* pName = (char*)((BYTE*) hMod + pNamesRvas[index]);
    if (0 != ::lstrcmpiA(pName, pFnName))
    { // This entry does not match.
      continue;
    }

    // Get this functions ordinal value.
    WORD ordinal = pNameOrdinals[index];

    // Get the address for the specified function.
    PROC* ppfn = (PROC*) &pFnAddresses[index];

    // Turn the new address into an RVA.
    pfnNew = (PROC) ((BYTE*) pfnNew - (BYTE*)hMod);

    // Update the function address.
    ReplaceFunctionAddress(ppfn, pfnNew);
    break;
  }
}

//  ****************************************************************************
void ApiHook::FixupModuleOnLoad(HMODULE hMod, DWORD flags)
{
  // If a new module is loaded,
  // hook the specified hook functions.
  if ( hMod != NULL
    && hMod != GetExcludeModuleHandle()
    && 0 == (flags & LOAD_LIBRARY_AS_DATAFILE)
    && 0 == (flags & LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE)
    && 0 == (flags & LOAD_LIBRARY_AS_IMAGE_RESOURCE))
  {
    // Process every registered API Hook.
    ApiHookArray::iterator iter = sm_hooks.begin();
    ApiHookArray::iterator end  = sm_hooks.end();
    for (; iter != end; ++iter)
    {
      ApiHook* pHook = *iter;
      if (pHook->m_pfnOrig)
      {
        ReplaceIATEntryEx(pHook->m_libName.c_str(), 
                          pHook->m_pfnOrig, 
                          pHook->m_pfnHook
                         );
      }
    }
  }
}

#ifdef WIN32
//  ****************************************************************************
HMODULE WINAPI ApiHook::LoadLibraryA(
  PCSTR pszModulePath
)
{
  typedef HMODULE (WINAPI *pfnLoadLibraryA)(PCSTR);

  HMODULE hMod = NULL;
  if ((PROC)sm_LoadLibraryA)
  {
    pfnLoadLibraryA pfnProc = (pfnLoadLibraryA)(PROC)sm_LoadLibraryA;
    hMod = pfnProc(pszModulePath);
  }
  else
  {
    // This function has not yet been hooked.
    hMod = ::LoadLibraryA(pszModulePath);
  }

  FixupModuleOnLoad(hMod, 0);
  return hMod;
}

//  ****************************************************************************
HMODULE WINAPI ApiHook::LoadLibraryW(
  PCWSTR pszModulePath
)
{
  typedef HMODULE (WINAPI *pfnLoadLibraryW)(PCWSTR);

  HMODULE hMod = NULL;
  if ((PROC)sm_LoadLibraryW)
  {
    pfnLoadLibraryW pfnProc = (pfnLoadLibraryW)(PROC)sm_LoadLibraryW;
    hMod = pfnProc(pszModulePath);
  }
  else
  {
    // This function has not yet been hooked.
    hMod = ::LoadLibraryW(pszModulePath);
  }

  FixupModuleOnLoad(hMod, 0);
  return hMod;
}

//  ****************************************************************************
HMODULE WINAPI ApiHook::LoadLibraryExA(
  PCSTR   pszModulePath,
  HANDLE  hFile,
  DWORD   flags
)
{
  typedef HMODULE (WINAPI *pfnLoadLibraryExA)(PCSTR, HANDLE, DWORD);

  HMODULE hMod = NULL;
  if ((PROC)sm_LoadLibraryExA)
  {
    pfnLoadLibraryExA pfnProc = (pfnLoadLibraryExA)(PROC)sm_LoadLibraryExA;
    hMod = pfnProc(pszModulePath, hFile, flags);
  }
  else
  {
    // This function has not yet been hooked.
    hMod = ::LoadLibraryExA(pszModulePath, hFile, flags);
  }

  FixupModuleOnLoad(hMod, flags);
  return hMod;
}

//  ****************************************************************************
HMODULE WINAPI ApiHook::LoadLibraryExW(
  PCWSTR  pszModulePath,
  HANDLE  hFile,
  DWORD   flags
)
{
  typedef HMODULE (WINAPI *pfnLoadLibraryExW)(PCWSTR, HANDLE, DWORD);

  HMODULE hMod = NULL;
  if ((PROC)sm_LoadLibraryExW)
  {
    pfnLoadLibraryExW pfnProc = (pfnLoadLibraryExW)(PROC)sm_LoadLibraryExW;
    hMod = pfnProc(pszModulePath, hFile, flags);
  }
  else
  {
    // This function has not yet been hooked.
    hMod = ::LoadLibraryExW(pszModulePath, hFile, flags);
  }

  FixupModuleOnLoad(hMod, flags);
  return hMod;
}

//  ****************************************************************************
FARPROC WINAPI ApiHook::GetProcAddress(
  HMODULE     hMod, 
  const char* pFnName
)
{
  // Get the true address of the function.
  FARPROC pfn = GetProcAddressRaw(hMod, pFnName);

  // Return the hook address if the requested function is hooked.
  ApiHookArray::iterator iter = sm_hooks.begin();
  ApiHookArray::iterator end  = sm_hooks.end();
  for (; iter != end; ++iter)
  {
    ApiHook* pHook = *iter;
    if (pfn == pHook->m_pfnOrig)
    {
      pfn = pHook->m_pfnHook;
      break;
    }
  }

  return pfn;
}
#endif

namespace // unnamed
{

//  ****************************************************************************
/// Determines which library module a requested address lives in.
///
/// @param pv        The address for the request.
/// @return          The HMODULE will be returned for the library that contains
///                  the requested address.  
///                  NULL is returned if an error occurs, or the address is 
///                  not valid.
///
HMODULE GetModuleFromAddress(
  PVOID pv
)
{
#ifdef WIN32
  MEMORY_BASIC_INFORMATION mbi;
  return  ::VirtualQuery(pv, &mbi, sizeof(mbi))
          ? HMODULE(mbi.AllocationBase)
          : NULL;
#else
  return NULL;
#endif
}

//  ****************************************************************************
/// Replaces the function pointer PROC. 
/// A new address is written at the specified original address.
///
/// @param ppfnOrig  The original address to replace.
/// @param pfnNew    The new address to write.
/// @return  true    The function succeeds, and the new address is written
///                  over the specified address.
/// @return false    The function could not replace the address.
///
bool ReplaceFunctionAddress(PROC* ppfnOrig, PROC pfnNew)
{
#ifdef WIN32
  if ( ::WriteProcessMemory(::GetCurrentProcess(),
                            ppfnOrig,
                            &pfnNew,
                            sizeof(PROC),
                            NULL
                          ))
  {
    return true;
  }

  // If this fails due to write-protection, 
  // disable write protection, and try again.
  if (ERROR_NOACCESS != ::GetLastError())
  {
    return false;
  }

  DWORD curProtect = 0;
  if (::VirtualProtect( ppfnOrig, 
                        sizeof(PROC), 
                        PAGE_WRITECOPY, 
                        &curProtect))
  {
    ::WriteProcessMemory( ::GetCurrentProcess(),
                          ppfnOrig,
                          &pfnNew,
                          sizeof(PROC),
                          NULL
                        );
    // Restore the original protection at this address.
    ::VirtualProtect(ppfnOrig, sizeof(PROC), curProtect, &curProtect);  

    return true;
  }

#else
# error "Platform Requires implementation."
#endif

  return false;
}

#ifdef WIN32
//  ****************************************************************************
/// Structured Exception Handler for Win32 ReadException.
///
/// @param pep       Pointer to the exception details.
/// @return          
/// 
LONG WINAPI InvalidReadExceptionFilter(
  PEXCEPTION_POINTERS pep
)
{
  // All unexpected exceptions are handled because no module is updated 
  // in this case.
  LONG disposition = EXCEPTION_EXECUTE_HANDLER;
  return disposition;
}
#endif

} // namespace unnamed

