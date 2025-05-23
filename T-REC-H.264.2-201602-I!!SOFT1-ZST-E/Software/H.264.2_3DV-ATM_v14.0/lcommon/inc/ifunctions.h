/*
 * Disclaimer of Warranty
 *
 * Copyright 2001-2015, International Telecommunication Union, Geneva
 *
 * These software programs are available to the user without any
 * license fee or royalty on an "as is" basis. The ITU disclaims
 * any and all warranties, whether express, implied, or statutory,
 * including any implied warranties of merchantability or of fitness
 * for a particular purpose.  In no event shall the ITU be liable for
 * any incidental, punitive, or consequential damages of any kind
 * whatsoever arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs
 * and the user's customers, employees, agents, transferees,
 * successors, and assigns.
 *
 * The ITU does not represent or warrant that the programs furnished
 * hereunder are free of infringement of any third-party patents.
 * Commercial implementations of ITU-T Recommendations, including
 * shareware, may be subject to royalty fees to patent holders.
 * Information regarding the ITU-T patent policy is available from the
 * ITU web site at http://www.itu.int.
 *
 * THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.
 *
 */



/*!
 ************************************************************************
 *  \file
 *     ifunctions.h
 *
 *  \brief
 *     define some inline functions that are used within the encoder.
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Karsten S�hring                 <suehring@hhi.de>
 *      - Alexis Tourapis                 <alexismt@ieee.org>
 *
 ************************************************************************
 */
#ifndef _IFUNCTIONS_H_
#define _IFUNCTIONS_H_

# if !(defined(WIN32) || defined(WIN64)) && (__STDC_VERSION__ < 199901L)
  #define static
  #define inline
#endif
#include <math.h>
#include <limits.h>


static inline short smin(short a, short b)
{
  return (short) (((a) < (b)) ? (a) : (b));
}

static inline short smax(short a, short b)
{
  return (short) (((a) > (b)) ? (a) : (b));
}

static inline int imin(int a, int b)
{
  return ((a) < (b)) ? (a) : (b);
}

