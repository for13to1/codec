/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2016, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TEncCavlc.h
    \brief    CAVLC encoder class (header)
*/

#ifndef __TENCCAVLC__
#define __TENCCAVLC__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TLibCommon/CommonDef.h"
#include "TLibCommon/TComBitStream.h"
#include "TLibCommon/TComRom.h"
#include "TEncEntropy.h"
#include "SyntaxElementWriter.h"

//! \ingroup TLibEncoder
//! \{

class TEncTop;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

class AUDWriter : public SyntaxElementWriter
{
public:
  AUDWriter() {};
  virtual ~AUDWriter() {};

  Void  codeAUD(TComBitIf& bs, const Int pictureType);
};

/// CAVLC encoder class
class TEncCavlc : public SyntaxElementWriter, public TEncEntropyIf
{
public:
  TEncCavlc();
  virtual ~TEncCavlc();

protected:
#if NH_MV
  TEncTop*      m_encTop; 
#endif

  Void codeShortTermRefPicSet              ( const TComReferencePictureSet* pcRPS, Bool calledFromSliceHeader, Int idx );
  Bool findMatchingLTRP ( TComSlice* pcSlice, UInt *ltrpsIndex, Int ltrpPOC, Bool usedFlag );

public:

  Void  resetEntropy          (const TComSlice *pSlice);
  SliceType determineCabacInitIdx  (const TComSlice* /*pSlice*/) { assert(0); return I_SLICE; };

  Void  setBitstream          ( TComBitIf* p )  { m_pcBitIf = p;  }
  Void  resetBits             ()                { m_pcBitIf->resetBits(); }
  UInt  getNumberOfWrittenBits()                { return  m_pcBitIf->getNumberOfWrittenBits();  }
  Void  codeVPS                 ( const TComVPS* pcVPS );
#if NH_MV
  Void  codeVPSExtension       ( const TComVPS *pcVPS );
  Void  codeVideoSignalInfo    ( const TComVideoSignalInfo* pcVideoSignalInfo ); 

  Void  codeDpbSize            ( const TComVPS* vps );

  Void  codeRepFormat           ( Int i, const TComRepFormat* curRepFormat, const TComRepFormat* prevRepFormat );
  Void  codeVPSVUI              ( const TComVPS* pcVPS );
  Void  codeVpsVuiBspHrdParameters( const TComVPS* pcVPS ); 
#endif 

  Void  codeVUI                 ( const TComVUI *pcVUI, const TComSPS* pcSPS );
#if NH_MV
  Void  codeSPSExtension        ( const TComSPS* pcSPS ); 
#endif
#if NH_3D
  Void  codeVPS3dExtension      ( const TComVPS* pcVPS ); 
  Void  codeSPS3dExtension      ( const TComSPS* pcSPS );
#endif
  Void  codeSPS                 ( const TComSPS* pcSPS );
  Void  codePPS                 ( const TComPPS* pcPPS );
#if NH_MV
  Void  codePpsMultilayerExtension( const TComPPS* pcPPS );
#endif
#if NH_3D
  Void  codePps3dExtension       ( const TComPPS* pcPPS );
#endif
  Void  codeSliceHeader         ( TComSlice* pcSlice );
  Void  codePTL                 ( const TComPTL* pcPTL, Bool profilePresentFlag, Int maxNumSubLayersMinus1);
  Void  codeProfileTier         ( const ProfileTierLevel* ptl, const Bool bIsSubLayer );
  Void  codeHrdParameters       ( const TComHRD *hrd, Bool commonInfPresentFlag, UInt maxNumSubLayersMinus1 );
  Void  codeTilesWPPEntryPoint( TComSlice* pSlice );
  Void  codeTerminatingBit      ( UInt uilsLast );
  Void  codeSliceFinish         ();

  Void codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void codeSAOBlkParam(SAOBlkParam& /*saoBlkParam*/, const BitDepths& /*bitDepths*/, Bool* /*sliceEnabled*/, Bool /*leftMergeAvail*/, Bool /*aboveMergeAvail*/, Bool /*onlyEstMergeInfo*/ = false){printf("only supported in CABAC"); assert(0); exit(-1);}
  Void codeCUTransquantBypassFlag( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if NH_3D_DIS
  Void codeDIS           ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif
  Void codeMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if NH_3D_DMM || NH_3D_SDC_INTRA || NH_3D_SDC_INTER
  Void  codeDeltaDC      ( TComDataCU* /*pcCU*/, UInt /*absPartIdx*/ ) { assert(0); };
#endif
#if NH_3D_ARP
  Void codeARPW          ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif
#if NH_3D_IC
  Void codeICFlag        ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif

#if NH_3D_SDC_INTRA || NH_3D_SDC_INTER
  Void codeSDCFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif
#if NH_3D_DBBP
  Void codeDBBPFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif
  Void codeAlfCtrlFlag   ( ComponentID /*component*/, UInt /*code*/ ) {printf("Not supported\n"); assert(0);}
  Void codeInterModeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiEncMode );
  Void codeSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void codePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void codePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx );

  Void codeIPCMInfo      ( TComDataCU* pcCU, UInt uiAbsPartIdx );

  Void codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx );
  Void codeQtCbf         ( TComTU &rTu, const ComponentID compID, const Bool lowestLevel );
  Void codeQtRootCbf     ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeQtCbfZero     ( TComTU &rTu, const ChannelType chType );
  Void codeQtRootCbfZero ( );
  Void codeIntraDirLumaAng( TComDataCU* pcCU, UInt absPartIdx, Bool isMultiple);
  Void codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeInterDir      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeRefFrmIdx     ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void codeMvd           ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );

  Void codeCrossComponentPrediction( TComTU &rTu, ComponentID compID );

  Void codeDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeChromaQpAdjustment( TComDataCU* pcCU, UInt uiAbsPartIdx );

  Void codeCoeffNxN      ( TComTU &rTu, TCoeff* pcCoef, const ComponentID compID );
  Void codeTransformSkipFlags ( TComTU &rTu, ComponentID component );

  Void estBit            ( estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, ChannelType chType );

  Void xCodePredWeightTable          ( TComSlice* pcSlice );

  Void codeScalingList  ( const TComScalingList &scalingList );
  Void xCodeScalingList ( const TComScalingList* scalingList, UInt sizeId, UInt listId);

  Void codeExplicitRdpcmMode( TComTU &rTu, const ComponentID compID );

#if NH_MV
  TEncTop* getEncTop()               { return m_encTop; };
  Void     setEncTop( TEncTop* et )  {  m_encTop = et; };
#endif

};

//! \}

#endif // !defined(AFX_TENCCAVLC_H__EE8A0B30_945B_4169_B290_24D3AD52296F__INCLUDED_)

