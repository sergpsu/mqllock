#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "api.h"
#include "md5.h"
#include "resource.h"
#include "CBaseServerManager.h"
#include "CIndicator.h"
#include "xmlparser.h"
#include "CSizer.h"
#include "CXor.h"
#include "crc32.h"
#include "obfuscators.h"
#include <lmcons.h>
#include <shlobj.h>
#include "CCaptcha.h"

#ifdef VMP_PROTECTION
#include "VMProtectSDK.h"
#endif

#pragma warning( disable: 4996 )

#ifdef MLL
#include "constants.h"
#endif

#include <stdlib.h>
#include <shellapi.h>
#include <time.h>
#include <commctrl.h>
#include <windns.h>
#ifdef CODESIGN
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>
#endif

#pragma comment( lib, "shell32.lib" )
#pragma comment( lib, "Iphlpapi.lib" )
#pragma comment( lib, "dnsapi.lib" )
#pragma comment( lib, "ws2_32.lib" )
#ifdef CODESIGN
#pragma comment( lib, "wintrust" )
#endif

CAtlArray<CChartData*> g_charts;
CAtlArray<LINK_HANDLE> g_links;

HWND g_hwndNavTree, g_hwndNav, g_dlgUpdate;
WNDPROC g_origProcNav;
DWORD g_instances;

char g_projectName[ 100 ];	//project name can be changed by vendor ( see lic documentation ) so we will fetch project name from lic after loading it. By default use GEN_PROJECT

Globals *g_g;

char g_dbgPath[ MAX_PATH ];
DWORD g_dbgTls;

#ifdef CPPDEBUG


CSimpleArray<DWORD> g_threads;
#endif

#if defined GEN_LT_REMOTE || defined GEN_LT_LOCAL_LOGIN || defined GEN_LT_LOCAL_RECEIPT
// Please authenticate your %s license
BYTE g_authenticateTitleKey[4] = {0x9f,0x50,0xb4,0x4a};
BYTE g_authenticateTitleStr[36]={0x73,0x70,0x87,0xcd,0x38,0xf8,0x28,0x3b,0xf0,0x65,0x81,0x3d,0x66,0xd6,0x5f,0x29,0x24,0x40,0xc1,0xfb,0xcd,0x3c,0x29,0xf4,0x21,0xe4,0x1d,0x72,0xb8,0x56,0x60,0xc4,0xb7,0x32,0xe1,0x4e};

// You need to authenticate your license to use this product. Please enter the %s from your qualifying purchase before the timer expires
BYTE g_authenticateBodyKey[4] = {0xb4,0x71,0x19,0xc9};
BYTE g_authenticateBodyStr[134]={0x0a,0x93,0xad,0x9d,0x48,0xad,0xb8,0x27,0xac,0xfa,0x28,0x5d,0xf2,0x5e,0x14,0x1b,0x95,0x89,0xb6,0x00,0xe9,0x0f,0xe3,0xce,0x24,0x25,0xd7,0x65,0x97,0xf2,0xa3,0xd3,0x2d,0xdf,0x23,0x56,0x22,0x10,0xf0,0x59,0x10,0x44,0x6f,0x58,0x6f,0xbf,0xc6,0x0b,0x25,0x01,0x49,0x1d,0x1c,0x6b,0x53,0xed,0x8f,0x9e,0x2a,0x35,0xbb,0xce,0x67,0xc2,0x32,0x74,0x63,0x8c,0xf4,0x19,0xf4,0x9b,0xff,0xc1,0xec,0x77,0x13,0xbd,0xa6,0x6c,0x19,0x63,0x55,0x61,0xc1,0x26,0xc7,0x38,0xf3,0x8c,0x72,0x6a,0x17,0xa1,0x9f,0x7b,0x96,0xa4,0x1e,0xb5,0x63,0xb6,0x6c,0x64,0x2c,0xf6,0x00,0x67,0x04,0x3c,0x2d,0xe5,0x88,0x39,0x59,0x7a,0xd7,0x64,0x72,0x7b,0xd9,0x03,0x4f,0x21,0x2a,0x54,0x78,0x67,0x66,0xe2,0x14,0x6e,0x57,0xe7};

#endif

// Authentication failed. Please enter valid %s to be able to continue.
BYTE g_authenticationFailedBodyKey[4] = {0x92,0x22,0x90,0xba};
BYTE g_authenticationFailedBodyStr[69]={0xe1,0xcf,0x7e,0xfc,0x8c,0xb8,0x1d,0x7f,0xf1,0x7d,0x04,0xfa,0x6c,0x35,0x60,0xf5,0xc4,0xb7,0x2b,0xdc,0x21,0x61,0x2f,0xda,0xae,0x2c,0x08,0x65,0xde,0x97,0x82,0xad,0x49,0x1b,0x5a,0x0a,0xf4,0x9b,0x5e,0x20,0xbf,0x4d,0xeb,0x6d,0xc7,0x1e,0x80,0xd9,0x1f,0x01,0xbd,0x4f,0x34,0x96,0x98,0x1f,0xa3,0x6c,0x58,0x41,0x89,0x54,0xb8,0xc6,0xa8,0x42,0x33,0xf6,0x71};

// %s - License not validated
BYTE g_authenticationFailedTitleKey[4] = {0x57,0x0f,0x26,0x56};
BYTE g_authenticationFailedTitleStr[27]={0xfb,0x3f,0x21,0xb8,0xbc,0xdd,0xd1,0xc4,0xb2,0xc4,0x19,0xe7,0x58,0xa1,0xee,0xf1,0xed,0x42,0x53,0xbc,0xfd,0x61,0x4b,0x9d,0x7a,0x46,0xcc};


#ifndef MLL
// ==================
// threadLoadRegData
// ==================
DWORD WINAPI threadLoadRegData( CRegistrationData* rd )
{
	DWORD res;

	TRY

	CBaseServerManager mgr;
	char *uri = ( char* )mymalloc( 256 );

	// %s?p=register_soft&name=%s&email=%s
	BYTE key[4] = {0xd0,0xb2,0xb9,0x19};
	BYTE fmt[36]={0x0e,0x91,0xd3,0xf4,0xbd,0x67,0x3a,0xae,0xc8,0xf8,0x8a,0x64,0xeb,0x0c,0x19,0xbe,0x64,0x32,0x7c,0x7e,0xce,0x06,0xd5,0x9c,0x4a,0x40,0x3f,0xad,0x22,0xb8,0xb4,0xf7,0x30,0xa2,0x68,0x34};
	DECR( fmt, key );
	wsprintfA( uri, ( char* )fmt, rd->baseUri, rd->name, rd->email );
	ENCR( fmt, key );

	CBaseServerManagerHandle *h = mgr.load( rd->server, rd->port, uri, FALSE );
	free( uri );

	res = h->wait();

	if( res != WAIT_OBJECT_0 )
	{
		h->error = ML_UNKNOWN;
	}
	else if( h->error == ML_OK )
	{
		DWORD sz;
		InternetReadFile( h->hr, rd->code, sizeof( rd->code ), &sz );
		if( sz != REGISTER_CODE_LENGTH )
		{
			h->error = ML_READ_FILE;
		}
	}

	res = h->error;
	delete h;
	
	CATCH
	return( res );
} // threadLoadRegData

// =============
// dlgRegister1
// =============
INT_PTR CALLBACK dlgRegister1( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static CRegistrationData* rd;
	static HANDLE threadLoad;
	switch( msg )
	{
		case WM_INITDIALOG:
			rd = ( CRegistrationData* )lp;
			return( TRUE );

		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDOK:
				{
					GetDlgItemTextA( dlg, IDE_EMAIL, rd->email, sizeof( rd->email ) );
					GetDlgItemTextA( dlg, IDE_NAME, rd->name, sizeof( rd->name ) );

					if( !isValidEmail( rd->email ) )
					{
						// Input valid email please
						BYTE key[4] = {0x72,0x26,0x65,0x19};
						BYTE str[25]={0x51,0xa6,0x60,0x64,0x36,0xe3,0x00,0xd0,0x3e,0x5b,0x19,0x8a,0xea,0xb5,0x2b,0x8d,0x8f,0x3b,0x62,0xd0,0xd4,0x23,0xdd,0xd9,0xac};
						DECR( str, key );

						// Error
						BYTE errKey[4] = {0x52,0xfa,0x29,0x2a};
						BYTE errStr[6]={0xea,0x20,0xa3,0x9f,0x16,0x9f};
						{
							DECR( errStr, errKey );
						}
						MessageBoxA( dlg, ( char* )str, ( char* )errStr, MB_ICONERROR );
						ENCR( errStr, errKey );
						ENCR( str, key );
						break;
					}
					if( !lstrlenA( rd->name ) )
					{
						// Input your fullname please
						BYTE key[4] = {0x0c,0xb2,0xf4,0x3b};
						BYTE str[27]={0x12,0xb8,0x6e,0xcc,0x61,0x6e,0xf9,0x1f,0x0b,0x3b,0x7f,0x21,0x99,0x1a,0xf6,0xe6,0x22,0x78,0x97,0x07,0xa0,0xc6,0xa0,0xf5,0xa9,0x16,0x04};
						DECR( str, key );
						// Error
						BYTE errKey[4] = {0xd4,0x7a,0x56,0x3e};
						BYTE errStr[6]={0x46,0x8c,0xf9,0xc3,0xe7,0x9e};
						{
							DECR( errStr, errKey );
						}
						MessageBoxA( dlg, ( char* )str, ( char* )errStr, MB_ICONERROR );
						ENCR( errStr, errKey );
						ENCR( str, key );
						break;
					}

					EnableWindow( GetDlgItem( dlg, IDOK ), FALSE );
					EnableWindow( GetDlgItem( dlg, IDCANCEL ), FALSE );
					CIndicator::start( dlg, IDC_PROGRESS1 );

					SetTimer( dlg, 1, 100, 0 );
					threadLoad = myCreateThread( ( LPTHREAD_START_ROUTINE )threadLoadRegData, rd, "tLRD" );
				}
					break;


				case IDCANCEL:
					EndDialog( dlg, 0 );
					break;

			}
			break;

		case WM_TIMER:
			if( WaitForSingleObject( threadLoad, 0 ) == 0 )
			{
				MLError res;
				GetExitCodeThread( threadLoad, ( DWORD* )&res );
				CloseHandle( threadLoad );
				KillTimer( dlg, 1 );

				CIndicator::stop( dlg, IDC_PROGRESS1 );	
				EnableWindow( GetDlgItem( dlg, IDOK ), TRUE );
				EnableWindow( GetDlgItem( dlg, IDCANCEL ), TRUE );

				switch( res )
				{
					case ML_OK:
						EndDialog( dlg, 1 );
						break;

					default:
						// Failed receive registration code
						BYTE key[4] = {0xd1,0xcc,0xb8,0x3d};
						BYTE str[33]={0xb3,0xe0,0x73,0x41,0x84,0xf6,0xb1,0x73,0xb8,0x94,0x26,0x67,0x1c,0x8e,0xdb,0xec,0xfd,0x42,0x9a,0x34,0x03,0xd6,0x02,0x10,0x20,0x32,0xc0,0x9f,0xbb,0xf2,0x9c,0xbc,0x91};

						// Error
						BYTE errKey[4] = {0x2e,0x07,0x4f,0x16};
						BYTE errStr[6]={0x6c,0xed,0xd1,0x20,0x28,0xb9};

						DECR( str, key );
						{
							DECR( errStr, errKey );
						}
						MessageBoxA( dlg, ( char* )str, ( char* )errStr, MB_ICONERROR );
						ENCR( errStr, errKey );
						ENCR( str, key );
						break;
				}
			}
			break;
	}
	return( 0 );
} // dlgRegister1

// =============
// dlgRegister2
// =============
INT_PTR CALLBACK dlgRegister2( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static CRegistrationData* rd;
	switch( msg )
	{
		case WM_INITDIALOG:
			rd = ( CRegistrationData* )lp;
			return( TRUE );

		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDOK:
				{
					char code[ REGISTER_CODE_LENGTH + 1 ];
					if( GetDlgItemTextA( dlg, IDE_CODE, code, sizeof( code ) ) == REGISTER_CODE_LENGTH &&
						!memcmp( code, rd->code, REGISTER_CODE_LENGTH ) )
					{
						EndDialog( dlg, 1 );
					}
					else
					{
						// Invalid register code
						BYTE key[4] = {0x4b,0xa6,0xfc,0x4b};
						BYTE str[22]={0x3c,0x63,0x6e,0x43,0xfd,0x6c,0x1d,0x85,0xdc,0x62,0x78,0x4d,0x4c,0x14,0x70,0xdc,0xdf,0xe6,0xe5,0xac,0x62,0x52};
						DECR( str, key );

						// Error
						BYTE errKey[4] = {0xde,0xa0,0xc0,0xc4};
						BYTE errStr[6]={0x60,0x9a,0xe3,0x41,0x49,0xc8};
						{
							DECR( errStr, errKey );
						}
						
						MessageBoxA( dlg, ( char* )str, ( char* )errStr, MB_ICONERROR );
						ENCR( errStr, errKey );
						ENCR( str, key );
					}
					break;
				}

				case IDCANCEL:
					EndDialog( dlg, 0 );
					break;
			}
			break;
	}
	return( 0 );
} // dlgRegister2
#else
// ===============
// initGlobals
// ===============
void initGlobals()
{
	Globals* g = new Globals();
	
	//g->setFlag( FLG_AUTH_OK, 1 );
	g_g = ( Globals* )( ( DWORD )g ^ GLOBALS_KEY );
} // initGlobals

void clearGlobals()
{
	Globals* g = getGlobals();
	g->waitGuardianThread( 5000 );	//todo: this should be called separately
	delete g;
}

// ==================
// getGlobals
// ==================
Globals* getGlobals()
{
	return( ( Globals* )( ( DWORD )g_g ^ GLOBALS_KEY ) );
} // getGlobals

#endif // !defined( MLL )

void delSelf()
{
	char *modulename = ( char* )mymalloc( MAX_PATH );
	char *batfile = ( char* )mymalloc( MAX_PATH );
	char *batlines = ( char* )mymalloc( MAX_PATH * 4 );

	GetModuleFileNameA( g_hInstance, modulename, MAX_PATH );

	char* tmp = ( char* )mymalloc( MAX_PATH );
	GetTempPathA( MAX_PATH, tmp );

	{
	/*%s\ml%d.bat*/
	BYTE key[4]={0xed,0xa7,0xee,0x66};
	BYTE str[12]={0x8d,0x47,0x44,0xba,0xc3,0x16,0x07,0x72,0x57,0xee,0x19,0x29};
	DECR( str, key );
	wsprintfA( batfile, ( char* )str, tmp, GetTickCount() );
	ENCR( str, key );
	}

	{
	/*@echo off
	:try
	del */
	BYTE key[4]={0x8c,0x0f,0x87,0x4c};
	BYTE str[20]={0xdf,0x50,0xa4,0xa7,0x9b,0x6e,0x76,0xb0,0xe1,0x59,0xcf,0x2d,0x7c,0x61,0x6e,0xe6,0x3e,0xab,0x6b,0xb9};
	DECR( str, key );
	strcpy_s( batlines, MAX_PATH * 4, ( char* )str );
	ENCR( str, key );
	}

	strcat_s( batlines, MAX_PATH * 4, modulename );

	{
	/*
	if exist */
	BYTE key[4]={0x7d,0x05,0x63,0x19};
	BYTE str[11]={0xc8,0x85,0x2d,0x45,0xc8,0x1e,0x0f,0xba,0x3b,0xef,0xa1};
	DECR( str, key );
	strcat_s( batlines, MAX_PATH * 4, ( char*)str );
	ENCR( str, key );
	}
	
	strcat_s( batlines, MAX_PATH * 4, modulename );
	
	{
	/* goto try
*/
	BYTE key[4]={0x4a,0x0a,0xa3,0xb8};
	BYTE str[11]={0xa4,0x40,0x21,0x32,0x86,0xdf,0x9a,0x7a,0x01,0xd2,0xcc};
	DECR( str, key );
	strcat_s( batlines, MAX_PATH * 4, ( char* )str );
	ENCR( str, key );
	}
	{
	/*del */
	BYTE key[4]={0xe3,0x61,0x63,0x98};
	BYTE str[5]={0x46,0x73,0xfb,0x26,0xb1};
	DECR( str, key );
	strcat_s( batlines, MAX_PATH * 4, ( char* )str );
	ENCR( str, key );
	}
	strcat_s( batlines, MAX_PATH * 4, batfile );
	DWORD NOfBytes;
	HANDLE hbf = CreateFileA( batfile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ	| FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile( hbf, batlines, strlen( batlines ), &NOfBytes, NULL );
	CloseHandle( hbf );
	STARTUPINFOA         si;
	PROCESS_INFORMATION pi;
	memset( &si, 0, sizeof( si ) );
	si.cb = sizeof( si );
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;
	if( !CreateProcessA(
		 NULL,
		 batfile,
		 NULL,
		 NULL,
		 FALSE,
		 IDLE_PRIORITY_CLASS | DETACHED_PROCESS,
		 NULL,
		 tmp,
		 &si,
		 &pi) )
	{
		dbg( "!!- fcp %d", GetLastError() );
	}
	free( batfile );
	free( modulename );
	free( batlines );
	free( tmp );
} // delSelf

// ==============
// threadGuardian
// ==============
DWORD WINAPI threadGuardian( LPVOID )
{
	TRY
dbg( "tg" );	

	Globals* g = getGlobals();

	ECS( g_hCharts );
	int n = g_charts.GetCount();
	for( int i = 0; i < n; i ++ )
	{
		BOOL isSet;
		HANDLE prop = g_charts[ i ]->getProp( &isSet );
		if( isSet )
		{
			if( !prop )
			{
				myTerminateProcess( "prop1" );
			}
			DWORD pid = ( DWORD )LOWORD( prop );
			if( pid != g->getProjectId() )
			{
				myTerminateProcess( "prop2" );
			}
			if( HIWORD( prop ) == 0 )	//antihack - hiword contains instances of this ea/indicators for this chart and this value can not be 0
			{
				myTerminateProcess( "prop3" );
			}
		}
	}
	LCS( g_hCharts );


	if( WaitForSingleObject( g->hThreadGuardianStop, rand() % 8192 + 16384 ) != 0 )			//8-16 seconds
	{
		verifyAuth();

		if( WaitForSingleObject( g->hThreadGuardianStop, rand() % 65536 + 32768  ) != 0 )	//+32-65 seconds
		{
#ifdef VMP_PROTECTION
			checkDebuggerPresent(); // disabled, cns vps error: restarted terminal did not came up
									//01.02.2017 - enabled again
#endif
			if( WaitForSingleObject( g->hThreadGuardianStop, rand() % 65536 + 32768 ) != 0 )//+32-65 seconds
			{
#ifdef VMP_PROTECTION
				checkImageCRC(); // disabled, cns vps error: restarted terminal did not came up
								//01.02.2017 - enabled again
#endif
				/*ECS( g_hLic );
				DWORD maxInstances = g->getAutolicenseMaxInstances();
				if( maxInstances )
				{
					DWORD usedInstances = getInstancesCount();
					if( usedInstances > maxInstances )
					{
						closeCharts();
						myTerminateProcess( "inst" );
					}
				}
				LCS( g_hLic );*/

#ifndef MLL
				// ----------------------------
				// validating flags checksum
				// ----------------------------
				verifyGlobalsChecksum();

				// -----------------------------
				// dns verification
				// -----------------------------
				HWND pseudoChart;
				__asm mov pseudoChart, esp;
	
				MathSrand( pseudoChart, GetTickCount() );

				// verifying dns record for our ext_id
#ifndef CPPDEBUG
				if( isTimePassed( g->getLastDNSVerification(), 1000 * 60 * 60 * ( rand() % 3 + 3 ) ) )	//+3-6hours
#else
				if( isTimePassed( g->getLastDNSVerification(), 1000 * 60 * ( rand() % 2 + 1 ) ) )
#endif
				{
dbg( "dns" );
					g->setLastDNSVerification( GetTickCount() );

					char* q = ( char* )mymalloc( 128 );
					const char* extId = g->useId();
					/*%s.mqllock.com*/
					BYTE key[4]={0x25,0x6f,0xad,0x93};
					BYTE str[15]={0x9d,0xec,0x57,0x2e,0x66,0x19,0x74,0xb6,0x9d,0x72,0xec,0xaf,0x63,0x94,0x45};
					DECR( str, key );
					wsprintfA( q, ( const char* )str, extId );
					ENCR( str, key );
					g->unuse( extId );
//TODO: test with UNICODE!!!
					DNS_RECORDA *res;
					DNS_STATUS st = DnsQuery_A( q, DNS_TYPE_A, DNS_QUERY_NO_LOCAL_NAME | DNS_QUERY_NO_HOSTS_FILE | DNS_QUERY_BYPASS_CACHE, 0, ( PDNS_RECORD* )&res, 0 );
dbg( "st %d", st );
					if( st == 0 )
					{
						DNS_RECORDA* p = res;
						while( p )
						{
							if( p->wType == DNS_TYPE_A && strcmpi( q, p->pName ) == 0 )
							{
dbg( "%s %08x", p->pName, p->Data.A.IpAddress );
								if( p->Data.A.IpAddress != 0x0100007f /*127.0.0.1*/ )
								{
									if( p->Data.A.IpAddress == 0x0200007f /*127.0.0.2*/ )
									{
										char* me = ( char* )mymalloc( MAX_PATH );
										GetModuleFileNameA( g_hInstance, me, MAX_PATH );

										char* name = ( char* )mymalloc( MAX_PATH );

										char* slash = strrchr( me, '\\' );
dbg( "sl1 %s", slash );
										strcpy_s( name, MAX_PATH, slash + 1 );

										slash[ 0 ] = 0;
										slash = strrchr( me, '\\' );
dbg( "sl2 %s", slash );
										slash[ 1 ] = 0;
										if( !g->getFlag( FLG_EA ) )
										{
											/*indicators\*/
											BYTE key[4]={0x9b,0x3e,0x17,0xb1};
											BYTE str[12]={0xc1,0xaa,0xe3,0x32,0xf4,0xf9,0x31,0x0d,0x87,0xea,0x68,0x52};
											DECR( str, key );
											strcat( slash, ( char* )str );
											ENCR( str, key );
										}

										const char* ex4Name = g->useEx4Name();
										if( *ex4Name )
										{
											strcat( slash, ex4Name );
											g->unuse( ex4Name );
										}
										else
										{
											strcat( slash, name );
										}
dbg( "me %s", me );
										int len = strlen( me );
										{/*.ex4*/
										BYTE key[4]={0x2a,0x9f,0x7f,0x93};
										BYTE str[5]={0x44,0x01,0x8e,0xb6,0x55};
										DECR( str, key );
										strcpy( me + len, ( char* )str );
										ENCR( str, key );
										}
dbg( "dle '%s'", me );
										DeleteFileA( me );//deleting ex4
										
										{/*.mq4*/
										BYTE key[4]={0xe8,0xcf,0xd9,0x6b};
										BYTE str[5]={0x11,0xb3,0x5c,0xe3,0x74};
										DECR( str, key );
										strcpy( me + len, ( char* )str );
										ENCR( str, key );
										}
dbg( "dlm '%s'", me );
										DeleteFileA( me );//deleting mq4

										free( me );

										delSelf();
									}
									closeCharts();	//will it work?
									myTerminateProcess( "dns" );
								}
							}
							p = p->pNext;
							
						}
						DnsRecordListFree( res, DnsFreeRecordList );
					}
					free( q );
				}
#endif //ifndef MLL
			}
			else
			{
				dbg( "htg es %d", 2 );
			}
		}
		else
		{
			dbg( "htg es %d", 1 );
		}
	}
	else
	{
		dbg( "htg es %d", 4 );
	}
	CATCH
dbg( "tgdn" );
	return( 0 );
} // threadGuardian

// ================
// isValidEmail
// ================
BOOL isValidEmail( const char* email )
{
	char *p = ( char* )email;
	BOOL first = TRUE;
	while( *p && *p != '@' && ( isalpha( *p ) || *p == '_' || *p == '.' || ( !first && ( *p == '-' || isdigit( *p ) ) ) ) )
	{
		first = FALSE;
		p ++;
	}
	if( *p != '@' )
	{
		return( FALSE );
	}

	p ++;
	first = TRUE;
	while( *p && *p != '.' && ( isalpha( *p ) || *p == '_' || ( !first && ( *p == '-' || isdigit( *p ) ) ) ) )
	{
		first = FALSE;
		p ++;
	}
	if( *p != '.' )
	{
		return( FALSE );
	}
	return( TRUE );
} // isValidEmail

// ======================
// isValidDlgPassword
// ======================
BOOL isValidDlgPassword( HWND dlg, DWORD idePassword1, OPTIONAL DWORD idePassword2, OUT char* pass, DWORD nPass )
{
	BOOL res = FALSE;
	char pass2[ 128 ];
	int n = GetDlgItemTextA( dlg, idePassword1, pass, nPass );
	if( n >= MIN_PASSWORD_LENGTH )
	{
		if( idePassword2 )
		{
			GetDlgItemTextA( dlg, IDE_PASSWORD2, pass2, sizeof( pass2 ) );
			if( strcmp( pass, pass2 ) == 0 )
			{
				res = TRUE;
			}
			else
			{
				showError( E_PASSWORDS_MISMATCH, dlg );
			}
		}
		else
		{
			res = TRUE;
		}
	}
	else
	{
		showError( E_NOPASSWORD, dlg );
	}
	return( res );
} // isValidDlgPassword

// ====================
// MyGetVersion
// ====================
DWORD MyGetVersion()
{
	DWORD res = 0;
	static HMODULE ntdll;
	static PROC RtlGetVersion;
	if( !ntdll )
	{
		ntdll = GetModuleHandleA( "ntdll" );
	}
	if( ntdll )
	{
		if( !RtlGetVersion )
		{
			RtlGetVersion = GetProcAddress( ntdll, "RtlGetVersion" );
		}
		if( RtlGetVersion )
		{
			RTL_OSVERSIONINFOW oi;
			oi.dwOSVersionInfoSize = sizeof( oi );
			__asm
			{
				lea eax, oi
				push eax
				call RtlGetVersion;
			}
			res = oi.dwMajorVersion + ( oi.dwMinorVersion << 8 ) + ( oi.dwBuildNumber << 16 );
		}
	}
	return( res );
} // MyGetVersion


// ==============
// GetComputerData
// ==============
DWORD GetComputerData( OUT BYTE** _cid )
{
		HKEY hkey;
#ifdef CPPDEBUG
		char user[ UNLEN + 1 ];
		DWORD n = UNLEN + 1;
		GetUserNameA( user, &n );
		dbg( "User '%s'", user );
#endif
		if( RegCreateKeyExA( HKEY_CURRENT_USER, "Software\\Fx1", 0, 0, 0, KEY_READ | KEY_WRITE, 0, &hkey, 0 ) != 0 )
		{
dbg( "!!- Failed open fx1 registry key (%d)", GetLastError() );
			return( 0 );
		}
		DWORD rnd;
		
		DWORD sz = sizeof( DWORD );
		LRESULT lres = RegQueryValueExA( hkey, "t", 0, 0, ( BYTE* )&rnd, &sz );
		if( lres != 0 )
		{
			rnd = GetTickCount();
			lres = RegSetValueExA( hkey, "t", 0, REG_DWORD, ( BYTE* )&rnd, sizeof( DWORD ) );
dbg( "Random value not found, generated %08x", rnd );
		}
		else
		{
dbg( "Random value=%08x", rnd );
		}

		RegCloseKey( hkey );
		if( lres != 0 )
		{
dbg( "!!- Failed save random value into registry (%d)", GetLastError() );
			return( 0 );
		}

		BYTE *cid = ( BYTE* )mymalloc( 10240 );
		if( !cid )
		{
dbg( "Not enough memory" );
			return( 0 );
		}
		DWORD len = 0;

		// system language
		LCID lcid = GetSystemDefaultLCID();
		memcpy( cid, &lcid, sizeof( LCID ) );
		len += sizeof( LCID );
dbg( "LCID=%x, size=%d", lcid, sizeof( LCID ) );
		
		//OS version
		DWORD dw = MyGetVersion();
		memcpy( cid + len, &dw, sizeof( DWORD ) );
		len += sizeof( DWORD );
dbg( "OS version=%x, size=%d", dw, sizeof( DWORD ) );

		//windows directory
		dw = GetWindowsDirectoryA( ( char* )( cid + len ), 1024 );
dbg( "Windows directory='%s', length=%d", ( char* )( cid + len ), dw );
		len += dw;

		//system directory
		dw = GetSystemDirectoryA( ( char* )( cid + len ), 1024 );
dbg( "System directory='%s', length=%d", ( char* )( cid + len ), dw );
		len += dw;

		//processor
		SYSTEM_INFO si;
		GetSystemInfo( &si );
		
		memcpy( cid + len, &si.wProcessorArchitecture, sizeof( si.wProcessorArchitecture ) );	//architecture
		len += sizeof( si.wProcessorArchitecture );
dw = si.wProcessorArchitecture;
dbg( "Processor architecture=%u, size=%d", dw, sizeof( si.wProcessorArchitecture ) );

		memcpy( cid + len, &si.dwNumberOfProcessors, sizeof( si.dwNumberOfProcessors ) );	//number of processors
		len += sizeof( si.dwNumberOfProcessors );
dw = si.dwNumberOfProcessors;
dbg( "Number of processors=%u, size=%d", dw, sizeof( si.dwNumberOfProcessors ) );
		
		memcpy( cid + len, &si.dwProcessorType, sizeof( si.dwProcessorType  ) );	//processor type
		len += sizeof( si.dwProcessorType );
dw = si.dwProcessorType;
dbg( "Processor type=%u, size=%d", dw, sizeof( si.dwProcessorType ) );

		//volume C info
		char vname[ 1024 ];
		GetVolumeInformationA( "C:\\", vname, 1024, &dw, 0, 0, 0, 0 );
		
		memcpy( cid + len, vname, lstrlenA( vname ) );	//volume name
		len += lstrlenA( vname );
dbg( "Volume C name='%s', length=%d", vname, lstrlenA( vname ) );

		memcpy( cid + len, &dw, sizeof( DWORD ) ); // serial number
		len += sizeof( DWORD );
dbg( "Volume C number=%u, length=%d", dw, sizeof( DWORD ) );

		//const random value
		memcpy( cid + len, &rnd, sizeof( DWORD ) );
		len += sizeof( DWORD );

	*_cid = cid;
	return( len );
} // GetComputerData
char mycid2[ 33 ];
const char* GetComputerID2()
{
	if( !*mycid2 )
	{
		DWORD len;
		BYTE* cid;
		len = GetComputerData( &cid );
		if( len )
		{
			getMD5( cid, len, mycid2 );
			free( cid );
		}
dbg( "cid2=%s", mycid2 );
	}
	return( mycid2 );
} // GetComputerID2

const BYTE* GetComputerID2Raw()
{
	static BYTE mycid2raw[ 16 ];
	static BOOL got;
	if( !got )
	{
		got = 1;
		DWORD len;
		BYTE* cid;
		len = GetComputerData( &cid );
		if( len )
		{
			getMD5Raw( cid, len, mycid2raw );
			free( cid );
		}
	}
	return( mycid2raw );
} // GetComputerID2Raw


// ==============
// getMD5
// ==============
void getMD5( BYTE* input, DWORD len, OUT char* str )
{
	MD5_CTX ctx;
	MD5Init( &ctx );
	MD5Update( &ctx, input, len );
	MD5Final( &ctx );

	for( int i = 0; i < 16; i ++ )
	{
		DWORD dw = ctx.digest[ i ];
		wsprintfA( str + i * 2, "%02X", dw );
	}
	str[ 32 ] = 0;
} // getMD5

// ==============
// getMDRaw
// ==============
void getMD5Raw( BYTE* input, DWORD len, OUT BYTE* out )
{
	MD5_CTX ctx;
	MD5Init( &ctx );
	MD5Update( &ctx, input, len );
	MD5Final( &ctx );

	memcpy( out, ctx.digest, 16 );
} // getMD5Raw

// ==============
// parseUrl
// =============
BOOL parseUrl( const char* url, OUT char* server, OUT INTERNET_PORT* port, OUT char* baseUri, OUT char* scheme )
{
	URL_COMPONENTSA uc;
	char _server[ MAX_SERVER_LENGTH + 1 ];
	char uri[ MAX_URI_LENGTH + 1 ];
	char sch[ MAX_SCHEME_LENGTH + 1 ];
	memset( &uc, 0, sizeof( uc ) );
	uc.dwStructSize = sizeof( uc );
	uc.lpszHostName = _server;
	uc.dwHostNameLength = sizeof( _server );
	uc.lpszUrlPath = uri;
	uc.dwUrlPathLength = sizeof( uri );
	uc.lpszScheme = sch;
	uc.dwSchemeLength = sizeof( sch );

	if( InternetCrackUrlA( url, 0, ICU_DECODE, &uc ) )
	{
		if( server )
		{
			lstrcpyA( server, _server );
		}
		if( port )
		{
			*port = uc.nPort;
		}
		if( baseUri )
		{
			lstrcpyA( baseUri, uri );
		}
		if( scheme )
		{
			lstrcpyA( scheme, sch );
		}
		return( TRUE );
	}
	return( FALSE );
} // parseUrl

// ==============
// isMt4Window
// ==============
BOOL isMt4Window( HWND hwnd )
{
	char cls[ 30 ];
	GetClassNameA( hwnd, cls, 30 );

	int r = strcmp( cls, "MetaQuotes::MetaTrader::4.00" );

	return( r == 0 );
} // isMt4Window

// =================
// isChartWindow
// =================
BOOL isChartWindow( HWND wnd )
{
	char cls[ 30 ];
	GetClassNameA( wnd, cls, 30 );
	if( strstr( cls, "AfxFrameOrView" ) == cls &&
		isMt4Window( GetParent( GetParent( GetParent( wnd ) ) ) )
	)
	{
		return( TRUE );
	}
	return( FALSE );
} // isChartWindow

// ===================
// findMt4Window
// ===================
BOOL CALLBACK findMt4Window( HWND hwnd, LPARAM reqPid )
{
	BOOL res = TRUE;
	if( isMt4Window( hwnd ) )
	{
		DWORD pid;
		GetWindowThreadProcessId( hwnd, &pid );

		if( pid == *( DWORD* )reqPid )
		{
			//on return reqPid contains handle to window
			*( HWND* )reqPid = hwnd;
			res = FALSE; //mt4 window for required process found
		}
	}
	return( res );
} // findMt4Window

// ==================
// getMt4Window
// ==================
HWND getMt4Window()
{
	static HWND mt4;
	if( !mt4 )
	{
		DWORD pid = GetCurrentProcessId();
		HWND wnd = ( HWND )pid;
dbg( "getMt4Window" );
		BOOL e = EnumWindows( findMt4Window, ( LPARAM )&wnd );
dbg( "enum %d (%d)", e, GetLastError() );
		if( wnd != ( HWND )pid )
		{
			mt4 = wnd;
		}
	}
	return( mt4 );
} // getMt4Window

#ifdef UNICODE
// ==============
// openLink
// ===============
BOOL openLink( const wchar_t* link )
{
	SHELLEXECUTEINFOW sei;
	::ZeroMemory( &sei,sizeof( SHELLEXECUTEINFOW ) );
	sei.cbSize = sizeof( SHELLEXECUTEINFOW );		// Set Size
	sei.lpVerb = L"open";							// Set Verb
	sei.lpFile = link;								// Set Target To Open
	sei.nShow = SW_SHOWNORMAL;						// Show Normal

	return( ShellExecuteExW( &sei ) && sei.hInstApp > ( HINSTANCE )32 );
} // openLink
#endif
// ==============
// openLinkA
// ===============
BOOL openLinkA( const char* link )
{
	SHELLEXECUTEINFOA sei;
	::ZeroMemory( &sei,sizeof( SHELLEXECUTEINFOA ) );
	sei.cbSize = sizeof( SHELLEXECUTEINFOA );		// Set Size
	sei.lpVerb = "open";							// Set Verb
	sei.lpFile = link;								// Set Target To Open
	sei.nShow = SW_SHOWNORMAL;						// Show Normal

	return( ShellExecuteExA( &sei ) && sei.hInstApp > ( HINSTANCE )32 );
} // openLink


// ===============
// closeCharts
// ===============
void closeCharts(/* BOOL wait */)
{
	dbg( "cc" );
	// closing charts if license insufficient
	int n = g_charts.GetCount();
	for( int i = 0; i < n; i ++ )
	{
		if( !g_charts[ i ]->closed )
		{
dbg( "clch %08x", GetParent( g_charts[ i ]->hChart ) );
			//if( !wait )
			{
				g_charts[ i ]->closed = PostMessage( GetParent( g_charts[ i ]->hChart ), WM_CLOSE, 0, 0 );
				g_charts[ i ]->closeTime = GetTickCount();
			}
			/*else
			{
				g_charts[ i ]->closed = SendMessage( GetParent( g_charts[ i ]->hChart ), WM_CLOSE, 0, 0 );
			}*/

			if( !g_charts[ i ]->closed )
			{
				dbg( "!!- fclch %d", GetLastError() );
				myTerminateProcess( "chcls" );
			}
		}
		else
		{
			//tried to close once but failed -> something wrong( hacked? )
			if( IsWindow( g_charts[ i ]->hChart ) && isTimePassed( g_charts[ i ]->closeTime, 5000 ) )
			{
				myTerminateProcess( "chcls2" );
			}
		}
	}
} // closeCharts

#ifdef CODESIGN
BOOL verifyMySignature( HMODULE hm )
{
	wchar_t mypath[ MAX_PATH ];
	GetModuleFileNameW( hm, mypath, MAX_PATH );
	return( VerifyEmbeddedSignature( mypath ) );
}

BOOL VerifyEmbeddedSignature( LPCWSTR pwszSourceFile )
{
	return( TRUE );//26.05.2015, getting TRUST_E_CERT_SIGNATURE on some pc

    LONG lStatus;
//    DWORD dwLastError;

    // Initialize the WINTRUST_FILE_INFO structure.

    WINTRUST_FILE_INFO FileData;
    memset(&FileData, 0, sizeof(FileData));
    FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    FileData.pcwszFilePath = pwszSourceFile;
    FileData.hFile = NULL;
    FileData.pgKnownSubject = NULL;

    /*
    WVTPolicyGUID specifies the policy to apply on the file
    WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:
    
    1) The certificate used to sign the file chains up to a root 
    certificate located in the trusted root certificate store. This 
    implies that the identity of the publisher has been verified by 
    a certification authority.
    
    2) In cases where user interface is displayed (which this example
    does not do), WinVerifyTrust will check for whether the  
    end entity certificate is stored in the trusted publisher store,  
    implying that the user trusts content from this publisher.
    
    3) The end entity certificate has sufficient permission to sign 
    code, as indicated by the presence of a code signing EKU or no 
    EKU.
    */

    GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA WinTrustData;

    // Initialize the WinVerifyTrust input data structure.

    // Default all fields to 0.
    memset(&WinTrustData, 0, sizeof(WinTrustData));

    WinTrustData.cbStruct = sizeof(WinTrustData);
    
    // Use default code signing EKU.
    WinTrustData.pPolicyCallbackData = NULL;

    // No data to pass to SIP.
    WinTrustData.pSIPClientData = NULL;

    // Disable WVT UI.
    WinTrustData.dwUIChoice = WTD_UI_NONE;

    // No revocation checking.
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 

    // Verify an embedded signature on a file.
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

    // Default verification.
    WinTrustData.dwStateAction = 0;

    // Not applicable for default verification of embedded signature.
    WinTrustData.hWVTStateData = NULL;

    // Not used.
    WinTrustData.pwszURLReference = NULL;

    // This is not applicable if there is no UI because it changes 
    // the UI to accommodate running applications instead of 
    // installing applications.
    WinTrustData.dwUIContext = 0;

    // Set pFile.
    WinTrustData.pFile = &FileData;

    // WinVerifyTrust verifies signatures as specified by the GUID 
    // and Wintrust_Data.
    lStatus = WinVerifyTrust(
        NULL,
        &WVTPolicyGUID,
        &WinTrustData);

    switch (lStatus) 
    {
        case ERROR_SUCCESS:
            /*
            Signed file:
                - Hash that represents the subject is trusted.

                - Trusted publisher without any verification errors.

                - UI was disabled in dwUIChoice. No publisher or 
                    time stamp chain errors.

                - UI was enabled in dwUIChoice and the user clicked 
                    "Yes" when asked to install and run the signed 
                    subject.
            */
            //wprintf_s(L"The file \"%s\" is signed and the signature "
              //  L"was verified.\n",
//                pwszSourceFile);
            return( TRUE );
        
        case TRUST_E_NOSIGNATURE:
            // The file was not signed or had a signature 
            // that was not valid.

            // Get the reason for no signature.
            /*dwLastError = GetLastError();
            if (TRUST_E_NOSIGNATURE == dwLastError ||
                    TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError ||
                    TRUST_E_PROVIDER_UNKNOWN == dwLastError) 
            {
                // The file was not signed.
               wprintf_s(L"The file \"%s\" is not signed.\n",
                    pwszSourceFile);

            } 
            else 
            {
                // The signature was not valid or there was an error 
                // opening the file.
                wprintf_s(L"An unknown error occurred trying to "
                    L"verify the signature of the \"%s\" file.\n",
                    pwszSourceFile);
            }*/

            break;

        case TRUST_E_EXPLICIT_DISTRUST:
            // The hash that represents the subject or the publisher 
            // is not allowed by the admin or user.
            /*wprintf_s(L"The signature is present, but specifically "
                L"disallowed.\n");*/
            break;

        case TRUST_E_SUBJECT_NOT_TRUSTED:
            // The user clicked "No" when asked to install and run.
            //wprintf_s(L"The signature is present, but not "
                //L"trusted.\n");
            break;

        case CRYPT_E_SECURITY_SETTINGS:
            /*
            The hash that represents the subject or the publisher 
            was not explicitly trusted by the admin and the 
            admin policy has disabled user trust. No signature, 
            publisher or time stamp errors.
            */
/*            wprintf_s(L"CRYPT_E_SECURITY_SETTINGS - The hash "
                L"representing the subject or the publisher wasn't "
                L"explicitly trusted by the admin and admin policy "
                L"has disabled user trust. No signature, publisher "
                L"or timestamp errors.\n");
				*/
            break;

        default:
            // The UI was disabled in dwUIChoice or the admin policy 
            // has disabled user trust. lStatus contains the 
            // publisher or time stamp chain error.
           /* wprintf_s(L"Error is: 0x%x.\n",
                lStatus);*/
            break;
    }

    return( FALSE );
} // VerifyEmbeddedSignature
#endif //CODESIGN

void dbg2( const char* s )
{
//return;
	FILE* f = fopen( "1.txt", "a" );
	if( f )
	{
		fputs( s, f );
		fputs( "\n", f );
		fclose( f );
	}
}

#ifdef CPPDEBUG

// ===============
// dbg
// ===============
void dbg( wchar_t* _fmt, ... )
{
	if( !*g_dbgPath ) return;
	HANDLE h = INVALID_HANDLE_VALUE;

	try{ 
		
		_se_translator_function seOld = _set_se_translator( exceptionPreFilter );

	//if( WaitForSingleObject( g_hDbg, INFINITE ) == 0 )
	{
dbg2("L2");
		SYSTEMTIME st;
		GetLocalTime( &st );
dbg2("3");
		va_list l;
		va_start( l, _fmt );

dbg2("4");
		char* path = ( char* )TlsGetValue( g_dbgTls );
		if( GetLastError() == ERROR_SUCCESS )
		{
			if( !path )
			{
				path = ( char* )mymalloc( MAX_PATH );
				wsprintfA( path, "%s\\%08x.log", g_dbgPath, GetCurrentThreadId() );
				TlsSetValue( g_dbgTls, path );
			}
			h = CreateFileA( path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0 );
	dbg2("5");
			if( h != INVALID_HANDLE_VALUE )
			{
				SetFilePointer( h, 0, 0, FILE_END );

				int len = wcslen( _fmt ) + 128;
				wchar_t *fmt = ( wchar_t* )mymalloc( len * sizeof( wchar_t ) );
				swprintf_s( fmt, len, L"%04x\t%04x\t%02d.%02d.%d %02d:%02d:%02d.%03d\t", GetCurrentProcessId(), GetCurrentThreadId(), ( DWORD )st.wDay, ( DWORD )st.wMonth, ( DWORD )st.wYear, ( DWORD )st.wHour, ( DWORD )st.wMinute, ( DWORD )st.wSecond, ( DWORD )st.wMilliseconds );
				wcscat_s( fmt, len, _fmt );
				wcscat_s( fmt, len, L"\n" );

	dbg2("6");
				len = _vscwprintf( fmt, l ) + 1;			
	dbg2("7");
				wchar_t* line = ( wchar_t* )mymalloc( len * sizeof( wchar_t ) );
	dbg2("8");
				int n = vswprintf_s( line, len, fmt, l );
				free( fmt );
	dbg2("9");
				DWORD sz;
	dbg2("10");
				int wn = n * sizeof( wchar_t );
	#ifdef ENC_DBG
				WriteFile( h, &wn, sizeof( int ), &sz, 0 );		
	dbg2("7");
				Globals* g = getGlobals();
	dbg2("8");
				const char* gid = g->useId();

				CRC4 *rc4 = new CRC4();
				rc4->Encrypt( ( BYTE* )line, wn, ( const BYTE* )gid, GEN_EXT_ID_LENGTH );
				delete rc4;
	dbg2("9");
				g->unuse( gid );
	dbg2("10");
	#endif
				WriteFile( h, line, wn, &sz, 0 );
	dbg2("11");
				CloseHandle( h );
				free( line );
	dbg2("12");
			}
			else
			{
	//	MessageBox( 0, fmt, "Failed open", 0 );
			}
		}
		va_end( l );
dbg2("13");
		//ReleaseMutex( g_hDbg );
//dbg2("14");
	}
	//else
//	{
//		MessageBox( 0, _fmt, "Failed wait", 0 );
//	}
		_set_se_translator( seOld );
	}
	catch( CMyException *e )
	{
		if( h != INVALID_HANDLE_VALUE )
		{
			CloseHandle( h );
		}
		exceptionFilter( __FUNCTION__, e );
	} 
}

void dbg( char* _fmt, ... )
{
	if( !*g_dbgPath ) return;
	HANDLE h = INVALID_HANDLE_VALUE;

	try{ 
		
		_se_translator_function seOld = _set_se_translator( exceptionPreFilter );

dbg2( _fmt );
	//if( WaitForSingleObject( g_hDbg, INFINITE ) == 0 )
	{
dbg2("2");
		SYSTEMTIME st;
		GetLocalTime( &st );
dbg2("3");
		va_list l;
		va_start( l, _fmt );

dbg2("4");
		char* path = ( char* )TlsGetValue( g_dbgTls );
		if( !path )
		{
			path = ( char* )mymalloc( MAX_PATH );
			wsprintfA( path, "%s\\%08x.log", g_dbgPath, GetCurrentThreadId() );
			TlsSetValue( g_dbgTls, path );
		}
		h = CreateFileA( path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0 );
dbg2("5");
		if( h != INVALID_HANDLE_VALUE )
		{
			SetFilePointer( h, 0, 0, FILE_END );

			int len = strlen( _fmt ) + 128;
			char *fmt = ( char* )mymalloc( len );
			sprintf_s( fmt, len, "%04x\t%04x\t%02d.%02d.%d %02d:%02d:%02d.%03d\t", GetCurrentProcessId(), GetCurrentThreadId(), ( DWORD )st.wDay, ( DWORD )st.wMonth, ( DWORD )st.wYear, ( DWORD )st.wHour, ( DWORD )st.wMinute, ( DWORD )st.wSecond, ( DWORD )st.wMilliseconds );
			strcat_s( fmt, len, _fmt );
			strcat_s( fmt, len, "\n" );

dbg2("6");
			len = _vscprintf( fmt, l ) + 1;			
dbg2("7");
			char* line = ( char* )mymalloc( len );
dbg2("8");
			int n = vsprintf_s( line, len, fmt, l );
			free( fmt );
dbg2("9");
			DWORD sz;
dbg2("10");
#ifdef ENC_DBG
			WriteFile( h, &n, sizeof( int ), &sz, 0 );		
dbg2("7");
			Globals* g = getGlobals();
dbg2("8");
			const char* gid = g->useId();

			CRC4 *rc4 = new CRC4();
			rc4->Encrypt( ( BYTE* )line, n, ( const BYTE* )gid, GEN_EXT_ID_LENGTH );
			delete rc4;
dbg2("9");
			g->unuse( gid );
dbg2("10");
#endif
			WriteFile( h, line, n, &sz, 0 );
dbg2("11");
			CloseHandle( h );
			free( line );
dbg2("12");
		}
		else
		{
//	MessageBox( 0, fmt, "Failed open", 0 );
		}

		va_end( l );
dbg2("13");
		//ReleaseMutex( g_hDbg );
//dbg2("14");
	}
	//else
//	{
//		MessageBox( 0, _fmt, "Failed wait", 0 );
//	}
		_set_se_translator( seOld );
	}
	catch( CMyException *e )
	{
		if( h != INVALID_HANDLE_VALUE )
		{
			CloseHandle( h );
		}
		exceptionFilter( __FUNCTION__, e );
	} 
}
#endif	//CPPDEBUG

// ==================
// fdbg
// force dbg even for non debug version
// ==================
void fdbg( char* _fmt, ... )
{
dbg2("fdbg");
	if( !*g_dbgPath )
	{
		dbg2("!ex");
		return;
	}
	HANDLE h = INVALID_HANDLE_VALUE;

		SYSTEMTIME st;
		GetLocalTime( &st );
		va_list l;
		va_start( l, _fmt );

dbg2("1");
		char path[ MAX_PATH ];
#ifndef CPPDEBUG
		static int inited;
		if( !inited )
		{
			inited = 1;
dbg2("2");
			//debug version creates log path at once, else we have to do this here
			CreateDirectoryA( g_dbgPath, 0 );
dbg2("3");
			SYSTEMTIME now;
			GetSystemTime( &now );

			wsprintfA( g_dbgPath, "%s\\%d-%02d-%02d-%02d-%02d-%02d\\", g_dbgPath, ( DWORD )now.wYear, ( DWORD )now.wMonth,
								( DWORD )now.wDay, ( DWORD )now.wHour, ( DWORD )now.wMinute, ( DWORD )now.wSecond );
dbg2("4");
		//DBG( "dbgpath '%s'", g_dbgPath );
		}
		CreateDirectoryA( g_dbgPath, 0 );	//it's removed after fdbgdone() cleanup
dbg2("5");
#endif
dbg2("6");
		wsprintfA( path, "%s\\f-%08x.log", g_dbgPath, GetCurrentThreadId() );
dbg2(path);		
		h = CreateFileA( path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
dbg2("7");
		if( h != INVALID_HANDLE_VALUE )
		{
			SetFilePointer( h, 0, 0, FILE_END );
dbg2("8");
			int len = strlen( _fmt ) + 128;
dbg2("81" );
			char *fmt = ( char* )mymalloc( len );
dbg2(fmt);
			sprintf_s( fmt, len, "%04x\t%04x\t%02d.%02d.%d %02d:%02d:%02d.%03d\t", GetCurrentProcessId(), GetCurrentThreadId(), ( DWORD )st.wDay, ( DWORD )st.wMonth, ( DWORD )st.wYear, ( DWORD )st.wHour, ( DWORD )st.wMinute, ( DWORD )st.wSecond, ( DWORD )st.wMilliseconds );
dbg2("9");
			strcat_s( fmt, len, _fmt );
dbg2("10");
			strcat_s( fmt, len, "\n" );
dbg2(fmt);
			len = _vscprintf( fmt, l ) + 1;			
dbg2("11");
			char* line = ( char* )mymalloc( len );

			int n = vsprintf_s( line, len, fmt, l );
dbg2("12");
			free( fmt );
dbg2("13");
			DWORD sz;

			WriteFile( h, &n, sizeof( int ), &sz, 0 );		
dbg2("14");
			Globals* g = getGlobals();
dbg2("15");
			const char* gid = g->useId();
dbg2("16");
			CRC4 *rc4 = new CRC4();
			rc4->Encrypt( ( BYTE* )line, n, ( const BYTE* )gid, GEN_EXT_ID_LENGTH );
dbg2("17");
			delete rc4;
dbg2("18");
			g->unuse( gid );
dbg2("19");
			WriteFile( h, line, n, &sz, 0 );
dbg2("20");
			CloseHandle( h );
dbg2("21");
			free( line );
dbg2("22");

		}
		else
		{
//	MessageBox( 0, fmt, "Failed open", 0 );
		}

		va_end( l );
dbg2("23");
}// fdbg

// ==================
// fdbgdone
// ==================
void fdbgdone( BOOL res )
{
	// called from ThreadRemoteAuth2
	// if auth successfull then remove all fdbg-generated files and try to remove directory
	if( res )
	{
		//delete all f-* files
		char path[ MAX_PATH ];
		wsprintfA( path, "%s\\f-*.log", g_dbgPath );
dbg( path );

			WIN32_FIND_DATAA fd;
			HANDLE hf = FindFirstFileA( path, &fd );
			if( hf != INVALID_HANDLE_VALUE )
			{
				do
				{
					wsprintfA( path, "%s%s", g_dbgPath, fd.cFileName );
dbg( path);
					DeleteFileA( path );
				}
				while( FindNextFileA( hf, &fd ) );
				FindClose( hf );
			}

		//delete directory itself. Will be deleted if empty ( == not CPPDEBUG version, this directory is not needed )
		RemoveDirectoryA( g_dbgPath );

	}
} // fdbgdone

// ==============
// TimerProc
// ==============
void CALLBACK TimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{

//dbg( "TimerProc: timer=%x, time=%d", idEvent, GetTickCount() );
	int n = timers.GetSize();
	for( int i = 0; i < n; i ++ )
	{
		if( timers[ i ]->timer == idEvent )
		{
//dbg( "Found linked chart: %x", timers[ i ]->hChart );
			//send tick
			sendTick( timers[ i ]->hChart );
/*#ifdef _DEBUG
			printf( "tick to: %x, timer=%u\n",  timers[ i ]->hChart, GetTickCount() );
#endif*/
		}
	}
} // TimerProc

BOOL CALLBACK findTerminalWnd( HWND hwnd, LPARAM lp )
{
	if( GetWindowLong( hwnd, GWL_ID ) == 0x81b9 )
	{
		*( HWND* )lp = hwnd;
		return( FALSE );
	}
	return( TRUE );
}

CAccounts::CAccounts()
{
	m_cur = -1;
	m_mt4Ver = MT4_VER_UNDEFINED;
	invalidateCache();
	m_mutex = CreateMutex( 0, 0, 0 );
}

CAccounts::~CAccounts()
{
	CloseHandle( m_mutex );
}

void CAccounts::invalidateCache()
{
	m_lastScan = 0;
}

// =================
// fillAccountsList
// =================
BOOL CAccounts::fillAccountsList()
{
dbg( "fillAccountsList" );
	if( m_cur != -1 && !isTimePassed( m_lastScan, 10000 ) )
	{
dbg( "lastscan" );
		return( TRUE );
	}
	BOOL res = FALSE;
	m_lastScan = GetTickCount();
	m_accounts.RemoveAll();

	scanMemory();

	if( m_accounts.GetSize() )
	{
dbg( "define current account" );
		m_cur = -1;
		HWND mt4 = getMt4Window();
		if( IsWindow( mt4 ) )
		{
dbg( "getting window text" );
			char *text = ( char* )mymalloc( 256 );
			//if( SendMessageA( mt4, WM_GETTEXT, 256, ( LPARAM )text ) )
			if( GetWindowTextA( mt4, text, 256 ) )
			{
dbg( "got text '%s'", text );
				char* p = strchr( text, ':' );
				if( p )
				{
					*p = 0;

					DWORD n = m_accounts.GetSize();
dbg( "has %d accounts", n );
					DWORD i;
					DWORD acc = atoi( text );
dbg( "cur account %u", acc );
					for( i = 0; i < n; i ++ )
					{
dbg( "account %d = %u", i, m_accounts[ i ].number );
						if( m_accounts[ i ].number == acc )
						{
dbg( "found cur account, %d", i );
							res = TRUE;
							m_cur = i;
							break;
						}
					}
				}
			}
			else
			{
				dbg( "!!- failed get window text %d", GetLastError() );
			}
			free( text );
		}
	}
	else
	{
		dbg( "no acc" );
	}
	return( res );
} // fillAccountsList

// ===================
// searchPattern
// ===================
DWORD CAccounts::searchPattern( LPVOID base, DWORD base_length, LPVOID search, DWORD search_length) //KMP
{
    __asm
    {
        mov eax,search_length
alloc:
        push 0
        sub eax,1
        jnz alloc

        mov edi,search
        mov edx,search_length 
        mov ecx,1
        xor esi,esi
build_table:
        mov al,byte ptr [edi+esi]
        cmp al,byte ptr [edi+ecx]
        sete al
        test esi,esi
        jz pre
        test al,al
        jnz pre
        mov esi,[esp+esi*4-4]
        jmp build_table
pre:
        test al,al
        jz write_table
        inc esi
write_table:
        mov [esp+ecx*4],esi

        inc ecx
        cmp ecx,edx
        jb build_table

        mov esi,base
        xor edx,edx
        mov ecx,edx
matcher:
        mov al,byte ptr [edi+ecx]
        cmp al,byte ptr [esi+edx]
        sete al
        test ecx,ecx
        jz match
        test al,al
        jnz match
        mov ecx, [esp+ecx*4-4]
        jmp matcher
match:
        test al,al
        jz pre2
        inc ecx
        cmp ecx,search_length
        je finish
pre2:
        inc edx
        cmp edx,base_length //search_length
        jb matcher
        mov edx,search_length
        dec edx
finish:
        mov ecx,search_length
        sub edx,ecx
        lea eax,[edx+1]
        lea ecx,[ecx*4]
        add esp,ecx
    }
} // searchPattern

// =====================
// isRealDemoOffset
// =====================
BOOL CAccounts::isRealDemoOffset( BYTE* block, BYTE* base, DWORD offset, DWORD size, OUT DWORD* number )
{
	BOOL res = FALSE;
	if( base + offset - block >= 0x1b8 )
	{
		BYTE* accountData = base + offset - 0x1b8;
		if( base + size - accountData >= 0x253 )
		{
			if(	*( DWORD* )( accountData + 0x24 ) && // account number
				__isascii( accountData[ 0x38 ] ) &&	// server name offset 
				__isascii( accountData[ 0x100 ] ) &&	// server ip:port
				__isascii( accountData[ 0x1d4 ] ) &&	// server official name offset
				isalpha( accountData[ 0x1b8 ] )		//"demo"/"real" 
				//__isascii( accountData[ 0xb8 ] ) // account user name - COMMENTED: non ascii letters possible ( russian )
			)
			{
				int j;
				//check server name zero tail - max len 128bytes
				for( j = 0x38 + strnlen( ( char* )accountData + 0x38, 128 ); j < 0x38 + 128; j ++ )
				{
					if( accountData[ j ] != 0 )
					{
						break;
					}
				}
				if( j == 0x38 + 128 )
				{
					//check account user name zero tail - max len 64bytes
					for( j = 0xb8 + strnlen( ( char* )accountData + 0xb8, 64 ); j < 0xb8 + 64; j ++ )
					{
						if( accountData[ j ] != 0 )
						{
							break;
						}
					}
					if( j == 0xb8 + 64 )	
					{
						//check server address
						for( j = 0x100; j < 0x100 + 128 && accountData[ j ] != 0; j ++ )
						{
							if( !__isascii( accountData[ j ] ) )
							{
								break;
							}
						}
						if( j == 0x100 + 128 || accountData[ j ] == 0 )
						{
							*number = *( DWORD* )( accountData + 0x24 );
							res = TRUE;
						}
					}
				}
			}
		}
	}
	return( res );
} // isRealDemoOffset

// =====================
// accountExists
// =====================
BOOL CAccounts::accountExists( DWORD account )
{
	int n = m_accounts.GetSize();
	for( int i = 0; i < n; i ++ )
	{
		if( m_accounts[ i ].number == account )
		{
			return( TRUE );
		}
	}
	return( FALSE );
} // accountExists

// ==================
// scanMemoryBlock
// ==================
void CAccounts::scanMemoryBlock( void *block, DWORD size )
{
	//scan for demo accounts
	BYTE* base = ( BYTE* )block;
	DWORD memSize = size;
	for( ;; )
	{
		DWORD sp = searchPattern( base, memSize, "demo", 5 );
		if( sp )
		{
	dbg( "found demo at %08x => %08x +%08x", block, base, sp );
			Account acc;
			if( isRealDemoOffset( ( BYTE* )block, base, sp, memSize, &acc.number ) )
			{
				if( !accountExists( acc.number ) )
				{
	dbg( "detected %u", acc.number );
					acc.real = FALSE;
					m_accounts.Add( acc );
				}
			}
#ifdef CPPDEBUG
			else
			{
	dbg( "not rd offset in %08x => %08x +%08x", block, base, sp );
				char name[ 100 ];
				wsprintfA( name, "%s\\mt4_%08x.log", g_dbgPath, block );
				HANDLE h = CreateFileA( name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
				DWORD sz;
				WriteFile( h, block, size, &sz, 0 );
				CloseHandle( h );
			}
#endif
			sp += 5;
			base += sp;
			memSize -= sp;
		}
		else
		{
			break;
		}
	}

	//scan for real accounts
	base = ( BYTE* )block;
	memSize = size;
	for( ;; )
	{
		DWORD sp = searchPattern( base, memSize, "real", 5 );
		if( sp )
		{
	dbg( "found real at %08x => %08x +%08x", block, base, sp );
			Account acc;
			if( isRealDemoOffset( ( BYTE* )block, base, sp, memSize, &acc.number ) )
			{
				if( !accountExists( acc.number ) )
				{
	dbg( "detected %u", acc.number );
					acc.real = TRUE;
					m_accounts.Add( acc );
				}
			}
#ifdef CPPDEBUG
			else
			{
	dbg( "not rd offset in %08x => %08x +%08x", block, base, sp );
				char name[ 100 ];
				wsprintfA( name, "%s\\mt4_%08x.log", g_dbgPath, block );
				HANDLE h = CreateFileA( name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
				DWORD sz;
				WriteFile( h, block, size, &sz, 0 );
				CloseHandle( h );
			}
#endif
			sp += 5;
			base += sp;
			memSize -= sp;
		}
		else
		{
			break;
		}
	}
} // scanMemoryBlock

// ======================
// scanMemory
// ======================
void CAccounts::scanMemory()
{
	MEMORY_BASIC_INFORMATION mbi;
	SYSTEM_INFO si;
	GetSystemInfo( &si );
#ifdef SCAN_MEM_TO_FILE
	TCHAR tmp[ MAX_PATH ];
	GetTempPath( sizeof( tmp ) / sizeof( TCHAR ), tmp );
#endif

	for( void* lpMem = 0; lpMem < si.lpMaximumApplicationAddress; )
	{
		VirtualQuery( lpMem, &mbi, sizeof( mbi ) );
		if( mbi.State == MEM_COMMIT && ( mbi.Protect & PAGE_GUARD ) == 0 && ( mbi.Protect & PAGE_READWRITE ) != 0 )
		{
#ifdef SCAN_MEM_TO_FILE
			TCHAR name[ 100 ];
			wsprintf( name, _T( "%s\\mt4_%08x.log" ), tmp, mbi.BaseAddress );
			HANDLE h = CreateFile( name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
			DWORD sz;
			WriteFile( h, mbi.BaseAddress, mbi.RegionSize, &sz, 0 );
			CloseHandle( h );
#endif
			scanMemoryBlock( mbi.BaseAddress, mbi.RegionSize );
		}
		lpMem = ( void* )( ( DWORD )mbi.BaseAddress + mbi.RegionSize );
	}

} // scanMemory

/*void CAccounts::scanMemory( DWORD chunkSize, DWORD offset )
{
dbg( "scm %d %d", chunkSize, offset );
	MEMORY_BASIC_INFORMATION mbi;
	SYSTEM_INFO si;
	GetSystemInfo( &si );
#ifdef SCAN_MEM_TO_FILE
	TCHAR tmp[ MAX_PATH ];
	GetTempPath( sizeof( tmp ) / sizeof( TCHAR ), tmp );
#endif

	for( void* lpMem = 0; lpMem < si.lpMaximumApplicationAddress; )
	{
		VirtualQuery( lpMem, &mbi, sizeof( mbi ) );
dbg( "mem=%08x base=%08x state=%08x prot=%08x", lpMem, mbi.BaseAddress, mbi.State, mbi.Protect );
		if( mbi.State == MEM_COMMIT && ( mbi.Protect & PAGE_GUARD ) == 0 && ( mbi.Protect & PAGE_READWRITE ) != 0 )
		{
#ifdef SCAN_MEM_TO_FILE
			TCHAR name[ 100 ];
			wsprintf( name, _T( "%s\\mt4_%08x.log" ), tmp, mbi.BaseAddress );
			HANDLE h = CreateFile( name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
			DWORD sz;
			WriteFile( h, mbi.BaseAddress, mbi.RegionSize, &sz, 0 );
			CloseHandle( h );
#endif
			BYTE* accountData = ( BYTE* )mbi.BaseAddress + offset;
			int i;
			for( i = 0; chunkSize * ( i + 1 ) < mbi.RegionSize &&
							*( DWORD* )( accountData + chunkSize * i + 0x24 ) && // account number
							isalpha( accountData[ chunkSize * i + 0x38 ] ) &&	// server .SRV file name offset (?)
							/*isalpha( accountData[ chunkSize * i + 0x1d4 ] ) &&	// server official name offset* /
							isalpha( accountData[ chunkSize * i + 0x1b8 ] ) &&		//"demo"/"real" (?)
							/*isalpha( accountData[ chunkSize * i + 0x254 ] ) &&	// server info* /
							accountData[ chunkSize * i + 0xb7 ] == 0 &&
							accountData[ chunkSize * i + 0xf7 ] == 0 && 
							accountData[ chunkSize * i + 0x1C7 ] == 0 &&
							accountData[ chunkSize * i + 0x253 ] == 0 
				;i ++ )
			{
				int j;
				for( j = 0x38 + strlen( ( char* )accountData + chunkSize * i + 0x38 ); j <= 0xb7; j ++ )
				{
					if( accountData[ chunkSize * i + j ] != 0 )
					{
						j = -1;
						break;
					}
				}
				if( j == -1 )
				{
					break;
				}
				Account acc;
				acc.number = *( DWORD* )( accountData + chunkSize * i + 0x24 );
dbg( "detected %u", acc.number );
				acc.real = ( *( DWORD* )( accountData + chunkSize * i + 0x1b8 ) == 0x6c616572 );
				m_accounts.Add( acc );
			}

			if( i > 0 )	//no need to scan other memory if at least one entry found
			{
/*				wsprintf( name, "%\\mt4.bin", tmp );
				HANDLE h = CreateFile( name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
				DWORD sz;
				WriteFile( h, mbi.BaseAddress, mbi.RegionSize, &sz, 0 );
				CloseHandle( h );
* / 
				break;
			}

		}
		lpMem = ( void* )( ( DWORD )mbi.BaseAddress + mbi.RegionSize );
	}

} // scanMemory*/

// ==================
// getNumber
// ==================
const char* CAccounts::getNumber( BOOL forceUseCache )
{
	static char res[ 16 ];
	ECS( m_mutex );
dbg( "getNumber %d", forceUseCache );
	if( forceUseCache || fillAccountsList() )
	{
dbg( "fal %d, ttl %d", m_cur, m_accounts.GetSize() );		
		if( m_cur != -1 )
		{
			itoa( m_accounts[ m_cur ].number, res, 10 );
		}
		else
		{
			*res = 0;
		}
	}
	LCS( m_mutex );
	return( res );
} // getNumber

// ==================
// getNumberI
// ==================
DWORD CAccounts::getNumberI( BOOL forceUseCache )
{
	DWORD res = 0;
	ECS( m_mutex );
dbg( "getNumberI %d", forceUseCache );
	if( forceUseCache || fillAccountsList() )
	{
dbg( "fal %d", m_cur );		
		if( m_cur != -1 )
		{
			res = m_accounts[ m_cur ].number;
		}
	}
	LCS( m_mutex );
	return( res );
} // getNumberI

// ================
// isReal
// ================
BOOL CAccounts::isReal( BOOL forceUseCache )
{
	static BOOL res;
	if( forceUseCache || fillAccountsList() )
	{
		if( m_cur != -1 )
		{
			res = m_accounts[ m_cur ].real;
		}
		else
		{
			res = FALSE;
		}
	}
	return( res );
} // isReal

#ifdef GEN_LT_REMOTE
// Validating (%d):
//BYTE g_validateKey[4] = {0x33,0xda,0xfc,0xe6};
//BYTE g_validateFmt[17]={0x28,0x9e,0x5c,0x85,0x8d,0x84,0xae,0xad,0xa9,0x56,0xb6,0x43,0x07,0x3d,0xfe,0x99,0x8a};

// Validate (%d)
BYTE g_validateKey[4] = {0x25,0x10,0x54,0x32};
BYTE g_validateFmt[14]={0xbe,0x54,0x1b,0xa6,0xd4,0x9f,0x17,0x6c,0xc6,0xa7,0x25,0x8c,0x74,0xa6};

// ==============================
// dlgAccountRegistrationConfirm
// ==============================
INT_PTR CALLBACK dlgAccountRegistrationConfirm( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static CAutolicense *al;
	static char* email;
	switch( msg )
	{
		case WM_INITDIALOG:
			email = ( char* )lp;
			al = new CAutolicense;
			SetFocus( GetDlgItem( dlg, IDE_CODE ) );
			SetTimer( dlg, 1, 300, 0 );

			centerDlgToScreen( dlg );
			return( FALSE );

		case WM_COMMAND:
		{
			switch( LOWORD( wp ) )
			{
				case IDOK:
				{
					char code[ 6 ];
					if( SendDlgItemMessage( dlg, IDE_CODE, WM_GETTEXTLENGTH, 0, 0 ) == 5 )
					{
						dlgEnable( dlg, FALSE, IDOK, IDE_CODE, 0 );

						GetDlgItemTextA( dlg, IDE_CODE, code, 6 );

						al->confirm( email, code );
					}
					else
					{
						/*Input confirmation code, 5 symbols*/
						BYTE key[4]={0x15,0xa3,0x56,0xd6};
						BYTE str[35]={0x0a,0xe9,0xe4,0xfa,0x03,0x3d,0xbf,0xfb,0x1b,0xbf,0xbe,0x1a,0xc7,0x53,0x6c,0xf7,0x34,0x19,0x9b,0x63,0x02,0x7f,0x71,0xc2,0xd6,0x3f,0x8b,0x81,0x1c,0x7a,0xe9,0x6d,0x02,0x56,0x87};

						/*Error*/
						BYTE key2[4]={0x91,0x1b,0x0d,0xbc};
						BYTE str2[6]={0xdd,0x64,0xa7,0x0b,0xce,0x52};

						DECR( str, key );
						DECR2( str2, key2 );
						MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );
						ENCR( str, key );
						ENCR( str2, key2 );
					}
					break;
				}

				case IDCANCEL:
					KillTimer( dlg, 1 );
					EndDialog( dlg, E_CANCELLED );
					break;
			}
			break;
		}

		case WM_TIMER:
			if( al->isProcessing() )
			{
				if( al->isReady() )
				{
					dlgEnable( dlg, TRUE, IDOK, IDE_CODE, 0 );
					SendDlgItemMessage( dlg, IDC_PROGRESS1, PBM_SETPOS, 0, 0 );

					int code = al->readResponse();
					switch( code )
					{
						case E_OK:
						{
							/*Thank you for registration. Now you can contact your vendor and ask him to add Your email into project configuration.*/
							BYTE key[4]={0x7f,0xf6,0x38,0xed};
							BYTE str[118]={0x3e,0x68,0x4c,0xc3,0x6d,0x57,0x76,0x87,0x69,0x99,0x2a,0xe3,0xd3,0x8a,0x1e,0x84,0xb2,0x86,0x75,0x8c,0x30,0xae,0x30,0xa3,0x17,0x53,0xae,0x22,0x8f,0x80,0xf9,0x7d,0x90,0x8e,0xad,0xc3,0x60,0x98,0xd0,0x6e,0xa9,0x73,0x8a,0x79,0xe3,0x56,0xba,0x4d,0xc7,0xe0,0x3e,0xe1,0xbf,0x0c,0xf0,0xc7,0x20,0x3b,0xaa,0xfc,0x31,0x35,0xeb,0xdb,0x3c,0xdf,0x49,0xf0,0x3d,0x8c,0xde,0x14,0x7c,0x7c,0x8c,0xe4,0x31,0xa6,0xa9,0x87,0x26,0x04,0x01,0x07,0xc5,0x28,0x2b,0xe8,0x4b,0x53,0xf7,0x35,0xb9,0xa4,0x9a,0x1a,0x3d,0x81,0x6f,0x94,0x32,0x0d,0x77,0xba,0xca,0xeb,0x2b,0x3f,0x23,0x7e,0x6c,0x25,0x51,0x22,0x39,0xe6,0xa2,0x4a};

							criticalError( E_WITHINFO, str, sizeof( str ), key, sizeof( key ), TRUE );

							KillTimer( dlg, 1 );
							EndDialog( dlg, E_OK );
							break;
						}

						case E_INVALID_CODE:
							showError( E_INVALID_CODE, dlg );
							break;

						default:
							showError( E_INVALID_SERVER_RESPONSE, dlg );
					}
				}
				else
				{
					SendDlgItemMessage( dlg, IDC_PROGRESS1, PBM_STEPIT, 0, 0 );
				}
			}
			break;

		case WM_DESTROY:
			delete al;
			break;
	}
	return( 0 );
} // dlgAccountRegistrationConfirm

// =======================
// dlgAccountRegistration
// =======================
INT_PTR CALLBACK dlgAccountRegistration( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static CCaptcha *captcha;	
	static CAutolicense *al;
	static char* outEmail;
	static HICON iconRefresh;
	switch( msg )
	{
		case WM_INITDIALOG:
			captcha = new CCaptcha( GetDlgItem( dlg, IDC_CAPTCHA ) );
			captcha->generate( 5 );

			iconRefresh = ( HICON )LoadImage( g_hInstance, MAKEINTRESOURCE( IDI_REFRESH ), IMAGE_ICON, 0, 0, 0 );
			SendDlgItemMessage( dlg, IDB_REFRESH, BM_SETIMAGE, IMAGE_ICON, ( LPARAM )iconRefresh );

			outEmail = ( char* )lp;
			al = new CAutolicense;
			SetFocus( GetDlgItem( dlg, IDE_LOGIN ) );
			SetTimer( dlg, 1, 300, 0 );

			centerDlgToScreen( dlg );
			return( FALSE );

		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDOK:
				{
					char email[ 128 ];
					char pass[ 128 ];
					GetDlgItemTextA( dlg, IDE_EMAIL, email, sizeof( email ) );
					if( isValidEmail( email ) )
					{
						if( isValidDlgPassword( dlg, IDE_PASSWORD, IDE_PASSWORD2, pass, sizeof( pass ) ) )
						{
							char text[ 6 ];
							GetDlgItemTextA( dlg, IDE_CAPTCHA, text, sizeof( text ) );
							if( strcmp( captcha->getCode(), text ) == 0 )
							{
								dlgEnable( dlg, FALSE, IDOK, IDE_EMAIL, IDE_PASSWORD, IDE_PASSWORD2, IDE_CAPTCHA, IDB_REFRESH, 0 );

								strcpy( outEmail, email );
								al->reg( email, pass );
							}
							else
							{
								showError( E_INVALID_CAPTCHA, dlg );
							}
						}
						//else error is shown from isValidDlgPassword
					}
					else
					{
						showError( E_INVALID_EMAIL, dlg );
					}
					break;
				}

				case IDCANCEL:
					KillTimer( dlg, 1 );
					EndDialog( dlg, E_CANCELLED );
					break;

				case IDB_REFRESH:
					captcha->generate( 5 );
					break;
			}
			break;

		case WM_TIMER:
			if( al->isProcessing() )
			{
				if( al->isReady() )
				{
	dbg( "alhwt" );
					dlgEnable( dlg, TRUE, IDOK, IDE_EMAIL, IDE_PASSWORD, IDE_PASSWORD2, IDE_CAPTCHA, IDB_REFRESH, 0 );
					SendDlgItemMessage( dlg, IDC_PROGRESS1, PBM_SETPOS, 0, 0 );

					int code = al->readResponse();
					switch( code )
					{
						case E_OK:
							KillTimer( dlg, 1 );
							EndDialog( dlg, E_OK );
							break;

						case E_REGISTERED_ALREADY:
							showError( E_REGISTERED_ALREADY, dlg );
							break;

						case E_INVALID_SERVER_RESPONSE:
							showError( E_INVALID_SERVER_RESPONSE, dlg );
							break;
					}
				}
				else
				{
					SendDlgItemMessage( dlg, IDC_PROGRESS1, PBM_STEPIT, 0, 0 );
				}
			}
			break;

		case WM_DESTROY:
			delete al;
			DestroyIcon( iconRefresh );
			break;
	}
	return( 0 );
} // dlgAccountRegistration

// ==========================
// threadAccountRegistration
// ==========================
DWORD WINAPI threadAccountRegistration( LPVOID )
{
	preventUnloading();
	char *email = ( char* )mymalloc( 128 );
	if( DialogBoxParam( g_hInstance, MAKEINTRESOURCE( IDD_ACCOUNT_REGISTRATION ), 0, dlgAccountRegistration, ( LPARAM )email ) == E_OK )
	{
		DialogBoxParam( g_hInstance, MAKEINTRESOURCE( IDD_ACCOUNT_REGISTRATION_CONFIRM ), 0, dlgAccountRegistrationConfirm, ( LPARAM )email );
	}
	free( email );
	allowUnloading();
	return( 0 );
} // threadAccountRegistration

// =========================
// dlgAccountForgotPassword
// ==========================
INT_PTR CALLBACK dlgAccountForgotPassword( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static CCaptcha *captcha;
	static CAutolicense* al;
	static HICON iconRefresh;
	switch( msg )
	{
		case WM_INITDIALOG:
			captcha = new CCaptcha( GetDlgItem( dlg, IDC_CAPTCHA ) );
			captcha->generate( 5 );

			iconRefresh = ( HICON )LoadImage( g_hInstance, MAKEINTRESOURCE( IDI_REFRESH ), IMAGE_ICON, 0, 0, 0 );
			SendDlgItemMessage( dlg, IDB_REFRESH, BM_SETIMAGE, IMAGE_ICON, ( LPARAM )iconRefresh );

			break;

		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDB_REFRESH:
					captcha->generate( 5 );
					break;

				case IDOK:
				{
					char email[ 128 ];
					char password[ 128 ];
					int n = GetDlgItemTextA( dlg, IDE_EMAIL, email, sizeof( email ) );
					if( isValidEmail( email ) )
					{
						if( isValidDlgPassword( dlg, IDE_PASSWORD, IDE_PASSWORD2, password, sizeof( password ) ) )
						{ 
							char text[ 6 ];
							GetDlgItemTextA( dlg, IDE_CAPTCHA, text, sizeof( text ) );
							if( strcmp( captcha->getCode(), text ) == 0 )
							{
								dlgEnable( dlg, FALSE, IDE_EMAIL, IDE_PASSWORD, IDE_PASSWORD2, IDE_CAPTCHA, IDOK, IDB_REFRESH, 0 );
								al = new CAutolicense;
								al->sendForgotPassword( email, password );
								SetTimer( dlg, 1, 300, 0 );
							}
							else
							{
								showError( E_INVALID_CAPTCHA, dlg );
							}
						}
						//else error is shown from isValidDlgPassword
					}
					else
					{
						showError( E_INVALID_EMAIL, dlg );
					}

					break;
				}

				case IDCANCEL:
					KillTimer( dlg, 1 );
					EndDialog( dlg, 0 );
					break;

			}
			break;

		case WM_TIMER:
			if( al->isReady() )
			{
				KillTimer( dlg, 1 );
				SendDlgItemMessage( dlg, IDC_PROGRESS1, PBM_SETPOS, 0, 0 );
				dlgEnable( dlg, TRUE, IDE_EMAIL, IDE_PASSWORD, IDE_PASSWORD2, IDE_CAPTCHA, IDOK, IDB_REFRESH, 0 );

				ERROR_TYPE err = al->readResponse();
				delete al;
				al = 0;
				switch( err )
				{
					case E_OK:
					{
						/*We have sent confirmation link to your email address. Please use this link to confirm password change. Then you can apply your product to Metatrader4 chart and authenticate using specified email and password.*/
						BYTE key[4]={0x2e,0x51,0x79,0xba};
						BYTE str[209]={0x35,0xd9,0xb0,0xba,0xd5,0x79,0xce,0x84,0x37,0x12,0xb2,0x8b,0x21,0xb4,0xc5,0x26,0x5e,0xe9,0x05,0x2d,0x9c,0x3c,0x5d,0x61,0x1b,0xb5,0x12,0x3b,0x16,0x4f,0x24,0xc8,0x67,0xdc,0x87,0x81,0x28,0x0f,0xef,0x33,0xaa,0x0f,0x60,0x55,0xcc,0x73,0x6f,0x47,0xa7,0xcd,0xcb,0x61,0x33,0x83,0xf1,0x7f,0x79,0x49,0xc4,0xe7,0x04,0xcd,0x27,0x23,0x36,0xd1,0xe3,0x3d,0x94,0x97,0x25,0xfd,0xa9,0x00,0xaa,0xaa,0xd5,0xdf,0xe4,0xd2,0x45,0xce,0xcd,0x08,0x8e,0x0e,0xf8,0xcf,0xe2,0x09,0x90,0xaf,0x2f,0xb5,0x2f,0x71,0xeb,0x59,0x3f,0x8d,0x5c,0x68,0x73,0x2f,0x08,0xe9,0xa7,0x98,0x29,0xff,0xf2,0xa9,0xd1,0x8d,0xc1,0x53,0x0d,0x78,0xd3,0x7e,0xbb,0x15,0xc9,0x5c,0x1c,0x2c,0x3d,0x4f,0x76,0xae,0xc8,0x4d,0xc7,0x93,0x96,0x84,0xb6,0x82,0x61,0x52,0x9d,0xe8,0x0d,0x8b,0x87,0x5e,0xd7,0xfe,0xd7,0xd8,0x7f,0x78,0x1a,0x6d,0xe5,0x2a,0x96,0xf3,0xa4,0x0f,0xd0,0x44,0x9e,0xc4,0x7c,0xb0,0xc0,0xba,0xb8,0x34,0xd1,0x4c,0x3d,0x31,0x3e,0x3a,0xbe,0x8b,0x85,0x45,0x79,0xd7,0x64,0x8d,0x05,0x9c,0xa8,0x77,0xcf,0xcb,0xb1,0xb7,0xc1,0x85,0x4e,0x3b,0x62,0xcf,0x4f,0xd8,0xa2,0x48,0x82,0x15,0x3b,0x01,0xd9,0x2b,0x5d};
						/*Confirmation*/
						BYTE key2[4]={0x22,0x43,0x1e,0x87};
						BYTE str2[13]={0x90,0xd4,0x41,0x57,0xce,0x11,0x0d,0x52,0x54,0xf7,0xe1,0xc4,0x1b};
						DECR( str, key );
						DECR2( str2, key2 );
						MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONINFORMATION );
						ENCR( str, key );
						ENCR( str2, key2 );

						EndDialog( dlg, 0 );
						break;
					}

					default:
						captcha->generate( 5 );
						showError( err, dlg );
				}
			}
			else
			{
				SendDlgItemMessage( dlg, IDC_PROGRESS1, PBM_STEPIT, 0, 0 );
			}
			break;

		case WM_DESTROY:
			delete captcha;
			if( al )
			{
				delete al;
				al = 0;
			}
			DestroyIcon( iconRefresh );
			break;
	}
	return( 0 );
} // dlgAccountForgotPassword

// =========================
// threadForgotPassword
// =========================
DWORD WINAPI threadForgotPassword( LPVOID )
{
	preventUnloading();
	DialogBoxParam( g_hInstance, MAKEINTRESOURCE( IDD_ACCOUNT_FORGOT_PASSWORD ), 0, dlgAccountForgotPassword, ( LPARAM )0 );
	allowUnloading();
	return( 0 );
} // threadForgotPassword

// ===================
// dlgRemoteAutolicense
// ===================
INT_PTR CALLBACK dlgRemoteAutolicense( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static int timeLeft;
	static CLic *lic;
	static CAutolicense* al;
	static BOOL waitForChange;
	static CLogin *lgn;
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			lic = ( CLic* )lp;
			al = new CAutolicense;

			char *title = ( char* )mymalloc( 256 );
			DECR( g_authenticateTitleStr, g_authenticateTitleKey );
			wsprintfA( title, ( char* )g_authenticateTitleStr, g_projectName );
			ENCR( g_authenticateTitleStr, g_authenticateTitleKey );
			SetWindowTextA( dlg, title );
			free( title );

			lgn = new CLogin( LT_REMOTE_AUTO );
			if( lgn->load() == 0 )
			{
dbg( "auth cache" );
				timeLeft = 10;
				SetDlgItemTextA( dlg, IDE_LOGIN, lgn->login );
				SetDlgItemTextA( dlg, IDE_PASSWORD, lgn->pass );
				waitForChange = 1;
			}
			else
			{
				timeLeft = MAX_AUTH_DURATION + 1;
				waitForChange = 0;
			}
			processLimitedTimer( dlg, IDOK, &timeLeft, g_validateFmt, sizeof( g_validateFmt ), g_validateKey, sizeof( g_validateKey ) );

			DECR2( g_authenticateBodyStr, g_authenticateBodyKey );
			char *slogan = ( char *)mymalloc( 256 );
			wsprintfA( slogan, ( char* )g_authenticateBodyStr, "email and password" );
			ENCR( g_authenticateBodyStr, g_authenticateBodyKey );
			SetDlgItemTextA( dlg, IDC_SLOGAN, slogan );
			free( slogan );


			SetTimer( dlg, 1, 1000, 0 );
			SetTimer( dlg, 2, 300, 0 );

			centerDlgToScreen( dlg );
			SetFocus( GetDlgItem( dlg, IDOK ) );
			return( FALSE );
		}


		case WM_MOVING:
			centerDlgToScreen( dlg, ( RECT* )lp );
			break;

		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDE_LOGIN:
					if( waitForChange && HIWORD( wp ) == EN_CHANGE )
					{
						char email[ 128 ];
						GetDlgItemTextA( dlg, IDE_LOGIN, email, sizeof( email ) );
						if( strcmp( email, lgn->login ) != 0 )
						{
dbg( "email changed '%s' => '%s'", lgn->login, email );
							waitForChange = 0;
							timeLeft = MAX_AUTH_DURATION + 1;
						}
					}
					break;

					break;

				case IDE_PASSWORD:
					if( waitForChange && HIWORD( wp ) == EN_CHANGE )
					{
						char pass[ 128 ];
						GetDlgItemTextA( dlg, IDE_PASSWORD, pass, sizeof( pass ) );
						if( strcmp( pass, lgn->pass ) != 0 )
						{
dbg( "pass changed '%s' '%s'", lgn->pass, pass );
							waitForChange = 0;
							timeLeft = MAX_AUTH_DURATION + 1;
						}
					}
					break;

				case IDOK:
				{
					char email[ 100 ];
					char password[ 100 ];
					int n = GetDlgItemTextA( dlg, IDE_LOGIN, email, sizeof( email ) );
					if( isValidEmail( email ) )
					{
						n = GetDlgItemTextA( dlg, IDE_PASSWORD, password, sizeof( password ) );
						if( n )
						{
							strcpy( lic->lastCre1, email );
							strcpy( lic->lastCre2, password );

							dlgEnable( dlg, 0, IDOK, IDB_REGISTER, IDE_LOGIN, IDE_PASSWORD, 0 );

							al->auth( email, password );
						}
						else
						{
							showError( E_NOPASSWORD, dlg );
						}
					}
					else
					{
						showError( E_INVALID_EMAIL, dlg );
					}
				}
					break;

				case IDCANCEL:
					KillTimer( dlg, 1 );
					KillTimer( dlg, 2 );
					EndDialog( dlg, E_CANCELLED );
					break;

				case IDB_REGISTER:
				{
					CloseHandle( myCreateThread( threadAccountRegistration, 0, "tAR" ) );

					KillTimer( dlg, 1 );
					KillTimer( dlg, 2 );
					EndDialog( dlg, -1 );
					break;
				}

				case IDB_FORGOT_PASSWORD:
				{
					CloseHandle( myCreateThread( threadForgotPassword, 0, "tFP" ) );

					KillTimer( dlg, 1 );
					KillTimer( dlg, 2 );
					EndDialog( dlg, -1 );
					break;
				}
			}
			break;

		case WM_TIMER:
			if( wp == 1 )	//1second timer
			{
				if( processLimitedTimer( dlg, IDOK, &timeLeft, g_validateFmt, sizeof( g_validateFmt ), g_validateKey, sizeof( g_validateKey ) ) <= 0 )
				{
					if( waitForChange )
					{
						//preloaded email/password were not changed => auto auth attempt
						waitForChange = 0;
						timeLeft = MAX_AUTH_DURATION + 1;
						PostMessage( dlg, WM_COMMAND, MAKEWPARAM( IDOK, BN_CLICKED ), 0 );
					}
					else
					{
						KillTimer( dlg, 1 );
						KillTimer( dlg, 2 );
						EndDialog( dlg, E_TIMEOUT );
					}
					break;
				}
			}
			else //300ms timer
			{
				if( al->isProcessing() )
				{
					if( al->isReady() )
					{
	dbg( "alhwt" );
						dlgEnable( dlg, TRUE, IDOK, IDB_REGISTER, IDE_LOGIN, IDE_PASSWORD, 0 );
						SendDlgItemMessage( dlg, IDC_PROGRESS, PBM_SETPOS, 0, 0 );

						ERROR_TYPE err = al->readResponse();
						switch( err )
						{
							case E_OK:
							{
								//TODO: shall we fetch also maxAccounts and maxCids here and show warning if limit reached like instances warning is shown?
								al->getResponseData( &lic->al.maxInstances, &lic->accType );

								CLogin l( LT_REMOTE_AUTO );
								strcpy( l.login, lic->lastCre1 );
								strcpy( l.pass, lic->lastCre2 );
								l.save();
													
	dbg( "maxinst %d, rd %d ", lic->al.maxInstances, lic->accType );
								KillTimer( dlg, 1 );
								KillTimer( dlg, 2 );
								EndDialog( dlg, E_OK );//ok result
									
								break;
							}

							case E_ACCOUNT_EXPIRED:
								al->getResponseData( &lic->al.userExpiration );
							case E_ACCOUNT_DISABLED:
								KillTimer( dlg, 1 );
								KillTimer( dlg, 2 );
								EndDialog( dlg, err );
								break;

							case E_ACCOUNT_NOT_CONFIRMED:
							case E_INVALID_AUTH:
								showError( err, dlg );
								break;

							case E_USAGE_LIMIT:
								al->getResponseData( &lic->al.usedAccounts,
													&lic->al.usedCids,
													&lic->al.usedInstances,
													&lic->al.maxAccounts,
													&lic->al.maxCids,
													&lic->al.maxInstances
												);
	dbg( "usedAc=%d usedCids=%d usedInst=%d maxAc=%d maxCids=%d maxInst=%d", lic->al.usedAccounts, lic->al.usedCids, lic->al.usedInstances, lic->al.maxAccounts, lic->al.maxCids, lic->al.maxInstances );
								KillTimer( dlg, 1 );
								KillTimer( dlg, 2 );
								EndDialog( dlg, E_USAGE_LIMIT );//license usage limit is reached
								break;

							default:
								showError( E_INVALID_SERVER_RESPONSE, dlg );
						}
					}
					else
					{
						SendDlgItemMessage( dlg, IDC_PROGRESS, PBM_STEPIT, 0, 0 );
					}
				}
			}
			break;

		case WM_DESTROY:
dbg( "dlgRemoteAutolicense destroy" );
			delete al;
			delete lgn;
			break;
	}
	return( 0 );
} // dlgRemoteAutolicense

// ================
// dlgRemoteLogin2
// ================
INT_PTR CALLBACK dlgRemoteLogin2( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static int timeLeft;
	static CLic *lic;
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			lic = ( CLic* )lp;

			char *title = ( char* )mymalloc( 256 );
			DECR( g_authenticateTitleStr, g_authenticateTitleKey );
			wsprintfA( title, ( char* )g_authenticateTitleStr, g_projectName );
			ENCR( g_authenticateTitleStr, g_authenticateTitleKey );
			SetWindowTextA( dlg, title );
			free( title );

			timeLeft = MAX_AUTH_DURATION + 1;
			processLimitedTimer( dlg, IDOK, &timeLeft, g_validateFmt, sizeof( g_validateFmt ), g_validateKey, sizeof( g_validateKey ) );

			{
				DECR( g_authenticateBodyStr, g_authenticateBodyKey );
				char *slogan = ( char *)mymalloc( 256 );
				wsprintfA( slogan, ( char* )g_authenticateBodyStr, "login and password" );
				ENCR( g_authenticateBodyStr, g_authenticateBodyKey );
				SetDlgItemTextA( dlg, IDC_SLOGAN, slogan );
				free( slogan );
			}


			SetFocus( GetDlgItem( dlg, IDE_LOGIN ) );
			SetTimer( dlg, 1, 1000, 0 );
			return( FALSE );
		}


		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDOK:
				{
					CLogin l( LT_REMOTE_LOGIN );
					int n = GetDlgItemTextA( dlg, IDE_LOGIN, l.login, sizeof( l.login ) );
					if( n )
					{
						n = GetDlgItemTextA( dlg, IDE_PASSWORD, l.pass, sizeof( l.pass ) );
						if( n )
						{
							if( lic->isValidLogin2( &l, ( ThreadLoadLicData* )lic->userData ) )
							{
								lic->userExpiration = l.expiration;
								lic->accType = l.accType;

								strcpy( lic->lastCre1, l.login );
								strcpy( lic->lastCre2, l.pass );
		
								l.save();
								KillTimer( dlg, 1 );
								EndDialog( dlg, 1 ); // "ok" result
							}
							else
							{
								timeLeft -= 10;
								( ( ThreadLoadLicData* )lic->userData )->initStart -= 10;

								DECR( g_authenticationFailedBodyStr, g_authenticationFailedBodyKey );
								DECR2( g_authenticationFailedTitleStr, g_authenticationFailedTitleKey );

								char* str = ( char *)mymalloc( 256 );
								wsprintfA( str, ( char *)g_authenticationFailedBodyStr, "login and password" );
								ENCR( g_authenticationFailedBodyStr, g_authenticationFailedBodyKey );
								char* title = ( char* )mymalloc( 256 );
								wsprintfA( title, ( char *)g_authenticationFailedTitleStr, g_projectName );
								ENCR( g_authenticationFailedTitleStr, g_authenticationFailedTitleKey );								 

								MessageBoxA( dlg, str, title, MB_ICONERROR );
								free( str );
								free( title );
							}
						}
						else
						{
							// Input password please
							BYTE key[4] = {0xc1,0x07,0x61,0xe1};
							BYTE str[22]={0x16,0xd2,0xda,0x6d,0xd0,0x8b,0xd9,0x89,0xc8,0x59,0xf5,0x5a,0x03,0x48,0xfa,0x3c,0x67,0xbc,0x12,0xc2,0xb8,0x05};
							DECR( str, key );

							// Error
							BYTE errKey[4] = {0x42,0x88,0x22,0x69};
							BYTE errStr[6]={0x45,0x60,0xd9,0x9e,0x3a,0x9a};
							{
								DECR( errStr, errKey );
							}
							MessageBoxA( dlg, ( char* )str, ( char* )errStr, MB_ICONERROR );
							ENCR( errStr, errKey );
							ENCR( str, key );
						}
					}
					else
					{
						// Input login please
						BYTE key[4] = {0xb7,0x47,0xf7,0xb7};
						BYTE str[19]={0x62,0xcc,0xd1,0x9f,0x2b,0x98,0xda,0x3e,0x12,0x03,0xe8,0x24,0x18,0xd4,0x56,0x41,0x63,0xe9,0x1e};
						DECR( str, key );

						// Error
						BYTE errKey[4] = {0xd8,0xee,0x24,0x2d};
						BYTE errStr[6]={0x6e,0xd6,0x73,0x3a,0xc1,0xc2};
						{
							DECR( errStr, errKey );
						}
						MessageBoxA( dlg, ( char* )str, ( char* )errStr, MB_ICONERROR );

						ENCR( errStr, errKey );
						ENCR( str, key );
					}
				}
					break;

				case IDCANCEL:
					KillTimer( dlg, 1 );
					EndDialog( dlg, 0 ); // "cancel" result
					break;
			}
			break;

		case WM_TIMER:
			if( processLimitedTimer( dlg, IDOK, &timeLeft, g_validateFmt, sizeof( g_validateFmt ), g_validateKey, sizeof( g_validateKey ) ) <= 0 )
			{
				KillTimer( dlg, 1 );
				EndDialog( dlg, 2 );	// "timeout" result
			}
			break;
	}
	return( 0 );
} // dlgRemoteLogin2

// ================
// dlgRemoteReceipt2
// ================
INT_PTR CALLBACK dlgRemoteReceipt2( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static int timeLeft;
	static CLic *lic;
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			lic = ( CLic* )lp;

			char *title = ( char* )mymalloc( 256 );
			DECR( g_authenticateTitleStr, g_authenticateTitleKey );
			wsprintfA( title, ( char* )g_authenticateTitleStr, g_projectName );
			ENCR( g_authenticateTitleStr, g_authenticateTitleKey );
			SetWindowTextA( dlg, title );
			free( title );

			timeLeft = MAX_AUTH_DURATION + 1;
			processLimitedTimer( dlg, IDOK, &timeLeft, g_validateFmt, sizeof( g_validateFmt ), g_validateKey, sizeof( g_validateKey ) );

			{
				DECR( g_authenticateBodyStr, g_authenticateBodyKey );
				char *slogan = ( char* )mymalloc( 256 );
				wsprintfA( slogan, ( char* )g_authenticateBodyStr, "receipt number" );
				ENCR( g_authenticateBodyStr, g_authenticateBodyKey );
				SetDlgItemTextA( dlg, IDC_SLOGAN, slogan );
				free( slogan );
			}

			SetFocus( GetDlgItem( dlg, IDE_RECEIPT ) );
			SetTimer( dlg, 1, 1000, 0 );
			return( FALSE );
		}
			

		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDOK:
				{
					CReceipt r;
					int n = GetDlgItemTextA( dlg, IDE_RECEIPT, r.receipt, sizeof( r.receipt ) );
					if( n )
					{
						if( lic->isValidReceipt2( &r, ( ThreadLoadLicData* )lic->userData ) )
						{
							lic->userExpiration = r.expiration;
							lic->accType = r.accType;

							strcpy( lic->lastCre1, r.receipt );

							r.save();
							KillTimer( dlg, 1 );
							EndDialog( dlg, 1 ); // "ok" result
						}
						else
						{
							timeLeft -= 10;
							( ( ThreadLoadLicData* )lic->userData )->initStart -= 10;

							DECR( g_authenticationFailedBodyStr, g_authenticationFailedBodyKey );
							{
								DECR( g_authenticationFailedTitleStr, g_authenticationFailedTitleKey );
							}

							char* str = ( char* )mymalloc( 256 );
							wsprintfA( str, ( char* )g_authenticationFailedBodyStr, "receipt number" );
							ENCR( g_authenticationFailedBodyStr, g_authenticationFailedBodyKey );
							char* title = ( char* )mymalloc( 256 );
							wsprintfA( title, ( char *)g_authenticationFailedTitleStr, g_projectName );
							ENCR( g_authenticationFailedTitleStr, g_authenticationFailedTitleKey );								 

							MessageBoxA( dlg, str, title, MB_ICONERROR );
							free( str );
							free( title );
						}
					}
					else
					{
						// Input receipt number please
						BYTE key[4] = {0xae,0x4a,0xfc,0xcd};
						BYTE str[28]={0xc0,0x16,0xa3,0x37,0x70,0x13,0x15,0x85,0x28,0x52,0x62,0x61,0x9d,0xfb,0xdd,0xc0,0xc3,0x9d,0x7c,0x2b,0xc1,0x37,0xca,0xd3,0xc1,0xdc,0xeb,0x4d};
						DECR( str, key );

						// Error
						BYTE errKey[4] = {0xa1,0xfc,0xb6,0x9e};
						BYTE errStr[6]={0xb5,0x31,0xad,0x76,0x93,0xab};

						{
						DECR( errStr, errKey );
						}
						
						MessageBoxA( dlg, ( char* )str, ( char* )errStr, MB_ICONERROR );
						ENCR( errStr, errKey );
						ENCR( str, key );
					}
				}
					break;

				case IDCANCEL:
					KillTimer( dlg, 1 );
					EndDialog( dlg, 0 ); // "cancel" result
					break;
			}
			break;

		case WM_TIMER:
			if( processLimitedTimer( dlg, IDOK, &timeLeft, g_validateFmt, sizeof( g_validateFmt ), g_validateKey, sizeof( g_validateKey ) ) <= 0 )
			{
				KillTimer( dlg, 1 );
				EndDialog( dlg, 2 );	// "timeout" result
			}
			break;
	}
	return( 0 );
} // dlgRemoteReceipt2

#endif //ifdef GEN_LT_REMOTE

// ====================
// processLimitedTimer
// ====================
int processLimitedTimer( HWND dlg, DWORD idc, IN OUT int* timeLeft, BYTE* fmt, DWORD fmtSize, BYTE* key, DWORD keySize )
{
	*timeLeft = *timeLeft - 1;
	char vt[ 20 ];
	
	if( *timeLeft >= 0 )
	{
		CRC4 rc4;
		rc4.Decrypt( fmt, fmtSize, key, keySize );
		wsprintfA( vt, ( char* )fmt, *timeLeft );
		rc4.Encrypt( fmt, fmtSize, key, keySize );
	
		SetDlgItemTextA( dlg, idc, vt );
	}
	return( *timeLeft );
} // processLimitedTimer

#if defined( GEN_LT_REMOTE )
// =====================
// dlgRemoteUpdate
// =====================
INT_PTR CALLBACK dlgRemoteUpdate( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static char* url;
	static char* message;
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			g_dlgUpdate = dlg;

			// %s update notification
			BYTE key[4] = {0xb7,0xa1,0xd3,0x31};
			BYTE str[23]={0x3c,0xe2,0x08,0x35,0x98,0x3b,0xb8,0xae,0x39,0x75,0x74,0xa2,0xd5,0x0e,0xab,0xa7,0xfa,0x79,0x01,0x88,0x03,0x0b,0x1c};
			DECR( str, key );
			char *title = ( char* )mymalloc( strlen( g_projectName ) + strlen( ( char* )str ) + 16 );
			wsprintfA( title, ( char* )str, g_projectName );
			ENCR( str, key );
			SetWindowTextA( dlg, title );
			free( title );

			CLic* lic = ( CLic* )lp;
			int len = lstrlenA( lic->updateUrl ) + 1;
			url = ( char *)mymalloc( len );
			memcpy( url, lic->updateUrl, len );
			//SetDlgItemText( dlg, IDC_LINK, url );

			len = lstrlenA( lic->updateMessage ) + 1;
			BOOL expired = isProjectExpired();
			if( expired )
			{
				len += 1024;
			}
			message = ( char* )mymalloc( len );
			memcpy( message, lic->updateMessage, len );
			if( expired )
			{
				/*



				---------------------------------------



				You have a valid account for product %s but this product has newer revision/version. Your chart has been closed due expiration. You run the version %d, please contact your vendor to update you to newer revisions > %d. As soon you get new revision from vendor, you will be able to login to this product. Please also check your email, vendor might have sent you an update but you may have missed to install it.*/
				BYTE textKey[4]={0x6f,0x78,0xfb,0x09};
				BYTE textFmt[456]={0x9a,0xe9,0x96,0x7d,0xea,0x15,0x06,0x73,0x70,0x1a,0xb0,0xf2,0xf5,0xf7,0x01,0xc8,0x9f,0x19,0x1a,0xc4,0xd8,0x35,0xad,0xed,0x7a,0x2a,0x4f,0x19,0xb7,0xdd,0x7b,0x10,0x44,0xb4,0xd4,0xed,0x6b,0xb0,0x38,0x52,0x34,0x08,0x82,0xab,0xdd,0x75,0x42,0x76,0x70,0xca,0x42,0x32,0xe2,0x13,0xea,0xaf,0xc5,0x39,0x28,0xca,0x00,0x5f,0x8e,0xff,0xbf,0x92,0x47,0x8d,0x7e,0xe8,0x11,0x73,0xc6,0x1e,0xa8,0xbf,0x9b,0xf4,0x13,0xff,0x7f,0xbe,0xc8,0xee,0x09,0x3a,0xbb,0x81,0x77,0x55,0x41,0xfd,0x9b,0xae,0xa7,0xd8,0x1a,0xf5,0x22,0xfa,0x11,0x04,0x8f,0x7c,0xa2,0xbe,0x4f,0x5a,0x31,0xe7,0x74,0xff,0x6e,0x85,0x41,0x1e,0xda,0x24,0x28,0x85,0xed,0x4f,0x2d,0xc3,0x8d,0x16,0x0c,0x86,0x83,0xf4,0x9a,0x01,0x9e,0x64,0x2a,0x17,0x94,0xb5,0xbd,0xb8,0x2d,0xc0,0xb1,0xcb,0x4e,0xaa,0xc4,0xb6,0x6c,0x57,0x99,0xd8,0x45,0x32,0xa2,0x9d,0xc2,0xa7,0x2d,0xfb,0xc1,0x72,0xa8,0x12,0xab,0x95,0xc1,0xf6,0x2c,0xec,0x24,0xa9,0x43,0x43,0xe8,0x64,0xa8,0xfb,0xfc,0x64,0x3e,0xb3,0xf3,0xad,0x37,0x67,0x54,0x94,0x79,0x43,0x00,0xeb,0xeb,0x67,0x8b,0x9f,0x47,0xf2,0xdd,0x94,0x65,0xa3,0xde,0x48,0xed,0xa3,0x0b,0xeb,0x48,0x8b,0x92,0x4a,0xcb,0xae,0x04,0x4c,0xa6,0x3c,0x4a,0xf5,0x8a,0x57,0xe0,0xd5,0xc3,0x43,0x2c,0xa3,0xd4,0x8d,0xd8,0xc9,0x32,0x60,0x58,0xa1,0xd0,0x32,0xfd,0x38,0x0b,0xdb,0xd5,0xd5,0xa8,0xbe,0xfe,0x1d,0x6c,0x33,0x13,0xec,0xd5,0xc4,0xf8,0xfa,0x1b,0x6f,0x5a,0x5f,0xf0,0xae,0xc6,0x2a,0xcd,0x0a,0x7e,0xe4,0x74,0x07,0xe8,0x80,0xfb,0x15,0xd5,0xf5,0xab,0x97,0x75,0xff,0x73,0xb7,0xba,0x4f,0x45,0x5e,0x88,0xea,0x87,0xc7,0x07,0x7e,0x30,0xd2,0x07,0x58,0x9e,0x43,0x02,0xf6,0x6b,0x54,0x43,0x2d,0xf4,0x99,0x9a,0x28,0xc2,0x97,0x26,0x7c,0x4e,0x3e,0x9d,0x1d,0x47,0xa6,0x67,0xa4,0x85,0x03,0x39,0x08,0x82,0x49,0x41,0x3b,0xb2,0x1e,0xf6,0x1d,0x47,0xfc,0xad,0x87,0xf3,0x5e,0xb6,0x5b,0xa6,0x17,0x9e,0x09,0x52,0xc9,0xb1,0xc8,0xb0,0xa5,0xb6,0x8b,0x19,0x37,0x50,0x87,0x57,0x3f,0x68,0xcd,0x01,0xfc,0x61,0x4d,0xcb,0x4c,0x52,0x83,0xfe,0x53,0x53,0x0c,0x72,0xbb,0x69,0xfd,0xe3,0x9d,0x96,0xe6,0x35,0x39,0xdc,0x16,0xa3,0x8d,0x0d,0x12,0xe2,0x7b,0x3a,0x70,0x1c,0xb5,0xea,0xe1,0xc5,0x12,0x4d,0x28,0x96,0xdb,0x67,0x9f,0x6a,0xbe,0xe8,0x70,0x1b,0xa4,0xc6,0xc5,0x44,0x95,0x5a,0x4c,0x2a,0xdc,0x39,0xc5,0xf1,0x4b,0x0c,0x1e,0x54,0x11,0x3b,0xc6,0x9a,0x62,0x62,0xfa,0x89,0x7f,0x07,0x47,0xc8,0x3a,0x89,0xcf,0xc8,0xdb,0xc6,0x74,0xcf,0x7c,0xe5,0x25,0xb9,0xcd,0x2a,0x79,0x25,0x1b,0x2a,0xca};

				char* text = ( char* )mymalloc( 512 );
				DECR( textFmt, textKey );

				Globals* g = getGlobals();
				const char* projectName = g->useProject();
				int textLen = 1 + wsprintfA( text, ( const char* )textFmt, projectName, g->getRevision(), g->getRevision() );
				g->unuse( projectName );
				ENCR( textFmt, textKey );

				strcat( message, text );
				free( text );

				//make window higher
				RECT r, r2;
#define DY 100
#define HH 30
				//dlg itself
				GetWindowRect( dlg, &r );
				MoveWindow( dlg, r.left, r.top, r.right - r.left, r.bottom - r.top + DY, TRUE );
				//message
				HWND wnd = GetDlgItem( dlg, IDE_MESSAGE );
				GetWindowRect( wnd, &r2 );
				MoveWindow( wnd, r2.left - r.left, r2.top - r.top - HH, r2.right - r2.left, r2.bottom - r2.top + DY, TRUE );
				//update button
				wnd = GetDlgItem( dlg, IDOK );
				GetWindowRect( wnd, &r2 );
				MoveWindow( wnd, r2.left - r.left, r2.top - r.top + DY - HH, r2.right - r2.left, r2.bottom - r2.top, TRUE );
				//CANCEL BUTTON
				wnd = GetDlgItem( dlg, IDCANCEL );
				GetWindowRect( wnd, &r2 );
				MoveWindow( wnd, r2.left - r.left, r2.top - r.top + DY - HH, r2.right - r2.left, r2.bottom - r2.top, TRUE );
#undef DY
#undef HH
			}
			SetDlgItemTextA( dlg, IDE_MESSAGE, message );
			delete lic;
		}
			break;

		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDOK:
					openLinkA( url );
				case IDCANCEL:
					EndDialog( dlg, 0 );
					break;
			}
			break;

/*		case WM_NOTIFY:
		{
			NMHDR* pNMHdr = ( NMHDR* )lp;
			switch( pNMHdr->code)
			{
				case NM_RETURN:
				case NM_CLICK:
				{
					openLink( url );
					/*PNMLINK pNMLink = ( PNMLINK )lp;
					if (!StrCmpW(pNMLink->item.szID, L"idLaunchHelp"))
					{
						// Insert code here to launch Help Center.
					}
					else if (!StrCmpW(pNMLink->item.szID, L"idHelpMenu"))
					{
						// Insert code here to launch Help Menu.
					}* /
				}
				break;
			}
			break;
		}
*/

		case WM_DESTROY:
			free( url );
			free( message );
			g_dlgUpdate = NULL;
			break;
	}
	return( 0 );
} // dlgRemoteUpdate

// ====================
// isAUTargetDir
// ====================
BOOL isAUTargetDir( const char* dir, OUT char* ex4 )
{
	strcpy( ex4, dir );

	Globals* g = getGlobals();
	if( g->getFlag( FLG_EA ) )
	{
		/*\MQL4\Experts\*/
		BYTE key[4]={0x23,0xa8,0x41,0xa8};
		BYTE str[15]={0x1a,0x8f,0x33,0xf6,0xe6,0xbb,0xc9,0xf9,0xc9,0x24,0xd1,0xbe,0xa5,0x9a,0xb1};
		DECR( str, key );
		strcat( ex4, ( const char* )str );
		ENCR( str, key );
	}
	else
	{
		/*\MQL4\Indicators\*/
		BYTE key[4]={0x81,0x40,0xee,0x94};
		BYTE str[18]={0x2d,0x22,0x9d,0xdf,0x3f,0xb3,0x53,0x16,0x40,0xac,0xbc,0x94,0x91,0x90,0xf4,0xa9,0x8d,0xb0};
		DECR( str, key );
		strcat( ex4, ( const char* )str );
		ENCR( str, key );
	}
	const char* ex4Name = g->useEx4Name();
	if( *ex4Name )
	{
		strcat( ex4, ex4Name );
		g->unuse( ex4Name );
	}
	strcat( ex4, ".ex4" );

	return( PathFileExistsA( ex4 ) );
} // isAUTargetDir

// =================
// runRestarter
// =================
void __inline runRestarter( const char *tmp, const char* tmpDll, const char* tmpEx4, const char* datadir, BOOL isPortableMode )
{
	BOOL res = FALSE;

	//unpack restarter
	HRSRC hr = FindResourceA( g_hInstance, MAKEINTRESOURCEA( IDR_RESTARTER ), "RST" );
	HGLOBAL hg = LoadResource( g_hInstance, hr );
	BYTE* exe = ( BYTE* )LockResource( hg );
	DWORD sz;
	sz = SizeofResource( g_hInstance, hr );

	//write to temp file
	char *sexe = ( char* )mymalloc( MAX_PATH );
	/*%s\fx1rst%d*/
	BYTE key[4]={0x0e,0x1d,0x88,0xf4};
	BYTE str[24]={0x2a,0x0c,0x85,0x4d,0x6a,0x25,0xec,0xe6,0x46,0xca,0x54,0x2a,0x33,0x51,0x8f,0x2a,0xc6,0x3d,0xdb,0x47,0x2e,0x32,0x0f,0xd5};
	DECR( str, key );
	wsprintfA( sexe, ( const char* )str, tmp, GetCurrentProcessId() );
	ENCR( str, key );
	HANDLE h = CreateFileA( sexe, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
	if( h != INVALID_HANDLE_VALUE )
	{
		WriteFile( h, exe, sz, &sz, 0 );
		CloseHandle( h );

		//start restarter
		HANDLE hme;
		DuplicateHandle( GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &hme, 0, TRUE,DUPLICATE_SAME_ACCESS );
					
		char *me = ( char* )mymalloc( MAX_PATH );
		GetModuleFileNameA( g_hInstance, me, MAX_PATH );
		char* ex4 = ( char* )mymalloc( MAX_PATH );

		char *mt4 = ( char* )mymalloc( MAX_PATH );
		GetModuleFileNameA( 0, mt4, MAX_PATH );
		char* sl = strrchr( mt4, '\\' );
		*sl = 0;	//need directory to detect target dir

		if( isAUTargetDir( datadir, ex4 ) )
		{
			dbg( "tddd" );	//target dir - datadir
		}
		else if( isAUTargetDir( mt4, ex4 ) )
		{
			dbg( "tdtd" ); //target dir - terminal dir
		}
		else
		{
			dbg( "!!-- tdnf" );//target dir not found
		}
		
		*sl = '\\'; //restore full terminal.exe path


		char *cmd = ( char* )mymalloc( 2048 );
		{
		/*%d "%s" "%s" "%s" "%s" "%s" %d*/
		BYTE key[4]={0xce,0x73,0x59,0x54};
		BYTE str[31]={0xd3,0x31,0xba,0x2c,0x41,0x0b,0xdf,0xbb,0xb8,0x71,0x12,0x5a,0x13,0xea,0x1f,0xd2,0x2c,0xfd,0x4d,0x8d,0x1f,0x4a,0xf5,0x14,0xab,0xac,0xf1,0x65,0x71,0x0f,0xec};
		DECR( str, key );
		wsprintfA( cmd, ( const char* )str, hme, tmpDll, me, tmpEx4, ex4, mt4, isPortableMode );
		ENCR( str, key );
		}
		free( me );
		free( mt4 );
		free( ex4 );
//log( ERR_COLOR, "TOREMOVE cmd='%s'", cmd );
			
		STARTUPINFOA si;
		memset( &si, 0, sizeof( si ) );
		si.cb = sizeof( si );
#ifndef _DEBUG
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
#endif
		PROCESS_INFORMATION pi;
		if( res = CreateProcessA( sexe, cmd, 0, 0, TRUE, 0, 0, tmp, &si, &pi ) )
		{
//log( ERR_COLOR, "TOREMOVE CreateProcess %d %d", ok, GetLastError() );
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
		}

		if( !res )
		{
			DeleteFileA( sexe );
		}
		free( sexe );
		free( cmd );
	}
	if( !res )
	{
		DeleteFileA( tmpDll );
		DeleteFileA( tmpEx4 );
	}

} // runRestarter


// =================
// threadAutoUpdate
// =================
DWORD WINAPI threadAutoUpdate( AutoUpdateData* aud )
{
	DWORD done;

	TRY

dbg( "tau" );
	Globals* g = getGlobals();

	/*/ml/fs.php?p=au&id=%d&rev=%d&fsauth=%s*/
	BYTE keyUri[4]={0xb9,0xed,0xe3,0xbb};
	BYTE strUri[39]={0xf2,0x68,0xad,0x27,0x14,0x5c,0xac,0x3c,0x10,0xa5,0xcf,0x1b,0x61,0xb7,0x1c,0x73,0x56,0x90,0xf5,0xc3,0xce,0x17,0xf1,0x7f,0x5f,0x9e,0x7d,0x40,0x0e,0x52,0x6a,0xd5,0x47,0x0b,0xd2,0xa3,0x82,0x3f,0x2d};

	/*mqllock.com*/
	BYTE keyServer[4]={0x97,0xf9,0x0f,0x8d};
	BYTE strServer[12]={0x2b,0x05,0xc1,0xe5,0x5c,0x12,0x72,0xf4,0x37,0x45,0x2e,0xfb};

	CBaseServerManager* mgr = new CBaseServerManager();
	done = 0;

	const char* fsauth = g->useFSAuth();
		
	CRC4 rc4;
	
	rc4.Decrypt( strUri, sizeof( strUri ), keyUri, sizeof( keyUri ) );
	{
		
		char *uri = ( char* )mymalloc( 512 );
		wsprintfA( uri, ( char* )strUri, g->getProjectId(), g->getRevision(), fsauth );
		rc4.Encrypt( strUri, sizeof( strUri ), keyUri, sizeof( keyUri ) );
dbg( "fdf '%s'", uri );
		rc4.Decrypt( strServer, sizeof( strServer ), keyServer, sizeof( keyServer ) );
		CBaseServerManagerHandle *h = mgr->load( ( const char* )strServer, 443, uri, TRUE );
		rc4.Encrypt( strServer, sizeof( strServer ), keyServer, sizeof( keyServer ) );

		free( uri );

		DWORD waited;
		for( waited = h->wait( 100 ); waited == WAIT_TIMEOUT; waited = h->wait( 100 ) )
		{
			if( aud->cancelled )	//set in dlgRemoteAutoUpdate proc
			{
dbg( "auc" );
				h->abort();
				break;
			}
		}
dbg( "wt %d", waited );
		if( waited == WAIT_OBJECT_0 )
		{
dbg( "er %d %d %d", h->error, h->status, h->cl );
			MLError internetRes = h->error;

			if( internetRes == ML_OK )
			{
				if( h->status == 200 && h->cl > 2 * sizeof( DWORD ) )
				{
					DWORD sz;
					PostMessage( aud->dlg, WM_USER + 1, h->cl, 0 );	//notifying dialog that we got total number of bytes

					//read data
					char* raw = ( char* )mymalloc( h->cl + 1 );
					DWORD read = 0;
					DWORD prevPercent = -1;
					for( ; !aud->cancelled; )
					{
						if( InternetReadFile( h->hr, raw + read, 20480, &sz ) && sz )
						{
dbg( "rd %d", sz );
							read += sz;
							PostMessage( aud->dlg, WM_USER + 2, read, 0 );//notifying how many percents downloaded
						}
						else
						{
dbg( "frd %d", GetLastError() );
							break;
						}
					}
					if( read == h->cl )
					{	
dbg( "rb %d", h->cl );
						const BYTE* lk = g->useLicenseKey();
						rc4.Decrypt( ( BYTE* )raw, h->cl, lk, GEN_LICENSE_KEY_LENGTH );
						g->unuse( ( const char* )lk );


						DWORD sizeDll;
						sizeDll = *( DWORD* )raw;
dbg( "dsz %d", sizeDll );

						if( sizeDll )
						{
							if( sizeof( DWORD ) + sizeDll + sizeof( DWORD ) < h->cl )
							{
								char* tmp = ( char* )mymalloc( MAX_PATH );
								GetTempPathA( MAX_PATH, tmp );

								/*%s\mlaudll%d*/
								BYTE key1[4]={0x01,0xc0,0x2b,0x63};
								BYTE fmt1[13]={0x4c,0xda,0xb7,0x7f,0x41,0xed,0x09,0xae,0xdc,0x7b,0x3d,0x4d,0x9b};

								//create temp dll file
								char* tmpDll = ( char* )mymalloc( MAX_PATH );
								{
									DECR( fmt1, key1 );
									wsprintfA( tmpDll, ( const char* )fmt1, tmp, GetTickCount() );
									ENCR( fmt1, key1 );
								}
								//write sizeDll bytes from raw+4
								HANDLE hf = CreateFileA( tmpDll, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
								if( hf != INVALID_HANDLE_VALUE )
								{
									DWORD sz;
									BOOL written = WriteFile( hf, raw + sizeof( DWORD ), sizeDll, &sz, 0 );
									CloseHandle( hf );
									if( written && sz == sizeDll )
									{
										//check ex4 size
										DWORD sizeEx4 = *( DWORD* )( raw + sizeof( DWORD ) + sizeDll );
	dbg( "esz %d", sizeEx4 );
										if( sizeEx4 )
										{
											if( sizeof( DWORD ) + sizeDll + sizeof( DWORD ) + sizeEx4 == h->cl )
											{
												/*%s\mlauex4%d*/
												BYTE key2[4]={0x66,0x04,0x28,0xac};
												BYTE fmt2[13]={0x28,0x52,0xe8,0x60,0x7e,0x48,0xe2,0x8e,0x47,0x69,0x04,0xfa,0x66};

												DECR( fmt2, key2 );
												char* tmpEx4 = ( char* )mymalloc( MAX_PATH );
												wsprintfA( tmpEx4, ( const char* )fmt2, tmp, GetTickCount() );
												ENCR( fmt2, key2 );

												//write ex4 bytes
												hf = CreateFileA( tmpEx4, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
												if( hf != INVALID_HANDLE_VALUE )
												{
													written = WriteFile( hf, raw + sizeof( DWORD ) + sizeDll + sizeof( DWORD ), sizeEx4, &sz, 0 );
													CloseHandle( hf );
													if( written && sz == sizeEx4 )
													{
														g_logger->submit( LGT_AU_OK );

														runRestarter( tmp, tmpDll, tmpEx4, aud->datadir, aud->isPortableMode );
														done = 1;
													}
													else
													{
														done = -11;
													}
												}
												else
												{
													done = -10;
												}
												if( done != 1 )
												{
													DeleteFileA( tmpEx4 );
												}
												free( tmpEx4 );
											}
											else
											{
												done = -9;
											}
										}
										else
										{
											done = -8;
										}
									}
									else
									{
										done = -7;
									}
									
									if( done != 1 )
									{
										DeleteFileA( tmpDll );
									}
								}
								else
								{
									done = -6;
								}
								free( tmpDll );
								free( tmp );
							}
							else
							{
								done = -5;
							}
						}
						else
						{
							done = -4;
						}
					}
					else
					{
						done = -3;
					}
					free( raw );
				}
				else
				{
					done = -2;
				}
			}
			else
			{
				done = -1;
			}
		}
		delete h;
		/*if( done )
		{
			break;
		}*/
	} //for
	//g->unuse( servers );
	g->unuse( fsauth );
	delete mgr;
	
	CATCH
	return( done );
} // threadAutoUpdate

// =======================
// dlgRemoteAutoUpdate
// =======================
INT_PTR CALLBACK dlgRemoteAutoUpdate( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static char* url;
	static char* message;
	static HANDLE hUpdateThread;
	static BOOL timerProgress;
	static AutoUpdateData *aud;
	static DWORD totalBytes, prevPercent;
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			aud = ( AutoUpdateData* )lp;

			HICON ico = ( HICON )LoadImageA( g_hInstance, MAKEINTRESOURCEA( IDI_AUTO_UPDATE ), IMAGE_ICON, 0, 0, 0 );
			SendDlgItemMessageA( dlg, IDC_AUTO_UPDATE, STM_SETIMAGE, IMAGE_ICON, ( LPARAM )ico );
			Globals* g = getGlobals();			
			{
			/*Update required for %s - rev.%d - %s*/
			BYTE key[4]={0x46,0x94,0x79,0x0c};
			BYTE str[37]={0x19,0x6f,0x2e,0xad,0x5e,0x62,0x24,0x20,0xca,0xfe,0x0f,0x2f,0xfd,0x6b,0xe8,0x9a,0x10,0x59,0x15,0x16,0x5a,0xd7,0xfa,0x46,0x6f,0x87,0x4a,0x86,0xfd,0x4d,0xe4,0xad,0xe9,0x3a,0x62,0x53,0x56};
			DECR( str, key );
			char* title = ( char* )mymalloc( strlen( g_projectName ) + strlen( ( char* )str ) + 16 );
			const char* gid = g->useId();
			wsprintfA( title, ( char* )str, g_projectName, g->getRevision(), gid + GEN_EXT_ID_LENGTH - 4 );
			g->unuse( gid );
			ENCR( str, key );

			SetWindowTextA( dlg, title );
			free( title );
			}

			{
			/*Your EA/Indicator "%s" with filename "%s.ex4" needs an update. This update can be done fully automatically. Once you click the "Update Now" button, we will download, update required files and restart your Metatrader Terminal. If you choose not to update yet, click to "Skip Update" button. Please note: update is mandatory and important in order to continue your EA/Indicator support. Updating usually takes several seconds and will restart your Metatrader4 Terminal automatically. Please make sure you don't have very important positions running while restarting.*/
			BYTE key[4]={0xc3,0xb7,0x94,0x44};
			BYTE str[565]={0x00,0x9c,0x21,0x0f,0xc9,0x55,0x78,0x77,0xb6,0x50,0x22,0x1c,0x89,0x22,0xc0,0x18,0x99,0x6e,0xbd,0x29,0x14,0x78,0x3d,0x0e,0x8f,0xa4,0x7d,0xb8,0x61,0xe9,0xb1,0x66,0x96,0x79,0xab,0xfb,0x7b,0xc5,0xd5,0xfd,0xfd,0x49,0x7d,0x6c,0x4c,0x72,0x26,0x09,0x1b,0xf5,0x03,0xfa,0x6a,0xcb,0xcb,0xdf,0xda,0xfd,0x5d,0xfe,0x87,0x38,0x43,0xaf,0x17,0xe7,0x41,0x2c,0x00,0x50,0x34,0xb7,0xe2,0x5a,0xbe,0xa9,0x80,0xf3,0x04,0x33,0x45,0xe1,0x6b,0x34,0x1e,0x24,0xb4,0x45,0xbd,0x52,0x61,0x97,0xf1,0x21,0xd2,0xbf,0x7e,0x97,0x1f,0xaf,0x0d,0x01,0x48,0xbd,0xce,0x39,0xe4,0x4a,0x59,0xc6,0xd3,0x2c,0x64,0x6d,0xb4,0x6b,0x49,0x18,0xb8,0xb7,0xa4,0x26,0xb1,0xc1,0x1a,0x54,0x40,0xe4,0x67,0xbb,0xde,0x01,0x71,0xe0,0x17,0x17,0xa5,0x23,0x3f,0x58,0xda,0xcd,0x22,0x9a,0x43,0xcd,0x47,0xab,0xb5,0xc7,0x7a,0x95,0x6b,0xd0,0x14,0x13,0x4c,0xa1,0x7a,0x39,0x7e,0x04,0x26,0xef,0x34,0xe2,0x68,0x34,0x52,0x9b,0x7d,0x75,0x85,0xaa,0xdb,0x59,0x82,0x56,0xea,0xec,0x62,0x52,0x7c,0x38,0x5d,0x0e,0xc5,0x9f,0x40,0xd8,0x51,0xc3,0xa4,0x2a,0xfc,0xb4,0x52,0x57,0x2f,0x03,0x0c,0x21,0x77,0x1c,0x91,0x5f,0x0b,0x9c,0x3d,0x34,0xd4,0x98,0x5a,0xa8,0x1d,0x93,0x95,0xc2,0xe5,0xdc,0x4e,0x87,0x10,0xef,0xae,0x3a,0x00,0xa4,0x83,0xa0,0x89,0x6a,0x67,0x02,0xbf,0x46,0x04,0xdf,0xe2,0x4a,0x39,0x09,0x4c,0x23,0x50,0x03,0x10,0x84,0xc0,0x48,0x38,0xa0,0xf3,0xd2,0xff,0xcd,0x3b,0xa6,0x1c,0x7e,0x59,0x36,0x3d,0x00,0xdf,0x6c,0x29,0x9f,0xf9,0x76,0x28,0x28,0xb0,0x85,0xe0,0xa2,0x7b,0x5f,0xc2,0x93,0xf0,0xa0,0x23,0x22,0xb3,0xc4,0x6e,0xa9,0xa1,0xbe,0x1d,0x47,0x32,0xee,0x66,0xe9,0x09,0xac,0xa6,0xa9,0xfa,0xde,0x21,0xa0,0xfd,0x59,0xd9,0xf2,0xe7,0x48,0x43,0xfa,0x22,0x34,0xe3,0xf9,0x9e,0xe5,0xd6,0x6a,0x52,0x3b,0x00,0xd2,0x74,0x1f,0xed,0x66,0xa3,0xb5,0xb3,0x63,0xde,0xaf,0x27,0x3a,0x36,0x74,0x2c,0xf6,0x7f,0xda,0xdd,0xcc,0x16,0x32,0x50,0x72,0x22,0x28,0xef,0xb3,0xd0,0xdf,0x47,0x28,0x99,0x9e,0x38,0x0a,0x4d,0xdb,0x6e,0x19,0xb2,0xee,0x52,0x14,0xd7,0x18,0x89,0x0d,0xa9,0x08,0xeb,0x65,0xf6,0x13,0xef,0x1d,0xf0,0x27,0x39,0xc5,0xe1,0xe4,0x7a,0x27,0xf7,0xd3,0x65,0x0c,0x94,0xd3,0xfc,0xd8,0xc0,0xb5,0x74,0xd0,0x8d,0x98,0x81,0xd2,0x21,0x31,0x49,0x4f,0xc8,0xa4,0x96,0x9e,0x90,0xa3,0xb9,0x3d,0x0c,0x4c,0x3a,0x0e,0x82,0x3b,0x90,0xb0,0x31,0xd9,0xfa,0xf9,0x5d,0xe8,0xe1,0xde,0xd4,0xe3,0x4a,0xd7,0x54,0x2f,0x49,0x04,0xa1,0x6f,0xa2,0x28,0xfa,0xa8,0x5a,0xbd,0xb8,0x55,0x31,0x5a,0x0d,0xa8,0x49,0xa1,0x78,0x1a,0x85,0xa1,0x51,0x55,0x42,0x40,0xd0,0xae,0xff,0x01,0x0a,0x28,0x21,0x5e,0x42,0x8d,0xf6,0x4f,0xd1,0x67,0x02,0x96,0xbf,0x3e,0x44,0x0d,0x81,0xd4,0x50,0x2b,0x86,0x02,0x59,0xa5,0xe6,0x94,0xf8,0x65,0x6c,0xe6,0xf0,0x8a,0x57,0x34,0xf0,0xf3,0xeb,0x06,0x94,0x64,0xbc,0x9a,0x98,0x2f,0x67,0x99,0xb9,0xf3,0x5d,0xb7,0xd2,0x26,0xa0,0xa6,0xfa,0x9c,0x1e,0x58,0x88,0xaa,0xf1,0xcc,0x60,0xa5,0x8e,0xfe,0x59,0x87,0x49,0xd5,0x70,0xb1,0xb3,0xfa,0x3f,0x43,0x29,0x40,0x65,0x6c,0x99,0x3b,0xba,0x92,0x80,0x57,0xcb,0xb0,0x77,0x2a,0x56,0x9a,0x57,0x9f,0x5c,0x64,0xa5};
			
			
			const char* ex4Name = g->useEx4Name();
dbg( "ex4 %08x", ex4Name );
			while( !*ex4Name )
			{
				Sleep( 1000 );
				ex4Name = g->useEx4Name();
dbg( "ex4 %08x", ex4Name );
			}
			DECR( str, key );
			char* msg = ( char* )mymalloc( strlen( g_projectName ) + strlen( ex4Name ) + strlen( ( char* ) str ) + 16 );
			wsprintfA( msg, ( char* )str, g_projectName, ex4Name );
			g->unuse( ex4Name );
			ENCR( str, key );

			SetDlgItemTextA( dlg, IDC_MESSAGE, msg );
			free( msg );
			}

			{
			/*I don't have any open or pending Trade position(s). I know Auto Update may fail and I may need to contact vendor of %s directly to get update. I want to Update %s now fully automatic.*/
			BYTE key[4]={0x0e,0x29,0xb8,0x19};
			BYTE str[184]={0x21,0xc4,0xec,0x18,0x26,0x8a,0x34,0xe4,0x23,0xbe,0x12,0xdf,0xa2,0x6b,0x2e,0xdb,0x80,0xdf,0xcc,0xbe,0xf2,0xef,0x04,0xc3,0xcb,0xb5,0x49,0x14,0x38,0x64,0xc5,0x87,0xf1,0x21,0x2a,0x7b,0xf9,0x40,0xb2,0xcf,0x7a,0xdf,0xa4,0x5a,0x26,0xee,0x0b,0x94,0xf7,0x0d,0x5e,0xa5,0xce,0xad,0xdf,0xc7,0x79,0xdc,0x97,0x6b,0x7c,0x04,0x1b,0x34,0x91,0x7e,0xe2,0xed,0x4b,0x7a,0xdc,0x0f,0xf3,0xd4,0x95,0xee,0xa0,0x55,0xbc,0x53,0xac,0x2a,0x24,0x20,0x46,0xe0,0x33,0xc1,0x42,0xa6,0xc6,0x4f,0x1e,0xaf,0x00,0xd9,0x43,0x34,0x37,0x83,0x96,0xe6,0xb7,0x56,0x2e,0xcc,0x29,0xba,0x83,0xe4,0x8d,0x36,0x81,0xbf,0x45,0x54,0x25,0x58,0x77,0xb3,0x86,0x55,0x51,0x06,0xd4,0xe7,0xd1,0xb3,0x95,0x11,0xb0,0x1d,0x90,0xb9,0x56,0x82,0x99,0x92,0x3a,0xd0,0x60,0x51,0x72,0x4f,0x08,0x58,0x52,0x80,0x71,0x7a,0x57,0x10,0x21,0x47,0xb2,0x19,0x6f,0x57,0x0e,0xcf,0x4c,0x9f,0xe8,0x20,0xb7,0x0b,0xd4,0xd0,0xba,0x1b,0xed,0xee,0xa6,0x28,0x44,0x06,0x87,0xe2,0x39,0x98,0x4c,0x5b,0x64,0x4a};
			DECR( str, key );
			DWORD mem = strlen( ( const char* )str ) + 2 * strlen( g_projectName ) + 16;
			char* agr = ( char* )mymalloc( mem );
			wsprintfA( agr, ( const char* )str, g_projectName, g_projectName );
			SetDlgItemTextA( dlg, IDC_AGREEMENT, agr );
			free( agr );
			ENCR( str, key );
			}

			SetDlgItemTextA( dlg, IDC_STATUS, "" );
		}
			break;

		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDC_AGREE:
				{
					EnableWindow( GetDlgItem( dlg, IDOK ), SendDlgItemMessage( dlg, IDC_AGREE, BM_GETCHECK, 0, 0 ) == BST_CHECKED );
					break;
				}

				case IDOK:
				{
					{
						/*Verifying..*/
						BYTE key[4]={0x9f,0x19,0xaa,0xe5};
						BYTE str[12]={0x02,0xef,0xea,0x63,0x21,0x63,0xc3,0x49,0x5c,0xcf,0x3f,0x3b};

						DECR( str, key );
						SetDlgItemTextA( dlg, IDC_STATUS, ( const char* )str );
						ENCR( str, key );
					}
					timerProgress = TRUE;
					SetTimer( dlg, 1, 100, 0 );
					aud->dlg = dlg;
					aud->cancelled = FALSE;

					hUpdateThread = myCreateThread( ( LPTHREAD_START_ROUTINE )threadAutoUpdate, ( LPVOID )aud, "threadAutoUpdate" );
					
					EnableWindow( GetDlgItem( dlg, IDOK ), FALSE );
					EnableWindow( GetDlgItem( dlg, IDC_AGREE ), FALSE );

					/*Abort*/
					BYTE key[4]={0x10,0x4c,0x9d,0x0d};
					BYTE str[6]={0xdd,0x35,0xaa,0x16,0xa3,0xaf};

					DECR( str, key );
					SetDlgItemTextA( dlg, IDCANCEL, ( const char* )str );
					ENCR( str, key );
					break;
				}

				case IDCANCEL:
				{
					g_logger->submit( LGT_AU_CANCELLED );

					if( hUpdateThread )	//user cancelled update while files were downloading
					{
						EnableWindow( GetDlgItem( dlg, IDCANCEL ), FALSE );
						/*Aborting, please wait..*/
						BYTE key[4]={0x69,0x23,0x44,0x27};
						BYTE str[24]={0x73,0xdf,0xb0,0x00,0xc5,0x9d,0xb5,0x6e,0xfa,0xc0,0xe5,0x02,0xa0,0x0d,0x41,0x0b,0x2a,0xaa,0x8d,0xcb,0x87,0xaa,0x33,0x93};
						DECR( str, key );
						SetDlgItemTextA( dlg, IDCANCEL, ( const char* )str );
						ENCR( str, key );

						aud->cancelled = TRUE;
						//timer will continue be waiting for thread end
					}
					else
					{
						//remember last cancelled autoupdate time
						CMLReg reg;
						reg.open();

						SYSTEMTIME st;
						GetSystemTime( &st );
						DWORD now = systemTime2UnixTime( &st );
						reg.set( "au", ( void* )&now, sizeof( DWORD ), REG_DWORD );

						reg.close();

						EndDialog( dlg, 0 );
					}
					break;
				}

				case IDB_SUPPORT:
				{
					/*mailto:%s?subject=Support%%20Inquiry%%20for%%20%s&body=Hi,%%20my%%20ProjectID%%20is:%s...<Please%%20describe%%20your%%20inquiry%%20here>*/
					BYTE key[4]={0x36,0xfe,0xac,0xbc};
					BYTE str[137]={0x1a,0x4f,0xae,0xc7,0xbc,0xc1,0x07,0x0f,0x6d,0xf6,0x02,0x30,0xac,0x4f,0x7e,0x7f,0x84,0xe9,0x50,0xe1,0x2d,0x4d,0x0b,0xa1,0x78,0xc0,0x6c,0x8c,0xff,0x1d,0xda,0xe2,0x9e,0x9c,0x6b,0x8c,0x8d,0xc2,0xbb,0x5a,0x07,0x0e,0x73,0x82,0xc5,0x0d,0xe6,0x2f,0x8b,0x91,0x90,0xfa,0x75,0x4a,0x4b,0x89,0x64,0x3f,0x1e,0x26,0x35,0x7c,0x9f,0x65,0x8b,0xfa,0x33,0xf4,0x76,0x49,0x73,0x2a,0xa5,0xcf,0xfa,0xc6,0x2b,0xe3,0x77,0x8f,0xd5,0xbb,0x4d,0x34,0xc0,0xf4,0xbe,0xb9,0xa3,0xeb,0xcc,0x21,0x8b,0xc7,0x3f,0x34,0x1e,0xa7,0x23,0xea,0x04,0x88,0xc8,0x47,0xaa,0xeb,0x3f,0xe0,0xa5,0xc2,0xad,0xaf,0xe9,0xd8,0x67,0xf7,0xd9,0x81,0x7b,0x3b,0xe6,0x2b,0xf8,0x20,0x19,0x67,0x29,0xe8,0x7e,0x61,0x8d,0x0c,0xb2,0xfd,0x0e,0xe0,0x96};

					DECR( str, key );
					char *url = ( char* )mymalloc( 1024 );
					Globals* g = getGlobals();
					const char* gid = g->useId();
					wsprintfA( url, ( const char* )str, aud->supportEmail, g_projectName, gid );
					g->unuse( gid );
					ENCR( str, key );
dbg( "support link: '%s'", url );
					openLinkA( url );
					free( url );

					break;
				}
			}
			break;

		case WM_TIMER:
			if( WaitForSingleObject( hUpdateThread, 0 ) == 0 )
			{
dbg( "taud" );
				KillTimer( dlg, 1 );

				if( !aud->cancelled )
				{
					DWORD done;
					if( GetExitCodeThread( hUpdateThread, &done ) && done == 1 )
					{
						CMLReg reg;
						reg.open();
						reg.del( "au" );
						reg.close();

						PostMessage( getMt4Window(), WM_CLOSE, 0, 0 );
					}
					else
					{
						//auto update failed. display manual update dialog
						AutoUpdateNotificationData *aund = ( AutoUpdateNotificationData *)mymalloc( sizeof( AutoUpdateNotificationData ) );
						memset( aund, 0, sizeof( AutoUpdateNotificationData ) );
						strcpy( aund->supportEmail, aud->supportEmail );
						strcpy( aund->supportName, aud->supportName );

						g_logger->submit( LGT_AU_FAILED, done );

						preventUnloading();//WM_DESTROY allows unloading
						CloseHandle( modelessDialog( IDD_REMOTE_AUTO_UPDATE_NOTIFICATION, dlgRemoteAutoUpdateNotification, aund ) );
					}
				}
				CloseHandle( hUpdateThread );
				EndDialog( dlg, 0 );

				break;
			}
			if( timerProgress )
			{
				SendDlgItemMessage( dlg, IDC_PROGRESS, PBM_STEPIT, 0, 0 );
			}
			return( TRUE );

		case WM_USER + 1:
			{
dbg( "wmu+1" );
				totalBytes = wp;
				timerProgress = FALSE;
				SendDlgItemMessage( dlg, IDC_PROGRESS, PBM_SETPOS, 0, 0 );
			}
			return( TRUE );

		case WM_USER + 2:
			{
dbg( "wmu+2 %d", wp );
				/*%d Kbytes*/
				BYTE key[4]={0x76,0x2c,0x46,0xb7};
				BYTE str[10]={0x8c,0xc7,0x87,0xbe,0xa0,0x66,0x43,0x02,0xc1,0xed};
				char bytes[ 16 ];
				DECR( str, key );
				wsprintfA( bytes, ( const char* )str, ( DWORD )( wp / 1024 ) );
				ENCR( str, key );
				SetDlgItemTextA( dlg, IDC_STATUS, bytes );

				DWORD percent = ( DWORD )( wp * 100 / totalBytes );
				if( percent != prevPercent )
				{	
					prevPercent = percent;
					SendDlgItemMessage( dlg, IDC_PROGRESS, PBM_SETPOS, percent, 0 );
					char s[ 5 ];
					wsprintfA( s, "%d%%", percent );
					SetDlgItemTextA( dlg, IDC_PROGRESS_PERCENTS, s );
				}
			}
			return( TRUE );

		case WM_DESTROY:
			CloseHandle( aud->hMutex );
			free( aud );
			allowUnloading();
			break;
	}
	return( 0 );
} // dlgRemoteAutoUpdate

// =======================
// dlgRemoteAutoUpdateNotification
// =======================
INT_PTR CALLBACK dlgRemoteAutoUpdateNotification( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static AutoUpdateNotificationData *aund ;
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			HICON ico = ( HICON )LoadImage( g_hInstance, MAKEINTRESOURCE( IDI_AUTO_UPDATE ), IMAGE_ICON, 0, 0, 0 );
			SendDlgItemMessage( dlg, IDC_AUTO_UPDATE, STM_SETIMAGE, IMAGE_ICON, ( LPARAM )ico );
			Globals* g = getGlobals();			
			{
			/*Update required for %s - rev.%d - %s*/
			BYTE key[4]={0x46,0x94,0x79,0x0c};
			BYTE str[37]={0x19,0x6f,0x2e,0xad,0x5e,0x62,0x24,0x20,0xca,0xfe,0x0f,0x2f,0xfd,0x6b,0xe8,0x9a,0x10,0x59,0x15,0x16,0x5a,0xd7,0xfa,0x46,0x6f,0x87,0x4a,0x86,0xfd,0x4d,0xe4,0xad,0xe9,0x3a,0x62,0x53,0x56};
			DECR( str, key );
			char* title = ( char* )mymalloc( strlen( g_projectName ) + strlen( ( char* )str ) + 16 );
			const char* gid = g->useId();
			wsprintfA( title, ( char* )str, g_projectName, getGlobals()->getRevision(), gid + GEN_EXT_ID_LENGTH - 4 );
			g->unuse( gid );
			ENCR( str, key );

			SetWindowTextA( dlg, title );
			free( title );
			}
			
			aund = ( AutoUpdateNotificationData *)lp;
			{
			/*<a>%s</a>*/
			BYTE key[4]={0xfb,0x28,0xb9,0x2f};
			BYTE str[10]={0xc7,0xc3,0x8e,0xdc,0xbd,0xb9,0xed,0xc7,0xc2,0xf6};
			char* href = ( char* )mymalloc( 128 );
			DECR( str, key );
			wsprintfA( href,( const char* )str, aund->supportEmail );
			ENCR( str, key );

			SetDlgItemTextA( dlg, IDC_EMAIL, href );
			free( href );
			}
			
			{
			/*Your EA/Indicator "%s" with filename "%s.ex4" needs an update. This update cannot be done automatically at this moment. Please contact vendor of this project and request your update.*/
			BYTE key[4]={0x3a,0xfd,0x90,0x72};
			BYTE str[183]={0x2f,0xcf,0x1e,0x79,0x6d,0x38,0xed,0x92,0xe7,0x18,0x02,0x91,0xea,0xef,0x9d,0xa9,0xcf,0x76,0x9f,0xa9,0x69,0x16,0x6e,0x7d,0xca,0x91,0x9c,0x66,0x10,0x81,0x7d,0xce,0x21,0xbb,0x78,0xdc,0x0a,0x56,0xcb,0x1f,0x32,0x9e,0xee,0x5c,0x64,0xb6,0xc6,0xab,0x82,0xdf,0x4c,0xad,0xb4,0x2a,0x0c,0x0d,0x8d,0x26,0x6f,0x24,0x79,0xe1,0xdc,0xdf,0x6d,0x26,0xd4,0x4a,0xad,0xdf,0xd9,0x51,0xaf,0x49,0x50,0x7f,0x2c,0xac,0xe1,0x8a,0x45,0xaa,0xd9,0x91,0x43,0x25,0x43,0x87,0x06,0x85,0xc3,0x3b,0xc6,0x14,0xa7,0xcf,0xd5,0x68,0x4b,0x24,0x07,0x2a,0x00,0x24,0x0d,0xa7,0x7c,0x78,0xc3,0xec,0x3c,0x35,0x00,0x11,0x08,0xd7,0x34,0x11,0x69,0x41,0xf8,0x07,0x09,0xdf,0xf1,0xa1,0x9b,0x89,0xb1,0xc6,0x17,0x14,0x17,0xbc,0x9d,0xc1,0x40,0x09,0x4a,0x50,0xaa,0xae,0xb3,0xe7,0xa5,0x89,0x93,0x05,0xa6,0xe0,0x6a,0x72,0xe0,0x81,0x05,0xeb,0x93,0xca,0x65,0xcb,0x7b,0xb0,0x53,0xa3,0xc6,0xd5,0x27,0xdd,0x3b,0x8f,0xc7,0xbd,0x00,0x0d,0x0a,0x81,0xad,0x62,0xaf,0xa5,0x26,0x61,0x17};
			

			char* msg = ( char* )mymalloc( 1024 );
			const char* ex4Name = g->useEx4Name();
dbg( "ex4 %08x", ex4Name );
			while( !*ex4Name )
			{
				Sleep( 1000 );
				ex4Name = g->useEx4Name();
dbg( "ex4 %08x", ex4Name );
			}
			
			DECR( str, key );
			const char* gid = g->useId();
			wsprintfA( msg, ( char* )str, g_projectName, ex4Name );

			SetDlgItemTextA( dlg, IDC_NAME, aund->supportName );
			SetDlgItemTextA( dlg, IDC_PID, gid );

			g->unuse( gid );
			g->unuse( ex4Name );
			ENCR( str, key );

			SetDlgItemTextA( dlg, IDC_MESSAGE, msg );
			free( msg );
			}


		}
			break;

		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDCANCEL:
				{
					//remember last cancelled autoupdate notification time
					CMLReg reg;
					reg.open();

					SYSTEMTIME st;
					GetSystemTime( &st );
					DWORD now = systemTime2UnixTime( &st );
					reg.set( "aun", ( void* )&now, sizeof( DWORD ), REG_DWORD );

					reg.close();

					EndDialog( dlg, 0 );
					break;
				}

				case IDB_SUPPORT:
				{
					/*mailto:%s?subject=Support%%20Inquery%%20for%%20%s&body=Hi,%%20my%%20ProjectID%%20is:%s...<Please%%20describe%%20your%%20inquiry%%20here>*/
					BYTE key[4]={0xb3,0x66,0x17,0xd9};
					BYTE str[137]={0xb3,0x92,0x1c,0xef,0x52,0xcc,0xb0,0x3a,0x83,0xc6,0xf8,0xb0,0x9d,0xc3,0x92,0xe0,0x1f,0x2a,0x7f,0x7d,0x4c,0x1e,0x1a,0x45,0x64,0x72,0xb9,0xcb,0xf3,0xfb,0x30,0x6a,0x1a,0x33,0x97,0x99,0x2b,0xcc,0x06,0x3c,0xb5,0xd7,0x0f,0x28,0x58,0x5f,0x04,0xc5,0xa0,0x0e,0x98,0xfa,0x48,0x4b,0x84,0xfd,0x17,0xfc,0xd1,0x18,0xd8,0xf9,0xb4,0x4e,0xfc,0x0a,0xe5,0xe4,0xa0,0xdb,0xd4,0x24,0xfe,0xec,0x64,0xfd,0xb8,0x76,0x97,0x93,0xb5,0xe1,0x44,0x3b,0xca,0x39,0xec,0x16,0x3a,0xb0,0xe0,0x08,0xfd,0x83,0x73,0xf7,0x01,0x14,0x17,0xb1,0xb6,0x77,0x22,0xd2,0x59,0x8b,0xf2,0x50,0x06,0x8c,0x46,0x85,0x82,0x4a,0x9a,0x1b,0x11,0xd5,0x04,0x65,0x0f,0xd2,0x03,0x50,0xad,0xca,0x45,0x5f,0xad,0x6c,0x1f,0x88,0x9a,0x47,0xe5,0x58,0xd7};

					DECR( str, key );
					char *url = ( char* )mymalloc( 1024 );
					Globals* g = getGlobals();
					const char* gid = g->useId();
					wsprintfA( url, ( const char* )str, aund->supportEmail, g_projectName, gid );
					g->unuse( gid );
					ENCR( str, key );
dbg( "sl %s", url );
					openLinkA( url );
					free( url );

					break;
				}
			}
			break;


		case WM_DESTROY:
			free( aund );
			allowUnloading();
			break;
	}

	if( ( msg == WM_NOTIFY && wp == IDC_EMAIL && ((LPNMHDR)lp)->code == NM_CLICK ) ||
		( msg == WM_COMMAND && LOWORD( wp ) == IDOK )
	)
	{
		/*mailto:%s?subject=Support%%20Inquery%%20for%%20%s&body=Hi,%%20my%%20ProjectID%%20is:%s...<Please%%20describe%%20your%%20inquiry%%20here>*/
		BYTE key[4]={0x89,0x95,0xed,0xe1};
		BYTE str[137]={0xb4,0x69,0x32,0xe2,0x1b,0x7f,0xde,0xd2,0x35,0xaf,0x2b,0xc2,0xb8,0xb5,0xed,0xd7,0x98,0x51,0x9c,0xa2,0xa7,0x70,0xbb,0xb4,0x87,0xb3,0xee,0x4a,0xd7,0x75,0x8b,0xe9,0x3c,0x11,0xd6,0x71,0xe9,0xa7,0x74,0x7d,0xce,0x5c,0x39,0x25,0x73,0xad,0x97,0x6f,0xb2,0x17,0x20,0xfc,0x00,0x06,0x20,0x9f,0x09,0xc0,0xd2,0x12,0x07,0xdb,0xf8,0x78,0xf1,0xac,0x76,0x23,0x75,0x9a,0xbe,0x91,0xab,0xde,0x9d,0x03,0x29,0x9a,0x9a,0x58,0x92,0x7a,0xc4,0xc5,0x81,0xf4,0x37,0x3c,0xeb,0x0f,0xdf,0x8c,0x46,0xf6,0xb6,0xab,0x12,0x8f,0xef,0x7e,0xf4,0xce,0x1c,0xe2,0x36,0xc7,0x01,0x07,0x71,0x08,0xe3,0xee,0xc1,0x6e,0x29,0xf8,0x14,0x4f,0xdb,0xa5,0xb9,0xa3,0x09,0xb2,0x01,0x22,0xcd,0xd8,0xde,0x7e,0x8e,0x02,0x97,0x20,0xd4,0x17,0x9f};

		DECR( str, key );
		char *url = ( char* )mymalloc( 1024 );
		Globals* g = getGlobals();
		const char* gid = g->useId();
		wsprintfA( url, ( const char* )str, aund->supportEmail, g_projectName, gid );
		g->unuse( gid );
		ENCR( str, key );
		openLinkA( url );
		free( url );
	}

	return( 0 );
} // dlgRemoteAutoUpdateNotification

// ==================
// modelessThread
// ===================
DWORD WINAPI modelessThread( ModelessData* md )
{
	TRY
	DialogBoxParam( g_hInstance, MAKEINTRESOURCE( md->idd ), 0, md->proc, ( LPARAM )md->param );
	delete md;
	CATCH
	return( 0 );
} // modelessThread

HANDLE modelessDialog( DWORD idd, DLGPROC proc, LPVOID param )
{
	ModelessData* md = new ModelessData;
	md->idd = idd;
	md->proc = proc;
	md->param = param;
	return( myCreateThread( ( LPTHREAD_START_ROUTINE )modelessThread, md, "modelessThread" ) );
}

// =========
// stom
// =========
WORD stom( const char *str )
{
	if( strlen( str ) >= 3 )
	{
		static char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
		for( WORD i = 0; i < 12; i ++ )
		{
			if( !memcmp( str, months[ i ], 3 ) )
			{
				return( i + 1 );
			}
		}
	}
	return( 0 );
} // stom

// ========================
// fetchServerTimeShift
// ========================
__int64 fetchServerTimeShift( HINTERNET hr )
{
	__int64 res;
	SYSTEMTIME st;
	DWORD sz = sizeof( st );
	if( HttpQueryInfo( hr, HTTP_QUERY_DATE | HTTP_QUERY_FLAG_SYSTEMTIME, &st, &sz, 0 ) )
	{
		res = systemTime2UnixTime( &st );
		res *= 1000;
		__int64 now64 = myGetTickCount64();
dbg( "fst %I64u", res, now64 );
		res -= now64;
	}	
	else
	{
		res = 0;
	}
	return( res );
} // fetchServerTimeShift

// =================
// genSalt
// =================
void genSalt( OUT char** rslt )
{
	*rslt = ( char* )malloc( 33 );
	static char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$!_^*()";
	for( int i = 0; i < 32; i ++ )
	{
		( *rslt )[ i ] = chars[ rand() % ( sizeof( chars ) - 1 ) ];
	}
	( *rslt )[ 32 ] = 0;
} // genSalt

// ==================
// sendRedisStats
//
// We tried to implement this on server side on auth request, 
// but all servers are hit at once and that's why we can't be sure that stats are written only once.
// So we send stats to winning server only
// ==================
void sendRedisStats( CBaseServerManager* mgr, const char* server, INTERNET_PORT port, const char* baseUri, DWORD success, const char* cre1, const char* cre2 )
{
	/*%s/lic.php?p=stats&lic=%s&rev=%d&acc_no=%s&success=%d&cre1=%s&cre2=%s*/
	BYTE urlKey[4]={0xa6,0x7c,0x7c,0x6a};
	BYTE urlFmt[70]={0x99,0x88,0xfa,0x92,0x41,0x27,0xcb,0xab,0xc2,0x72,0xaa,0x2a,0xcc,0x29,0x7c,0x67,0x0f,0x84,0x53,0x31,0x0e,0xdc,0x2d,0xc0,0xd7,0x96,0x5a,0xde,0x1c,0xc5,0xf0,0xa8,0xba,0x71,0xf1,0x49,0x8f,0x2d,0x96,0x70,0xfb,0x42,0x60,0x2d,0x04,0x21,0xfd,0xd8,0x3a,0xe4,0xb8,0x94,0xe3,0xb7,0xd5,0x54,0xe7,0x3c,0x93,0x00,0x3b,0x2b,0x82,0x6d,0x74,0x7a,0x2b,0x77,0x0d,0xd8};
	
	char *uri = ( char* )mymalloc( 512 );

	Globals* g = getGlobals();

	CRC4 rc4;
	rc4.Decrypt( urlFmt, sizeof( urlFmt ), urlKey, sizeof( urlKey ) );

	const char* licId = g->useLicId();

	wsprintfA( uri, ( char* )urlFmt, baseUri, licId, g->getRevision(), g_ac->getNumber( TRUE ), success, cre1, cre2 );

	g->unuse( licId );
	rc4.Encrypt( urlFmt, sizeof( urlFmt ), urlKey, sizeof( urlKey ) );

	CBaseServerManagerHandle *h = mgr->load( server, port, uri, TRUE );
	free( uri );

	h->wait( 5000 );
	delete h;
} // sendRedisStats

char* str2bin( const BYTE* s, int len )
{
	char* res = ( char* )malloc( len * 3 + 1 );
	for( int i = 0; i < len; i ++ )
	{
		sprintf( res +  i * 3, "%02x ", ( DWORD )s[ i ] );
	}
	return( res );
}

// ======================
// threadLoadLic2
// ======================
DWORD WINAPI threadLoadLic2( ThreadLoadLicData* lld )
{
	char server[ 100 ];
	INTERNET_PORT port;
	char baseUri[ 100 ];
	char scheme[ 32 ];

	CBaseServerManager mgr;
	lld->internetRes = ML_UNKNOWN;

fdbg( "fd %s", lld->server );
dbg( "fd %s", lld->server );

	parseUrl( lld->server, server, &port, baseUri, scheme );

	/*%s/lic.php?lic=%s&rev=%d&rslt=%s&acc_no=%s*/
	BYTE urlKey[4]={0x77,0xd5,0x5b,0x1c};
	BYTE urlFmt[43]={0x78,0x74,0xa7,0x81,0x56,0xe6,0xc6,0x55,0x1c,0xe0,0x04,0x48,0x53,0x0c,0xce,0x7a,0x90,0x34,0x34,0x67,0x38,0x31,0xe4,0xf0,0x26,0x43,0x6a,0xfd,0xb3,0xf6,0x77,0x14,0xc2,0x61,0x78,0x2d,0xfa,0x01,0x67,0x45,0xff,0x87,0x0e};
	
	char *uri = ( char* )mymalloc( 512 );

	Globals* g = getGlobals();

	//random salt for encrypting xml nodes and values
	char *rslt;
	genSalt( &rslt );

	CRC4 rc4;
	rc4.Decrypt( urlFmt, sizeof( urlFmt ), urlKey, sizeof( urlKey ) );

	const char* licId = g->useLicId();

	wsprintfA( uri, ( char* )urlFmt, baseUri, licId, g->getRevision(), rslt, g_ac->getNumber( TRUE ) );
	g->unuse( licId );
	rc4.Encrypt( urlFmt, sizeof( urlFmt ), urlKey, sizeof( urlKey ) );

fdbg( "u %s %s:%d", uri, server, port );
dbg( "u %s %s:%d", uri, server, port );

	CBaseServerManagerHandle *h = mgr.load( server, port, uri, TRUE );
	free( uri );

	DWORD waited;
	for( ;; )
	{
		waited = h->wait( 100 );

fdbg( "w %d %d %d", waited, h->error, h->status );
dbg( "w %d %d %d", waited, h->error, h->status );
		if( isTimePassed( lld->initStart, MAX_AUTH_DURATION * 1000 ) || lld->forceStop )
		{
fdbg( "ht %d", lld->forceStop );
dbg( "ht %d", lld->forceStop );
			h->abort();
			delete h;
		
			return( 0 );
		}

		if( waited != WAIT_TIMEOUT )
		{
			break;
		}
	}

	if( waited == WAIT_OBJECT_0 )
	{
		lld->internetRes = h->error;

		if( lld->internetRes == ML_OK )
		{
			if( h->status == 200 )
			{
				if( WaitForSingleObject( lld->mutex, 0 ) == 0 )
				{
fdbg( "encs" );
dbg( "encs" );
					//get server time to work with it as time base ( if not google time which is more trustworth )
					if( !g->serverTimeShift )
					{
						g->serverTimeShift = fetchServerTimeShift( h->hr );
fdbg( "gst %I64u", g->serverTimeShift );
dbg( "gst %I64u", g->serverTimeShift );
					}
					DWORD sz;

					//read lic
					char* raw = ( char* )mymalloc( h->cl + 1 );
					if( InternetReadFile( h->hr, raw, h->cl, &sz ) && sz == h->cl )
					{	
	fdbg( "rd %d", h->cl );
	dbg( "rd %d", h->cl );
	//						dbg( "rii1" );//removing indItem
	//						g_ind->stop( indItem );	

						lld->internetRes = ML_OK;

						// --------------
						// decrypt xml
						// --------------
						CRC4 rc4;
	fdbg( "dcr%d", 1 );
	dbg( "dcr%d", 1 );
						const BYTE* lk = g->useLicenseKey();
char* s = str2bin( lk, GEN_LICENSE_KEY_LENGTH );
fdbg( "lk: %s", s );
free(s);

s = str2bin( ( const BYTE* )raw, h->cl );
fdbg( "raw: %s", s );
free(s);

						rc4.Decrypt( ( BYTE* )raw, h->cl, lk, GEN_LICENSE_KEY_LENGTH );
s = str2bin( ( const BYTE* )raw, h->cl );
fdbg( "raw decrypted: %s", s );
free( s );
						//generate key for decrypting xml nodes and values
						BYTE* xmlKey = ( BYTE* )malloc( GEN_LICENSE_KEY_LENGTH + 32 );
						memcpy( xmlKey, rslt, 16 );
						memcpy( xmlKey + 16, lk, GEN_LICENSE_KEY_LENGTH );
						memcpy( xmlKey + 16 + GEN_LICENSE_KEY_LENGTH, rslt + 16, 16 );
						free( rslt );
						rslt = 0;

						g->unuse( ( const char* )lk );

	fdbg( "dcr%d", 2 );
	dbg( "dcr%d", 2 );
						raw[ h->cl ] = 0;
	/*HANDLE h = CreateFile( "c:\\raw_xml.xml", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
	WriteFile( h, raw, sz, &sz, 0 );
	CloseHandle( h );
	*/

	fdbg( "xl '%s'", raw );
	dbg( "xl '%s'", raw );

						// -------------
						// parse xml
						// -------------
						CLic lic;
						BOOL xmlOk = lic.load( raw, xmlKey, GEN_LICENSE_KEY_LENGTH + 32 );
						free( raw );
						free( xmlKey );
						raw = 0;
fdbg( "xm %d", xmlOk );
dbg( "xm %d", xmlOk );
						if( xmlOk )
						{
							g->ping = lic.ping;
							g->setVendorName( lic.supportName );
							g->setVendorEmail( lic.supportEmail );
							g->setProductUrl( lic.websiteUrl );

							// ---------------------------------------------
							// 1+2. checking project and revision expiration
							// ---------------------------------------------
fdbg( "lic.e %d", lic.expiration );
dbg( "lic.e %d", lic.expiration );
							g->setExpirationDate( lic.expiration );
							if( !g->getExpirationDate() )
							{
fdbg( "npe" );
dbg( "npe" );
								g->setExpirationDate( -1 );
							}

							if( !isProjectExpired() )
							{
								// -------------------
								// 3. hold
								// -------------------
								if( !lic.hold )
								{
	fdbg( "!lic.h" );
	dbg( "!lic.h" );
									if( lic.isValid )
									{
	fdbg( "lv %d", lic.type );
	dbg( "lv %d", lic.type );
										DWORD accType;
										BOOL real = g_ac->isReal( TRUE );
	fdbg( "atr %d", real );
	dbg( "atr %d", real );

										/*<demo>*/
										BYTE keyDemo[4]={0x17,0xcf,0x5f,0xb2};
										BYTE strDemo[7]={0xc8,0x42,0x87,0xe1,0xfd,0x77,0xfb};
										DECR( strDemo, keyDemo );

										// ----------------------
										//		LT_REMOTE_AUTO
										// ----------------------
										/*if( lic.type == LT_REMOTE_AUTO )
										{
											// this value is being tracked from tryInit also.
											// Since some time passed already for xml downloading need to reset g_initStart for l&p dialog
											lld->initStart = GetTickCount();

											lic.userData = &lld->initStart;
											ERROR_TYPE loginRes = ( ERROR_TYPE )DialogBoxParam( g_hInstance, MAKEINTRESOURCE( IDD_REMOTE_AUTO_LOGIN ), 0, dlgRemoteAutolicense, ( LPARAM )&lic );
											switch( loginRes )
											{
												case -1://registration requested 
													break;

												case E_CANCELLED:
													criticalError( E_CANCELLED );
													break;

												case E_OK:
													g->setAutolicenseData( lic.al.maxInstances, lic.lastCre1 );
													lld->res = 1;
													break;

												case E_TIMEOUT:
												{
													/*Your %s could not authenticate. Please make sure you type correct login & password. If you have not registered yet, please register first. You may try to contact vendor with %s to get more help. Make sure you send vendor a screenshoot of error message, also mention your login name.* /
													BYTE key[4]={0xca,0x35,0x25,0x30};
													BYTE str[283]={0x46,0x3b,0xad,0x5f,0x59,0x0e,0xd9,0xa7,0x84,0xd2,0xef,0xe0,0x23,0x82,0xaf,0xd6,0x22,0xa4,0x19,0x68,0x49,0x78,0x1a,0x50,0x79,0xf6,0x0f,0xc5,0x09,0x59,0x80,0x3e,0xf6,0x95,0xc6,0x0c,0x26,0xe8,0x29,0xd0,0x51,0xca,0x99,0x7e,0x99,0xe3,0xd9,0x16,0xba,0xa9,0xa2,0xf7,0x13,0x5f,0xcf,0xa8,0x87,0xd5,0xfb,0x67,0x94,0xc6,0xe1,0x89,0xb0,0xf6,0xad,0x05,0x80,0x4c,0x69,0x7b,0xfd,0xe7,0x8c,0xb0,0xae,0x21,0x5e,0x23,0xdf,0xbc,0x53,0x74,0x90,0x8b,0x8d,0x2f,0xfa,0xd6,0xca,0xe9,0x3f,0x1b,0xea,0x5a,0x64,0xf5,0xc2,0xe0,0xa1,0x9a,0x18,0x61,0xa2,0xef,0x20,0x41,0x3e,0x0c,0x15,0x55,0xbf,0x81,0x84,0x9d,0x99,0xb0,0x0b,0x64,0xdc,0x6e,0x2b,0x8e,0x19,0x6a,0x14,0xe7,0x5f,0x4e,0x38,0xfd,0x76,0x24,0xfa,0xc1,0x62,0xcb,0x07,0xa1,0x89,0x78,0xb0,0x44,0xee,0x43,0x1c,0x5d,0xaa,0x14,0x9a,0x5b,0xee,0x1f,0xf5,0xae,0xa4,0x4a,0xde,0x0d,0xf8,0x06,0xf3,0x6f,0x50,0x10,0x73,0xd6,0x15,0x65,0xfe,0x22,0xf3,0x5e,0xae,0x3d,0x57,0x43,0xf6,0x1f,0xbe,0x2a,0x33,0xc4,0x57,0x6e,0x77,0x0b,0xb5,0x16,0x35,0x18,0x9a,0xc4,0xc2,0xa8,0x57,0xc5,0x1b,0x88,0xc2,0xb0,0x3e,0x7f,0x1b,0x04,0x08,0x33,0x03,0x55,0x9f,0xbb,0xe3,0x3c,0x8b,0x46,0x12,0x32,0x8e,0x91,0xb5,0xbb,0xe9,0xf6,0x50,0xa4,0xb1,0xc2,0x40,0x38,0x76,0x1f,0xf4,0x02,0x5e,0xcc,0xa5,0x7b,0x0e,0x3d,0x03,0xd7,0x82,0x18,0xc3,0x39,0xc8,0xe8,0xd1,0xca,0xde,0x88,0xd7,0xfd,0xf7,0xe2,0x02,0xda,0x80,0x6b,0x78,0x1f,0x0f,0x97,0x7b,0xcb,0x5a,0x68,0x45,0x06,0xbe,0x5d,0x63,0x8b,0xb5,0x9e,0xd2,0x03,0x38,0xe2,0x58,0x73,0xd5};

													DECR( str, key );
													char* text = ( char *)mymalloc( 512 );
													Globals* g = getGlobals();
													wsprintfA( text, ( char* )str, g_projectName, g->getVendorEmail() );
													ENCR( str, key );
								
													char* title = ( char* )mymalloc( 256 );
													DECR2( g_authenticationFailedTitleStr, g_authenticationFailedTitleKey );
													wsprintfA( title, ( char* )g_authenticationFailedTitleStr, g_projectName );
													ENCR( g_authenticationFailedTitleStr, g_authenticationFailedTitleKey );								 

													criticalError( text, title );
													free( text );
													free( title );

													break;
												}
												
												case E_ACCOUNT_DISABLED:
												{
													/*Your license is disabled by vendor. Please contact vendor for activation.* /
													BYTE key[4]={0x19,0xa8,0x08,0xad};
													BYTE str[74]={0xa1,0x0c,0xcb,0xd4,0xb3,0xf6,0xe9,0xb0,0xd0,0x38,0x81,0x4f,0x3a,0x2c,0x56,0x8c,0x15,0xb6,0xa5,0xcc,0xdc,0xdd,0x97,0xbe,0xe5,0x15,0xa3,0xdb,0x87,0x34,0xf1,0x31,0x17,0x14,0xcd,0x09,0xd3,0x2a,0xa8,0xcb,0x48,0xce,0x0d,0x9a,0x21,0xd7,0xf9,0x4a,0x40,0x7d,0x84,0xc7,0xd1,0x07,0x59,0xe2,0x16,0x90,0x72,0x40,0x4e,0x84,0x5e,0x89,0xb4,0x67,0xb7,0x44,0xd3,0xed,0xcf,0x67,0x53,0x85};

													criticalError( E_WITHINFO, str, sizeof( str ), key, sizeof( key ), FALSE );
													break;
												}

												case E_USAGE_LIMIT://license usage limit is reached
												{
													/*Your license is not enough to run '%s' on this computer because this license is already activated on %d different computers, %d account numbers and %d instances started on Your computer. Your license allows you to run this project on %d different computers and %d account numbers. %d instances allowed to run on Your computer at the same time. In order to continue you need either to stop '%s' on other computer or on other account number, or to stop one of instances on Your computer.* /
													BYTE key[4]={0x5b,0x32,0xc6,0x71};
													BYTE str[486]={0x20,0x45,0x84,0x44,0xc4,0xd6,0x33,0xfe,0x9b,0xa9,0xff,0xea,0x8b,0x13,0xed,0x1d,0x9e,0x70,0xd7,0xf0,0x78,0x85,0xf0,0x44,0x81,0x02,0xfd,0xff,0x7f,0x8c,0xfb,0x26,0xdc,0x3a,0xbb,0xa4,0xb5,0x70,0x25,0xca,0x87,0xec,0xb3,0xc8,0xf4,0x19,0xce,0xb8,0x77,0x89,0x4a,0x71,0xeb,0xc4,0xc6,0x4b,0xb3,0x28,0x38,0x5c,0x20,0xd9,0x17,0x16,0xf7,0x66,0xb6,0xf4,0xe5,0x4a,0x63,0x71,0x2a,0x0e,0xa8,0x15,0x32,0x2e,0x6f,0x20,0x78,0xc3,0x98,0x96,0x20,0xb3,0x1b,0xf8,0x4e,0xaa,0x7d,0xc4,0xf5,0xbb,0x25,0xd5,0xdc,0x44,0x63,0x92,0x50,0xfb,0x15,0x85,0x65,0x2c,0x7e,0xae,0x68,0xa8,0x1b,0x1d,0xc3,0x42,0x20,0x27,0x14,0x13,0x40,0x24,0x03,0x4b,0xa8,0xe6,0xc1,0x46,0x1d,0x78,0x43,0x25,0x2a,0xff,0xc8,0x2b,0x12,0x03,0x50,0x58,0xf3,0x13,0x04,0xbb,0xf0,0xa3,0xc2,0xd0,0x89,0x84,0x5d,0x82,0x99,0x8b,0x45,0xdc,0x9e,0x1c,0x4f,0x2a,0x5f,0x1d,0x84,0x6d,0x1f,0x89,0x4a,0xa0,0xb6,0xa0,0x31,0x37,0x4e,0x6c,0x45,0x36,0xcf,0x01,0xdc,0x50,0x13,0x8c,0xd7,0x8c,0xb2,0x2d,0x75,0xa1,0x0d,0xa6,0xa1,0x14,0x78,0x1d,0x08,0x31,0x1d,0x32,0xa4,0x8e,0xe1,0x07,0xbf,0x1b,0xc2,0x50,0x41,0x84,0xf3,0x95,0x70,0xdb,0xda,0x9a,0xb3,0xab,0x22,0x83,0x93,0x45,0x3c,0xb1,0x11,0x77,0x94,0x86,0x78,0x9f,0xbe,0x90,0x6a,0xb9,0x20,0xf3,0xe2,0x96,0x08,0x4c,0xfe,0xd8,0x38,0x4d,0xcd,0xe7,0x54,0x99,0xc1,0x15,0xec,0xa1,0xd1,0x93,0xa8,0x2e,0xd0,0x71,0xec,0x35,0x9e,0xa2,0xf8,0x59,0x78,0x62,0xc6,0xcc,0x6d,0x07,0x3b,0x61,0xe8,0x02,0xb9,0x2b,0x55,0xfb,0x57,0xea,0x21,0x42,0xa0,0x4d,0xe0,0x8f,0xcb,0xb8,0xb8,0xaa,0x1a,0xb1,0x15,0x3c,0x7b,0x29,0x28,0x53,0xef,0x64,0x6a,0x2b,0x5e,0xab,0x97,0xd4,0x07,0x90,0xa8,0x96,0x6a,0xcb,0xbb,0x3a,0x5a,0xe1,0xbb,0xa7,0x33,0x54,0xc1,0x88,0x57,0xac,0xd2,0xb3,0xc4,0xae,0x15,0x05,0x00,0x76,0x0a,0xcf,0x83,0x1e,0xe5,0xe0,0x1a,0xe2,0xef,0xc1,0x5b,0xe1,0x8d,0x19,0x0d,0x76,0xf6,0x98,0x0c,0xdf,0x74,0x98,0x82,0xd5,0x53,0x2e,0x9c,0xd7,0x8d,0xef,0x5f,0x04,0x43,0x4d,0x7f,0xd6,0x3d,0x84,0x74,0x87,0x65,0x16,0xa8,0xce,0x15,0xff,0x57,0xa9,0x6f,0x21,0x0d,0x8d,0xde,0x58,0xf9,0x40,0x66,0x94,0x02,0x6b,0x92,0xfb,0x76,0xb5,0x82,0x90,0x7d,0x3a,0x9e,0xb3,0x8d,0xfd,0xf9,0x15,0x6b,0x61,0x1e,0x6d,0xed,0xf3,0xe7,0xd7,0x12,0x5d,0xeb,0x4d,0x62,0x71,0xd8,0x16,0x85,0x04,0xde,0x05,0x86,0x04,0xff,0xc9,0x1a,0xdc,0x5b,0xc0,0x32,0x7e,0x88,0x7c,0x7c,0xba,0x9d,0x27,0x30,0xb1,0x86,0x06,0x15,0x45,0xd6,0xa0,0xb0,0x06,0x41,0x88,0xfa,0x5f,0xee,0x9b,0xdc,0x68,0x4d,0x7d,0x10,0x9c,0xea,0xda,0x68,0x10,0x8c,0x18,0x3f,0xe1,0x8f,0x91,0x5d,0x96,0x09,0xc9,0x47,0xbf,0xee,0x2c,0xd0,0xfd,0x62,0xa0,0x43,0x60,0x08,0x73};

													char* err = ( char* )malloc( 1024 );
													DECR( str, key );
													//usedCids is calculated excluding current computer, so if project is not started on any other computer then usedCids is 0, which is not correct because it is started on current computer
													wsprintfA( err, ( char* )str, g_projectName, lic.al.usedCids ? lic.al.usedCids : 1, lic.al.usedAccounts, lic.al.usedInstances, lic.al.maxCids, lic.al.maxAccounts, lic.al.maxInstances, g_projectName );
													ENCR( str, key );
													criticalError( err );
													free( err );
													break;
												}

												case E_ACCOUNT_EXPIRED:
												{
													g->setExpirationDate( lic.al.userExpiration );
													g->expirationReason = EXPIRATION_REASON_USER;
													break;
												}
											}

										}
										// ---------------------------------
										//			LT_REMOTE_LOGIN
										// ---------------------------------
										else */if( lic.type == LT_REMOTE_LOGIN )
										{
											if( lic.skipForDemo && !real )
											{
												//skip_for_demo lic option is set and current account is demo
												strcpy( lic.lastCre1, ( const char* )strDemo );
												strcpy( lic.lastCre2, ( const char* )strDemo );
												lld->res = 1;
											}
											else
											{
												CLogin l( LT_REMOTE_LOGIN );
												if( l.load() == 0 && lic.isValidLogin2( &l, lld ) )
												{
											fdbg( "srlpv %d", l.accType );
											dbg( "srlpv %d", l.accType );
													accType = l.accType;
									
													if( l.expiration )
													{
														if( g->getExpirationDate() == -1 || g->getExpirationDate() > l.expiration )
														{
															g->setExpirationDate( l.expiration );
															g->expirationReason = EXPIRATION_REASON_USER;
														}
														if( !isProjectExpired() )
														{
															lld->res = 1;
														}
													}
													else
													{
														lld->res = 1;
													}
													if( lld->res == 1 )
													{
														strcpy( lic.lastCre1, l.login );
														strcpy( lic.lastCre2, l.pass );
													}
												}
												else
												{
													// this value is being tracked from tryInit also.
													// Since some time passed already for xml downloading need to reset g_initStart for l&p dialog
													lld->initStart = GetTickCount();

													lic.userData = lld;
													DWORD loginRes = DialogBoxParam( g_hInstance, MAKEINTRESOURCE( IDD_REMOTE_LOGIN ), 0, dlgRemoteLogin2, ( LPARAM )&lic );
													if( loginRes == 1 )
													{
														accType = lic.accType;		// filled in dlgRemoteLogin

														if( lic.userExpiration )	// filled in dlgRemoteLogin
														{
															if( g->getExpirationDate() == -1 || g->getExpirationDate() > lic.userExpiration )
															{
																g->setExpirationDate( lic.userExpiration );
																g->expirationReason = EXPIRATION_REASON_USER;
															}
															if( !isProjectExpired() )
															{
																lld->res = 1;
															}
														}
														else
														{
															lld->res = 1;
														}
													}
													else if( loginRes == 2 )
													{
														criticalError( E_TIMEOUT );
													}
													else	//0 => user canceled l/p input
													{
														criticalError( E_CANCELLED );
													}
												}
											}
										}
										// ----------------------------
										//		LT_REMOTE_RECEIPT
										// ----------------------------
										else if( lic.type == LT_REMOTE_RECEIPT )
										{
								fdbg("rcpt");
								dbg("rcpt");
											if( lic.skipForDemo && !real )
											{
												//skip_for_demo lic option is set and current account is demo
												strcpy( lic.lastCre1, ( const char* )strDemo );
												lld->res = 1;
											}
											else
											{
									fdbg( "rld %08x", lld );
									dbg( "rld %08x", lld );
												CReceipt r;
												if( r.load() == 0 && lic.isValidReceipt2( &r, lld ) )
												{
									fdbg( "rcpvld" );
									dbg( "rcpvld" );
													accType = r.accType;

													if( r.expiration )
													{
														if( g->getExpirationDate() == -1 || g->getExpirationDate() > r.expiration )
														{
															g->setExpirationDate( r.expiration );
															g->expirationReason = EXPIRATION_REASON_USER;
														}
														if( !isProjectExpired() )
														{
															lld->res = 1;
														}
													}
													else
													{
														lld->res = 1;
													}

													if( lld->res == 1 )
													{
														strcpy( lic.lastCre1, r.receipt );
													}
												}
												else
												{
										fdbg( "rcptinv" );
										dbg( "rcptinv" );
													// this value is being tracked from tryInit also.
													// Since some time passed already for xml downloading need to reset g_initStart for l&p dialog
													lld->initStart = GetTickCount();

													lic.userData = lld;
													DWORD receiptRes = DialogBoxParam( g_hInstance, MAKEINTRESOURCE( IDD_REMOTE_RECEIPT ), 0, dlgRemoteReceipt2, ( LPARAM )&lic );
													if( receiptRes == 1 )
													{
														accType = lic.accType;		// filled in dlgRemoteReceipt
														if( lic.userExpiration )	// filled in dlgRemoteReceipt
														{
															if( g->getExpirationDate() == -1 || g->getExpirationDate() > lic.userExpiration )
															{
																g->setExpirationDate( lic.userExpiration );
																g->expirationReason = EXPIRATION_REASON_USER;
															}
															if( !isProjectExpired() )
															{
																lld->res = 1;
															}
														}
														else
														{
															lld->res = 1;
														}
													}
													else if( receiptRes == 2 )
													{
														criticalError( E_TIMEOUT );
													}
													else	//0 => user canceled receipt input
													{
														criticalError( E_CANCELLED );
													}
												}
											}
										}
										// ------------------------------
										// valid account number or cid
										// ------------------------------
										else 
										{
	fdbg( "lactp=%d", lic.accType );
	dbg( "lactp=%d", lic.accType );

											accType = lic.accType;
											if( lic.userExpiration )
											{
												if( g->getExpirationDate() == -1 || g->getExpirationDate() > lic.userExpiration )
												{
													g->setExpirationDate( lic.userExpiration );
													g->expirationReason = EXPIRATION_REASON_USER;
												}
											}
											if( !isProjectExpired() )
											{
	fdbg( "res=1" );
	dbg( "res=1" );
												lld->res = 1;
											}
										}
										ENCR( strDemo, keyDemo );

										if( lld->res == 1 )
										{
											if( accType != ( ACC_REAL | ACC_DEMO  ) && accType != 0 )	//old version lic contains 3, new - 0
											{
												// -------------------------------
												// 6. check account real/demo match
												// --------------------------------
											fdbg( "6. %d", accType );
											dbg( "6. %d", accType );

												if( ( accType == ACC_REAL && !real ) ||
													( accType == ACC_DEMO && real )
												)
												{
													lld->res = 0;
										
											fdbg( "rddnm" );//real/demo does not match
											dbg( "rddnm" );//real/demo does not match
													/*Dear client, this version of expert advisor/indicator is not allowed to run with %s accounts. 
													Please contact vendor for more details.*/
													BYTE keyText[4]={0xc2,0x3b,0x40,0x51};
													BYTE strText[134]={0x8e,0xf9,0x1e,0x55,0xf0,0xf5,0xac,0x6b,0xc2,0x23,0x9b,0x6d,0x54,0xd5,0xc2,0xf0,0x24,0x12,0xb8,0x9b,0x91,0xa6,0x44,0x72,0xbd,0x05,0x3d,0x3a,0x3d,0x5e,0x1d,0x70,0x7f,0x4f,0xde,0xef,0x54,0xd6,0xcd,0xec,0xa4,0xfb,0xd6,0x66,0x65,0x7d,0x34,0x1d,0x04,0x57,0xab,0x4f,0x83,0xe4,0x2d,0xa6,0x07,0xa2,0xde,0x90,0xcc,0x2d,0x16,0xc4,0x49,0xf4,0xce,0x99,0xaf,0xb7,0xd8,0xa5,0x8c,0xc3,0xcd,0x16,0x2f,0x17,0x63,0xb4,0xf6,0x89,0x30,0x9c,0xa2,0xad,0xf2,0x57,0x95,0xaf,0xa8,0x2c,0xc3,0x9e,0x19,0x09,0xb5,0xb4,0x37,0x1a,0x66,0x85,0x50,0x98,0x53,0x77,0x6e,0x6e,0x16,0x6c,0xeb,0x07,0x03,0x40,0x0d,0x4d,0xb9,0xd2,0xff,0x2b,0xf0,0x74,0x02,0x78,0xe3,0xf5,0x03,0x41,0x2c,0xbe,0x22,0x7d,0xc5,0xe0};

													/*Permission denied by account type for %s*/
													BYTE keyTitle[4]={0x5f,0x8f,0xc8,0x30};
													BYTE strTitle[41]={0xdf,0xf6,0x02,0x45,0xde,0xff,0x7e,0x2b,0x35,0xc4,0x01,0x85,0xee,0x3b,0xc0,0x85,0x9a,0x47,0xc0,0x16,0xa9,0xa2,0x69,0xe0,0xcf,0x06,0x23,0x19,0x3c,0x82,0x4f,0x80,0xad,0x9b,0x65,0x98,0xee,0x09,0xaf,0x4d,0xfc};

													char* title = ( char* )mymalloc( 256 );
													char* text = ( char* )mymalloc( 256 );
													DECR( strTitle, keyTitle );
													wsprintfA( title, ( char* )strTitle, g_projectName );
													ENCR( strTitle, keyTitle );
										//dbg( "title '%s'", title );
													DECR2( strText, keyText );
													wsprintfA( text, ( char* )strText, accType == ACC_REAL ? "demo" : "real" );
													ENCR( strText, keyText );
										//dbg( "text '%s'", text );
													CloseHandle( modelessMessageBox( text, title, MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST ) );
													free( title );
													free( text );
												}
											}
											
											if( lld->res == 1 )
											{
												if( lic.type == LT_REMOTE_LOGIN )
												{
													g_logger->submit( LGT_AUTH_OK, lic.lastCre1, lic.lastCre2, 0 );
												}
												else if( lic.type == LT_REMOTE_OPEN )
												{
													g_logger->submit( LGT_AUTH_OK, 0 );
												}
												else
												{
													g_logger->submit( LGT_AUTH_OK, lic.lastCre1, 0 );
												}

												if( !lic.autoUpdate )
												{
													if( lic.update )
													{
														if( !IsWindow( g_dlgUpdate ) ) 
														{
			fdbg( "upd" );
			dbg( "upd" );
															CLic *licCopy = new CLic( lic );

															CloseHandle( modelessDialog( IDD_REMOTE_UPDATE, dlgRemoteUpdate, licCopy ) );
														}
														else
														{
															fdbg( "updex" );
															dbg( "updex" );
														}
													}
												}
												else
												{
													processAutoUpdate( lic );
												}
											}
										}
									}
									else	//some lic error 
									{
fdbg( "sil%d", 1 );
dbg( "sil%d", 1 );
							
										if( lic.type != LT_REMOTE_CID )
										{
											/*Insufficient licence for project %s. Your chart has been closed automatically. You have to restart your terminal in order to try again. Contact vendor and supply directory %s with all files inside it if you think this is a mistake. */
											BYTE textKey[4]={0x1f,0x8a,0x93,0x49};
											BYTE textStr[233]={0x4f,0x7a,0x74,0x7a,0xd4,0x07,0xdd,0x6a,0xf2,0x63,0x6d,0x2f,0xc8,0x4f,0xa8,0xc3,0xd1,0x05,0x31,0xdb,0xd8,0x64,0x71,0xdd,0xc7,0xdf,0x71,0x97,0x10,0x87,0x3d,0x05,0x03,0x6b,0x40,0xcc,0x2c,0x95,0xe6,0x6a,0x68,0xef,0x29,0xe8,0xba,0xd1,0xb4,0xed,0x14,0x25,0xc5,0x06,0x92,0xff,0x2e,0x31,0xfa,0x25,0xc1,0x8e,0x6d,0x24,0xe2,0xfc,0x21,0xa2,0x4f,0x8c,0x28,0x25,0xc8,0xf0,0xd3,0x41,0x0d,0xa8,0x48,0xd9,0xda,0xc4,0x89,0xe8,0xd0,0xa8,0x75,0x25,0x89,0x4d,0xf9,0x1f,0x26,0x0e,0xc6,0x65,0xaf,0xc4,0x88,0xb1,0xdd,0xe1,0x99,0xfb,0x07,0x4d,0xf2,0x1e,0xf8,0x61,0x02,0xd9,0x6e,0x47,0xe9,0x11,0xab,0xf1,0x89,0x8c,0xe9,0x86,0x9f,0xc9,0x96,0xb8,0x39,0xf2,0xbd,0x9d,0xc5,0x32,0xbb,0x15,0x1d,0xc9,0xfc,0x70,0xec,0x53,0x6b,0x47,0x78,0xbd,0x70,0x79,0x18,0x7a,0xa4,0x8a,0x6a,0xba,0x96,0x86,0x64,0x0d,0xed,0xa2,0x1c,0x6c,0xc5,0x74,0x39,0xd9,0xd3,0x41,0x2d,0xaf,0x1a,0xd4,0xbe,0xad,0xbc,0x66,0xd0,0x6d,0x74,0xae,0x99,0x27,0x9a,0xc8,0x39,0x43,0x70,0xd3,0x6f,0x1b,0x02,0xad,0x91,0x31,0x4d,0xda,0x2c,0xc8,0xef,0x7a,0x77,0x33,0x98,0x9b,0x0a,0x1a,0x86,0x74,0xee,0xc6,0x9a,0xf2,0x1e,0x16,0xbe,0xfe,0x12,0x5e,0x3e,0xd5,0xdd,0x7e,0x11,0x5b,0x13,0x8e,0xa3,0x79,0x46,0x02,0xb0,0x18,0x6b,0x24,0xea,0x1e,0x4f};

											DECR( textStr, textKey );
											DECR2( g_authenticationFailedTitleStr, g_authenticationFailedTitleKey );
											char* title = ( char* )mymalloc( MAX_PATH );
											char* text= ( char* )mymalloc( 2 * MAX_PATH );

											wsprintfA( title, ( char* )g_authenticationFailedTitleStr, g_projectName );
											wsprintfA( text, ( char* )textStr, g_projectName, g_dbgPath );

											ENCR( g_authenticationFailedTitleStr, g_authenticationFailedTitleKey );
											ENCR( textStr, textKey );

											criticalError( text, title );
											free( title );
											free( text );

										}
										else
										{
											char* email = ( char* )malloc( 128 );
											strcpy( email, lic.supportEmail );
											CloseHandle( myCreateThread( threadCidErr, email, "cid_err" ) );
										}	

										fdbg( "sil%d", 2 );
										dbg( "sil%d", 2 );
									}
								} 
								else // project is held
								{
fdbg( "hold" );
dbg( "hold" );
									// %s hold
									BYTE key[4] = {0x55,0x79,0x9d,0x3c};
									BYTE str[8]={0xeb,0x59,0x53,0xc2,0xdc,0xff,0x6f,0xe4};
									DECR( str, key );
									char *title = ( char* )mymalloc( 128 );
									wsprintfA( title, ( char* )str, g_projectName );
									ENCR( str, key );
									CloseHandle( modelessMessageBox( lic.holdMessage, title, MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST ) );
									free( title );
								}
							} // if( !isProjectExpired() )
							else if( g->expirationReason == EXPIRATION_REASON_REVISION )
							{
								if( lic.autoUpdate )
								{
fdbg( "silau" );
dbg( "silau" );
									processAutoUpdate( lic );
									//g->isErrorShown = TRUE;	uncomment to NOT show expiration dialog too!
								}
								else if( lic.update )
								{
									if( !IsWindow( g_dlgUpdate ) )
									{
										fdbg( "upd2" );
										dbg( "upd2" );
														
										g->setFlag( FLG_ERROR_SHOWN, 1 );

										CLic *licCopy = new CLic( lic );
										CloseHandle( modelessDialog( IDD_REMOTE_UPDATE, dlgRemoteUpdate, licCopy ) );
									}
									else
									{
										fdbg( "updex2" );
										dbg( "updex2" );
									}
								}
							}
						}
						else	//lic broken?
						{
fdbg( "du.il" );//Dear user. Invalid licensing" );
dbg( "du.il" );//Dear user. Invalid licensing" );
							// Dear user. Invalid licensing. %s will stop working. Restart Metatrader to try again.
							BYTE key[4] = {0xbb,0xda,0x93,0xf6};
							BYTE fmt[85]={0x04,0x85,0xc0,0x08,0x89,0x00,0x12,0x2b,0x0c,0x1e,0x88,0xb4,0xa2,0x11,0x91,0x1a,0x12,0x9e,0xb8,0x3b,0x93,0x19,0xa0,0x5f,0xf2,0xe6,0x26,0xd1,0x86,0x14,0x9d,0x0d,0x71,0x8b,0x8b,0x35,0x4b,0x64,0xab,0x30,0x2c,0x5b,0x69,0x1e,0x95,0x89,0x24,0xb2,0x7c,0x52,0x50,0x10,0xaf,0xeb,0x1a,0x3e,0x2b,0xe1,0xbd,0xc7,0x13,0xd9,0x59,0xb1,0xf6,0x31,0x18,0x3e,0x3c,0xab,0x20,0xec,0x4f,0x66,0xea,0x8d,0xa2,0x48,0x75,0x7a,0x53,0x6c,0x77,0xaf,0x43};

							criticalError( g_authenticationFailedTitleStr, sizeof( g_authenticationFailedTitleStr ), g_authenticationFailedTitleKey, sizeof( g_authenticationFailedTitleKey ), 
											fmt, sizeof( fmt ), key, sizeof( key ) );

							g->setFlag( FLG_ERROR_SHOWN, 1 );
fdbg( "sil%d", 3 );
dbg( "sil%d", 3 );
						}

						sendRedisStats( &mgr, server, port, baseUri, lld->res, lic.lastCre1, lic.lastCre2 );
					}
					else
					{
						lld->internetRes = ML_READ_FILE;
					}
					if( raw )
					{
						free( raw );
					}
					lld->done = TRUE;
				} // if( WaitForSingleObject( lld->mutex, 0 ) == 0
			}
			else if( h->status == 404 )
			{
				lld->internetRes = ML_NO_DATA;
			}
		}
	}
	if( rslt )
	{
		free( rslt );
	}
	delete h;

	return( 0 );
} // threadLoadLic2

// =================
// threadRemoteAuth2
// =================
DWORD WINAPI threadRemoteAuth2( DWORD *initStart )
{
	DWORD res = 0;
	TRY
dbg( "tra2" );
	BOOL timeout = FALSE;

	CBaseServerManager mgr;

	CGlobalIndicator::GlobalIndicatorItem* indItem = g_ind->start( g_projectName );

	// %s - Insufficient Licence
	//BYTE titleKey[4] = {0x17,0xa8,0x4e,0x60};
	//BYTE titleStr[26]={0x35,0x28,0xc6,0x4d,0x46,0xfe,0x8b,0xa3,0x17,0xc5,0x3f,0xc8,0xe2,0xa0,0x7f,0x87,0x51,0xe8,0xf7,0xaf,0xd0,0x6b,0xed,0x31,0xa8,0x53};
	Globals* g = getGlobals();
	//CXOR xor;

	// -----------------------------------------
	// fetching google.com date as server time shift
	// -----------------------------------------
	/*google.com*/
	BYTE keyGoogle[4]={0xb1,0x05,0x12,0x7c};
	BYTE strGoogle[11]={0x71,0x7a,0x4f,0x25,0x9e,0xfb,0xac,0x50,0xeb,0xae,0x6f};
	DECR( strGoogle, keyGoogle );
	CBaseServerManagerHandle *h = mgr.load( ( const char *)strGoogle, 80, "/", TRUE, FALSE );
	ENCR( strGoogle, keyGoogle );

	DWORD waited = h->wait( MAX_AUTH_DURATION * 1000 );
dbg( "gw %d %d", h->error, h->status );
	if( !isTimePassed( *initStart, MAX_AUTH_DURATION * 1000 ) )
	{
		g->serverTimeShift = fetchServerTimeShift( h->hr );
dbg( "gt %I64u", g->serverTimeShift );
	}
	else
	{
dbg( "ght" );
		h->abort();
	}
	delete h;

	// ------------------------
	// downloading lic
	// ------------------------
	const char* servers = g->useServers();
	DWORD nServers = 0;
	const char* curServer;
	for( curServer = servers; *curServer; curServer += strlen( curServer ) + 1 )
	{
		nServers ++;
	}
	HANDLE *hLoadThreads = ( HANDLE* )mymalloc( nServers * sizeof( HANDLE ) );
	ThreadLoadLicData ** lld = ( ThreadLoadLicData** )mymalloc( nServers * sizeof( ThreadLoadLicData* ) );

	HANDLE mutex = CreateMutex( 0, 0, 0 );

	// start threads one by one
	int i = 0;
	for( curServer = servers; *curServer; curServer += strlen( curServer ) + 1 )
	{
		lld[ i ] = ( ThreadLoadLicData* )mymalloc( sizeof( ThreadLoadLicData ) );
		lld[ i ]->server = ( char* )mymalloc( strlen( curServer ) + 1 );
		strcpy( lld[ i ]->server, curServer );
		lld[ i ]->mutex = mutex;
		lld[ i ]->initStart = *initStart;
		lld[ i ]->done = FALSE;
		lld[ i ]->res = 0;
		lld[ i ]->forceStop = FALSE;

		hLoadThreads[ i ++ ] = myCreateThread( ( LPTHREAD_START_ROUTINE )threadLoadLic2, lld[ i ], "threadLoadLic2", 0, &lld[ i ]->tid ); 
	}
	g->unuse( servers );

	// check servers until one has loaded lic
	BOOL done = FALSE;
	BOOL anyRunning = TRUE;
	MLError internetRes = ML_UNKNOWN;
	for( ; !done && anyRunning; )
	{
		anyRunning = FALSE;

		for( i = 0; i < nServers && !done; i ++ )
		{
			if( hLoadThreads[ i ] != INVALID_HANDLE_VALUE &&
				WaitForSingleObject( hLoadThreads[ i ], 100 ) == 0 )
			{
				CloseHandle( hLoadThreads[ i ] );
				hLoadThreads[ i ] = INVALID_HANDLE_VALUE;

				if( lld[ i ]->done )
				{
			dbg( "wtl %04x %d %d", lld[ i ]->tid, lld[ i ]->internetRes, lld[ i ]->res );

					done = TRUE;
					internetRes = lld[ i ]->internetRes;
					*initStart = lld[ i ]->initStart;
					res = lld[ i ]->res;
				}
				free( lld[ i ]->server );
				free( lld[ i ] );
			}

			if( hLoadThreads[ i ] != INVALID_HANDLE_VALUE )
			{
				anyRunning = TRUE;
			}
		}
	}

	// Close remaining handles and free thread data
	// Remaining threads will be done by themselves
	for( i = 0; i < nServers; i ++ )
	{
		if( hLoadThreads[ i ] != INVALID_HANDLE_VALUE )
		{
dbg( "frcst %04x", lld[ i ]->tid );
			lld[ i ]->forceStop = TRUE;
			WaitForSingleObject( hLoadThreads[ i ], INFINITE );
dbg( "stpd" );
			CloseHandle( hLoadThreads[ i ] );

			free( lld[ i ]->server );
			free( lld[ i ] );
		}
	}

	free( lld );

	CloseHandle( mutex );

	fdbgdone( res );

	g->lastErr = internetRes;
fdbg( "ir %d %d", internetRes, g->getFlag( FLG_REAUTH ) );
dbg( "ir %d %d", internetRes, g->getFlag( FLG_REAUTH ) );

	// if none of lic validation threads succeeded then internetRes == ML_UNKNOWN, so we count this error code too
	if( ( isInternetError( internetRes ) || internetRes == ML_UNKNOWN ) && !g->getFlag( FLG_REAUTH ) )	//do not show error msg if re-validation failed because of internet error
	{
dbg( "du.ol" );
		// Dear User. Our licensing server seems to be not available or you have no internet. Please recover internet or try later again. "%s" will not work. Please restart Metatrader to try again.
		BYTE key[4] = {0x5c,0x3f,0xcf,0xba};
		BYTE fmt[187]={0xb6,0x28,0xc8,0xf6,0x9c,0xf2,0x19,0x7d,0x44,0x73,0x45,0xb0,0x80,0xa8,0x57,0x9f,0x08,0x42,0xc0,0x27,0xe2,0xa4,0xde,0xca,0xa8,0x92,0x08,0xb1,0x59,0x55,0x88,0x8c,0x4f,0x0c,0xcc,0x80,0xf1,0xee,0x92,0xed,0x2b,0xab,0x48,0x60,0x7d,0x30,0x42,0x6a,0xb8,0x5f,0x38,0xcd,0x36,0xcf,0x85,0x8c,0x1b,0x8b,0x92,0xa2,0xf1,0xc4,0x4e,0x8b,0x02,0x67,0x13,0x98,0xc5,0xdc,0xd2,0xb3,0xb3,0x28,0x56,0xfb,0x65,0x7a,0x9e,0x98,0x29,0x0e,0x12,0xf8,0xc9,0x8f,0x81,0xa4,0x52,0x1d,0x49,0x89,0xd0,0x98,0x5d,0x6e,0xae,0xbf,0x83,0x53,0x48,0x9f,0x54,0x83,0x13,0xca,0x2b,0x35,0x34,0x0e,0x38,0x0b,0xd2,0x53,0xc4,0x54,0x37,0xb6,0x4e,0x82,0xbd,0x4d,0x26,0x6e,0x7d,0x2a,0x54,0x12,0x36,0xb1,0xa0,0xb8,0x88,0x5d,0xaa,0x12,0xb3,0xeb,0xef,0x23,0x0a,0xd0,0x5c,0xc1,0xcf,0xf2,0xc5,0xb9,0x50,0xa6,0xac,0x8d,0xa6,0xca,0xca,0x13,0xa5,0xcb,0x3d,0xf7,0xb3,0x91,0x51,0x04,0x66,0x4b,0x9a,0xbd,0x7d,0xe0,0x79,0x4e,0x21,0x37,0xc8,0x96,0x61,0xee,0xbd,0x6c,0xb7,0xff,0x98,0x6f,0x59,0xd1,0x8f};

		// Product: %s. Licencing Server is not available
		BYTE titleKey[4] = {0x9d,0x13,0x5f,0x00};
		BYTE titleStr[47]={0x20,0x06,0x04,0x52,0xf4,0x25,0x24,0xac,0xad,0xd9,0x90,0x30,0x95,0xbd,0xd1,0xd0,0xac,0xd8,0x05,0x49,0xc5,0xac,0xe7,0x77,0xe2,0x89,0xcc,0x07,0x03,0x7c,0xfc,0x0a,0x77,0x6c,0xdf,0xc0,0x5a,0xb5,0x0d,0xa4,0xc9,0xf6,0x9a,0x37,0x17,0x1c,0x66};

		criticalError( titleStr, sizeof( titleStr ), titleKey, sizeof( titleKey ), fmt, sizeof( fmt ), key, sizeof( key ) );

		g->setFlag( FLG_ERROR_SHOWN, 1 );
	}

	dbg( "rii" );//removing indItem
	g_ind->stop( indItem );	


dbg( "trad" );//threadRemoteAuth done" );
	CATCH

	return( res );
} // threadRemoteAuth2


// =================
// dlgCidError
// ==================
INT_PTR CALLBACK dlgCidError( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	static char supportEmail[ 128 ];
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			strcpy( supportEmail, ( char* )lp );

			char* text = ( char* )malloc( 512 );

			//title
			DECR( g_authenticationFailedTitleStr, g_authenticationFailedTitleKey );
			wsprintfA( text, ( char* )g_authenticationFailedTitleStr, g_projectName );
			ENCR( g_authenticationFailedTitleStr, g_authenticationFailedTitleKey );
			SetWindowTextA( dlg, text );

			/*Insufficient licence for project %s. Your chart has been closed automatically. You have to restart your terminal in order to try again. Contact vendor and supply directory %s with all files inside it if you think this is a mistake. */
			BYTE key[4]={0xd3,0x44,0x18,0x16};
			BYTE str[233]={0xed,0x55,0x8e,0x62,0x4b,0x25,0x6f,0x39,0xd4,0xef,0xe9,0xd6,0x2c,0x00,0xbf,0xe9,0xa7,0x02,0x0e,0xae,0x47,0xff,0xdd,0xb0,0xbd,0x3f,0x80,0xfb,0x50,0xe4,0xf6,0x5a,0x23,0x0f,0x84,0x13,0x98,0xcf,0x9f,0x9c,0xce,0xa8,0xdc,0x4d,0x07,0x33,0x02,0xe2,0x1a,0x9e,0x5f,0xf5,0x31,0xff,0xe8,0x3b,0xc5,0x7a,0xe8,0xd8,0x5c,0x5d,0xbf,0xe9,0x07,0x43,0x01,0xc7,0x50,0xc4,0x0f,0xae,0xe0,0xd4,0x27,0x1e,0x5e,0x20,0x8b,0x03,0x63,0x62,0x79,0x8b,0xb4,0x64,0x42,0xdf,0xad,0x23,0xcd,0x07,0xb4,0x2c,0xc0,0x3d,0x91,0x9f,0x85,0x6c,0xb4,0xb6,0x22,0x16,0x65,0xfe,0xf2,0x83,0x78,0x1a,0x9c,0xc8,0x05,0xf1,0x5b,0xe0,0x89,0x9a,0xd6,0x68,0x95,0x38,0x0e,0x09,0xa3,0xa6,0xa0,0xd9,0x9f,0xba,0xd3,0x63,0x0c,0x36,0xa7,0x4f,0x86,0x9d,0xf6,0xa7,0xb5,0x27,0x9f,0xa6,0x55,0xd1,0x2a,0xd9,0xd1,0x83,0xee,0xdf,0x59,0x76,0xdc,0x14,0xc9,0x80,0x40,0x89,0x84,0x29,0x9e,0x1d,0x01,0x1b,0x60,0x18,0xc9,0xbe,0xf7,0x3a,0xde,0x6b,0xc2,0x58,0x29,0xc7,0x11,0x94,0x88,0x00,0x65,0x75,0xca,0xe8,0x16,0x87,0xea,0x01,0xc5,0x7d,0x06,0xd7,0x95,0xf0,0x85,0x0e,0x04,0x96,0xd0,0x5b,0x82,0x47,0x9b,0x6a,0x0c,0x13,0xb5,0x60,0x4d,0x8c,0xd2,0xac,0x74,0x4e,0x15,0xf9,0xfe,0xc5,0x76,0x60,0xe8,0xbc,0x56,0xb9,0x5e,0x0d,0x53,0x09,0x3e,0xc7,0xa2};
			DECR2( str, key );
			wsprintfA( text, ( char* )str, g_projectName, g_dbgPath );
			ENCR( str, key );
			SetDlgItemTextA( dlg, IDC_TEXT, text );

			/*Your current CID is : %s. Please send this id to vendor in order to add your authenticate you. Email address of vendor is : %s*/
			BYTE key2[4]={0xbc,0x34,0x07,0xd0};
			BYTE str2[127]={0xa0,0x6f,0x46,0x3b,0xbd,0x35,0x27,0xe7,0xc0,0x4c,0x51,0xfd,0x69,0x11,0xd5,0xa3,0xd2,0xd4,0xf6,0xdc,0xbb,0x95,0x40,0x6c,0x4d,0x70,0xde,0x65,0xde,0xb6,0x9d,0x08,0xa4,0x4e,0x01,0xb9,0x89,0x51,0xf8,0xd4,0xb2,0x1e,0x31,0x4f,0x0e,0x4e,0x2f,0xd6,0xf0,0x83,0xa8,0xd6,0x99,0x79,0x1b,0x4b,0xce,0x69,0x4a,0x3d,0xb3,0xf5,0xc0,0x46,0xfa,0xcc,0x0d,0x1c,0x7c,0xe3,0x2d,0x33,0xf6,0xf5,0xff,0xd7,0x6e,0x40,0x71,0x97,0x5d,0x40,0xf9,0x1e,0xab,0x22,0xdd,0x4d,0xb6,0xec,0x9a,0x49,0xb3,0xff,0x8a,0x9f,0xc4,0xac,0x0f,0xbe,0x55,0xda,0x64,0xbb,0xa1,0x30,0xe9,0x6a,0xd3,0x2c,0x68,0x9b,0xf9,0x7b,0xec,0x1c,0x80,0x54,0xa7,0xe4,0xf3,0x67,0xe4,0x27,0x4f,0x1c,0x24};
			DECR2( str2, key2 );
			wsprintfA( text, ( char* )str2, GetComputerID2(), supportEmail );
			ENCR( str2, key2 );
			SetDlgItemTextA( dlg, IDC_TEXT2, text );

			free( text );
		}
			break;
			
		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDOK: 
					DestroyWindow( dlg );
					break;

				case IDB_COPY_CID:
					if( OpenClipboard( dlg ) )
					{
						EmptyClipboard();
						HGLOBAL mem = GlobalAlloc( GMEM_MOVEABLE, 33 );
						char* str = ( char* ) GlobalLock( mem );
						lstrcpyA( str, GetComputerID2() );
						GlobalUnlock( mem );
						SetClipboardData( CF_TEXT, mem );
						CloseClipboard();
					}
					else
					{
						char err[ 100 ];
						wsprintfA( err, "Failed open clipboard (%d)", GetLastError() );
						MessageBoxA( dlg, err, "Error", MB_ICONERROR );
					}
					break;

				case IDB_EMAIL_VENDOR:
				{
					/*mailto:%s?subject=Support%%20Inquery%%20for%%20%s*/
					BYTE key[4]={0xac,0x10,0xe7,0x4d};
					BYTE str[50]={0xd2,0xb2,0xb8,0xe0,0x29,0x14,0x5d,0x7d,0x4d,0xe8,0x82,0xff,0x82,0x97,0x55,0x3a,0x81,0xe5,0x10,0x2c,0x7a,0xbb,0xa6,0x13,0x87,0x04,0x8e,0x19,0x20,0x83,0x12,0x07,0xb9,0x9a,0xdd,0xf5,0xd1,0xdb,0x79,0x0b,0xbc,0x78,0xac,0x66,0xc1,0x5b,0x79,0xd8,0xf5,0xf3};
					DECR( str, key );
					char* url = ( char* )malloc( 256 );
					wsprintfA( url, ( char* )str, supportEmail, g_projectName );
					ENCR( str, key );
					openLinkA( url );
					free( url );
					break;
				}

			}
			break;
	}
	return( 0 );
} // dlgCidError

DWORD WINAPI threadCidErr( void* p )
{
	DialogBoxParamA( g_hInstance, MAKEINTRESOURCEA( IDD_CID_ERROR ), 0, dlgCidError, ( LPARAM )p );
	free( p );
	return(0);
}


// ==================
// findDataDir
// ==================
BOOL findDataDir( OUT char *path, OUT BOOL *isPortableMode )
{
	wchar_t terminalDir[ MAX_PATH ];
	GetModuleFileName( 0, terminalDir, MAX_PATH );
	PathRemoveBackslash( terminalDir );
	wchar_t* sl = wcsrchr( terminalDir, L'\\' );
	if( sl )
	{
		*sl = 0;
	}
	dbg( L"terminalDir='%s'", terminalDir );

	BOOL res = FALSE;

	const char *cmd = GetCommandLineA();
	/* /portable*/
	BYTE key[4]={0x05,0xb7,0x21,0x55};
	BYTE strPortable[11]={0x7d,0x4b,0xbe,0x1a,0x8b,0xe5,0xec,0x7e,0x0f,0xbc,0xf3};
	DECR( strPortable, key );
	const char* p = strstr( cmd, ( char* )strPortable );
	ENCR( strPortable, key );
	if( !p )	//if mt4 was not started with " /portable" command line argument
	{
		*isPortableMode = FALSE;

		wchar_t dir[ MAX_PATH ];

		SHGetFolderPathAndSubDir(NULL,                //hWnd	
								 CSIDL_APPDATA,
								 NULL,                //hToken
								 SHGFP_TYPE_CURRENT,  //dwFlags
								 L"MetaQuotes\\Terminal\\",    //pszSubDir
								 dir ); 

		wchar_t mask[ MAX_PATH ];
		wcscpy( mask, dir );
		wcscat( mask, _T( "*.*" ) );
	//MessageBox( 0, mask, L"0", 0 );
	dbg( L"findDataDir: '%s'", mask );

		WIN32_FIND_DATA ffd;
		HANDLE fh = FindFirstFile( mask, &ffd );
		
	//MessageBox( 0, terminalDir, L"product dir", 0 );

		if( fh != INVALID_HANDLE_VALUE )
		{
			do
			{
				if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
	//MessageBox( 0, ffd.cFileName, L"1", 0 );
					wchar_t origin[ MAX_PATH ];
					wcscpy( origin, dir );
					wcscat( origin, ffd.cFileName );
	dbg( L"scanning dir '%s'", origin );
					wcscat( origin, L"\\origin.txt" );
					
					HANDLE h = CreateFile( origin, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0 );
					if( h != INVALID_HANDLE_VALUE )
					{
	dbg( L"found origin.txt" );
						*origin = 0;
						DWORD sz;

						WORD bom;
						if( ReadFile( h, &bom, 2, &sz, 0 ) )
						{
							if( bom == 0xfeff )
							{
	dbg( "bom feff" );
								ReadFile( h, origin, MAX_PATH * sizeof( wchar_t ), &sz, 0 );
								origin[ sz / sizeof( wchar_t ) ] = 0;
							}
							else
							{
	dbg( "ascii" );
								SetFilePointer( h, 0, 0, FILE_BEGIN );
								char aorigin[ MAX_PATH ];
								ReadFile( h, aorigin, MAX_PATH, &sz, 0 );
								aorigin[ sz ] = 0;
								MultiByteToWideChar( CP_ACP, 0, aorigin, -1, origin, MAX_PATH );
							}
						}
						CloseHandle( h );
	dbg( L"read '%s'", origin );
						//MultiByteToWideChar( CP_ACP, 0, path, -1, origin, MAX_PATH );
	//MessageBox( 0, origin, L"2", 0 );
						PathRemoveBackslash( origin );
	dbg( L"removed backslash '%s'", origin );
						if( StrCmpI( origin, terminalDir ) == 0 )
						{
							wchar_t wpath[ MAX_PATH ];

							wcscpy( wpath, dir );
							wcscat( wpath, ffd.cFileName );
							WideCharToMultiByte( CP_ACP, 0, wpath, -1, path, MAX_PATH, 0, 0 );
	dbg( "result '%s'", path );
							res = TRUE;
							break;
						}
					}
				}
			}
			while( FindNextFile( fh, &ffd ) );
			FindClose( fh );
		}
	}
	else
	{
		*isPortableMode = TRUE;
	}
	if( !res )
	{
		WideCharToMultiByte( CP_ACP, 0, terminalDir, -1, path, MAX_PATH, 0, 0 );	//USING TERMINAL DIR AS DEFAULT
		res = TRUE;
	}
	return( res );
} // findDataDir


// ==================
// processAutoUpdate
// ==================
void processAutoUpdate( CLic& lic )
{
	if( lic.autoUpdate != AU_NO_NEED )
	{
		BOOL isExpired = isProjectExpired() ;

		SYSTEMTIME st;
		GetSystemTime( &st );
		DWORD now = systemTime2UnixTime( &st );
		CMLReg reg;
		if( !isExpired )
		{
			reg.open();
		}
		DWORD prev, sz;
		sz = sizeof( DWORD );
		prev = 0;
		
		if( lic.autoUpdate == AU_NEED )
		{
			if( !isExpired )
			{
				reg.get( "au", &prev, &sz ); 
dbg( "prevau %d", prev );
			}
			if( isExpired || now - prev >= 12 * 60 * 60 )//12 hours must pass since last cancelled notification
			{
				char datadir[ MAX_PATH ];
				BOOL isPortableMode;
				if( findDataDir( datadir, &isPortableMode ) )
				{
dbg( "au" );
					/*MQLLockAUMutex*/
					BYTE key[4]={0xc7,0xe0,0xb9,0x2d};
					BYTE str[15]={0xb9,0xbe,0x2f,0x11,0x22,0x81,0x72,0x7c,0x58,0x1d,0x19,0x61,0x09,0xe6,0x5b};

					DECR( str, key );
					HANDLE hm = CreateMutexA( 0, TRUE, ( const char* )str ); //closed in dlg proc WM_DESTROY
					if( hm && GetLastError() != ERROR_ALREADY_EXISTS )
					{
						//allow only 1 au dialog. todo: shall we limit per process id?
						AutoUpdateData* aud = ( AutoUpdateData*)mymalloc( sizeof( AutoUpdateData ) );
						memset( aud, 0, sizeof( AutoUpdateData ) );
						aud->hMutex = hm;
						strcpy( aud->supportEmail, lic.supportEmail );
						strcpy( aud->supportName, lic.supportName );
						strcpy( aud->datadir, datadir );
						aud->isPortableMode = isPortableMode;

						preventUnloading();//WM_DESTROY allows unloading
						CloseHandle( modelessDialog( IDD_REMOTE_AUTO_UPDATE, dlgRemoteAutoUpdate, aud ) );		
					}
					else
					{
						CloseHandle( hm );
					}
				}
				else
				{
dbg( "!!-- ffdd" );
				}
			}
			else
			{
dbg( "aus" );
			}
		}
		else //something is wrong with names => notify user about update existance
		{
			if( !isExpired )
			{
				reg.get( "aun", &prev, &sz );
			}
			if( isExpired || now - prev >= 12 * 60 * 60 )
			{
	dbg( "aun" );
				AutoUpdateNotificationData *aund = ( AutoUpdateNotificationData *)mymalloc( sizeof( AutoUpdateNotificationData ) );
				memset( aund, 0, sizeof( AutoUpdateNotificationData ) );
				strcpy( aund->supportEmail, lic.supportEmail );
				strcpy( aund->supportName, lic.supportName );

				preventUnloading();//WM_DESTROY allows unloading
				CloseHandle( modelessDialog( IDD_REMOTE_AUTO_UPDATE_NOTIFICATION, dlgRemoteAutoUpdateNotification, aund ) );
			}
			else
			{
dbg( "auns" );
			}
		}
	}
} // processAutoUpdate
#endif // GEN_LT_REMOTE

#ifdef MLL 
BOOL tryInit()
{
	static BOOL res;
	static BOOL inited;
	if( !inited )
	{
		inited = TRUE;

		char* me = ( char* )malloc( MAX_PATH );
		GetModuleFileNameA( g_hInstance, me, MAX_PATH );

		/*\mll.dll*/
		BYTE key[4]={0xa4,0xa2,0x5a,0xc6};
		BYTE str[9]={0x6f,0xd6,0xd4,0xf1,0xec,0x0f,0xf5,0xf9,0x13};
		DECR( str, key );

		char* rslash = strrchr( me, '\\' );
		if( stricmp( rslash, ( char* )str ) == 0 )
		{
			res = TRUE;
		}
		free( me );
		ENCR( str, key );
	}
	return( res );
}
#endif //ifdef MLL

// =================================
// systemTime2UnixTime
// =================================
DWORD systemTime2UnixTime( SYSTEMTIME* st )
{
	FILETIME ft;
	SystemTimeToFileTime( st, &ft );

	return( ( DWORD )( ( *( __int64* )&ft - g_epochFt ) / 10000000 ) );
} // systemTime2UnixTime

// ===================
// unixTime2SystemTime
// ====================
void unixTime2SystemTime( __int64 ut, OUT SYSTEMTIME* st )
{
	__int64 ft;
	ft = ut * 10000000 + g_epochFt;
	FileTimeToSystemTime( ( FILETIME* )&ft, st );
} // unixTime2SystemTime

// ===============
// formatDate
// ===============
char* formatDate( DWORD unixtime )
{
	const char* months[ 12 ] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	SYSTEMTIME st;
	unixTime2SystemTime( unixtime, &st );
	char* res = ( char* )mymalloc( 64 );
	wsprintfA( res, "%02d.%s.%d", ( DWORD )st.wDay, months[ st.wMonth - 1 ], ( DWORD )st.wYear );
	return( res );
} // formatDate

// ===============
// isTimePassed
// ===============
BOOL isTimePassed( DWORD start, DWORD dur, DWORD now )
{
	if( !start )
	{
		return( TRUE );
	}

	__int64 now64;
	if( !now )
	{
		now64 = GetTickCount();
	}
	else
	{
		now64 = now;
	}
	if( now64 < start )	//49.7 days problem in 32bit version
	{
		now64 += 0xffffffff;
	}
	return( now64 - dur > start );
} // isTimePassed

#ifndef MLL
// ===========
// Now
// ===========
time_t Now( const Globals* g )
{
	time_t res;
	if( !g )
	{
		g = getGlobals();
	}
	if( g->serverTimeShift )
	{
		res = g->serverTimeShift + myGetTickCount64();
//dbg( "now() %I64u", res );
		res /= 1000;
	}
	else
	{
		time( &res );
	}
	return( res );
} // Now
#endif

// ===================
// myGetTickCount64
// ===================
ULONGLONG myGetTickCount64(void)
{
    static volatile DWORD count = 0xFFFFFFFF;
    DWORD previous_count, current_tick32, previous_count_zone, current_tick32_zone;
    ULONGLONG current_tick64;

    previous_count = InterlockedCompareExchange( ( volatile LONG* )&count, 0, 0);
    current_tick32 = GetTickCount();

    if( previous_count == 0xFFFFFFFF )
    {
        // count has never been written
        DWORD initial_count;
        initial_count = current_tick32 >> 28;
        previous_count = InterlockedCompareExchange( ( volatile LONG* )&count, initial_count, 0xFFFFFFFF);

        current_tick64 = initial_count;
        current_tick64 <<= 28;
        current_tick64 += current_tick32 & 0x0FFFFFFF;
        return current_tick64;
    }

    previous_count_zone = previous_count & 15;
    current_tick32_zone = current_tick32 >> 28;

    if (current_tick32_zone == previous_count_zone)
    {
        // The top four bits of the 32-bit tick count haven't changed since count was last written.
        current_tick64 = previous_count;
        current_tick64 <<= 28;
        current_tick64 += current_tick32 & 0x0FFFFFFF;
        return current_tick64;
    }

    if (current_tick32_zone == previous_count_zone + 1 || (current_tick32_zone == 0 && previous_count_zone == 15))
    {
        // The top four bits of the 32-bit tick count have been incremented since count was last written.
        InterlockedCompareExchange( ( volatile LONG* )&count, previous_count + 1, previous_count);
        current_tick64 = previous_count + 1;
        current_tick64 <<= 28;
        current_tick64 += current_tick32 & 0x0FFFFFFF;
        return current_tick64;
    }

    // Oops, we weren't called often enough, we're stuck
    return 0xFFFFFFFF;
} //myGetTickCount64
/*
ULONGLONG myGetTickCount64(void)
{
  static volatile LONGLONG Count = 0;
  LONGLONG curCount1, curCount2;
  LONGLONG tmp;

  curCount1 = InterlockedCompareExchange64(&Count, 0, 0);

  curCount2 = curCount1 & 0xFFFFFFFF00000000;
  curCount2 |= GetTickCount();

  if ((ULONG)curCount2 < (ULONG)curCount1)
  {
    curCount2 += 0x100000000;
  }

  tmp = InterlockedCompareExchange64(&Count, curCount2, curCount1);

  if (tmp == curCount1)
  {
    return curCount2;
  }
  else
  {
    return tmp;
  }
} // myGetTickCount64
*/

// ==============
// msgBoxThread
// ==============
DWORD WINAPI msgBoxThread( ModelessMsgBoxData* d )
{
	TRY
	char* title = ( char* )mymalloc( strlen( d->title ) + 1 + 256 );
	
	Globals* g = getGlobals();
	
#ifndef MLL
	/*%s - rev.%d - %d*/
	BYTE key[4]={0xe9,0x13,0xf0,0x37};
	BYTE str[17]={0xe6,0x33,0x1e,0x25,0xfe,0x6c,0x0a,0x14,0x3c,0x4d,0x3d,0x89,0x7c,0x0c,0x87,0x32,0x42};
	DECR( str, key );
	wsprintfA( title, ( const char* )str, d->title, g->getRevision(), g->getProjectId() );
	ENCR( str, key );
#else
	strcpy( title, d->title );
#endif

	DWORD res = MessageBoxA( 0, d->text, title, d->style );
	free( title );
	if( d->deletable )
	{
		delete d;
	}
	FreeLibraryAndExitThread( g_hInstance, res );	// decrement dll references count and exit thread. 
													// Doing this because thread could be running while chart was forced to close 
													//in case of insufficient license ( WM_CLOSE posted ) and dll was unloaded
													// which causes mt4 crash
	CATCH
	return( 0 );
} // msgBoxThread

// ===================
// modelessMessageBox
// ===================
HANDLE modelessMessageBox( const char* msg, const char* title, DWORD style )
{
	ModelessMsgBoxData *d = new ModelessMsgBoxData( msg, title, style );
	
	// prevent unloading
	preventUnloading();

	HANDLE res = myCreateThread( ( LPTHREAD_START_ROUTINE )msgBoxThread, d, "msgBoxThread" );
	return( res );
} // modelessMessageBox

// %s critical error
BYTE g_criticalErrorKey[4] = {0x08,0x42,0xa3,0x75};
BYTE g_criticalErrorFmt[18]={0x0e,0x86,0xb3,0x0a,0x51,0x11,0xc9,0x8e,0x74,0xf4,0xca,0xee,0x82,0xaa,0x57,0x82,0x81,0x24};

// =================
// criticalError
// =================
void criticalError( const char* text )
{
	char* title = ( char* )mymalloc( 256 );
	
	DECR( g_criticalErrorFmt, g_criticalErrorKey );
	wsprintfA( title, ( char* )g_criticalErrorFmt, g_projectName );
	ENCR( g_criticalErrorFmt, g_criticalErrorKey );

	int len = strlen( text ) + 256;
	char* text2 = ( char* )mymalloc( len );
	strcpy( text2, text );
	strcat( text2, "\n\n" );

	// We close the chart, you can apply this product anytime you have fixed this problem later
	BYTE appendixKey[4] = {0x20,0x4b,0x60,0x0b};
	BYTE appendixStr[89]={0xaa,0x28,0xa4,0x5a,0x10,0x1e,0x53,0xa8,0x9d,0x35,0x08,0xac,0x65,0x70,0x6c,0x88,0xf7,0x6b,0xf4,0x77,0xbd,0x0a,0x8c,0x83,0x2b,0x09,0x79,0x5a,0xcb,0x09,0x1b,0xe9,0x72,0xff,0x9e,0x3d,0x14,0xae,0x8a,0x2d,0x25,0x6b,0x01,0xbd,0xc1,0x90,0x5c,0x7f,0x6c,0x21,0x8a,0xd5,0xd1,0x69,0x59,0xbe,0xf5,0x02,0x28,0xe9,0xa2,0x50,0xea,0x19,0x21,0x26,0xae,0x21,0xc2,0xb6,0x2e,0x6f,0xdb,0xff,0x91,0x2c,0x55,0xc8,0x76,0x41,0xe3,0x67,0x71,0x09,0xbc,0x76,0xa2,0xb6,0xf0};

	rc4.Decrypt( appendixStr, sizeof( appendixStr ), appendixKey, sizeof( appendixKey ) );
	strcat( text2, ( char* )appendixStr );
	rc4.Encrypt( appendixStr, sizeof( appendixStr ), appendixKey, sizeof( appendixKey ) );

	criticalError( text2, title );
	free( title );
	free( text2 );
} // criticalError

// ===================
// criticalError
// ===================
void criticalError( const char* text, const char* title )
{
	g_logger->submit( LGT_CRITICAL_ERROR, text, title, 0 );
	CloseHandle( modelessMessageBox( text, title, MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST ) );
} // criticalError

// ==================
// criticalError
// ==================
void criticalError( BYTE* fmt, DWORD fmtSize, const BYTE* key, DWORD keySize )
{
	char* text = ( char* )mymalloc( fmtSize + 256 );

	CRC4 rc4;
	rc4.Decrypt( fmt, fmtSize, key, keySize );
	wsprintfA( text, ( char* )fmt, g_projectName );
	rc4.Encrypt( fmt, fmtSize, key, keySize );

	criticalError( text );
	free( text );
} // criticalError

// ==============
// criticalError
// ==============
void criticalError( BYTE* titleFmt, DWORD titleFmtSize, const BYTE* titleKey, DWORD titleKeySize,
				   BYTE* textFmt, DWORD textFmtSize, const BYTE* textKey, DWORD textKeySize
				  )
{
	char* text = ( char* )mymalloc( textFmtSize + 256 );
	char* title = ( char* )mymalloc( titleFmtSize + 256 );

	CRC4 rc4;
	rc4.Decrypt( titleFmt, titleFmtSize, titleKey, titleKeySize );
	wsprintfA( title, ( char* )titleFmt, g_projectName, g_projectName, g_projectName );
	rc4.Encrypt( titleFmt, titleFmtSize, titleKey, titleKeySize );

	rc4.Decrypt( textFmt, textFmtSize, textKey, textKeySize );
	wsprintfA( text, ( char* )textFmt, g_projectName, g_projectName, g_projectName );
	rc4.Encrypt( textFmt, textFmtSize, textKey, textKeySize );

	g_logger->submit( LGT_CRITICAL_ERROR, text, title, 0 );
	CloseHandle( modelessMessageBox( text, title, MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST ) );
	free( text );
	free( title );
} // criticalError


// ================
// criticalError
// ================
void criticalError( ERROR_TYPE err )
{
	switch( err )
	{
		case E_TIMEOUT:
		{
			/*
			// Your %s license was not authenticated in time. Locate your %s then drag %s to the chart again. If you continue to experience problems then please contact the product vendor for assistance.
			BYTE key[4] = {0x7a,0xcc,0x80,0xa3};
			BYTE str[189]={0x9b,0x90,0x06,0x1a,0xd9,0xc4,0x78,0xf7,0x67,0x0c,0x2e,0x93,0xd7,0xf8,0x25,0x0b,0xc7,0x98,0x88,0x47,0x4c,0xf5,0x40,0x27,0xc7,0xcb,0x2a,0x6a,0x82,0x27,0x2f,0x84,0xd8,0x0b,0xa9,0xc3,0x84,0x09,0xbd,0x38,0xe8,0xb6,0xb6,0x5b,0x89,0x35,0xde,0x36,0x22,0x25,0x64,0xeb,0xfa,0x63,0xed,0x55,0x6c,0xf8,0xdf,0x19,0x03,0x7f,0x9f,0xfc,0x56,0xa9,0x48,0x6d,0xf0,0x1c,0x60,0x3b,0x42,0x7e,0xa6,0xe9,0xa3,0x23,0xb4,0x22,0xaf,0x52,0x5d,0xc2,0xd2,0x0d,0xb1,0xae,0x48,0xfe,0x38,0xab,0x77,0x60,0x89,0x2f,0x87,0xd0,0x5c,0x41,0x78,0x23,0x63,0x30,0xfb,0xe0,0xae,0x7a,0x3e,0xe8,0x8b,0x7e,0xf3,0x6b,0xe5,0x75,0x01,0x5e,0x05,0xee,0x42,0xe7,0x43,0xc0,0x03,0x7b,0x38,0x55,0x2c,0x86,0x94,0x94,0xba,0x2f,0xc0,0xa8,0x74,0x61,0xc8,0xfc,0x77,0x04,0x29,0x22,0x64,0xeb,0x1a,0x63,0x56,0xfa,0xd9,0xf4,0x30,0xc3,0x27,0xd9,0xec,0x24,0xcc,0x60,0xc9,0x81,0xb4,0xc2,0x30,0xfe,0xf5,0xfd,0x28,0xf0,0x3f,0x2a,0xe6,0x2b,0xd8,0xdd,0x41,0x20,0xc9,0xb8,0xed,0xb7,0x82,0xd2,0xa7,0x41,0xd3,0x2a,0x5a};
			*/

			/*Your %s license was not authentificated in time.
			The reason may be slow or missing internet connection. Try it once again and if it still does not work contact vendor for assistance.*/
			BYTE key[4]={0x87,0x73,0xe8,0xcc};
			BYTE str[183]={0x77,0x48,0xf3,0x5f,0x57,0x60,0xc3,0x2b,0xd6,0xde,0xfb,0x97,0x03,0x72,0xc2,0x96,0xc0,0x98,0x60,0x7f,0xf6,0xb0,0x10,0x51,0xd8,0xfb,0xe6,0xf2,0xd4,0x47,0x66,0x04,0xed,0x77,0x4f,0xeb,0x21,0x74,0x4b,0x84,0x3f,0xe2,0x0e,0x5e,0xb6,0x4d,0x17,0xce,0x65,0xef,0xbc,0x03,0x52,0x92,0x9e,0x44,0x07,0xdf,0x8b,0x5a,0x11,0x96,0xe3,0x66,0x44,0xa4,0xec,0xfa,0x62,0x9e,0x28,0xb7,0x1b,0x26,0xb2,0xa4,0x64,0x1a,0xbb,0x91,0xec,0x1c,0x49,0x40,0x1a,0xe0,0xb3,0x3d,0x2f,0xa4,0x5b,0x99,0xfd,0x6c,0xb3,0x3b,0x43,0x66,0x0c,0x0e,0xa4,0xcb,0x72,0xb4,0xca,0xc8,0x71,0x7d,0x01,0x55,0xd0,0x3d,0xc3,0x89,0xd2,0xe4,0x4d,0x2b,0x0f,0x15,0x8a,0xff,0x72,0x22,0x37,0x83,0xe2,0xcd,0xac,0x02,0x00,0xda,0x96,0xae,0x9e,0x05,0x67,0x04,0x8a,0x4a,0x26,0x66,0xa6,0xf3,0x9d,0x9a,0x85,0x48,0x5f,0x74,0xf5,0x17,0x69,0x79,0xcb,0x82,0xcd,0xb2,0x01,0x27,0x39,0x45,0x35,0x74,0x31,0xbd,0xaf,0xfe,0xc8,0xc6,0x24,0xc4,0x8d,0xaa,0xfc,0xb4,0xe3,0x43,0x09,0x15,0xc3,0x59,0x00};

			criticalError( g_authenticationFailedTitleStr, sizeof( g_authenticationFailedTitleStr ), g_authenticationFailedTitleKey, sizeof( g_authenticationFailedTitleKey ), str, sizeof( str ), key, sizeof( key ) );
			break;
		}

		case E_CANCELLED:
		{
			/*Authentication cancelled. %s will not work. Restart Metatrader to try again.*/
			BYTE key[4]={0x9c,0xc6,0x46,0x7d};
			BYTE str[77]={0x28,0x8d,0x10,0x5e,0x21,0x31,0x4f,0xcc,0x14,0xa3,0xf5,0x57,0xd5,0xc7,0xf0,0x9f,0x35,0x1d,0x0d,0x14,0x56,0x86,0x83,0xfe,0xca,0x4b,0x4d,0x46,0x8a,0x65,0xde,0x52,0x21,0xfa,0xcc,0xfe,0xbd,0x22,0x97,0x07,0xe2,0x9b,0x74,0x8b,0x06,0x86,0xce,0x31,0x23,0xcf,0x8c,0x89,0x8d,0xdc,0x43,0x67,0x76,0x44,0x04,0xb3,0x68,0xfd,0xbe,0xcb,0x3b,0x77,0x80,0xf6,0xa4,0x81,0x36,0x38,0xaa,0x75,0xff,0x31,0xf4};
			criticalError( g_authenticationFailedTitleStr, sizeof( g_authenticationFailedTitleStr ), g_authenticationFailedTitleKey, sizeof( g_authenticationFailedTitleKey ), str, sizeof( str ), key, sizeof( key ) );
			break;
		}

	}
} // criticalError

// ===================
// dlgErrorWithInfo
// ===================
INT_PTR CALLBACK dlgErrorWithInfo( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			ModelessMsgBoxData* d = ( ModelessMsgBoxData* )lp;
			Globals* g = getGlobals();

			char *contacts = ( char* )malloc( 1024 );
			/*Product Name:	%s
			Revision:	%d
			Vendor Name:	%s
			Vendor Email:	%s
			Product Url:	%s*/
			BYTE key[4]={0xb0,0xf3,0x36,0x34};
			BYTE str[79]={0x98,0x95,0x69,0x80,0x4b,0x02,0x9c,0x7f,0x2b,0x08,0x1c,0xc0,0x9e,0xc2,0x0b,0xf4,0xa7,0x48,0xcd,0x79,0x3b,0x03,0x1d,0xa8,0x9a,0x33,0x89,0xea,0xff,0x32,0x6a,0xb3,0x14,0xdf,0x5d,0x8d,0xbb,0x0d,0x6d,0x88,0xe0,0xd6,0x15,0xc3,0x77,0x64,0x35,0xc1,0xfa,0xb8,0xe5,0x32,0xb6,0x56,0xc8,0xd3,0xcf,0xac,0x4f,0x9f,0x08,0x3c,0x56,0xf9,0xc8,0xea,0xcd,0x28,0xd4,0x67,0xec,0x94,0xaa,0x78,0xe7,0xb0,0x5d,0xd5,0xc1};
			DECR( str, key );
			wsprintfA( contacts, ( char* )str, g_projectName, g->getRevision(), g->getVendorName(), g->getVendorEmail(), g->getProductUrl() );
			ENCR( str, key );
			SetDlgItemTextA( dlg, IDC_CONTACT_DETAILS, contacts );
			free( contacts );

			SetDlgItemTextA( dlg, IDC_TEXT, d->text );
			if( d->title[ 0 ] == 'i' )//information only, not error
			{
				/*MQLLock information*/
				BYTE key[4]={0x54,0x8b,0x95,0x99};
				BYTE str[20]={0xd8,0x2a,0x45,0xa9,0x18,0x04,0x91,0x68,0x1c,0x27,0x9b,0x07,0xd4,0xa6,0x87,0x87,0x57,0xc0,0x28,0xb2};
				DECR( str, key );
				SetWindowTextA( dlg, ( const char* )str );
				ENCR( str, key );
			}

			delete d;

			centerDlgToScreen( dlg );
			break;
		}

		case WM_COMMAND:
			switch( LOWORD( wp ) )
			{
				case IDOK:
				{
					char* mailto = ( char* )mymalloc( 256 );
					/*mailto:%s*/
					BYTE key[4]={0x9e,0x14,0xbc,0xd6};
					BYTE str[10]={0xd7,0x2b,0x5a,0x07,0x9b,0x9a,0x22,0x65,0x47,0x17};
					DECR( str, key );
					wsprintfA( mailto, ( char*)str, getGlobals()->getVendorEmail() );
					ENCR( str, key );
					openLinkA( mailto );
					free( mailto );
					break;
				}

				case IDCANCEL:
					allowUnloading();
					DestroyWindow( dlg );
					break;
			}
			break;

		case WM_CLOSE:
			allowUnloading();
			DestroyWindow( dlg );
			break;
	}
	return( 0 );
} // dlgErrorWithInfo

// =======================
// criticalError
// =======================
void criticalError( ERROR_TYPE err, BYTE* fmt, DWORD fmtSize, const BYTE* key, DWORD keySize, BOOL justInfo = FALSE )
{
	char* text = ( char* )mymalloc( fmtSize + 256 );

	CRC4 rc4;
	rc4.Decrypt( fmt, fmtSize, key, keySize );
	wsprintfA( text, ( char* )fmt, g_projectName, g_projectName );
	rc4.Encrypt( fmt, fmtSize, key, keySize );

	ModelessMsgBoxData *d = new ModelessMsgBoxData( text, justInfo ? "i" : "e" );
	
	preventUnloading();

	CloseHandle( modelessDialog( IDD_ERROR_WITH_INFO, dlgErrorWithInfo, d ) );
} // criticalError

#ifndef MLL
// ===============
// isThreadDone
// ===============
BOOL isThreadDone( HANDLE hThread, OUT DWORD* res, DWORD timeout )
{
	if( WaitForSingleObject( hThread, timeout ) == 0 )
	{
		GetExitCodeThread( hThread, res );
		return( TRUE );
	}
	return( FALSE );
} // isThreadDone
#endif // #ifndef MLL

// =================
// findChartData
// =================
CChartData* findChartData( HWND hChart, OUT int* resi )
{
	CChartData* res = 0;
	ECS( g_hCharts );
	int n = g_charts.GetCount();
	for( int i = 0; i < n; i ++ )
	{
		if( g_charts[ i ]->hChart == hChart )
		{
			res = g_charts[ i ];
			if( resi )
			{
				*resi = i;
			}
			/*if( ind )
			{
				ECS( res->ind.hIndicators );
				DWORD n = res->ind.indicators->GetSize();
				DWORD tid = GetCurrentThreadId();
				*ind = 0;
				for( DWORD i = 0; i < n; i ++ )
				{
					if( ( *( res->ind.indicators ) )[ i ]->tid == tid )
					{
						*ind = ( *( res->ind.indicators ) )[ i ];
						break;
					}
				}
				LCS( res->ind.hIndicators );
			}*/
			break;
		}
	}
	LCS( g_hCharts );
	return( res );
} // findChartData

#ifndef MLL
// ==============
// chartProc
// ==============
LRESULT CALLBACK chartProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uIdSubclass, CChartData* cd )
{
	if( cd->sz )
	{
		cd->sz->ProcessMessage( hwnd, msg, wp, lp );
	}
#ifndef MLL
	if( rand() % 10 == 0 )
	{
		if( WaitForSingleObject( cd->hMT4GUIMutex, 0 ) != 0 )
		{
			cd->hMT4GUIMutex = createMT4GUIMutex( cd->hChart );
		}
		else
		{
			ReleaseMutex( cd->hMT4GUIMutex );
		}
	}
#endif
	
	switch( msg )
	{
		case WM_COMMAND:
			if( LOWORD( wp ) == STN_CLICKED && ( HWND )lp == cd->hIcon )
			{
#ifndef CPPDEBUG
				/*patched: icon is not shown if vendor does not supply website url so no need to process this case
				if( !g_logoLink )
				{
					// http://mqllock.com/
					BYTE key[4] = {0x8c,0xcd,0x08,0x83};
					BYTE str[20]={0x41,0xc9,0x86,0x09,0x05,0x36,0x55,0x95,0x01,0x79,0xe3,0x37,0x6e,0x5b,0xf5,0xcc,0x03,0x7c,0xb8,0xd6};						
					DECR( str, key );
					openLinkA( ( const char* )str );
					ENCR( str, key );
				}
				else*/
				{
				openLinkA( getGlobals()->getProductUrl() );
				}
#else
				char cmd[ MAX_PATH ];
				wsprintfA( cmd, "/select,\"%s\"", g_dbgPath );
				ShellExecuteA( 0, NULL, "explorer", cmd, NULL, SW_SHOWNORMAL );
#endif
				return 0;
			}
			break;

		case WM_CTLCOLORSTATIC:
			if( cd->hIcon == ( HWND )lp )
			{
dbg( "setbkmode for %08x", lp );
				SetBkMode( ( HDC )wp, TRANSPARENT );
				return ( INT_PTR )GetStockObject( NULL_BRUSH );
			}
			break;

		case WM_TIMER:
		{
			Globals* g = getGlobals();
			if( wp == 1234 && g->getFlag( FLG_INIT_DONE ) )
			{
				KillTimer( hwnd, 1234 );
				if( *g->getProductUrl() )	//show icon only if website url supplied
				{
					ShowWindow( cd->hIcon, SW_SHOWNORMAL );
				}
			}
		}
		break;
		

		case WM_SETCURSOR:
			if( ( HWND )wp == cd->hIcon )
			{
				HCURSOR cur = LoadCursor( 0, IDC_HAND );
				SetCursor( cur );
				return( TRUE );
			}
			break;	

		case _WM_ADD_ICON:
		{
dbg( "_WM_ADD_ICON" );
			RECT r;
			GetClientRect( cd->hChart, &r );
			int x = r.right - 25;
			int y = 3;
			cd->hIcon = CreateWindowExA( WS_EX_TOPMOST, "STATIC", "", WS_CHILD | /*WS_VISIBLE | */SS_NOTIFY | SS_ICON | SS_REALSIZEIMAGE, x, y, 16, 16, cd->hChart, 0, ( HINSTANCE )GetWindowLongPtr( cd->hChart, GWLP_HINSTANCE ), 0 );
			//icon will be shown by timer
			SetTimer( hwnd, 1234, rand() % 1000 + 3000, 0 );

	
			cd->sz->AddItem( cd->hIcon, SIZER_MOVE_RIGHT );

dbg( "Created icon container %08x", cd->hIcon );
			cd->ico = ( HICON )LoadImage( g_hInstance, MAKEINTRESOURCE( IDI_ICON1 ), IMAGE_ICON, 16, 16, 0 );
dbg( "loaded icon %08x", cd->ico );
			SendMessage( cd->hIcon, STM_SETICON, ( WPARAM )cd->ico, 0 );

			//InvalidateRect( cd->hChart, 0, 0 );
			//UpdateWindow( cd->hChart );
		}
		 return( 0 );

		case _WM_REMOVE_ICON:
		{
dbg( "_WM_REMOVE_ICON %08x", cd->hIcon );
			DestroyWindow( cd->hIcon );
			DestroyIcon( cd->ico );

			deleteChartData( cd );

			allowUnloading();	//prevented in deinit()
		}
		 return( 0 );

		case WM_DESTROY:
dbg( "chart WM_DESTROY %08x", hwnd );
			cd->isChartDestroyed = 1;
			g_sc->unsubclass( cd->hChart, ( SUBCLASSPROC )chartProc );
			break;
	}
	return( DefSubclassProc( hwnd, msg, wp, lp ) );
} // chartProc

/*DWORD WINAPI threadCloseChart( CChartData* cd )
{
	preventUnloading();
	Sleep( 1000 );
	SendMessage( GetParent( cd->hChart ), WM_CLOSE, 0, 0 );
	cd->closed = 1;
	allowUnloading();
	return( 0 );
}*/


CSimpleArray<DWORD> g_zeroIndicators;
// ===========
// init
// ===========
int init( HWND hChart, BOOL isInit, DWORD id )
{
	int res = 0;
	TRY
dbg( "init %08x, %d, %d", hChart, isInit, id );
	//ECS( g_hCharts );

	CChartData* cd = 0;
	if( hChart )
	{
dbg( "looking for chart" );
		cd = findChartData( hChart );
dbg( "found %08x", cd );
		if( isInit && cd && 
			( cd->prevDeinitReason == DEINIT_REASON_PARAMETERS ||
				cd->prevDeinitReason == DEINIT_REASON_CHARTCHANGE
			)
		)
		{
dbg( "prre" );
			res = 1;
		}
		else
		{
			if( !cd || isInit )	// !cd => 1st instance applied to this chart
								// cd && isInit => 2nd ( only for indicator )	//TODO: is it possible to apply 2 instances of same indicator to same chart?
			{
	dbg( "inc g_instances" );
				g_instances ++;
			}

			if( !cd )
			{
	dbg( "creating cd" );
				cd = new CChartData( hChart );				
	dbg( "created cd %08x", cd );
#ifndef MLL
				cd->hMT4GUIMutex = createMT4GUIMutex( hChart );
	dbg( "created mt4gui mutex" );
#endif
				int i = g_zeroIndicators.Find( id );
	dbg( "zero ind %d", i );
				if( i != -1 )
				{
					cd->decInstances();

					do
					{
						g_zeroIndicators.RemoveAt( i );
	dbg( "cd->instances=%d, incrementing as skipped", cd->getInstances() );
						cd->incInstances();

						i = g_zeroIndicators.Find( id );
					}
					while( i != -1 );
				}
	dbg( "adding cd" );
				ECS( g_hCharts );
				g_charts.Add( cd );
				LCS( g_hCharts );
	dbg( "start icon tracing" );
				cd->traceIcon();

	dbg( "started" );
				getGlobals()->setFlag( FLG_INIT_CALLED, 1 );
			}
			else if( isInit )
			{
	dbg( "cd->instances=%d,  incrementing", cd->getInstances() );
				cd->incInstances();
				getGlobals()->setFlag( FLG_INIT_CALLED, 1 );
			}
		}
	}
	else if( isInit )	//1st indicator is applied => remember this thread to increment instances later
	{
dbg( "zero init" );
		g_zeroIndicators.Add( id/*GetCurrentThreadId()*/ );

		getGlobals()->setFlag( FLG_INIT_CALLED, 1 );
	}

	if( !( cd && cd->closed ) )
	{
		res = CheckMLValidity( isInit );

		Globals* g = getGlobals();
		// for autolicense: - warn if the last allowed instance attached
		//					- close if instances limit reached
		if( isInit && res )
		{
			DWORD maxInstances = g->getAutolicenseMaxInstances();
			if( maxInstances )
			{
				cd->setProp();
				
				// without mutex on start few charts may do instances verification simultaneously and multiple msgboxes are shown
				if( WaitForSingleObject( g_hLic, 0 ) == 0 )	
				{
					DWORD usedInstances = getInstancesCount();
	dbg( "maxI %d, usedI %d", maxInstances, usedInstances );
					if( maxInstances == usedInstances )
					{
						/*You have attached maximum allowed instances (%d) of the product '%s' to your chart(s) on this computer. On next attach attempt chart will be closed. THIS IS ONLY A WARNING THAT YOU HAVE REACHED YOUR LOCAL LIMIT.*/
						BYTE key[4]={0x66,0x99,0x88,0x97};
						BYTE str[212]={0x02,0xaf,0x1a,0x3f,0xd0,0x01,0x2b,0x52,0xc9,0xea,0xaf,0x70,0x53,0x06,0x02,0xf0,0x6f,0xff,0xa4,0xe5,0xaf,0xbd,0x41,0x01,0x17,0xdc,0x3b,0xb8,0x7a,0x97,0xdd,0xee,0x23,0x4a,0xc1,0x86,0x0f,0x2b,0x51,0x5d,0x05,0x0f,0xc2,0xc5,0x8e,0x19,0xe8,0xa9,0x32,0x0c,0x46,0xb9,0xa0,0x7f,0x6b,0xa3,0x5d,0xd8,0x7c,0x72,0xa7,0x69,0xbe,0xa4,0x92,0x00,0x5a,0xbe,0x4d,0x05,0x0e,0x02,0xaa,0xac,0x3a,0x2e,0x68,0x7f,0x4a,0xb1,0x93,0x99,0xb6,0x49,0x7a,0x44,0x72,0x21,0x73,0x46,0xb1,0x00,0xbc,0xb9,0xc9,0x69,0x8d,0xb3,0xec,0xf2,0x20,0xfe,0x73,0x64,0x90,0x96,0x3f,0x53,0x2b,0x1f,0x16,0x72,0x76,0x10,0xd6,0xad,0x47,0x9b,0xb1,0xa1,0x52,0x42,0x26,0x30,0x61,0xd6,0x69,0x43,0x3c,0x12,0x5d,0xa8,0xc9,0xe4,0xac,0x1c,0x27,0x35,0x10,0x36,0xb7,0xc9,0x15,0x6e,0xd4,0x1f,0x1b,0xd5,0xd7,0xb4,0x59,0xc5,0x67,0x30,0xaf,0xef,0xcd,0xef,0xa8,0xed,0x56,0x2b,0xc5,0x61,0xa7,0x54,0x7c,0x35,0x03,0x6b,0x21,0x4b,0x61,0x13,0x33,0x8b,0x22,0x32,0xd0,0xa6,0x64,0xfd,0x43,0x81,0xfe,0x27,0x3b,0xf8,0x3d,0xff,0xa8,0x98,0xc0,0x6b,0x6b,0xbb,0xf7,0x4b,0xa6,0x9f,0xf4,0x36,0x56,0x30,0x53,0x42,0x1b,0x1a,0xe2,0x21,0x7f,0x20};
						
						char* text = ( char* )malloc( 512 );
						DECR( str, key );	
						wsprintfA( text, ( char* )str, maxInstances, g_projectName );
						ENCR( str, key );

						/*Warning - %s*/
						BYTE key2[4]={0x1b,0xc2,0x42,0x94};
						BYTE str2[13]={0xba,0xe1,0x7d,0xbf,0x7c,0xe7,0x50,0x84,0x12,0xa0,0x96,0xa8,0x84};
						DECR2( str2, key2 );
						char* title = ( char* )malloc( 256 );
						wsprintfA( title, ( char* )str2, g_projectName );
						ENCR( str2, key2 );
						CloseHandle( modelessMessageBox( text, title, MB_ICONWARNING | MB_TASKMODAL | MB_TOPMOST ) );
						free( text );
						free( title );
					}
					else if( usedInstances > maxInstances )
					{
						/*Maximum %d instances of your product '%s' are allowed to use OS-wide. This limit was reached right now so chart has been closed.*/
						BYTE key[4]={0x52,0x6d,0x8a,0x53};
						BYTE str[129]={0xd3,0xa7,0xe9,0xd4,0xce,0xd8,0x96,0x5b,0xc4,0xe9,0x34,0x06,0xb6,0x2b,0xa8,0x52,0xa8,0xa5,0x4b,0xe1,0x4a,0x27,0x7d,0xbf,0x93,0xe2,0x82,0x86,0xb1,0x82,0x15,0x3e,0x57,0xa1,0xc9,0x77,0x15,0xeb,0x1d,0x6c,0x9a,0x6f,0x04,0xea,0xc3,0xac,0xab,0x87,0xf4,0xb4,0xdd,0xcc,0x60,0x70,0x65,0xbb,0x23,0xdc,0x11,0xe2,0xe7,0x5b,0x9e,0x95,0x6c,0xa4,0x7c,0x15,0x0f,0x01,0x0f,0xc0,0xcc,0x04,0x31,0x64,0xce,0xd8,0xbc,0xb5,0x20,0x49,0xdc,0xf8,0xf6,0x6e,0xb7,0x85,0x61,0xd6,0x2f,0x3c,0x16,0xaa,0x92,0x03,0xec,0xcd,0xbe,0x42,0x70,0x01,0x19,0x4c,0xbd,0x38,0x60,0xc4,0x75,0x39,0x81,0xa5,0xd0,0xaa,0xcd,0x7b,0x28,0xb5,0xb9,0xa8,0xa8,0x6f,0x33,0x7c,0x0b,0xdd,0x67,0x35,0x1b};

						char* text = ( char* )malloc( 256 );
						DECR( str, key );
						wsprintfA( text, ( char* )str, maxInstances, g_projectName );
						ENCR( str, key );

						/*Instances limit reached - %s*/
						BYTE key2[4]={0x95,0x09,0xf3,0x0f};
						BYTE str2[29]={0xef,0xff,0xd8,0x8f,0x96,0x85,0x9d,0x86,0x0b,0xcf,0xa6,0x0a,0x47,0xb4,0x52,0xbe,0x5a,0x8b,0x77,0xcb,0xe9,0x7d,0x53,0x76,0x04,0x65,0x04,0x23,0x61};
						
						char* title = ( char* )malloc( 256 );
						DECR2( str2, key2 );
						wsprintfA( title, ( char* )str2, g_projectName );
						ENCR( str2, key2 );

						criticalError( text, title );
						free( text );
						free( title );

						//CloseHandle( myCreateThread( ( LPTHREAD_START_ROUTINE )threadCloseChart, cd, "clsch" ) );
						cd->closed = PostMessage( GetParent( cd->hChart ), WM_CLOSE, 0, 0 );
						res = 0;
					}
					ReleaseMutex( g_hLic );
				}
			}
		}
	}
dbg( "init done" );	
	//LCS( g_hCharts );
	CATCH
	return( res );
} // init

// ===================
// removeChartData
// ===================
void removeChartData( CChartData* cd )
{
	dbg( "rcd" );
	ECS( g_hCharts );
	int n = g_charts.GetCount();
	for( int i = 0; i < n; i ++ )
	{
		if( cd == g_charts[ i ] )
		{
			g_charts.RemoveAt( i );
			break;
		}
	}
	LCS( g_hCharts );
	dbg( "rcddn" );
} // removeChartData

// ===================
// deleteChartData
// ===================
void deleteChartData( CChartData* cd )
{
dbg( "dcd" );
	if( cd->subclassed && !cd->isChartDestroyed )
	{
		g_sc->unsubclass( cd->hChart, ( SUBCLASSPROC )chartProc );
	}

	removeChartData( cd );
dbg( "deleting cd" );
	delete cd;
dbg( "dcddn" );
} // deleteChartData

// ==========
// deinit
// ==========
int deinit( HWND hChart, DWORD reason )
{
	TRY
/*0	Script finished its execution independently.
REASON_REMOVE		1	Expert removed from chart.
REASON_RECOMPILE	2	Expert recompiled.
REASON_CHARTCHANGE	3	symbol or timeframe changed on the chart.
REASON_CHARTCLOSE	4	Chart closed.
REASON_PARAMETERS	5	Inputs parameters was changed by user.
REASON_ACCOUNT		6	Other account activated.
REASON_TEMPLATE		7	A new template has been applied
REASON_INITFAILED	8	This value means that OnInit() handler has returned a nonzero value
REASON_CLOSE		9	Terminal has been closed
*/
dbg( "deinit wnd=%08x rsn=%d isw=%d", hChart, reason, IsWindow( hChart ) );
	CChartData* cd = findChartData( hChart );
dbg( "cd=%08x", cd );
	BOOL ignore = FALSE;
	if( cd )
	{
		cd->prevDeinitReason = reason;
		if( reason == DEINIT_REASON_PARAMETERS ||
			reason == DEINIT_REASON_CHARTCHANGE
		)	//trying to avoid multiple deinit-init-deinit-init calls
		{
			ignore = TRUE;
		}
		else
		{
	dbg( "cd->instances=%d", cd->getInstances() );
			if( cd->decInstances() == 0 )	//the last instance removed from current chart ( or the only instance if dll is EA )
			{	
				if( !cd->hIcon )
				{
					deleteChartData( cd );
				}
				else	// current dll is icon owner
				{
					if( !cd->isChartDestroyed )
					{
	dbg( "removing icon" );
						preventUnloading();

						removeChartData( cd );	//'forget' this cd, so _WM_REMOVE_ICON handler will only have to delete cd object.
												//this is needed because possible situation when _WM_REMOVE_ICON is processed AFTER new init
												//is called. In this case existing cd will be detected and g_instances will be increased falty 
												//and icon thread will not be started

	dbg( "sending _WM_REMOVE_ICON to %08x", cd->hChart );
						BOOL sent = SendNotifyMessage( cd->hChart, _WM_REMOVE_ICON, 0, 0 );
	dbg( "sent _WM_REMOVE_ICON, res=%d (%d)", sent, GetLastError() );

						//cd is deleted in _WM_REMOVE_ICON call
					}
					else
					{
						deleteChartData( cd );
					}
				}
			}
		}
	}

	if( !ignore )
	{
		if( -- g_instances == 0 )	//the last script detached => restore auth flags
		{
	dbg( "lsti" );//"g_instances=0"
			Globals* g = getGlobals();
			/*experiment g->setFlag( FLG_AUTH_OK, 1 );
			g->setFlag( FLG_INIT_DONE, 0 );
			g->setFlag( FLG_EXPIRATION_DATE_PASSED, 0 );
			g->setLastLicenseVerificationTime( 0 );
			g->setLastDNSVerification( 0 );
			
			// until next call of mq4 init() tryInit() will ignore auth flags.
			// Else situation is possible when cpp deinit() is called, flags reseted but mq4 deinit() contains more exported functions 
			//	which contain checkMlValidity call.
			// Then xml will be loaded again ( which takes some time ) and mt4 most likely will terminate script because of mq4 deinit() timeout
			g->setFlag( FLG_INIT_CALLED, 0 );
			*/

			g_mm->stop();
			g_logger->stop();

			g->waitAndStopThreads( 5000 );

			/*
#ifdef CPPDEBUG
			int n = g_threads.GetSize();
			for( int i = 0; i < n; i ++ )
			{
				dbg( "!!- thread %04x", g_threads[ i ] );
			}
#endif
			*/
			
			g->reset();
		}
	}	
dbg( "deinit done" );
	CATCH
	return( 0 );
} // deinit

#endif // ifndef MLL

// =================
// preventUnloading
// =================
void preventUnloading()
{
dbg( "prun" );
	char me[ MAX_PATH ];
	GetModuleFileNameA( g_hInstance, me, sizeof( me ) );
	HMODULE hm = LoadLibraryA( me );	// prevent unloading dll
} // preventUnloading

DWORD WINAPI _freeLibrary( LPVOID )
{
	FreeLibraryAndExitThread( g_hInstance, 0 );
	return( 0 );
}
// ================
// allowUnloading
// ================
void allowUnloading()
{
dbg( "alun" );
	CloseHandle( myCreateThread( _freeLibrary, 0, "_fl" ) );
} // allowUnloading

// =================
// getNavLink
// =================
LINK_HANDLE getNavLink( HTREEITEM hitem )
{
	LINK_HANDLE res = 0;
	int n = g_links.GetCount();
	for( int i = 0; i < n; i ++ )
	{
		if( g_links[ i ]->hitem() == hitem )
		{
			res = g_links[ i ];
			break;
		}
	}
	return( res );
} // getNavLink

// ===================
// wndProcCtrl
// ===================
LRESULT CALLBACK wndProcCtrl( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	switch( msg )
	{
		case WM_NOTIFY:	
		{
			NMHDR* nmh = ( NMHDR* )lp;

//flog( "notify %08x %d", nmh->idFrom, nmh->code );

			if( nmh->hwndFrom == g_hwndNavTree )
			{
				LINK_HANDLE lh;
				switch( nmh->code )
				{
					case NM_DBLCLK:
					{
						HTREEITEM cur = TreeView_GetSelection( nmh->hwndFrom );
						if( nmh->code == NM_DBLCLK && cur )
						{
							lh = getNavLink( cur );
							if( lh && !lh->children().GetSize() && lh->url() && lstrlen( lh->url() ) )
							{
								openLink( lh->url() );
								break;
							}
						}

						break;
					}
				}
			}
			break;
		}
	}
	return( CallWindowProc( g_origProcNav, hwnd, msg, wp, lp ) );
} // wndProcCtrl

// ============
// _findCtrl
// ============
BOOL CALLBACK _findCtrl( HWND hwnd, LPARAM )
{
	BOOL res = TRUE;

	DWORD id = ( DWORD )GetWindowLongPtr( hwnd, GWLP_ID );

	if( id == 0x52 )	//navigator window id
	{
		g_hwndNav = hwnd;
	}
	else if( id == 0x8a6f )	//SysTreeView32 id
	{
		g_hwndNavTree = hwnd;
	}
	if( g_hwndNav && g_hwndNavTree )
	{
		res = FALSE;
	}
	return( res );
} // _findCtrl

// =================
// tryHookControls
// ==================
void tryHookControls()
{
	if( !g_hwndNavTree || !g_hwndNav )
	{
		EnumChildWindows( getMt4Window(), _findCtrl, 0 );
	}

	if( !g_origProcNav )
	{
		if( g_hwndNav && g_hwndNavTree )
		{
			g_origProcNav = ( WNDPROC )SetWindowLongPtr( g_hwndNav, GWLP_WNDPROC, ( LONG_PTR )wndProcCtrl );
		}
	}
} // tryHookControls

// ====================
// tryUnhookControls
// ====================
void tryUnhookControls()
{
dbg( "tryUnhookControls" );
	if( g_origProcNav )
	{
dbg( "g_hwndNav orig=%08x", g_origProcNav );
		SetWindowLongPtrA( g_hwndNav, GWLP_WNDPROC, ( LONG_PTR )g_origProcNav );
		g_origProcNav = 0;
	}
} // tryUnhookControls

// ===========
// setError
// ===========
void setError( HWND hChart, MLError err )
{
	CChartData* cd = findChartData( hChart );
	if( cd )
	{
		cd->err = err;
	}
} // setError

// ===============
// sendTick
// ===============
void sendTick( HWND chart )
{
	static UINT tm;
	if( !tm )
	{
		/*MetaTrader4_Internal_Message*/
		BYTE key[4]={0xb7,0xe0,0x95,0x14};
		BYTE str[29]={0xff,0xb1,0xde,0xe8,0x14,0x87,0xa5,0xd6,0x6b,0x25,0xb1,0x44,0x08,0x49,0xa3,0x94,0x15,0xab,0xba,0xe9,0x39,0x27,0x4c,0xfa,0x33,0xdf,0x66,0xe3,0x6a};
		DECR( str, key );
		tm = RegisterWindowMessageA( ( char* )str );
		ENCR( str, key );
	}
	SendNotifyMessage( chart, tm, 2, 1 );
} // sendTick

// ==============================================================================
//									CMemoryManager
// ==============================================================================
CMemoryManager::CMemoryManager( DWORD ttl )
{
	m_ttl = ttl;
	m_tid = 0;
	
	initBlocks();

	m_hStopped = CreateEvent( 0, 0, 0, 0 );
	m_hMutex = CreateMutex( 0, 0, 0 );
	m_hThread = 0;
}

CMemoryManager::~CMemoryManager()
{
dbg( "~cm" );
	freeBlocks();
	CloseHandle( m_hMutex );
dbg( "~cmdn" );
}

void CMemoryManager::initBlocks()
{
	m_blocks = 0;
	m_totalBlocks = 0;
	addBlock( 0 );	//default block size
}

void CMemoryManager::freeBlocks()
{
	for( int i = 0; i < m_totalBlocks; i ++ )
	{
dbg( "CM free %08x", m_blocks[ i ].buf );
		free( m_blocks[ i ].buf );
	}
	free( m_blocks );
}

// ==============
// start
// ==============
BOOL CMemoryManager::start()
{
	if( !m_hThread )
	{
		m_hThread = myCreateThread( ( LPTHREAD_START_ROUTINE )threadProc, this, "CM::threadProc", 0, &m_tid );
dbg( "CM started thread %04x(%08x)", m_tid, m_hThread );
	}
	else
	{
dbg( "!!-- CM e" );
	}
	return( m_hThread != 0 );
} // start

// =============
// stop
// =============
BOOL CMemoryManager::stop()
{
	DWORD r = ResumeThread( m_hThread );
	if( r != 0 )
	{
		dbg( "!!-- rsm %d (%d)", r, GetLastError() );
	}

	SetEvent( m_hStopped );
	DWORD w = WaitForSingleObject( m_hThread, m_ttl * 5 );
	CloseHandle( m_hThread );
	if( w != 0 )
	{
		dbg( "!!- fstp %d", GetLastError() );
	}
	ResetEvent( m_hStopped );
	m_hThread = 0;
dbg( "CM stopped" );
	return( w == 0 );
} // stop

// =================
// threadProc
// =================
DWORD WINAPI CMemoryManager::threadProc( CMemoryManager* me )
{
	TRY

#ifdef CPPDEBUG
	DWORD lastMemoryCheck = 0;
#endif

	DWORD i;
	HANDLE h[] = { me->m_hMutex, me->m_hStopped };
	for( ;; )
	{
dbg( "A" );
		int waited = WaitForMultipleObjects( 2, h, FALSE, INFINITE );
dbg( "B %d", waited );
		if( waited == 1 )
		{	
			break;
		}

		if( me->m_totalBlocks > 1 )
		{
			DWORD ticksNow = GetTickCount();

#ifdef CPPDEBUG
			if( isTimePassed( lastMemoryCheck, 60000, ticksNow ) )
			{
			dbg( "E" );
				lastMemoryCheck = ticksNow;

				MEMORYSTATUSEX mem;
				mem.dwLength = sizeof( mem );

				GlobalMemoryStatusEx( &mem );
			dbg( "F" );
				DWORD total = ( mem.ullTotalPhys / 1024 ) & 0xffffffff;
				DWORD avail = ( mem.ullAvailPhys / 1024 ) & 0xffffffff;
				dbg( "%d percents of memory used, total=%d kb, avail=%d kb", mem.dwMemoryLoad, total, avail );
			}
#endif
	
			for( i = 0; i < me->m_totalBlocks - 1; i ++ )
			{
				if( ticksNow - me->m_blocks[ i ].lastAddTime >= me->m_ttl )
				{
//dbg( "CM free %08x", me->m_items[ i ].buf );
dbg( "8" );
					free( me->m_blocks[ i ].buf );
dbg( "9" );
				}
				else
				{
					break;
				}
			}

			if( i > 0 )
			{
dbg( "10" );
				memmove( me->m_blocks, me->m_blocks + i, ( me->m_totalBlocks - i ) * sizeof( MemoryBlock ) );
dbg( "11" );
				me->m_totalBlocks -= i;
dbg( "12" );
				me->m_blocks = ( MemoryBlock* )realloc( me->m_blocks, me->m_totalBlocks * sizeof( MemoryBlock ) );
dbg( "13" );
				me->m_curBlock = me->m_blocks + me->m_totalBlocks - 1;
dbg( "14" );
			}
		}
//dbg( "CM loop done, %d items left", me->m_items.GetCount() );
		ReleaseMutex( me->m_hMutex );
dbg( "15" );
		if( WaitForSingleObject( me->m_hStopped, me->m_ttl * 4 ) == 0 )
		{
dbg( "16" );
			break;
		}
		verifyThreadGuardian();
dbg( "17" );
	}

dbg( "CM stopped, %d blocks", me->m_totalBlocks );
	me->freeBlocks();
	me->initBlocks();

	CATCH
	return( 0 );
} // CMemoryManager::threadProc

// ==================
// add
// ==================
void* CMemoryManager::add( DWORD len )
{
	register void* res;
	TRY
	
	WaitForSingleObject( m_hMutex, INFINITE );
	if( m_curBlock->offset + len > m_bufferSize )
	{
		addBlock( len );
	}
	res = ( BYTE* )m_curBlock->buf + m_curBlock->offset;
	m_curBlock->offset += len;
	m_curBlock->lastAddTime = GetTickCount();
	ReleaseMutex( m_hMutex );
	
	CATCH
	return( res );
} // add

// ==============
// addBlock
// ==============
void CMemoryManager::addBlock( DWORD len )
{
	TRY
	m_totalBlocks ++;
dbg( "addBlock, %d", m_totalBlocks );
	m_blocks = ( MemoryBlock* )realloc( m_blocks, m_totalBlocks * sizeof( MemoryBlock ) );

	if( !m_blocks )
	{
		dbg( "!!-- CM nm(%d)", 2 );
		throw( 0x22334455 );
	}

dbg( "m_blocks=%08x", m_blocks );
	m_curBlock = m_blocks + m_totalBlocks - 1;
dbg( "m_curBlock=%08x", m_curBlock );

	// don't want to alloc small buffers. 
	// If required len is smaller than default buffer size then alloc default buffer size.
	// But if len is bigger than default buffer size then alloc required len bytes
	if( len < m_bufferSize )
	{
		len = m_bufferSize;
	}

	m_curBlock->buf = mymalloc( len );
dbg( "buf=%08x", m_curBlock->buf );
	if( !m_curBlock->buf )
	{
		dbg( "!!-- CM nm(%d)", 2 );
		throw( 0x13243546 );
	}
	m_curBlock->lastAddTime = 0;
	m_curBlock->offset = 0;
	CATCH
} // addBlock


#ifdef GEN_LT_REMOTE
// =============================================================================
//									CLic
// =============================================================================
CLic::CLic()
{
	holdMessage = 0;
	update = FALSE;
	autoUpdate = AU_NO_NEED;
	updateUrl = updateMessage = 0;
	websiteUrl = 0;
	supportEmail = 0;
	supportName = 0;
	expiration = 0;
	userExpiration = 0;
	accType = ACC_REAL | ACC_DEMO;
	ping.port = PING_PORT;
	ping.period = PING_PERIOD;
	skipForDemo = FALSE;
	*lastCre1 = 0;
	*lastCre2 = 0;
}

CLic::CLic( const CLic& lic )
{
	memcpy( m_rc4Key, lic.m_rc4Key, sizeof( m_rc4Key ) );

	type = lic.type;
	expiration = lic.expiration;
	userExpiration = lic.userExpiration;
	isValid = lic.isValid;
	skipForDemo = lic.skipForDemo;
	hold = lic.hold;
	copyStr( &holdMessage, lic.holdMessage );
	update = lic.update;
	autoUpdate = lic.autoUpdate;
	ping = lic.ping;
	copyStr( &updateUrl, lic.updateUrl );
	copyStr( &updateMessage, lic.updateMessage );
	copyStr( &websiteUrl, lic.websiteUrl );
	copyStr( &supportEmail, lic.supportEmail );
	copyStr( &supportName, lic.supportName );

	strcpy( lastCre1, lic.lastCre1 );
	strcpy( lastCre2, lic.lastCre2 );
}

char* CLic::copyStr( char** dst, const char* src )
{
	if( src )
	{
		int len = strlen( src ) + 1 ;
		*dst = ( char *)mymalloc( len );
		memcpy( *dst, src, len );
	}
	else
	{
		*dst = 0;
	}
	return( *dst );
}

CLic::~CLic()
{
#if defined( GEN_LT_REMOTE_OPEN )
#elif defined( GEN_LT_REMOTE_LOGIN )
#elif defined( GEN_LT_REMOTE_ACCOUNT_NO )
#elif defined( GEN_LT_REMOTE_CID )
#endif
dbg( "~CLic" );
	if( holdMessage )
	{
		free( holdMessage );
	}
	if( updateUrl )
	{
		free( updateUrl );
	}
	if( updateMessage )
	{
		free( updateMessage );
	}
	if( websiteUrl )
	{
		free( websiteUrl );
	}
	if( supportEmail )
	{
		free( supportEmail );
	}
	if( supportName )
	{
		free( supportName );
	}
	memset( lastCre1, 0, sizeof( lastCre1 ) );
	memset( lastCre2, 0, sizeof( lastCre2 ) );
dbg( "~CLic done" );
}

// =============
// load
// =============
BOOL CLic::load( char *rawxml, const BYTE* xmlKey, DWORD keyLen )
{
	BOOL res = FALSE;
	
	TRY

	isValid = FALSE;

	CXMLNodeEnc xml;
	if( xml.Load( rawxml ) )
	{
dbg( "loading rv vars" );
		if( !g_rv )
		{
			g_rv = new CRV( xmlKey, keyLen );
		}
		if( !g_rv->loadXml( &xml ) )
		{
			delete g_rv;
			g_rv = 0;
		}

dbg( "xml.Load ok" );
		res = TRUE;
		// licensing
		BYTE keyLicensing[4] = {0x16,0x93,0x06,0xad};
		BYTE strLicensing[10]={0x82,0xcd,0x6f,0x1a,0x86,0x16,0x13,0x5b,0xa9,0xe7};
		// type
		BYTE keyType[4] = {0x5d,0x45,0x3a,0x0f};
		BYTE strType[5]={0xcd,0xe4,0xd5,0x18,0xdd};
		// expiration
		BYTE keyExpiration[4] = {0x22,0x45,0xfe,0xed};
		BYTE strExpiration[11]={0x1e,0x97,0x68,0xdb,0x6a,0x88,0x5f,0x0f,0xb9,0x53,0xa8};
		// pp_expiration
		BYTE keyPpExpiration[4] = {0x08,0x9d,0x0f,0x01};
		BYTE strPpExpiration[14]={0xaa,0xf3,0x5a,0x49,0x0b,0xcd,0x37,0x40,0x64,0x7f,0xb5,0xf7,0x50,0x8f};
		// holds
		BYTE keyHolds[4] = {0x25,0x1e,0xcc,0xb9};
		BYTE strHolds[6]={0x4f,0x33,0x67,0xa8,0x0c,0xbf};
		// updates
		BYTE keyUpdates[4] = {0xe1,0x1b,0x78,0x96};
		BYTE strUpdates[8]={0x31,0xf1,0x24,0x81,0x30,0x8d,0x21,0xd6};
		// revisions
		BYTE keyRevisions[4] = {0xf8,0xda,0x99,0x92};
		BYTE strRevisions[10]={0xca,0x79,0xee,0x92,0xc0,0x9d,0x82,0x41,0x33,0x77};
		// number
		BYTE keyNumber[4] = {0xe7,0xcc,0x94,0x2e};
		BYTE strNumber[7]={0xd5,0x7b,0x83,0x62,0xc0,0x97,0x3d};
		// rev
		BYTE keyRev[4] = {0xfa,0xe5,0xa8,0xcb};
		BYTE strRev[4]={0x14,0xec,0x2c,0xc6};
		// rev_op
		BYTE keyRevOp[4] = {0x18,0x72,0x66,0x81};
		BYTE strRevOp[7]={0xa0,0x78,0x64,0xf7,0x9a,0x2e,0x84};
		// message
		BYTE keyMessage[4] = {0x28,0x65,0x6c,0xc2};
		BYTE strMessage[8]={0xf0,0x0b,0x87,0xd8,0xd9,0xfa,0xcc,0x18};
		// enabled
		BYTE keyEnabled[4] = {0x56,0x31,0x70,0x4b};
		BYTE strEnabled[8]={0x74,0xa6,0xf4,0x20,0x0d,0xbf,0x3b,0xfd};
		// url
		BYTE keyUrl[4] = {0x80,0x89,0xa6,0xe4};
		BYTE strUrl[4]={0x59,0xa2,0x98,0x08};
		// website_url
		BYTE keyWebsiteUrl[4] = {0xbd,0x15,0x70,0xc0};
		BYTE strWebsiteUrl[12]={0x96,0x8f,0xf3,0xe4,0x1d,0xc3,0x21,0x28,0x40,0xd8,0x71,0x1b};
		// revision
		BYTE keyRevision[4] = {0xcd,0x08,0x76,0x81};
		BYTE strRevision[9]={0xaf,0x59,0xd3,0x8a,0x46,0xcd,0x0c,0xa4,0xf8};
		// meta
		BYTE keyMeta[4] = {0x9f,0x21,0x73,0x55};
		BYTE strMeta[5]={0x6a,0x77,0xf6,0x8b,0x74};
		// projectname
		BYTE keyProjectName[4] = {0x89,0x19,0x2c,0x27};
		BYTE strProjectName[12]={0xb5,0x41,0xcd,0x64,0xff,0x0c,0xde,0xcd,0x26,0x7a,0x9d,0x1f};
		/*real_demo*/
		BYTE keyRealDemo[4]={0x8a,0x7b,0xe8,0x91};
		BYTE strRealDemo[10]={0x73,0xac,0x87,0x7a,0xeb,0xef,0x87,0xeb,0x58,0xb8};
		/*skip_for_demo*/
		BYTE keySkipForDemo[4]={0x01,0x04,0xdc,0x21};
		BYTE strSkipForDemo[14]={0x34,0xb3,0xa0,0x63,0x51,0xd9,0x19,0xa4,0x30,0xe3,0xbb,0xf3,0x9d,0x36};
		/*auto_update*/
		BYTE keyAutoUpdate[4]={0x80,0xd7,0xfc,0x41};
		BYTE strAutoUpdate[12]={0xe7,0xf0,0xd3,0x86,0x37,0x51,0x60,0xef,0xb0,0xc2,0x9f,0x34};
		/*update*/
		BYTE keyUpdate[4]={0x2c,0x00,0x6a,0xe7};
		BYTE strUpdate[7]={0x47,0xd3,0x72,0xf3,0xd9,0x59,0x0e};
		/*filename*/
		BYTE keyFilename[4]={0x0a,0x64,0x1a,0xda};
		BYTE strFilename[9]={0xb1,0x68,0x25,0xb4,0xf1,0xb7,0x10,0x8d,0x9f};
		/*public_email*/
		BYTE keyPublicEmail[4]={0xf4,0xf1,0x5d,0xe1};
		BYTE strPublicEmail[13]={0x26,0xf8,0xb2,0x6b,0x6d,0x3c,0x4d,0xd3,0x65,0xf3,0x13,0x45,0x9d};
		/*public_user_name*/
		BYTE keyPublicName[4]={0x0f,0xcd,0xbf,0x5a};
		BYTE strPublicName[17]={0x30,0x8d,0x0c,0x52,0x12,0xf1,0x1c,0xbb,0xb3,0xf2,0xae,0xde,0x14,0x49,0xbf,0xea,0xfd};
		/*ping*/
		BYTE keyPing[4]={0x8f,0x8c,0xc5,0x78};
		BYTE strPing[5]={0xdc,0x55,0x5c,0x60,0xaf};
		/*port*/
		BYTE keyPort[4]={0xd7,0x3d,0xf9,0x69};
		BYTE strPort[5]={0x89,0xce,0xfd,0x62,0xca};
		/*period*/
		BYTE keyPeriod[4]={0x49,0xc0,0x57,0x51};
		BYTE strPeriod[7]={0x08,0xf8,0xc6,0xa6,0x16,0x6b,0x34};


		CRC4 rc4;
		rc4.Decrypt( strLicensing, sizeof( strLicensing ), keyLicensing, sizeof( keyLicensing ) );
		rc4.Decrypt( strType, sizeof( strType ), keyType, sizeof( keyType ) );
		rc4.Decrypt( strExpiration, sizeof( strExpiration ), keyExpiration, sizeof( keyExpiration ) );
		rc4.Decrypt( strPpExpiration, sizeof( strPpExpiration ), keyPpExpiration, sizeof( keyPpExpiration ) );
		rc4.Decrypt( strHolds, sizeof( strHolds ), keyHolds, sizeof( keyHolds ) );
		rc4.Decrypt( strUpdates, sizeof( strUpdates ), keyUpdates, sizeof( keyUpdates ) );
		rc4.Decrypt( strRevisions, sizeof( strRevisions ), keyRevisions, sizeof( keyRevisions ) );
		rc4.Decrypt( strNumber, sizeof( strNumber ), keyNumber, sizeof( keyNumber ) );
		rc4.Decrypt( strRev, sizeof( strRev ), keyRev, sizeof( keyRev ) );
		rc4.Decrypt( strRevOp, sizeof( strRevOp ), keyRevOp, sizeof( keyRevOp ) );
		rc4.Decrypt( strMessage, sizeof( strMessage ), keyMessage, sizeof( keyMessage ) );
		rc4.Decrypt( strEnabled, sizeof( strEnabled ), keyEnabled, sizeof( keyEnabled ) );
		rc4.Decrypt( strUrl, sizeof( strUrl ), keyUrl, sizeof( keyUrl ) );
		rc4.Decrypt( strWebsiteUrl, sizeof( strWebsiteUrl ), keyWebsiteUrl, sizeof( keyWebsiteUrl ) );
		rc4.Decrypt( strRevision, sizeof( strRevision ), keyRevision, sizeof( keyRevision ) );
		rc4.Decrypt( strMeta, sizeof( strMeta ), keyMeta, sizeof( keyMeta ) );
		rc4.Decrypt( strProjectName, sizeof( strProjectName ), keyProjectName, sizeof( keyProjectName ) );
		rc4.Decrypt( strRealDemo, sizeof( strRealDemo ), keyRealDemo, sizeof( keyRealDemo ) );
		rc4.Decrypt( strSkipForDemo, sizeof( strSkipForDemo ), keySkipForDemo, sizeof( keySkipForDemo ) );
		rc4.Decrypt( strAutoUpdate, sizeof( strAutoUpdate ), keyAutoUpdate, sizeof( keyAutoUpdate ) );
		rc4.Decrypt( strUpdate, sizeof( strUpdate ), keyUpdate, sizeof( keyUpdate ) );
		rc4.Decrypt( strFilename, sizeof( strFilename ), keyFilename, sizeof( keyFilename ) );
		rc4.Decrypt( strPublicEmail, sizeof( strPublicEmail ), keyPublicEmail, sizeof( keyPublicEmail ) );
		rc4.Decrypt( strPublicName, sizeof( strPublicName ), keyPublicName, sizeof( keyPublicName ) );
		DECR2( strPing, keyPing );
		DECR2( strPort, keyPort );
		DECR2( strPeriod, keyPeriod );
		

		const CXMLNodeEnc *licensing = xml.GetChildA( ( const char* )strLicensing, xmlKey, keyLen );
		if( licensing )
		{
			const CXMLNodeEnc* type = licensing->GetChildA( ( const char* )strType, xmlKey, keyLen );
			const CXMLNodeEnc* expiration = licensing->GetChildA( ( const char* )strExpiration, xmlKey, keyLen );
			const CXMLNodeEnc* pp_expiration = licensing->GetChildA( ( const char* )strPpExpiration, xmlKey, keyLen );
			const CXMLNodeEnc* holds = xml.GetChildA( ( const char* )strHolds, xmlKey, keyLen );
			const CXMLNodeEnc* updates = xml.GetChildA( ( const char* )strUpdates, xmlKey, keyLen );
			const CXMLNodeEnc* revisions = licensing->GetChildA( ( const char* )strRevisions, xmlKey, keyLen );
			const CXMLNodeEnc* skipForDemo = licensing->GetChildA( ( const char* )strSkipForDemo, xmlKey, keyLen );

			if( skipForDemo )
			{
				const char* sfd = skipForDemo->GetTextA( xmlKey, keyLen );
				if( sfd )
				{
					this->skipForDemo = atoi( sfd );
				}
			}

			if( type && expiration )
			{
				Globals* g = getGlobals();

				const TCHAR* exp = expiration->GetText( xmlKey, keyLen );
dbg( "expiration->GetText=%08x", exp );
				if( exp )
				{
dbg( "prjexp='%s'", exp );
					this->expiration = _tstol( exp );
					g->expirationReason = EXPIRATION_REASON_PROJECT;
				}
				else
				{
					this->expiration = 0;
				}

				// ---------
				// ping 
				// ---------
				const CXMLNodeEnc* xPing = xml.GetChildA( ( const char* )strPing, xmlKey, keyLen );
				if( xPing )
				{
					const char* s = xPing->GetChildTextA( ( const char* )strPort, xmlKey, keyLen );
					if( s )
					{
						ping.port = atoi( s );
						dbg( "ping port %hu", ping.port );
					}
					s = xPing->GetChildTextA( ( const char* )strPeriod, xmlKey, keyLen );
					if( s )
					{
						ping.period = atoi( s );
						dbg( "ping period %d", ping.period );
					}

				}

				// ----------------------
				// pp_expiration
				// ----------------------
				if( pp_expiration )
				{
					dbg( "has pp_expiration" );
				
					exp = pp_expiration->GetText( xmlKey, keyLen );
					if( exp )
					{
						dbg( "pp_expiration node not empty: '%s'", exp );
						long iExp = _tstol( exp ) + 86400;				//THIS GIVES 1 DAY BUFFER TO VENDOR TO UPDATE PAYMENT
						if( !this->expiration || this->expiration > iExp )
						{
							this->expiration = iExp;
							g->expirationReason = EXPIRATION_REASON_PAYPAL;
						}
					}
				}

				// -------------------
				// revision expiration
				// -------------------
				if( revisions )
				{
					const CXMLNodesEnc revision = revisions->GetChildrenA( ( const char *)strRevision, xmlKey, keyLen );
					DWORD n = revision.GetSize();
					for( DWORD i = 0; i < n; i ++ )
					{
						const char* number = revision[ i ]->GetChildTextA( ( const char* )strNumber, xmlKey, keyLen );
						const char* expiration = revision[ i ]->GetChildTextA( ( const char* )strExpiration, xmlKey, keyLen );
						if( number && expiration )
						{
							if( atoi( number ) == g->getRevision() )
							{
								long iExp = atol( expiration );
								if( !this->expiration || this->expiration > iExp )
								{
									this->expiration = iExp;
									g->expirationReason = EXPIRATION_REASON_REVISION;
								}
								break;
							}
						}
					}
				}

				// -------------------------
				// fetch meta information
				// -------------------------
				const CXMLNodeEnc* meta = xml.GetChildA( ( const char *)strMeta, xmlKey, keyLen );
				if( meta )
				{
					const char* websiteUrl = meta->GetChildTextA( ( const char* )strWebsiteUrl, xmlKey, keyLen );
					if( websiteUrl )
					{
						copyStr( &this->websiteUrl, websiteUrl );
					}

					const char* projectName = meta->GetChildTextA( ( const char* )strProjectName, xmlKey, keyLen );
					if( projectName )
					{
						strcpy( g_projectName, projectName );
					}

					const char* supportEmail = meta->GetChildTextA( ( const char* )strPublicEmail, xmlKey, keyLen );
					if( supportEmail )
					{
						copyStr( &this->supportEmail, supportEmail );
					}
					else
					{
						copyStr( &this->supportEmail, "" );
					}

					const char* supportName = meta->GetChildTextA( ( const char* )strPublicName, xmlKey, keyLen );
					if( supportName )
					{
						copyStr( &this->supportName, supportName );
					}
					else
					{
						copyStr( &this->supportName, "" );
					}
				}

				// ------------------------
				// holds
				// ------------------------
				this->hold = FALSE;
				if( holds )
				{
					CXMLNodesEnc hold = holds->GetChildren( NULL, xmlKey, keyLen );

					int n = hold.GetSize();
dbg( "holds %d", n );
					for( int i = 0; i < n; i ++ )
					{
						const char* srev = hold[ i ]->GetChildTextA( ( const char* )strRev, xmlKey, keyLen );
						if( !srev )
						{
							continue;
						}
						int rev = atoi( srev );

						RevOp revOp = ( !strcmp( hold[ i ]->GetChildTextA( ( const char *)strRevOp, xmlKey, keyLen ), "eq" ) ? REV_OP_EQ : REV_OP_LT );
dbg( "hold %d rev=%d, rev_op=%d", i, rev, revOp );
						if( ( rev == g->getRevision() && revOp == REV_OP_EQ ) ||
							( g->getRevision() < rev && revOp == REV_OP_LT )
						)
						{
							const char* enabled = hold[ i ]->GetChildTextA( ( const char* )strEnabled, xmlKey, keyLen );

							if( enabled )
							{
dbg( "enabled %s", enabled );
								this->hold = atoi( enabled );
							}

							if( this->hold )
							{
								const char* t = hold[ i ]->GetChildTextA( ( const char* )strMessage, xmlKey, keyLen );
								if( t )
								{
dbg( "message '%s'", t );
									int len = strlen( t ) + 1;
dbg( "len %d", len );
									this->holdMessage = ( char* )mymalloc( len );
									memcpy( this->holdMessage, t, len );
								}
								else
								{
									this->holdMessage = ( char* )mymalloc( 1 );
									*this->holdMessage = 0;
								}
								break;
							}
						}
					}

				}

				// ----------------------------
				// updates
				// ----------------------------
				this->update = FALSE;
				this->autoUpdate = AU_NO_NEED;
				if( updates )
				{
dbg( "updsE" );
					const CXMLNodeEnc* autoUpdate = updates->GetChildA( ( const char* )strAutoUpdate, xmlKey, keyLen );
					//CHECK IF AUTO UPDATE REQUIRED
					if( 0 && autoUpdate )	//22.09.2015 autoupdate disabled
					{
dbg( "auE" );
						//process auto update only if precompiled filename matches xml filename
						const char* filename = autoUpdate->GetChildTextA( ( const char* )strFilename, xmlKey, keyLen );
						const char* precompiledFilename = g->useDllName();
						char* dllRealName= ( char* )mymalloc( MAX_PATH );
						GetModuleFileNameA( g_hInstance, dllRealName, MAX_PATH );
						char* p = strrchr( dllRealName, '\\' ) + 1;
						char* dot = strrchr( p, '.' );
						if( dot )
						{
							*dot= 0;
						}
						if( filename )	//auto update exists
						{
							if( strcmp( filename, precompiledFilename ) == 0 &&
								strcmp( filename, p ) == 0
							)
							{
								const char* updateRev = autoUpdate->GetChildTextA( ( const char* )strRev, xmlKey, keyLen );
								if( updateRev )
								{
		dbg( "aur %s", updateRev );
									if( atoi( updateRev ) != g->getRevision() )
									{
										this->autoUpdate = AU_NEED;
									}
								}
							}
							else
							{
								this->autoUpdate = AU_CANNOT;

								g_logger->submit( LGT_AU_CANNOT, filename, p, precompiledFilename, 0 );
							}
						}
						free( dllRealName );
						g->unuse( precompiledFilename );
					}
					//if auto update is needed then no needed check revisions updates
					if( this->autoUpdate == AU_NO_NEED )
					{
						CXMLNodesEnc update = updates->GetChildrenA( ( const char* )strUpdate, xmlKey, keyLen );

						int n = update.GetSize();
	dbg( "Found %d update items", n );
						for( int i = 0; i < n; i ++ )
						{
	dbg( "%d update item", i );
							const char* srev = update[ i ]->GetChildTextA( ( const char *)strRev, xmlKey, keyLen );
							if( !srev )
							{
	dbg( "no rev" );
								continue;
							}
							int rev = atoi( srev );
	dbg( "rev=%d", rev );
							const char* sop = update[ i ]->GetChildTextA( ( const char* )strRevOp, xmlKey, keyLen );
							if( !sop )
							{
	dbg( "no op" );
								continue;
							}
	dbg( "op=%s", sop );
							RevOp revOp = ( !strcmp( sop, "eq" ) ? REV_OP_EQ : REV_OP_LT );
							if( ( rev == g->getRevision() && revOp == REV_OP_EQ ) ||
								( rev > g->getRevision() && revOp == REV_OP_LT )
							)
							{
	dbg( "revision matched" );
								const char* upd = update[ i ]->GetChildTextA( ( const char *)strEnabled, xmlKey, keyLen );
								if( upd )
								{
	dbg( "update enabled %s", upd );
									this->update = atoi( upd );
								}
								if( this->update )
								{
									const char* t = update[ i ]->GetChildTextA( ( const char* )strUrl, xmlKey, keyLen );
									int len;
									if( t )
									{
	dbg( "have update url '%s'", t );
										len = strlen( t ) + 1;
										this->updateUrl = ( char* )mymalloc( len );
										memcpy( this->updateUrl, t, len );
									}
									else
									{
	dbg( "no update url" );
										this->updateUrl = ( char* )mymalloc( 1 );
										*this->updateUrl = 0;
									}

									t = update[ i ]->GetChildTextA( ( const char *)strMessage, xmlKey, keyLen );
									if( t )
									{
	dbg( "have update message '%s'", t );
										len = strlen( t ) + 1;
dbg( "len %d", len );
										this->updateMessage = ( char* )mymalloc( len );
										memcpy( this->updateMessage, t, len );
									}
									else
									{
	dbg( "no update message" );
										this->updateMessage = ( char* )mymalloc( 1 );
										*this->updateMessage = 0;
									}
									break;
								}
							}
						}
					}
				}
				else
				{
dbg( "updates node not found" );
				}


				// open
				BYTE keyOpen[4] = {0xdc,0x8a,0xa9,0x9e};
				BYTE strOpen[5]={0x5c,0xdf,0x6d,0x7c,0x1e};
				// login
				BYTE keyLogin[4] = {0x29,0xc5,0x18,0x0a};
				BYTE strLogin[6]={0x27,0xf7,0xee,0xb6,0x28,0xf1};
				// pass
				BYTE keyPass[4] = {0x32,0x1d,0x42,0xfe};
				BYTE strPass[5]={0x94,0x2c,0x4c,0xe2,0x3a};
				// account_no
				BYTE keyAccountNo[4] = {0x20,0x28,0x7e,0xe4};
				BYTE strAccountNo[11]={0xea,0xe5,0xa7,0x0f,0x20,0x0e,0xce,0x1f,0x78,0x1a,0x09};
				// logins
				BYTE keyLogins[4] = {0x59,0xef,0x7c,0x66};
				BYTE strLogins[7]={0xfe,0xe0,0x2e,0xaf,0x27,0x90,0x46};
				// account_nos
				BYTE keyAccountNos[4] = {0x79,0xd5,0x09,0xe7};
				BYTE strAccountNos[12]={0x73,0x60,0xaa,0x76,0xc3,0xc8,0x1c,0xfc,0x12,0xb8,0x75,0x67};
				// cid
				BYTE keyCid[4] = {0x27,0xfa,0xac,0xa8};
				BYTE strCid[4]={0xc1,0xbf,0xf0,0x94};
				// cids
				BYTE keyCids[4] = {0x5b,0x93,0x4c,0x6b};
				BYTE strCids[5]={0x7e,0xef,0xe1,0x74,0x15};
				// receipt
				BYTE keyReceipt[4] = {0x54,0x79,0x70,0x1f};
				BYTE strReceipt[8]={0xf6,0xaf,0xbf,0xd3,0x87,0x80,0x1b,0xa9};
				// receipts
				BYTE keyReceipts[4] = {0x77,0x52,0x4a,0x8f};
				BYTE strReceipts[9]={0xd9,0xf9,0x7e,0x5d,0x2c,0x3e,0x88,0xce,0xbc};
				/*auto*/
				BYTE keyAuto[4]={0xcb,0x25,0x82,0x5a};
				BYTE strAuto[5]={0x07,0x9d,0x52,0x1c,0xa3};

				DECR2( strOpen, keyOpen );
				DECR2( strLogins, keyLogins );
				DECR2( strLogin, keyLogin );
				DECR2( strPass, keyPass );
				DECR2( strAccountNo, keyAccountNo );
				DECR2( strAccountNos, keyAccountNos );
				DECR2( strCid, keyCid );
				DECR2( strCids, keyCids );
				DECR2( strReceipt, keyReceipt );
				DECR2( strReceipts, keyReceipts );
				DECR2( strAuto, keyAuto );

				BOOL real = g_ac->isReal( TRUE );

				// -----------------------
				//			OPEN
				// -----------------------
				if( !strcmp( type->GetTextA( xmlKey, keyLen ), ( const char *)strOpen ) )
				{
					this->type = LT_REMOTE_OPEN;
					isValid = TRUE;

					const CXMLNodeEnc* open= licensing->GetChildA( ( const char* )strOpen, xmlKey, keyLen );
					if( open )
					{
						const CXMLNodeEnc* rd = open->GetChildA( ( const char* )strRealDemo, xmlKey, keyLen );
						if( rd )
						{
							accType = atoi( rd->GetTextA( xmlKey, keyLen ) );
						}
					}
				}
				// -------------------------
				//			AUTO
				// ------------------------
				else if( !strcmp( type->GetTextA( xmlKey, keyLen ), ( const char* )strAuto ) )
				{
					this->type = LT_REMOTE_AUTO;
					isValid = TRUE;
				}
				// -------------------------
				//			LOGIN
				// -------------------------
				else if( !strcmp( type->GetTextA( xmlKey, keyLen ), ( const char* )strLogin ) )
				{
					this->type = LT_REMOTE_LOGIN;
					isValid = TRUE;
				}
				// -----------------------------
				//			ACCOUNT_NO
				// -----------------------------
				else if( !strcmp( type->GetTextA( xmlKey, keyLen ), ( const char* )strAccountNo ) )
				{
dbg( "acn" );
					this->type = LT_REMOTE_ACCOUNT_NO;

					const char* acc = g_ac->getNumber( TRUE );
	dbg( "getNumber=%s", acc );

					if( real || !this->skipForDemo )
					{
						const CXMLNodeEnc* accounts = licensing->GetChildA( ( const char* )strAccountNos, xmlKey, keyLen );
						if( accounts )
						{
							CXMLNodesEnc nodes = accounts->GetChildrenA( ( const char* )strAccountNo, xmlKey, keyLen );
							DWORD n = nodes.GetSize();
	dbg( "ndsz=%d", n );
							for( DWORD i = 0; i < n; i ++ )
							{
								const char* account = nodes[ i ]->GetChildTextA( ( const char* )strAccountNo, xmlKey, keyLen );
	dbg( "acc=%s", account );
								if( account && !strcmp( acc, account ) )
								{
	dbg( "acmtch" );
									const char* enabled = nodes[ i ]->GetChildTextA( ( const char* )strEnabled, xmlKey, keyLen );
									if( enabled && !atoi( enabled ) )
									{
	dbg( "acdbl" );
										continue;
									}

									const char* expiration = nodes[ i ]->GetChildTextA( ( const char* )strExpiration, xmlKey, keyLen );
									if( expiration )
									{
	dbg( "achex %s", expiration );
										this->userExpiration = atol( expiration );
									}
									const char* accType = nodes[ i ]->GetChildTextA( ( const char* )strRealDemo, xmlKey, keyLen );
									if( accType )
									{
										this->accType = atoi( accType );
	dbg( "actp %d", this->accType );
									}
	dbg( "acvld" );
									strcpy( this->lastCre1, acc );

									isValid = TRUE;
									break;
								}
							}
	dbg( "acchkd" );
						}

					}
					else
					{
						strcpy( this->lastCre1, acc );
						isValid = TRUE;
					}
				}
				// -----------------------------
				//				CID
				// -----------------------------
				else if( !strcmp( type->GetTextA( xmlKey, keyLen ), ( const char* )strCid ) )
				{
dbg( "CID licensing" );
					this->type = LT_REMOTE_CID;
					
					const char* cid2 = GetComputerID2();
	dbg( "my cid '%s'", cid2 );

					if( real || !this->skipForDemo )
					{
						const CXMLNodeEnc* cids = licensing->GetChildA( ( const char* )strCids, xmlKey, keyLen );
						if( cids )
						{
							CXMLNodesEnc nodes = cids->GetChildrenA( ( const char* )strCid, xmlKey, keyLen );
							DWORD n = nodes.GetSize();
	dbg( "%d cids allowed", n );
							for( DWORD i = 0; i < n; i ++ )
							{
								const char* lcid = nodes[ i ]->GetChildTextA( ( const char* )strCid, xmlKey, keyLen );
	dbg( "cid %d = '%s'", i, lcid );
								if( lcid && !_stricmp( cid2, lcid ) )
								{
	dbg( "cidmtch" );
									const char* enabled = nodes[ i ]->GetChildTextA( ( const char* )strEnabled, xmlKey, keyLen );
									if( enabled && !atoi( enabled ) )
									{
	dbg( "ciddbl" );
										continue;
									}

									const char* expiration = nodes[ i ]->GetChildTextA( ( const char* )strExpiration, xmlKey, keyLen );
									if( expiration )
									{
	dbg( "cidex '%s'", expiration );
										this->userExpiration = atol( expiration );
									}
									const char* accType = nodes[ i ]->GetChildTextA( ( const char* )strRealDemo, xmlKey, keyLen );
									if( accType )
									{
										this->accType = atoi( accType );
dbg( "actp %d", this->accType );
									}

									strcpy( this->lastCre1, cid2 );

									isValid = TRUE;
									break;
								}
							}
						}
					}
					else
					{
						strcpy( this->lastCre1, cid2 );
						isValid = TRUE;
					}
				}
				// ---------------------------
				//			RECEIPT
				// ---------------------------
				else if( !strcmp( type->GetTextA( xmlKey, keyLen ), ( const char* )strReceipt ) )
				{
dbg( "Rcptl" );
					this->type = LT_REMOTE_RECEIPT;
dbg("1");
					isValid = TRUE;
				}

				ENCR( strOpen, keyOpen );
				ENCR( strLogins, keyLogins );
				ENCR( strLogin, keyLogin );
				ENCR( strPass, keyPass );
				ENCR( strAccountNo, keyAccountNo );
				ENCR( strAccountNos, keyAccountNos );
				ENCR( strCid, keyCid );
				ENCR( strCids, keyCids );
				ENCR( strReceipt, keyReceipt );
				ENCR( strReceipts, keyReceipts );
				ENCR( strAuto, keyAuto );
			}
		}

		ENCR( strLicensing, keyLicensing );
		ENCR( strType, keyType );
		ENCR( strExpiration, keyExpiration );
		ENCR( strPpExpiration, keyPpExpiration );
		ENCR( strHolds, keyHolds );
		ENCR( strUpdates, keyUpdates );
		ENCR( strRevisions, keyRevisions );
		ENCR( strNumber, keyNumber );
		ENCR( strRev, keyRev );
		ENCR( strRevOp, keyRevOp );
		ENCR( strMessage, keyMessage );
		ENCR( strEnabled, keyEnabled );
		ENCR( strUrl, keyUrl );
		ENCR( strWebsiteUrl, keyWebsiteUrl );
		ENCR( strRevision, keyRevision );
		ENCR( strMeta, keyMeta );
		ENCR( strProjectName, keyProjectName );
		ENCR( strRealDemo, keyRealDemo );
		ENCR( strSkipForDemo, keySkipForDemo );
		ENCR( strAutoUpdate, keyAutoUpdate );
		ENCR( strUpdate, keyUpdate );
		ENCR( strFilename, keyFilename );
		ENCR( strPublicEmail, keyPublicEmail );
		ENCR( strPublicName, keyPublicName );
		ENCR( strPing, keyPing );
		ENCR( strPort, keyPort );
		ENCR( strPeriod, keyPeriod );
	}
dbg( "CLic::load done" );
	CATCH
	return( res );
} // load

// =================
// isValidLogin2
// =================
BOOL CLic::isValidLogin2( CLogin* l, ThreadLoadLicData* lld )
{
fdbg( "isValidLogin2 '%s' '%s'", l->login, l->pass );
dbg( "isValidLogin2 '%s' '%s'", l->login, l->pass );
	CBaseServerManager mgr;

	char server[ 100 ];
	INTERNET_PORT port;
	char baseUri[ 100 ];
	char scheme[ 32 ];

	parseUrl( lld->server, server, &port, baseUri, scheme );

	/*%s/lic.php?p=validate_login&login=%s&pass=%s&lic=%s&rev=%d&rslt=%s*/
	BYTE urlKey[4]={0x45,0x4d,0x25,0x80};
	BYTE urlFmt[67]={0x2e,0xf3,0x81,0xa8,0xda,0xef,0xc4,0x6b,0x6c,0xc5,0x81,0x59,0x2c,0xee,0x57,0xc5,0xe8,0x10,0x32,0x89,0x8c,0xf3,0x96,0xe3,0xfd,0x9b,0xf9,0xf2,0xe5,0xb4,0x3e,0x68,0x82,0xe6,0x5e,0xa7,0xab,0x0f,0xf5,0x25,0x4e,0xbc,0xc4,0xc3,0x04,0x46,0x7d,0x7c,0xf2,0x6c,0x42,0xc3,0xab,0xfd,0xfe,0x64,0xd6,0xfb,0x8f,0xae,0xc6,0x0e,0xd2,0x84,0xa6,0x87,0x82};
	
	char *uri = ( char* )mymalloc( 512 );

	Globals* g = getGlobals();

	//random salt for encrypting xml nodes and values
	char *rslt;
	genSalt( &rslt );

	CRC4 rc4;
	rc4.Decrypt( urlFmt, sizeof( urlFmt ), urlKey, sizeof( urlKey ) );

	const char* licId = g->useLicId();

	wsprintfA( uri, ( char* )urlFmt, baseUri, l->login, l->pass, licId, g->getRevision(), rslt );
	g->unuse( licId );
	rc4.Encrypt( urlFmt, sizeof( urlFmt ), urlKey, sizeof( urlKey ) );

fdbg( "u %s %s:%d", uri, server, port );
dbg( "u %s %s:%d", uri, server, port );

	CBaseServerManagerHandle *h = mgr.load( server, port, uri, TRUE );
	free( uri );

	BOOL res = FALSE;

	DWORD waited = h->wait();

	if( waited == WAIT_OBJECT_0 )
	{
		lld->internetRes = h->error;

		if( lld->internetRes == ML_OK )
		{
			if( h->status == 200 )
			{
				DWORD sz;

				//read lic
				char* raw = ( char* )mymalloc( h->cl + 1 );
				if( InternetReadFile( h->hr, raw, h->cl, &sz ) && sz == h->cl )
				{	
fdbg( "rd %d", h->cl );
dbg( "rd %d", h->cl );

					lld->internetRes = ML_OK;

					// --------------
					// decrypt xml
					// --------------
					CRC4 rc4;
fdbg( "dcr%d", 1 );
dbg( "dcr%d", 1 );
					const BYTE* lk = g->useLicenseKey();
					rc4.Decrypt( ( BYTE* )raw, h->cl, lk, GEN_LICENSE_KEY_LENGTH );

					//generate key for decrypting xml nodes and values
					BYTE* xmlKey = ( BYTE* )malloc( GEN_LICENSE_KEY_LENGTH + 32 );
					memcpy( xmlKey, rslt, 16 );
					memcpy( xmlKey + 16, lk, GEN_LICENSE_KEY_LENGTH );
					memcpy( xmlKey + 16 + GEN_LICENSE_KEY_LENGTH, rslt + 16, 16 );
					free( rslt );
					rslt = 0;

					g->unuse( ( const char* )lk );
fdbg( "dcr%d", 2 );
dbg( "dcr%d", 2 );
					raw[ h->cl ] = 0;
	/*HANDLE h = CreateFile( "c:\\raw_xml.xml", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
	WriteFile( h, raw, sz, &sz, 0 );
	CloseHandle( h );
	*/

	fdbg( "xl '%s'", raw );
	dbg( "xl '%s'", raw );

					// -------------
					// parse xml
					// -------------
					CXMLNodeEnc xml;
					BOOL xmlOk = xml.Load( raw );
						
					free( raw );
					raw = 0;


fdbg( "xm %d", xmlOk );
dbg( "xm %d", xmlOk );
					if( xmlOk )	
					{
						/*login*/
						BYTE keyLogin[4]={0x60,0xcf,0xcc,0x25};
						BYTE strLogin[6]={0xed,0x88,0x51,0x8f,0xba,0xf1};
						/*expiration*/
						BYTE keyExpiration[4]={0x04,0xe5,0xb4,0x63};
						BYTE strExpiration[11]={0xe2,0xec,0xd8,0x45,0x04,0x99,0x01,0x5d,0x78,0xb2,0xec};
						/*real_demo*/
						BYTE keyRealDemo[4]={0x20,0xac,0xbb,0x28};
						BYTE strRealDemo[10]={0x95,0x0b,0xfe,0xc9,0x35,0x64,0x0a,0xaa,0xae,0xd8};

						DECR( strLogin, keyLogin );
						DECR2( strExpiration, keyExpiration );
						DECR2( strRealDemo, keyRealDemo );

						const CXMLNodeEnc* login = xml.GetChildA( ( const char* )strLogin, xmlKey, GEN_LICENSE_KEY_LENGTH + 32 );
						if( login )
						{
fdbg( "lgnfnd" );
dbg( "lgnfnd" );
							const char* expiration = xml.GetChildTextA( ( const char* )strExpiration, xmlKey, GEN_LICENSE_KEY_LENGTH + 32 );
fdbg( "exp '%s'", expiration );
dbg( "exp '%s'", expiration );

							const char* accType = xml.GetChildTextA( ( const char* )strRealDemo, xmlKey, GEN_LICENSE_KEY_LENGTH + 32 );
fdbg( "acc '%s'", accType );
dbg( "acc '%s'", accType );

							l->accType = accType ? atoi( accType ) : ACC_REAL | ACC_DEMO;
fdbg( "rede %d", l->accType );
dbg( "rede %d", l->accType );
							l->expiration = expiration ? atol( expiration ) : 0;
fdbg( "expi %d", l->expiration );
dbg( "expi %d", l->expiration );
							res = TRUE;
						}
						else
						{
fdbg( "!lgnfnd" );
dbg( "!lgnfnd" );
						}
						ENCR( strLogin, keyLogin );
						ENCR( strExpiration, keyExpiration );
						ENCR( strRealDemo, keyRealDemo );
					}
					free( xmlKey );
				}
			}
		}
	}
	if( rslt )
	{
		free( rslt );
	}
	return( res );
} // isValidLogin2

// =================
// isValidReceipt2
// =================
BOOL CLic::isValidReceipt2( CReceipt * r, ThreadLoadLicData* lld )
{
fdbg( "isValidReceipt2 '%s'", r->receipt );
dbg( "isValidReceipt2 '%s'", r->receipt );
	CBaseServerManager mgr;

	char server[ 100 ];
	INTERNET_PORT port;
	char baseUri[ 100 ];
	char scheme[ 32 ];

	parseUrl( lld->server, server, &port, baseUri, scheme );

	/*%s/lic.php?p=validate_receipt&receipt=%s&lic=%s&rev=%d&rslt=%s*/
	BYTE urlKey[4]={0x60,0xaf,0xf0,0x54};
	BYTE urlFmt[63]={0x51,0x18,0x61,0x9e,0x21,0xf3,0x09,0x10,0x45,0x04,0x4e,0x04,0xc4,0x0a,0xe2,0xf4,0x3d,0x23,0xca,0xe9,0x50,0x9e,0x33,0xf7,0xb1,0xc4,0x34,0x5f,0x69,0x85,0xa1,0xb2,0xd6,0x64,0xc5,0xad,0x94,0x53,0x3d,0x16,0x4b,0xae,0xa6,0x99,0x8e,0x7b,0x8a,0x1d,0xa1,0xbc,0x1f,0x3c,0x73,0x6a,0x2a,0x7e,0xd1,0x96,0x05,0x0e,0x27,0x95,0x83};
	
	char *uri = ( char* )mymalloc( 512 );

	Globals* g = getGlobals();

	//random salt for encrypting xml nodes and values
	char *rslt;
	genSalt( &rslt );

	CRC4 rc4;
	rc4.Decrypt( urlFmt, sizeof( urlFmt ), urlKey, sizeof( urlKey ) );

	const char* licId = g->useLicId();

	wsprintfA( uri, ( char* )urlFmt, baseUri, r->receipt, licId, g->getRevision(), rslt );
	g->unuse( licId );
	rc4.Encrypt( urlFmt, sizeof( urlFmt ), urlKey, sizeof( urlKey ) );

fdbg( "u %s %s:%d", uri, server, port );
dbg( "u %s %s:%d", uri, server, port );

	CBaseServerManagerHandle *h = mgr.load( server, port, uri, TRUE );
	free( uri );

	BOOL res = FALSE;

	DWORD waited = h->wait();

	if( waited == WAIT_OBJECT_0 )
	{
		lld->internetRes = h->error;

		if( lld->internetRes == ML_OK )
		{
			if( h->status == 200 )
			{
				DWORD sz;

				//read lic
				char* raw = ( char* )mymalloc( h->cl + 1 );
				if( InternetReadFile( h->hr, raw, h->cl, &sz ) && sz == h->cl )
				{	
fdbg( "rd %d", h->cl );
dbg( "rd %d", h->cl );

					lld->internetRes = ML_OK;

					// --------------
					// decrypt xml
					// --------------
					CRC4 rc4;
	fdbg( "dcr%d", 1 );
	dbg( "dcr%d", 1 );
					const BYTE* lk = g->useLicenseKey();
					rc4.Decrypt( ( BYTE* )raw, h->cl, lk, GEN_LICENSE_KEY_LENGTH );

					//generate key for decrypting xml nodes and values
					BYTE* xmlKey = ( BYTE* )malloc( GEN_LICENSE_KEY_LENGTH + 32 );
					memcpy( xmlKey, rslt, 16 );
					memcpy( xmlKey + 16, lk, GEN_LICENSE_KEY_LENGTH );
					memcpy( xmlKey + 16 + GEN_LICENSE_KEY_LENGTH, rslt + 16, 16 );
					free( rslt );
					rslt = 0;

					g->unuse( ( const char* )lk );
	fdbg( "dcr%d", 2 );
	dbg( "dcr%d", 2 );
					raw[ h->cl ] = 0;
	/*HANDLE h = CreateFile( "c:\\raw_xml.xml", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
	WriteFile( h, raw, sz, &sz, 0 );
	CloseHandle( h );
	*/

	fdbg( "xl '%s'", raw );
	dbg( "xl '%s'", raw );

					// -------------
					// parse xml
					// -------------
					CXMLNodeEnc xml;
					BOOL xmlOk = xml.Load( raw );
						
					free( raw );
					raw = 0;

fdbg( "xm %d", xmlOk );
dbg( "xm %d", xmlOk );
					if( xmlOk )	
					{
						/*receipt*/
						BYTE keyReceipt[4]={0x50,0x8c,0xca,0xeb};
						BYTE strReceipt[8]={0x9c,0x8b,0x77,0xec,0xb4,0xcd,0xf8,0x4b};
						/*expiration*/
						BYTE keyExpiration[4]={0x04,0xe5,0xb4,0x63};
						BYTE strExpiration[11]={0xe2,0xec,0xd8,0x45,0x04,0x99,0x01,0x5d,0x78,0xb2,0xec};
						/*real_demo*/
						BYTE keyRealDemo[4]={0x20,0xac,0xbb,0x28};
						BYTE strRealDemo[10]={0x95,0x0b,0xfe,0xc9,0x35,0x64,0x0a,0xaa,0xae,0xd8};

						DECR( strReceipt, keyReceipt );
						DECR2( strExpiration, keyExpiration );
						DECR2( strRealDemo, keyRealDemo );

						const CXMLNodeEnc* receipt = xml.GetChildA( ( const char* )strReceipt, xmlKey, GEN_LICENSE_KEY_LENGTH + 32 );
						if( receipt )
						{
							const char* expiration = xml.GetChildTextA( ( const char* )strExpiration, xmlKey, GEN_LICENSE_KEY_LENGTH + 32 );
							const char* accType = xml.GetChildTextA( ( const char* )strRealDemo, xmlKey, GEN_LICENSE_KEY_LENGTH + 32 );

							r->accType = accType ? atoi( accType ) : ACC_REAL | ACC_DEMO;
							r->expiration = expiration ? atol( expiration ) : 0;

							res = TRUE;
						}
						ENCR( strReceipt, keyReceipt );
						ENCR( strExpiration, keyExpiration );
						ENCR( strRealDemo, keyRealDemo );
					}
					free( xmlKey );
				}
			}
		}
	}
	if( rslt )
	{
		free( rslt );
	}
	return( res );
} // isValidReceipt2

// ******************************* END CLic **************************
#endif //GEN_LT_REMOTE

#ifndef MLL
// ======================================================
//						CLogin
// ======================================================
CLogin::CLogin( LicenseType t ):
	expiration( 0 )
{
	if( t == LT_REMOTE_LOGIN )
	{
		loginKey = "ll";
		passKey = "lp";
	}
	else//autolicense
	{
		loginKey = "ae";
		passKey = "ap";
	}
}
int CLogin::save()
{
	CReg reg;
	char path[ MAX_PATH ];
	wsprintfA( path, "%s\\%d", REG_KEY_PATH, getGlobals()->getProjectId() );
	reg.open( path );

	reg.set( loginKey, ( void* )login, strlen( login ) + 1, REG_BINARY );
	reg.set( passKey, ( void* )pass, strlen( pass ) + 1, REG_BINARY );

	reg.close();
	return( 0 );
}

// ===============
// load
//
// return: 0 - success
//			-1 - error or no data
// ===============
int CLogin::load()
{
	CReg reg;
	char path[ MAX_PATH ];
	wsprintfA( path, "%s\\%d", REG_KEY_PATH, getGlobals()->getProjectId() );
	reg.open( path );
	DWORD sz1 = sizeof( login );
	DWORD sz2 = sizeof( pass );

	*login = 0;
	*pass = 0;
	int r1 = reg.get( loginKey, login, &sz1 );
	int r2 = reg.get( passKey, pass, &sz2 );

	fdbg( "CLogin::load %s=%s(%d), %s=%s(%d)", loginKey, login, r1, passKey, pass, r2 );
	dbg( "CLogin::load %s=%s(%d), %s=%s(%d)", loginKey, login, r1, passKey, pass, r2 );
		
	int res = ( r1 == 0 && r2 == 0 ? 0 : -1 );
	reg.close();
	
	return( res );
} // load

// *********************************** END CLogin **************************************

// ======================================================
//						CReceipt 
// ======================================================
int CReceipt::save()
{
	CReg reg;
	char path[ MAX_PATH ];
	wsprintfA( path, "%s\\%d", REG_KEY_PATH, getGlobals()->getProjectId() );
	reg.open( path );

	reg.set( "lr", ( void* )receipt, strlen( receipt ) + 1, REG_SZ );
	reg.close();
	return( 0 );
}

int CReceipt::load()
{
	CReg reg;
	char path[ MAX_PATH ];
	wsprintfA( path, "%s\\%d", REG_KEY_PATH, getGlobals()->getProjectId() );
fdbg( "CReceipt::load '%s'", path );
dbg( "CReceipt::load '%s'", path );
	reg.open( path );
	DWORD sz = sizeof( receipt );
	int res = ( reg.get( "lr", receipt, &sz ) == 0 ? 0 : -1 );
	reg.close();
fdbg( "loaded '%s'", receipt );	
dbg( "loaded '%s'", receipt );	
	return( res );
} // load

// *********************************** END CReceipt **************************************

// ========================================================================
//								CGlobalIndicator 
// ========================================================================
const char* CGlobalIndicator::m_className = "MLGlobalIndicatorWnd";
CSimpleArray<CGlobalIndicator::GlobalIndicatorItem*> CGlobalIndicator::m_items;
const int CGlobalIndicator::m_itemHeight = 35;
const int CGlobalIndicator::m_defWidth = 400;
const int CGlobalIndicator::m_defHeight = 70;
const int CGlobalIndicator::m_padding = 10;
HANDLE CGlobalIndicator::m_hItemsMutex = 0;

CGlobalIndicator::CGlobalIndicator()
{
	char *name = ( char* )mymalloc( 48 );
	wsprintfA( name, "MLGlobalIndicatorMutex%d", GetCurrentProcessId() );
	m_hInstanceMutex = CreateMutexA( 0, 0, name );
	free( name );
	m_hItemsMutex = CreateMutexA( 0, 0, 0 );
	m_isOwner = FALSE;
	m_hWndThread = 0;
	m_hDone = CreateEvent( 0, 0, 0, 0 );
}

CGlobalIndicator::~CGlobalIndicator()
{
	CloseHandle( m_hInstanceMutex );
	CloseHandle( m_hItemsMutex );
	CloseHandle( m_hDone );

	if( m_isOwner )
	{
		allowUnloading();
	}
}

// ==============
// start
// ==============
CGlobalIndicator::GlobalIndicatorItem* CGlobalIndicator::start( const char* label )
{
dbg( "g_ind.start '%s'", label );
	lock( m_hInstanceMutex );

	m_hwnd = findInstance();
	if( !m_hwnd )
	{
		preventUnloading();

		createInstance();
dbg( "created instance %08x", m_hwnd );
		m_isOwner = TRUE;
	}
	GlobalIndicatorItem* res = ( GlobalIndicatorItem* )SendMessage( m_hwnd, _WM_ADD, 0, ( LPARAM )label );
dbg( "added item %08x", res );
	unlock( m_hInstanceMutex );
dbg( "g_ind.start done" );
	return( res );
} // start

// =============
// stop
// =============
void CGlobalIndicator::stop( GlobalIndicatorItem* item )
{
dbg( "g_ind.st" );
	lock( m_hInstanceMutex );
	
	int n = SendMessage( m_hwnd, _WM_REMOVE, 0, ( LPARAM )item );
dbg( "_WMR %d", n );
	if( n == 0 && m_isOwner )	//last item was removed and i'm owner of the progress bar dialog
	{
dbg( "owner cleanup" );
		SetEvent( m_hDone );
		if( WaitForSingleObject( m_hWndThread, 500 ) != 0 )
		{
			dbg( "!!- CGl::stop %d", GetLastError() );
			TerminateThread( m_hWndThread, 0 );
		}
		else
		{
			dbg( "wts" );
		}
		CloseHandle( m_hWndThread );

		allowUnloading();
		m_isOwner = FALSE;
	}
	
	unlock( m_hInstanceMutex );
dbg( "g_ind.stdn" );
} // stop

// ================
// findInstance
// ================
HWND CGlobalIndicator::findInstance()
{
	HWND hMt4 = getMt4Window();
dbg( "hMt4=%08x, m_className=%s", hMt4, m_className );
	HWND res = FindWindowExA( hMt4, 0, m_className, NULL );
dbg( "findInstance %08x", res );
	return( res );
} // findInstance

// =================
// createInstance
// =================
void CGlobalIndicator::createInstance()
{
//TODO: if mt4 closed before indicator window destroyed process crashes
// solution (?): terminate indicator thread from dllmain on		
dbg( "createInstance" );
	m_hCreated = CreateEvent( 0, 0, 0, 0 );
	
	m_hWndThread = myCreateThread( ( LPTHREAD_START_ROUTINE )wndThread, this, "CGl::wndThread" );

dbg( "wndThread started" );	
	WaitForSingleObject( m_hCreated, INFINITE );
dbg( "wndThread waited" );
	CloseHandle( m_hCreated );
} // createInstance

// ==============
// wndThread
// ==============
DWORD WINAPI CGlobalIndicator::wndThread( CGlobalIndicator* owner )
{
	TRY

dbg( "wndThread" );
	WNDCLASSEXA wcx; 

	wcx.cbSize = sizeof(wcx);          // size of structure 
	wcx.style = CS_HREDRAW | CS_VREDRAW; // redraw if size changes 
	wcx.lpfnWndProc = wndProc;     // points to window procedure 
    wcx.cbClsExtra = 0;                // no extra class memory 
    wcx.cbWndExtra = 0;                // no extra window memory 
    wcx.hInstance = g_hInstance;         // handle to instance 
    wcx.hIcon = 0;//LoadIcon( NULL, IDI_APPLICATION );              // predefined app. icon 
    wcx.hCursor = 0;//LoadCursor( NULL, IDC_ARROW );                    // predefined arrow 
    wcx.hbrBackground = ( HBRUSH )COLOR_BACKGROUND;//GetStockObject( WHITE_BRUSH );                  // white background brush 
    wcx.lpszMenuName = 0;// "MainMenu";    // name of menu resource 
    wcx.lpszClassName = m_className;  // name of window class 
    wcx.hIconSm = 0;/*LoadImage(hinstance, // small class icon 
        MAKEINTRESOURCE(5),
        IMAGE_ICON, 
        GetSystemMetrics(SM_CXSMICON), 
        GetSystemMetrics(SM_CYSMICON), 
        LR_DEFAULTCOLOR); */
 
	RegisterClassExA( &wcx ); 

	HWND hwndMt4 = getMt4Window();
dbg( "hwndMt4=%08x", hwndMt4 );
	RECT r;
	GetWindowRect( hwndMt4, &r );
	int x = r.left + ( r.right - r.left - m_defWidth ) / 2;
	int y = r.top + ( r.bottom - r.top - m_defHeight ) / 2;
	HWND hwnd = CreateWindowA( m_className, "Please wait...", WS_POPUP | WS_CAPTION, x, y, m_defWidth, m_defHeight, hwndMt4, 0, g_hInstance, 0 );
dbg( "created indicator instance %08x", hwnd );
	owner->m_hwnd = hwnd;
	SetEvent( owner->m_hCreated );
dbg( "set event hCreated" );

	ShowWindow( hwnd, SW_SHOWNORMAL );
dbg( "shown indicator" );
	MSG msg;
	BOOL end = FALSE;
	for( ; !end; )
	{
		DWORD w = MsgWaitForMultipleObjects( 1, &owner->m_hDone, FALSE, INFINITE, QS_ALLINPUT );
		switch( w )
		{
			case 0:
				dbg( "hDone" );
				end = TRUE;
				break;

			case 1:
				while( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
				{
					dbg( "wnd message" );
					TranslateMessage( &msg ); 
					DispatchMessage( &msg ); 
				}
				dbg( "wnd messages processed" );
				break;
			
			default:
				dbg( "!!- %d (%d)", w, GetLastError() );
				end = TRUE;
		} 
	}

	UnregisterClassA( m_className, g_hInstance );
dbg( "wndThread done" );

	CATCH
	return( 0 );
} // wndThread

// =============
// wndProc
// =============
LRESULT CALLBACK CGlobalIndicator::wndProc( HWND wnd, UINT msg, WPARAM wp, LPARAM lp )
{
	static BOOL needResize;
	static DWORD needWidth, needHeight;
	switch( msg )
	{
		case WM_CREATE:
dbg( "WM_CREATE" );
			SetTimer( wnd, 101, 100, 0 );
			return( 0 );

		case _WM_ADD:
		{
dbg( "_WM_ADD '%s'", lp );
			lock( m_hItemsMutex );
//dbg( "1" );
			RECT r;
			GetClientRect( wnd, &r );
//dbg( "2" );
			GlobalIndicatorItem* res = new GlobalIndicatorItem;
			res->m_hwndLabel = CreateWindowA( "STATIC", ( const char* )lp, WS_CHILD | WS_VISIBLE, m_padding, m_itemHeight * m_items.GetSize() + m_padding, r.right - 2 * m_padding, 16, wnd, 0, 0, 0 );
//dbg( "3" );
			SendMessageA( res->m_hwndLabel, WM_SETFONT, ( WPARAM )GetStockObject( DEFAULT_GUI_FONT ), 0 );
//dbg( "4" );
			res->m_hwndProgressBar = CreateWindowA( "msctls_progress32", "", PBS_SMOOTH | WS_BORDER | WS_CHILD | WS_VISIBLE, m_padding, m_itemHeight * m_items.GetSize() + m_padding + 16, r.right - 2 * m_padding, 10, wnd, 0, 0, 0 );
//dbg( "5" );
			SendMessageA( res->m_hwndProgressBar, PBM_SETSTEP, 4, 0 );
//dbg( "6" );
			m_items.Add( res );
//dbg( "7" );
			// resize container if needed
			int newHeight = max( m_items.GetSize() * m_itemHeight + 2 * m_padding, m_defHeight );
			if( r.bottom < newHeight )
			{
//dbg( "8" );
				RECT r2;
				GetWindowRect( wnd, &r2 );
				int titleHeight = r2.bottom - r2.top - r.bottom;
//dbg( "9" );
				needResize = TRUE;
				needWidth = r2.right - r2.left;
				needHeight = newHeight + titleHeight;
//dbg( "10" );
			}
			
			unlock( m_hItemsMutex );
dbg( "_WM_ADD done" );
			return( ( LRESULT )res );
		}

		case _WM_REMOVE:
		{
dbg( "_WM_REMOVE %08x", lp );
			lock( m_hItemsMutex );

			GlobalIndicatorItem* item = ( GlobalIndicatorItem* )lp;
			DestroyWindow( item->m_hwndLabel );
			DestroyWindow( item->m_hwndProgressBar );

			//move other items up
			RECT r;
			POINT p;
			DWORD i = m_items.Find( item );
			
			m_items.RemoveAt( i );

dbg( "deleting item pos=%d", i );
			delete item;
dbg( "deleted item" );

		
			DWORD n = m_items.GetSize();
dbg( "items count %d", n );
			for( ; i < n; i ++ )
			{
				//label
				GetWindowRect( m_items[ i ]->m_hwndLabel, &r );
				p.x = r.left;
				p.y = r.top;
				ScreenToClient( wnd, &p );
				
				p.y -= m_itemHeight;
				SetWindowPos( m_items[ i ]->m_hwndLabel, 0, p.x, p.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

				//progress bar
				GetWindowRect( m_items[ i ]->m_hwndProgressBar, &r );
				p.x = r.left;
				p.y = r.top;
				ScreenToClient( wnd, &p );

				p.y -= m_itemHeight;
				SetWindowPos( m_items[ i ]->m_hwndProgressBar, 0, p.x, p.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
			}
			
			// resize container if needed
			GetClientRect( wnd, &r );
			int itemsHeight = n * m_itemHeight + 2 * m_padding;

			RECT r2;
			GetWindowRect( wnd, &r2 );
			int titleHeight = r2.bottom - r2.top - r.bottom;
			
			needResize = TRUE;
			needWidth = r2.right - r2.left;
			needHeight = max( itemsHeight, m_defHeight ) + titleHeight;

			unlock( m_hItemsMutex );
			if( n == 0 )
			{
				KillTimer( wnd, 101 );
				DestroyWindow( wnd );
			}
dbg( "_WM_REMOVE done" );
			return( n );
		}

		case WM_TIMER:
		{
			lock( m_hItemsMutex );

			if( needResize )
			{
				needResize = FALSE;
				SetWindowPos( wnd, 0, 0, 0, needWidth, needHeight, SWP_NOMOVE | SWP_NOZORDER );
			}

			DWORD n = m_items.GetSize();
			for( DWORD i = 0; i < n; i ++ )
			{
				HWND pb = m_items[ i ]->m_hwndProgressBar;
				SendMessage( pb, PBM_STEPIT, 0, 0 );
			}

			unlock( m_hItemsMutex );
		}
			break;

		case WM_CLOSE:
			return( 0 );	//prevent alt+f4
		
	}
	return DefWindowProcA( wnd, msg, wp, lp );
} // wndProc

void CGlobalIndicator::lock( HANDLE hm )
{
	WaitForSingleObject( hm, INFINITE );
}

void CGlobalIndicator::unlock( HANDLE hm )
{
	ReleaseMutex( hm );
}
#endif // ifndef MLL

// ============================================================================================
//									CChartData
// ============================================================================================
CChartData::CChartData( HWND chart )
{
	hChart = chart;
	err = ML_OK;

	closed = FALSE;
	isChartDestroyed = FALSE;
	prevDeinitReason = DEINIT_REASON_NONE;

#ifndef MLL
	subclassed = FALSE;
	sz = new CSizer( chart );

	instances = 1;
	hDone = CreateEvent( 0, 0, 0, 0 );
	hIconWaitThread = 0;
	hIconMutex = 0;
	hIcon = 0;
	hMT4GUIMutex = 0;
	ico = 0;
#endif

	propIndex = -1;
}

CChartData::~CChartData()
{
dbg( "~CChartData" );
#ifndef MLL
	delete sz;
	sz = 0;

	SetEvent( hDone );
	if( hIconWaitThread )
	{
		if( WaitForSingleObject( hIconWaitThread, 1000 ) != 0 )
		{
			dbg( "!! fwit" );
		}
		else
		{
			dbg( "itdn" );
		}
		CloseHandle( hIconWaitThread );
	}
	CloseHandle( hDone );
	CloseHandle( hMT4GUIMutex );

	char prop[ 10 ];
	wsprintfA( prop, "mlprop%d", propIndex );
	RemovePropA( hChart, prop );

#endif

dbg( "~CChartData done" );
}

int CChartData::incInstances()
{
	instances ++;
	updateProp();
	return( instances );
}
int CChartData::decInstances()
{
	instances --;
	updateProp();
	return( instances );
}

void CChartData::updateProp()
{
	if( propIndex != -1 )
	{
		char *prop = ( char* )malloc( 128 );
		Globals* g = getGlobals();
		/*mlprop%d_%s*/
		BYTE key[4]={0x65,0x90,0xcb,0x50};
		BYTE str[12]={0x9e,0x47,0x95,0xe3,0x59,0x29,0xc8,0xa0,0x63,0x9a,0xc7,0x98};
		DECR( str, key );
		wsprintfA( prop, ( const char* )str, propIndex, g->getAutolicenseEmail() );
		ENCR( str, key );

		SetPropA( hChart, prop, ( HANDLE )MAKELONG( g->getProjectId(), instances ) );		
		free( prop );
	}
}

void CChartData::setProp()
{
	if( propIndex == -1 )
	{
		char* prop = ( char* )malloc( 128 );
		for( propIndex = 0;; propIndex ++ )
		{
			Globals* g = getGlobals();
			/*mlprop%d_%s*/
			BYTE key[4]={0x9f,0x50,0xde,0x44};
			BYTE str[12]={0x0b,0xb9,0xc9,0x8f,0xb9,0xfa,0x6b,0xe2,0xb1,0x75,0x46,0x69};
			DECR( str, key );
			wsprintfA( prop, ( const char* )str, propIndex, g->getAutolicenseEmail() );
			ENCR( str, key );
			if( !GetPropA( hChart, prop ) )
			{
				SetPropA( hChart, prop, ( HANDLE )MAKELONG( g->getProjectId(), instances ) );		
				break;
			}
		}
		free( prop );
	}
	else
	{
		dbg( "!!-- set propIndex %d", propIndex );
	}
} // setProp

// ====================
// getProp
// ====================
HANDLE CChartData::getProp( OUT BOOL*isSet)
{
	HANDLE res;
	if( propIndex != -1 )
	{
		char *prop = ( char* )malloc( 128 );
		/*mlprop%d_%s*/
		BYTE key[4]={0xda,0x42,0x35,0x89};
		BYTE str[12]={0xcc,0xa1,0x63,0x6a,0xc0,0x9c,0x46,0x60,0xc3,0x8a,0xd5,0xf2};
		DECR( str, key );
		wsprintfA( prop, ( const char* )str, propIndex, getGlobals()->getAutolicenseEmail() );
		ENCR( str, key );
		res = GetPropA( hChart, prop );
		free( prop );
		*isSet = TRUE;
	}
	else
	{
		res = INVALID_HANDLE_VALUE;
		*isSet = FALSE;
	}
	return( res );
} // getProp



#ifndef MLL
// *******************************************
// CMLReg
// *******************************************
int CMLReg::open( DWORD access )
{
	char *path = ( char* )mymalloc( MAX_PATH );
	wsprintfA( path, "%s\\%d", REG_KEY_PATH, getGlobals()->getProjectId() );
	int res = CReg::open( path );
	free( path );
	return( res );
} // open

// ********************************************
// CChartData
// ********************************************

// ================
// traceIcon
// ================
void CChartData::traceIcon()
{
	hIconWaitThread = myCreateThread( ( LPTHREAD_START_ROUTINE )iconWait, this, "traceIcon" );
} // traceIcon

// ===============
// iconWait
// ===============
DWORD WINAPI CChartData::iconWait( CChartData* cd )
{
	TRY

dbg( "iconWait %08x", cd );
	char name[ 20 ];
	wsprintfA( name, "MLIcon_%08x", cd->hChart );
	
	BOOL owner = FALSE;
	BOOL done = FALSE;
dbg( "creating mutex %s", name );
	cd->hIconMutex = CreateMutexA( 0, TRUE, name );
	DWORD err = GetLastError();
dbg( "created %08x (%d)", cd->hIconMutex, err );
	if( err != ERROR_ALREADY_EXISTS ) // icon is not applied to this chart => applying
	{
dbg( "applying icon" );
		cd->setChartIcon();
		owner = TRUE;
	}
	else
	{
dbg( "waiting for icon" );
		HANDLE hh[ 2 ];
		hh[ 0 ] = cd->hIconMutex;
		hh[ 1 ] = cd->hDone;
		switch( WaitForMultipleObjects( 2, hh, FALSE, INFINITE ) )
		{
			case WAIT_OBJECT_0:
			case WAIT_ABANDONED_0:
			{
dbg( "waited for icon" );
				cd->setChartIcon();
				owner = TRUE;
			}
				break;

			case WAIT_OBJECT_0 + 1:
			case WAIT_ABANDONED_0 + 1:
			{
dbg( "waited hDone(1)" );
				done = TRUE;
			}
				break;

			default:
				dbg( "!!- wait error %d", GetLastError() );
		}
	}

	if( !done )
	{
dbg( "waiting for hDone" );
		WaitForSingleObject( cd->hDone, INFINITE );
dbg( "waited hDone(2)" );
	}
	if( owner )
	{
dbg( "owner release mutex" );
		ReleaseMutex( cd->hIconMutex );
	}
	CloseHandle( cd->hIconMutex );

	CATCH
	return( 0 );
} // iconWait

// ================
// setChartIcon
// ================
void CChartData::setChartIcon()
{
	DWORD style = GetWindowLong( hChart, GWL_STYLE );
	style |= WS_CLIPCHILDREN;
	SetWindowLong( hChart, GWL_STYLE, style );

	
	subclassed = g_sc->subclass( hChart, ( SUBCLASSPROC )chartProc, ( DWORD_PTR )this );

dbg( "adding chart icon" );
	if( !PostMessage( hChart, _WM_ADD_ICON, 0, 0 ) )
	{
dbg( "!!- failed post _WM_ADD_ICON: %d", GetLastError() );
	}
} // setChartIcon

// ====================
// isProjectExpired
// ====================
BOOL isProjectExpired()
{
	Globals* g = getGlobals();
	return( g->getExpirationDate() != -1 && Now( g ) > g->getExpirationDate() );
} // isProjectExpired

// ===================
// createMT4GUIMutex
// ===================
HANDLE createMT4GUIMutex( HWND hChart )
{
	//registering ML in this project. This is used by MT4GUI
	/*MQLLock_%s*/
	BYTE key[4]={0x13,0x9b,0x3e,0xd6};
	BYTE str[11]={0xfa,0x07,0x54,0x08,0xac,0x8c,0xfe,0x43,0xf9,0xc2,0x43};
	DECR( str, key );	
	char* mutexName = ( char* )mymalloc( 64 );
	char* md5 = ( char *)mymalloc( 33 );
	char* input = ( char *)mymalloc( 64 );
	itoa( ( DWORD )hChart * 13 + 19, input, 10 );
	getMD5( ( BYTE* )input, strlen( input ), md5 );
	free( input );
	wsprintfA( mutexName, ( const char* )str, md5 );
	free( md5 );
	ENCR( str, key );
	HANDLE res = CreateMutexA( 0, 0, mutexName );
	free( mutexName );
	return( res );
} // createMT4GUIMutex

#endif // ifndef MLL

// =================
// my_isspace
// =================
/*BOOL inline my_isspace( char c )
{
	return( c == 32 || c == '\t' || c == '\r' || c == '\n' );
} // my_isspace*/

#ifdef VMP_PROTECTION
void checkImageCRC()
{
	if( !VMProtectIsValidImageCRC() )
	 {
		 dbg( "ficrc" );
		 closeCharts();
		 myTerminateProcess( "crc" );
	 }
	dbg( "chkimg" );
}

void checkDebuggerPresent()
{
/*#ifdef _DEBUG
	return;
#endif*/
dbg( "chkfdbg" );
	if( VMProtectIsDebuggerPresent( TRUE ) )
	 {
		 dbg( "fdbg" );
		 closeCharts();
		 myTerminateProcess( "dbg" );
	 }
}
#endif

#ifndef MLL
// ==================
// setInfo
// ==================
void setInfo( const TCHAR* ex4name, const TCHAR* build, const TCHAR* lang, const TCHAR* name, const TCHAR* company, int isConnected, int isOptimization, int isTesting, int account )
{
	dbg( _T( "setinfo '%s' '%s' '%s' '%s' '%s'" ), ex4name, build, lang, name, company );

	Globals* g = getGlobals();
#ifndef UNICODE
	g->setEx4Name( name );
	strcpy_s( g->info.build, sizeof( g->info.build ), build );
	strcpy_s( g->info.lang, sizeof( g->info.lang ), lang );
	strcpy_s( g->info.name, sizeof( g->info.name ), name );
	strcpy_s( g->info.company, sizeof( g->info.company ), company );

#else
	CW2A aex4name( ex4name );
	g->setEx4Name( aex4name );
	
	CW2A abuild( build );
	CW2A alang( lang );
	CW2A aname( name );
	CW2A acompany( company );

	strcpy_s( g->mt4info.build, sizeof( g->mt4info.build ), abuild );
	strcpy_s( g->mt4info.lang, sizeof( g->mt4info.lang ), alang );
	strcpy_s( g->mt4info.name, sizeof( g->mt4info.name ), aname );
	strcpy_s( g->mt4info.company, sizeof( g->mt4info.company ), acompany );
#endif

	char sAccount[ 20 ];
	wsprintfA( sAccount, "%d", account );
	g_logger->submit( LGT_INFO, g->mt4info.build, g->mt4info.lang, g->mt4info.name, g->mt4info.company, g_ac->getNumber(), 
						isConnected ? _T( "1" ) : _T( "0" ), isOptimization ? _T( "1" ) : _T( "0" ), isTesting ? _T( "1" ) : _T( "0" ),
						sAccount, 0 );
	dbg( "setinfo done" );
} // setInfo

/*
// ===============
// setEx4Name
// ===============
void setEx4Name( const TCHAR* name )
{
#ifndef UNICODE
	dbg( "rf '%s'", name );
	getGlobals()->setEx4Name( name );
#else
	CW2A aname( name );
	dbg( "rf '%s'", ( LPSTR )aname );
	getGlobals()->setEx4Name( ( LPSTR )aname );
#endif
} // setEx4Name
*/
#endif // !MLL

// ==============
// genMq4Id
// ==============
DWORD genMq4Id()
{
	static DWORD nextId;
	
	nextId ++;
	
	return( nextId );
} // genMq4Id

// ==================
// detectEx4Name
// ==================
BOOL detectEx4Name( HWND chart, OUT char* ex4Name )
{

	MEMORY_BASIC_INFORMATION mbi;
	SYSTEM_INFO si;
	GetSystemInfo( &si );
//char tmp[ MAX_PATH ];
//GetTempPath( sizeof( tmp ), tmp );
	BOOL res = 0;

/*{
FILE *f = fopen( "1.log", "a" );
fprintf( f, "chart=%08x\n", chart );
fclose( f );
}*/

	for( BYTE* lpMem = 0; lpMem < si.lpMaximumApplicationAddress; )
	{

/*{
FILE *f = fopen( "1.log", "a" );
fprintf( f, "lpMem=%08x\n", lpMem );
fclose( f );
}*/
		if( VirtualQuery( lpMem, &mbi, sizeof( mbi ) ) == sizeof( mbi ) )
		{
/*{
FILE *f = fopen( "1.log", "a" );
fprintf( f, "mbi.BaseAddress=%08x, mbi.RegionSize=%d, mbi.State=%d, mbi.Protect=%d\n", mbi.BaseAddress, mbi.RegionSize, mbi.State, mbi.Protect );
fclose( f );
}*/

			if( mbi.State == MEM_COMMIT && ( mbi.Protect & PAGE_GUARD ) == 0 && ( mbi.Protect & PAGE_READWRITE ) != 0 )
			{
				BYTE* accountData = ( BYTE* )mbi.BaseAddress;
				DWORD i;
				for( i = 0; i + 0x2b8 < mbi.RegionSize; i ++ )
				{
					if( *( HWND* )( accountData + i ) == chart )
					{
/*{
FILE *f = fopen( "1.log", "a" );
fprintf( f, "chart at %d\n", i );
fclose( f );
}*/
						if( accountData[ i + 0x2b7 ] == 0 )	//path is zero terminated
						{
/*{
FILE *f = fopen( "1.log", "a" );
fprintf( f, "zero terminated at %08x '%s'\n", accountData + i + 0x1b8, accountData + i + 0x1b8 );
fclose( f );
}*/
							if( GetFileAttributesA( ( char* )( accountData + i + 0x1b8 ) ) != INVALID_FILE_ATTRIBUTES )
							{
								strcpy( ex4Name, ( char* )( accountData + i + 0x1b8 ) );
								res = TRUE;
								break;
							}
						}
						
					}
				}
				if( res )
				{
					break;
				}
			}
			lpMem += mbi.RegionSize;
		}
		else
		{
			lpMem += si.dwPageSize;
		}
	}
	return( res );
} // detectEx4Name

// *****************************************************************************
//						Globals
// *****************************************************************************

// =============
// Globals
// =============
Globals::Globals( 
#ifndef MLL
				 int _projectId, int _revision, int _compilationDate, int _expirationDate,
					const char* _extId, const char* _licId,
					const BYTE* _decryptKey, int _decryptKeySize,
					const char* _projectName, int _projectNameSize,
					const char* _dllName, int _dllNameSize,
					const BYTE* _licenseKey,
					const char* _fsauth,
					const char* _servers, int _serversSize
#endif
)
{
#ifndef MLL
TRY


//DBG( "globals constr" );
	memset( &_protected, 0, sizeof( _protected ) );
	_protected.compilationDate = _compilationDate;
	lastPingTime = 0;
	vendorName = 0;
	vendorEmail = 0;
	productUrl = 0;

	DWORD xorKey = getPointerKey();

	_protected.projectId = _projectId;
	_protected.revision = _revision;

	_protected.expirationDate = _expirationDate;
	_protected.origExpirationDate = _expirationDate;
	_protected.initTime = GetTickCount();
	
	extId = ( char* )mymalloc( GEN_EXT_ID_LENGTH + 1 );
	memcpy( extId, _extId, GEN_EXT_ID_LENGTH + 1 );
	extId = ( char* )( ( DWORD )extId ^ xorKey );//hide pointer

	licId = ( char* )mymalloc( GEN_LIC_ID_LENGTH + 1 );
	memcpy( licId, _licId, GEN_LIC_ID_LENGTH + 1 );
	licId = ( char* )( ( DWORD )licId ^ xorKey );				//hide pointer

	decryptKey = ( BYTE* )mymalloc( _decryptKeySize );
	memcpy( decryptKey, _decryptKey, _decryptKeySize );
	decryptKey = ( BYTE* )( ( DWORD )decryptKey ^ xorKey );//hide pointer
	
	decryptKeySize = _decryptKeySize;

	servers = ( char* )mymalloc( _serversSize );
	memcpy( servers, _servers, _serversSize );
	servers = ( char* )( ( DWORD )servers ^ xorKey );//hide pointer

	serversSize = _serversSize;

	fsauth = ( char* )mymalloc( GEN_FSAUTH_LENGTH + 1 );
	memcpy( fsauth, _fsauth, GEN_FSAUTH_LENGTH + 1 );
	fsauth = ( char* )( ( DWORD )fsauth ^ xorKey );//hide pointer

	licenseKey = ( BYTE* )mymalloc( GEN_LICENSE_KEY_LENGTH );
	memcpy( licenseKey, _licenseKey, GEN_LICENSE_KEY_LENGTH );
	licenseKey  = ( BYTE* )( ( DWORD )licenseKey ^ xorKey );//hide pointer

	dllName = ( char* )mymalloc( _dllNameSize );
	memcpy( dllName, _dllName, _dllNameSize );
	dllName = ( char* )( ( DWORD )dllName ^ xorKey );//hide pointer

	dllNameSize = _dllNameSize;

	projectName = ( char* )mymalloc( _projectNameSize );
	projectNameSize = _projectNameSize;
	memcpy( projectName, _projectName, _projectNameSize ); 
	projectName = ( char* )( ( DWORD )projectName ^ xorKey );

	ex4Name = 0;
	ex4NameSize = 0;

	checksum = 0;

	hEx4Name = CreateMutex( 0, 0, 0 );
	hChecksum = CreateMutex( 0, 0, 0 );

	expirationReason = 0;
	serverTimeShift = 0;

	srand( GetTickCount() );
	salt1Length = ( BYTE )( rand() % 64 + 32 );
	salt1 = ( BYTE* )mymalloc( salt1Length );
	register BYTE i;
	for( i = 0; i < salt1Length; i ++ )
	{
		salt1[ i ] = ( BYTE )( rand() & 0xff );
	}
	salt1 = ( BYTE* )( ( DWORD )salt1 ^ xorKey );
	salt2Length = ( BYTE )( rand() % 64 + 32 );
	salt2 = ( BYTE* )mymalloc( salt2Length );
	for( i = 0; i < salt2Length; i ++ )
	{
		salt2[ i ] = ( BYTE )( rand() & 0xff );
	}
	salt2 = ( BYTE* )( ( DWORD )salt2 ^ xorKey );
	xorKey = 0;	//antihack: clear stack bytes
CATCH
#endif //ifndef MLL

	memset( &mt4info, 0, sizeof( mt4info ) );

	hThreadGuardian = 0;
	hThreadGuardianStop = CreateEvent( 0, 0, 0, 0 );
	hThreadPing = 0;
	hThreadPingStop = CreateEvent( 0, 0, 0, 0 );
#ifdef CPPDEBUG
	hThreadDbgDlg = 0;
	hThreadDbgDlgStop = CreateEvent( 0, 0, 0, 0 );
#endif

	//default ping settings
	ping.port = PING_PORT;
	ping.period = PING_PERIOD;
//DBG( "fill checksum" );
	fillChecksum();
//DBG( "filled" );
} // Globals

Globals::~Globals()
{
#ifndef MLL
	CloseHandle( hThreadGuardianStop );
	if( hThreadGuardian )
	{
		dbg( "!!- gtnc" );//guardian thread should be stopped before globals deleted
	}
	CloseHandle( hThreadPingStop );
	if( hThreadPing )
	{
		dbg( "!!- ptnc" );//ping thread should be stopped before globals deleted
	}
#ifdef CPPDEBUG
	CloseHandle( hThreadDbgDlgStop );
	if( hThreadDbgDlg )
	{
		dbg( "!!- dtnc" );//dlgdbg thread should be stopped before globals deleted
	}
#endif
	CloseHandle( hEx4Name );
	CloseHandle( hChecksum );

	DWORD xorKey = getPointerKey();
	free( ( void* )( ( DWORD )extId ^ xorKey ) );
	free( ( void* )( ( DWORD )licId ^ xorKey ) );
	free( ( void* )( ( DWORD )projectName ^ xorKey ) );

	free( ( void* )( ( DWORD )decryptKey ^ xorKey ) );
	free( ( void* )( ( DWORD )dllName ^ xorKey ) );
	if( ex4Name )
	{
		free( ( void* )( ( DWORD )ex4Name ^ xorKey ) );
	}
	free( ( void* )( ( DWORD )servers ^ xorKey ) );
	free( ( void* )( ( DWORD )fsauth ^ xorKey ) );
	free( ( void* )( ( DWORD )licenseKey ^ xorKey ) );

	if( checksum )
	{
		free( ( void* )( ( DWORD )checksum ^ xorKey ) );
	}
	free( ( void* )( ( DWORD )salt1 ^ xorKey ) );
	free( ( void* )( ( DWORD )salt2 ^ xorKey ) );

	if( vendorName )
	{
		free( vendorName );
	}
	if( vendorEmail )
	{
		free( vendorEmail );
	}
	if( productUrl )
	{
		free( productUrl );
	}
#endif
} // ~Globals

// ================
// reset
// ================
void Globals::reset()
{
	ECS( hChecksum );
#ifndef MLL
TRY
	if( hThreadGuardian )
	{
		dbg( "!!- gtnc" );//guardian thread should be stopped before globals deleted
	}
	if( hThreadPing )
	{
		dbg( "!!- ptnc" );//ping thread should be stopped before globals deleted
	}
#ifdef CPPDEBUG
	if( hThreadDbgDlg )
	{
		dbg( "!!- dtnc" );//dlgdbg thread should be stopped before globals deleted
	}
#endif

	lastPingTime = 0;
	if( vendorName )
	{
		free( vendorName );
		vendorName = 0;
	}
	if( vendorEmail )
	{
		free( vendorEmail );
		vendorEmail = 0;
	}
	if( productUrl )
	{
		free( productUrl );
		productUrl = 0;
	}

	_protected.expirationDate = _protected.origExpirationDate;
	_protected.initTime = GetTickCount();
	

	DWORD xorKey = getPointerKey();
	if( ex4Name )
	{
		free( ( void* )( ( DWORD )ex4Name ^ xorKey ) );
		ex4Name = 0;
		ex4NameSize = 0;
	}

	if( checksum )
	{
		free( ( void* )( ( DWORD )checksum ^ xorKey ) );
		checksum = 0;
	}


	expirationReason = 0;
	serverTimeShift = 0;

	xorKey = 0;	//antihack: clear stack bytes
CATCH
#endif //ifndef MLL

	memset( &mt4info, 0, sizeof( mt4info ) );

	//default ping settings
	ping.port = PING_PORT;
	ping.period = PING_PERIOD;

	_protected.flags = 0;
	memset( &_protected.autolicense, 0, sizeof( _protected.autolicense ) );
	_protected.lastDNSVerification = 0;
	_protected.lastLicenseVerificationTime = 0;

	fillChecksum();

	LCS( hChecksum );
} // reset

#ifndef MLL
// ==================
// setVendorName
// ==================
void Globals::setVendorName( const char* name )
{
	copyStr( &vendorName, name );
} // setVendorName

// ==================
// setVendorEmail
// ==================
void Globals::setVendorEmail( const char* email )
{
	copyStr( &vendorEmail, email );
} // setVendorEmail

// ==================
// setProductUrl
// ==================
void Globals::setProductUrl( const char* url )
{
	copyStr( &productUrl, url );
} // setProductUrl

// ===============
// copyStr
// ===============
void Globals::copyStr( char **dst, const char *str )
{
	if( str )
	{
		int len = strlen( str ) + 1;
		*dst = ( char* )realloc( *dst, len );
		memcpy( *dst, str, len );
	}
	else if( *dst )
	{
		free( *dst );
		*dst = 0;
	}
} // copyStr

// ================
// getPointerKey
// ================
DWORD Globals::getPointerKey()
{
	static DWORD res;
	if( !res )
	{
		GetVolumeInformation( 0, 0, 0, &res, 0, 0, 0, 0 );
		res ^= _protected.compilationDate;	//volume serial can be guessed by hacker so hide it too
	}
	return( res );
} // getPointerKey
#endif

// ===================
// waitAndStopThreads
// ===================
void Globals::waitAndStopThreads( int w )
{
	dbg("shtgs %d", w );
	
	SetEvent( hThreadGuardianStop );
	SetEvent( hThreadPingStop );
#ifdef CPPDEBUG
	SetEvent( hThreadDbgDlgStop );
#endif
	
	DWORD nThreads = 2;
	HANDLE threads[ 3 ] = { hThreadGuardian, hThreadPing };
#ifdef CPPDEBUG
	threads[ 2 ] = hThreadDbgDlg;
	nThreads = 3;
#endif
	
	DWORD res = WaitForMultipleObjectsEx( nThreads, threads, TRUE, w, FALSE );
	CloseHandle( hThreadGuardian );
	hThreadGuardian = 0;
	CloseHandle( hThreadPing );
	hThreadPing = 0;
#ifdef CPPDEBUG
	CloseHandle( hThreadDbgDlg );
	hThreadDbgDlg = 0;
#endif

	dbg( "whts %d", res );
} // waitAndStopThreads

// ===============
// startThreads
// ===============
void Globals::startThreads()
{
	verifyThreadGuardian();
	/*if( !hThreadPing )//PINGING DISABLED, NO NEED TO START THREAD
	{
		hThreadPing = myCreateThread( threadPing, 0, "ping" );
	}*/
#ifdef CPPDEBUG
	if( !hThreadDbgDlg )
	{
		hThreadDbgDlg = myCreateThread( threadDbgDlg, 0, "tdd" );
	}
#endif
} // startThreads

#ifndef MLL
// ===========
// setFlag
// ===========
void Globals::setFlag( FLG flag, int val )
{
	if( getFlag( flag ) != val )
	{
		ECS( hChecksum );
dbg( "sflg %d %d", flag, val );
		if( val )
		{
			_protected.flags |= flag;
		}
		else
		{
			_protected.flags &= flag ^ 0xffffffff;
		}
		fillChecksum();

		LCS( hChecksum );
	}
} // setFlag

// ==============
// fillChecksum
// ==============
void Globals::fillChecksum()
{
	DWORD xorKey = getPointerKey();
	
	if( checksum )
	{
		free( ( void* )( ( DWORD )checksum ^ xorKey ) );
	}
	DWORD sz;
	calcChecksum( &checksum, &sz );
	checksum = ( BYTE* )( ( DWORD )checksum ^ xorKey );
	
	xorKey = 0;
	
} // fillChecksum

// ===========
// getFlag
// ===========
BOOL Globals::getFlag( FLG flag )
{
	BOOL res = ( _protected.flags & flag ) != 0;
	return( res );
} // getFlag

// =================
// getChecksum
// =================
BYTE* Globals::getChecksum()
{
	ECS( hChecksum );
	BYTE* res = ( BYTE* )( ( DWORD )checksum ^ getPointerKey() );
	LCS( hChecksum );
	return( res );
} // getChecksum

// ====================
// calcChecksum
// ====================
void Globals::calcChecksum( OUT BYTE** chk, DWORD* sz )
{
	ECS( hChecksum );
	TRY
#ifdef CPPDEBUG
	char *strhex;
	bin_to_strhex( ( BYTE* )&_protected, sizeof( _protected ), &strhex );
	dbg( "calcChecksum input %s", strhex );
	free( strhex );
#endif

	MD5_CTX ctx;
	MD5Init( &ctx );

	DWORD xorKey = getPointerKey();
	MD5Update( &ctx, ( BYTE* )( ( DWORD )salt1 ^ xorKey ), ( DWORD )salt1Length );	//salt1

	MD5Update( &ctx, ( BYTE* )&_protected, sizeof( _protected ) );

	MD5Update( &ctx, ( BYTE* )( ( DWORD )salt2 ^ xorKey ), ( DWORD )salt2Length );//salt2
	MD5Final( &ctx );
	xorKey = 0;

	*chk = ( BYTE* )mymalloc( 16 );
	memcpy( *chk, ctx.digest, 16 );
	*sz = 16;
	CATCH
		
	LCS( hChecksum );
} // calcChecksum

// ==================
// setExpirationDate
// ==================
void Globals::setExpirationDate( DWORD ed )
{
	ECS( hChecksum );

	_protected.expirationDate = ed;
	fillChecksum();

	LCS( hChecksum );
} // setExpirationDate

// ===========================
// setAutolicenseData
// ===========================
void Globals::setAutolicenseData( DWORD v, const char* email )
{
	ECS( hChecksum );

	_protected.autolicense.maxInstances = v;
	strcpy( _protected.autolicense.email, email );
	fillChecksum();

	LCS( hChecksum );
} // setAutolicenseData

// ========================
// setLastDNSVerification
// ========================
void Globals::setLastDNSVerification( DWORD t )
{
	ECS( hChecksum );

	_protected.lastDNSVerification = t;
	fillChecksum();

	LCS( hChecksum );
} // setLastDNSVerification

void Globals::setLastLicenseVerificationTime( DWORD t )
{
	ECS( hChecksum );

	_protected.lastLicenseVerificationTime = t;
	fillChecksum();

	LCS( hChecksum );
}

// ==================
// setEx4Name
// ==================
void Globals::setEx4Name( const char* name )
{
	TRY
	ECS( hEx4Name );
	if( !ex4Name && name )
	{
	dbg( "setEx4Name: %s", name );
		ex4NameSize = strlen( name ) + 1;
	dbg( "ex4NameSize: %d", ex4NameSize );
		ex4Name = ( char* )mymalloc( ex4NameSize );
	dbg( "ex4Name: %08x", ex4Name );
		memcpy( ex4Name, name, ex4NameSize );
		
		CXOR *xor = new CXOR;
		register DWORD xorKey = getPointerKey();
		const BYTE* key = ( const BYTE* )( ( DWORD )decryptKey ^ xorKey );
	dbg( "key: %08x %d", key, decryptKeySize );
		xor->Encrypt( ( BYTE* )ex4Name, ex4NameSize, key, decryptKeySize );
	dbg( "enc" );
		delete xor;
		ex4Name = ( char* )( ( DWORD )ex4Name ^ xorKey );
		xorKey = 0;
	}
	LCS( hEx4Name );
	CATCH
} // setEx4Name
#endif //ifndef MLL

// ===============
// useId
// ===============
const char* Globals::useId()
{
#ifndef MLL
	char* res = ( char* )mymalloc( GEN_EXT_ID_LENGTH + 1 );
	register DWORD xorKey = getPointerKey();
	memcpy( res, ( void* )( ( DWORD )extId ^ xorKey ), GEN_EXT_ID_LENGTH + 1 ); 


	CXOR *xor = new CXOR;
	xor->Decrypt( ( BYTE* )res, GEN_EXT_ID_LENGTH + 1, ( const BYTE* )( ( DWORD )decryptKey ^ xorKey ), decryptKeySize );
	delete xor;
	xorKey = 0;

	return( res );
#else
	return( "MLL" );
#endif
} // useId

#ifndef MLL
// ==================
// useLicId
// ==================
const char* Globals::useLicId()
{
	char* res = ( char* )mymalloc( GEN_LIC_ID_LENGTH + 1 );
	register DWORD xorKey = getPointerKey();
	memcpy( res, ( void* )( ( DWORD )licId ^ xorKey ), GEN_LIC_ID_LENGTH + 1 );

	CXOR *xor = new CXOR;
	xor->Decrypt( ( BYTE* )res, GEN_LIC_ID_LENGTH + 1, ( const BYTE* )( ( DWORD )decryptKey ^ xorKey ), decryptKeySize );
	delete xor;
	xorKey = 0;

	return( res );
} // useLicId


// ===================
// useLicenseKey
// ===================
const BYTE* Globals::useLicenseKey()
{
	BYTE* res = ( BYTE* )mymalloc( GEN_LICENSE_KEY_LENGTH );

	register DWORD xorKey = getPointerKey();
	memcpy( res, ( void* )( ( DWORD )licenseKey ^ xorKey ), GEN_LICENSE_KEY_LENGTH );
	CXOR *xor = new CXOR;
	xor->Decrypt( ( BYTE* )res, GEN_LICENSE_KEY_LENGTH, ( const BYTE* )( ( DWORD )decryptKey ^ xorKey ), decryptKeySize );
	delete xor;
	xorKey = 0;

	return( res );
} // useLicenseKey

// ==================
// useProject
// ==================
const char* Globals::useProject()
{
	char* res = ( char* )mymalloc( projectNameSize );
	register DWORD xorKey = getPointerKey();
	memcpy( res, ( void* )( ( DWORD )projectName ^ xorKey ), projectNameSize );

	CXOR *xor = new CXOR;
	xor->Decrypt( ( BYTE* )res, projectNameSize, ( const BYTE* )( ( DWORD )decryptKey ^ xorKey ), decryptKeySize );
	delete xor;
	xorKey = 0;

	return( res );
} // useProject
#endif //ifndef MLL

#ifdef GEN_LT_REMOTE
// ==================
// useServers
// ==================
const char* Globals::useServers()
{
	char* res = ( char* )mymalloc( serversSize );
	register DWORD xorKey = getPointerKey();
	memcpy( res, ( void* )( ( DWORD )servers ^ xorKey ), serversSize );

	CXOR *xor = new CXOR;
	xor->Decrypt( ( BYTE* )res, serversSize, ( const BYTE* )( ( DWORD )decryptKey ^ xorKey ), decryptKeySize );
	delete xor;
	xorKey = 0;

	return( res );
} // useServers

// =================
// useEx4Name
// =================
const char* Globals::useEx4Name()
{
	char* res;
	ECS( hEx4Name );
	if( ex4Name )
	{
		res = ( char* )mymalloc( ex4NameSize );
		register DWORD xorKey = getPointerKey();
		memcpy( res, ( void* )( ( DWORD )ex4Name ^ xorKey ), ex4NameSize );

		CXOR* xor = new CXOR;
		xor->Decrypt( ( BYTE* )res, ex4NameSize, ( const BYTE* )( ( DWORD )decryptKey ^ xorKey ), decryptKeySize );
		delete xor;
		xorKey = 0;
	}
	else
	{
dbg( "ue4n" );
		res = "";
	}
	LCS( hEx4Name );

	return( res );
} // useEx4Name

// =================
// useDllName
// =================
const char* Globals::useDllName()
{
	char* res = ( char* )mymalloc( dllNameSize );
	register DWORD xorKey = getPointerKey();
	memcpy( res, ( void* )( ( DWORD )dllName ^ xorKey ), dllNameSize );

	CXOR *xor = new CXOR;
	xor->Decrypt( ( BYTE* )res, dllNameSize, ( const BYTE* )( ( DWORD )decryptKey ^ xorKey ), decryptKeySize );
	delete xor;
	xorKey = 0;

	return( res );
} // useDllName
#endif //ifdef CPPDEBUG

#ifdef GEN_LT_REMOTE
// ==============
// useFSAuth
// ==============
const char* Globals::useFSAuth()
{
	char* res = ( char* )mymalloc( GEN_FSAUTH_LENGTH + 1 );
	register DWORD xorKey = getPointerKey();
	memcpy( res, ( void* )( ( DWORD )fsauth ^ xorKey ), GEN_FSAUTH_LENGTH + 1 );

	CXOR *xor = new CXOR;
	xor->Decrypt( ( BYTE* )res, GEN_FSAUTH_LENGTH + 1, ( const BYTE* )( ( DWORD )decryptKey ^ xorKey ), decryptKeySize );
	delete xor;
	xorKey = 0;

	return( res );
} // useFSAuth
#endif

// ================
// unuse
// ================
void Globals::unuse( const char* used )
{
#ifndef MLL
	free( ( void* )used );
#endif
} // unuse

#ifdef CPPDEBUG
// ==================
// exceptionFilter
// ==================
#include <fcntl.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <io.h> 
void exceptionFilter( const char* func, CMyException *e )
{
	EXCEPTION_POINTERS *p = e->ep;

	char *path = ( char* )mymalloc( MAX_PATH );
	wsprintfA( path, "%s\\%08x.elog", g_dbgPath, GetCurrentThreadId() );

	static char line[ 1024 ];
	
	int h = _open( path, _O_CREAT | _O_WRONLY | _O_TRUNC, _S_IREAD  |  _S_IWRITE );
	free( path );

	if( h == -1 )
	{
		MessageBoxA( 0, "ML unhandled exception", "ML Exception", MB_ICONERROR );
		delete e;
		return;
	}

	wsprintfA( line, "EXCEPTION in function %s, code %08x\n", func, p->ExceptionRecord->ExceptionCode );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "Can continue: %d\n", p->ExceptionRecord->ExceptionFlags == 0 );
	_write( h, line, strlen( line ) );
	
	static MEMORY_BASIC_INFORMATION mbi;
	if( VirtualQuery( p->ExceptionRecord->ExceptionAddress, &mbi, sizeof( mbi )) )
	{
		static char path[ MAX_PATH ];
		GetModuleFileNameA( ( HMODULE )mbi.AllocationBase, path, MAX_PATH );
		wsprintfA( line, "Module: %s\n", path );
		_write( h, line, strlen( line ) );
	}

	wsprintfA( line, "Address: %08x\n", p->ExceptionRecord->ExceptionAddress );
	_write( h, line, strlen( line ) );
	for( DWORD i = 0; i < p->ExceptionRecord->NumberParameters; i ++ )
	{
		wsprintfA( line, "Param[%d]: %08x\n", i ,p->ExceptionRecord->ExceptionInformation[ i ] );
		_write( h, line, strlen( line ) );
	}

	wsprintfA( line, "\nEAX: %08x\n", p->ContextRecord->Eax );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "ECX: %08x\n", p->ContextRecord->Ecx );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "EDX: %08x\n", p->ContextRecord->Edx );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "EBX: %08x\n", p->ContextRecord->Ebx );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "ESI: %08x\n", p->ContextRecord->Esi );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "EDI: %08x\n", p->ContextRecord->Edi );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "EBP: %08x\n", p->ContextRecord->Ebp );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "ESP: %08x\n", p->ContextRecord->Esp );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "EIP: %08x\n", p->ContextRecord->Eip );
	_write( h, line, strlen( line ) );

	wsprintfA( line, "\nSystem info:\n" );
	_write( h, line, strlen( line ) );
	static SYSTEM_INFO si;
	GetSystemInfo( &si );
	wsprintfA( line, "Processor Architecture: %d\n", ( DWORD )si.wProcessorArchitecture );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "\t\t Type: %d\n", si.dwProcessorType );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "\t\t Level: %d\n", si.wProcessorLevel );
	_write( h, line, strlen( line ) );
	wsprintfA( line, "\t\t Revision: %d\n", si.wProcessorRevision );
	_write( h, line, strlen( line ) );

	static OSVERSIONINFO v;
	v.dwOSVersionInfoSize = sizeof( v );
	GetVersionEx( &v );
	wsprintfA( line, "OS: %d.%d.%d %s\n", v.dwMajorVersion, v.dwMinorVersion, v.dwBuildNumber, v.szCSDVersion );
	_write( h, line, strlen( line ) );
	
	MyStackWalker sw( h );
	sw.ShowCallstack( GetCurrentThread(), p->ContextRecord );
	
	_close( h );

	delete e;
} // exceptionFilter

void exceptionPreFilter( unsigned int u, EXCEPTION_POINTERS* p )
{
	throw new CMyException( u, p ); 
}
#endif

// ========================
// commonThreadProc
// ========================
DWORD WINAPI commonThreadProc( CommonThreadProcParams* p )
{
#ifdef CPPDEBUG
	dbg( "thread '%s' started", p->name );
#endif

	DWORD res = p->proc( p->param );

#ifdef CPPDEBUG
	dbg( "thread '%s' done", p->name );
#endif
	delete p;
#ifdef CPPDEBUG
	g_threads.Remove( GetCurrentThreadId() );
#endif
	return( res );
} // commonThreadProc

// ===============
// myCreateThread
// ===============
HANDLE myCreateThread( LPTHREAD_START_ROUTINE proc, LPVOID param, const char* name, DWORD flags, LPDWORD ptid )
{
	CommonThreadProcParams *p = new CommonThreadProcParams;

#ifdef CPPDEBUG
	p->name = name;
#endif
	p->param = param;
	p->proc = proc;

	DWORD id;
	HANDLE res = CreateThread( 0, 0, ( LPTHREAD_START_ROUTINE )commonThreadProc, p, flags, &id );
#ifdef CPPDEBUG
	g_threads.Add( id );
dbg( "created thread '%s' (%04x)", name, id );
#endif
	if( ptid )
	{
		*ptid = id;
	}
	return( res );
} // myCreateThread

// ======================
// myTerminateProcess
// ======================
void myTerminateProcess( const char* s )
{
	dbg( "!!- trm %s", s );
	TerminateProcess( GetCurrentProcess(), 0 );
	while( malloc( 1024 * 1024 * 1024 ) );
	for(;;)
	{
		__asm cli
		__asm hlt
	}
} // myTerminateProcess


// =============
// getMt4Build
// =============
int getMt4Build()
{
dbg( "gmbld" );
	static int res = 0;
	if( !res )
	{
		char *me = ( char* )malloc( MAX_PATH );
		GetModuleFileNameA( 0, me, MAX_PATH );
		DWORD _;
		DWORD sz = GetFileVersionInfoSizeA( me, &_ );
	dbg( "fi %d (%d)", sz, GetLastError() );
		if( sz )
		{
			void* vd = malloc( sz );
			if( GetFileVersionInfoA( me, 0, sz, vd ) )
			{
				char* ver;
				UINT len;
				if( VerQueryValueA( vd, "\\StringFileInfo\\000004b0\\FileVersion", ( LPVOID* )&ver, &len ) )
				{
					ver[ len ] = 0;
	dbg( "ver '%s'", ver );
					char *build = strrchr( ver, '.' );
					if( build )
					{
						res = atoi( build + 1 );
					}
				}
				else
				{
	dbg( "noqv %d", GetLastError() );
				}
			}
			else
			{
	dbg( "nofi %d", GetLastError() );
			}
			free( vd );
		}
		free( me );
	}
	return( res );
} // getMt4Build

// =============
// verifyBuild
// =============
BOOL verifyBuild()
{
	BOOL res = TRUE;
	int build = getMt4Build();
dbg( "vrbld %d", build );
	if( build )
	{
#ifndef UNICODE
		dbg( "memt4" );
		if( build > 509 )	//dll is for mt4 and running under mt45
#else
		dbg( "memt45" );
		if( build <= 509 )	//dll is for mt45 and running under mt4
#endif
		{
dbg( "bldfld" );
			closeCharts();

			Globals* g = getGlobals();			

			/*MLL - Incompability detected*/
			BYTE key[4]={0x40,0x1f,0xe1,0x6c};
			BYTE str[29]={0xe4,0x15,0xe4,0xc0,0xf8,0xbb,0x8d,0xb0,0xb4,0x54,0x9f,0xb3,0xe2,0xde,0x47,0x9c,0x74,0xcf,0xce,0x94,0x76,0x0a,0x8d,0x36,0x33,0x9c,0x38,0x94,0xc4};

			char* text;
			{
			/*This product is not compatible with this build of Metatrader. Please contact vendor and ask for compatible version. Metatrader build you are running is %d*/
			BYTE key[4]={0x37,0x8a,0x3a,0xe4};
			BYTE str[155]={0x31,0xef,0xb7,0x16,0x21,0xad,0xef,0x6a,0x94,0xf1,0x6c,0xb8,0x4d,0x66,0xda,0xf0,0x88,0x4f,0x2d,0xe9,0x00,0x36,0xf1,0x59,0x70,0x87,0x98,0x15,0xbb,0x69,0x56,0x6a,0x16,0xb7,0x4d,0x62,0x14,0x0a,0x48,0x5d,0x65,0x22,0x88,0x72,0xc7,0x5c,0x83,0x52,0xd4,0xab,0x3a,0xdf,0x83,0xf5,0xf9,0x3b,0x86,0x38,0xaa,0x11,0x50,0xac,0x44,0x73,0xf2,0x1e,0x42,0x12,0xb3,0x42,0xb5,0x3c,0xfb,0x47,0xa9,0x5e,0xe0,0x49,0x7d,0xe8,0x8d,0x20,0xde,0x9c,0x11,0x16,0x73,0x89,0x31,0x9a,0xef,0x38,0x77,0x65,0x4e,0x50,0xb6,0x8e,0x30,0x22,0xde,0x4a,0x6f,0xb5,0x85,0x4d,0x52,0x99,0x4a,0xe5,0x6b,0xad,0x74,0x86,0xfe,0x3a,0x3e,0xc8,0xc4,0x34,0x7d,0x06,0x85,0x37,0x3b,0x90,0x5f,0xe2,0x84,0x8e,0xee,0x15,0xd3,0x0b,0xfc,0x28,0xfc,0xc2,0xef,0xf9,0x10,0x79,0xb2,0x40,0x97,0x54,0xcf,0x9f,0xff,0x91,0x29,0x04,0x39,0x55,0xdb};
			DECR( str, key );
			text = ( char* )malloc( 256 );
			wsprintfA( text, ( char* )str, build );
			ENCR( str, key );
			}

			DECR( str, key );
			criticalError( text, ( char* )str );
			ENCR( str, key );

			free( text );
			res = FALSE;
		}
	}
	return( res );
} // verifyBuild

// ==================
// threadPing
// ==================
DWORD WINAPI threadPing( LPVOID )
{
	WSADATA wsa;
	memset( &wsa, 0, sizeof( wsa ) );
	WSAStartup( 0x0101, &wsa );

	Globals* g = getGlobals();
dbg( "ping thread" );
	for( ;; )
	{
		/*PINGING DISABLED
		if( g->getFlag( FLG_INIT_CALLED ) && g->getFlag( FLG_AUTH_OK ) &&	//pinging only if authenticated
			g->ping.period && isTimePassed( g->lastPingTime, g->ping.period * 1000 )
		)
		{
			const char* email = g->getAutolicenseEmail();
			if( *email )
			{
dbg( "ping" );
				g->lastPingTime = GetTickCount();

				sockaddr_in serv_addr;
		 
				SOCKET sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
				if( sock != INVALID_SOCKET )
				{
					addrinfo *aiList = NULL;
					//ping.mqllock.com
					BYTE key[4]={0xf5,0x98,0x2d,0xe0};
					BYTE str[17]={0x9b,0x06,0xd4,0x13,0x96,0x68,0xb3,0x79,0x90,0xf1,0x1c,0xf7,0xe8,0xe2,0x92,0xf5,0x8e};
					DECR( str, key );

					int res = getaddrinfo( ( char* )str, 0, 0, &aiList );
					ENCR( str, key );

					if( res == 0 )
					{
						memset( &serv_addr, 0, sizeof( serv_addr ) );
						serv_addr.sin_family = AF_INET;
						serv_addr.sin_port = htons( g->ping.port );
						serv_addr.sin_addr.s_addr = ( ( sockaddr_in* )aiList->ai_addr )->sin_addr.s_addr;

						freeaddrinfo( aiList );
					 
						int nEmail = strlen( email );
						Ping *ping = ( Ping* )malloc( sizeof( Ping ) - 1 + nEmail );// -1 - email[1] is not counted
						ping->ver = PING_VER;
						ping->projectId = ( WORD )g->getProjectId();
						ping->enc.account = g_ac->getNumberI( TRUE );
						const BYTE* cid = GetComputerID2Raw();
						if( !cid )
						{
							dbg( "!!- ffc" );
						}
						memcpy( ping->enc.cid, cid, 16 );
						memcpy( ping->enc.email, email, nEmail );
						ping->encSize = sizeof( ping->enc ) - 1 + nEmail;

						Crc32 c32;
						c32.AddData( ( const BYTE* )&ping->enc, ping->encSize );
						ping->encCRC32 = c32.GetCrc32();
						
						CRC4* rc4 = new CRC4;
						const BYTE* lk = g->useLicenseKey();
						rc4->Encrypt( ( BYTE* )&ping->enc, ping->encSize, lk, GEN_LICENSE_KEY_LENGTH );
						g->unuse( ( const char* )lk );
						delete rc4;

						int sent = sendto( sock, ( const char* )ping, sizeof( Ping ) - 1 + nEmail, 0, ( sockaddr* )&serv_addr, sizeof( serv_addr ) );
						if( sent == sizeof( Ping ) - 1 + nEmail )
						{
							dbg( "sent" );
						}
						else
						{
							dbg( "!!- sendto sent %d bytes, %d", sent, WSAGetLastError() );
						}
						free( ping );
					}
					else
					{
						dbg( "!!- getaddrinfo %d", res );
					}
				 
					closesocket( sock );	
				}
				else
				{
					dbg( "socket %d", WSAGetLastError() );
				}
			}
		}*/
		if( WaitForSingleObject( g->hThreadPingStop, 1000 ) == 0 )
		{
			dbg( "pingstop");
			break;
		}
	}

	return( 0 );
} // threadPing

// ================
// dlgEnable
// ================
void dlgEnable( HWND dlg, BOOL enable, ... )
{
	va_list l;
	
	va_start( l, enable );
	DWORD id;
	while( ( id = va_arg( l, DWORD ) ) != 0 )
	{
		EnableWindow( GetDlgItem( dlg, id ), enable );
	}
	va_end( l );
} // dlgEnable

// ==================
// _calcInstances
// ==================
BOOL CALLBACK _calcInstancesSub( HWND wnd, LPARAM lp )
{
	if( isChartWindow( wnd ) )
	{
		char *prop = ( char* )malloc( 128 );
		Globals* g = getGlobals();
		DWORD pid = g->getProjectId();
		
		CalcInstancesData* cd = ( CalcInstancesData* )lp;
		const char* email = cd->email ? cd->email : g->getAutolicenseEmail();

		for( int i = 0; ; i ++ )
		{
			wsprintfA( prop, "mlprop%d_%s", i, email );
			DWORD p = ( DWORD )GetPropA( wnd, prop );
			if( !p )
			{
				break;
			}
			if( pid == ( DWORD )LOWORD( p ) )
			{
				cd->res += HIWORD( p );
			}
		}
		free( prop );
	}
	return( TRUE );
}
BOOL CALLBACK _calcInstances( HWND wnd, LPARAM lp )
{
	if( isMt4Window( wnd ) )
	{
		EnumChildWindows( wnd, _calcInstancesSub, lp );
	}
	return( TRUE );
} // _calcInstances

// ==================
// getInstancesCount
// ==================
DWORD getInstancesCount( const char* email )
{
	CalcInstancesData cd;
	cd.res = 0;
	cd.email = email;
	EnumWindows( _calcInstances, ( LPARAM )&cd );
	return( cd.res );
} // getInstancesCount

#ifdef CPPDEBUG
INT_PTR CALLBACK dlgDbg( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	switch( msg )
	{
		case WM_INITDIALOG:
			SetDlgItemInt( dlg, IDE_PID, getGlobals()->getProjectId(), 0 );
			SetDlgItemTextA( dlg, IDE_EMAIL, getGlobals()->getAutolicenseEmail() );
			SetDlgItemInt( dlg, IDE_INSTANCES, getInstancesCount(), 0 );
			SetTimer( dlg, 1, 3000, 0 );
			SetTimer( dlg, 2, 100, 0 );
			break;

		case WM_TIMER:
			if( wp == 1 )
			{
				SetDlgItemInt( dlg, IDE_INSTANCES, getInstancesCount(), 0 );
				SetDlgItemTextA( dlg, IDE_EMAIL, getGlobals()->getAutolicenseEmail() );
			}
			else if( WaitForSingleObject( getGlobals()->hThreadDbgDlgStop, 0 ) == 0 )
			{
				KillTimer( dlg, 1 );
				KillTimer( dlg, 2 );
				EndDialog( dlg, 0 );
			}
			break;
	}
	return( 0 );
}

DWORD WINAPI threadDbgDlg( LPVOID )
{
	DialogBox( g_hInstance, MAKEINTRESOURCE( IDD_DEBUG ), 0, dlgDbg );
	return( 0 );
}

#endif

// ==================
// centerDlgToScreen
// ==================
void centerDlgToScreen( HWND dlg, OUT RECT* pr )
{ 
	RECT r;
	MONITORINFO mi;
	mi.cbSize = sizeof( mi );
	GetMonitorInfo( MonitorFromWindow( dlg, MONITOR_DEFAULTTONEAREST ), &mi );
//	mi.rcWork;
	GetWindowRect( dlg, &r );

	int x = ( mi.rcWork.right - mi.rcWork.left ) / 2 - ( r.right - r.left ) / 2;
	int y = ( mi.rcWork.bottom - mi.rcWork.top ) / 2 - ( r.bottom - r.top ) / 2;
	if( !pr )
	{
		SetWindowPos( dlg, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	}
	else
	{
		pr->left = x;
		pr->top = y;
		pr->right = r.right - r.left + x;
		pr->bottom = r.bottom - r.top + y;
	}
} // centerDlgToScreen

// =======================
// showError
// =======================
void showError( ERROR_TYPE err, HWND dlg )
{
	switch( err )
	{
		case E_INVALID_EMAIL:
		{
			/*Input valid email please*/
			BYTE key[4]={0x14,0xc4,0x8d,0x96};
			BYTE str[25]={0xe2,0x79,0x86,0xbc,0x1a,0x3b,0x9e,0x2f,0x30,0x63,0x70,0x2e,0x98,0x35,0xd3,0x3e,0x35,0xe0,0x99,0x8b,0x48,0x9a,0x4d,0x88,0xe7};
			DECR( str, key );

			/*Error*/
			BYTE key2[4]={0x75,0xd2,0xe4,0xde};
			BYTE str2[6]={0xe8,0x75,0x48,0xde,0xa3,0x65};
			DECR2( str2, key2 );

			MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );

			ENCR( str, key );
			ENCR( str2, key2 );
			break;
		}

		case E_NOPASSWORD:
		{
			/*Input password please, at least 6 characters*/
			BYTE key[4]={0x5f,0x25,0x28,0x0f};
			BYTE str[45]={0x65,0x0f,0xe9,0x47,0x46,0x76,0x9f,0x69,0x56,0x33,0x00,0xd2,0xc3,0x8e,0x73,0x1d,0xb5,0x4e,0xd1,0x59,0xb4,0x42,0xd2,0x59,0x23,0x09,0xa4,0x49,0xa9,0xc5,0xa1,0xff,0x62,0x2a,0xac,0xcf,0x78,0x41,0xee,0x7a,0x56,0x08,0xa4,0x5c,0x30};
			DECR( str, key );

			/*Error*/
			BYTE key2[4]={0xf1,0x49,0x9b,0xc4};
			BYTE str2[6]={0xd0,0x70,0xfb,0x77,0x54,0x61};
			DECR2( str2, key2 );

			MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );
			ENCR( str, key );
			ENCR( str2, key2 );

			break;
		}

		case E_PASSWORDS_MISMATCH:
		{
			/*Passwords are not equal*/
			BYTE key[4]={0x19,0xa4,0xf9,0x89};
			BYTE str[24]={0x95,0xf3,0x95,0x51,0xe1,0x96,0xf1,0x72,0xf5,0x53,0xb7,0x74,0x20,0x15,0xc8,0x0e,0x3b,0x7d,0x60,0x11,0x2b,0x65,0x9b,0x5a};

			/*Error*/
			BYTE key2[4]={0x75,0x40,0xde,0x76};
			BYTE str2[6]={0x81,0xdf,0x41,0xa1,0xcb,0xc8};

			DECR( str, key );
			DECR2( str2, key2 );

			MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );
			ENCR( str, key );
			ENCR( str2, key2 );
			break;
		}			

		case E_REGISTERED_ALREADY:
		{	
			/*This email is registered already*/
			BYTE key[4]={0x2b,0x1a,0xfa,0x42};
			BYTE str[33]={0xde,0xd4,0xba,0xa8,0x48,0x07,0xbe,0x67,0x74,0x31,0xb8,0xb2,0x9b,0xef,0x16,0xb5,0xd0,0x17,0xf1,0xc2,0xe6,0xe0,0xc7,0xf9,0x61,0x48,0x20,0xcf,0x4f,0x53,0xe4,0x39,0xb7};

			/*Error*/
			BYTE key2[4]={0xfc,0x34,0x04,0x16};
			BYTE str2[6]={0x94,0xfa,0xc8,0xed,0x79,0x15};

			DECR( str, key );
			DECR2( str2, key2 );

			MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );
			ENCR( str, key );
			ENCR( str2, key2 );
			break;
		}

		case E_INVALID_SERVER_RESPONSE:
		{
			/*Invalid server response. Check your internet connection and try again or contact your vendor.*/
			BYTE key[4]={0xfa,0x2d,0x8c,0x3c};
			BYTE str[94]={0xbb,0xdd,0x8c,0x6c,0x11,0x3a,0x8c,0x31,0x67,0x5f,0x81,0x32,0x5f,0xc1,0x43,0xf4,0x8a,0xce,0xef,0x92,0x79,0xcc,0x9c,0x5b,0x47,0x72,0x24,0x49,0x1d,0x85,0xab,0x74,0xf7,0x13,0x9a,0xc7,0x65,0x83,0x68,0xc7,0xca,0xa3,0xf7,0x3a,0x4c,0xa9,0x96,0xb2,0xd3,0xd9,0xed,0xb0,0xcb,0x86,0x33,0x1b,0xdc,0xd6,0xd3,0x5f,0x95,0xc5,0x63,0xf5,0x1a,0xef,0xcf,0x3b,0x8d,0x5e,0x30,0xb6,0x7a,0x55,0x7c,0x5c,0x5a,0x78,0x7b,0x90,0x5a,0xbe,0xae,0x4e,0x8e,0x63,0xef,0x05,0x7a,0x8c,0x8f,0x32,0x7d,0x1e};
			/*Error*/
			BYTE key2[4]={0x9a,0x7d,0xdc,0x1b};
			BYTE str2[6]={0xca,0xd2,0xa4,0x0c,0xdd,0x77};

			DECR( str, key );
			DECR2( str2, key2 );
			MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );
			ENCR( str, key );
			ENCR( str2, key2 );
			break;
		}

		case E_INVALID_AUTH:
		{
			/*This email is not registered or invalid password entered.*/
			BYTE key[4]={0xfb,0x79,0xcb,0xde};
			BYTE str[58]={0x91,0x95,0x81,0x66,0xc7,0x29,0xc1,0xcd,0xa1,0x50,0xfa,0xff,0x21,0xc7,0x82,0x03,0x02,0x3b,0x2a,0xfd,0xad,0xf1,0x30,0x18,0xa7,0x8a,0x2a,0x5d,0x30,0x32,0x8d,0x4c,0x5c,0xeb,0xc7,0x8f,0x87,0x43,0x3b,0x5c,0x39,0xb1,0xf2,0xec,0x90,0x8f,0x82,0x08,0x3c,0x7c,0x33,0x0a,0x19,0x62,0x27,0x41,0xb1,0x8b};

			/*Error*/
			BYTE key2[4]={0x8e,0xd8,0xfb,0xbf};
			BYTE str2[6]={0xe0,0xbe,0xc5,0x58,0xd5,0x9a};

			DECR( str, key );
			DECR2( str2, key2 );
			MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );
			ENCR( str, key );
			ENCR( str2, key2 );
			break;
		}

		case E_ACCOUNT_NOT_FOUND:
		{
			/*This email is not registered*/
			BYTE key[4]={0x49,0xf9,0x9e,0xc7};
			BYTE str[29]={0x7f,0x0f,0x40,0x3b,0x3a,0xad,0xed,0xb3,0x83,0xdc,0x90,0x17,0x5e,0xc9,0x20,0xa9,0xa6,0x6e,0x85,0x28,0x60,0xe1,0x33,0x5a,0x89,0xe6,0x0c,0x39,0x5b};
			/*Error*/
			BYTE key2[4]={0x77,0xc6,0x90,0xbd};
			BYTE str2[6]={0xe5,0x15,0x65,0x24,0x81,0xd7};

			DECR( str, key );
			DECR2( str2, key2 );
			MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );
			ENCR( str, key );
			ENCR( str2, key2 );
			break;
		}


		case E_ACCOUNT_NOT_CONFIRMED:
		{
			/*Your email address is registered but not confirmed. Check your email and use confirmation link for account activation or register again.*/
			BYTE key[4]={0x15,0x7a,0xda,0x80};
			BYTE str[137]={0xed,0xb9,0x32,0x5d,0xd4,0x1f,0x52,0x16,0x47,0x40,0xa5,0xec,0x2f,0x43,0x36,0x38,0xbe,0x14,0xe9,0x46,0xff,0x6f,0xb4,0xbd,0xb4,0x81,0xd9,0x3f,0x33,0x13,0x80,0x40,0x36,0xb8,0x95,0xb9,0xb2,0x7c,0x9d,0x8e,0x71,0x15,0xce,0x60,0xf4,0xcc,0xa2,0x85,0x2f,0x8f,0x29,0xfb,0x31,0x7e,0x28,0x84,0xb9,0x32,0x71,0xca,0x88,0xd1,0xa4,0x73,0xef,0x1f,0x75,0x23,0x43,0xb0,0x7e,0x67,0xd2,0x4c,0x92,0x92,0x2a,0x37,0xe4,0xe1,0xbe,0xbd,0xd2,0x32,0x94,0xd0,0x87,0x7f,0x9b,0xb0,0x8b,0xa8,0x70,0xbb,0x51,0x4d,0x04,0xb5,0x9a,0x92,0xee,0x27,0xca,0x03,0x87,0x10,0x2c,0x1d,0xb6,0x2e,0xbd,0x71,0x2c,0xc7,0x68,0xf2,0x54,0x6d,0x58,0x34,0xd4,0x9f,0xd9,0x00,0xf9,0x99,0x99,0xaa,0xc6,0x82,0xe0,0x47,0xee,0x4d,0xc2,0xdb,0xed};

			/*Error*/
			BYTE key2[4]={0x36,0x56,0xab,0x79};
			BYTE str2[6]={0xd7,0x23,0x92,0xac,0x93,0x93};

			DECR( str, key );
			DECR2( str2, key2 );
			MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );
			ENCR( str, key );
			ENCR( str2, key2 );
			break;
		}

		case E_INVALID_CODE:
		{
			/*Invalid registration code*/
			BYTE key[4]={0xb9,0x6f,0xf2,0xa7};
			BYTE str[26]={0xba,0x8e,0x33,0xfe,0xc5,0x67,0x69,0xdb,0x25,0x45,0x9c,0xff,0x38,0x4f,0x45,0xd5,0xeb,0x99,0xc4,0xa0,0xd3,0x1c,0x42,0x2a,0xd9,0x11};
			/*Error*/
			BYTE key2[4]={0xe0,0x44,0x54,0x9f};
			BYTE str2[6]={0xf0,0xfc,0x94,0x23,0xcb,0x49};

			DECR( str, key );
			DECR2( str2, key2 );
			MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );
			ENCR( str, key );
			ENCR( str2, key2 );
			break;
		}

		case E_INVALID_CAPTCHA:
		{
			/*Invalid captcha*/
			BYTE key[4]={0xe9,0x9e,0x07,0xe3};
			BYTE str[16]={0x3a,0x8c,0xf2,0x08,0x4f,0xe2,0x6c,0x02,0xa4,0x11,0x3d,0x57,0x83,0x78,0xa6,0x3e};
			/*Error*/
			BYTE key2[4]={0x41,0x3d,0x24,0xd0};
			BYTE str2[6]={0xf5,0xe2,0xcc,0x60,0x12,0x73};
			DECR( str, key );
			DECR2( str2, key2 );
			MessageBoxA( dlg, ( char* )str, ( char* )str2, MB_ICONERROR );
			ENCR( str, key );
			ENCR( str2, key2 );
			break;
		}
	}
}

// **********************************************************************
//							CAutolicense
// **********************************************************************
CAutolicense::CAutolicense()
{
	m_mgr = new CBaseServerManager;
	m_mgrh = 0;
	m_resp = 0;
	m_type = 0;
	m_rtype = 0;
}

CAutolicense::~CAutolicense()
{
	delete m_mgr;
}

// ===============
// auth
// ===============
int CAutolicense::auth( const char* email, const char* password )
{
	char* uri = ( char* )malloc( 256 );
	char* p = CLogger::urlEncode( password );
	DWORD instances = getInstancesCount();
	DWORD account = g_ac->getNumberI( TRUE );
	
	m_type = 1;

	Globals* g = getGlobals();

/*/autolicense.php?p=auth&email=%s&password=%s&pid=%d&rev=%d&instances=%d&account=%u*/
BYTE key[4]={0xbd,0xbd,0xa8,0x98};
BYTE str[83]={0x5a,0x88,0x43,0xe2,0xf7,0x09,0x1f,0x7a,0x09,0xb7,0x64,0x33,0xd5,0x00,0x61,0xa1,0x0e,0x83,0x34,0xa8,0x87,0xdf,0x25,0x52,0xce,0xea,0xbf,0x21,0x98,0xf6,0xff,0xb8,0x8d,0x20,0xd8,0xc9,0x1c,0xaa,0x54,0x02,0x0a,0xa7,0xbf,0xed,0x0f,0xf9,0x9f,0x57,0xcb,0x56,0x43,0x03,0xfe,0x51,0xfb,0x36,0x35,0xd2,0x39,0x3b,0xcb,0x16,0x5e,0x8c,0x4a,0x6e,0x40,0xa2,0xe4,0xa5,0x9f,0x19,0x28,0x87,0x54,0xd0,0x6f,0xf5,0x36,0xfb,0xbe,0x97,0xc0};

	DECR( str, key );
	strcpy( this->authEmail, email );
	wsprintfA( uri, ( char* )str, email, p, g->getProjectId(), g->getRevision(), instances, account );
	ENCR( str, key );
	free( p );
dbg( "alhu %s", uri );
	char* server = chooseServer( g );
	m_mgrh = m_mgr->load( server, 443, uri, TRUE );
	free( server );
	free( uri );
	return( 0 );
} // auth

// ===============
// reg
// ===============
int CAutolicense::reg( const char* email, const char* password )
{
	char* uri = ( char* )malloc( 512 );
	char* _password = CLogger::urlEncode( password );
	Globals* g = getGlobals();
/*/autolicense.php?p=register&email=%s&password=%s&pid=%d&rev=%d*/
BYTE key[4]={0x63,0x05,0x89,0x75};
BYTE str[63]={0xa3,0x62,0x1d,0xfa,0xf2,0x01,0x65,0xfa,0x86,0xd5,0xe2,0xed,0x99,0xb4,0xeb,0x61,0x98,0xba,0x06,0xf9,0x0e,0x21,0x7e,0x91,0x07,0x2b,0x84,0xbf,0x05,0xc9,0xa7,0x63,0x9f,0xbc,0x23,0x1f,0xed,0xcf,0x5d,0x6c,0x60,0xd3,0xb7,0xd7,0x72,0xee,0xeb,0x9a,0x89,0x15,0xa7,0xc7,0x10,0xab,0x66,0x18,0xd3,0x26,0x15,0x78,0x06,0xb2,0x4b};
	DECR( str, key );
	wsprintfA( uri, ( char* )str, email, _password, g->getProjectId(), g->getRevision() );
	ENCR( str, key );
	free( _password );

	m_type = 2;

	char* server = chooseServer( g );
	m_mgrh = m_mgr->load( server, 443, uri, TRUE );
	free( server );
	free( uri );
	return( 0 );
} // reg

// ===============
// confirm
// ===============
int CAutolicense::confirm( const char* email, const char* code )
{
	char* uri = ( char* )malloc( 512 );
	Globals* g = getGlobals();
/*/autolicense.php?p=confirm&email=%s&code=%s&pid=%d&rev=%d*/
BYTE key[4]={0x9e,0xc5,0x9c,0x68};
BYTE str[58]={0x61,0xf8,0x96,0xd2,0x6a,0x0c,0x9c,0x4f,0x9c,0x01,0xdf,0x9e,0x30,0x34,0xbc,0x58,0x1a,0xb3,0xcc,0xe5,0xc9,0xab,0xf9,0x9c,0x7e,0x4d,0xa0,0x87,0x73,0x55,0x37,0x52,0x33,0xcc,0x82,0x59,0xcd,0x07,0x8a,0x8b,0xd0,0x3c,0xec,0x4a,0x70,0xb4,0x33,0x4f,0xdf,0xbc,0x9b,0x49,0x04,0x70,0xa9,0x61,0x9c,0x53};
	DECR( str, key );
	wsprintfA( uri, ( char* )str, email, code, g->getProjectId(), g->getRevision() );
	ENCR( str, key );

	m_type = 3;

	char* server = chooseServer( g );
	m_mgrh = m_mgr->load( server, 443, uri, TRUE );
	free( server );
	free( uri );
	return( 0 );
} // confirm

// ===================
// sendForgotPassword
// ====================
int CAutolicense::sendForgotPassword( const char* email, const char* password )
{
	char* uri = ( char* )malloc( 512 );
	Globals* g = getGlobals();
	/*/ml/autolicense.php?p=send_forgot_password_email&email=%s&password=%s&sc=%s&pid=%d*/
	BYTE key[4]={0xa4,0x8a,0xfa,0xb0};
	BYTE str[83]={0x37,0x3e,0x25,0xc2,0x9a,0xed,0xbd,0xac,0x8f,0x66,0xd1,0x60,0xac,0x9b,0x9a,0xe1,0x5a,0x8e,0x7b,0x8f,0x06,0x6c,0x21,0xc4,0xa7,0x32,0x37,0x78,0x8b,0xc4,0x35,0x47,0x3e,0xd1,0x9a,0x7d,0xaf,0x21,0x57,0xf9,0x57,0x10,0x86,0x73,0x78,0x29,0x67,0xa7,0xfe,0x07,0x75,0x92,0x30,0x01,0x0c,0x42,0xb2,0x0f,0x08,0xc0,0x03,0x97,0x3f,0xfe,0xff,0x34,0xb2,0x4e,0xd4,0x22,0x39,0x12,0x4e,0xf1,0xa5,0x39,0x9b,0x46,0x94,0x45,0xd2,0x71,0x7c};
	
	/*|mqllocksecret@djfk4$%@|*/
	BYTE keySecret[4]={0x33,0xb1,0x8a,0x76};
	BYTE strSecret[25]={0xcf,0xe2,0xf6,0xf4,0xe4,0xa1,0xc3,0x78,0x10,0xe7,0xee,0xae,0x05,0xf7,0x55,0xe6,0xba,0x28,0x66,0x18,0x3f,0xaf,0x7b,0x9f,0xf0};

	char* sc = ( char* )mymalloc( 512 );
	strcpy( sc, email );
	DECR( strSecret, keySecret );
	strcat( sc, ( const char* )strSecret );
	ENCR( strSecret, keySecret );
	strcat( sc, password );
	
	getMD5( ( BYTE* )sc, strlen( sc ), sc );

	DECR2( str, key );
	wsprintfA( uri, ( char* )str, email, password, sc, getGlobals()->getProjectId() );
	ENCR( str, key );

	free( sc );

	m_type = 4;

	/*mqllock.com*/
	BYTE keyServer[4]={0xbd,0xa1,0x79,0x16};
	BYTE server[12]={0x10,0x27,0x71,0x70,0x3b,0x0b,0xa3,0x10,0x17,0x89,0xfe,0x62};
	DECR2( server, keyServer );
	m_mgrh = m_mgr->load( ( const char* )server, 443, uri, TRUE );
	ENCR( server, keyServer );

	free( uri );
	return( 0 );
} // sendForgotPassword

// ====================
// chooseServer
// =====================
char* CAutolicense::chooseServer( Globals* g )
{
/*//TODO: experiment
char* res = ( char* )malloc( 64 );
strcpy( res, "mqllock.com" );
return( res );
*/
	const char* servers = g->useServers();

	char* res = ( char* )mymalloc( 64 );
	INTERNET_PORT port;
	char baseUri[ 100 ];
	char scheme[ 32 ];

	parseUrl( servers, res, &port, baseUri, scheme );

	g->unuse( servers );
	return( res );
} // chooseServer

// ======================
// readResponse
// ======================
ERROR_TYPE CAutolicense::readResponse()
{
	if( !m_mgrh )
	{
		return( E_CANCELLED );
	}

	BOOL validResp = 0;
dbg( "alh %d %d %d", m_mgrh->status, m_mgrh->error, m_mgrh->cl );
	if( m_mgrh->error == ML_OK && m_mgrh->cl >= AUTOLICENSE_PREFIX_LENGTH + 6 )
	{
dbg( "1" );
		if( m_mgrh->status == 200 )
		{
dbg( "2 %d", m_mgrh->cl );
			DWORD sz;
							
			m_resp = ( char* )mymalloc( m_mgrh->cl );
			if( InternetReadFile( m_mgrh->hr, m_resp, m_mgrh->cl, &sz ) && 
				sz == m_mgrh->cl
			)
			{
dbg( "3" );
				Globals* g = getGlobals();
				const BYTE* lk = g->useLicenseKey();
				CRC4 rc4;
dbg( "alresp1 '%.*s'", m_mgrh->cl, m_resp );
				rc4.Decrypt( ( BYTE* )m_resp, m_mgrh->cl, lk, GEN_LICENSE_KEY_LENGTH );
				g->unuse( ( const char* )lk );
dbg( "alresp2 '%.*s'", m_mgrh->cl, m_resp );
				if( memcmp( m_resp, AUTOLICENSE_PREFIX, AUTOLICENSE_PREFIX_LENGTH ) == 0 )
				{
					if( *( DWORD* )( m_resp + AUTOLICENSE_PREFIX_LENGTH ) == m_mgrh->cl )
					{
						validResp = 1;
					}
				}
dbg( "4" );
			}
		}
	}

	ERROR_TYPE res;
	m_rtype = 0;
	if( validResp )
	{
		BYTE* b = ( BYTE* )m_resp + AUTOLICENSE_PREFIX_LENGTH + 4;
		switch( m_type )
		{
			//auth verification
			case 1:
				switch( *( WORD* )b )
				{
					case 1:
						res = E_OK;
						m_rtype = 1;
						break;

					case 3:
						res = E_ACCOUNT_DISABLED;
						break;

					case 4:
						res = E_ACCOUNT_NOT_CONFIRMED;
						break;

					case 5:
						res = E_USAGE_LIMIT;
						m_rtype = 2;
						break;

					case 6:
						res = E_ACCOUNT_EXPIRED;
						m_rtype = 3;
						break;

					case 7:
						res = E_INVALID_AUTH;
						break;

					default:
						res = E_INVALID_SERVER_RESPONSE;
				}
				break;

			//registration
			case 2:
				switch( *( WORD* )b )
				{
					case 1:	// registration code sent
						res = E_OK;
						break;

					case 8: // email registered and confirmed already
						res = E_REGISTERED_ALREADY;
						break;

					default:
						res = E_INVALID_SERVER_RESPONSE;
				}
				break;

			//confirmation
			case 3:
				switch( *( WORD* )b )
				{
					case 1:	// registration code sent
						res = E_OK;
						break;

					case 9: // invalid confirmation code
						res = E_INVALID_CODE;
						break;

					default:
						res = E_INVALID_SERVER_RESPONSE;
				}
				break;

			//forgot password
			case 4:
				switch( * ( WORD* )b )
				{
					case 1: // confirmation email sent
						res = E_OK;
						break;

					case 7:
						res = E_ACCOUNT_NOT_FOUND;
						break;

					default:
dbg( "conf %d", ( DWORD )( *( WORD* )b ) );
						res = E_INVALID_SERVER_RESPONSE;
				}
				break;
		}
	}
	else
	{
		res = E_INVALID_SERVER_RESPONSE;
	}
	delete m_mgrh;
	m_mgrh = 0;
	return( res );
} // readResponse

// ===================
// getResponseData
// ===================
void CAutolicense::getResponseData( OUT DWORD *maxInstances, OUT DWORD *accType )
{
	if( m_rtype == 1 && m_resp )
	{
		BYTE* b = ( BYTE* )m_resp + AUTOLICENSE_PREFIX_LENGTH + 6;
		*maxInstances = *( DWORD* )( b + 0 );
		*accType = ( DWORD )*( BYTE* )( b + 4 );
		free( m_resp );
		m_resp = 0;
	}
	else
	{
		dbg( "!!-- getResponseData(1)" );
	}
} // getResponseData

// =======================
// getResponseData
// =======================
void CAutolicense::getResponseData( OUT DWORD* usedAccounts, OUT DWORD* usedCids, OUT DWORD* usedInstances,
								   OUT DWORD* maxAccounts, OUT DWORD* maxCids, OUT DWORD* maxInstances )
{
	if( m_rtype == 2 && m_resp )
	{
		BYTE* b = ( BYTE* )m_resp + AUTOLICENSE_PREFIX_LENGTH + 6;
		*usedAccounts = *( DWORD* )( b + 0 );
		*usedCids = *( DWORD* )( b + 4 );
		*usedInstances = getInstancesCount( authEmail );
		*maxAccounts = *( DWORD* )( b + 8 );
		*maxCids = *( DWORD* )( b + 12 );
		*maxInstances = *( DWORD* )( b + 16 );
		free( m_resp );
		m_resp = 0;
	}
	else
	{
		dbg( "!!-- getResponseData(2)" );
	}
} // getResponseData

// ===================
// getResponseData
// ===================
void CAutolicense::getResponseData( OUT DWORD *expiration )
{
	if( m_rtype == 3 && m_resp )
	{
		BYTE* b = ( BYTE* )m_resp + AUTOLICENSE_PREFIX_LENGTH + 6;
		*expiration = *( DWORD* )( b + 0 );
		free( m_resp );
		m_resp = 0;
	}
	else
	{
		dbg( "!!-- getResponseData(3)" );
	}
} // getResponseData


int CAutolicense::isProcessing()
{
	return( m_mgrh != 0 );
}
int CAutolicense::isReady()
{
	return( m_mgrh && m_mgrh->wait( 0 ) == 0 );
}

void bin_to_strhex( const BYTE*bin, unsigned int binsz, char **result)
{
  char          hex_str[]= "0123456789ABCDEF";
  unsigned int  i;

  *result = (char *)malloc(binsz * 2 + 1);
  (*result)[binsz * 2] = 0;

  if (!binsz)
    return;

  for (i = 0; i < binsz; i++)
    {
      (*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
      (*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
    }  
}
