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

#if !defined  _MCTF_H_
#define       _MCTF_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class PreProcessorParameter;

H264AVC_NAMESPACE_BEGIN

class SliceHeader;
class MbDataCtrl;
class MbEncoder;
class MotionEstimation;
class Frame;


class MCTF
{
protected:
  MCTF          ();
  virtual ~MCTF ();

public:
  static ErrVal create              ( MCTF*&                      rpcMCTF );
  ErrVal        destroy             ();
  ErrVal        init                ( PreProcessorParameter*      pcParameter,
                                      MbEncoder*                  pcMbEncoder,
                                      YuvBufferCtrl*              pcYuvFullPelBufferCtrl,
                                      YuvBufferCtrl*              pcYuvHalfPelBufferCtrl,
                                      QuarterPelFilter*           pcQuarterPelFilter,
                                      MotionEstimation*           pcMotionEstimation );
  ErrVal        uninit              ();
  ErrVal        process             ( PicBuffer*                  pcOrgPicBuffer,
                                      PicBuffer*                  pcRecPicBuffer,
                                      PicBufferList&              rcPicBufferOutputList,
                                      PicBufferList&              rcPicBufferUnusedList );

protected:
  //===== main functions =====
  ErrVal        xProcessGOP         ( PicBufferList&              rcPicBufferInputList,
                                      PicBufferList&              rcPicBufferOutputList,
                                      PicBufferList&              rcPicBufferUnusedList );

  //===== data management =====
  ErrVal  xCreateData               ( const SequenceParameterSet& rcSPS );
  ErrVal  xDeleteData               ();
  ErrVal  xInitGOP                  ( PicBufferList&              rcPicBufferInputList );
  ErrVal  xFinishGOP                ( PicBufferList&              rcPicBufferInputList,
                                      PicBufferList&              rcPicBufferOutputList,
                                      PicBufferList&              rcPicBufferUnusedList );
  ErrVal  xStoreReconstruction      ( PicBufferList&              rcPicBufferOutputList );

  //===== decomposition / composition =====
  ErrVal  xMotionEstimationStage    ( UInt                        uiBaseLevel );
  ErrVal  xDecompositionStage       ( UInt                        uiBaseLevel );
  ErrVal  xCompositionStage         ( UInt                        uiBaseLevel,
                                      PicBufferList&              rcPicBufferInputList );

  //===== control data initialization =====
  ErrVal  xSetScalingFactors        ( UInt                        uiBaseLevel );
  ErrVal  xGetListSizes             ( UInt                        uiTemporalLevel,
                                      UInt                        uiFrameIdInGOP,
                                      UInt                        auiPredListSize[2],
                                      UInt                        aauiUpdListSize[MAX_DSTAGES][2] );
  ErrVal  xInitSliceHeader          ( UInt                        uiTemporalLevel,
                                      UInt                        uiFrameIdInGOP );
  ErrVal  xClearBufferExtensions    ();
  ErrVal  xGetAndSetPredictionLists ( UInt                        uiBaseLevel,
                                      UInt                        uiFrame,
                                      Bool                        bHalfPel );
  ErrVal  xGetPredictionLists       ( RefFrameList&               rcRefList0,
                                      RefFrameList&               rcRefList1,
                                      UInt                        uiBaseLevel,
                                      UInt                        uiFrame,
                                      Bool                        bHalfPel );
  ErrVal  xGetUpdateLists           ( RefFrameList&               rcRefList0,
                                      RefFrameList&               rcRefList1,
                                      CtrlDataList&               rcCtrlList0,
                                      CtrlDataList&               rcCtrlList1,
                                      UInt                        uiBaseLevel,
                                      UInt                        uiFrame );
  ErrVal  xInitControlDataMotion    ( UInt                        uiBaseLevel,
                                      UInt                        uiFrame,
                                      Bool                        bMotionEstimation );

  //===== motion estimation / compensation =====
  ErrVal  xMotionCompensation       ( Frame*                      pcMCFrame,
                                      RefListStruct&              rcRefListStruct,
                                      MbDataCtrl*                 pcMbDataCtrl,
                                      SliceHeader&                rcSH );
  ErrVal  xMotionEstimation         ( const Frame*             pcOrigFrame,
                                      ControlData&                rcControlData );
  ErrVal  xUpdateCompensation       ( Frame*                   pcMCFrame,
                                      RefFrameList*               pcRefFrameList,
                                      CtrlDataList*               pcCtrlDataList,
                                      ListIdx                     eListUpd );

  //===== auxiliary functions =====
  ErrVal  xFillAndUpsampleFrame     ( Frame*                   rcFrame );
  ErrVal  xFillAndExtendFrame       ( Frame*                   rcFrame );
  ErrVal  xZeroIntraMacroblocks     ( Frame*                   pcFrame,
                                      ControlData&                pcCtrlData );

protected:
  SequenceParameterSet*         m_pcSPS;
  PictureParameterSet*          m_pcPPS;
  YuvBufferCtrl*                m_pcYuvFullPelBufferCtrl;
  YuvBufferCtrl*                m_pcYuvHalfPelBufferCtrl;
  MbEncoder*                    m_pcMbEncoder;
  QuarterPelFilter*             m_pcQuarterPelFilter;
  MotionEstimation*             m_pcMotionEstimation;

  Bool                          m_bFirstGOPCoded;                     // true if first GOP of a sequence has been coded
  UInt                          m_uiGOPSize;                          // current GOP size
  UInt                          m_uiDecompositionStages;              // number of decomposition stages
  UInt                          m_uiFrameWidthInMb;                   // frame width in macroblocks
  UInt                          m_uiFrameHeightInMb;                  // frame height in macroblocks
  UInt                          m_uiMbNumber;                         // number of macroblocks in a frame
  Double                        m_adBaseQpLambdaMotion[MAX_DSTAGES];  // base QP's for mode decision and motion estimation
  Frame*                        m_pcFrameTemp;                        // auxiliary frame memory
  Frame**                       m_papcFrame;                          // frame stores
  Frame**                       m_papcResidual;                       // frame stores for residual data
  ControlData*                  m_pacControlData;                     // control data arrays
  PicBufferList                 m_cOrgPicBufferList;
  PicBufferList                 m_cRecPicBufferList;
};

H264AVC_NAMESPACE_END

#endif // _MCTF_H_
