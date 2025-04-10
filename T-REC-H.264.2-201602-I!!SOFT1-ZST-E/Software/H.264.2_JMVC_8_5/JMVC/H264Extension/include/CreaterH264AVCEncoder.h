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


#if !defined(AFX_CREATERH264AVCENCODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_)
#define AFX_CREATERH264AVCENCODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



H264AVC_NAMESPACE_BEGIN



class H264AVCEncoder;
class MbData;
class Frame;
class FrameMng;
class BitWriteBuffer;
class Transform;
class YuvBufferCtrl;
class QuarterPelFilter;
class ParameterSetMng;
class LoopFilter;
class SampleWeighting;
class PocCalculator;

class BitCounter;
class SliceEncoder;
class UvlcWriter;
class MbCoder;
class MbEncoder;
class IntraPredictionSearch;
class CodingParameter;
class CabacWriter;
class NalUnitEncoder;
class Distortion;
class MotionEstimation;
class MotionEstimationQuarterPel;
class RateDistortion;
class RateDistortionRateConstraint;
class History;
class XDistortion;
class ControlMngH264AVCEncoder;
class ReconstructionBypass;
class PicEncoder;



class H264AVCENCODERLIB_API CreaterH264AVCEncoder 
{
protected:
	CreaterH264AVCEncoder();
	virtual ~CreaterH264AVCEncoder();

public:
  static ErrVal create  ( CreaterH264AVCEncoder*& rpcCreaterH264AVCEncoder );
  ErrVal        destroy ();

  ErrVal init               ( CodingParameter*    pcCodingParameter);
  ErrVal uninit             ();
  ErrVal writeParameterSets ( ExtBinDataAccessor* pcExtBinDataAccessor,
                              Bool&               rbMoreSets );
//JVT-W080
	ErrVal writePDSSEIMessage ( ExtBinDataAccessor* pcExtBinDataAccessor
														, const UInt uiSPSId
														, const UInt uiNumView
													  , UInt* num_refs_list0_anc
														, UInt* num_refs_list1_anc 
													  , UInt* num_refs_list0_nonanc
														, UInt* num_refs_list1_nonanc 
														, UInt  PDSInitialDelayAnc
														, UInt  PDSInitialDelayNonAnc
														);
//~JVT-W080 
  ErrVal process(  ExtBinDataAccessorList&  rcExtBinDataAccessorList, 
                   PicBuffer*               apcOriginalPicBuffer    [MAX_LAYERS],
                   PicBuffer*               apcReconstructPicBuffer [MAX_LAYERS],
                   PicBufferList*           apcPicBufferOutputList,
                   PicBufferList*           apcPicBufferUnusedList );

  ErrVal finish (  ExtBinDataAccessorList&  rcExtBinDataAccessorList, 
                   PicBufferList*           apcPicBufferOutputList,
                   PicBufferList*           apcPicBufferUnusedList,
                   UInt&                    ruiNumCodedFrames,
                   Double&                  rdHighestLayerOutputRate );


  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  ErrVal writeQualityLevelInfosSEI(ExtBinDataAccessor* pcExtBinDataAccessor,
                                   UInt* uiaQualityLevel, 
                                   UInt *uiaDelta, 
                                   UInt uiNumLevels, 
                                   UInt uiLayer);
  //}}Quality level estimation and modified truncation- JVTO044 and m12007

  ErrVal writeNestingSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor );// SEI LSJ
  ErrVal writeViewScalInfoSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor ); // SEI LSJ
  ErrVal writeMultiviewSceneInfoSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor ); // SEI JVT-W060
  ErrVal writeMultiviewAcquisitionInfoSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor ); // SEI JVT-W060

  Bool getScalableSeiMessage ( Void );
	Void SetVeryFirstCall ( Void );
  // JVT-S080 LMI {
  ErrVal xWriteScalableSEILayersNotPresent( ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiInputLayers, UInt* m_layer_id);
  ErrVal xWriteScalableSEIDependencyChange( ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiNumLayers, UInt* uiLayerId, Bool* pbLayerDependencyInfoPresentFlag, 
												  UInt* uiNumDirectDependentLayers, UInt** puiDirectDependentLayerIdDeltaMinus1, UInt* puiLayerDependencyInfoSrcLayerIdDeltaMinus1);
  // JVT-S080 LMI }
protected:
  ErrVal xCreateEncoder();

protected:
  H264AVCEncoder*           m_pcH264AVCEncoder;
  FrameMng*                 m_pcFrameMng;

  SliceEncoder*             m_pcSliceEncoder;
  ControlMngH264AVCEncoder* m_pcControlMng;
  BitWriteBuffer*           m_pcBitWriteBuffer;
  BitCounter*               m_pcBitCounter;
  NalUnitEncoder*           m_pcNalUnitEncoder;

  UvlcWriter*               m_pcUvlcWriter;
  UvlcWriter*               m_pcUvlcTester;
  MbCoder*                  m_pcMbCoder;
  LoopFilter*               m_pcLoopFilter;
  MbEncoder*                m_pcMbEncoder;
  Transform*                m_pcTransform;
  IntraPredictionSearch*    m_pcIntraPrediction;
  YuvBufferCtrl*            m_apcYuvFullPelBufferCtrl [MAX_LAYERS];
  YuvBufferCtrl*            m_apcYuvHalfPelBufferCtrl [MAX_LAYERS];
  QuarterPelFilter*         m_pcQuarterPelFilter;
  CodingParameter*          m_pcCodingParameter;
  ParameterSetMng*          m_pcParameterSetMng;
  PocCalculator*            m_apcPocCalculator        [MAX_LAYERS];
  SampleWeighting*          m_pcSampleWeighting;
  CabacWriter*              m_pcCabacWriter;
  XDistortion*              m_pcXDistortion;
  MotionEstimation*         m_pcMotionEstimation;
  RateDistortion*           m_pcRateDistortion;
  History*                  m_pcHistory;
  ReconstructionBypass*     m_pcReconstructionBypass;
  PicEncoder*               m_pcPicEncoder;
  Bool                      m_bTraceEnable;
};

H264AVC_NAMESPACE_END


#endif // !defined(AFX_CREATERH264AVCENCODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_)
