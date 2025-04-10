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


#if !defined(AFX_MBCODER_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_)
#define AFX_MBCODER_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MbSymbolWriteIf.h"
#include "RateDistortionIf.h"


H264AVC_NAMESPACE_BEGIN

class MbCoder
{
protected:
	MbCoder();
	virtual ~MbCoder();

public:
  static ErrVal create( MbCoder*& rpcMbCoder );
  ErrVal destroy();

  ErrVal initSlice( const SliceHeader& rcSH,
                    MbSymbolWriteIf* pcMbSymbolWriteIf,
                    RateDistortionIf* pcRateDistortionIf );


  ErrVal uninit();

  ErrVal  encode            ( MbDataAccess& rcMbDataAccess,
                              MbDataAccess* pcMbDataAccessBase,
                              Int									iSpatialScalabilityType,
                              Bool          bTerminateSlice, 
							  Bool          bSendTerminateSlice);

  ErrVal  encodeMotion      ( MbDataAccess& rcMbDataAccess,
                              MbDataAccess* pcMbDataAccessBase );
  UInt    getBitCount       ()  { return m_pcMbSymbolWriteIf->getNumberOfWrittenBits(); }

protected:
  ErrVal xWriteIntraPredModes ( MbDataAccess& rcMbDataAccess );
  
  
  
  ErrVal xWriteMotionPredFlags_FGS( MbDataAccess& rcMbDataAccess,
                                    MbDataAccess* pcMbDataAccessBase,
                                    MbMode        eMbMode,
                                    ListIdx       eLstIdx );
  ErrVal xWriteMotionPredFlags    ( MbDataAccess& rcMbDataAccess,
                                    MbMode        eMbMode,
                                    ListIdx       eLstIdx );
  ErrVal xWriteReferenceFrames    ( MbDataAccess& rcMbDataAccess,
                                    MbMode        eMbMode,
                                    ListIdx       eLstIdx );
  ErrVal xWriteMotionVectors      ( MbDataAccess& rcMbDataAccess,
                                    MbMode        eMbMode,
                                    ListIdx       eLstIdx );
  
  //-- JVT-R091
	ErrVal xWriteTextureInfo    ( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase, const MbTransformCoeffs& rcMbTCoeff, Bool bTrafo8x8Flag );
	//--
  ErrVal xWriteBlockMv        ( MbDataAccess& rcMbDataAccess, B8x8Idx c8x8Idx, ListIdx eLstIdx );


  ErrVal xScanLumaIntra16x16  ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, Bool bAC );
  ErrVal xScanLumaBlock       ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, LumaIdx cIdx );
  ErrVal xScanChromaDc        ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff );
  ErrVal xScanChromaAcU       ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff );
  ErrVal xScanChromaAcV       ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff );
  ErrVal xScanChromaBlocks    ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, UInt uiChromCbp );

protected:
  MbSymbolWriteIf* m_pcMbSymbolWriteIf;
  RateDistortionIf* m_pcRateDistortionIf;

  Bool m_bInitDone;
  Bool  m_bCabac;
  Bool  m_bPrevIsSkipped;
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_MBCODER_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_)
