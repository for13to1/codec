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

#if !defined(AFX_CABACWRITER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_)
#define AFX_CABACWRITER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MbSymbolWriteIf.h"
#include "H264AVCCommonLib/CabacContextModel2DBuffer.h"
#include "BitWriteBufferIf.h"
#include "CabaEncoder.h"
#include "H264AVCCommonLib/ContextTables.h"


H264AVC_NAMESPACE_BEGIN

class CabacWriter :
public MbSymbolWriteIf
, private CabaEncoder
{
public:
//protected://JVT-X046
	CabacWriter();
	virtual ~CabacWriter();

public:
  static ErrVal create( CabacWriter*& rpcCabacWriter );
  ErrVal destroy();

  ErrVal init( BitWriteBufferIf* pcBitWriteBufferIf );
  ErrVal uninit();

  MbSymbolWriteIf* getSymbolWriteIfNextSlice();
  Void             setTraceEnableBit( Bool bActive ) { CabacWriter::m_bTraceEnable = bActive; CabaEncoder::m_bTraceEnable = bActive; }

  ErrVal  startSlice( const SliceHeader& rcSliceHeader );
  ErrVal  getLastByte(UChar &uiLastByte, UInt &uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal  setFirstBits(UChar ucByte,UInt uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal  finishSlice();


  ErrVal  fieldFlag           ( MbDataAccess& rcMbDataAccess );

  ErrVal  blockModes( MbDataAccess& rcMbDataAccess );
  ErrVal  mbMode( MbDataAccess& rcMbDataAccess/*, Bool bBLQRefFlag*/ );
  ErrVal  resPredFlag( MbDataAccess& rcMbDataAccess );

  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );

  ErrVal  cbp( MbDataAccess& rcMbDataAccess, UInt uiStart, UInt uiStop );

  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, ResidualMode eResidualMode, UInt uiStart = 0, UInt uiStop = 16 );
  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode, UInt uiStart = 0, UInt uiStop = 16 );

  ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess, UInt uiStart, UInt uiStop );
  ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx, ResidualMode eResidualMode, UInt uiStart = 0, UInt uiStop = 16 );

  ErrVal  deltaQp( MbDataAccess& rcMbDataAccess );
  ErrVal  intraPredModeLuma( MbDataAccess& rcMbDataAccess, LumaIdx cIdx );
  ErrVal  intraPredModeChroma( MbDataAccess& rcMbDataAccess );
  ErrVal  samplesPCM( MbDataAccess& rcMbDataAccess );
  ErrVal  skipFlag( MbDataAccess& rcMbDataAccess );
  ErrVal  BLSkipFlag( MbDataAccess& rcMbDataAccess );
  ErrVal  terminatingBit ( UInt uiIsLast );
  UInt getNumberOfWrittenBits();

	//JVT-X046 {
	CabacContextModel2DBuffer& getFieldFlagCCModel(void)  {return m_cFieldFlagCCModel;   }
	CabacContextModel2DBuffer& getFldMapCCModel(void)     {return m_cFldMapCCModel;      }
	CabacContextModel2DBuffer& getFldLastCCModel(void)    {return m_cFldLastCCModel;     }
	CabacContextModel2DBuffer& getBLSkipCCModel(void)     {return m_cBLSkipCCModel;      }

	CabacContextModel2DBuffer& getBCbpCCModel(void)       {return m_cBCbpCCModel;        }
	CabacContextModel2DBuffer& getMapCCModel(void)        {return m_cMapCCModel;         }
	CabacContextModel2DBuffer& getLastCCModel(void)       {return m_cLastCCModel;        }

	CabacContextModel2DBuffer& getOneCCModel(void)        {return m_cOneCCModel;         }
	CabacContextModel2DBuffer& getAbsCCModel(void)        {return m_cAbsCCModel;         }
	CabacContextModel2DBuffer& getChromaPredCCModel(void) {return m_cChromaPredCCModel;  }

	CabacContextModel2DBuffer& getMbTypeCCModel(void)     {return m_cMbTypeCCModel;      }
	CabacContextModel2DBuffer& getBlockTypeCCModel(void)  {return m_cBlockTypeCCModel;   }
	CabacContextModel2DBuffer& getMvdCCModel(void)        {return m_cMvdCCModel;         }
	CabacContextModel2DBuffer& getRefPicCCModel(void)     {return m_cRefPicCCModel;      }
  CabacContextModel2DBuffer& getMotPredFlagCCModel(void){return m_cMotPredFlagCCModel; }
	CabacContextModel2DBuffer& getResPredFlagCCModel(void){return m_cResPredFlagCCModel; }
	CabacContextModel2DBuffer& getDeltaQpCCModel(void)    {return m_cDeltaQpCCModel;     }
	CabacContextModel2DBuffer& getIntraPredCCModel(void)  {return m_cIntraPredCCModel;   }
	CabacContextModel2DBuffer& getCbpCCModel(void)        {return m_cCbpCCModel;         }
	CabacContextModel2DBuffer& getTransSizeCCModel(void)  {return m_cTransSizeCCModel;   }

	void loadCabacWrite(MbSymbolWriteIf *pcMbSymbolWriteIf);
	void loadUvlcWrite(MbSymbolWriteIf *pcMbSymbolWriteIf) { }
    UInt getBitsWritten(void) { return m_pcBitWriteBufferIf->getBitsWritten(); }
  //JVT-X046 }

