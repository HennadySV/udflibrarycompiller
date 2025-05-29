#ifndef FSFBUDF_EXPORTS
#define FSFBUDF_EXPORTS
#endif

//��������� �������� � "������������" ��������
#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "ibase.h"
#include "ib_util.h"
#include "FsFBUDF.h"
//���������� ���������� ib_util
#pragma comment(lib, "ib_util_ms.lib")
#pragma comment(lib, "fbclient_ms.lib")

//������� ��� ��������� PARAMDSC (��������� �� ���������� ����������, ������������ Firebird'��)
//�� ��������� ������������� ��� ���������� ����� ��������� ������ �� ���������� � ������
//�� ���� ���� DOUBLE PRECISION ���� NUMERIC
bool IsNull( const PARAMDSC *X )
{
	if ( ( !X ) || ( !X->dsc_address ) || ( X->dsc_flags & DSC_null) ) return true;
	else return false;
}

ISC_INT64 StrToNumber( const ISC_UCHAR *iStr, ISC_SHORT &oScale )
{
	const ISC_UCHAR *vStr = iStr;
	ISC_UCHAR *vIntStr = (ISC_UCHAR *)malloc( strlen( (ISC_SCHAR *)iStr ) );
	ISC_USHORT vInd;
	ISC_INT64 vBuf = 0; //����������� ����� ���������� � ���� �������� ����� ������������� vIntStr
	if (vStr[0] == '-')
	{
		vIntStr[0] = '-';
		vInd = 1;
		++vStr;
	}
	else
	{
		vInd = 0;
	}
	oScale = -1;
	while ( *vStr ) // ���� �� ���������������
	{
		if ( ( *vStr >= '0' ) && ( *vStr <= '9' ) ) //���� �������� �����
		{
			if ( oScale >= 0 ) ++oScale; //���������� ���-�� ������ ����� �������, ���� ������� ��� ���������
			vIntStr[vInd] = *vStr; //������������ ����� � ������ ��� ��������������
			++vInd;
		}
		else if ( ( ( *vStr == '.' ) || ( *vStr == ',' ) ) && ( oScale == -1 ) ) //���� ������ �� ����������� � �� � ������ ���
		{
			++oScale;
		}
		else if ( *vStr > 32)//������� ������������ ������ � � ���� ����� �� �����������, ������ ����� ������
		{
			vInd = 0; // ��������� ������� ������������ ��������, � ���������� vIntStr ��� ������ �� ����� ��������
			oScale = 0;
			break;
		}
		++vStr; //���������� ���������
	}
	if ( oScale == -1 ) oScale = 0; //���� ������� ��� � �� ��������� �� �������� 0
	vIntStr[vInd] = '\0'; // ����������� ������
	vBuf = _atoi64( (const char *) vIntStr );
	free( vIntStr );
	oScale *= -1;
	return vBuf;
}

//������� ����������� INT64+iScale � ������� ������ � ������. ��������� ���������� �� ������ ����������
ISC_SCHAR *NumberToStr( ISC_SCHAR *oString, ISC_INT64 iNumber, ISC_SHORT iScale )
{
	ISC_SHORT vSign = 1;
	if ( iNumber < 0 ) vSign = -1;
	iNumber *= vSign;
	ISC_INT64 vPow = 10;
	while ( ( iNumber % vPow == 0 ) && ( iScale > 1 ) ) //������� ��� '0' � �����, ����� 1 - �� ����� ����� �������
	{
 		iScale -= 1;
		iNumber /= 10;
	}
	vPow = pow( (double)10, (double)iScale );
	ISC_INT64 vMod = iNumber / vPow; //����� �����
	ISC_INT64 vFract = iNumber % vPow; //�������
	ISC_SCHAR *vBufStr = (ISC_SCHAR *)malloc(21);
	if ( vSign < 0) sprintf(vBufStr, "-%lli.%0*lli", vMod, iScale, vFract);
	else sprintf(vBufStr, "%lli.%0*lli", vMod, iScale, vFract);
	sprintf( oString, "%.20s", vBufStr ); //�������� �� 20 ��������
	free( vBufStr );
	return oString;
}


//���� � ��� ����� ��� ����� � ������ ��������� � INT64 � �������� �� ������, 
//��� �������� �������� ������� � ���������, � ����� �������� � NUMERIC
//���������� ���������� ��� ������ ������� � ��������������� �������� ���������
struct vvary
{
	ISC_USHORT	vary_length;
	ISC_UCHAR	vary_string[65000];
};

ISC_INT64 DescToInt( const PARAMDSC *iIn, ISC_SHORT &oScale )
{
	ISC_SHORT vInd = 0;
	ISC_INT64 vBufInt64 = 0; //�����, ����� ����� ��� ��������� ����������� �� ������� ���������
	ISC_UCHAR* vStr;
	double vBufDouble = 0.0;
	ISC_SCHAR *vBufChar = (char *)malloc(64);
	if ( IsNull( iIn ) ) return 0;
	switch ( iIn->dsc_dtype )
	{
	case dtype_text: //�������������� ������
		vBufInt64 = StrToNumber( iIn->dsc_address, oScale );
		break;
	case dtype_cstring: //�������������� ������
		vBufInt64 = StrToNumber( iIn->dsc_address, oScale );
		break;
	case dtype_varying: //�������������� varchar
		vStr = reinterpret_cast<vvary*>(iIn->dsc_address)->vary_string;	
		vBufInt64 = StrToNumber( vStr, oScale );
		break;
	case dtype_short: //2-� ������� ���
		vBufInt64 = *reinterpret_cast<ISC_SHORT*>( iIn->dsc_address );
		oScale = iIn->dsc_scale;
		break;
	case dtype_long: //4-� ������� ���
		vBufInt64 = *reinterpret_cast<ISC_LONG*>( iIn->dsc_address );
		oScale = iIn->dsc_scale;
		break;
	case dtype_int64:  //8-������� ���
		vBufInt64 = *reinterpret_cast<ISC_INT64*>( iIn->dsc_address );
		oScale = iIn->dsc_scale;
		break;
	case dtype_real:
		vBufDouble = static_cast<double>( *reinterpret_cast<float*>( iIn->dsc_address ) );
		sprintf( vBufChar, "%f", vBufDouble);
		vBufInt64 = StrToNumber( (const ISC_UCHAR*) vBufChar, oScale );
		break;
	case dtype_double:
		vBufDouble = *reinterpret_cast<double*>( iIn->dsc_address );
		sprintf( vBufChar, "%f", vBufDouble);
		vBufInt64 = StrToNumber( (const ISC_UCHAR*) vBufChar, oScale );
		break;
	default:
		return 0;
		break;
	}
	free (vBufChar);
	if (oScale < 0) oScale *= -1; //�������� ��������� ������ �������� � �������� ������
	return vBufInt64;
}


//������� � ��������� ����������� ��� 1/3 ��������� Firebird
#ifdef FB_DIALECT3
const ISC_SHORT vNumericScale = 7; //����������� NUMERIC ������������ � 3-� �������� 

//���� ������ � Numeric � ��� ����� ��� ����� ��������� � int64 ��� ���� ��� �����������
//�� �������� �������� �� �� ������ �������. ����� �� ������ ����������� ������� �� ������� �������� 
//�� �� �������� � �����������. 
//�� ���� (��� ��������� �������� NUMERIC = 7) ����� 
//123.1234567 = 1231234567
//123.123 = 1231230000
//123.123456789123=1231234567
ISC_INT64 NumericToInt( const PARAMDSC *iIn )
{
	ISC_SHORT vScale = 0;
	ISC_INT64 result = DescToInt( iIn, vScale );
	//�������� ����� � ��������� ������� ��������
	ISC_SHORT vInd = 0;
	for (vInd = 1; vScale - vInd >= vNumericScale; ++vInd) result /= 10; //��������� �� ���������
	for (vInd = 1; vScale + vInd <= vNumericScale; ++vInd) result *= 10; //������� ����� ���������
	return result;
}

//��� ������� ������������ ������ � 3-� �������� ��� ������ � NUMERIC
PARAMDSC* IntToNumeric( PARAMDSC *oOut, ISC_INT64 iInt64)
{
	oOut->dsc_dtype = dtype_int64;
	oOut->dsc_scale = -7;
	oOut->dsc_length = 8;
	oOut->dsc_flags = 0;
	*reinterpret_cast<ISC_INT64*>(oOut->dsc_address) = iInt64;
	return oOut;
}

UDFEXPORT PARAMDSC* _abs( const PARAMDSC *iIn, PARAMDSC *oOut )
{
	ISC_INT64 vBuf;
	vBuf = NumericToInt( iIn );
	if ( vBuf < 0 ) vBuf *= -1;
	return IntToNumeric( oOut, vBuf );
}

