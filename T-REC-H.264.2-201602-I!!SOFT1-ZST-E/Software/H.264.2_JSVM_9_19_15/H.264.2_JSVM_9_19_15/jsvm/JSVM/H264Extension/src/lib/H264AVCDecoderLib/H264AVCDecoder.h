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

#if !defined(AFX_H264AVCDECODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_)
#define AFX_H264AVCDECODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "GOPDecoder.h"
#include "H264AVCCommonLib/Sei.h"
#include "H264AVCCommonLib/MotionCompensation.h"
#include "H264AVCCommonLib/LoopFilter.h"

H264AVC_NAMESPACE_BEGIN

class SliceReader;
class SliceDecoder;
class PocCalculator;
class LoopFilter;
class HeaderSymbolReadIf;
class ParameterSetMng;
class NalUnitParser;
class ControlMngIf;
class AccessUnit;
class NALUnit;
class NonVCLNALUnit;


class H264AVCDECODERLIB_API H264AVCDecoder
{
protected:
	H264AVCDecoder         ();
  virtual ~H264AVCDecoder();

public:
  //===== creation and initialization =====
  static  ErrVal  create  ( H264AVCDecoder*&    rpcH264AVCDecoder );
  ErrVal  destroy         ();
  ErrVal  init            ( NalUnitParser*      pcNalUnitParser,
                            HeaderSymbolReadIf* pcHeaderSymbolReadIf,
                            ParameterSetMng*    pcParameterSetMngAUInit,
                            ParameterSetMng*    pcParameterSetMngDecode,
                            LayerDecoder*       apcLayerDecoder[MAX_LAYERS] );
  ErrVal  uninit          ();

  //===== main processing functions =====
  ErrVal  initNALUnit     ( BinData*&         rpcBinData,
                            AccessUnit&       rcAccessUnit );
  ErrVal  processNALUnit  ( PicBuffer*        pcPicBuffer,
                            PicBufferList&    rcPicBufferOutputList,
                            PicBufferList&    rcPicBufferUnusedList,
                            BinDataList&      rcBinDataList,
                            NALUnit&          rcNALUnit );

  //===== update decoded picture buffer of all layers =====
  ErrVal  updateDPB       ( UInt              uiTargetDependencyId,
                            PicBufferList&    rcPicBufferOutputList,
                            PicBufferList&    rcPicBufferUnusedList );

  //===== get inter-layer prediction data =====
  ErrVal  getBaseLayerData              ( SliceHeader&      rcELSH,
                                          Frame*&           pcFrame,
                                          Frame*&           pcResidual,
                                          MbDataCtrl*&      pcMbDataCtrl,
                                          ResizeParameters& rcResizeParameters,
                                          UInt              uiBaseLayerId );
  ErrVal  getBaseSliceHeader            ( SliceHeader*&     rpcSliceHeader,
                                          UInt              uiRefLayerDependencyId );

protected:
  ErrVal  xProcessNonVCLNALUnit         ( NonVCLNALUnit&    rcNonVCLNALUnit );


protected:
  Bool                m_bInitDone;
  NalUnitParser*      m_pcNalUnitParser;
  HeaderSymbolReadIf* m_pcHeaderSymbolReadIf;
  ParameterSetMng*    m_pcParameterSetMngAUInit;
  ParameterSetMng*    m_pcParameterSetMngDecode;
  LayerDecoder*       m_apcLayerDecoder[MAX_LAYERS];
  UInt                m_auiLastDQTPId[4]; // only for setting correct parameters in filler data instances
};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_H264AVCDECODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_)
