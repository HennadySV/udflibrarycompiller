//FANSY Firebird User Defined Functions
//FANSY RG (c)2011

#include "ibase.h"

//Если WIN компилятор
#if defined (WIN32) || defined(_WIN32) || defined(__WIN32__)
//Если динамический экспорт
#ifdef FSFBUDF_EXPORTS
#define UDFEXPORT __declspec(dllexport)
//Если статический экспорт
#else
#define UDFEXPORT __declspec(dllimport)
#endif
//Если *NIX компилятор
#else
#define UDFEXPORT
//У GCC нет функции _atoi64, вместо нее atoll, потому делаем линк
#define _atoi64 atoll
#endif

//Если С++ компилятор
#ifdef __cplusplus
extern "C" {
#endif

/***************Объявление функций***************/
//Функции с различной реализацией для 1/3 диалектов Firebird
#ifdef FB_DIALECT3
UDFEXPORT PARAMDSC *_abs( const PARAMDSC *, PARAMDSC * );
UDFEXPORT PARAMDSC *_floor( const PARAMDSC *, PARAMDSC * );
UDFEXPORT PARAMDSC *Round( const PARAMDSC *, ISC_LONG *, PARAMDSC * );
UDFEXPORT PARAMDSC *max_d( const PARAMDSC *, const PARAMDSC *, PARAMDSC * );
UDFEXPORT PARAMDSC *min_d( const PARAMDSC *, const PARAMDSC *, PARAMDSC * );
UDFEXPORT PARAMDSC *AsFloat( const PARAMDSC *, PARAMDSC * );
UDFEXPORT void *iif_d( const PARAMDSC *, ISC_SCHAR *, const PARAMDSC *, void *, void * );
#else
UDFEXPORT double *_abs( double * );
UDFEXPORT double *_floor( double *);
UDFEXPORT double Round( double *, ISC_LONG * );
UDFEXPORT double max_d( double *, double * );
UDFEXPORT double min_d( double *, double * );
UDFEXPORT double AsFloat( ISC_SCHAR * );
UDFEXPORT void *iif_d( double *, ISC_SCHAR *, double *, void *, void * );
#endif
	
//Алгебраические функции
UDFEXPORT double _Power( double *, double * );
UDFEXPORT double _Exp( double * );

//Функции по работе с множеством
UDFEXPORT ISC_LONG SetIn( ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_LONG SetPlus( ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_LONG SetMinus( ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_LONG SetMult( ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_LONG SetIncl( ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_LONG SetExcl( ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_LONG SetEmpty();
UDFEXPORT ISC_LONG SetFull();

//Функции сравнения
UDFEXPORT ISC_LONG max_i( ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_LONG min_i( ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_SCHAR *max_s( ISC_SCHAR *, ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *min_s( ISC_SCHAR *, ISC_SCHAR * );
UDFEXPORT ISC_QUAD *max_t( ISC_QUAD *, ISC_QUAD * );
UDFEXPORT ISC_QUAD *min_t( ISC_QUAD *, ISC_QUAD * );

//IIF Функции
UDFEXPORT void *iif_i( ISC_LONG *, ISC_SCHAR *, ISC_LONG *, void *, void *);
UDFEXPORT void *iif_s( ISC_SCHAR *, ISC_SCHAR *, ISC_SCHAR *, void *, void *);
UDFEXPORT void *iif_t( ISC_QUAD *, ISC_SCHAR *, ISC_QUAD *, void *, void *);

//Передача Blob поля в Grid
UDFEXPORT ISC_UCHAR *BlobToStr( blobcallback *, ISC_LONG * );
UDFEXPORT ISC_SCHAR *BIG_to60( ISC_SCHAR * );

//Функции для работы с датой
UDFEXPORT ISC_QUAD *Date_( ISC_QUAD * );
UDFEXPORT ISC_QUAD *Time_( ISC_QUAD * );
UDFEXPORT ISC_QUAD *AddDate( ISC_QUAD *, ISC_LONG * );
UDFEXPORT ISC_LONG DateDiff( ISC_QUAD *, ISC_QUAD * );
UDFEXPORT ISC_SCHAR *StrTime( ISC_QUAD * );
UDFEXPORT ISC_SCHAR *StrDate( ISC_QUAD * );
UDFEXPORT ISC_LONG _DayOfWeek( ISC_QUAD * );
UDFEXPORT ISC_LONG _DayOfMonth( ISC_QUAD * );
UDFEXPORT ISC_LONG _Month( ISC_QUAD * );
UDFEXPORT ISC_LONG _Year( ISC_QUAD * );
UDFEXPORT void NormEncodeDate( struct tm *, ISC_QUAD * );
UDFEXPORT ISC_QUAD *_EncodeDate( ISC_LONG *, ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_LONG IsDate( ISC_SCHAR * );
UDFEXPORT ISC_QUAD *_AsDate( ISC_SCHAR * );
UDFEXPORT ISC_QUAD *Now2IB();
UDFEXPORT ISC_LONG SuffixToInt( ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *DocNumPrefix( ISC_SCHAR * );
UDFEXPORT ISC_LONG DocNum( ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *DocNumSuffix( ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *DocNumDate( ISC_SCHAR *, ISC_QUAD * );
UDFEXPORT ISC_SCHAR *ContractNum( ISC_SCHAR *Num );
UDFEXPORT ISC_QUAD *ChangeDateTime( ISC_QUAD *, ISC_LONG *, ISC_LONG *, ISC_LONG *, ISC_LONG *, ISC_LONG *, ISC_LONG * );

//Функции конвертации данных
UDFEXPORT ISC_SCHAR *DateToXML( ISC_QUAD * );
UDFEXPORT ISC_SCHAR *DateToStr( ISC_QUAD * );
UDFEXPORT ISC_SCHAR *DateTimeToStr( ISC_QUAD * );
UDFEXPORT ISC_SCHAR *IntToStr ( ISC_LONG * );
UDFEXPORT ISC_SCHAR *IntToNStr( ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_SCHAR *FloatToStr( const PARAMDSC * );
UDFEXPORT ISC_LONG IsInteger( ISC_SCHAR * );
UDFEXPORT ISC_LONG AsInteger( ISC_SCHAR * );
UDFEXPORT ISC_LONG IsFloat( ISC_SCHAR * );

//Функции для работы со строками
UDFEXPORT ISC_SCHAR *Item( ISC_SCHAR *, ISC_LONG * );
UDFEXPORT ISC_LONG StrLen( ISC_SCHAR * );
UDFEXPORT ISC_LONG SubStr( ISC_SCHAR *, ISC_SCHAR * );
UDFEXPORT ISC_LONG _Pos     ( ISC_SCHAR *, ISC_SCHAR * );
UDFEXPORT ISC_LONG _RightPos( ISC_SCHAR *, ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *Left   ( ISC_SCHAR *, ISC_LONG * );
UDFEXPORT ISC_SCHAR *Right  ( ISC_SCHAR *, ISC_LONG * );
UDFEXPORT ISC_SCHAR *Mid    ( ISC_SCHAR *, ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_SCHAR *_Copy  ( ISC_SCHAR *, ISC_LONG *, ISC_LONG * );
UDFEXPORT ISC_SCHAR *LTrim  ( ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *LRTrim ( ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *RTrim  ( ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *_Upper ( ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *_Concat ( ISC_SCHAR *, ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *_Concat3( ISC_SCHAR *, ISC_SCHAR *, ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *Token( ISC_SCHAR *, ISC_SCHAR *, ISC_LONG * );
UDFEXPORT ISC_SCHAR *Rus12Lat( ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *NormStr ( ISC_SCHAR * );
UDFEXPORT ISC_SCHAR *ReplaceStr(ISC_SCHAR *, ISC_SCHAR *, ISC_SCHAR * );
UDFEXPORT ISC_LONG IsBlank( ISC_SCHAR * );
/************************************************/

//Закрытие С++ компилятора
#ifdef __cplusplus
}
#endif

