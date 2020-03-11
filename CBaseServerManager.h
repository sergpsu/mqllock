#ifndef __CBASE_SERVER_MANAGER_H__
#define __CBASE_SERVER_MANAGER_H__

#include <wininet.h>
#include <stdlib.h>
#include "api.h"
#include "ml_api.h"


class CBaseServerManagerHandle 
{
	public:
		char			server[ 128 ];	
		char			*uri;			
		int				port;			
		BOOL			ssl;			
		HANDLE			hThread;		
		MLError			error;	//OUT	
		DWORD			status;	//OUT, http status code
		DWORD			cl;		//OUT, content length
		HINTERNET		hi, hc, hr;	
		//BOOL			sendingRequest;
		BOOL			ignoreServerStamp;
		BOOL			aborted;		
		
	public:
		DWORD wait( DWORD limit = INFINITE )
		{
			return( WaitForSingleObject( hThread, limit ) );
		}
		BOOL abort()
		{
			aborted = TRUE;
			if( hi )
			{
				if( hc )
				{
					if( hr )
					{
						InternetCloseHandle( hr );
						hr = 0;
					}
					InternetCloseHandle( hc );
					hc = 0;
				}
				InternetCloseHandle( hi );
				hi = 0;
			}
			return( WaitForSingleObject( hThread, INFINITE ) == 0 );
		}

	public:
		CBaseServerManagerHandle( const char* server, int port, const char* uri, BOOL _ignoreServerStamp, BOOL _ssl )
		{
			lstrcpyA( this->server, server );
			this->port = port;
			ssl = _ssl;
			int n = lstrlenA( uri ) + 1;
			this->uri = ( char* )mymalloc( n );
			memcpy( this->uri, uri, n );

			hThread = 0;
			hr = hc = hi = 0;

			ignoreServerStamp = _ignoreServerStamp;
			aborted = FALSE;
			//sendingRequest = FALSE;
		}

		~CBaseServerManagerHandle()
		{
			free( uri );
			if( hThread )
			{
				CloseHandle( hThread );
			}
			if( hi )
			{
				if( hc )
				{
					if( hr )
					{
						InternetCloseHandle( hr );
					}
					InternetCloseHandle( hc );
				}
				InternetCloseHandle( hi );
			}
		}
};


class CBaseServerManager
{
	private:
		static DWORD send( char* server, char* uri, OUT HINTERNET *hr );
		static DWORD WINAPI loadProc( CBaseServerManagerHandle* );
		static DWORD getGMT();

	public:
		CBaseServerManagerHandle* load( const char* server, int port, const char* uri, BOOL ignoreServerStamp, BOOL ssl = TRUE );

	public:
		//static void cancel( CBaseServerManagerHandle );

	public:
		CBaseServerManager();
};

#endif