UDFEXPORT PARAMDSC* _floor( const PARAMDSC *iIn, PARAMDSC *oOut )
{
	ISC_INT64 vBuf;
	ISC_INT64 vMod;
	vBuf = NumericToInt( iIn );
	if ( vBuf < 0 ) vBuf -=10000000;
	vMod = vBuf % 10000000;
	vBuf -= vMod;
	return IntToNumeric( oOut, vBuf );
}

UDFEXPORT PARAMDSC *Round( const PARAMDSC *iIn, ISC_LONG *iPrecision, PARAMDSC *oOut )
{
	ISC_INT64 vBuf = NumericToInt( iIn );
	ISC_SHORT vSign = (vBuf < 0 ? -1 : 1);
	double vPower = pow( (double)10, (double)( ( vNumericScale - 1 ) - *iPrecision ) );
	vBuf *= vSign; //������� ������ �����, ���� ����� ���� ���������
	vBuf = vBuf + vPower * 5; //������� 0,5 � ���������� ��������� ������� ����������
	vPower *= 10; //����� ��������� (��������� ��������) ������
	vBuf /= vPower; //������ ����������� ��� ���������� �������
	vBuf *= vPower; //��������������� �����������
	vBuf *= vSign; //���������� ����
	return IntToNumeric( oOut, vBuf );
}

UDFEXPORT PARAMDSC *max_d( const PARAMDSC *iLeft, const PARAMDSC *iRight, PARAMDSC *oOut )
{
	ISC_INT64 vLeft = NumericToInt( iLeft );
	ISC_INT64 vRight = NumericToInt( iRight );
	return IntToNumeric( oOut, vLeft > vRight ? vLeft : vRight );
}

UDFEXPORT PARAMDSC *min_d( const PARAMDSC *iLeft, const PARAMDSC *iRight, PARAMDSC *oOut )
{
	ISC_INT64 vLeft = NumericToInt( iLeft );
	ISC_INT64 vRight = NumericToInt( iRight );
	return IntToNumeric( oOut, vLeft < vRight ? vLeft : vRight );
}

UDFEXPORT PARAMDSC *AsFloat( const PARAMDSC *iIn, PARAMDSC *oOut )
{
	ISC_SHORT vScale = 0;
	ISC_INT64 vBufI = NumericToInt( iIn );
	return IntToNumeric( oOut, vBufI );
}


UDFEXPORT void *iif_d( const PARAMDSC *iLeft, ISC_SCHAR *Op, const PARAMDSC *iRight, void *iTrue, void *iFalse )
{
	ISC_INT64 vLeft = NumericToInt( iLeft );
	ISC_INT64 vRight = NumericToInt( iRight );
	if ( strcmp( Op, "=" ) == 0 && vLeft == vRight
		|| strcmp( Op, ">" ) == 0 && vLeft > vRight
		|| strcmp( Op, "<" ) == 0 && vLeft < vRight
		|| strcmp( Op, "=>" ) == 0 && vLeft >= vRight
		|| strcmp( Op, ">=" ) == 0 && vLeft >= vRight
		|| strcmp( Op, "=<" ) == 0 && vLeft <= vRight
		|| strcmp( Op, "<=" ) == 0 && vLeft <= vRight
		|| strcmp( Op, "<>" ) == 0 && vLeft != vRight
		|| strcmp( Op, "#" ) == 0 && vLeft != vRight )
		return iTrue;
	else 
		return iFalse;
}
#else
//������� ����������� DOUBLE � Int64 � ��������� ��������
UDFEXPORT double *_abs( double *X )
{
	if ( *X < 0 ) *X = -*X;
	return X;
}

UDFEXPORT double *_floor( double *X )
{
	if ( X >= 0 ) 
		modf( *X, X );
	else          
		modf( *X - 1, X );
	return X;
}

UDFEXPORT double Round( double *Value, ISC_LONG *Precision )
{
	double X, Power, ProcFix;
	Power = pow( (double)10, (double)*Precision );
	X = (*Value ) * Power;
	ProcFix = X / pow( (double)10, (double)13); //��������� ���� �������� ��� ������ ���������� ����������
  //ProcFix = X / pow( (double)10, (double)12); //TVA 23.11.2021: �� ��������� 91184754.18491 �� .18
  //ProcFix = X / pow( (double)10, (double)15); //TVA 11.9.2020: �� ��������� 0.324999999999999 �� 0.33
  //ProcFix = X / pow( (double)10, (double)16); //TVA 27.8.2020: �� �������� f_Round(cast(583.935 as double precision) * 77941, 2)
	if ( abs( ProcFix ) < 1 / Power ) X += ProcFix; //���� �������� ����� �� ������� �� ������� ��������, �� ������ ����
	if ( X >= 0 )
	{
		modf( X+0.5, &X );
	}
	else
	{
		modf( X-0.5, &X );
	}
	return X / Power;
}

UDFEXPORT double max_d( double *Left, double *Right )
{
	return *Left > *Right ? *Left : *Right;
}

UDFEXPORT double min_d( double *Left, double *Right )
{
	return *Left < *Right ? *Left : *Right;
}

UDFEXPORT double AsFloat( ISC_SCHAR *iString )
{
	return atof( iString );
}

UDFEXPORT void *iif_d( double *Left, ISC_SCHAR *Op, double *Right, void *iTrue, void *iFalse )
{
	if ( strcmp( Op, "=" ) == 0 && *Left == *Right
		|| strcmp( Op, ">" ) == 0 && *Left > *Right
		|| strcmp( Op, "<" ) == 0 && *Left < *Right
		|| strcmp( Op, "=>" ) == 0 && *Left >= *Right
		|| strcmp( Op, ">=" ) == 0 && *Left >= *Right
		|| strcmp( Op, "=<" ) == 0 && *Left <= *Right
		|| strcmp( Op, "<=" ) == 0 && *Left <= *Right
		|| strcmp( Op, "<>" ) == 0 && *Left != *Right
		|| strcmp( Op, "#" ) == 0 && *Left != *Right )
		return iTrue;
	else 
		return iFalse;
}
#endif


//�������������� �������
UDFEXPORT double _Power( double *Value, double *Precision )
{
	return pow( *Value, *Precision );
}

UDFEXPORT double _Exp( double *Value )
{
	return exp( *Value );
}

//������� �� ������ � ����������
UDFEXPORT ISC_LONG SetIn( ISC_LONG *BitSet, ISC_LONG *Item )
{
	if ( ( *BitSet >> *Item ) & 1 )
		return 1;
	else
		return 0;
}

UDFEXPORT ISC_LONG SetPlus( ISC_LONG *XSet, ISC_LONG *YSet )
{
	return *XSet | *YSet;
}

UDFEXPORT ISC_LONG SetMinus( ISC_LONG *XSet, ISC_LONG *YSet )
{
	return *XSet & ~*YSet;
}

UDFEXPORT ISC_LONG SetMult( ISC_LONG *XSet, ISC_LONG *YSet )
{
	return *XSet & *YSet;
}

UDFEXPORT ISC_LONG SetIncl( ISC_LONG *BitSet, ISC_LONG *Item )
{
	return *BitSet | (1 << *Item);
}

UDFEXPORT ISC_LONG SetExcl( ISC_LONG *BitSet, ISC_LONG *Item )
{
	return *BitSet & ~(1 << *Item);
}

UDFEXPORT ISC_LONG SetEmpty()
{
	return 0;
}

UDFEXPORT ISC_LONG SetFull()
{
	return -1;
}

//������� ���������
UDFEXPORT ISC_LONG max_i( ISC_LONG *Left, ISC_LONG *Right )
{
	return *Left > *Right ? *Left : *Right;
}

UDFEXPORT ISC_LONG min_i( ISC_LONG *Left, ISC_LONG *Right )
{
	return *Left < *Right ? *Left : *Right;
}

UDFEXPORT ISC_SCHAR *max_s( ISC_SCHAR *Left, ISC_SCHAR *Right )
{
	return strcmp( Left, Right ) > 0 ? Left : Right;
}

UDFEXPORT ISC_SCHAR *min_s( ISC_SCHAR *Left, ISC_SCHAR *Right )
{
	return strcmp( Left, Right ) < 0 ? Left : Right;
}

UDFEXPORT ISC_QUAD *max_t( ISC_QUAD *Left, ISC_QUAD *Right )
{
	if ( Left->gds_quad_high > Right->gds_quad_high ||
         Left->gds_quad_high == Right->gds_quad_high && Left->gds_quad_low > Right->gds_quad_low )
		return Left;
	else
		return Right;
}

