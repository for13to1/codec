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


#if !defined(AFX_UVLCREADER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_)
#define AFX_UVLCREADER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MbSymbolReadIf.h"
#include "H264AVCCommonLib/Quantizer.h"

H264AVC_NAMESPACE_BEGIN

class BitReadBuffer;

#define CAVLC_SYMGRP_SIZE   3 

class UcSymGrpReader; 

class UvlcReader
: public HeaderSymbolReadIf
, public MbSymbolReadIf
, public Quantizer

{
public:
  typedef struct
  {
    UChar nVal;
    UChar nSize;
  }Vlc;

protected:
	UvlcReader();
	virtual ~UvlcReader();
  ErrVal  xRQdecodeNewTCoeffs ( TCoeff*       piCoeff,
                                TCoeff*       piCoeffBase,
                                UInt          uiStart,
                                UInt          uiStop,
                                UInt          uiStride,
                                const UChar*  pucScan,
                                UInt          uiScanIndex,
                                UInt*         pauiEobShift,
                                Bool&         rbLast,
                                UInt&         ruiNumCoefRead );
  ErrVal  xRQdecodeTCoeffsRef ( TCoeff*       piCoeff,
                                TCoeff*       piCoeffBase,
                                const UChar*  pucScan,
                                UInt          uiScanIndex );

public:
  static ErrVal create( UvlcReader*& rpcUvlcReader );
  ErrVal destroy();

  ErrVal init   ( BitReadBuffer* pcBitReadBuffer );
  ErrVal uninit ();

  Bool    moreRBSPData();
  ErrVal  getUvlc     ( UInt& ruiCode,                const Char* pcTraceString );
  ErrVal  getCode     ( UInt& ruiCode, UInt uiLength, const Char* pcTraceString );
  ErrVal  getSvlc     ( Int&  riCode,                 const Char* pcTraceString );
  ErrVal  getFlag     ( Bool& rbFlag,                 const Char* pcTraceString );
  ErrVal  readByteAlign();
  ErrVal  readZeroByteAlign();//SEI LSJ

  ErrVal  codeFromBitstream2Di( const UInt* auiCode, const UInt* auiLen, UInt uiWidth, UInt uiHeight, UInt& uiVal1, UInt& uiVal2 )
    {return xCodeFromBitstream2Di(auiCode, auiLen, uiWidth, uiHeight, uiVal1, uiVal2);};
  Bool    isMbSkipped ( MbDataAccess& rcMbDataAccess );
  Bool    isBLSkipped ( MbDataAccess& rcMbDataAccess );

  Bool    isEndOfSlice();
  ErrVal  blockModes  ( MbDataAccess& rcMbDataAccess );
  ErrVal  mbMode      ( MbDataAccess& rcMbDataAccess );
  ErrVal  resPredFlag ( MbDataAccess& rcMbDataAccess );
  ErrVal  resPredFlag_FGS ( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff );
	ErrVal  smoothedRefFlag( MbDataAccess& rcMbDataAccess );	// JVT-R091

  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );

  ErrVal  cbp( MbDataAccess& rcMbDataAccess );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, LumaIdx   cIdx, ResidualMode eResidualMode, UInt& ruiMbExtCbp);
  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode );

  ErrVal  deltaQp             ( MbDataAccess& rcMbDataAccess );
  ErrVal  intraPredModeLuma   ( MbDataAccess& rcMbDataAccess, LumaIdx cIdx );
  ErrVal  intraPredModeChroma ( MbDataAccess& rcMbDataAccess );
  ErrVal  fieldFlag           ( MbDataAccess& rcMbDataAccess );
  ErrVal  samplesPCM          ( MbDataAccess& rcMbDataAccess );

  ErrVal  startSlice          ( const SliceHeader& rcSliceHeader );
  ErrVal  finishSlice         ( );
  
  ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess);
  ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx );
  ErrVal  intraPredModeLuma8x8( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx ); // HS: bug fix by Nokia
  ErrVal  RQdecodeCycleSymbol ( UInt& uiCycle );
  ErrVal  RQdecodeDeltaQp     ( MbDataAccess&   rcMbDataAccess );
  ErrVal  RQdecode8x8Flag     ( MbDataAccess&   rcMbDataAccess,
                                MbDataAccess&   rcMbDataAccessBase );
  Bool    RQdecodeBCBP_4x4     ( MbDataAccess&   rcMbDataAccessBase,
                                 LumaIdx         cIdx );
  Bool    RQdecodeBCBP_ChromaDC( MbDataAccess&   rcMbDataAccessBase,
                                 ChromaIdx       cIdx );
  Bool    RQdecodeBCBP_ChromaAC( MbDataAccess&   rcMbDataAccessBase,
                                 ChromaIdx       cIdx );
  Bool    RQdecodeCBP_Chroma   ( MbDataAccess&   rcMbDataAccess,
                                 MbDataAccess&   rcMbDataAccessBase );
  Bool    RQdecodeCBP_ChromaAC ( MbDataAccess&   rcMbDataAccess,
                                 MbDataAccess&   rcMbDataAccessBase );
  Bool    RQdecodeCBP_8x8      ( MbDataAccess&   rcMbDataAccess,
                                 MbDataAccess&   rcMbDataAccessBase,
                                 B8x8Idx         c8x8Idx );
  ErrVal  RQdecodeTermBit     ( UInt&           ruiBit ) { ruiBit = 1; return Err::m_nOK; };
  ErrVal  RQdecodeNewTCoeff_8x8    ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     B8x8Idx         c8x8Idx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefRead );
  ErrVal  RQeo8b                   ( Bool&           bEob );
  ErrVal  RQdecodeTCoeffRef_8x8    ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     B8x8Idx         c8x8Idx,
                                     UInt            uiScanIndex );
  ErrVal  RQdecodeNewTCoeff_Luma   ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     LumaIdx         cIdx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefRead );
  ErrVal  RQdecodeTCoeffRef_Luma   ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     LumaIdx         cIdx,
                                     UInt            uiScanIndex );
  ErrVal  RQdecodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     ChromaIdx       cIdx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefRead );
  ErrVal  RQdecodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     ChromaIdx       cIdx,
                                     UInt            uiScanIndex );
  ErrVal  RQdecodeEobOffsets_Luma  ();
  ErrVal  RQdecodeEobOffsets_Chroma();
  ErrVal  RQdecodeBestCodeTableMap ( UInt            uiMaxH );
  ErrVal  RQupdateVlcTable         ();
  ErrVal  RQvlcFlush               ();
  Bool    RQpeekCbp4x4( MbDataAccess&  rcMbDataAccessBase, LumaIdx cIdx);
  Bool m_bTruncated;

