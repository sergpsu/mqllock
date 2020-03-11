#include <windows.h>
#include "ml_api.h"
#include "CReg.h"
#include "resource.h"
#include "api.h"
#include "Base64_RC4.h"
#include "CXOR.h"

DWORD g_mt4ServerTimezone;
extern CMemoryManager* g_mm;
CRC4 g_rc4;
CXOR g_xor;

// ========================
// mlapiTryRegister
// ========================
MLError mlapiTryRegister( const char* _server )
{
	CReg reg;
	MLError res;
	char server[ 100 ];
	INTERNET_PORT port;
	char path[ 100 ];

	parseUrl( _server, server, &port, path );

	if( reg.open( REG_KEY_PATH ) == 0 )
	{
		CRegistrationData *rd = new CRegistrationData( server, port, path );
	
		DWORD sz = sizeof( rd->code );
		if( reg.get( "reg_code", rd->code, &sz ) == 0 )
		{
			res = ML_OK;
		}
		else
		{
			if( DialogBoxParam( g_hInstance, MAKEINTRESOURCE( IDD_REGISTRATION1 ), 0, dlgRegister1, ( LPARAM )rd ) == 1 )
			{
				if( DialogBoxParam( g_hInstance, MAKEINTRESOURCE( IDD_REGISTRATION2 ), 0, dlgRegister2, ( LPARAM )rd ) == 1 )
				{
					reg.set( "reg_code", rd->code, REGISTER_CODE_LENGTH, REG_BINARY );
				}
			}
		}
		delete rd;
		reg.close();
	}
	else
	{
		res = ML_REG_OPEN;
	}
	return( res );
} // mlapiTryRegister

// ==================
// mlapiReturnValueI
// ==================
int mlapiReturnValueI( int in )
{
	return( in - 1 + in % 3 );
} // mlapiReturnValueI

// ====================
// mlapiReturnValueS
// ====================
char* mlapiReturnValueS( char* in )
{
	const char replaces[] = { 'A', 'À', 'a', 'à', 'B', 'Â', 'C', 'Ñ', 'c', 'ñ', 'E', 'Å', 'e', 'å', 'H', 'Í', 'I', '²', 'i', '³', 
								'M', 'Ì', 'O', 'Î', 'o', 'î', 'P', 'Ð', 'p', 'ð', 'T', 'Ò', 'X', 'Õ', 'x', 'õ', 0 };

	int len = lstrlenA( in ) + 1;
dbg( "mlapiReturnValueS" );
	char* res = ( char* )g_mm->add( len );
	memcpy( res, in, len );

	int i, j;
	for( i = 0; in[ i ]; i ++ )
	{
		for( j = 0; replaces[ j ]; j += 2 )
		{
			if( res[ i ] == replaces[ j ] )
			{
				res[ i ] = replaces[ j + 1 ];
				break;
			}
		}
	}
	return( res );
} // mlapiReturnValueS

// ===================
// mlapiReturnValueF
// ===================
double mlapiReturnValueF( double in )
{
	return( GetTickCount() % 2 ? in + 0.001 : in - 0.001 );
} // mlapiReturnValueF

// ==================
// mlapiReturnValueB
// ==================
BOOL mlapiReturnValueB( BOOL in )
{
	return( !in );
} // mlapiReturnValueB

// ===============
// mlapiRC4Decrypt
// ===============
char* mlapiRC4Decrypt( const char* str, DWORD len )
{
//dbg( "mlapiRC4Decrypt" );
	char* res = ( char* )g_mm->add( len + 1 );
	memcpy( res, str, len + 1 );
	g_rc4.Decrypt( ( BYTE* )res, len, getGlobals()->getDecryptKey(), getGlobals()->getDecryptKeyLength() );
	return( res );
} // mlapiRC4Decrypt

// ==================
// mlapiXORDecrypt
// ==================
char* mlapiXORDecrypt( const char* str, DWORD len )
{
//dbg( "mlapiXORDecrypt" );
	char* res = ( char* )g_mm->add( len + 1 );
	memcpy( res, str, len + 1 );
	g_xor.Decrypt( ( BYTE* )res, len, getGlobals()->getDecryptKey(), getGlobals()->getDecryptKeyLength() );
//dbg( "decrypted '%s'", res );
	return( res );
} // mlapiXORDecrypt