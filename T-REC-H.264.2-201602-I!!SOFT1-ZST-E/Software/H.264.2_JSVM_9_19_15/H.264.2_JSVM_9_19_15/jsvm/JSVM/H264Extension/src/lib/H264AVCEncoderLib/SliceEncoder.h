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

#if !defined(AFX_SLICEENCODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_)
#define AFX_SLICEENCODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/ControlMngIf.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "MbEncoder.h"


H264AVC_NAMESPACE_BEGIN


class CodingParameter;
class MbCoder;
class PocCalculator;


class SliceEncoder
{
protected:
	SliceEncoder();
	virtual ~SliceEncoder();

public:
  static ErrVal create        ( SliceEncoder*&    rpcSliceEncoder );
  ErrVal destroy              ();
  ErrVal init                 ( MbEncoder*        pcMbEncoder,
                                MbCoder*          pcMbCoder,
                                ControlMngIf*     pcControlMng,
                                CodingParameter*  pcCodingParameter,
                                PocCalculator*    pcPocCalculator,
                                Transform*        pcTransform);
  ErrVal uninit               ();

  ErrVal  encodeSliceSVC      ( ControlData&      rcControlData,
                                Frame&            rcOrgFrame,
                                Frame&            rcFrame,
                                Frame*            pcResidualFrameLF,
                                Frame*            pcResidualFrameILPred,
                                Frame*            pcPredFrame,
                                PicType           ePicType,
                                UInt              uiNumMaxIter,
                                UInt              uiIterSearchRange,
                                Bool              bBiPred8x8Disable,
                                Bool              bMCBlks8x8Disable,
                                UInt              uiMaxDeltaQp,
                                UInt&             ruiBits );
  ErrVal  encodeMbAffSliceSVC ( ControlData&      rcControlData,
                                Frame&            rcOrgFrame,
                                Frame&            rcFrame,
                                Frame*            pcResidualFrameLF,
                                Frame*            pcResidualFrameILPred,
                                Frame*            pcPredFrame,
                                UInt              uiNumMaxIter,
                                UInt              uiIterSearchRange,
                                Bool              bBiPred8x8Disable,
                                Bool              bMCBlks8x8Disable,
                                UInt              uiMaxDeltaQp,
                                UInt&             ruiBits );
  ErrVal  encodeSlice         ( SliceHeader&      rcSliceHeader,
                                Frame*            pcFrame,
                                MbDataCtrl*       pcMbDataCtrl,
                                RefListStruct&    rcRefListStruct,
                                Bool              bMCBlks8x8Disable,
                                UInt              uiMbInRow,
                                Double            dlambda );
  MbEncoder*  getMbEncoder    ()                  { return m_pcMbEncoder; }

//TMM_WP
  ErrVal  xSetPredWeights     ( SliceHeader&      rcSliceHeader,
                                Frame*            pOrgFrame,
                                RefListStruct&    rcRefListStruct );
  ErrVal  xInitDefaultWeights ( Double*           pdWeights,
                                UInt              uiLumaWeightDenom,
                                UInt              uiChromaWeightDenom );
//TMM_WP
  //S051{
  Void		setUseBDir			    ( Bool b )          { m_pcMbEncoder->setUseBDir( b ); }
  //S051}

  ErrVal updatePictureResTransform  ( ControlData&    rcControlData,
                                      UInt            uiMbInRow );
  ErrVal updateBaseLayerResidual    ( ControlData&    rcControlData,
                                      UInt            uiMbInRow );
  // JVT-V035
  ErrVal updatePictureAVCRewrite    ( ControlData&    rcControlData,
                                      UInt            uiMbInRow );

  ErrVal xAddTCoeffs2               ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase );

  Void   setIntraBLFlagArrays       ( Bool*           apabBaseModeFlagAllowedArrays[2] )
  {
    m_apabBaseModeFlagAllowedArrays[0] = apabBaseModeFlagAllowedArrays[0];
    m_apabBaseModeFlagAllowedArrays[1] = apabBaseModeFlagAllowedArrays[1];
  }

public://protected://JVT-X046
  MbEncoder*        m_pcMbEncoder;
  MbCoder*          m_pcMbCoder;
  ControlMngIf*     m_pcControlMng;
  CodingParameter*  m_pcCodingParameter;
  PocCalculator*    m_pcPocCalculator;
  Transform*        m_pcTransform;
  Bool              m_bInitDone;
  UInt              m_uiFrameCount;
  SliceType         m_eSliceType;
  Bool              m_bTraceEnable;
  Bool*             m_apabBaseModeFlagAllowedArrays[2];
	//JVT-X046 {
  UInt              m_uiSliceMode;
  UInt              m_uiSliceArgument;
  //JVT-X046 }
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_SLICEENCODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_)
