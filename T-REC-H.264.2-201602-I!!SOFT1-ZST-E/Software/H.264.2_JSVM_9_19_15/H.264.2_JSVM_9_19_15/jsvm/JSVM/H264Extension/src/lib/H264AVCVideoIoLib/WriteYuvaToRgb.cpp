/*
********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2009-2012, International Telecommunications Union, Geneva

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

#include "H264AVCVideoIoLib.h"
#include "WriteYuvaToRgb.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

__inline
Int Clip( const Int iX )
{
  const Int i2 = (iX & 0xFF);
  if( i2 == iX )  { return iX; }
  if( iX < 0 ) { return 0x00; }
  else { return 0xFF; }
}

#define dematrix R = Clip(((Y *37 + V * 51 ) >> 5) - 223); G = Clip(((Y *37 - U * 13 - V * 26 ) >> 5) + 135); B = Clip(((Y *37 + U * 65) >> 5) - 277);

#define pack32 ((R << 16) | (G << 8) | B)
#define pack15 (((R >> 3) << 10 ) | ((G >> 3) << 5 ) | (B >> 3) )
#define pack16 (((R >> 3) << 11 ) | ((G >> 2) << 5 ) | (B >> 3) )


WriteYuvaToRgb::WriteYuvaToRgb()
{
}

WriteYuvaToRgb::~WriteYuvaToRgb()
{
}



ErrVal WriteYuvaToRgb::create( WriteYuvaToRgb*& rpcWriteYuvaToRgb )
{
  rpcWriteYuvaToRgb = new WriteYuvaToRgb;

  ROT( NULL == rpcWriteYuvaToRgb );

  return Err::m_nOK;
}

ErrVal WriteYuvaToRgb::destroy()
{
  delete this;

  return Err::m_nOK;
}

ErrVal WriteYuvaToRgb::setFrameDimension( UInt uiLumHeight, UInt uiLumWidth )
{
  m_uiHeight = uiLumHeight;
  m_uiWidth =  uiLumWidth;
  return Err::m_nOK;
}

ErrVal WriteYuvaToRgb::writeFrameRGB( UChar *pucRGB,
                                      UInt uiDestStride,
                                      const UChar *pLum,
                                      const UChar *pCb,
                                      const UChar *pCr,
                                      UInt uiLumHeight,
                                      UInt uiLumWidth,
                                      UInt uiLumStride )

{
  CHECK( pLum );
  CHECK( pCb );
  CHECK( pCr );


  const UChar *pu, *pv, *py;
  UInt  column, row;
  Int   Y, U, V, R, G, B;
  UInt  *argb;
  UChar *pucDest = pucRGB;
  Int   iWidth = uiDestStride;

  if( uiLumHeight > m_uiHeight)
  {
    uiLumHeight = m_uiHeight;
  }

  if( uiLumWidth > m_uiWidth)
  {
    uiLumWidth = m_uiWidth;
  }


  for( row = 0; row < uiLumHeight; row++)
  {
    argb = (UInt*)pucDest;
    pucDest += iWidth;
    pu   = pCb + (row>>1) * (uiLumStride >> 1);
    pv   = pCr + (row>>1) * (uiLumStride >> 1);
    py   = pLum +  row * uiLumStride;

    for( column = 0; column < uiLumWidth; column+= 2)
    {
      Y = *py++;
      U = *pu++;
      V = *pv++;

      dematrix
      *argb++ = pack32;

      Y = *py++;

      dematrix
      *argb++ = pack32;
    }
  }

  return Err::m_nOK;
}


ErrVal WriteYuvaToRgb::writeFrameYV12( UChar *pucDest,
                                       UInt uiDestStride,
                                       const UChar *pLum,
                                       const UChar *pCb,
                                       const UChar *pCr,
                                       UInt uiLumHeight,
                                       UInt uiLumWidth,
                                       UInt uiLumStride )

{
  CHECK( pLum );
  CHECK( pCb );
  CHECK( pCr );


  UInt *pDest = (UInt*)pucDest;
  UInt  row;
  Int   iWidth = uiDestStride;

  if( uiLumHeight > m_uiHeight)
  {
    uiLumHeight = m_uiHeight;
  }

  if( uiLumWidth > m_uiWidth)
  {
    uiLumWidth = m_uiWidth;
  }

  UInt uiStride = uiLumStride;
  UInt uiWidth  = uiLumWidth;
  UInt uiHeight = uiLumHeight;

  {
    const UChar* pSrc = pLum;
    UChar* pDes = (UChar*)pDest;
    for( row = 0; row < uiHeight; row++)
    {
      memcpy( pDes, pSrc, uiWidth);
      pSrc += uiStride;
      pDes += iWidth;
    }
  }

  uiStride >>= 1;
  uiWidth  >>= 1;
  uiHeight >>= 1;
  {
    const UChar* pSrc1 = pCb;
    const UChar* pSrc2 = pCr;
    UChar* pDes = (UChar*)pDest + 2*uiHeight * uiDestStride;

    memset( pDes, 0x80, 2*uiHeight * uiDestStride/2 );

    for( row = 0; row < uiHeight; row++)
    {
      memcpy( pDes, pSrc2, uiWidth);
      pDes += uiDestStride/2;
      pSrc2 += uiStride;
    }

    for( row = 0; row < uiHeight; row++)
    {
      memcpy( pDes, pSrc1, uiWidth);
      pDes += uiDestStride/2;
      pSrc1 += uiStride;
    }
  }

  return Err::m_nOK;
}

ErrVal WriteYuvaToRgb::writeFrameYUYV( UChar* pucYUYV,
                                       UInt uiDestStride,
                                       const UChar *pLum,
                                       const UChar *pCb,
                                       const UChar *pCr,
                                       UInt uiLumHeight,
                                       UInt uiLumWidth,
                                       UInt uiLumStride )

{
  CHECK( pLum );
  CHECK( pCb );
  CHECK( pCr );


  UChar *pucDest = pucYUYV;
  const UChar *pu, *pv, *py;
  UInt  column, row;
  Int   iWidth = uiDestStride;

  if( uiLumHeight > m_uiHeight)
  {
    uiLumHeight = m_uiHeight;
  }

  if( uiLumWidth > m_uiWidth)
  {
    uiLumWidth = m_uiWidth;
  }

  for( row = 0; row < uiLumHeight; row++)
  {
    UChar* Yuvy = pucDest;
    pucDest += iWidth;
    pu   = pCb + (row>>1) * (uiLumStride >> 1);
    pv   = pCr + (row>>1) * (uiLumStride >> 1);
    py   = pLum +  row * uiLumStride;

    for( column = 0; column < uiLumWidth; column+= 2)
    {
      Yuvy[0] = *py++;
      Yuvy[2] = *py++;
      Yuvy[1] = *pu++;
      Yuvy[3] = *pv++;
      Yuvy += 4;
    }
  }

  return Err::m_nOK;
}


