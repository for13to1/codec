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

#if !defined(AFX_SLICEDECODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_)
#define AFX_SLICEDECODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/ControlMngIf.h"

H264AVC_NAMESPACE_BEGIN

class MbDecoder;
class Frame;

class SliceDecoder
{
protected:
	SliceDecoder();
	virtual ~SliceDecoder();

public:
  static ErrVal create  ( SliceDecoder*&      rpcSliceDecoder );
  ErrVal        destroy ();

  ErrVal        init    ( MbDecoder*          pcMbDecoder,
                          ControlMngIf*       pcControlMng );
  ErrVal        uninit  ();

  ErrVal  decode        ( SliceHeader&        rcSH,
                          MbDataCtrl*         pcMbDataCtrl,
                          MbDataCtrl*         pcMbDataCtrlBase,
                          Frame*              pcFrame,
                          Frame*              pcResidualLF,
                          Frame*              pcResidualILPred,
                          Frame*              pcBaseLayer,
                          Frame*              pcBaseLayerResidual,
                          RefFrameList*       pcRefFrameList0,
                          RefFrameList*       pcRefFrameList1,
                          MbDataCtrl*         pcMbDataCtrl0L1,
                          Bool                bReconstructAll );
  ErrVal  decodeMbAff   ( SliceHeader&        rcSH,
                          MbDataCtrl*         pcMbDataCtrl,
                          MbDataCtrl*         pcMbDataCtrlBase,
                          MbDataCtrl*         pcMbDataCtrlBaseField,
                          Frame*              pcFrame,
                          Frame*              pcResidualLF,
                          Frame*              pcResidualILPred,
                          Frame*              pcBaseLayer,
                          Frame*              pcBaseLayerResidual,
                          RefFrameList*       pcRefFrameList0,
                          RefFrameList*       pcRefFrameList1,
                          MbDataCtrl*         pcMbDataCtrl0L1,
                          Bool                bReconstructAll );

  Void setIntraBLFlagArrays( Bool* apabBaseModeFlagAllowedArrays[2] )
  {
    m_apabBaseModeFlagAllowedArrays[0] = apabBaseModeFlagAllowedArrays[0];
    m_apabBaseModeFlagAllowedArrays[1] = apabBaseModeFlagAllowedArrays[1];
  }

protected:
  Bool            m_bInitDone;
  ControlMngIf*   m_pcControlMng;
  MbDecoder*      m_pcMbDecoder;
  Bool*           m_apabBaseModeFlagAllowedArrays[2];
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_SLICEDECODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_)
