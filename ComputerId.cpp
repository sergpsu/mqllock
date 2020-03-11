#include "stdafx.h"
#include "md5.h"
#include "defs.h"
#include "secure.h"

char mycid[ 33 ];
char mycid2old[ 33 ];
char mycid2[ 33 ];
extern HBRUSH g_dlgBgBrush;

// ==============
// GetComputerID
// ==============
const char* GetComputerID()
{
	if( !*mycid )
	{
		BYTE *cid = new BYTE[ 10240 ];
		if( !cid )
		{
			return( 0 );
		}
		DWORD len = 0;

		// system language
		LCID lcid = GetSystemDefaultLCID();
		memcpy( cid, &lcid, sizeof( LCID ) );
		len += sizeof( LCID );
 
		//OS version
		DWORD dw = GetVersion();
		memcpy( cid + len, &dw, sizeof( DWORD ) );
		len += sizeof( DWORD );

		//windows directory
		len += GetWindowsDirectoryA( ( char* )( cid + len ), 1024 );

		//system directory
		len += GetSystemDirectoryA( ( char* )( cid + len ), 1024 );

		//processor
		SYSTEM_INFO si;
		GetSystemInfo( &si );
		
		memcpy( cid + len, &si.wProcessorArchitecture, sizeof( si.wProcessorArchitecture ) );	//architecture
		len += sizeof( si.wProcessorArchitecture );
		
		memcpy( cid + len, &si.dwNumberOfProcessors, sizeof( si.dwNumberOfProcessors ) );	//number of processors
		len += sizeof( si.dwNumberOfProcessors );
		
		memcpy( cid + len, &si.dwProcessorType, sizeof( si.dwProcessorType  ) );	//processor type
		len += sizeof( si.dwProcessorType );

		//volume C info
		char vname[ 1024 ];
		GetVolumeInformationA( "C:\\", vname, 1024, &dw, 0, 0, 0, 0 );
		
		memcpy( cid + len, vname, lstrlenA( vname ) );	//volume name
		len += lstrlenA( vname );
		
		memcpy( cid + len, &dw, sizeof( DWORD ) ); // serial number
		len += sizeof( DWORD );

		MD5_CTX ctx;
		MD5Init( &ctx );
		MD5Update( &ctx, cid, len );
		MD5Final( &ctx );

		delete[] cid;

		for( int i = 0; i < 16; i ++ )
		{
			dw = ctx.digest[ i ];
			wsprintfA( mycid + i * 2, "%02X", dw );
		}
		mycid[ 32 ] = 0;
	}
	return( mycid );
} // GetComputerID


// ==============
// GetComputerID2old
// ==============
const char* GetComputerID2old( const char* regkey )
{
	if( !*mycid2old )
	{
		HKEY hkey;
		if( RegCreateKeyEx( HKEY_CURRENT_USER, regkey, 0, 0, 0, KEY_READ | KEY_WRITE, 0, &hkey, 0 ) == 0 )
		{
			DWORD rnd;
			
			DWORD sz = sizeof( DWORD );
			LRESULT lres = RegQueryValueEx( hkey, "t", 0, 0, ( BYTE* )&rnd, &sz );
			if( lres != 0 )
			{
				rnd = GetTickCount();
				lres = RegSetValueEx( hkey, "t", 0, REG_DWORD, ( BYTE* )&rnd, sizeof( DWORD ) );
			}
			RegCloseKey( hkey );
			if( lres == 0 )
			{

				BYTE *cid = ( BYTE* )malloc( 10240 );
				if( cid )
				{
					DWORD len = 0;

					// system language
					LCID lcid = GetSystemDefaultLCID();
					memcpy( cid, &lcid, sizeof( LCID ) );
					len += sizeof( LCID );

					//OS version
					DWORD dw = GetVersion();
					memcpy( cid + len, &dw, sizeof( DWORD ) );
					len += sizeof( DWORD );

					//windows directory
					len += GetWindowsDirectoryA( ( char* )( cid + len ), 1024 );

					//system directory
					len += GetSystemDirectoryA( ( char* )( cid + len ), 1024 );

					//processor
					SYSTEM_INFO si;
					GetSystemInfo( &si );
					
					memcpy( cid + len, &si.wProcessorArchitecture, sizeof( si.wProcessorArchitecture ) );	//architecture
					len += sizeof( si.wProcessorArchitecture );
					
					memcpy( cid + len, &si.dwNumberOfProcessors, sizeof( si.dwNumberOfProcessors ) );	//number of processors
					len += sizeof( si.dwNumberOfProcessors );
					
					memcpy( cid + len, &si.dwProcessorType, sizeof( si.dwProcessorType  ) );	//processor type
					len += sizeof( si.dwProcessorType );

					//volume C info
					char vname[ 1024 ];
					GetVolumeInformationA( "C:\\", vname, 1024, &dw, 0, 0, 0, 0 );
					
					memcpy( cid + len, vname, lstrlenA( vname ) );	//volume name
					len += lstrlenA( vname );
					
					memcpy( cid + len, &dw, sizeof( DWORD ) ); // serial number
					len += sizeof( DWORD );

					//const random value
					memcpy( cid + len, &rnd, sizeof( DWORD ) );
					len += sizeof( DWORD );

					MD5_CTX ctx;
					MD5Init( &ctx );
					MD5Update( &ctx, cid, len );
					MD5Final( &ctx );

					free( cid );

					for( int i = 0; i < 16; i ++ )
					{
						dw = ctx.digest[ i ];
						wsprintfA( mycid2old + i * 2, "%02X", dw );
					}
					mycid2[ 32 ] = 0;
				}
			}
		}
	}
	return( mycid2 );
} // GetComputerID2old

