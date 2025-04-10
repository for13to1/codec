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


#include "H264AVCEncoderLib.h"

#include "MotionEstimationQuarterPel.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"


H264AVC_NAMESPACE_BEGIN


const UChar  g_aucFilter[9] =
{ 0, 2, 2, 1, 1, 3, 3, 3, 3 };

const Mv g_acHPSearchMv[9] =
{
//hor,ver
 Mv(  0,  0 ), // 0
 Mv(  0, -2 ), // 1
 Mv(  0,  2 ), // 2
 Mv( -2,  0 ), // 3
 Mv(  2,  0 ), // 4
 Mv( -2, -2 ), // 5
 Mv(  2, -2 ), // 6
 Mv( -2,  2 ), // 7
 Mv(  2,  2 )  // 8
};


const Mv g_acQPSearchMv[9] =
{
//hor,ver
 Mv(  0,  0 ), // 0
 Mv(  0, -1 ), // 1
 Mv(  0,  1 ), // 2
 Mv( -1, -1 ), // 5
 Mv(  1, -1 ), // 6
 Mv( -1,  0 ), // 3
 Mv(  1,  0 ), // 4
 Mv( -1,  1 ), // 7
 Mv(  1,  1 )  // 8
};


MotionEstimationQuarterPel::MotionEstimationQuarterPel() :
  m_uiBestMode( 0 )
{
}

MotionEstimationQuarterPel::~MotionEstimationQuarterPel()
{
}


ErrVal MotionEstimationQuarterPel::create( MotionEstimation*& rpcMotionEstimation )
{
  MotionEstimationQuarterPel* pcMotionEstimationQuarterPel;

  pcMotionEstimationQuarterPel = new MotionEstimationQuarterPel;

  rpcMotionEstimation = pcMotionEstimationQuarterPel;

  ROT( NULL == rpcMotionEstimation );

  pcMotionEstimationQuarterPel->xInitBuffer();
  return Err::m_nOK;
}


Void MotionEstimationQuarterPel::xInitBuffer()
{
  m_apXHPelSearch[0] = &m_aXHPelSearch[0];
  m_apXHPelSearch[1] = &m_aXHPelSearch[1*17*X1];
  m_apXHPelSearch[2] = &m_aXHPelSearch[1*17*X1+X1];
  m_apXHPelSearch[3] = &m_aXHPelSearch[2*17*X1];
  m_apXHPelSearch[4] = &m_aXHPelSearch[2*17*X1+1];
  m_apXHPelSearch[5] = &m_aXHPelSearch[3*17*X1];
  m_apXHPelSearch[6] = &m_aXHPelSearch[3*17*X1+1];
  m_apXHPelSearch[7] = &m_aXHPelSearch[3*17*X1+X1];
  m_apXHPelSearch[8] = &m_aXHPelSearch[3*17*X1+X1+1];

  m_apXQPelSearch[0] = &m_aXQPelSearch[4*16*16];
  m_apXQPelSearch[1] = &m_aXQPelSearch[1*16*16];
  m_apXQPelSearch[2] = &m_aXQPelSearch[7*16*16];
  m_apXQPelSearch[3] = &m_aXQPelSearch[0*16*16];
  m_apXQPelSearch[4] = &m_aXQPelSearch[2*16*16];
  m_apXQPelSearch[5] = &m_aXQPelSearch[3*16*16];
  m_apXQPelSearch[6] = &m_aXQPelSearch[5*16*16];
  m_apXQPelSearch[7] = &m_aXQPelSearch[6*16*16];
  m_apXQPelSearch[8] = &m_aXQPelSearch[8*16*16];
}