UDFEXPORT ISC_QUAD *min_t( ISC_QUAD *Left, ISC_QUAD *Right )
{
	if ( Left->gds_quad_high < Right->gds_quad_high ||
         Left->gds_quad_high == Right->gds_quad_high && Left->gds_quad_low < Right->gds_quad_low )
		return Left;
	else
		return Right;
}

//IIF �������
UDFEXPORT void *iif_i( ISC_LONG *Left, ISC_SCHAR *Op, ISC_LONG *Right, void *iTrue, void *iFalse )
{
	if ( strcmp( Op, "=" ) == 0 && *Left == *Right
		|| strcmp( Op, ">" ) == 0 && *Left > *Right
		|| strcmp( Op, "<" ) == 0 && *Left < *Right
		|| strcmp( Op, "=>" ) == 0 && *Left >= *Right
		|| strcmp( Op, ">=" ) == 0 && *Left >= *Right
		|| strcmp( Op, "=<" ) == 0 && *Left <= *Right
		|| strcmp( Op, "<=" ) == 0 && *Left <= *Right
		|| strcmp( Op, "<>" ) == 0 && *Left != *Right
		|| strcmp( Op, "#" ) == 0 && *Left != *Right )
		return iTrue;
	else 
		return iFalse;
}

UDFEXPORT void *iif_s( ISC_SCHAR *Left, ISC_SCHAR *Op, ISC_SCHAR *Right, void *iTrue, void *iFalse )
{
	ISC_LONG i;
	i = strcmp( Left, Right );
	if ( strcmp( Op, "=" ) == 0 && i == 0
		|| strcmp( Op, ">" ) == 0 && i > 0
		|| strcmp( Op, "<" ) == 0 && i < 0
		|| strcmp( Op, "=>" ) == 0 && i >= 0
		|| strcmp( Op, ">=" ) == 0 && i >= 0
		|| strcmp( Op, "=<" ) == 0 && i <= 0
		|| strcmp( Op, "<=" ) == 0 && i <= 0
		|| strcmp( Op, "<>" ) == 0 && i != 0
		|| strcmp( Op, "#" ) == 0 && i != 0 )
		return iTrue;
	else 
		return iFalse;
}

UDFEXPORT void *iif_t( ISC_QUAD *Left, ISC_SCHAR *Op, ISC_QUAD *Right, void *iTrue, void *iFalse )
{
	if ( strcmp( Op, "=" ) == 0 &&  Left->gds_quad_high == Right->gds_quad_high && Left->gds_quad_low == Right->gds_quad_low
		|| strcmp( Op, ">" ) == 0 &&  Left->gds_quad_high > Right->gds_quad_high
		|| strcmp( Op, ">" ) == 0 &&  Left->gds_quad_high == Right->gds_quad_high && Left->gds_quad_low > Right->gds_quad_low
		|| strcmp( Op, "<" ) == 0 &&  Left->gds_quad_high < Right->gds_quad_high
		|| strcmp( Op, "<" ) == 0 &&  Left->gds_quad_high == Right->gds_quad_high && Left->gds_quad_low < Right->gds_quad_low
		|| strcmp( Op, "=>" ) == 0 &&  Left->gds_quad_high > Right->gds_quad_high
		|| strcmp( Op, "=>" ) == 0 &&  Left->gds_quad_high == Right->gds_quad_high && Left->gds_quad_low >= Right->gds_quad_low
		|| strcmp( Op, ">=" ) == 0 &&  Left->gds_quad_high > Right->gds_quad_high
		|| strcmp( Op, ">=" ) == 0 &&  Left->gds_quad_high == Right->gds_quad_high && Left->gds_quad_low >= Right->gds_quad_low
		|| strcmp( Op, "=<" ) == 0 &&  Left->gds_quad_high < Right->gds_quad_high
		|| strcmp( Op, "=<" ) == 0 &&  Left->gds_quad_high == Right->gds_quad_high && Left->gds_quad_low <= Right->gds_quad_low
		|| strcmp( Op, "<=" ) == 0 &&  Left->gds_quad_high < Right->gds_quad_high
		|| strcmp( Op, "<=" ) == 0 &&  Left->gds_quad_high == Right->gds_quad_high && Left->gds_quad_low <= Right->gds_quad_low
		|| strcmp( Op, "<>" ) == 0 && (Left->gds_quad_high != Right->gds_quad_high || Left->gds_quad_low != Right->gds_quad_low)
		|| strcmp( Op, "#" ) == 0 && (Left->gds_quad_high != Right->gds_quad_high || Left->gds_quad_low != Right->gds_quad_low) )
		return iTrue;
	else 
		return iFalse;
}

//�������� Blob ���� � Grid
//blobcallback �������� � ������ ibase.h
UDFEXPORT ISC_UCHAR *BlobToStr(blobcallback *Blob, ISC_LONG *Number )
{
	ISC_USHORT bytes_to_read;
	ISC_LONG C;
	ISC_UCHAR *result;
	result = 0;
	if ( Blob == 0 || Blob->blob_handle == 0 || Blob->blob_total_length == 0 || *Number <= 0) return 0;
	if ( *Number > 255 ) *Number = 255;
//��������� �������� ������������� �������������� long � short, �� �� ����� ��� ��� ��������� %)
#pragma warning( disable : 4244 )
	if ( Blob->blob_total_length < *Number )
		bytes_to_read = Blob->blob_total_length;
	else 
		bytes_to_read = *Number;
#pragma warning( default : 4244 )
	result = (ISC_UCHAR*)ib_util_malloc( bytes_to_read + 1 );
	result[bytes_to_read] = 0;
	Blob->blob_get_segment( Blob->blob_handle, result, bytes_to_read, &bytes_to_read );
	if ( Blob->blob_total_length > *Number && *Number > 3) 
	{
		result[*Number-3] = '.';
		result[*Number-2] = '.';
		result[*Number-1] = '.';
	}
	for ( C = bytes_to_read - 1; C >= 0; C-- )
		if ( result[C] >= 0 && result[C] < 32 ) result[C]=' ';
	return result;
}
//���������� ������ 60 �������� �� ������� ������ ��� ����������� � �����
UDFEXPORT ISC_SCHAR *BIG_to60(ISC_SCHAR *cstring )
{ 
	ISC_LONG C;
	if ( strlen( cstring ) > 60 ) strcpy( &cstring[57], "..." );
	for ( C = strlen( cstring ) - 1; C >= 0; C-- )
		if ( cstring[C] >= 0 && cstring[C] < 32 ) cstring[C]=' ';
	return cstring;
}

//������� ��� ������ � �����
UDFEXPORT ISC_QUAD *Date_( ISC_QUAD *ib_date )
{
	ib_date->isc_quad_low = 0;
	return ib_date;
}

UDFEXPORT ISC_QUAD *Time_( ISC_QUAD *ib_date )
{
	ib_date->isc_quad_high = 0;
	return ib_date;
}

UDFEXPORT ISC_QUAD *AddDate( ISC_QUAD *ib_date, ISC_LONG *days_to_add )
{
	ib_date->isc_quad_high = ib_date->isc_quad_high + *days_to_add;
	return ib_date;
}

UDFEXPORT ISC_LONG DateDiff( ISC_QUAD *ib_date1, ISC_QUAD *ib_date2 )
{
	return ib_date2->isc_quad_high - ib_date1->isc_quad_high;
}

UDFEXPORT ISC_SCHAR *StrTime( ISC_QUAD *ib_date )
{
	struct tm tm_date;
	ISC_SCHAR *result = (ISC_SCHAR *)ib_util_malloc(9);
	isc_decode_date( ib_date, &tm_date );
	sprintf( result, "%02d:%02d:%02d", tm_date.tm_hour, tm_date.tm_min, tm_date.tm_sec );
	return result;
}

UDFEXPORT ISC_SCHAR *StrDate( ISC_QUAD *ib_date )
{
	struct tm tm_date;
	ISC_SCHAR *result = (ISC_SCHAR *)ib_util_malloc(11);
	isc_decode_date( ib_date, &tm_date );
	sprintf( result, "%02d.%02d.%04d", tm_date.tm_mday, tm_date.tm_mon+1, tm_date.tm_year+1900 );
	return result;
}

UDFEXPORT ISC_LONG _DayOfWeek( ISC_QUAD *ib_date )
{
	struct tm t;
	isc_decode_date( ib_date, &t );
	return t.tm_wday + 1;
}

UDFEXPORT ISC_LONG _DayOfMonth( ISC_QUAD *ib_date )
{
	struct tm t;
	if ( ib_date->isc_quad_high != 0 ) 
	{
		isc_decode_date ( ib_date, &t );
		return t.tm_mday;
	}
	else
		return 1;
}

