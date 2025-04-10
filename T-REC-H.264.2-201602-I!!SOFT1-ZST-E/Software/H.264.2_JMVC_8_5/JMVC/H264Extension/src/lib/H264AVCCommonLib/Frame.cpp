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


#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/FrameUnit.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"


H264AVC_NAMESPACE_BEGIN



  Frame::Frame( YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& rcYuvHalfPelBufferCtrl, PicType ePicType ):
  m_cFullPelYuvBuffer( rcYuvFullPelBufferCtrl, ePicType ),
      m_cHalfPelYuvBuffer( rcYuvHalfPelBufferCtrl, ePicType ),
      m_ePicType( ePicType ),
  m_pcFrameUnit( NULL ),
  m_iStamp( 0 )
{
  m_bPOCisSet = false;
  m_iPOC      = 0;
  m_uiViewId = 0;
}

Frame::~Frame()
{
}


ErrVal Frame::init( Pel* pucYuvBuffer, FrameUnit* pcFrameUnit )
{
  ROF( pcFrameUnit )

  m_pcFrameUnit = pcFrameUnit;
  m_bPOCisSet   = false;

  RNOK( m_cFullPelYuvBuffer.init( pucYuvBuffer ) );

  return Err::m_nOK;
}


ErrVal Frame::uninit()
{
  RNOK( m_cFullPelYuvBuffer.uninit() );
  RNOK( m_cHalfPelYuvBuffer.uninit() );

  m_bPOCisSet   = false;
  m_iPOC        = 0;
  m_pcFrameUnit = NULL;
  m_iStamp++;  // !!!

  return Err::m_nOK;
}

Bool Frame::isShortTerm() const
{
    return m_pcFrameUnit->isShortTerm( m_ePicType );
}

const Bool Frame::isUsed() const
{
    return m_pcFrameUnit->isUsed( m_ePicType );
}

ErrVal
Frame::extendFrame( QuarterPelFilter* pcQuarterPelFilter, Bool bFrameMbsOnly, Bool bFGS )
{
    Bool bNoHalfPel = ( NULL == pcQuarterPelFilter );

    RNOK( getFullPelYuvBuffer()->fillMargin() );

    if( FRAME == getPicType() )
    {
        Frame* pcTopField = NULL;
        Frame* pcBotField = NULL;

        pcTopField = &getFrameUnit()->getTopField();
        pcBotField = &getFrameUnit()->getBotField();

		pcTopField->setViewId( this->getViewId() );
		pcBotField->setViewId( this->getViewId() );

        // speed up only
        if( ! bFrameMbsOnly )
        {
            // remove the default yuv memory from buffers
            RNOK( pcTopField->getFullPelYuvBuffer()->uninit() );
            RNOK( pcBotField->getFullPelYuvBuffer()->uninit() );

            // creates private full pel buffer
            Pel* pHalfPelField = NULL;
            RNOK( pcTopField->getFullPelYuvBuffer()->init( pHalfPelField ) );
            RNOK( pcBotField->getFullPelYuvBuffer()->init( pHalfPelField ) );

            // perform border padding on the full pel buffer
            RNOK( pcTopField->getFullPelYuvBuffer()->loadBufferAndFillMargin( getFullPelYuvBuffer() ) );
            RNOK( pcBotField->getFullPelYuvBuffer()->loadBufferAndFillMargin( getFullPelYuvBuffer() ) );
        }

        // if cond is true no sub pel buffer is used
        ROTRS( bNoHalfPel, Err::m_nOK );

        // creates private half pel buffer
        Pel* pFullPelFrame = NULL;
        RNOK( getHalfPelYuvBuffer()->init( pFullPelFrame ) );
        // create half pel samples
        ANOK( pcQuarterPelFilter->filterFrame( getFullPelYuvBuffer(), getHalfPelYuvBuffer() ) );

        // speed up only
        if( ! bFrameMbsOnly )
        {
            // creates private half pel buffer
            Pel* pHalfPelField = NULL;
            RNOK( pcTopField->getHalfPelYuvBuffer()->init( pHalfPelField ) );
            RNOK( pcBotField->getHalfPelYuvBuffer()->init( pHalfPelField ) );

            // create half pel samples
            ANOK( pcQuarterPelFilter->filterFrame( pcTopField->getFullPelYuvBuffer(),
                pcTopField->getHalfPelYuvBuffer() ) );

            // create half pel samples
            ANOK( pcQuarterPelFilter->filterFrame( pcBotField->getFullPelYuvBuffer(),
                pcBotField->getHalfPelYuvBuffer() ) );
        }
    }
    else
    {
        AOT( bFGS );
        Frame* pcFramePic = &getFrameUnit()->getFrame();

        if( pcFramePic->getFullPelYuvBuffer()->isValid() )
        {
            // first field
            // remove the default yuv memory from buffers
			//if(BOT_FIELD == getPicType())
			//{
			//	// perform border padding on the full pel buffer
			//	RNOK( pcFramePic->getFullPelYuvBuffer()->fillMargin());
			//	
			//}
            RNOK( pcFramePic->getFullPelYuvBuffer()->uninit() );

            // if cond is true no sub pel buffer is used
            ROTRS( bNoHalfPel, Err::m_nOK );

            Frame* pcTopField = &getFrameUnit()->getTopField();
            Frame* pcBotField = &getFrameUnit()->getBotField();

            // creates private half pel buffer
            Pel* pHalfPelField = NULL;
            RNOK( pcTopField->getHalfPelYuvBuffer()->init( pHalfPelField ) );
            RNOK( pcBotField->getHalfPelYuvBuffer()->init( pHalfPelField ) );

            // create half pel samples
            ANOK( pcQuarterPelFilter->filterFrame( getFullPelYuvBuffer(), getHalfPelYuvBuffer() ) );
        }
        else
        {
            // second field
            // creates private full pel buffer
            Pel* pFullPelFrame = NULL;
            RNOK( pcFramePic->getFullPelYuvBuffer()->init( pFullPelFrame ) );

            // perform border padding on the full pel buffer
            RNOK( pcFramePic->getFullPelYuvBuffer()->loadBufferAndFillMargin( getFullPelYuvBuffer() ) );
			

            // if cond is true no sub pel buffer is used
            ROTRS( bNoHalfPel, Err::m_nOK );

            // creates private half pel buffer
            Pel* pHalfPelFrame = NULL;
            RNOK( pcFramePic->getHalfPelYuvBuffer()->init( pHalfPelFrame ) );
            // create half pel samples
            ANOK( pcQuarterPelFilter->filterFrame( pcFramePic->getFullPelYuvBuffer(),
                pcFramePic->getHalfPelYuvBuffer() ) );

            // create half pel samples
            ANOK( pcQuarterPelFilter->filterFrame( getFullPelYuvBuffer(),
                getHalfPelYuvBuffer() ) );
        }
    }
    return Err::m_nOK;
}



H264AVC_NAMESPACE_END
