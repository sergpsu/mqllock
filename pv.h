#ifndef __PV_H__
#define __PV_H__

#include <atlcoll.h>

#define ECS( h ) WaitForSingleObject( h, INFINITE )
#define LCS( h ) ReleaseMutex( h )

struct PVRECORD
{
	public:
		enum PVRECORDTYPE
		{
			PV_STRING,
			PV_INTEGER,
			PV_DOUBLE,

			PV_DELETE	//used for deleting values
		};
		union PVRECORDVALUE
		{
			TCHAR*	s;
			int		d;
			double	f;
		};

		PVRECORDTYPE type;
		TCHAR* key;
		PVRECORDVALUE value;

	protected:
		void init( const TCHAR* key );
		void free();

	public:
		PVRECORD& operator=( PVRECORD& pvr );

	public:
		PVRECORD( const TCHAR* key, const TCHAR* value );
		PVRECORD( const TCHAR* key, int value );
		PVRECORD( const TCHAR* key, double value );
		PVRECORD( const TCHAR* key, PVRECORDTYPE type );

		~PVRECORD();
};

class PVDATA
{
	protected:
		HKEY				hkey;
		HANDLE				hstr;	//mutex for strings access
		CSimpleArray<TCHAR*>	strings;
		CAtlArray<PVRECORD*>	queue;
		HANDLE				hqueue; //mutex for queue access
		HANDLE				hQueueThread;//write thread
		HANDLE				hmem;
		BOOL				closing;
		BOOL				memory;

	public:
		int nLastError;

	private:
		void init( BOOL memory );
		int write( PVRECORD* pvr );
		int read( PVRECORD* pvr );
		static DWORD WINAPI queueWriteProc( PVDATA* owner );

	public:
		int write( const TCHAR* key, const TCHAR* value );
		int write( const TCHAR* key, int value );
		int write( const TCHAR* key, double value );
		int read( const TCHAR* key, OUT TCHAR** value );
		int read( const TCHAR* key, OUT int* value );
		int read( const TCHAR* key, OUT double* value );
		int del( const TCHAR* key );
		int count();
		LONG enumerate( DWORD i, OUT TCHAR** name );

		void release();
		int releaseOne( TCHAR *str );
		int close();
		BOOL isMemory();

	public:
		PVDATA();
		PVDATA( HKEY hkey );
		PVDATA( TCHAR* alias );

		~PVDATA();
};

#endif;