UDFEXPORT ISC_LONG _Month( ISC_QUAD *ib_date )
{
	struct tm t;
	if ( ib_date->isc_quad_high != 0 ) 
	{
		isc_decode_date ( ib_date, &t );
		return t.tm_mon + 1;
	}
	else
		return 1;
}

UDFEXPORT ISC_LONG _Year( ISC_QUAD *ib_date )
{
	struct tm t;
	if ( ib_date->isc_quad_high != 0 ) 
	{
		isc_decode_date ( ib_date, &t );
		return t.tm_year + 1900;
	}
	else
		return 1;
}


/*TVA 30.03.2010 ����������� ����� ����, �.�. ������� isc_encode_date ����� ����������� 
  (������ ��� �������� ����� ����� �������) � ������ ��������� ������ gDateResult ����� isc_encode_date*/
UDFEXPORT void NormEncodeDate( struct tm *pt, ISC_QUAD *pDateResult )
{
	pt->tm_year = pt->tm_year + ( pt->tm_mon / 12 );
	pt->tm_mon  = pt->tm_mon % 12;
	isc_encode_date( pt, pDateResult );
}

UDFEXPORT ISC_QUAD *_EncodeDate( ISC_LONG *y, ISC_LONG *m, ISC_LONG *d )
{
	struct tm t;
	static ISC_QUAD gDateResult;
	t.tm_sec = 0;
	t.tm_min = 0;
	t.tm_hour = 0;

	t.tm_wday = 0;
	t.tm_yday = 0;
	t.tm_isdst = 0;

	t.tm_year = *y - 1900;
	t.tm_mon  = *m - 1;
	t.tm_mday = *d;
	NormEncodeDate( &t, &gDateResult );
	return &gDateResult;
}