Void MotionEstimationQuarterPel::xSubPelSearch( IntYuvPicBuffer*  pcPelData,
                                                Mv&               rcMv,
                                                UInt&             ruiSAD,
                                                UInt              uiBlk,
                                                UInt              uiMode,
                                                Bool              bQPelOnly)
{
  // TMM_ESS {
  Mv    cIniMv = rcMv;
  if( bQPelOnly && (rcMv.getHor() % 2 || rcMv.getVer() % 2) )
  {
    rcMv >>= 1;
    rcMv <<= 1;
  }
// TMM_ESS }
  
  Mv    cMvBestMatch  = rcMv;
  UInt  uiBestSad     = MSYS_UINT_MAX;
  UInt  uiNumHPelPos  = 9;
  UInt  uiNumQPelPos  = 9;
  UInt  uiBestHPelPos = MSYS_UINT_MAX;

  Int   n;
  UInt  uiYSize = 0;
  UInt  uiXSize = 0;

  if( bQPelOnly )
  {
    AOT( (rcMv.getHor() % 2) || (rcMv.getVer() % 2) );
    cMvBestMatch >>= 2;
    cMvBestMatch <<= 2;

    //--- get best h-pel search position ---
    Mv cMvDiff = rcMv - cMvBestMatch;
    for( Int iHIndex = 0; iHIndex < 9; iHIndex++ )
    { 
      if( cMvDiff == g_acHPSearchMv[iHIndex] )
      {
        uiBestHPelPos = iHIndex;
        break;
      }
    }
    AOT( uiBestHPelPos == 5 || uiBestHPelPos == 1 || uiBestHPelPos == 6 ||
         uiBestHPelPos == 3 || uiBestHPelPos == 7 || uiBestHPelPos == MSYS_UINT_MAX );
    
    rcMv = cMvBestMatch;
  }

  xGetSizeFromMode      ( uiXSize, uiYSize, uiMode);
  xCompensateBlocksHalf ( m_aXHPelSearch, pcPelData, cMvBestMatch, uiMode, uiYSize, uiXSize ) ;
  m_cXDSS.iYStride = X1;

  if( ! bQPelOnly || 0 == uiBestHPelPos )
  {
    n = 0;
    Mv cMvTest = g_acHPSearchMv[n];
    m_cXDSS.pYSearch = m_apXHPelSearch[n];
    UInt uiSad = m_cXDSS.Func( &m_cXDSS );

    cMvTest += rcMv;
    uiSad += xGetCost( cMvTest );

    if( uiSad < uiBestSad )
    {
      m_uiBestMode = n;
      uiBestSad = uiSad;
      cMvBestMatch = cMvTest;
    }
  }

  //compute SAD for initial 1/2 pel MV
  for( n = 1; n < (Int)uiNumHPelPos; n++ )
  {
    if( bQPelOnly && n != uiBestHPelPos )
    {
      continue;
    }

    Mv cMvTest = g_acHPSearchMv[n];
    m_cXDSS.pYSearch = m_apXHPelSearch[n];
    UInt uiSad = m_cXDSS.Func( &m_cXDSS );

    cMvTest += rcMv;
    UInt uiCost = xGetCost( cMvTest );
    uiSad += uiCost;
//JVT-W080 BUG_FIX
		if( OmitPDISearch( cMvTest.getHor(), cMvTest.getVer(), true ) )
			continue;
//~JVT-W080 BUG_FIX


    if( uiSad < uiBestSad )
    {
      m_uiBestMode = n;
      uiBestSad = uiSad;
      cMvBestMatch = cMvTest;
    }
  }

  rcMv = cMvBestMatch;

  XPel* pPel = pcPelData->getLumBlk();
  Int iStride = pcPelData->getLStride();
  pPel += (cMvBestMatch.getHor()>>1) + (cMvBestMatch.getVer()>>1) * iStride;

  m_pcQuarterPelFilter->filterBlock( m_aXQPelSearch, pPel, iStride, uiXSize, uiYSize, g_aucFilter[m_uiBestMode]);
  m_cXDSS.iYStride = 16;

  //1/4 pel refinement around initial 1/2 pel MV
  for( n = 1; n < (Int)uiNumQPelPos; n++ )
  {
    Mv cMvTest = g_acQPSearchMv[n];
    m_cXDSS.pYSearch = m_apXQPelSearch[n];
    UInt uiSad = m_cXDSS.Func( &m_cXDSS );

    cMvTest += rcMv;
    uiSad += xGetCost( cMvTest );
//JVT-W080 BUG_FIX
		if( OmitPDISearch( cMvTest.getHor(), cMvTest.getVer(), true ) )
			continue;
//~JVT-W080	BUG_FIX

    // TMM_ESS {
    if( bQPelOnly && (abs(cIniMv.getHor()-cMvTest.getHor()) > 1 || abs(cIniMv.getVer()-cMvTest.getVer()) > 1) )
      uiSad = MSYS_UINT_MAX;
    // TMM_ESS }
	
    if( uiSad < uiBestSad )
    {
      m_uiBestMode = n<<4;
      uiBestSad = uiSad;
      cMvBestMatch = cMvTest;
    }
  }

  rcMv = cMvBestMatch;
  ruiSAD = uiBestSad;

// TMM_ESS {
//---
  if( bQPelOnly && (cIniMv.getHor() % 2 || cIniMv.getVer() % 2) )
  {
    int   dx0 = ((cIniMv.getHor()>>1)<<1);
    int   dy0 = ((cIniMv.getVer()>>1)<<1);
    int   dx1 = (cIniMv.getHor() % 2) ? (dx0+2) : dx0;
    int   dy1 = (cIniMv.getVer() % 2) ? (dy0+2) : dy0;

    for (int dx=dx0; dx<=dx1; dx+=2)
    {
      for (int dy=dy0; dy<=dy1; dy+=2)
      {
        if (dx!=dx0 || dy!=dy0)
        {
          rcMv.setHor(dx);
          rcMv.setVer(dy);

          UInt  uiBestSad1     = MSYS_UINT_MAX;
          Mv    cMvBestMatch1 = rcMv;
          cMvBestMatch1 >>= 2;
          cMvBestMatch1 <<= 2;

          //--- get best h-pel search position ---
          Mv cMvDiff = rcMv - cMvBestMatch1;
          for( Int iHIndex = 0; iHIndex < 9; iHIndex++ )
          { 
            if( cMvDiff == g_acHPSearchMv[iHIndex] )
            {
              uiBestHPelPos = iHIndex;
              break;
            }
          }
          AOT( uiBestHPelPos == 5 || uiBestHPelPos == 1 || uiBestHPelPos == 6 ||
               uiBestHPelPos == 3 || uiBestHPelPos == 7 || uiBestHPelPos == MSYS_UINT_MAX );
    
          rcMv = cMvBestMatch1;


          xGetSizeFromMode      ( uiXSize, uiYSize, uiMode);
          xCompensateBlocksHalf ( m_aXHPelSearch, pcPelData, cMvBestMatch1, uiMode, uiYSize, uiXSize ) ;
          m_cXDSS.iYStride = X1;

          if( 0 == uiBestHPelPos )
          {
            n = 0;
            Mv cMvTest = g_acHPSearchMv[n];
            m_cXDSS.pYSearch = m_apXHPelSearch[n];
            UInt uiSad = m_cXDSS.Func( &m_cXDSS );

            cMvTest += rcMv;
            uiSad += xGetCost( cMvTest );

            if( uiSad < uiBestSad1 )
            {
              m_uiBestMode = n;
              uiBestSad1 = uiSad;
              cMvBestMatch1 = cMvTest;
            }
          }

          //compute SAD for initial 1/2 pel MV
          for( n = 1; n < (Int)uiNumHPelPos; n++ )
          {
            if( n != uiBestHPelPos )
            {
              continue;
            }

            Mv cMvTest = g_acHPSearchMv[n];
            m_cXDSS.pYSearch = m_apXHPelSearch[n];
            UInt uiSad = m_cXDSS.Func( &m_cXDSS );

            cMvTest += rcMv;
            UInt uiCost = xGetCost( cMvTest );
            uiSad += uiCost;

            if( uiSad < uiBestSad1 )
            {
              m_uiBestMode = n;
              uiBestSad1 = uiSad;
              cMvBestMatch1 = cMvTest;
            }
          }

          rcMv = cMvBestMatch1;

          pPel = pcPelData->getLumBlk();
          iStride = pcPelData->getLStride();
          pPel += (cMvBestMatch1.getHor()>>1) + (cMvBestMatch1.getVer()>>1) * iStride;

          m_pcQuarterPelFilter->filterBlock( m_aXQPelSearch, pPel, iStride, uiXSize, uiYSize, g_aucFilter[m_uiBestMode]); //1/4 pel interp
          m_cXDSS.iYStride = 16;

          //1/4 pel refinement around initial 1/2 pel MV
          for( n = 1; n < (Int)uiNumQPelPos; n++ )
          {
            Mv cMvTest = g_acQPSearchMv[n];
            m_cXDSS.pYSearch = m_apXQPelSearch[n];
            UInt uiSad = m_cXDSS.Func( &m_cXDSS );

            cMvTest += rcMv;
            uiSad += xGetCost( cMvTest );

            if( bQPelOnly && (abs(cIniMv.getHor()-cMvTest.getHor()) > 1 || abs(cIniMv.getVer()-cMvTest.getVer()) > 1) )
              uiSad = MSYS_UINT_MAX;

            if( uiSad < uiBestSad1 )
            {
              m_uiBestMode = n<<4;
              uiBestSad1 = uiSad;
              cMvBestMatch1 = cMvTest;
            }
          }

          if (uiBestSad1 < uiBestSad)
          {
            uiBestSad = uiBestSad1;
            rcMv = cMvBestMatch1;
            ruiSAD = uiBestSad;
          }
        } // if (dx!=dx0 || dy!=dy0)
      } // for (int dy=dy0; dy<=dy1; dy+=2)
    } // for (int dx=dx0; dx<=dx1; dx+=2)
  }
//---
// TMM_ESS }
  return;
}

