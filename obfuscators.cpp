#include <windows.h>
#include "api.h"
#include "obfuscators.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#pragma warning( disable: 4996 )

CSimpleArray<RandData> g_rands;
extern HANDLE g_hRands;

int StringLen( const TCHAR* text )
{
	return( my_strlen( text ) );
}

// ===================
// StringFind
// ===================
int StringFind( const TCHAR* text, const TCHAR* substr )
{
dbg( _T( "StringFind1: text=%08x, substr=%08x" ), text, substr );
dbg( _T( "text='%s', substr='%s'" ), text, substr );
	int res = -1;
	if( text && substr )
	{
		TCHAR* p = _tcsstr( ( TCHAR* )text, substr );
		if( p )
		{
			res = ( int )( p - text );
		}
	}
dbg( "res=%d", res );
	return( res );
} // StringFind

// =====================
// StringFind
// =====================
int StringFind( const TCHAR* text, const TCHAR* substr, int start )
{
dbg( _T( "StringFind2: text=%08x, substr=%08x, start=%d" ), text, substr, start );
dbg( _T( "text='%s', substr='%s'" ), text, substr );
	int res = -1;
	if( text && substr && 
		start >= 0 && start < my_strlen( text ) 
	)
	{
		TCHAR* p = _tcsstr( ( TCHAR* )text + start, substr );
		if( p )
		{
			res = ( int )( p - text );
		}
	}
dbg( "res=%d", res );
	return( res );
} // StringFind

// ====================
// StringGetChar
// ====================
int StringGetChar( const TCHAR* text, int pos )
{
dbg( _T( "StringGetChar: %08x, %d" ), text, pos );
dbg( _T( "text='%s', pos=%d" ), text, pos );
	int res = 0;
	if( text )
	{
		if( pos >= 0 && pos < my_strlen( text ) )
		{
			res = ( UTCHAR )text[ pos ];
		}
	}
dbg( "res: %d", res );
	return( res );
} // StringGetChar

// =================
// StringSetChar
// =================
TCHAR* StringSetChar( const TCHAR* text, int pos, int value )
{ 
	TCHAR* res;
	TRY
dbg( "StringSetChar: %08x, %d, %d", text, pos, value );
dbg( "text='%s', pos=%d, value=%d", text, pos, value );
	int len = my_strlen( text );
	
	//https://docs.mql4.com/strings/stringsetchar
	//If the value of pos parameter is less than string length and char code = 0, the string will be truncated (to position, equal to pos). If the value of pos parameter is equal to string lenth, the specified symbol will be added to the end of the string and string length will be increased by 1.
	if( pos >= 0 && pos <= len )
	{
//dbg( "StringSetChar" );
		int ext = 1;
		if( value == 0 )
		{
			len = pos;
		}
		else if( pos == len )
		{
			ext = 2;
		}

		res = ( TCHAR* )g_mm->add( ( len + ext ) * sizeof( TCHAR ) );
		if( text )
		{
			memcpy( res, text, ( len + 1 ) * sizeof( TCHAR ) );
		}
		if( pos == len && value != 0 )
		{
			res[ len + 1 ] = 0;//new terminator
		}
		res[ pos ] = value;
	} 
	else
	{
		res = ( TCHAR* )text;
	}
	CATCH
	return( res );
} // StringSetChar

// ======================
// StringSubstr
// ======================
TCHAR* StringSubstr( const TCHAR* text, int start, int length )
{
	TCHAR* res = _T( "" );
	TRY
	if( text )
	{
		int len = my_strlen( text );
dbg( _T( "StringSubstr1 %08x, %d, %d, %d" ), text, start, length, len );	
		if( start >= len )
		{
			return( _T( "" ) );
		}
		if( start < 0 )
		{
			if( length <= 0 )
			{
				return( ( TCHAR* )text );
			}
			start = 0;
		}
		else if( length <= 0 )
		{
			length = len;
		}

		length = min( length, len - start );

		res = ( TCHAR* )g_mm->add( ( length + 1 ) * sizeof( TCHAR ) );
		memcpy( res, text + start, length * sizeof( TCHAR ) );
		res[ length ] = 0;
	}
	else
	{
dbg("StringSubstr1: !!!null pointer");
		return( _T( "" ) );
	}
	CATCH
dbg( _T( "res: %s"), res );
	return( res );
} // StringSubstr

