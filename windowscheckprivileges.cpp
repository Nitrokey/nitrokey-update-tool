#include "windowscheckprivileges.h"

WindowsCheckPrivileges::WindowsCheckPrivileges()
{}

#ifdef Q_OS_WIN
#include <Windows.h>
bool WindowsCheckPrivileges::IsElevated( ) {
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if( OpenProcessToken( GetCurrentProcess( ),TOKEN_QUERY,&hToken ) ) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof( TOKEN_ELEVATION );
        if( GetTokenInformation( hToken, TokenElevation, &Elevation, sizeof( Elevation ), &cbSize ) ) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if( hToken ) {
        CloseHandle( hToken );
    }
    return fRet;
}
#else
bool WindowsCheckPrivileges::IsElevated( ) {return false;}
#endif