Void MotionEstimationQuarterPel::xGetSizeFromMode( UInt& ruiXSize, UInt& ruiYSize, UInt uiMode )
{
  switch( uiMode )
  {
  case MODE_16x16:
    {
      ruiYSize = 16;
      ruiXSize = 16;
      break;
    }
  case MODE_8x16:
    {
      ruiYSize = 16;
      ruiXSize = 8;
      break;
    }
  case MODE_16x8:
    {
      ruiYSize = 8;
      ruiXSize = 16;
      break;
    }
  case BLK_8x8:
    {
      ruiYSize = 8;
      ruiXSize = 8;
      break;
    }

  case BLK_4x8:
    {
      ruiYSize = 8;
      ruiXSize = 4;
      break;
    }

  case BLK_8x4:
    {
      ruiYSize = 4;
      ruiXSize = 8;
      break;
    }

  case BLK_4x4:
    {
      ruiYSize = 4;
      ruiXSize = 4;
      break;
    }
  default:
    assert( 0 );
    return;
  }
}

Void MotionEstimationQuarterPel::xCompensateBlocksHalf( XPel *pPelDes, IntYuvPicBuffer *pcRefPelData, Mv cMv, UInt uiMode, UInt uiYSize, UInt uiXSize )
{
  XPel* pPelSrc = pcRefPelData->getLumBlk();
  Int iSrcStride = pcRefPelData->getLStride();
  pPelSrc += (cMv.getHor()>>1) + (cMv.getVer()>>1) * iSrcStride;

  uiXSize++;
  UInt x, y;
  for( y = 0; y < uiYSize; y++)
  {
    for( x = 0; x < uiXSize; x++)
    {
      Int x4 = x<<1;
      pPelDes[3*X1*17] = pPelSrc[x4-iSrcStride-1],
      pPelDes[1*X1*17] = pPelSrc[x4-iSrcStride],
      pPelDes++;
    }
    pPelDes -= uiXSize;
    for( x = 0; x < uiXSize; x++)
    {
      Int x4 = x<<1;
      pPelDes[2*X1*17] = pPelSrc[x4-1];
      pPelDes[0*X1*17] = pPelSrc[x4],
      pPelDes++;
    }
    pPelDes += X1-uiXSize;
    pPelSrc += 2*iSrcStride;
  }

  for( x = 0; x < uiXSize; x++)
  {
    Int x4 = x<<1;
    pPelDes[3*X1*17] = pPelSrc[x4-iSrcStride-1],
    pPelDes[1*X1*17] = pPelSrc[x4-iSrcStride],
    pPelDes++;
  }
  return;
}


