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


#if !defined(AFX_MBDECODER_H__F725C8AD_2589_44AD_B904_62FE2A7F7D8D__INCLUDED_)
#define AFX_MBDECODER_H__F725C8AD_2589_44AD_B904_62FE2A7F7D8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/YuvPicBuffer.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/Transform.h"

H264AVC_NAMESPACE_BEGIN

class Transform;
class IntraPrediction;
class MotionCompensation;
class FrameMng;

class IntFrame;
class IntFrameBase;
class IntYuvPicBuffer;
class IntYuvPicBufferBase;
class IntYuvMbBuffer;
class IntYuvMbBufferBase;


class MbDecoder
{
protected:
	MbDecoder();
	virtual ~MbDecoder();

public:
  static ErrVal create          ( MbDecoder*&         rpcMbDecoder );
  ErrVal destroy                ();

  ErrVal init                   ( Transform*          pcTransform,
                                  IntraPrediction*    pcIntraPrediction,
                                  MotionCompensation* pcMotionCompensation,
                                  FrameMng*           pcFrameMng  );
  ErrVal uninit                 ();

  ErrVal process                ( MbDataAccess& rcMbDataAccess,
                                  Bool          bReconstructAll  );
  ErrVal decode                 ( MbDataAccess& rcMbDataAccess,
                                  MbDataAccess* pcMbDataAccessBase,
                                  IntFrame*     pcFrame,
                                  IntFrame*     pcResidual,
                                  IntFrame*     pcPredSignal,
                                  IntFrame*     pcBaseLayer,
                                  IntFrame*     pcBaseLayerResidual,
                                  RefFrameList* pcRefFrameList0,
                                  RefFrameList* pcRefFrameList1,
                                  Bool          bReconstructAll );
  ErrVal calcMv                 ( MbDataAccess& rcMbDataAccess,
                                  MbDataAccess* pcMbDataAccessBaseMotion );
  ErrVal compensatePrediction   ( MbDataAccess& rcMbDataAccess );

protected:
	ErrVal xDecodeMbPCM           ( MbDataAccess&     rcMbDataAccess,
                                  YuvMbBuffer&      rcRecYuvBuffer );
  ErrVal xDecodeMbInter         ( MbDataAccess&     rcMbDataAccess,
                                  YuvMbBuffer&      rcRecYuvBuffer, 
                                  IntYuvMbBuffer&   rcPredIntYuvMbBuffer,
                                  IntYuvMbBuffer&   rcResIntYuvMbBuffer,
                                  Bool              bReconstruct );

  ErrVal xDecodeChroma          ( MbDataAccess&     rcMbDataAccess,
                                  YuvMbBuffer&      rcRecYuvBuffer,
                                  UInt              uiChromaCbp,
                                  Bool              bPredChroma );
  
  
  ErrVal xDecodeMbPCM           ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvPicBuffer*  pcRecYuvBuffer );
  ErrVal xDecodeMbIntra4x4      ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvMbBuffer&   cYuvMbBuffer
                                  ,IntYuvMbBuffer&  rcPredBuffer );
  ErrVal xDecodeMbIntra8x8      ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvMbBuffer&   cYuvMbBuffer
                                  ,IntYuvMbBuffer&  rcPredBuffer );
  ErrVal xDecodeMbIntra16x16    ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvMbBuffer&   cYuvMbBuffer
                                  ,IntYuvMbBuffer&  rcPredBuffer );
  ErrVal xDecodeMbIntraBL       ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvPicBuffer*  pcRecYuvBuffer,
                                  IntYuvMbBuffer&   rcPredBuffer,
                                  IntYuvPicBuffer*  pcBaseYuvBuffer );
  ErrVal xDecodeMbInter         ( MbDataAccess&     rcMbDataAccess,
                                  MbDataAccess*     pcMbDataAccessBase,
                                  IntYuvMbBuffer&   rcPredBuffer,
                                  IntYuvPicBuffer*  pcRecYuvBuffer,
                                  IntFrame*         pcResidual,
                                  IntFrame*         pcBaseResidual,
                                  RefFrameList&     rcRefFrameList0, 
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bReconstruct );
  ErrVal xDecodeChroma          ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvMbBuffer&   rcRecYuvBuffer,
                                  IntYuvMbBuffer&   rcPredBuffer,
                                  UInt              uiChromaCbp,
                                  Bool              bPredChroma );
  
  
  ErrVal xScaleTCoeffs          ( MbDataAccess&      rcMbDataAccess );
  ErrVal xScale4x4Block         ( TCoeff*            piCoeff,
                                  const UChar*       pucScale,
                                  UInt               uiStart,
                                  const QpParameter& rcQP );
  ErrVal xScale8x8Block         ( TCoeff*            piCoeff,
                                  const UChar*       pucScale,
                                  const QpParameter& rcQP );

  ErrVal xPredictionFromBaseLayer( MbDataAccess&  rcMbDataAccess,
                                   MbDataAccess*  pcMbDataAccessBase );
protected:
  MbTransformCoeffs   m_cTCoeffs;
  
  Transform*          m_pcTransform;
  IntraPrediction*    m_pcIntraPrediction;
  MotionCompensation* m_pcMotionCompensation;
  FrameMng*           m_pcFrameMng;
  Bool                m_bInitDone;
};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_MBDECODER_H__F725C8AD_2589_44AD_B904_62FE2A7F7D8D__INCLUDED_)