UDFEXPORT ISC_LONG IsDate( ISC_SCHAR *iString )
{
	ISC_QUAD *gDateResult = _AsDate( iString );
	if ( ( _Year( gDateResult ) == 1900 ) || ( _Year( gDateResult ) == 3800 ) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

UDFEXPORT ISC_QUAD *_AsDate( ISC_SCHAR *sz )
{
	ISC_LONG l,i,j,val;
	struct tm t;
	static ISC_QUAD gDateResult;
	t.tm_sec  = 0;  t.tm_min  = 0;  t.tm_hour  = 0;
	t.tm_wday = 0;  t.tm_yday = 0;  t.tm_isdst = 0;
	t.tm_year = 1900;  t.tm_mon  = 1;  t.tm_mday = 1;
	NormEncodeDate( &t, &gDateResult );

	l=strlen( sz );  
	i=0;  j=0;  val=0;
	while ( i <= l ) 
	{
		if ( ( '0' <= sz[i] ) && ( sz[i] <= '9' ) )
			val = val * 10 + sz[i] - '0';
		else if ( ( sz[i] == ' ' ) && ( val == 0 ) ) //������ �� ������� ������� ����� ������������ �������
			;	
		else if ( (sz[i] == 0) && ( val == 0 ) ) //���� ��� ��������� ������ � ��� = 0 �� ������ �� ���� ������
			;
		else
		{
			if ( j == 0 ) 
			{
				if ( ( sz[i] != '.' ) || ( val == 0) ) return &gDateResult;
				t.tm_mday = val;
			} 
			else if ( j == 1 ) 
			{
				if ( ( sz[i] != '.' ) || ( val == 0) ) return &gDateResult;
				t.tm_mon = val;
			} 
			else if ( j == 2 ) 
			{
				if ( ( ( sz[i] != ' ' ) && ( i != l ) ) || ( val == 0 ) ) return &gDateResult;
				t.tm_year = val;
			} 
			else if ( j == 3 ) 
			{
				if ( sz[i] != ':' ) return &gDateResult;
				t.tm_hour = val;
			} 
			else if ( j == 4 ) 
			{
				if ( ( sz[i] != ':' ) && ( i != l ) ) return &gDateResult;
				t.tm_min = val;
			} 
			else if ( j == 5 ) 
			{
				i = l;
				t.tm_sec = val;
			}
			j++;
			val = 0;
		}
		i++;
	}
	t.tm_mon  = t.tm_mon - 1;
	t.tm_year = t.tm_year - 1900;
	NormEncodeDate( &t, &gDateResult );
	return &gDateResult;
}

UDFEXPORT ISC_QUAD *Now2IB()
{
	time_t time_sec;
	struct tm *tbuf;
	static ISC_QUAD gDateResult;
	time( &time_sec );
	tbuf = localtime( &time_sec );
	NormEncodeDate( tbuf, &gDateResult );
	return &gDateResult;
}

/* --- ��������� ��������� ���������� --- */
UDFEXPORT ISC_LONG SuffixToInt( ISC_SCHAR *sz )
{
	//���� ������ ������������ �� ����� �� �-��� ������ ��� �����, ����� 0 }
	ISC_LONG i;
	i = strlen( sz );
	while ( i > 0 && sz[i-1] >= '0' && sz[i-1] <= '9') i--;
	if ( i == strlen( sz ) ) 
		return 0;
	else 
		return atoi( sz + i );
}

UDFEXPORT ISC_SCHAR *DocNumPrefix( ISC_SCHAR *sz )
{
	//���� ������ ������������ �� ����� �� �-��� ������ �� ��� �� ����� �����, ���� ������
	ISC_LONG i;
	i = strlen( sz );
	while ( i > 0 && sz[i-1] >= '0' && sz[i-1] <= '9') i--;
	sz[i] = 0;
	return sz;
}

UDFEXPORT ISC_LONG DocNum( ISC_SCHAR *sz )
{
	ISC_LONG i, Res;
	Res = 0; i = 0;
	while ( sz[i] == ' ' && i < 255 ) i++; // 255 ��� ����� ��������� ��� UDF � IB
	while ( '0' <= sz[i] && sz[i] <= '9' && i < 255 )
	{
	    if ( Res > 214748300 ) return 2147483000; // ���������� ���������� ������ ����� ������� � ������������. ������ �� ���� ����� ����������
		Res = Res * 10 + sz[i] - '0';
		i++;
	}
	return Res;
}

UDFEXPORT ISC_SCHAR *DocNumSuffix( ISC_SCHAR *sz )
{
	ISC_LONG i;
	i = 0;
	while ( sz[i] == ' ' && i < 255 ) i++; // 255 ��� ����� ��������� ��� UDF � IB
	while ( '0' <= sz[i] && sz[i] <= '9' && i < 255 ) i++;
	return &sz[i];
}


UDFEXPORT ISC_SCHAR *DocNumDate( ISC_SCHAR *sz, ISC_QUAD *DT )
{
	struct tm t;
	ISC_SCHAR DatStr[15]; /* ��������: ' �� 01.01.2000' */
	if ( DT->gds_quad_high != 0 ) 
	{
		if ( sz[0] == 0 ) strcpy( sz, "�/�" );
		isc_decode_date( DT, &t );
		if ( t.tm_year >= 0) 
		{
			sprintf( DatStr, " �� %02d.%02d.%4d", t.tm_mday, t.tm_mon+1, t.tm_year+1900 );
			strncat( sz, DatStr, 255 );
		}
	}
  return sz;
}

UDFEXPORT ISC_SCHAR *ContractNum( ISC_SCHAR *Num )
{
	ISC_SCHAR *s, *p;
	while (*Num==' ') Num++; // �������� ������� �����
	p = strstr( Num, "  " );
	s = strstr( Num, " (" );
	if (p==NULL || (p>s && s!=NULL)) p = s;
	if (p!=NULL) p[0] = 0;
	return Num;
}

UDFEXPORT ISC_QUAD *ChangeDateTime( ISC_QUAD *CurrentDate, ISC_LONG *Year, ISC_LONG *Month, ISC_LONG *Day,
							   ISC_LONG *Hour, ISC_LONG *Min, ISC_LONG *Sec )
{
	struct tm t;
	static ISC_QUAD gDateResult;
	if ( ( CurrentDate->gds_quad_high == 0 ) && ( CurrentDate->gds_quad_low == 0 ))
	{
		return NULL;
	}
	isc_decode_date( CurrentDate, &t);
	//��������� ����� �� ������� � ��������
	ISC_LONG DeltaSeconds = ( *Hour * 60 * 60 ) + ( *Min * 60 ) + *Sec;
	//��������� ���������� ������ � ������� ����
	ISC_LONG TimeSeconds = ( t.tm_hour * 60 * 60 ) + ( t.tm_min * 60 ) + t.tm_sec;
	//���� �������� �������� ����� ������� ��� �� ����������� ���, ��� ��� ��� �� ������� ������� �� �����
	ISC_LONG SummSeconds = TimeSeconds + DeltaSeconds;
	if ( ( SummSeconds < 0) || ( SummSeconds > 86399 ) )
	{
		*Day += DeltaSeconds / 86400; //��������� ���������� � ����
		*Sec = DeltaSeconds % 86400; //������� ���������� � ������� � �������� ������ � ����
		*Min = 0;
		*Hour = 0;
		//���� ������� �������������, �� ��� ����� ������ ����� 0 ���, ������� ���� ������ ��� ���� � �������� ������� ������
		if ( *Sec < 0 )
		{
			*Day -= 1;
			*Sec += 86400;
		}
	}
	t.tm_year = t.tm_year + *Year;
	t.tm_mon = t.tm_mon + *Month;
	t.tm_mday = t.tm_mday + *Day;
	t.tm_hour = t.tm_hour + *Hour;
	t.tm_min = t.tm_min + *Min;
	t.tm_sec = t.tm_sec + *Sec;
	NormEncodeDate( &t, &gDateResult );
	return &gDateResult;
	//
}


//������� ����������� ������
UDFEXPORT ISC_SCHAR *DateToXML( ISC_QUAD *ib_date )
{
	struct tm tm_date;
	ISC_SCHAR *result = (ISC_SCHAR *)ib_util_malloc(11);
	isc_decode_date( ib_date, &tm_date );
	if ( ( ib_date->gds_quad_high == 0 ) && ( ib_date->gds_quad_low == 0 ) ) result[0] = '\0';
	else sprintf( result, "%04d-%02d-%02d", tm_date.tm_year+1900, tm_date.tm_mon+1, tm_date.tm_mday );
	return result;
}

UDFEXPORT ISC_SCHAR *DateToStr( ISC_QUAD *ib_date )
{
	struct tm tm_date;
	ISC_SCHAR *result = (ISC_SCHAR *)ib_util_malloc(11);
	isc_decode_date( ib_date, &tm_date );
	if ( ( ib_date->gds_quad_high == 0 ) && ( ib_date->gds_quad_low == 0 ) ) result[0] = '\0';
	else sprintf( result, "%02d.%02d.%04d", tm_date.tm_mday, tm_date.tm_mon+1, tm_date.tm_year+1900 );
	return result;
}

UDFEXPORT ISC_SCHAR *DateTimeToStr( ISC_QUAD *ib_date )
{
	struct tm tm_date;
	ISC_SCHAR *result = (ISC_SCHAR *)ib_util_malloc(22);
	isc_decode_date( ib_date, &tm_date );
	if ( ( ib_date->gds_quad_high == 0 ) && ( ib_date->gds_quad_low == 0 ) ) result[0] = '\0';
	else sprintf( result, "%02d.%02d.%04d %02d:%02d:%02d", tm_date.tm_mday, tm_date.tm_mon+1, tm_date.tm_year+1900, tm_date.tm_hour, tm_date.tm_min, tm_date.tm_sec );
	return result;
}
UDFEXPORT ISC_SCHAR *IntToStr( ISC_LONG *iNum )
{
	ISC_SCHAR *result = (ISC_SCHAR *)ib_util_malloc(21);
    if (*iNum < 0) {                            //��� ���������� ������� ������, ���
		sprintf(&result[1], "%li", -(*iNum));   // � Linux ���� sprintf ������������� ������������� ����� ��� �����������;
		result[0] = '-';                        // ��� ���������, �������� �������������� cast as integer ���� ������ (� Windows ����� ���������� ����� ���� �� ������)
    }
	else sprintf(result, "%li", *iNum);
	return result;
}
UDFEXPORT ISC_SCHAR *IntToNStr( ISC_LONG *iNum, ISC_LONG *iLen )
{
	ISC_SCHAR *result = (ISC_SCHAR *)ib_util_malloc(21);
    short neg = (*iNum < 0) ? 1 : 0;              // 1-����������� ������������� �����; 0-�������������
    sprintf(result, "%li", *iNum * (neg?-1:1));
    short SL = strlen(result);
    short piLen = *iLen>20? 20: *iLen;
          piLen = piLen<0? 0: piLen;
    short i = piLen-SL>0? piLen-SL: 0; // ���������� ������ �� �������� ������
    if (neg && (i==0)) i++;
    memmove(&result[i], result, SL+1);
    while (i>0) result[--i] = '0';
    if    (neg) result[0] = '-';                        // ��� ���������, �������� �������������� cast as integer ���� ������ (� Windows ����� ���������� ����� ���� �� ������)
	return result;
}

UDFEXPORT ISC_SCHAR *FloatToStr( const PARAMDSC *iFloat )
{
	ISC_SHORT vScale = 0;
	ISC_INT64 vBufI = DescToInt( iFloat, vScale );
	ISC_SCHAR *result = (ISC_SCHAR *)ib_util_malloc(21);
	result = NumberToStr( result, vBufI, vScale );
	return result;
}

UDFEXPORT ISC_LONG IsInteger( ISC_SCHAR *iString )
{
	ISC_LONG vIntBuf;
	vIntBuf = atoi( (const char *) iString );
	if ( vIntBuf == 0 )
	{
		if ( ( strlen( iString ) == 1 ) && ( iString[0] == '0' ) )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 1;
	}
}

UDFEXPORT ISC_LONG AsInteger( ISC_SCHAR *iString )
{
	ISC_LONG result;
	result = atoi( (const char *)iString );
	if ( result == 0x7FFFFFFF )
	{ 
		result = 0;
	}
	return result;
}

UDFEXPORT ISC_LONG IsFloat( ISC_SCHAR *iString )
{
	double vFloatBuf;
	vFloatBuf = atof( (const char *) iString );
	if ( vFloatBuf == 0 )
	{
		if ( ( ( strlen( iString ) == 1 ) && ( iString[0] == '0' ) )
			|| ( ( strlen( iString ) == 3 ) && ( iString[0] == '0' ) && ( iString[1] == '.' ) && ( iString[2] == '0' ) ) )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 1;
	}

}


//������� ��� ������ �� ��������
UDFEXPORT ISC_SCHAR *Item( ISC_SCHAR *S, ISC_LONG *I )
{
	ISC_LONG p, b;
	p = 0;  b = 0;
	while ( *I > 0 ) 
	{
		while ( S[p] == 32 || S[p] == 9 || S[p] == 10 || S[p] == 13 ) p++;
		b = p;
		if ( S[p] == ':' || S[p] == '_' || S[p] == ';' || S[p] == '/' || S[p] == '(' || S[p] == ')' ||
			 S[p] == '[' || S[p] == ']' || S[p] == '{' || S[p] == '}' )
			p++;
		else
			while ( S[p] != 9 && S[p] != 10 && S[p] != 13 && S[p] != 0 &&
					S[p] != ':' && S[p] != '_' && S[p] != ';' && S[p] != '/' && S[p] != '(' && 
					S[p] != ')' && S[p] != '[' && S[p] != ']' && S[p] != '{' && S[p] != '}' )
				p++;
			(*I)--;
	}
	S[p] = 0;
	return S + b;
}

UDFEXPORT ISC_LONG StrLen( ISC_SCHAR *Source )
{
	return strlen( Source );
}

UDFEXPORT ISC_LONG SubStr( ISC_SCHAR *szSubstr, ISC_SCHAR *szStr )
{
	ISC_SCHAR * s;
	if ( ( ( *szSubstr ) && ( *szStr ) ) == 0 ) //���� ���� �� ����� ������, �� ������� �����
		return -1;
	s = strstr( szStr, szSubstr );
	if ( s )
		return s - szStr;
	else
		return -1;
}

// ���������� ������������ Pos, ������� ������� ������� � ������ � 1
UDFEXPORT ISC_LONG _Pos( ISC_SCHAR *szSubstr, ISC_SCHAR *szStr )
{
	ISC_SCHAR * s;
	if ( ( ( *szSubstr ) && ( *szStr ) ) == 0 ) //���� ���� �� ����� ������, �� ������� �����
		return 0;
	s = strstr( szStr, szSubstr );
	if ( s )
		return s - szStr + 1;
	else
		return 0;
}

UDFEXPORT ISC_LONG _RightPos( ISC_SCHAR *szSubstr, ISC_SCHAR *szStr )
{
	ISC_SCHAR * s=NULL, * p=szStr;
	if ( ( ( *szSubstr ) && ( *szStr ) ) == 0 ) //���� ���� �� ����� ������, �� ������� �����
		return 0;
	while (true) {
	   p = strstr( p, szSubstr );
	   if ( p == NULL ) break;
	   s = p;
	   p++;
	}
	if ( s ) return s - szStr + 1;
		else return 0;
}

UDFEXPORT ISC_SCHAR *Left( ISC_SCHAR *sz, ISC_LONG *Number )
{
	if ( *Number < 0 ) *Number = 0;
	if ( *Number < strlen( sz ) ) sz[*Number] = 0;
	return sz;
}

UDFEXPORT ISC_SCHAR *Right( ISC_SCHAR *sz, ISC_LONG *Number )
{
	ISC_LONG p;
	if ( *Number < 0 ) *Number = 0;
	p = strlen(sz) - *Number;
	if ( p < 0 ) p = 0;
	return &sz[p];
}

UDFEXPORT ISC_SCHAR *Mid( ISC_SCHAR *sz, ISC_LONG *Start, ISC_LONG *Number )
{
	if ( ( *Number > 0) && ( *Start < strlen( sz ) ) )
	{
		if ( *Start < 0 ) *Start = 0;
		if ( *Start + *Number < strlen( sz ) )
		sz[*Start + *Number] = 0;
		return sz + *Start;
	}
	else
		return sz + strlen( sz );
}

// ������������ Copy �������� ���������, ������� ������� � ������ � 1
UDFEXPORT ISC_SCHAR *_Copy( ISC_SCHAR *sz, ISC_LONG *Start, ISC_LONG *Number )
{
	if ( ( *Number > 0) && ( *Start <= strlen( sz ) ) )
	{
		if ( *Start <= 0 ) *Start = 1;
		if ( *Start + *Number <= strlen( sz ) )  sz[*Start + *Number - 1] = 0;
		return sz + *Start - 1;
	}
	else
		return sz + strlen( sz ); //������� ������ ������
}

UDFEXPORT ISC_SCHAR *LRTrim( ISC_SCHAR *sz )
{
	ISC_LONG i, L;
	L = strlen( sz )-1;
	i = 0;
	while (i<=L && ( sz[i]==7 || sz[i]==8 || sz[i]==9 || sz[i]==10 || sz[i]==13 || sz[i]==26 || sz[i]==27 || sz[i]==' ' || sz[i]==-96 )) i++;
	while (L>=i && ( sz[L]==7 || sz[L]==8 || sz[L]==9 || sz[L]==10 || sz[L]==13 || sz[L]==26 || sz[L]==27 || sz[L]==' ' || sz[L]==-96 )) L--;
					  // Beep,  BackSpace,        Tab,          LF,          CR,         EOF,         Esc,        Space,    HardSpace
	sz[L+1] = 0;
	return sz + i;
}

UDFEXPORT ISC_SCHAR *LTrim( ISC_SCHAR *sz )
{
	ISC_LONG i, L;
	L = strlen( sz )-1;
	i = 0;
	while (i<=L && ( sz[i]==7 || sz[i]==8 || sz[i]==9 || sz[i]==10 || sz[i]==13 || sz[i]==26 || sz[i]==27 || sz[i]==' ' || sz[i]==-96 )) i++;
	return sz + i;
}

UDFEXPORT ISC_SCHAR *RTrim( ISC_SCHAR *sz )
{
	ISC_LONG L;
	L = strlen( sz )-1;
	while (L>=0 && ( sz[L]==7 || sz[L]==8 || sz[L]==9 || sz[L]==10 || sz[L]==13 || sz[L]==26 || sz[L]==27 || sz[L]==' ' || sz[L]==-96 )) L--;
	sz[L+1] = 0;
	return sz;
}

UDFEXPORT ISC_SCHAR *_Upper( ISC_SCHAR *sz )
{
	ISC_LONG i;
	i = strlen( sz );
	while (--i >= 0)
	{
		if      ( sz[i] == -72 ) sz[i] = -88;           /* ANSI-����� � */
		else if (( sz[i] >= -32 ) && (sz[i] < 0)) sz[i] = sz[i]-32;     /* ��������� ����� ANSI-��������� */
		else                      sz[i] = toupper(sz[i]); /* ��������� ������� */
	}
	return sz;
}


UDFEXPORT ISC_SCHAR *_Concat( ISC_SCHAR *S1, ISC_SCHAR *S2 )
{
	ISC_USHORT vLen = 254 - strlen( S1 );
	if ( vLen > 254 ) vLen = 0;
	return strncat( S1, S2, vLen );
}

UDFEXPORT ISC_SCHAR *_Concat3( ISC_SCHAR *S1, ISC_SCHAR *S2, ISC_SCHAR *S3 )
{
	return _Concat( _Concat( S1, S2 ), S3 );
}

UDFEXPORT ISC_SCHAR *Token( ISC_SCHAR *sz, ISC_SCHAR *sep, ISC_LONG *n )
{
	ISC_LONG b,e,L,count;
	ISC_SCHAR ch[2];
	ISC_USHORT is_single_sep;
	/* ���������� � ������ str ������� (n-1)-��� ����������� */
	ch[1] = 0;
	b = 1; count = 0;
	L = strlen( sz );
	is_single_sep = ( strlen( sep ) == 1 );
	if ( is_single_sep )
	{ /* � ��������� �� 1 ����������� - ���� ������� */
		while ( ( b <= L ) && ( count < (*n) - 1 ) )
		{
			if ( sz[b-1] == sep[0] ) count++;
			b++;
		}
	}
	else
	{ /* ���� ������������ ������, �� ���� ��������� */
		while ( ( b <= L ) && ( count < (*n) - 1 ) )
		{
			ch[0] = sz[b-1];
			if ( strstr( sep, ch ) != 0 ) count++;
			b++;
		}
	}
	/* ����� �������, b - ������ �������� ��������� ��� ����� ������, ���� ��-� �� ������ */
	e = b;
	if ( is_single_sep )
	{
		while ( e <= L )
		{
			if ( sz[e-1] == sep[0] ) break;
			e++;
		}
	}
	else
	{
		while ( e <= L )
		{
			ch[0] = sz[e-1];
			if ( strstr( sep, ch ) != 0) break;
			e++;
		}
	}
	if ( e - b > 255 ) e = b + 255; /* ���� ������ �� ������, ��� 255 �������� */
	sz[e-1] = 0;
	return &sz[b-1];
}

UDFEXPORT ISC_SCHAR *Rus12Lat( ISC_SCHAR *sz )
{
	ISC_USHORT i;
    for (i=0; sz[i]; i++) {
       if      ( sz[i]==-72 ) sz[i]=-88;                 /* ANSI-����� � */
	   else if ((sz[i]==-32) || (sz[i]==-64)) sz[i]='A'; /* 12 ������� ���� */
	   else if ((sz[i]==-30) || (sz[i]==-62)) sz[i]='B';
	   else if ((sz[i]==-27) || (sz[i]==-59)) sz[i]='E';
	   else if ((sz[i]==-22) || (sz[i]==-54)) sz[i]='K';
	   else if ((sz[i]==-20) || (sz[i]==-52)) sz[i]='M';
	   else if ((sz[i]==-19) || (sz[i]==-51)) sz[i]='H';
	   else if ((sz[i]==-18) || (sz[i]==-50)) sz[i]='O';
	   else if ((sz[i]==-16) || (sz[i]==-48)) sz[i]='P';
	   else if ((sz[i]==-15) || (sz[i]==-47)) sz[i]='C';
	   else if ((sz[i]==-14) || (sz[i]==-46)) sz[i]='T';
	   else if ((sz[i]==-13) || (sz[i]==-45)) sz[i]='Y';
	   else if ((sz[i]==-11) || (sz[i]==-43)) sz[i]='X';
       else if ((sz[i]>=-32) && (sz[i] < 0 )) sz[i]=sz[i]-32;       /* ��������� ����� ANSI-��������� */
       else                                   sz[i]=toupper(sz[i]); /* ��������� ������� */
	}
    return sz;
}

UDFEXPORT ISC_SCHAR *NormStr( ISC_SCHAR *Source )
{
	ISC_SCHAR *vRes;
	ISC_USHORT vInd, i;
	ISC_USHORT vLength;
	ISC_USHORT vCounter;

    for (vInd=0; Source[vInd]; vInd++)
       if ((Source[vInd]==-36) ||(Source[vInd]==-38) ||(Source[vInd]==-4) ||(Source[vInd]==-6)) // �����: � � � �
          for (i=vInd; i<32000; i++) {
             Source[i] = Source[i+1];  // ������ ��� �����, ��� ����� �� � ������
             if (Source[i]==0) break;
          }

	vLength = StrLen( Source );
	vRes = ( ISC_SCHAR* )ib_util_malloc( vLength + 1 + 4); //��� 4 ������� ������� ��� ������������� �������� ���, ���, ���
    vInd = 0;
	vCounter = 0;
	while ( vInd < vLength )
	{
		//���� ������������ � ����� ����, ������ ��������
		if ( ( Source[vInd] > 47 ) && ( Source[vInd] < 58 ) )
		{
			vRes[vCounter] = Source[vInd]; //�������� ������
		}
		//���� ��������� ��������, �� �������� � ��������� �� ������� � ��������� ����
		else if ( ( Source[vInd] > 96 ) && ( Source[vInd] < 123 ) )
		{
			Source[vInd] = toupper( Source[vInd] ); //������ ����� ��������
			vCounter--;
			vInd--;
		}
		//���� ��������� �������, �� �������� � ��������� �� ������� � ��������� ����
		else if ( ( Source[vInd] > -33 ) && ( Source[vInd] < 0 ) )
		{
			Source[vInd] = Source[vInd] - 32; //������ ����� ��������
			vCounter--;
			vInd--;
		}
		//���� ������� ��������, �� ���������
		else if ( ( Source[vInd] > 64 ) && ( Source[vInd] < 91 ) )
		{
			if ( Source[vInd] == 73 ) vRes[vCounter] = 49; //I - 1
			else if ( Source[vInd] == 79 )	vRes[vCounter] = 48; //O - 0
			else if ( Source[vInd] == 83 )	vRes[vCounter] = 53; //S - 5
			else vRes[vCounter] = Source[vInd]; //�������� ������
		}
		//���� ������� ������� �����, �� ���������
		else if ( ( Source[vInd] > -65 ) && ( Source[vInd] < -32 ) )
		{
			if ( Source[vInd] == -64 ) vRes[vCounter] = 65; //� - A
			else if ( Source[vInd] == -62 )	vRes[vCounter] = 66; //� - B
			else if ( Source[vInd] == -47 )	vRes[vCounter] = 67; //� - C
			else if ( Source[vInd] == -59 )	vRes[vCounter] = 69; //� - E
			else if ( Source[vInd] == -51 ) vRes[vCounter] = 72; //� - H
			else if ( Source[vInd] == -54 ) vRes[vCounter] = 75; //� - K
			else if ( Source[vInd] == -52 )	vRes[vCounter] = 77; //� - M
			else if ( Source[vInd] == -48 )	vRes[vCounter] = 80; //� - P
			else if ( Source[vInd] == -46 )	vRes[vCounter] = 84; //� - T
			else if ( Source[vInd] == -43 )	vRes[vCounter] = 88; //� - X
			else if ( Source[vInd] == -45 )	vRes[vCounter] = 89; //� - Y
			else if ( Source[vInd] == -50 )	vRes[vCounter] = 48; //� - 0
			else if ( Source[vInd] == -40 )	vRes[vCounter] = 87; //� - W
			else if ( Source[vInd] == -39 )	vRes[vCounter] = 87; //� - W
			else if ( Source[vInd] == -41 )	vRes[vCounter] = 52; //� - 4
			else if ( Source[vInd] == -63 )	vRes[vCounter] = 54; //� - 6
			else if ( Source[vInd] == -57 )	vRes[vCounter] = 51; //� - 3
            /*
			else if ( Source[vInd] == -38 )
			{
				Source[vInd] = 32; //� - ' ' //�������� �� ������, ��� ������, ����� ��� ��������� �������, �� ����������
				vCounter--;
				vInd--;
			}
			else if ( Source[vInd] == -36 )
			{
				Source[vInd] = 32; //� - ' ' //�������� �� ������, ��� ������, ����� ��� ��������� �������, �� ����������
				vCounter--;
				vInd--;
			}
            */
			else vRes[vCounter] = Source[vInd]; //�������� ������
		}
		else if ( Source[vInd] == -72 )	vRes[vCounter] = 69; //� - E
		else if ( Source[vInd] == -88 )	vRes[vCounter] = 69; //� - E
		//���� ������, �� ���������
		else if ( Source[vInd] == 32 )
		{
			//���� ��������� ������ ���� ������, �� �������� ������� ����� "������" ������
			if ( Source[vInd + 1] == 32 ) vCounter--; //������������ ����� ����� ���������� ������ ������� ���������
			//���� ��� ������ ������, �� �� ��������� ������
			else if ( vCounter == 0 ) vCounter--; //������������ ����� ����� ���������� ������ ������� ���������
			//���� ������� ������ � ���������� - ������, �� �������� �������, ����� "������" ������� ������
			else if ( vRes[vCounter - 1] == 32 ) vCounter--; //������������ ����� ����� ���������� ������ ������� ���������
			//���� ��� �������� �� �� �� �����, �� ������ ��������� ������
			else vRes[vCounter] = Source[vInd]; //�������� ������
		}
		//���� ����������/����, �������� �� ������, ��� ������ ��������, ����� ��� ��� ��������
		else
		{
			Source[vInd] = 32;
			vCounter--;
			vInd--;
		}
		vInd++;
		vCounter++;
	}
	vRes[vCounter] = 0;
	//������ ��� ������� ������, ����� �������� ���, ��� ��� ��� �������������� � �����
	while ( ( vRes[vCounter] == 32 ) || ( vRes[vCounter] == 0 ) )
	{
		vRes[vCounter] = 0;
        if (vCounter==0) break;
		vCounter--;
	}
    if ( StrLen(vRes) < 4 ) return vRes;
	vCounter++;
	//���
	if ( ( vRes[0] == 48 ) && ( vRes[1] == 48 ) && ( vRes[2] == 48 ) && ( vRes[3] == 32 ) )
	{
		vRes[vCounter] = 32;
		vRes[vCounter + 1] = 48;
		vRes[vCounter + 2] = 48;
		vRes[vCounter + 3] = 48;
		vRes[vCounter + 4] = 0;
		//vRes += 4;
        vInd = 3;
        while (vRes[vInd]) { vRes[vInd-3] = vRes[vInd+1]; vInd++; } //��������� 4� �������� �����
		return vRes;
	}
	//�A�
	else if ( ( vRes[0] == 48 ) && ( vRes[1] == 65 ) && ( vRes[2] == 48 ) && ( vRes[3] == 32 ) )
	{
		vRes[vCounter] = 32;
		vRes[vCounter + 1] = 48;
		vRes[vCounter + 2] = 65;
		vRes[vCounter + 3] = 48;
		vRes[vCounter + 4] = 0;
		//vRes += 4;
        vInd = 3;
        while (vRes[vInd]) { vRes[vInd-3] = vRes[vInd+1]; vInd++; } //��������� 4� �������� �����
		return vRes;
	}
	//���
	else if ( ( vRes[0] == 51 ) && ( vRes[1] == 65 ) && ( vRes[2] == 48 ) && ( vRes[3] == 32 ) )
	{
		vRes[vCounter] = 32;
		vRes[vCounter + 1] = 51;
		vRes[vCounter + 2] = 65;
		vRes[vCounter + 3] = 48;
		vRes[vCounter + 4] = 0;
		//vRes += 4;
        vInd = 3;
        while (vRes[vInd]) { vRes[vInd-3] = vRes[vInd+1]; vInd++; } //��������� 4� �������� �����
		return vRes;
	}
	else return vRes;
}

/*UDFEXPORT ISC_SCHAR *NormStr( ISC_SCHAR *Source )
{
	ISC_SCHAR *vRes;
	ISC_USHORT vInd;
	ISC_USHORT vLength;
	ISC_USHORT vCounter;
	vLength = StrLen( Source );
	vRes = ( ISC_SCHAR* )ib_util_malloc( vLength + 1 + 4); //��� 4 ������� ������� ��� ������������� �������� ���, ���, ���
    vInd = 0;
	vCounter = 0;
	while ( vInd < vLength )
	{
		//���� ������������ � ����� ����, ������ ��������
		if ( ( Source[vInd] > 47 ) && ( Source[vInd] < 58 ) )
		{
			vRes[vCounter] = Source[vInd]; //�������� ������
		}
		//���� ��������� ��������, �� �������� � ��������� �� ������� � ��������� ����
		else if ( ( Source[vInd] > 96 ) && ( Source[vInd] < 123 ) )
		{
			Source[vInd] = toupper( Source[vInd] ); //������ ����� ��������
			vCounter--;
			vInd--;
		}
		//���� ��������� �������, �� �������� � ��������� �� ������� � ��������� ����
		else if ( ( Source[vInd] > -33 ) && ( Source[vInd] < 0 ) )
		{
			Source[vInd] = Source[vInd] - 32; //������ ����� ��������
			vCounter--;
			vInd--;
		}
		//���� ������� ��������, �� ���������
		else if ( ( Source[vInd] > 64 ) && ( Source[vInd] < 91 ) )
		{
			if ( Source[vInd] == 73 ) vRes[vCounter] = 49; //I - 1
			else if ( Source[vInd] == 79 )	vRes[vCounter] = 48; //O - 0
			else if ( Source[vInd] == 83 )	vRes[vCounter] = 53; //S - 5
			else vRes[vCounter] = Source[vInd]; //�������� ������
		}
		//���� ������� ������� �����, �� ���������
		else if ( ( Source[vInd] > -65 ) && ( Source[vInd] < -32 ) )
		{
			if ( Source[vInd] == -64 ) vRes[vCounter] = 65; //� - A
			else if ( Source[vInd] == -62 )	vRes[vCounter] = 66; //� - B
			else if ( Source[vInd] == -47 )	vRes[vCounter] = 67; //� - C
			else if ( Source[vInd] == -59 )	vRes[vCounter] = 69; //� - E
			else if ( Source[vInd] == -51 ) vRes[vCounter] = 72; //� - H
			else if ( Source[vInd] == -54 ) vRes[vCounter] = 75; //� - K
			else if ( Source[vInd] == -52 )	vRes[vCounter] = 77; //� - M
			else if ( Source[vInd] == -48 )	vRes[vCounter] = 80; //� - P
			else if ( Source[vInd] == -46 )	vRes[vCounter] = 84; //� - T
			else if ( Source[vInd] == -43 )	vRes[vCounter] = 88; //� - X
			else if ( Source[vInd] == -45 )	vRes[vCounter] = 89; //� - Y
			else if ( Source[vInd] == -50 )	vRes[vCounter] = 48; //� - 0
			else if ( Source[vInd] == -40 )	vRes[vCounter] = 87; //� - W
			else if ( Source[vInd] == -39 )	vRes[vCounter] = 87; //� - W
			else if ( Source[vInd] == -41 )	vRes[vCounter] = 52; //� - 4
			else if ( Source[vInd] == -63 )	vRes[vCounter] = 54; //� - 6
			else if ( Source[vInd] == -57 )	vRes[vCounter] = 51; //� - 3
			else if ( Source[vInd] == -38 )
			{
				Source[vInd] = 32; //� - ' ' //�������� �� ������, ��� ������, ����� ��� ��������� �������, �� ����������
				vCounter--;
				vInd--;
			}
			else if ( Source[vInd] == -36 )
			{
				Source[vInd] = 32; //� - ' ' //�������� �� ������, ��� ������, ����� ��� ��������� �������, �� ����������
				vCounter--;
				vInd--;
			}
			else vRes[vCounter] = Source[vInd]; //�������� ������
		}
		else if ( Source[vInd] == -72 )	vRes[vCounter] = 69; //� - E
		else if ( Source[vInd] == -88 )	vRes[vCounter] = 69; //� - E
		//���� ������, �� ���������
		else if ( Source[vInd] == 32 )
		{
			//���� ��������� ������ ���� ������, �� �������� ������� ����� "������" ������
			if ( Source[vInd + 1] == 32 ) vCounter--; //������������ ����� ����� ���������� ������ ������� ���������
			//���� ��� ������ ������, �� �� ��������� ������
			else if ( vCounter == 0 ) vCounter--; //������������ ����� ����� ���������� ������ ������� ���������
			//���� ������� ������ � ���������� - ������, �� �������� �������, ����� "������" ������� ������
			else if ( vRes[vCounter - 1] == 32 ) vCounter--; //������������ ����� ����� ���������� ������ ������� ���������
			//���� ��� �������� �� �� �� �����, �� ������ ��������� ������
			else vRes[vCounter] = Source[vInd]; //�������� ������
		}
		//���� ����������/����, �������� �� ������, ��� ������ ��������, ����� ��� ��� ��������
		else
		{
			Source[vInd] = 32;
			vCounter--;
			vInd--;
		}
		vInd++;
		vCounter++;
	}
	vRes[vCounter] = 0;
	//������ ��� ������� ������, ����� �������� ���, ��� ��� ��� �������������� � �����
	while ( ( vRes[vCounter] == 32 ) || ( vRes[vCounter] == 0 ) )
	{
		vRes[vCounter] = 0;
		vCounter--;
	}
	vCounter++;
	//���
	if ( ( vRes[0] == 48 ) && ( vRes[1] == 48 ) && ( vRes[2] == 48 ) && ( vRes[3] == 32 ) )
	{
		vRes[vCounter] = 32;
		vRes[vCounter + 1] = 48;
		vRes[vCounter + 2] = 48;
		vRes[vCounter + 3] = 48;
		vRes[vCounter + 4] = 0;
		//vRes += 4;
        vInd = 3;
        while (vRes[vInd]) { vRes[vInd-3] = vRes[vInd+1]; vInd++; } //��������� 4� �������� �����
		return vRes;
	}
	//�A�
	else if ( ( vRes[0] == 48 ) && ( vRes[1] == 65 ) && ( vRes[2] == 48 ) && ( vRes[3] == 32 ) )
	{
		vRes[vCounter] = 32;
		vRes[vCounter + 1] = 48;
		vRes[vCounter + 2] = 65;
		vRes[vCounter + 3] = 48;
		vRes[vCounter + 4] = 0;
		//vRes += 4;
        vInd = 3;
        while (vRes[vInd]) { vRes[vInd-3] = vRes[vInd+1]; vInd++; } //��������� 4� �������� �����
		return vRes;
	}
	//���
	else if ( ( vRes[0] == 51 ) && ( vRes[1] == 65 ) && ( vRes[2] == 48 ) && ( vRes[3] == 32 ) )
	{
		vRes[vCounter] = 32;
		vRes[vCounter + 1] = 51;
		vRes[vCounter + 2] = 65;
		vRes[vCounter + 3] = 48;
		vRes[vCounter + 4] = 0;
		//vRes += 4;
        vInd = 3;
        while (vRes[vInd]) { vRes[vInd-3] = vRes[vInd+1]; vInd++; } //��������� 4� �������� �����
		return vRes;
	}
	else return vRes;
}*/

UDFEXPORT ISC_LONG IsBlank( ISC_SCHAR *Src )
{
	ISC_USHORT i;
	ISC_USHORT vLen;
	vLen = StrLen( Src );
	i = 0;
	while ( i < vLen )
	{
		if ( ! (Src[i]==7  || Src[i]==8  || Src[i]==9  ||   // Beep, BackSpace, Tab
				Src[i]==10 || Src[i]==13 || Src[i]==26 ||   // LF, CR, EOF
				Src[i]==27 || Src[i]==32 || Src[i]==-96) )  // Esc, Space, HardSpace
		{
			return 0;
		}
		i++;
	}
	return 1;
}

UDFEXPORT ISC_SCHAR *ReplaceStr(ISC_SCHAR *Source, ISC_SCHAR *OldStr, ISC_SCHAR *NewStr)
{
	ISC_SCHAR *vResult; // the return string
	ISC_SCHAR *vIns;    // the next insert point
	ISC_SCHAR *vTmp;    // varies
	ISC_LONG vOldLength;  // length of OldStr
	ISC_LONG vNewLength; // length of NewStr
	ISC_LONG vNextLength; // distance between OldStr and end of last OldStr
	ISC_LONG vCount;    // number of replacements

	if (!Source) return NULL;
	if (!OldStr || !(vOldLength = strlen(OldStr)) || !(vIns = strstr(Source, OldStr))) {
		vResult = (ISC_SCHAR *)ib_util_malloc(strlen(Source) + 1);
		if (!vResult) return NULL;
		strcpy(vResult, Source);
		return vResult;
	} else {
		if (!NewStr) NewStr = "";
		vNewLength = strlen(NewStr);

		for (vCount = 0; vTmp = strstr(vIns, OldStr); ++vCount) {
			vIns = vTmp + vOldLength;
		}

		// first time through the loop, all the variable are set correctly
		// from here on,
		//    tmp points to the end of the vResult string
		//    ins points to the next occurrence of OldStr in Source
		//    Source points to the remainder of Source after "end of OldStr"
		vTmp = vResult = (ISC_SCHAR *)ib_util_malloc(strlen(Source) + (vNewLength - vOldLength) * vCount + 1);

		if (!vResult) return NULL;

		while (vCount--) {
			vIns = strstr(Source, OldStr);
			vNextLength = vIns - Source;
			vTmp = strncpy(vTmp, Source, vNextLength) + vNextLength;
			vTmp = strcpy(vTmp, NewStr) + vNewLength;
			Source += vNextLength + vOldLength; // move to next "end of OldStr"
		}
		strcpy(vTmp, Source);
		return vResult;
	}
}

