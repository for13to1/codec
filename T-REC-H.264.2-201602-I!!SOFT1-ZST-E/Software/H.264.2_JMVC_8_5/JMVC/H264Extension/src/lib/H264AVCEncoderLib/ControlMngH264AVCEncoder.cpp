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
#include "ControlMngH264AVCEncoder.h"


H264AVC_NAMESPACE_BEGIN


ControlMngH264AVCEncoder::ControlMngH264AVCEncoder():
  m_pcFrameMng            ( NULL ),
  m_pcSliceEncoder        ( NULL ),
  m_pcControlMng          ( NULL ),
  m_pcBitWriteBuffer      ( NULL ),
  m_pcNalUnitEncoder      ( NULL ),
  m_pcUvlcWriter          ( NULL ),
  m_pcUvlcTester          ( NULL ),
  m_pcMbCoder             ( NULL ),
  m_pcLoopFilter          ( NULL ),
  m_pcMbEncoder           ( NULL ),
  m_pcQuarterPelFilter    ( NULL ),
  m_pcCodingParameter     ( NULL ),
  m_pcParameterSetMng     ( NULL ),
  m_pcSampleWeighting     ( NULL ),
  m_pcCabacWriter         ( NULL ),
  m_pcXDistortion         ( NULL ),
  m_pcMotionEstimation    ( NULL ),
  m_pcRateDistortion      ( NULL ),
  m_pcMbDataCtrl          ( NULL ),
  m_pcMbSymbolWriteIf     ( NULL )
{
  ::memset( m_apcYuvFullPelBufferCtrl, 0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_apcYuvHalfPelBufferCtrl, 0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_apcPocCalculator,        0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_auiMbXinFrame,           0x00, MAX_LAYERS*sizeof(UInt ) );
  ::memset( m_auiMbYinFrame,           0x00, MAX_LAYERS*sizeof(UInt ) );
}

ControlMngH264AVCEncoder::~ControlMngH264AVCEncoder()
{
}


ErrVal
ControlMngH264AVCEncoder::initParameterSets( const SequenceParameterSet&  rcSPS,
                                             const PictureParameterSet&   rcPPSLP,
                                             const PictureParameterSet&   rcPPSHP )
{
  UInt  uiLayer             = rcSPS.getLayerId          ();
  UInt  uiMbX               = rcSPS.getFrameWidthInMbs  ();
  UInt  uiMbY               = rcSPS.getFrameHeightInMbs ();
  m_auiMbXinFrame[uiLayer]  = uiMbX;
  m_auiMbYinFrame[uiLayer]  = uiMbY;

  //===== initialize buffer controls and MCTFEncoder =====
  RNOK( m_apcYuvFullPelBufferCtrl[uiLayer]->initSPS( uiMbY<<4, uiMbX<<4, YUV_Y_MARGIN, YUV_X_MARGIN    ) );
  RNOK( m_apcYuvHalfPelBufferCtrl[uiLayer]->initSPS( uiMbY<<4, uiMbX<<4, YUV_Y_MARGIN, YUV_X_MARGIN, 1 ) );

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCEncoder::create( ControlMngH264AVCEncoder*& rpcControlMngH264AVCEncoder )
{
  rpcControlMngH264AVCEncoder = new ControlMngH264AVCEncoder;

  ROT( NULL == rpcControlMngH264AVCEncoder );

  return Err::m_nOK;
}

ErrVal ControlMngH264AVCEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal ControlMngH264AVCEncoder::uninit()
{
  m_pcFrameMng = NULL;
  m_pcSliceEncoder = NULL;
  m_pcControlMng = NULL;
  m_pcBitWriteBuffer = NULL;
  m_pcBitCounter = NULL;
  m_pcNalUnitEncoder = NULL;
  m_pcUvlcWriter = NULL;
  m_pcUvlcTester = NULL;
  m_pcMbCoder = NULL;
  m_pcLoopFilter = NULL;
  m_pcMbEncoder = NULL;
  m_pcTransform = NULL;
  m_pcIntraPrediction = NULL;
  m_pcQuarterPelFilter = NULL;
  m_pcCodingParameter = NULL;
  m_pcParameterSetMng = NULL;
  m_pcSampleWeighting = NULL;
  m_pcCabacWriter = NULL;
  m_pcXDistortion = NULL;
  m_pcMotionEstimation = NULL;
  m_pcRateDistortion = NULL;


  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_apcYuvFullPelBufferCtrl [uiLayer] = 0;
    m_apcYuvHalfPelBufferCtrl [uiLayer] = 0;
    m_apcPocCalculator        [uiLayer] = 0;
  }


  return Err::m_nOK;
}

