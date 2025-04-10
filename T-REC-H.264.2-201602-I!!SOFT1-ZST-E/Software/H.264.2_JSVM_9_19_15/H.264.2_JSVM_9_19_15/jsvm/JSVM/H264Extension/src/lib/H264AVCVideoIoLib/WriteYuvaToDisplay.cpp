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
#include "WriteYuvaToDisplay.h"
#include "WriteYuvaToRgb.h"
#include "WriteYuvaToRgbMMX.h"
#include "Display.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


#ifndef NO_MMX_SUPPORT
#include "CpuInfoIf.h"
#endif


ErrVal WriteYuvaToDisplay::xUseMMX()
{
#ifndef NO_MMX_SUPPORT

  CpuInfoIf* pcCpuInfo = NULL;

  RNOK( CpuInfoIf::create( pcCpuInfo ) );

  m_bMMX = (0 < pcCpuInfo->hasMmx());

  delete pcCpuInfo;
  pcCpuInfo = NULL;
#endif
  return Err::m_nOK;
}

WriteYuvaToDisplay::WriteYuvaToDisplay() :
  m_pcDisplay( NULL ),
  m_pcWriteYuvToRgb( NULL ),
  m_uiHeight( 0 ),
  m_uiWidth( 0 ),
  m_bInitDone( false ),
  m_bMMX( false )
{

}

WriteYuvaToDisplay::~WriteYuvaToDisplay()
{

}


ErrVal WriteYuvaToDisplay::create( WriteYuvIf*& rpcWriteYuv, const std::string& rcTitle )
{
  WriteYuvaToDisplay* pcWriteYuvaToDisplay;

  pcWriteYuvaToDisplay = new WriteYuvaToDisplay;

  rpcWriteYuv = pcWriteYuvaToDisplay;

  ROT( NULL == pcWriteYuvaToDisplay );

  pcWriteYuvaToDisplay->m_cTitle = rcTitle;

  RNOK( pcWriteYuvaToDisplay->xUseMMX() );
  return Err::m_nOK;
}


ErrVal WriteYuvaToDisplay::destroy()
{
  if( m_pcDisplay )
  {
    RNOK( m_pcDisplay->destroy() );
  }

  if( m_pcWriteYuvToRgb )
  {
    RNOK( m_pcWriteYuvToRgb->destroy() );
  }

  delete this;

  return Err::m_nOK;
}


ErrVal WriteYuvaToDisplay::xSetFrameDimension( UInt uiLumHeight, UInt uiLumWidth )
{
  //ROT( m_bInitDone );

  ROTRS( 0 == uiLumHeight || 0 ==  uiLumWidth, Err::m_nInvalidParameter );
  ROTRS( m_uiHeight == uiLumHeight && m_uiWidth ==  uiLumWidth, Err::m_nOK );

  if( m_pcDisplay )
  {
    RNOK( m_pcDisplay->destroy() );
  }

  if( m_pcWriteYuvToRgb )
  {
    RNOK( m_pcWriteYuvToRgb->destroy() );
  }

  m_uiHeight = uiLumHeight;
  m_uiWidth =  uiLumWidth;

  if( m_bMMX && 0 == (uiLumWidth & 7))
  {
    RNOK( WriteYuvaToRgbMMX::create( m_pcWriteYuvToRgb ) );
  }
  else
  {
    RNOK( WriteYuvaToRgb::create( m_pcWriteYuvToRgb ) );
  }

  RNOK( DisplayRGB::create( m_pcDisplay, m_uiWidth, m_uiHeight, m_cTitle ) );

  RNOK( m_pcWriteYuvToRgb->setFrameDimension( m_uiHeight, m_uiWidth ) );
  m_bInitDone = true;

  return Err::m_nOK;
}


ErrVal WriteYuvaToDisplay::writeFrame( const UChar *pLum,
                                       const UChar *pCb,
                                       const UChar *pCr,
                                       UInt uiLumHeight,
                                       UInt uiLumWidth,
                                       UInt uiLumStride )
{

  RNOK( xSetFrameDimension( uiLumHeight, uiLumWidth ) );

  UChar  *pucDisplayBuffer;
  UInt  uiBufferStride;

  RNOK( m_pcDisplay->getDisplayBuffer( pucDisplayBuffer, uiBufferStride ) );

  ROF( m_bInitDone );
  if( ! m_pcDisplay->isHardwareAccelerationInUse() )
  {
    RNOK( m_pcWriteYuvToRgb->writeFrameRGB( pucDisplayBuffer, uiBufferStride,
                                            pLum, pCb, pCr,
                                            uiLumHeight, uiLumWidth, uiLumStride ) );
  }
  else if( m_pcDisplay->isHardwareAccelerationInUse() == 0x32595559 )
  {
    RNOK( m_pcWriteYuvToRgb->writeFrameYUYV( pucDisplayBuffer, uiBufferStride,
                                             pLum, pCb, pCr,
                                             uiLumHeight, uiLumWidth, uiLumStride ) );
  }
  else if( m_pcDisplay->isHardwareAccelerationInUse() == 0x32315659 )
  {
    RNOK( m_pcWriteYuvToRgb->writeFrameYV12( pucDisplayBuffer, uiBufferStride,
                                             pLum, pCb, pCr,
                                             uiLumHeight, uiLumWidth, uiLumStride ) );
  }
  else
  {
    AOT(1);
  }

  RNOK( m_pcDisplay->displayFrame( pucDisplayBuffer ) );

  return Err::m_nOK;
}


ErrVal WriteYuvaToDisplay::writeField( Bool bTop,
                                       const UChar *pLum,
                                       const UChar *pCb,
                                       const UChar *pCr,
                                       UInt uiLumHeight,
                                       UInt uiLumWidth,
                                       UInt uiLumStride )
{

  RNOK( xSetFrameDimension( uiLumHeight*2, uiLumWidth ) );

  UChar  *pucDisplayBuffer;
  UInt  uiBufferStride;

  RNOK( m_pcDisplay->getDisplayBuffer( pucDisplayBuffer, uiBufferStride ) );

  if( ! bTop )
  {
    pucDisplayBuffer += uiBufferStride;
  }


  ROF( m_bInitDone );
  if( ! m_pcDisplay->isHardwareAccelerationInUse() )
  {
    RNOK( m_pcWriteYuvToRgb->writeFrameRGB( pucDisplayBuffer, uiBufferStride*2,
                                            pLum, pCb, pCr,
                                            uiLumHeight, uiLumWidth, uiLumStride ) );
  }
  else if( m_pcDisplay->isHardwareAccelerationInUse() == 0x32595559 )
  {
    RNOK( m_pcWriteYuvToRgb->writeFrameYUYV( pucDisplayBuffer, uiBufferStride*2,
                                             pLum, pCb, pCr,
                                             uiLumHeight, uiLumWidth, uiLumStride ) );
  }
  else if( m_pcDisplay->isHardwareAccelerationInUse() == 0x32315659 )
  {
    RNOK( m_pcWriteYuvToRgb->writeFrameYV12( pucDisplayBuffer, uiBufferStride*2,
                                             pLum, pCb, pCr,
                                             uiLumHeight, uiLumWidth, uiLumStride ) );
  }
  else
  {
    AOT(1);
  }

  RNOK( m_pcDisplay->displayFrame( pucDisplayBuffer ) );

  return Err::m_nOK;
}