ErrVal MotionEstimationQuarterPel::compensateBlock( IntYuvMbBuffer* pcRecPelData,
                                                    UInt            uiBlk,
                                                    UInt            uiMode,
                                                    IntYuvMbBuffer* pcRefPelData2 )
{
  pcRecPelData->set4x4Block( B4x4Idx(uiBlk) );
  XPel iStride = pcRecPelData->getLStride();
  XPel* pDes = pcRecPelData->getLumBlk();
  XPel* pSrc;
  Int iAdd;

  if( 0x10 > m_uiBestMode )
  {
    pSrc = m_apXHPelSearch[m_uiBestMode];
    iAdd = X1;
  }
  else
  {
    pSrc = m_apXQPelSearch[m_uiBestMode>>4];
    iAdd = 16;
  }

  UInt uiXSize = 0;
  UInt uiYSize = 0;
  xGetSizeFromMode( uiXSize, uiYSize, uiMode);

  if( pcRefPelData2 == NULL )
  {
    for( UInt y = 0; y < uiYSize; y++)
    {
      for( UInt x = 0; x < uiXSize; x+=4 )
      {
        pDes[x+0] = pSrc[x+0];
        pDes[x+1] = pSrc[x+1];
        pDes[x+2] = pSrc[x+2];
        pDes[x+3] = pSrc[x+3];
      }
      pDes += iStride;
      pSrc += iAdd;
    }
  }
  else
  {
    pcRefPelData2->set4x4Block( B4x4Idx(uiBlk) );
    XPel iStride2 = pcRefPelData2->getLStride();
    XPel* pSrc2 = pcRefPelData2->getLumBlk();
    for( UInt y = 0; y < uiYSize; y++)
    {
      for( UInt x = 0; x < uiXSize; x+=4 )
      {
        pDes[x+0] = (pSrc[x+0] + pSrc2[x+0] + 1)>>1;
        pDes[x+1] = (pSrc[x+1] + pSrc2[x+1] + 1)>>1;
        pDes[x+2] = (pSrc[x+2] + pSrc2[x+2] + 1)>>1;
        pDes[x+3] = (pSrc[x+3] + pSrc2[x+3] + 1)>>1;
      }
      pDes  += iStride;
      pSrc  += iAdd;
      pSrc2 += iStride2;
    }

  }
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
