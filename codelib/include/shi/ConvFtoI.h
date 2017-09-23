#ifndef _CONVFTOI_H_
#define _CONVFTOI_H_


/* This include is required to get the typedefs used here */
#include "shi.h"

/******************************************************/
/*                                                    */
/*  This series of structures is used for building    */
/*  the FloatIntTemplate structure below.             */
/*                                                    */
/******************************************************/

#if defined(CHIPSET_LITTLE_ENDIAN)

typedef struct { Int16   value; Int16 high16;   Int32 reserved; } Float64Int16;
typedef struct { Int32   value; Int32 reserved;                 } Float64Int32;
typedef struct { Float32 value; Int32 reserved;                 } Float64Float32;

#else

typedef struct { Int32 reserved; Int16   high16; Int16 value; } Float64Int16;
typedef struct { Int32 reserved; Int32   value;               } Float64Int32;
typedef struct { Int32 reserved; Float32 value;               } Float64Float32;

#endif

typedef struct { Float64 value;} Float64Float64;


/******************************************************/
/*  FloatIntTemplate                                  */
/*                                                    */
/*  This structure is arranged to allow for accessing */
/*  of various componenets of floats and integers.    */
/*  It is intended as a method of portable            */
/*  float-to-int conversions.                         */
/*                                                    */
/******************************************************/

typedef union {
  Float64Float64 in_Float64;
  Float64Float32 in_Float32;
  Float64Int32   in_Int32;
  Float64Int16   in_Int16;
} FloatIntTemplate;


/******************************************************/
/*  FLOATING POINT TO INTEGER CONVERSION ROUTINES     */
/*                                                    */
/*  METHOD: Use addition on a Float64 to force the    */
/*  significant into the proper bit-positions of the  */
/*  lower 32-bits.                                    */
/*                                                    */
/*  This method of conversion comes from Kuswara      */
/*  Pranawahadi.                                      */
/*                                                    */
/******************************************************/

#define FLOAT64_TO_INT32_K 6755399441055744.0 /* ( 2 ** 52 ) * 1.5 */

/* MSVC seems to have problems with inline functions, */

inline Int32
FloatScaledToInt32(Float32     x,
                   const Int32 n)
{
  FloatIntTemplate number;
  
  /* The compiler has to be good enough to optimize between */
  /* these comments to a single floating-point add.         */
  
  if (n >= 0)
    number.in_Float64.value = x + (FLOAT64_TO_INT32_K/(1<<( n)));
  else
    number.in_Float64.value = x + (FLOAT64_TO_INT32_K*(1<<(-n)));

  /**********************************************************/
  
  return (number.in_Int32.value);
}

#if 0 //LRKLUDGE doesn't work for single precision
inline Int32
FloatToInt32(Float32 x)
{
  return(FloatScaledToInt32(x,0));
}
#endif

#if defined(_MSC_VER)

#pragma warning(disable : 4035)

inline
FloatToInt32(float x)
{
  __asm {
    fld   dword ptr [x];
    fistp dword ptr [x];
    mov   eax, dword ptr [x];
  }
}

inline void
FloatToInt32Store(int*, float x)
{
  __asm {              
    fld   dword ptr [x];
    mov   eax, dword ptr[x];   
    fistp dword ptr [eax];  
  }
}


#pragma warning(default : 4035)

#else /* _MSC_VER */

/* Stupid method for non-Intel at the moment, should */
/* be the float-add conversion method.               */

#define FloatToInt32(X)      ((int)(X))
#define FloatToInt32Store(X) (*(IX)=((int)(X)))

#endif /* _MSC_VER */

#endif // _CONVFTOI_H_
