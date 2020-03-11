#pragma once

#define REG_KEY_PATH		"Software\\Fx1\\ML"

#define MAX_SERVER_LENGTH	100
#define MAX_URI_LENGTH		256
#define MAX_SCHEME_LENGTH	32


// should be set in DllMain on PROCESS_ATTACH
extern HMODULE g_hInstance;
extern DWORD g_mt4ServerTimezone;

// error codes
enum MLError
{
	ML_OK				= 0,
	ML_REG_OPEN			= -1,			// failed open registry
	ML_INVALID_SERVER	= -2,		// invalid server response
	ML_READ_FILE		= -3,
	ML_SEND_REQUEST		= -4,
	ML_OPEN_URI			= -5,
	ML_CONNECT_SERVER	= -6,
	ML_INTERNET_OPEN	= -7,
	ML_NO_DATA			= -8,
	ML_INVALID_HANDLE	= -9,
	ML_INVALID_PARAMETER = -10,
	ML_TIMEOUT			= -11,
	ML_DATA_LIMIT		= -12,
	ML_DUPLICATE_DATA	= -13,
	ML_INVALID_AUTH		= -14,
	ML_RENAMED			= -15,
	
	ML_UNKNOWN			= -97,
	ML_UNKNOWN2			= -98,	//reserved
	ML_UNKNOWN3			= -99	//reserved
};

// api functions
MLError mlapiTryRegister( const char* server );
int mlapiReturnValueI( int in );
char* mlapiReturnValueS( char* in );
double mlapiReturnValueF( double in );
BOOL mlapiReturnValueB( BOOL in );
char* mlapiRC4Decrypt( const char* str, DWORD len );
char* mlapiXORDecrypt( const char* str, DWORD len );