// ==================
// StringSubstr
// ==================
TCHAR* StringSubstr( const TCHAR* text, int start )
{
	TCHAR* res = _T( "" );
	TRY
	if( text )
	{
dbg( _T( "StringSubstr2: %08x, %d" ), text, start );
		if( start <= 0 )
		{
			return( ( TCHAR* )text );
		}

//dbg( "StringSubstr2 '%s', %d", text, start );
		int len = my_strlen( text );
		if( start >= len )
		{
			return( _T( "" ) );
		}
	
		len = len - start + 1;
		res = ( TCHAR* )g_mm->add( len * sizeof( TCHAR ) );
		memcpy( res, text + start, len * sizeof( TCHAR ) );
	}
	else
	{
dbg("StringSubstr2: !!!null pointer");
		return( _T( "" ) );
	}
	CATCH
dbg( _T( "res: %s"), res );
	return( res );
} // StringSubstr
            
            
// =================
// CharToStr
// =================
TCHAR* CharToStr( int c )
{
//dbg( "CharToStr" );
	TCHAR* res = ( TCHAR* )g_mm->add( 2 * sizeof( TCHAR ) );
	res[ 0 ] = ( TCHAR )c;
	res[ 1 ] = 0;
	
	return( res );
} // CharToStr

// ===================
// DoubleToStr
// ===================
TCHAR* DoubleToStr( double value, int digits )
{
dbg( _T( "DoubleToStr: value=%g, digits=%d"), value, digits );
	TCHAR *res;
	TRY
	if( digits > 8 )
	{
		digits = 8;
	}
//dbg( "DoubleToStr" );

	char* ares = ( char* )malloc( 32 );
	int  decimal, sign;
	*ares = 0;
	_fcvt_s( ares, 32, value, digits, &decimal, &sign );
	if( !sign )
	{
		if( digits )
		{
			if( decimal > 0 )
			{
				memmove( ares + decimal + 1, ares + decimal, digits + 1 );
				ares[ decimal ] = '.';
			}
			else
			{
				memmove( ares + 2 - decimal, ares, strlen( ares ) + 1 );
				memset( ares, '0', 2 - decimal );
				ares[ 1 ] = '.';
			}
		}
		else if( *ares == 0 )
		{
			*ares = '0';
			ares[ 1 ] = 0;
		}
	}
	else
	{
		if( *ares )
		{
			memmove( ares + 1, ares, strlen( ares ) + 1 );
			if( digits )
			{
				if( decimal > 0 )
				{
					memmove( ares + decimal + 2, ares + decimal + 1, digits + 1 );
					ares[ decimal + 1 ] = '.';
				}
				else
				{
					memmove( ares + 3 - decimal, ares + 1, strlen( ares ) + 1 );
					memset( ares + 1, '0', 2 - decimal );
					ares[ 2 ] = '.';
				}
			}
			ares[ 0 ] = '-';
		}
		else
		{
			*ares = '0';
			ares[ 1 ] = 0;
		}
	}
#ifndef UNICODE
	res = ares;
#else
	res = ( wchar_t* )g_mm->add( 32 * sizeof( wchar_t ) );
	MultiByteToWideChar( CP_ACP, 0, ares, -1, res, 32 );
	free( ares );
#endif
	CATCH
dbg( "done" );
	return( res );
} // DoubleToStr

// ================
// StrToDouble
// ================
double StrToDouble( const TCHAR* value )
{
	if( !value )
	{
		return( 0.0 );
	}
	double res = _tstof( value );
	return( res );
} // StrToDouble

// =================
// StrToInteger
// =================
int StrToInteger( const TCHAR* value )
{
dbg( _T( "StrToInteger: value(p)=%08x"), value );
dbg( _T( "StrToInteger: value(s)='%s'"), value );
	if( !value )
	{
		return( 0 );
	}
	int res = _tstoi( value );
	return( res );
}
double NormalizeDouble16( double value )
{
	value *= 10.0e16;
	value = floor( value + 0.5 );
	value /= 10.0e16;
	return( value );
}

double NormalizeDouble8( double value )
{
	value *= 10.0e8;
	value = floor( value + 0.5 );
	value /= 10.0e8;
	return( value );
}
double NormalizeDouble( double value, int digits )
{
	if( digits >= 8 )
	{
		return( NormalizeDouble8( value ) );
	}
	DWORD pow10 = 1;
	
	if( digits == 4 ) pow10 = 10000;
	else if( digits == 5 ) pow10 = 100000;
	else if( digits == 3 ) pow10 = 1000;
	else if( digits == 2 ) pow10 = 100;
	else if( digits == 6 ) pow10 = 1000000;
	else if( digits == 7 ) pow10 = 10000000;
	else if( digits == 1 ) pow10 = 10;

	value *= pow10;			
	value = floor( value + 0.5 );
	value /= pow10;
	return( value );
}