static inline int imax(int a, int b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline int imedian(int a,int b,int c)
{
  if (a > b) // a > b
  { 
    if (b > c) 
      return(b); // a > b > c
    else if (a > c) 
      return(c); // a > c > b
    else 
      return(a); // c > a > b
  }
  else // b > a
  { 
    if (a > c) 
      return(a); // b > a > c
    else if (b > c)
      return(c); // b > c > a
    else
      return(b);  // c > b > a
  }
}

static inline int imedian_old(int a, int b, int c)
{
  return (a + b + c - imin(a, imin(b, c)) - imax(a, imax(b ,c)));
}

static inline double dmin(double a, double b)
{
  return ((a) < (b)) ? (a) : (b);
}

static inline double dmax(double a, double b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline int64 i64min(int64 a, int64 b)
{
  return ((a) < (b)) ? (a) : (b);
}

static inline int64 i64max(int64 a, int64 b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline distblk distblkmin(distblk a, distblk b)
{
  return ((a) < (b)) ? (a) : (b);
}

static inline distblk distblkmax(distblk a, distblk b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline short sabs(short x)
{
  static const short SHORT_BITS = (sizeof(short) * CHAR_BIT) - 1;
  short y = (short) (x >> SHORT_BITS);
  return (short) ((x ^ y) - y);
}

static inline int iabs(int x)
{
  static const int INT_BITS = (sizeof(int) * CHAR_BIT) - 1;
  int y = x >> INT_BITS;
  return (x ^ y) - y;
}

static inline double dabs(double x)
{
  return ((x) < 0) ? -(x) : (x);
}

static inline int64 i64abs(int64 x)
{
  static const int64 INT64_BITS = (sizeof(int64) * CHAR_BIT) - 1;
  int64 y = x >> INT64_BITS;
  return (x ^ y) - y;
}

static inline double dabs2(double x)
{
  return (x) * (x);
}

static inline int iabs2(int x) 
{
  return (x) * (x);
}

static inline int64 i64abs2(int64 x)
{
  return (x) * (x);
}

static inline int isign(int x)
{
  return ( (x > 0) - (x < 0));
}

static inline int isignab(int a, int b)
{
  return ((b) < 0) ? -iabs(a) : iabs(a);
}

static inline int rshift_rnd(int x, int a)
{
  return (a > 0) ? ((x + (1 << (a-1) )) >> a) : (x << (-a));
}

static inline int rshift_rnd_sign(int x, int a)
{
  return (x > 0) ? ( ( x + (1 << (a-1)) ) >> a ) : (-( ( iabs(x) + (1 << (a-1)) ) >> a ));
}

static inline unsigned int rshift_rnd_us(unsigned int x, unsigned int a)
{
  return (a > 0) ? ((x + (1 << (a-1))) >> a) : x;
}

static inline int rshift_rnd_sf(int x, int a)
{
  return ((x + (1 << (a-1) )) >> a);
}

static inline int shift_off_sf(int x, int o, int a)
{
  return ((x + o) >> a);
}

static inline unsigned int rshift_rnd_us_sf(unsigned int x, unsigned int a)
{
  return ((x + (1 << (a-1))) >> a);
}

static inline int iClip1(int high, int x)
{
  x = imax(x, 0);
  x = imin(x, high);

  return x;
}

static inline int iClip3(int low, int high, int x)
{
  x = imax(x, low);
  x = imin(x, high);

  return x;
}

static inline short sClip3(short low, short high, short x)
{
  x = smax(x, low);
  x = smin(x, high);

  return x;
}

static inline double dClip3(double low, double high, double x)
{
  x = dmax(x, low);
  x = dmin(x, high);

  return x;
}


static inline distblk weighted_cost(int factor, int bits)
{
#if JCOST_CALC_SCALEUP
  return (((distblk)(factor))*((distblk)(bits)));
#else
#if (USE_RND_COST)
  return (rshift_rnd_sf((lambda) * (bits), LAMBDA_ACCURACY_BITS));
#else
  return (((factor)*(bits))>>LAMBDA_ACCURACY_BITS);
#endif
#endif
}

static inline int RSD(int x)
{
 return ((x&2)?(x|1):(x&(~1)));
}

static inline int power2(int x) 
{
  return 1 << (x);
}


static const int64 po2[64] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000,0x8000,
                              0x10000,0x20000,0x40000,0x80000,0x100000,0x200000,0x400000,0x800000,0x1000000,0x2000000,0x4000000,0x8000000,
                              0x10000000,0x20000000,0x40000000,0x80000000,0x100000000,0x200000000,0x400000000,0x800000000,
                              0x1000000000,0x2000000000,0x4000000000,0x8000000000,0x10000000000,0x20000000000,0x40000000000,0x80000000000,
                              0x100000000000,0x200000000000,0x400000000000,0x800000000000,
                              0x1000000000000,0x2000000000000,0x4000000000000,0x8000000000000,
                              0x10000000000000,0x20000000000000,0x40000000000000,0x80000000000000,
                              0x100000000000000,0x200000000000000,0x400000000000000,0x800000000000000,
                              0x1000000000000000,0x2000000000000000,0x4000000000000000,0x8000000000000000};

static inline int64 i64_power2(int x)
{
  return((x > 63) ? 0 : po2[x]);
}

static inline int float2int (float x)
{
  return (int)((x < 0) ? (x - 0.5f) : (x + 0.5f));
}

static inline int get_bit(int64 x,int n)
{
  return (int)(((x >> n) & 1));
}

#if ZEROSNR
static inline float psnr(int max_sample_sq, int samples, float sse_distortion ) 
{
  return (float) (10.0 * log10(max_sample_sq * (double) ((double) samples / (sse_distortion < 1.0 ? 1.0 : sse_distortion))));
}
#else
static inline float psnr(int max_sample_sq, int samples, float sse_distortion ) 
{
  return (float) (sse_distortion == 0.0 ? 0.0 : (10.0 * log10(max_sample_sq * (double) ((double) samples / sse_distortion))));
}
#endif

static inline int CheckCost_Shift(int64 mcost, int64 min_mcost)  
{
  if((mcost<<LAMBDA_ACCURACY_BITS) >= min_mcost)  
    return 1;
  else
    return 0; 
}

static inline int CheckCost(int64 mcost, int64 min_mcost)
{
  return ((mcost) >= (min_mcost));
}

static inline void down_scale(distblk *pblkdistCost) 
{
#if JCOST_CALC_SCALEUP
#if (IMGTYPE < 2)
  *pblkdistCost = (*pblkdistCost)>>LAMBDA_ACCURACY_BITS;
#else
  *pblkdistCost = (*pblkdistCost)/(1<<LAMBDA_ACCURACY_BITS);
#endif
#endif
}

static inline void up_scale(distblk *pblkdistCost) 
{
#if JCOST_CALC_SCALEUP
#if (IMGTYPE < 2)
  *pblkdistCost = (*pblkdistCost)<<LAMBDA_ACCURACY_BITS;
#else
  *pblkdistCost = (*pblkdistCost)*(1<<LAMBDA_ACCURACY_BITS);
#endif
#endif
}

static inline distblk dist_scale(distblk blkdistCost) 
{
#if JCOST_CALC_SCALEUP
#if (IMGTYPE < 2)
  return ((blkdistCost)<<LAMBDA_ACCURACY_BITS);
#else
  return ((blkdistCost) *((distblk) (1<<LAMBDA_ACCURACY_BITS));
#endif
#else
  return (blkdistCost);
#endif
}

static inline int dist_down(distblk blkdistCost) 
{
#if JCOST_CALC_SCALEUP
#if (IMGTYPE < 2)
  return ((int)((blkdistCost)>>LAMBDA_ACCURACY_BITS));
#else
  return ((int)(blkdistCost/((distblk) (1<<LAMBDA_ACCURACY_BITS))));
#endif
#else
  return ((int)blkdistCost);
#endif
}

# if !(defined(WIN32) || defined(WIN64)) && (__STDC_VERSION__ < 199901L)
  #undef static
  #undef inline
#endif

#endif