ErrVal ControlMngH264AVCEncoder::init( FrameMng*               pcFrameMng,
                                    SliceEncoder*           pcSliceEncoder,
                                    ControlMngH264AVCEncoder*  pcControlMng,
                                    BitWriteBuffer*         pcBitWriteBuffer,
                                    BitCounter*             pcBitCounter,
                                    NalUnitEncoder*         pcNalUnitEncoder,
                                    UvlcWriter*             pcUvlcWriter,
                                    UvlcWriter*             pcUvlcTester,
                                    MbCoder*                pcMbCoder,
                                    LoopFilter*             pcLoopFilter,
                                    MbEncoder*              pcMbEncoder,
                                    Transform*              pcTransform,
                                    IntraPredictionSearch*  pcIntraPrediction,
                                    YuvBufferCtrl*          apcYuvFullPelBufferCtrl [MAX_LAYERS],
                                    YuvBufferCtrl*          apcYuvHalfPelBufferCtrl [MAX_LAYERS],
                                    QuarterPelFilter*       pcQuarterPelFilter,
                                    CodingParameter*        pcCodingParameter,
                                    ParameterSetMng*        pcParameterSetMng,
                                    PocCalculator*          apcPocCalculator        [MAX_LAYERS],
                                    SampleWeighting*        pcSampleWeighting,
                                    CabacWriter*            pcCabacWriter,
                                    XDistortion*            pcXDistortion,
                                    MotionEstimation*       pcMotionEstimation,
                                    RateDistortion*         pcRateDistortion )
{
  ROT( NULL == pcFrameMng);
  ROT( NULL == pcSliceEncoder);
  ROT( NULL == pcControlMng);
  ROT( NULL == pcBitWriteBuffer);
  ROT( NULL == pcBitCounter);
  ROT( NULL == pcNalUnitEncoder);
  ROT( NULL == pcUvlcWriter);
  ROT( NULL == pcUvlcTester);
  ROT( NULL == pcMbCoder);
  ROT( NULL == pcLoopFilter);
  ROT( NULL == pcMbEncoder);
  ROT( NULL == pcTransform);
  ROT( NULL == pcIntraPrediction);
  ROT( NULL == pcQuarterPelFilter);
  ROT( NULL == pcCodingParameter);
  ROT( NULL == pcParameterSetMng);
  ROT( NULL == pcSampleWeighting);
  ROT( NULL == pcCabacWriter);
  ROT( NULL == pcXDistortion);
  ROT( NULL == pcMotionEstimation);
  ROT( NULL == pcRateDistortion);


  m_pcFrameMng          = pcFrameMng;
  m_pcSliceEncoder      = pcSliceEncoder;
  m_pcControlMng        = pcControlMng;
  m_pcBitWriteBuffer    = pcBitWriteBuffer;
  m_pcBitCounter        = pcBitCounter;
  m_pcNalUnitEncoder    = pcNalUnitEncoder;
  m_pcUvlcWriter        = pcUvlcWriter;
  m_pcUvlcTester        = pcUvlcTester;
  m_pcMbCoder           = pcMbCoder;
  m_pcLoopFilter        = pcLoopFilter;
  m_pcMbEncoder         = pcMbEncoder;
  m_pcTransform         = pcTransform;
  m_pcIntraPrediction   = pcIntraPrediction;
  m_pcQuarterPelFilter  = pcQuarterPelFilter;
  m_pcCodingParameter   = pcCodingParameter;
  m_pcParameterSetMng   = pcParameterSetMng;
  m_pcSampleWeighting   = pcSampleWeighting;
  m_pcCabacWriter       = pcCabacWriter;
  m_pcXDistortion       = pcXDistortion;
  m_pcMotionEstimation  = pcMotionEstimation;
  m_pcRateDistortion    = pcRateDistortion;


  ROT( NULL == apcPocCalculator );
  ROT( NULL == apcYuvFullPelBufferCtrl );
  ROT( NULL == apcYuvHalfPelBufferCtrl );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    ROT( NULL == apcPocCalculator       [uiLayer] );
    ROT( NULL == apcYuvFullPelBufferCtrl[uiLayer] );
    ROT( NULL == apcYuvHalfPelBufferCtrl[uiLayer] );

    m_apcPocCalculator        [uiLayer] = apcPocCalculator        [uiLayer];
    m_apcYuvFullPelBufferCtrl [uiLayer] = apcYuvFullPelBufferCtrl [uiLayer];
    m_apcYuvHalfPelBufferCtrl [uiLayer] = apcYuvHalfPelBufferCtrl [uiLayer];
  }

  m_bMVCMode = ( pcCodingParameter->getMVCmode() != 0 );

  return Err::m_nOK;
}