protected:
  ErrVal xInitContextModels( const SliceHeader& rcSliceHeader );

  ErrVal xWriteMvdComponent( Short sMvdComp, UInt uiAbsSum, UInt uiCtx );
  ErrVal xWriteMvd( MbDataAccess& rcMbDataAccess, Mv cMv, LumaIdx cIdx, ListIdx eLstIdx );
  ErrVal xRefFrame      ( MbDataAccess& rcMbDataAccess, UInt uiRefFrame, ListIdx eLstIdx, ParIdx8x8 eParIdx );
  ErrVal xMotionPredFlag( Bool bFlag,      ListIdx eLstIdx );

  ErrVal xWriteBCbp( MbDataAccess& rcMbDataAccess, UInt uiNumSig, ResidualMode eResidualMode, LumaIdx cIdx, UInt uiStart, UInt uiStop );
  ErrVal xWriteBCbp( MbDataAccess& rcMbDataAccess, UInt uiNumSig, ResidualMode eResidualMode, ChromaIdx cIdx, UInt uiStart, UInt uiStop );
  ErrVal xWriteCoeff( UInt          uiNumSig,
                      TCoeff*       piCoeff,
                      ResidualMode  eResidualMode,
                      const UChar*  pucScan,
                      Bool          bFrame,
                      UInt          uiStart,
                      UInt          uiStop );
  UInt   xGetNumberOfSigCoeff( TCoeff* piCoeff, ResidualMode eResidualMode, const UChar* pucScan, UInt uiStart, UInt uiStop );
  ErrVal xWriteBlockMode( UInt uiBlockMode );


protected:
  CabacContextModel2DBuffer m_cFieldFlagCCModel;
  CabacContextModel2DBuffer m_cFldMapCCModel;
  CabacContextModel2DBuffer m_cFldLastCCModel;
  CabacContextModel2DBuffer m_cBLSkipCCModel;

  CabacContextModel2DBuffer m_cBCbpCCModel;
  CabacContextModel2DBuffer m_cMapCCModel;
  CabacContextModel2DBuffer m_cLastCCModel;

  CabacContextModel2DBuffer m_cOneCCModel;
  CabacContextModel2DBuffer m_cAbsCCModel;
  CabacContextModel2DBuffer m_cChromaPredCCModel;

  CabacContextModel2DBuffer m_cMbTypeCCModel;
  CabacContextModel2DBuffer m_cBlockTypeCCModel;
  CabacContextModel2DBuffer m_cMvdCCModel;
  CabacContextModel2DBuffer m_cRefPicCCModel;
  CabacContextModel2DBuffer m_cMotPredFlagCCModel;
  CabacContextModel2DBuffer m_cResPredFlagCCModel;
  CabacContextModel2DBuffer m_cDeltaQpCCModel;
  CabacContextModel2DBuffer m_cIntraPredCCModel;
  CabacContextModel2DBuffer m_cCbpCCModel;
  CabacContextModel2DBuffer m_cTransSizeCCModel;

  const SliceHeader* m_pcSliceHeader;
  UInt m_uiBitCounter;
  UInt m_uiPosCounter;
  UInt m_uiLastDQpNonZero;
  Bool m_bTraceEnable;

  // new variables for switching bitstream inputs
  CabaEncoder*    m_apcCabacEncoder [MAX_NUM_PD_FRAGMENTS];
  CabacWriter    *m_pcNextCabacWriter;
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_CABACWRITER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_)
