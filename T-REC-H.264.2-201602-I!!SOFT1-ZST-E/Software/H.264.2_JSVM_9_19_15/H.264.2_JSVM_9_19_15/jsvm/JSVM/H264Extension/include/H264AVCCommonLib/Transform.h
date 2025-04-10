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

#if !defined(AFX_INVERSETRANSFORM_H__B2D732EC_10EA_4C2F_9387_4456CFCA4439__INCLUDED_)
#define AFX_INVERSETRANSFORM_H__B2D732EC_10EA_4C2F_9387_4456CFCA4439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/Quantizer.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"

H264AVC_NAMESPACE_BEGIN

class H264AVCCOMMONLIB_API CoeffLevelPred
{
protected:
	CoeffLevelPred();
	virtual ~CoeffLevelPred();

public:

  ErrVal ScaleCoeffLevels       ( TCoeff* piCoeff,
                                  TCoeff* piRefCoeff,
                                  UInt uiQp,
                                  UInt uiRefQp,
                                  UInt uiNumCoeff );

};

class H264AVCCOMMONLIB_API Transform :
public Quantizer
, CoeffLevelPred
{
protected:
	Transform();
	virtual ~Transform();

public:
  static ErrVal create( Transform*& rpcTransform );
  ErrVal destroy();
  Void setClipMode( Bool bEnableClip ) { m_bClip = bEnableClip; }

  Bool getClipMode()                   { return m_bClip; }

  // JVT-V035 functions for SVC to AVC rewrite
  ErrVal        predict4x4Blk             ( TCoeff* piCoeff, TCoeff* piRef, UInt uiRefQp, UInt& ruiAbsSum );
  ErrVal        predict8x8Blk             ( TCoeff* piCoeff, TCoeff* piRef, UInt uiRefQp, UInt& ruiAbsSum );
  ErrVal        predictChromaBlocks       ( UInt uiComp, TCoeff* piCoeff, TCoeff* piRef, UInt uiRefQp, UInt& ruiDcAbs, UInt& ruiAcAbs );
  ErrVal        predictScaledACCoeffs     ( UInt uiComp, TCoeff* piCoeff, TCoeff* piRef, UInt uiRefQp, const UChar* pucScale );
  ErrVal        predictScaledChromaCoeffs ( TCoeff* piCoeff, TCoeff* piRef, UInt uiRefCbQp, UInt uiRefCrQp, const UChar* pucScaleCb, const UChar* pucScaleCr );
  ErrVal        predictMb16x16            ( TCoeff* piCoeff, TCoeff* piRef, UInt uiRefQp, UInt& ruiDcAbs, UInt& ruiAcAbs );
  ErrVal        addPrediction4x4Blk       ( TCoeff* piCoeff, TCoeff* piRefCoeff, UInt uiDstQp, UInt uiRefQp, UInt &uiCoded  );
  ErrVal        addPrediction8x8Blk       ( TCoeff* piCoeff, TCoeff* piRefCoeff, UInt uiQp, UInt uiRefQp, Bool& bCoded  );
  ErrVal        addPredictionChromaBlocks ( TCoeff* piCoeff, TCoeff* piRef, UInt uiQp, UInt uiRefQp, Bool& bDCflag, Bool& bACflag );

  ErrVal        transform8x8Blk           ( YuvMbBuffer*       pcOrgData,
                                            YuvMbBuffer*       pcPelData,
                                            TCoeff*               piCoeff,
                                            const UChar*          pucScale,
                                            UInt&                 ruiAbsSum );
  ErrVal        transform4x4Blk           ( YuvMbBuffer*       pcOrgData,
                                            YuvMbBuffer*       pcPelData,
                                            TCoeff*               piCoeff,
                                            const UChar*          pucScale,
                                            UInt&                 ruiAbsSum );
  ErrVal        transformMb16x16          ( YuvMbBuffer* pcOrgData,
                                            YuvMbBuffer* pcPelData,
                                            TCoeff* piCoeff,
                                            const UChar* pucScale,
                                            UInt& ruiDcAbs,
                                            UInt& ruiAcAbs );
  ErrVal        transformChromaBlocks     ( XPel* pucOrg,
                                            XPel* pucRec,
                                            const CIdx            cCIdx,
                                            Int iStride,
                                            TCoeff* piCoeff,
                                            TCoeff* piQuantCoeff,
                                            const UChar* pucScale,
                                            UInt& ruiDcAbs,
                                            UInt& ruiAcAbs );

  ErrVal        invTransform8x8Blk        ( XPel* puc, Int iStride, TCoeff* piCoeff );
  ErrVal        invTransform4x4Blk        ( XPel* puc, Int iStride, TCoeff* piCoeff );
  ErrVal        invTransformChromaBlocks  ( XPel* puc, Int iStride, TCoeff* piCoeff );

  ErrVal        invTransform4x4Blk        ( Pel*  puc, Int iStride, TCoeff* piCoeff );
  ErrVal        invTransformChromaBlocks  ( Pel*  puc, Int iStride, TCoeff* piCoeff );

  ErrVal        invTransformDcCoeff       ( TCoeff* piCoeff, const Int iQpScale, const Int iQpPer );
  ErrVal        invTransformDcCoeff       ( TCoeff* piCoeff, Int iQpScale );
  Void          invTransformChromaDc      ( TCoeff* piCoeff );

  void         setStoreCoeffFlag(Bool flag)   { m_storeCoeffFlag = flag;}
  Bool         getStoreCoeffFlag()        {return  m_storeCoeffFlag;}
  ErrVal       transform4x4BlkCGS         ( YuvMbBuffer*         pcOrgData,
                                            YuvMbBuffer*         pcPelData,
                                            TCoeff*                 piCoeff,
                                            TCoeff*                 piCoeffBase,
                                            const UChar*            pucScale,
                                            UInt&                   ruiAbsSum );
  ErrVal       transform8x8BlkCGS           ( YuvMbBuffer*       pcOrgData,
                                            YuvMbBuffer*         pcPelData,
                                            TCoeff*                 piCoeff,
                                            TCoeff*                 piCoeffBase,
                                            const UChar*            pucScale,
                                            UInt&                   ruiAbsSum );

  ErrVal        transformChromaBlocksCGS  ( XPel* pucOrg,
                                            XPel* pucRec,
                                            const CIdx            cCIdx,
                                            Int iStride,
                                            TCoeff* piCoeff,
                                            TCoeff* piQuantCoeff,
                                            TCoeff* piCoeffBase,
                                            const UChar* pucScale,
                                            UInt& ruiDcAbs,
                                            UInt& ruiAcAbs );

private:
  Void xForTransform8x8Blk      ( XPel* pucOrg, XPel* pucRec, Int iStride, TCoeff* piPredCoeff );
  Void xForTransform4x4Blk      ( XPel* pucOrg, XPel* pucRec, Int iStride, TCoeff* piPredCoeff );

  Void xInvTransform4x4Blk      ( XPel* puc, Int iStride, TCoeff* piCoeff );
  Void xInvTransform4x4Blk      ( Pel*  puc, Int iStride, TCoeff* piCoeff );

  Void xInvTransform4x4BlkNoAc  ( XPel* puc, Int iStride, TCoeff* piCoeff );
  Void xInvTransform4x4BlkNoAc  ( Pel*  puc, Int iStride, TCoeff* piCoeff );

  Void xForTransformChromaDc    ( TCoeff* piCoeff );
  Void xForTransformLumaDc      ( TCoeff* piCoeff );

  Int  xRound                   ( Int i     )             { return ((i)+(1<<5))>>6; }
  Int  xClip                    ( Int iPel  )             { return ( m_bClip ? gClip( iPel ) : iPel); }

  Void xQuantDequantUniform8x8      ( TCoeff* piQCoeff,
                                      TCoeff* piCoeff,
                                      const QpParameter& rcQp,
                                      const UChar* pucScale,
                                      UInt& ruiAbsSum );
  Void xQuantDequantUniform4x4      ( TCoeff* piQCoeff,
                                      TCoeff* piCoeff,
                                      const QpParameter& rcQp,
                                      const UChar* pucScale,
                                      UInt& ruiAbsSum );
  Void xQuantDequantNonUniformLuma  ( TCoeff* piQCoeff,
                                      TCoeff* piCoeff,
                                      const QpParameter& rcQp,
                                      const UChar* pucScale,
                                      UInt& ruiDcAbs,
                                      UInt& ruiAcAbs );
  Void xQuantDequantNonUniformChroma( TCoeff* piQCoeff,
                                      TCoeff* piCoeff,
                                      const QpParameter& rcQp,
                                      const UChar* pucScale,
                                      UInt& ruiDcAbs,
                                      UInt& ruiAcAbs );

  Void x4x4DequantChroma  ( TCoeff*             piQCoeff,
                            TCoeff*             piCoeff,
                            const QpParameter&  rcQp,
                            const UChar*        pucScale );

protected:
  const SliceHeader*  m_pcSliceHeader;
  Bool                m_bClip;
  Bool                m_storeCoeffFlag;
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_INVERSETRANSFORM_H__B2D732EC_10EA_4C2F_9387_4456CFCA4439__INCLUDED_)
