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

#if !defined(AFX_CONTROLMNGH264AVCENCODER_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_)
#define AFX_CONTROLMNGH264AVCENCODER_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/ControlMngIf.h"
#include "MbSymbolWriteIf.h"
#include "H264AVCEncoder.h"
#include "H264AVCCommonLib/MbData.h"
#include "BitWriteBuffer.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/SampleWeighting.h"
#include "H264AVCCommonLib/PocCalculator.h"

#include "SliceEncoder.h"
#include "UvlcWriter.h"
#include "MbCoder.h"
#include "MbEncoder.h"
#include "IntraPredictionSearch.h"
#include "CodingParameter.h"
#include "CabacWriter.h"
#include "NalUnitEncoder.h"
#include "Distortion.h"
#include "MotionEstimation.h"
#include "MotionEstimationQuarterPel.h"
#include "RateDistortion.h"
#include "GOPEncoder.h"

#include "H264AVCCommonLib/CFMO.h"

H264AVC_NAMESPACE_BEGIN

class ControlMngH264AVCEncoder : public ControlMngIf
{
protected:
	ControlMngH264AVCEncoder();
	virtual ~ControlMngH264AVCEncoder();

public:
  static ErrVal create( ControlMngH264AVCEncoder*& rpcControlMngH264AVCEncoder );
  ErrVal init(  LayerEncoder*           apcLayerEncoder         [MAX_LAYERS],
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
                RateDistortion*         pcRateDistortion );

  ErrVal uninit();
  ErrVal destroy();


  ErrVal initSlice0       (SliceHeader *rcSH)                     { return Err::m_nERR; }

  // TMM_ESS
  ErrVal initSPS          ( SequenceParameterSet&       rcSPS, UInt  uiLayer   ) { return Err::m_nERR; }

  ErrVal initParameterSets( const SequenceParameterSet& rcSPS,
                            const PictureParameterSet&  rcPPS );

  ErrVal initMbForFiltering( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex );

  ErrVal finishSlice( const SliceHeader& rcSH, Bool& rbPicDone, Bool& rbFrameDone );

  ErrVal initMbForParsing( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex ) { return Err::m_nERR; }
  ErrVal initMbForDecoding( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff ) { return Err::m_nERR; }

  ErrVal initSliceForCoding   ( const SliceHeader& rcSH );
  ErrVal initSliceForReading  ( const SliceHeader& rcSH ) { return Err::m_nERR; }
  ErrVal initSliceForDecoding ( const SliceHeader& rcSH ) { return Err::m_nERR; }
  ErrVal initSliceForFiltering( const SliceHeader& rcSH );

 ErrVal initMbForCoding      ( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff, Bool bFieldFlag );
 ErrVal initMbForDecoding    ( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff ) { return Err::m_nERR; };

  ErrVal initMbForFiltering( MbDataAccess& rcMbDataAccess,   UInt uiMbY, UInt uiMbX, Bool bMbAff );
  ErrVal initMbForFiltering( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff );

  UvlcWriter*  getUvlcWriter()  { return m_pcUvlcWriter;  };
  CabacWriter* getCabacWriter() { return m_pcCabacWriter; };

//TMM_WP
  ErrVal initSliceForWeighting ( const SliceHeader& rcSH);
//TMM_WP
//TMM_INTERLACE{
  virtual ErrVal removeFrameFieldBuffer   ( ) { return Err::m_nERR; }
//TMM_INTERLACE}

  MbDataCtrl* getMbDataCtrl() { return m_pcMbDataCtrl;} //JVT-T054
  //--TM 0109.2006
  const FMO* getFMO() const {return m_pcFMO;}
  Void setFMO( const FMO* fmo){m_pcFMO = fmo;}

protected:
  LayerEncoder*           m_apcLayerEncoder         [MAX_LAYERS];
  SliceEncoder*           m_pcSliceEncoder;
  ControlMngH264AVCEncoder*  m_pcControlMng;
  BitWriteBuffer*         m_pcBitWriteBuffer;
  BitCounter*             m_pcBitCounter;
  NalUnitEncoder*         m_pcNalUnitEncoder;
  UvlcWriter*             m_pcUvlcWriter;
  UvlcWriter*             m_pcUvlcTester;
  MbCoder*                m_pcMbCoder;
  LoopFilter*             m_pcLoopFilter;
  MbEncoder*              m_pcMbEncoder;
  Transform*              m_pcTransform;
  IntraPredictionSearch*  m_pcIntraPrediction;
  YuvBufferCtrl*          m_apcYuvFullPelBufferCtrl [MAX_LAYERS];
  YuvBufferCtrl*          m_apcYuvHalfPelBufferCtrl [MAX_LAYERS];
  QuarterPelFilter*       m_pcQuarterPelFilter;
  CodingParameter*        m_pcCodingParameter;
  ParameterSetMng*        m_pcParameterSetMng;
  PocCalculator*          m_apcPocCalculator        [MAX_LAYERS];
  SampleWeighting*        m_pcSampleWeighting;
  CabacWriter*            m_pcCabacWriter;
  XDistortion*            m_pcXDistortion;
  MotionEstimation*       m_pcMotionEstimation;
  RateDistortion*         m_pcRateDistortion;

  MbDataCtrl*             m_pcMbDataCtrl;
  MbSymbolWriteIf*        m_pcMbSymbolWriteIf;
  UInt                    m_auiMbXinFrame           [MAX_LAYERS];
  UInt                    m_auiMbYinFrame           [MAX_LAYERS];
  UInt                    m_uiCurrLayer;
  Bool                    m_bLayer0IsAVC;
  Bool                    m_bAVCMode;

  const FMO* m_pcFMO;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_CONTROLMNGH264AVCENCODER_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_)