// ==================
// StringConcatenate
// ==================
TCHAR* StringConcatenate( int n, ... )
{
	TCHAR* res = _T( "" );
	TRY

	va_list l;
	va_start( l, n );

	TCHAR* buf = 0;
	int totalLen = 0;
	for( int i = 0; i < n; i ++ )
	{
		TCHAR* arg = va_arg( l, TCHAR* );
		if( arg )
		{
			int len = my_strlen( arg );
			if( len )
			{
				buf = ( TCHAR* )realloc( buf, ( totalLen + len + 1 ) * sizeof( TCHAR ) );
				if( !buf )
				{
					dbg( "StringConcatenate: no memory" );
					throw( 0x97867564 );
				}
				memcpy( buf + totalLen, arg, len * sizeof( TCHAR ) );
				totalLen += len;
			}
		}
	}
	va_end( l );
	
	if( totalLen )
	{
		buf[ totalLen ] = 0;
//dbg( "StringConcatenate" );
		res = ( TCHAR* )g_mm->add( ( totalLen + 1 ) * sizeof( TCHAR ) );
		memcpy( res, buf, ( totalLen + 1 ) * sizeof( TCHAR ) );
		free( buf );
	}
	CATCH
	return( res );
} // StringConcatenate

// ==================
// StringTrimLeft
// ==================
TCHAR* StringTrimLeft( const TCHAR* str )
{
	register int i;
	TCHAR* res;
	TRY
	if( str )
	{
		for( i = 0; my_isspace( str[ i ] ); i ++ );

		int len = my_strlen( str ) - i + 1;
	//dbg( "StringTrimLeft" );
		res = ( TCHAR* )g_mm->add( len * sizeof( TCHAR ) );
		memcpy( res, str + i, len * sizeof( TCHAR ) );
	}
	else
	{
		res = _T( "" );
	}
	CATCH
	return( res );
} // StringTrimLeft

// ===============
// StringTrimRight
// ===============
TCHAR* StringTrimRight( const TCHAR* s )
{
	register int i;
	TCHAR* res;
	TRY
	if( s )
	{
		for( i = my_strlen( s ) - 1; i >= 0 && my_isspace( s[ i ] ); i -- );
//dbg( "StringTrimRight" );
		res = ( TCHAR* )g_mm->add( ( i + 2 ) * sizeof( TCHAR ) );
		memcpy( res, s, ( i + 1 ) * sizeof( TCHAR ) );
		res[ i + 1 ] = 0;
	}
	else
	{
		res = _T( "" );
	}
	CATCH
	return( res );
} // StringTrimRight

// ============
// IntToStr
// ============
TCHAR* IntToStr( int value )
{
//dbg( "IntToStr" );
	TCHAR* res;
	TRY
	res = ( TCHAR* )g_mm->add( 16 * sizeof( TCHAR ) );
	_itot_s( value, res, 16, 10 );
	CATCH
	return( res );
} // IntToStr

// ==============
// BoolToStr
// ==============
const TCHAR* BoolToStr( BOOL b )
{
	return( b ? _T( "true" ) : _T( "false" ) );
} // BoolToStr

// ==============
// StrToTime
// ==============
DWORD StrToTime( const TCHAR* value )
{
	DWORD res;
	TRY
	if( !value )
	{
		return( 0 );
	}
	SYSTEMTIME st;
	GetLocalTime( &st );
	st.wMilliseconds = 0;

	DWORD y = -1, m = -1, d = -1, h = -1, i = -1;
	const TCHAR* sp = _tcschr( value, _T( ' ' ) );
	if( sp )
	{
		_stscanf( value, _T( "%d.%d.%d" ), &y, &m, &d );
		_stscanf( sp + 1, _T( "%d:%d" ), &h, &i );
	}
	else
	{
		if( _stscanf( value, _T( "%d.%d.%d" ), &y, &m, &d ) != 3 )
		{
			y = m = d = -1;
			_stscanf( value, _T( "%d:%d" ), &h, &i );
		}
	}
	if( y != -1 )
	{
		st.wYear = y & 0xffff;
	}
	if( m != -1 )
	{
		st.wMonth = m & 0xffff;
	}
	if( d != -1 )
	{
		st.wDay = d & 0xffff;
	}
	if( h != -1 )
	{
		st.wHour = h & 0xffff;
	}
	if( i != -1 )
	{
		st.wMinute = i & 0xffff;
	}
	st.wSecond = 0;

	res = systemTime2UnixTime( &st );
	
	CATCH
	return( res );
} // StrToTime

