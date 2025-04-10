/*
********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005-2009, International Telecommunications Union, Geneva

The Fraunhofer HHI hereby donate this source code to the ITU, with the following
understanding:
    1. Fraunhofer HHI retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. Fraunhofer HHI retain full patent rights (if any exist) in the technical
       content of techniques and algorithms herein.
    3. The ITU shall make this code available to anyone, free of license or
       royalty fees.

DISCLAIMER OF WARRANTY

These software programs are available to the user without any license fee or
royalty on an "as is" basis. The ITU disclaims any and all warranties, whether
express, implied, or statutory, including any implied warranties of
merchantability or of fitness for a particular purpose. In no event shall the
contributor or the ITU be liable for any incidental, punitive, or consequential
damages of any kind whatsoever arising from the use of these programs.

This disclaimer of warranty extends to the user of these programs and user's
customers, employees, agents, transferees, successors, and assigns.

The ITU does not represent or warrant that the programs furnished hereunder are
free of infringement of any third-party patents. Commercial implementations of
ITU-T Recommendations, including shareware, may be subject to royalty fees to
patent holders. Information regarding the ITU-T patent policy is available from 
the ITU Web site at http://www.itu.int.

THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.

********************************************************************************
*/


#if !defined(AFX_GLOBALFUNCTIONS_H__85D028A1_590E_423B_9072_AB351037B1E8__INCLUDED_)
#define AFX_GLOBALFUNCTIONS_H__85D028A1_590E_423B_9072_AB351037B1E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))


H264AVC_NAMESPACE_BEGIN

__inline
const Int gClip( const Int iX )
{
  const Int i2 = (iX & 0xFF);
  if( i2 == iX )  { return iX; }
  if( iX < 0 )    { return 0x00; }
  else            { return 0xFF; }
}


__inline
const Int gClipMinMax( const Int iX, const Int iMin, const Int iMax )
{
  return max( min( iX, iMax ), iMin );
}

//TMM_WP
__inline
Int gIntRandom(const Int iMin, const Int iMax)
{
    Double fRange = (Double)(iMax - iMin + 1);
    Int iValue = (Int)(fRange*rand()/(RAND_MAX+1.0));

    AOT_DBG( (iValue + iMin)> iMax );
    return iValue + iMin;
}
//TMM_WP

H264AVC_NAMESPACE_END


#endif // !defined(AFX_GLOBALFUNCTIONS_H__85D028A1_590E_423B_9072_AB351037B1E8__INCLUDED_)