ErrVal ControlMngH264AVCEncoder::finishSlice( const SliceHeader& rcSH, Bool& rbPicDone, Bool& rbFrameDone )
{
  ROT( NULL == m_pcMbSymbolWriteIf ) ;

  RNOK( m_pcMbSymbolWriteIf->finishSlice() );

  m_pcMbSymbolWriteIf = NULL;

  rbPicDone   = m_pcMbDataCtrl->isPicDone( rcSH );
  rbFrameDone = m_pcMbDataCtrl->isFrameDone( rcSH );

  return Err::m_nOK;
}



ErrVal ControlMngH264AVCEncoder::initSliceForCoding( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getSPS().getLayerId();

  Bool bCabac = rcSH.getPPS().getEntropyCodingModeFlag();
  if( bCabac )
  {
    m_pcMbSymbolWriteIf = m_pcCabacWriter;
  }
  else
  {
    m_pcMbSymbolWriteIf = m_pcUvlcWriter;
  }

  RNOK( m_pcMbSymbolWriteIf   ->startSlice( rcSH ) );
  RNOK( m_pcMbEncoder         ->initSlice ( rcSH ) )
  RNOK( m_pcMbCoder           ->initSlice ( rcSH, m_pcMbSymbolWriteIf, m_pcRateDistortion ) );
  RNOK( m_pcMotionEstimation  ->initSlice ( rcSH ) );
  RNOK( m_pcSampleWeighting   ->initSlice ( rcSH ) );

  setFMO( rcSH.getFMO());

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCEncoder::initSliceForFiltering( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getSPS().getLayerId();

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCEncoder::initSlice( SliceHeader& rcSH, ProcessingState eProcessingState )
{
  m_uiCurrLayer   = rcSH.getSPS().getLayerId();
  m_pcMbDataCtrl = rcSH.getFrameUnit()->getMbDataCtrl();

  RNOK( m_pcMbDataCtrl          ->initSlice( rcSH, eProcessingState, false, NULL ) );
  RNOK( m_pcMotionEstimation    ->initSlice( rcSH ) );
  RNOK( m_pcSampleWeighting     ->initSlice( rcSH ) );
  RNOK( m_pcMbEncoder           ->initSlice( rcSH ) );

  if( ENCODE_PROCESS == eProcessingState )
  {
    Bool bCabac = rcSH.getPPS().getEntropyCodingModeFlag();
    if( bCabac )
    {
      m_pcMbSymbolWriteIf = m_pcCabacWriter;
    }
    else
    {
      m_pcMbSymbolWriteIf = m_pcUvlcWriter;
    }

    RNOK( m_pcMbSymbolWriteIf->startSlice( rcSH ) );
    RNOK( m_pcMbCoder->initSlice( rcSH, m_pcMbSymbolWriteIf, m_pcRateDistortion ) );
  }

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCEncoder::initMbForCoding( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff, Bool bFieldFlag )
{
    ROF( m_uiCurrLayer < MAX_LAYERS );
    if( bMbAff )
    {
        rcMbDataAccess.setFieldMode( bFieldFlag );
    }
    else
    {
        //rpcMbDataAccess->getMbData().setFieldFlag( false ); // now: done above
        rcMbDataAccess.getMbMotionData( LIST_0 ).setFieldMode( false );
        rcMbDataAccess.getMbMotionData( LIST_1 ).setFieldMode( false );
    }

    RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb( uiMbY, uiMbX, bMbAff ) );
    RNOK( m_apcYuvHalfPelBufferCtrl[m_uiCurrLayer]->initMb( uiMbY, uiMbX, bMbAff ) );

    RNOK( m_pcMotionEstimation->initMb( uiMbY, uiMbX, rcMbDataAccess ) );

  return Err::m_nOK;
}

ErrVal ControlMngH264AVCEncoder::initMbForFiltering( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );

//  m_pcMbDataCtrl->initMb( rpcMbDataAccess, uiMbY, uiMbX );//test

  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb( uiMbY, uiMbX, bMbAff ) );
  return Err::m_nOK;
}

//TMM_WP
ErrVal ControlMngH264AVCEncoder::initSliceForWeighting ( const SliceHeader& rcSH)
{
   return m_pcSampleWeighting->initSlice( rcSH ); 
}
//TMM_WP

H264AVC_NAMESPACE_END