private:
  ErrVal xGetFlag     ( UInt& ruiCode );
  ErrVal xGetCode     ( UInt& ruiCode, UInt uiLength );
  ErrVal xGetUvlcCode ( UInt& ruiVal  );
  ErrVal xGetSvlcCode ( Int&  riVal   );
  ErrVal xGetRefFrame ( Bool bWriteBit, UInt& uiRefFrame, ListIdx eLstIdx );
  ErrVal xGetMotionPredFlag( Bool& rbFlag );
  ErrVal xGetMvd      ( Mv& cMv );

  ErrVal xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, UInt& uiCoeffCount, UInt& uiTrailingOnes );
  ErrVal xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, UInt& uiCoeffCount, UInt& uiTrailingOnes );
  ErrVal xGetTrailingOnes16( UInt uiLastCoeffCount, UInt& uiCoeffCount, UInt& uiTrailingOnes );
  ErrVal xCodeFromBitstream2D( const UChar* aucCode, const UChar* aucLen, UInt uiWidth, UInt uiHeight, UInt& uiVal1, UInt& uiVal2 );
  ErrVal xCodeFromBitstream2Di( const UInt* auiCode, const UInt* auiLen, UInt uiWidth, UInt uiHeight, UInt& uiVal1, UInt& uiVal2 );
  ErrVal xGetRunLevel( Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt& uiTotalRun );
  ErrVal xGetLevelVLC0( Int& iLevel );
  ErrVal xGetLevelVLCN( Int& iLevel, UInt uiVlcLength );
  ErrVal xGetRun( UInt uiVlcPos, UInt& uiRun  );
  ErrVal xGetTotalRun16( UInt uiVlcPos, UInt& uiTotalRun );
  ErrVal xGetTotalRun4( UInt& uiVlcPos, UInt& uiTotalRun );
  ErrVal xGetTrailingOnes4( UInt& uiCoeffCount, UInt& uiTrailingOnes );
  ErrVal xRQdecodeEobOffsets       ( UInt* pauiShift, UInt            uiLen );
  ErrVal xGetGolomb(UInt& uiSymbol, UInt uiK);
  ErrVal xGetSigRunCode( UInt& uiSymbol, UInt uiTableIdx );
  ErrVal xGetUnaryCode( UInt& uiSymbol );
  ErrVal xGetCodeCB1( UInt& uiSymbol );
  ErrVal xGetCodeCB2( UInt& uiSymbol );
  ErrVal xGetSigRunTabCode(UInt& uiTab);
  ErrVal  xDecodeMonSeq           ( UInt*           auiSeq,
                                    UInt uiStart,
                                     UInt            uiLen );
  ErrVal xRQdecodeSigMagGreater1( TCoeff* piCoeff,
                                     TCoeff* piCoeffBase,
                                     const UChar* pucScan,
                                     UInt    uiTermSym,
                                     UInt    uiStart,
                                     UInt    uiStop,
                                     UInt    uiStride = 1 );

protected:
  BitReadBuffer*  m_pcBitReadBuffer;
  UInt            m_uiBitCounter;
  UInt            m_uiPosCounter;
  Bool            m_bRunLengthCoding;
  UInt            m_uiRun;
  UInt m_auiShiftLuma[16];
  UInt m_auiShiftChroma[16];
  UInt m_auiBestCodeTab[16];
  UInt m_uiCbpStats[3][2];
  UInt m_uiCbp8x8;
  UInt m_uiCbpStat4x4[2];
  UInt m_uiCurrCbp4x4;
  UcSymGrpReader* m_pSymGrp; 
  UInt m_auiSymbolBuf[CAVLC_SYMGRP_SIZE];
  UInt m_uiRefSymCounter; 
};


class UcSymGrpReader
{
public:
  UcSymGrpReader( UvlcReader* pParent );
  ErrVal Init();
  ErrVal Flush();
  ErrVal xFetchSymbol( UInt uiMaxSym );
  Bool   UpdateVlc();
  UInt   GetCode()    { return m_uiCode;   }
  Void   setCodedFlag(UInt uiFlag)    { m_uiCodedFlag = uiFlag; }

protected:
  UvlcReader* m_pParent;
  UInt m_auiSymCount[3];
  UInt m_uiTable;
  UInt m_uiCodedFlag;
  UInt m_uiCode;
  UInt m_uiLen;
};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_UVLCREADER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_)
