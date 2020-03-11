#pragma once
#include "atlcoll.h"

class CBaseServerManager;
class CBaseServerManagerHandle;

enum LoggerType
{
	LGT_DLL_INIT = 0,
	LGT_AUTH_OK = 1,
	LGT_CRITICAL_ERROR = 2,
	LGT_AU_OK = 3,
	LGT_AU_CANNOT = 4,
	LGT_AU_CANCELLED = 5,
	LGT_AU_FAILED = 6,
	LGT_INFO = 7
};

class CLogger
{
	private:
		CBaseServerManager *m_mgr;
		CAtlArray<CBaseServerManagerHandle*> m_handles;
		HANDLE m_threadCleanup;
		HANDLE m_hHandles;
		HANDLE m_hDone;
		char m_sessId[ 33 ];

	private:
		static DWORD WINAPI cleanupThread( CLogger* me );
		void cleanup();
		
	public:
		static char* urlEncode( const char *str );	

	public:
		int start();
		int stop();

		int submit( LoggerType t, ... );

	public:
		CLogger();
		~CLogger();
};
