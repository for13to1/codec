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

#include "H264AVCEncoderLib.h"
#include "MCTF.h"
#include "MCTFPreProcessor.h"
#include "CodingParameter.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/SampleWeighting.h"
#include "../../lib/H264AVCEncoderLib/MbEncoder.h"
#include "../../lib/H264AVCEncoderLib/IntraPredictionSearch.h"
#include "../../lib/H264AVCEncoderLib/MotionEstimation.h"
#include "../../lib/H264AVCEncoderLib/MotionEstimationQuarterPel.h"
#include "../../lib/H264AVCEncoderLib/RateDistortion.h"


H264AVC_NAMESPACE_BEGIN


MCTFPreProcessor::MCTFPreProcessor()
: m_pcMCTF                  ( NULL )
, m_pcMbEncoder             ( NULL )
, m_pcTransform             ( NULL )
, m_pcIntraPrediction       ( NULL )
, m_pcYuvFullPelBufferCtrl  ( NULL )
, m_pcYuvHalfPelBufferCtrl  ( NULL )
, m_pcQuarterPelFilter      ( NULL )
, m_pcSampleWeighting       ( NULL )
, m_pcXDistortion           ( NULL )
, m_pcMotionEstimation      ( NULL )
, m_pcRateDistortion        ( NULL )
{
}


MCTFPreProcessor::~MCTFPreProcessor()
{
}


ErrVal
MCTFPreProcessor::process( PicBuffer*     pcOriginalPicBuffer,
                           PicBuffer*     pcReconstructPicBuffer,
                           PicBufferList& rcPicBufferOutputList,
                           PicBufferList& rcPicBufferUnusedList )
{
  if( pcOriginalPicBuffer )
  {
    RNOK( m_pcMCTF->process( pcOriginalPicBuffer, pcReconstructPicBuffer,
                             rcPicBufferOutputList, rcPicBufferUnusedList ) );
  }
  else if( pcReconstructPicBuffer )
  {
    rcPicBufferUnusedList.push_back( pcReconstructPicBuffer );
  }
  return Err::m_nOK;
}


ErrVal
MCTFPreProcessor::finish( PicBufferList&  rcPicBufferOutputList,
                          PicBufferList&  rcPicBufferUnusedList )
{
  RNOK( m_pcMCTF->process( NULL, NULL,
                           rcPicBufferOutputList, rcPicBufferUnusedList ) );
  return Err::m_nOK;
}


ErrVal
MCTFPreProcessor::create( MCTFPreProcessor*& rpcMCTFPreProcessor )
{
  rpcMCTFPreProcessor = new MCTFPreProcessor;
  ROF ( rpcMCTFPreProcessor );
  RNOK( rpcMCTFPreProcessor->xCreateMCTFPreProcessor() );
  return Err::m_nOK;
}


ErrVal
MCTFPreProcessor::xCreateMCTFPreProcessor()
{
  RNOK( MCTF                        ::create( m_pcMCTF ) );
  RNOK( MbEncoder                   ::create( m_pcMbEncoder ) );
  RNOK( IntraPredictionSearch       ::create( m_pcIntraPrediction ) );
  RNOK( MotionEstimationQuarterPel  ::create( m_pcMotionEstimation ) );
  RNOK( QuarterPelFilter            ::create( m_pcQuarterPelFilter ) );
  RNOK( Transform                   ::create( m_pcTransform ) );
  RNOK( SampleWeighting             ::create( m_pcSampleWeighting ) );
  RNOK( XDistortion                 ::create( m_pcXDistortion ) );
  RNOK( YuvBufferCtrl               ::create( m_pcYuvFullPelBufferCtrl ) );
  RNOK( YuvBufferCtrl               ::create( m_pcYuvHalfPelBufferCtrl ) );
  return Err::m_nOK;
}


ErrVal
MCTFPreProcessor::destroy()
{
  RNOK( m_pcMbEncoder             ->destroy() );
  RNOK( m_pcTransform             ->destroy() );
  RNOK( m_pcIntraPrediction       ->destroy() );
  RNOK( m_pcQuarterPelFilter      ->destroy() );
  RNOK( m_pcXDistortion           ->destroy() );
  RNOK( m_pcMotionEstimation      ->destroy() );
  RNOK( m_pcSampleWeighting       ->destroy() );
  RNOK( m_pcMCTF                  ->destroy() );
  RNOK( m_pcYuvFullPelBufferCtrl  ->destroy() );
  RNOK( m_pcYuvHalfPelBufferCtrl  ->destroy() );
  if( NULL != m_pcRateDistortion )
  {
    RNOK( m_pcRateDistortion      ->destroy() );
  }
  delete this;
  return Err::m_nOK;
}


ErrVal
MCTFPreProcessor::init( PreProcessorParameter* pcParameter,
                        CodingParameter*       pcCodingParameter )
{
  ROF( pcParameter );
  ROF( pcCodingParameter );

  RNOK( RateDistortion::create( m_pcRateDistortion ) );
  RNOK( m_pcXDistortion       ->init() );
  RNOK( m_pcSampleWeighting   ->init() );
  RNOK( m_pcQuarterPelFilter  ->init() );
  RNOK( m_pcMbEncoder         ->init( m_pcTransform,
                                      m_pcIntraPrediction,
                                      m_pcMotionEstimation,
                                      pcCodingParameter,
                                      m_pcRateDistortion,
                                      m_pcXDistortion ) );
  RNOK( m_pcMotionEstimation  ->init( m_pcXDistortion,
                                      pcCodingParameter,
                                      m_pcRateDistortion,
                                      m_pcQuarterPelFilter,
                                      m_pcTransform,
                                      m_pcSampleWeighting) );
  RNOK( m_pcMCTF              ->init( pcParameter,
                                      m_pcMbEncoder,
                                      m_pcYuvFullPelBufferCtrl,
                                      m_pcYuvHalfPelBufferCtrl,
                                      m_pcQuarterPelFilter,
                                      m_pcMotionEstimation ) );
  return Err::m_nOK;
}


ErrVal
MCTFPreProcessor::uninit()
{
  RNOK( m_pcQuarterPelFilter      ->uninit() );
  RNOK( m_pcSampleWeighting       ->uninit() );
  RNOK( m_pcMbEncoder             ->uninit() );
  RNOK( m_pcIntraPrediction       ->uninit() );
  RNOK( m_pcMotionEstimation      ->uninit() );
  RNOK( m_pcMotionEstimation      ->uninit() );
  RNOK( m_pcXDistortion           ->uninit() );
  RNOK( m_pcMCTF                  ->uninit() );
  RNOK( m_pcYuvFullPelBufferCtrl  ->uninit() );
  RNOK( m_pcYuvHalfPelBufferCtrl  ->uninit() );
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END