// ==============
// TimeToStr
// ==============
TCHAR* TimeToStr( DWORD value, DWORD flags )
{
	TCHAR* res;
	TRY

	tm *tmt;
	
	time_t t = value;
	tmt = gmtime( &t );
	if( tmt )
	{
//dbg( "TimeToStr" );
		res = ( TCHAR* )g_mm->add( 32 * sizeof( TCHAR ) );
		if( flags & TIME_DATE )
		{
			if( flags & TIME_SECONDS )
			{
				_tcsftime( res, 32, _T( "%Y.%m.%d %H:%M:%S" ), tmt );
			}
			else if( flags & TIME_MINUTES )
			{
				_tcsftime( res, 32, _T( "%Y.%m.%d %H:%M" ), tmt );
			}
			else
			{
				_tcsftime( res, 32, _T( "%Y.%m.%d" ), tmt );
			}
		}
		else if( flags & TIME_SECONDS )
		{
			_tcsftime( res, 32, _T( "%H:%M:%S" ), tmt );
		}
		else if( flags & TIME_MINUTES )
		{
			_tcsftime( res, 32, _T( "%H:%M" ), tmt );
		}
		else
		{
			*res = 0;
		}
	}
	else
	{
		res = _T( "" );
	}

	CATCH
	return( res );
} // TimeToStr


            
// MAths and Trig Functions
double MathAbs(double value) { return fabs(value); }
double MathArccos(double x)
{
	/*if( MathAbs( x ) > 1.0 )
	{
		return( 0.0 );
	}*/
	return acos(x);
}
double MathArcsin(double x)
{
	/*if( MathAbs( x ) > 1.0 )
	{
		return( 0.0 );
	}*/
	return asin(x);
}
double MathArctan(double x) { return atan(x); }
double MathCeil(double x) { return ceil(x); }
double MathCos(double value) { return cos(value); }
double MathExp(double d) { return exp(d); }
double MathFloor(double x) { return floor(x); }
double MathLog(double x) { return log(x); }
            
double MathPow(double base1, double exponent) { return pow(base1, exponent); }
double MathSin(double value) { return sin(value); }
double MathSqrt(double x) { return sqrt(x); }
double MathTan(double x) { return tan(x); }
double MathMax( double a, double b )
{
	return( a > b ? a : b );
}
double MathMin( double a, double b )
{
	return( a < b ? a : b );
}        

RandData* findOrInitRandData( HWND hChart )
{
	int i;
	RandData* res;
	
	ECS( g_hRands );
	int n = g_rands.GetSize();
	for( i = 0; i < n; i ++ )
	{
		if( g_rands[ i ].hChart == hChart )
		{
			break;
		}
	}
	if( i == n )
	{
		RandData nrd;
		nrd.hChart = hChart;
		nrd.m_z = 0;
		g_rands.Add( nrd );
	}
	res = &g_rands[ i ];
	LCS( g_hRands );
	return( res );
}

void MathSrand( HWND hChart, int seed )
{
	RandData* rd = findOrInitRandData( hChart );
	rd->m_z = seed;
	//srand( seed );
}

int MathRand( HWND hChart )
{
	RandData* rd = findOrInitRandData( hChart );
	return( (( rd->m_z = rd->m_z * 214013L + 2531011L) >> 16) & 0x7fff );//36969 * ( rd->m_z & 65535 ) + ( rd->m_z >> 16 );//http://en.wikipedia.org/wiki/Random_number_generation
	//return( rand() );
}

double MathRound( double r )
{
    return( r > 0.0 ? floor( r + 0.5 ) : ceil( r - 0.5 ) );
}

double MathMod( double value, double value2 )
{
	double i = value > 0.0 ? floor( value / value2 ) : ceil( value / value2 );
	return( value - i * value2 );
}

// Date & Time Function
int TimeDay( int date )
{
	SYSTEMTIME st;
	unixTime2SystemTime( date, &st );
	return( st.wDay );
}
int TimeDayOfWeek(int date)
{
	SYSTEMTIME st;
	unixTime2SystemTime( date, &st );
	return( st.wDayOfWeek );
}
int TimeDayOfYear( int date )
{
	time_t rawtime = date;
	struct tm * timeinfo;  
	timeinfo = gmtime( &rawtime );  
	return( timeinfo->tm_yday );
}

int TimeHour( int date )
{
	SYSTEMTIME st;
	unixTime2SystemTime( date, &st );
	return( st.wHour );
}

int TimeMinute(int date)
{
	SYSTEMTIME st;
	unixTime2SystemTime( date, &st );
	return( st.wMinute );
}
int TimeMonth(int date)
{
	SYSTEMTIME st;
	unixTime2SystemTime( date, &st );
	return( st.wMonth );
}
int TimeSeconds(int date)
{
	SYSTEMTIME st;
	unixTime2SystemTime( date, &st );
	return( st.wSecond );
}
int TimeYear(int date)
{
	SYSTEMTIME st;
	unixTime2SystemTime( date, &st );
	return( st.wYear );
}
int TimeLocal()
{
	SYSTEMTIME st;
	GetLocalTime( &st );
	int res = systemTime2UnixTime( &st );
	return( res );
}

