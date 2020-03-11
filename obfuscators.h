#pragma once
#define TIME_DATE		1
#define TIME_MINUTES	2
#define TIME_SECONDS	4

struct RandData
{
	HWND hChart;
	int m_z;
};


int StringLen( const TCHAR* text );
int StringFind( const TCHAR* text, const TCHAR* substr );
int StringFind( const TCHAR* text, const TCHAR* substr, int start );
int StringGetChar( const TCHAR* text, int pos );
TCHAR* StringSetChar( const TCHAR* text, int pos, int value );
TCHAR* StringSubstr( const TCHAR* text, int start, int length );
TCHAR* StringSubstr( const TCHAR* text, int start );
TCHAR* CharToStr( int c );
TCHAR* DoubleToStr( double value, int digits );
double StrToDouble( const TCHAR* value );
int StrToInteger( const TCHAR* value );
double NormalizeDouble8( double value );
double NormalizeDouble( double value, int digits );
TCHAR* StringConcatenate( int n, ... );
TCHAR* StringTrimLeft( const TCHAR* str );
TCHAR* StringTrimRight( const TCHAR* str );

DWORD StrToTime( const TCHAR* value );
TCHAR* TimeToStr( DWORD t, DWORD flags );

// ------ non mql ---------
TCHAR* IntToStr( int value );
const TCHAR* BoolToStr( BOOL b );
// ------------------------

double MathAbs( double value );
double MathArccos(double x);
double MathArcsin(double x);
double MathArctan(double x);
double MathCeil(double x);
double MathCos(double value);
double MathExp(double d);
double MathFloor(double x);
double MathLog(double x);
double MathMax( double a, double b );
double MathMin( double a, double b );
void MathSrand( HWND hChart, int seed );
int MathRand( HWND hChart );
double MathRound( double v );
double MathMod( double value, double value2 );
            
double MathPow(double base1, double exponent);
double MathSin(double value);
double MathSqrt(double x);
double MathTan(double x);

int TimeDay( int date );
int TimeDayOfWeek(int date);
int TimeDayOfYear(int date);
int TimeHour(int date);
int TimeMinute(int date);
int TimeMonth(int date);
int TimeSeconds(int date);
int TimeYear(int date);
int TimeLocal();