// ==============
// GetComputerID2
// ==============
const char* GetComputerID2()
{
	if( !*mycid2 )
	{
		HKEY hkey;
		if( RegCreateKeyExA( HKEY_CURRENT_USER, "Software\\Fx1", 0, 0, 0, KEY_READ | KEY_WRITE, 0, &hkey, 0 ) != 0 )
		{
			return( 0 );
		}
		DWORD rnd;
		
		DWORD sz = sizeof( DWORD );
		LRESULT lres = RegQueryValueExA( hkey, "t", 0, 0, ( BYTE* )&rnd, &sz );
		if( lres != 0 )
		{
			rnd = GetTickCount();
			lres = RegSetValueExA( hkey, "t", 0, REG_DWORD, ( BYTE* )&rnd, sizeof( DWORD ) );
		}
		RegCloseKey( hkey );
		if( lres != 0 )
		{
			return( 0 );
		}

		BYTE *cid = ( BYTE* )malloc( 10240 );
		if( !cid )
		{
			return( 0 );
		}
		DWORD len = 0;

		// system language
		LCID lcid = GetSystemDefaultLCID();
		memcpy( cid, &lcid, sizeof( LCID ) );
		len += sizeof( LCID );

		//OS version
		DWORD dw = GetVersion();
		memcpy( cid + len, &dw, sizeof( DWORD ) );
		len += sizeof( DWORD );

		//windows directory
		len += GetWindowsDirectoryA( ( char* )( cid + len ), 1024 );

		//system directory
		len += GetSystemDirectoryA( ( char* )( cid + len ), 1024 );

		//processor
		SYSTEM_INFO si;
		GetSystemInfo( &si );
		
		memcpy( cid + len, &si.wProcessorArchitecture, sizeof( si.wProcessorArchitecture ) );	//architecture
		len += sizeof( si.wProcessorArchitecture );
		
		memcpy( cid + len, &si.dwNumberOfProcessors, sizeof( si.dwNumberOfProcessors ) );	//number of processors
		len += sizeof( si.dwNumberOfProcessors );
		
		memcpy( cid + len, &si.dwProcessorType, sizeof( si.dwProcessorType  ) );	//processor type
		len += sizeof( si.dwProcessorType );

		//volume C info
		char vname[ 1024 ];
		GetVolumeInformationA( "C:\\", vname, 1024, &dw, 0, 0, 0, 0 );
		
		memcpy( cid + len, vname, lstrlenA( vname ) );	//volume name
		len += lstrlenA( vname );
		
		memcpy( cid + len, &dw, sizeof( DWORD ) ); // serial number
		len += sizeof( DWORD );

		//const random value
		memcpy( cid + len, &rnd, sizeof( DWORD ) );
		len += sizeof( DWORD );

		MD5_CTX ctx;
		MD5Init( &ctx );
		MD5Update( &ctx, cid, len );
		MD5Final( &ctx );

		free( cid );

		for( int i = 0; i < 16; i ++ )
		{
			dw = ctx.digest[ i ];
			wsprintfA( mycid2 + i * 2, "%02X", dw );
		}
		mycid2[ 32 ] = 0;
	}
	return( mycid2 );
} // GetComputerID2
