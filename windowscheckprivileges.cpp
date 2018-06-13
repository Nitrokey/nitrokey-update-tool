#include "windowscheckprivileges.h"

WindowsCheckPrivileges::WindowsCheckPrivileges()
{}

#if defined(__MINGW32__) or defined(WIN32) or defined(Q_OS_WIN)
#include <windows.h>
bool WindowsCheckPrivileges::IsElevated( ) {
    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    b = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup);
    if(b)
    {
        if (!CheckTokenMembership( NULL, AdministratorsGroup, &b))
        {
             b = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    return(b);
}

#else
bool WindowsCheckPrivileges::IsElevated( ) {return false;}
#endif
