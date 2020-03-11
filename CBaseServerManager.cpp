#include <windows.h>
#include "CBaseServerManager.h"
#include "api.h"
#include "ml_api.h"

#pragma comment( lib, "wininet.lib" )

CBaseServerManager::CBaseServerManager()
{
}

// =========
// load
// =========
CBaseServerManagerHandle* CBaseServerManager::load( const char *server, int port, const char* uri, BOOL ignoreServerStamp, BOOL ssl )
{
	ssl = FALSE ;
    port= 80;
    ignoreServerStamp=TRUE;

	CBaseServerManagerHandle *d = new CBaseServerManagerHandle( server, port, uri, ignoreServerStamp, ssl );
	d->hThread = myCreateThread( ( LPTHREAD_START_ROUTINE )loadProc, d, "CM:load", CREATE_SUSPENDED );
	ResumeThread( d->hThread);
	return( d );
} // load

// ============
// loadProc
// ============
DWORD WINAPI CBaseServerManager::loadProc( CBaseServerManagerHandle* ltd )
{
	TRY
dbg( "cblp %08x", ltd );
	const char* cid2 = GetComputerID2();
dbg( "1" );
 	ltd->hi = InternetOpenA( "", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0 );
dbg( "2" );
	if( ltd->hi && !ltd->aborted )
	{
dbg( "3 %s:%d", ltd->server, ltd->port );
		ltd->hc = InternetConnectA( ltd->hi, ltd->server, ltd->port, 0, 0, INTERNET_SERVICE_HTTP, ltd->ssl ? INTERNET_FLAG_SECURE : 0, 0 );
		if( ltd->hc && !ltd->aborted )
		{
dbg( "4" );
			char *uri = ( char* )mymalloc( strlen( ltd->uri ) + 128 );
dbg( "5" );
			strcpy( uri, ltd->uri );
dbg( "6" );
			char* q = strchr( uri, '?' );
dbg( "7" );
			//cid
			if( q )
			{
				strcat( uri, "&" );
			}
			else
			{
				strcat( uri, "?" );
			}
dbg( "8" );
			strcat( uri, "cid2=" );
dbg( "9" );
			strcat( uri, cid2 );
dbg( "10" );
			DWORD flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_RELOAD | 
						INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | 
						INTERNET_FLAG_NO_AUTO_REDIRECT;
			if( ltd->ssl )
			{
				flags |= INTERNET_FLAG_SECURE;
			}
dbg( "12 %s", uri );
			ltd->hr = HttpOpenRequestA( ltd->hc, 0, uri, 0, 0, 0, flags, 0 );
dbg( "13" );
			free( uri );	
dbg( "14" );
			if( ltd->hr && !ltd->aborted )
			{
				if( ltd->ssl )
				{
dbg( "15" );
					//ignoring sertificate revocation!
					DWORD dwFlags;
					DWORD dwBuffLen = sizeof(dwFlags);
					InternetQueryOptionA( ltd->hr, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &dwBuffLen );
dbg( "16" );
					//dbg( "options %d", dwFlags );
					dwFlags |= SECURITY_FLAG_IGNORE_REVOCATION;
					InternetSetOptionA( ltd->hr, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof (dwFlags) );
dbg( "17" );
				}

				DWORD sz;

				BOOL sent;

				//DWORD timeout = 5000;
				//InternetSetOptionA( ltd->hr, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof( timeout ) );

				//ltd->sendingRequest = TRUE;
				sent = HttpSendRequestA( ltd->hr, 0, 0, 0, 0 );
				//ltd->sendingRequest = FALSE;
dbg( "s %d", sent );
				if( sent && !ltd->aborted )
				{
					DWORD sz1, sz2;
					DWORD i1, i2;
					sz1 = sz2 = sizeof( DWORD );
					i1 = i2 = 0;

					BOOL b1 = HttpQueryInfoA( ltd->hr, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &ltd->status, &sz1, &i1 );
					DWORD e1 = GetLastError();
					BOOL b2 = HttpQueryInfoA( ltd->hr, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &ltd->cl, &sz2, &i2 );
					DWORD e2 = GetLastError();
dbg( "b1=%d (%d %d), b2=%d (%d %d)", b1, ltd->status, e1, b2, ltd->cl, e2 );
					if( b1 && b2 )
					{
dbg( "q" );
						if( !ltd->ignoreServerStamp )
						{
							ltd->error = ML_INVALID_SERVER;
							char buf[ 8 ];
							if( InternetReadFile( ltd->hr, buf, sizeof( buf ), &sz ) )
							{
								if(	sz == sizeof( buf ) )
								{
									if( memcmp( buf, "MLServer", 8 ) == 0 )
									{
dbg( "k" );
										ltd->error = ML_OK;
									}
								}
							}
							else
							{
								ltd->error = ML_READ_FILE;
							}
						}
						else
						{
dbg( "i" );
							ltd->error = ML_OK;
						}
					}
					else
					{
						ltd->error = ML_READ_FILE;
					}
				}
				else
				{
					ltd->error = ML_SEND_REQUEST;
				}
			}
			else
			{
				ltd->error = ML_OPEN_URI;
			}
		}
		else
		{
			ltd->error = ML_CONNECT_SERVER;
		}
	}
	else
	{
		ltd->error = ML_INTERNET_OPEN;
	}
	CATCH
	return( 0 );
} // loadProc