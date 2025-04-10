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
#include "H264AVCCommonLib/ReconstructionBypass.h"


#include "H264AVCCommonLib/IntFrame.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"

H264AVC_NAMESPACE_BEGIN

ReconstructionBypass::ReconstructionBypass() 
{
}

ErrVal ReconstructionBypass::create( ReconstructionBypass*& rpcReconstructionBypass )
{
  rpcReconstructionBypass = new ReconstructionBypass;
  ROT( NULL == rpcReconstructionBypass ) ;
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::init()
{
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::uninit()
{
  return Err::m_nOK;
}
 
ErrVal ReconstructionBypass::padRecFrame( IntFrame* pcIntFrame, const MbDataCtrl* pcMbDataCtrl, YuvBufferCtrl* pcYuvFullPelBufferCtrl, UInt uiFrameWidthInMb, UInt uiFrameHeightInMb )
{
  IntYuvPicBuffer* pcIntYuvPicBuffer = pcIntFrame->getFullPelYuvBuffer();

  // loop over macroblocks
  for( UInt uiMbY = 0; uiMbY < uiFrameWidthInMb; uiMbY++ )
  {
    for( UInt uiMbX = 0; uiMbX < uiFrameHeightInMb; uiMbX++ )
    {
      UInt uiMask = 0;

      //===== init macroblock =====
      RNOK( pcMbDataCtrl   ->getBoundaryMask( uiMbY, uiMbX, uiMask ) );
      RNOK( pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX ) );

      if( uiMask )
      {
		    IntYuvMbBufferExtension cBuffer;
		    cBuffer.loadSurrounding( pcIntYuvPicBuffer );

        RNOK( padRecMb( &cBuffer, uiMask ) );

  	    pcIntYuvPicBuffer->loadBuffer( &cBuffer );
      }
    }
  }

  return Err::m_nOK;
}

ErrVal ReconstructionBypass::padRecMb( IntYuvMbBufferExtension* pcBuffer, UInt uiMask )
{

  Bool bAboveIntra      = 0 != (uiMask & 0x01);
  Bool bBelowIntra      = 0 != (uiMask & 0x10);
  Bool bLeftIntra       = 0 != (uiMask & 0x40);
  Bool bRightIntra      = 0 != (uiMask & 0x04);
  Bool bLeftAboveIntra  = 0 != (uiMask & 0x80);
  Bool bRightAboveIntra = 0 != (uiMask & 0x02);
  Bool bLeftBelowIntra  = 0 != (uiMask & 0x20);
  Bool bRightBelowIntra = 0 != (uiMask & 0x08);

  for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    switch( cIdx.b8x8Index() )
    {
    case 0:
      {
        if( bAboveIntra )
        {
          if( bLeftIntra )
          {
            pcBuffer->mergeFromLeftAbove( cIdx, bLeftAboveIntra );
          }
          else
          {
            pcBuffer->copyFromAbove( cIdx );
          }
        }
        else
        {
          if( bLeftIntra )
          {
            pcBuffer->copyFromLeft( cIdx );
          }
          else if( bLeftAboveIntra )
          {
            pcBuffer->copyFromLeftAbove( cIdx );
          }
        }
      }
      break;
    case 1:
      {
        if( bAboveIntra )
        {
          if( bRightIntra )
          {
            pcBuffer->mergeFromRightAbove( cIdx, bRightAboveIntra );
          }
          else
          {
            pcBuffer->copyFromAbove( cIdx );
          }
        }
        else
        {
          if( bRightIntra )
          {
            pcBuffer->copyFromRight( cIdx );
          }
          else if( bRightAboveIntra )
          {
            pcBuffer->copyFromRightAbove( cIdx );
          }
        }
      }
      break;
    case 2:
      {
        if( bBelowIntra )
        {
          if( bLeftIntra )
          {
            pcBuffer->mergeLeftBelow( cIdx, bLeftBelowIntra );
          }
          else
          {
            pcBuffer->copyFromBelow( cIdx );
          }
        }
        else
        {
          if( bLeftIntra )
          {
            pcBuffer->copyFromLeft( cIdx );
          }
          else if( bLeftBelowIntra )
          {
            pcBuffer->copyFromLeftBelow( cIdx );
          }
        }
      }
      break;
    case 3:
      {
        if( bBelowIntra )
        {
          if( bRightIntra )
          {
            pcBuffer->mergeRightBelow( cIdx, bRightBelowIntra );
          }
          else
          {
            pcBuffer->copyFromBelow( cIdx );
          }
        }
        else
        {
          if( bRightIntra )
          {
            pcBuffer->copyFromRight( cIdx );
          }
          else if( bRightBelowIntra )
          {
            pcBuffer->copyFromRightBelow( cIdx );
          }
        }
      }
      break;
    default:
      break;
    }
  }

  return Err::m_nOK;
}

H264AVC_NAMESPACE_END




