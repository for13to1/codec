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

/** \file     TEncSlice.cpp
    \brief    slice encoder class
*/

#include "TEncTop.h"
#include "TEncSlice.h"
#include <math.h>

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncSlice::TEncSlice()
 : m_encCABACTableIdx(I_SLICE)
{
}

TEncSlice::~TEncSlice()
{
  destroy();
}

Void TEncSlice::create( Int iWidth, Int iHeight, ChromaFormat chromaFormat, UInt iMaxCUWidth, UInt iMaxCUHeight, UChar uhTotalDepth )
{
  // create prediction picture
  m_picYuvPred.create( iWidth, iHeight, chromaFormat, iMaxCUWidth, iMaxCUHeight, uhTotalDepth, true );

  // create residual picture
  m_picYuvResi.create( iWidth, iHeight, chromaFormat, iMaxCUWidth, iMaxCUHeight, uhTotalDepth, true );
}

Void TEncSlice::destroy()
{
  m_picYuvPred.destroy();
  m_picYuvResi.destroy();

  // free lambda and QP arrays
  m_vdRdPicLambda.clear();
  m_vdRdPicQp.clear();
  m_viRdPicQp.clear();
}

Void TEncSlice::init( TEncTop* pcEncTop )
{
  m_pcCfg             = pcEncTop;
  m_pcListPic         = pcEncTop->getListPic();

  m_pcGOPEncoder      = pcEncTop->getGOPEncoder();
  m_pcCuEncoder       = pcEncTop->getCuEncoder();
  m_pcPredSearch      = pcEncTop->getPredSearch();

  m_pcEntropyCoder    = pcEncTop->getEntropyCoder();
  m_pcSbacCoder       = pcEncTop->getSbacCoder();
  m_pcBinCABAC        = pcEncTop->getBinCABAC();
  m_pcTrQuant         = pcEncTop->getTrQuant();

  m_pcRdCost          = pcEncTop->getRdCost();
  m_pppcRDSbacCoder   = pcEncTop->getRDSbacCoder();
  m_pcRDGoOnSbacCoder = pcEncTop->getRDGoOnSbacCoder();

  // create lambda and QP arrays
  m_vdRdPicLambda.resize(m_pcCfg->getDeltaQpRD() * 2 + 1 );
  m_vdRdPicQp.resize(    m_pcCfg->getDeltaQpRD() * 2 + 1 );
  m_viRdPicQp.resize(    m_pcCfg->getDeltaQpRD() * 2 + 1 );
  m_pcRateCtrl        = pcEncTop->getRateCtrl();

  m_numIDRs     = SCM_T0048_PALETTE_PRED_IN_PPS_REFRESH;
  m_numFrames   = 0;
}



#if SHARP_LUMA_DELTA_QP
Void TEncSlice::updateLambda(TComSlice* pSlice, Double dQP)
{
  Int iQP = (Int)dQP;
  Double dLambda = calculateLambda(pSlice, m_gopID, pSlice->getDepth(), pSlice->getSliceQp(), dQP, iQP);

  setUpLambda(pSlice, dLambda, iQP);
}
#endif

Void
TEncSlice::setUpLambda(TComSlice* slice, const Double dLambda, Int iQP)
{
  m_pcRdCost->setRGBFormatFlag               (  m_pcCfg->getRGBFormatFlag() );
  m_pcRdCost->setUseColourTrans              (  slice->getPPS()->getPpsScreenExtension().getUseColourTrans() );
  m_pcRdCost->setUseLossless                 (  m_pcCfg->getUseLossless() );

  // store lambda
  m_pcRdCost ->setLambda( dLambda, slice->getSPS()->getBitDepths() );

  Int map[52] =
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 3, 3, 4,
    4, 5, 5, 6, 6, 6, 6, 6, 6, 6,
    6, 6
  };

  // for RDO
  // in RdCost there is only one lambda because the luma and chroma bits are not separated, instead we weight the distortion of chroma.
  Double dLambdas[MAX_NUM_COMPONENT] = { dLambda };
  for(UInt compIdx=1; compIdx<MAX_NUM_COMPONENT; compIdx++)
  {
    const ComponentID compID=ComponentID(compIdx);
    Int chromaQPOffset = slice->getPPS()->getQpOffset(compID) + slice->getSliceChromaQpDelta(compID);
    Int qpc=(iQP + chromaQPOffset < 0) ? iQP : getScaledChromaQP(iQP + chromaQPOffset, m_pcCfg->getChromaFormatIdc());
    Double tmpWeight = pow( 2.0, (iQP-qpc)/3.0 );  // takes into account of the chroma qp mapping and chroma qp Offset
    if(m_pcCfg->getRGBFormatFlag() && slice->getPPS()->getPpsScreenExtension().getUseColourTrans())
    {
      tmpWeight = tmpWeight*pow( 2.0, (0-map[iQP])/3.0 );
    }
    m_pcRdCost->setDistortionWeight(compID, tmpWeight);
    dLambdas[compIdx]=dLambda/tmpWeight;
  }

#if RDOQ_CHROMA_LAMBDA
// for RDOQ
  m_pcTrQuant->setLambdas( dLambdas );
#else
  m_pcTrQuant->setLambda( dLambda );
#endif

// For SAO
  slice->setLambdas( dLambdas );
}



/**
 - non-referenced frame marking
 - QP computation based on temporal structure
 - lambda computation based on QP
 - set temporal layer ID and the parameter sets
 .
 \param pcPic         picture class
 \param pocLast       POC of last picture
 \param pocCurr       current POC
 \param iNumPicRcvd   number of received pictures
 \param iGOPid        POC offset for hierarchical structure
 \param rpcSlice      slice header class
 \param isField       true for field coding
 */

Void TEncSlice::initEncSlice( TComPic* pcPic, const Int pocLast, const Int pocCurr, const Int iGOPid, TComSlice*& rpcSlice, const Bool isField )
{
  Double dQP;
  Double dLambda;

  rpcSlice = pcPic->getSlice(0);
  rpcSlice->setSliceBits(0);
  rpcSlice->setPic( pcPic );
  rpcSlice->initSlice();
  rpcSlice->setPicOutputFlag( true );
  rpcSlice->setPOC( pocCurr );

#if SHARP_LUMA_DELTA_QP
  pcPic->setField(isField);
  m_gopID = iGOPid;
#endif

  // depth computation based on GOP size
  Int depth;
  {
    Int poc = rpcSlice->getPOC();
    if(isField)
    {
      poc = (poc/2) % (m_pcCfg->getGOPSize()/2);
    }
    else
    {
      poc = poc % m_pcCfg->getGOPSize();   
    }

    if ( poc == 0 )
    {
      depth = 0;
    }
    else
    {
      Int step = m_pcCfg->getGOPSize();
      depth    = 0;
      for( Int i=step>>1; i>=1; i>>=1 )
      {
        for ( Int j=i; j<m_pcCfg->getGOPSize(); j+=step )
        {
          if ( j == poc )
          {
            i=0;
            break;
          }
        }
        step >>= 1;
        depth++;
      }
    }

    if(m_pcCfg->getHarmonizeGopFirstFieldCoupleEnabled() && poc != 0)
    {
      if (isField && ((rpcSlice->getPOC() % 2) == 1))
      {
        depth ++;
      }
    }
  }

  // slice type
  SliceType eSliceType;

  eSliceType=B_SLICE;
  if(!(isField && pocLast == 1) || !m_pcCfg->getEfficientFieldIRAPEnabled())
  {
    if(m_pcCfg->getDecodingRefreshType() == 3)
    {
      eSliceType = (pocLast == 0 || pocCurr % m_pcCfg->getIntraPeriod() == 0             || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
    }
    else
    {
      eSliceType = (pocLast == 0 || (pocCurr - (isField ? 1 : 0)) % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
    }
  }

  rpcSlice->setSliceType    ( eSliceType );

  // ------------------------------------------------------------------------------------------------------------------
  // Non-referenced frame marking
  // ------------------------------------------------------------------------------------------------------------------

  if(pocLast == 0)
  {
    rpcSlice->setTemporalLayerNonReferenceFlag(false);
  }
  else
  {
    rpcSlice->setTemporalLayerNonReferenceFlag(!m_pcCfg->getGOPEntry(iGOPid).m_refPic);
  }
  rpcSlice->setReferenced(true);

  // ------------------------------------------------------------------------------------------------------------------
  // QP setting
  // ------------------------------------------------------------------------------------------------------------------

#if X0038_LAMBDA_FROM_QP_CAPABILITY
  dQP = m_pcCfg->getQPForPicture(iGOPid, rpcSlice);
#else
  dQP = m_pcCfg->getQP();
  if(eSliceType!=I_SLICE)
  {
#if SHARP_LUMA_DELTA_QP
    if (!(( m_pcCfg->getMaxDeltaQP() == 0) && (!m_pcCfg->getLumaLevelToDeltaQPMapping().isEnabled()) && (dQP == -rpcSlice->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA) ) && (rpcSlice->getPPS()->getTransquantBypassEnabledFlag())))
#else
    if (!(( m_pcCfg->getMaxDeltaQP() == 0 ) && (dQP == -rpcSlice->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA) ) && (rpcSlice->getPPS()->getTransquantBypassEnabledFlag())))
#endif
    {
      dQP += m_pcCfg->getGOPEntry(iGOPid).m_QPOffset;
    }
  }

  // modify QP
  const Int* pdQPs = m_pcCfg->getdQPs();
  if ( pdQPs )
  {
    dQP += pdQPs[ rpcSlice->getPOC() ];
  }

  if (m_pcCfg->getCostMode()==COST_LOSSLESS_CODING)
  {
    dQP=LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP;
    m_pcCfg->setDeltaQpRD(0);
  }
#endif

  // ------------------------------------------------------------------------------------------------------------------
  // Lambda computation
  // ------------------------------------------------------------------------------------------------------------------

#if X0038_LAMBDA_FROM_QP_CAPABILITY
  const Int temporalId=m_pcCfg->getGOPEntry(iGOPid).m_temporalId;
#if !SHARP_LUMA_DELTA_QP
  const std::vector<Double> &intraLambdaModifiers=m_pcCfg->getIntraLambdaModifier();
#endif
#endif
  Int iQP;
  Double dOrigQP = dQP;

  // pre-compute lambda and QP values for all possible QP candidates
  for ( Int iDQpIdx = 0; iDQpIdx < 2 * m_pcCfg->getDeltaQpRD() + 1; iDQpIdx++ )
  {
    // compute QP value
    dQP = dOrigQP + ((iDQpIdx+1)>>1)*(iDQpIdx%2 ? -1 : 1);
#if SHARP_LUMA_DELTA_QP
    dLambda = calculateLambda(rpcSlice, iGOPid, depth, dQP, dQP, iQP );
#else
    // compute lambda value
    Int    NumberBFrames = ( m_pcCfg->getGOPSize() - 1 );
    Int    SHIFT_QP = 12;

#if FULL_NBIT
    Int    bitdepth_luma_qp_scale = 6 * (rpcSlice->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA) - 8);
#else
    Int    bitdepth_luma_qp_scale = 0;
#endif
    Double qp_temp = (Double) dQP + bitdepth_luma_qp_scale - SHIFT_QP;
#if FULL_NBIT
    Double qp_temp_orig = (Double) dQP - SHIFT_QP;
#endif
    // Case #1: I or P-slices (key-frame)
    Double dQPFactor = m_pcCfg->getGOPEntry(iGOPid).m_QPFactor;
    if ( eSliceType==I_SLICE )
    {
      if (m_pcCfg->getIntraQpFactor()>=0.0 && m_pcCfg->getGOPEntry(iGOPid).m_sliceType != I_SLICE)
      {
        dQPFactor=m_pcCfg->getIntraQpFactor();
      }
      else
      {
#if X0038_LAMBDA_FROM_QP_CAPABILITY
        if(m_pcCfg->getLambdaFromQPEnable())
        {
          dQPFactor=0.57;
        }
        else
        {
#endif
        Double dLambda_scale = 1.0 - Clip3( 0.0, 0.5, 0.05*(Double)(isField ? NumberBFrames/2 : NumberBFrames) );
        
        dQPFactor=0.57*dLambda_scale;
#if X0038_LAMBDA_FROM_QP_CAPABILITY
        }
#endif
      }
    }
#if X0038_LAMBDA_FROM_QP_CAPABILITY
    else if( m_pcCfg->getLambdaFromQPEnable() )
    {
      dQPFactor=0.57;
    }
#endif

    dLambda = dQPFactor*pow( 2.0, qp_temp/3.0 );

#if X0038_LAMBDA_FROM_QP_CAPABILITY
    if(!m_pcCfg->getLambdaFromQPEnable() && depth>0)
#else
    if ( depth>0 )
#endif
    {
#if FULL_NBIT
        dLambda *= Clip3( 2.00, 4.00, (qp_temp_orig / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
#else
        dLambda *= Clip3( 2.00, 4.00, (qp_temp / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
#endif
    }

    // if hadamard is used in ME process
    if ( !m_pcCfg->getUseHADME() && rpcSlice->getSliceType( ) != I_SLICE )
    {
      dLambda *= 0.95;
    }

#if X0038_LAMBDA_FROM_QP_CAPABILITY
    Double lambdaModifier;
    if( rpcSlice->getSliceType( ) != I_SLICE || intraLambdaModifiers.empty())
    {
      lambdaModifier = m_pcCfg->getLambdaModifier( temporalId );
    }
    else
    {
      lambdaModifier = intraLambdaModifiers[ (temporalId < intraLambdaModifiers.size()) ? temporalId : (intraLambdaModifiers.size()-1) ];
    }
    dLambda *= lambdaModifier;
#endif

    iQP = max( -rpcSlice->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), min( MAX_QP, (Int) floor( dQP + 0.5 ) ) );
#endif

    m_vdRdPicLambda[iDQpIdx] = dLambda;
    m_vdRdPicQp    [iDQpIdx] = dQP;
    m_viRdPicQp    [iDQpIdx] = iQP;
  }

  // obtain dQP = 0 case
  dLambda = m_vdRdPicLambda[0];
  dQP     = m_vdRdPicQp    [0];
  iQP     = m_viRdPicQp    [0];

#if !X0038_LAMBDA_FROM_QP_CAPABILITY
  const Int temporalId=m_pcCfg->getGOPEntry(iGOPid).m_temporalId;
  const std::vector<Double> &intraLambdaModifiers=m_pcCfg->getIntraLambdaModifier();
#endif

#if W0038_CQP_ADJ
  if(rpcSlice->getPPS()->getSliceChromaQpFlag())
  {
    const Bool bUseIntraOrPeriodicOffset = rpcSlice->getSliceType()==I_SLICE || (m_pcCfg->getSliceChromaOffsetQpPeriodicity()!=0 && (rpcSlice->getPOC()%m_pcCfg->getSliceChromaOffsetQpPeriodicity())==0);
    Int cbQP = bUseIntraOrPeriodicOffset? m_pcCfg->getSliceChromaOffsetQpIntraOrPeriodic(false) : m_pcCfg->getGOPEntry(iGOPid).m_CbQPoffset;
    Int crQP = bUseIntraOrPeriodicOffset? m_pcCfg->getSliceChromaOffsetQpIntraOrPeriodic(true)  : m_pcCfg->getGOPEntry(iGOPid).m_CrQPoffset;

    cbQP = Clip3( -12, 12, cbQP + rpcSlice->getPPS()->getQpOffset(COMPONENT_Cb) ) - rpcSlice->getPPS()->getQpOffset(COMPONENT_Cb); 
    crQP = Clip3( -12, 12, crQP + rpcSlice->getPPS()->getQpOffset(COMPONENT_Cr) ) - rpcSlice->getPPS()->getQpOffset(COMPONENT_Cr); 
    rpcSlice->setSliceChromaQpDelta(COMPONENT_Cb, Clip3( -12, 12, cbQP));
    assert(rpcSlice->getSliceChromaQpDelta(COMPONENT_Cb)+rpcSlice->getPPS()->getQpOffset(COMPONENT_Cb)<=12 && rpcSlice->getSliceChromaQpDelta(COMPONENT_Cb)+rpcSlice->getPPS()->getQpOffset(COMPONENT_Cb)>=-12);
    rpcSlice->setSliceChromaQpDelta(COMPONENT_Cr, Clip3( -12, 12, crQP));
    assert(rpcSlice->getSliceChromaQpDelta(COMPONENT_Cr)+rpcSlice->getPPS()->getQpOffset(COMPONENT_Cr)<=12 && rpcSlice->getSliceChromaQpDelta(COMPONENT_Cr)+rpcSlice->getPPS()->getQpOffset(COMPONENT_Cr)>=-12);
  }
  else
  {
    rpcSlice->setSliceChromaQpDelta( COMPONENT_Cb, 0 );
    rpcSlice->setSliceChromaQpDelta( COMPONENT_Cr, 0 );
  }
#endif

#if !X0038_LAMBDA_FROM_QP_CAPABILITY
  Double lambdaModifier;
  if( rpcSlice->getSliceType( ) != I_SLICE || intraLambdaModifiers.empty())
  {
    lambdaModifier = m_pcCfg->getLambdaModifier( temporalId );
  }
  else
  {
    lambdaModifier = intraLambdaModifiers[ (temporalId < intraLambdaModifiers.size()) ? temporalId : (intraLambdaModifiers.size()-1) ];
  }

  dLambda *= lambdaModifier;
#endif

  setUpLambda(rpcSlice, dLambda, iQP);

  if (m_pcCfg->getFastMEForGenBLowDelayEnabled())
  {
    // restore original slice type

    if(!(isField && pocLast == 1) || !m_pcCfg->getEfficientFieldIRAPEnabled())
    {
      if(m_pcCfg->getDecodingRefreshType() == 3)
      {
        eSliceType = (pocLast == 0 || (pocCurr)                     % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
      }
      else
      {
        eSliceType = (pocLast == 0 || (pocCurr - (isField ? 1 : 0)) % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
      }
    }

    rpcSlice->setSliceType        ( eSliceType );
  }

  if (m_pcCfg->getUseRecalculateQPAccordingToLambda())
  {
    dQP = xGetQPValueAccordingToLambda( dLambda );
    iQP = max( -rpcSlice->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), min( MAX_QP, (Int) floor( dQP + 0.5 ) ) );
  }

  rpcSlice->setSliceQp           ( iQP );
#if ADAPTIVE_QP_SELECTION
  rpcSlice->setSliceQpBase       ( iQP );
#endif
  rpcSlice->setSliceQpDelta      ( 0 );
#if !W0038_CQP_ADJ
  rpcSlice->setSliceChromaQpDelta( COMPONENT_Cb, 0 );
  rpcSlice->setSliceChromaQpDelta( COMPONENT_Cr, 0 );
#endif
  rpcSlice->setUseChromaQpAdj( rpcSlice->getPPS()->getPpsRangeExtension().getChromaQpOffsetListEnabledFlag() );
  rpcSlice->setNumRefIdx(REF_PIC_LIST_0,m_pcCfg->getGOPEntry(iGOPid).m_numRefPicsActive);
  rpcSlice->setNumRefIdx(REF_PIC_LIST_1,m_pcCfg->getGOPEntry(iGOPid).m_numRefPicsActive);

  if ( m_pcCfg->getDeblockingFilterMetric() )
  {
    rpcSlice->setDeblockingFilterOverrideFlag(true);
    rpcSlice->setDeblockingFilterDisable(false);
    rpcSlice->setDeblockingFilterBetaOffsetDiv2( 0 );
    rpcSlice->setDeblockingFilterTcOffsetDiv2( 0 );
  }
  else if (rpcSlice->getPPS()->getDeblockingFilterControlPresentFlag())
  {
    rpcSlice->setDeblockingFilterOverrideFlag( rpcSlice->getPPS()->getDeblockingFilterOverrideEnabledFlag() );
    rpcSlice->setDeblockingFilterDisable( rpcSlice->getPPS()->getPPSDeblockingFilterDisabledFlag() );
    if ( !rpcSlice->getDeblockingFilterDisable())
    {
      if ( rpcSlice->getDeblockingFilterOverrideFlag() && eSliceType!=I_SLICE)
      {
        rpcSlice->setDeblockingFilterBetaOffsetDiv2( m_pcCfg->getGOPEntry(iGOPid).m_betaOffsetDiv2 + m_pcCfg->getLoopFilterBetaOffset()  );
        rpcSlice->setDeblockingFilterTcOffsetDiv2( m_pcCfg->getGOPEntry(iGOPid).m_tcOffsetDiv2 + m_pcCfg->getLoopFilterTcOffset() );
      }
      else
      {
        rpcSlice->setDeblockingFilterBetaOffsetDiv2( m_pcCfg->getLoopFilterBetaOffset() );
        rpcSlice->setDeblockingFilterTcOffsetDiv2( m_pcCfg->getLoopFilterTcOffset() );
      }
    }
  }
  else
  {
    rpcSlice->setDeblockingFilterOverrideFlag( false );
    rpcSlice->setDeblockingFilterDisable( false );
    rpcSlice->setDeblockingFilterBetaOffsetDiv2( 0 );
    rpcSlice->setDeblockingFilterTcOffsetDiv2( 0 );
  }

  rpcSlice->setDepth            ( depth );

  pcPic->setTLayer( temporalId );
  if(eSliceType==I_SLICE)
  {
    pcPic->setTLayer(0);
  }
  rpcSlice->setTLayer( pcPic->getTLayer() );

  pcPic->setPicYuvPred( &m_picYuvPred );
  pcPic->setPicYuvResi( &m_picYuvResi );
  rpcSlice->setSliceMode            ( m_pcCfg->getSliceMode()            );
  rpcSlice->setSliceArgument        ( m_pcCfg->getSliceArgument()        );
  rpcSlice->setSliceSegmentMode     ( m_pcCfg->getSliceSegmentMode()     );
  rpcSlice->setSliceSegmentArgument ( m_pcCfg->getSliceSegmentArgument() );
  rpcSlice->setMaxNumMergeCand      ( m_pcCfg->getMaxNumMergeCand()      );
}


#if SHARP_LUMA_DELTA_QP
Double TEncSlice::calculateLambda( const TComSlice* slice,
                                   const Int        GOPid, // entry in the GOP table
                                   const Int        depth, // slice GOP hierarchical depth.
                                   const Double     refQP, // initial slice-level QP
                                   const Double     dQP,   // initial double-precision QP
                                         Int       &iQP )  // returned integer QP.
{
  enum   SliceType eSliceType    = slice->getSliceType();
  const  Bool      isField       = slice->getPic()->isField();
  const  Int       NumberBFrames = ( m_pcCfg->getGOPSize() - 1 );
  const  Int       SHIFT_QP      = 12;
#if X0038_LAMBDA_FROM_QP_CAPABILITY
  const Int temporalId=m_pcCfg->getGOPEntry(GOPid).m_temporalId;
  const std::vector<Double> &intraLambdaModifiers=m_pcCfg->getIntraLambdaModifier();
#endif

#if FULL_NBIT
  Int    bitdepth_luma_qp_scale = 6 * (slice->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA) - 8);
#else
  Int    bitdepth_luma_qp_scale = 0;
#endif
  Double qp_temp = dQP + bitdepth_luma_qp_scale - SHIFT_QP;
  // Case #1: I or P-slices (key-frame)
  Double dQPFactor = m_pcCfg->getGOPEntry(GOPid).m_QPFactor;
  if ( eSliceType==I_SLICE )
  {
    if (m_pcCfg->getIntraQpFactor()>=0.0 && m_pcCfg->getGOPEntry(GOPid).m_sliceType != I_SLICE)
    {
      dQPFactor=m_pcCfg->getIntraQpFactor();
    }
    else
    {
#if X0038_LAMBDA_FROM_QP_CAPABILITY
      if(m_pcCfg->getLambdaFromQPEnable())
      {
        dQPFactor=0.57;
      }
      else
      {
#endif
        Double dLambda_scale = 1.0 - Clip3( 0.0, 0.5, 0.05*(Double)(isField ? NumberBFrames/2 : NumberBFrames) );
        dQPFactor=0.57*dLambda_scale;
#if X0038_LAMBDA_FROM_QP_CAPABILITY
      }
#endif
    }
  }
#if X0038_LAMBDA_FROM_QP_CAPABILITY
  else if( m_pcCfg->getLambdaFromQPEnable() )
  {
    dQPFactor=0.57;
  }
#endif

  Double dLambda = dQPFactor*pow( 2.0, qp_temp/3.0 );

#if X0038_LAMBDA_FROM_QP_CAPABILITY
  if( !(m_pcCfg->getLambdaFromQPEnable()) && depth>0 )
#else
  if ( depth>0 )
#endif
  {
#if FULL_NBIT
      Double qp_temp_ref_orig = refQP - SHIFT_QP;
      dLambda *= Clip3( 2.00, 4.00, (qp_temp_ref_orig / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
#else
      Double qp_temp_ref = refQP + bitdepth_luma_qp_scale - SHIFT_QP;
      dLambda *= Clip3( 2.00, 4.00, (qp_temp_ref / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
#endif
  }

  // if hadamard is used in ME process
  if ( !m_pcCfg->getUseHADME() && slice->getSliceType( ) != I_SLICE )
  {
    dLambda *= 0.95;
  }

#if X0038_LAMBDA_FROM_QP_CAPABILITY
  Double lambdaModifier;
  if( eSliceType != I_SLICE || intraLambdaModifiers.empty())
  {
    lambdaModifier = m_pcCfg->getLambdaModifier( temporalId );
  }
  else
  {
    lambdaModifier = intraLambdaModifiers[ (temporalId < intraLambdaModifiers.size()) ? temporalId : (intraLambdaModifiers.size()-1) ];
  }
  dLambda *= lambdaModifier;
#endif

  iQP = max( -slice->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), min( MAX_QP, (Int) floor( dQP + 0.5 ) ) );
  
  // NOTE: the lambda modifiers that are sometimes applied later might be best always applied in here.
  return dLambda;
}
#endif

Void TEncSlice::resetQP( TComPic* pic, Int sliceQP, Double lambda )
{
  TComSlice* slice = pic->getSlice(0);

  // store lambda
  slice->setSliceQp( sliceQP );
#if ADAPTIVE_QP_SELECTION
  slice->setSliceQpBase ( sliceQP );
#endif
  setUpLambda(slice, lambda, sliceQP);
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

//! set adaptive search range based on poc difference
Void TEncSlice::setSearchRange( TComSlice* pcSlice )
{
  Int iCurrPOC = pcSlice->getPOC();
  Int iRefPOC;
  Int iGOPSize = m_pcCfg->getGOPSize();
  Int iOffset = (iGOPSize >> 1);
  Int iMaxSR = m_pcCfg->getSearchRange();
  Int iNumPredDir = pcSlice->isInterP() ? 1 : 2;

  for (Int iDir = 0; iDir < iNumPredDir; iDir++)
  {
    RefPicList  e = ( iDir ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );
    for (Int iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(e); iRefIdx++)
    {
      iRefPOC = pcSlice->getRefPic(e, iRefIdx)->getPOC();
      Int newSearchRange = Clip3(m_pcCfg->getMinSearchWindow(), iMaxSR, (iMaxSR*ADAPT_SR_SCALE*abs(iCurrPOC - iRefPOC)+iOffset)/iGOPSize);
      m_pcPredSearch->setAdaptiveSearchRange(iDir, iRefIdx, newSearchRange);
    }
  }
}

/**
 Multi-loop slice encoding for different slice QP

 \param pcPic    picture class
 */
Void TEncSlice::precompressSlice( TComPic* pcPic )
{
  // if deltaQP RD is not used, simply return
  if ( m_pcCfg->getDeltaQpRD() == 0 )
  {
    return;
  }

  if ( m_pcCfg->getUseRateCtrl() )
  {
    printf( "\nMultiple QP optimization is not allowed when rate control is enabled." );
    assert(0);
    return;
  }

  TComSlice* pcSlice        = pcPic->getSlice(getSliceIdx());

  if (pcSlice->getDependentSliceSegmentFlag())
  {
    // if this is a dependent slice segment, then it was optimised
    // when analysing the entire slice.
    return;
  }

  if (pcSlice->getSliceMode()==FIXED_NUMBER_OF_BYTES)
  {
    // TODO: investigate use of average cost per CTU so that this Slice Mode can be used.
    printf( "\nUnable to optimise Slice-level QP if Slice Mode is set to FIXED_NUMBER_OF_BYTES\n" );
    assert(0);
    return;
  }

  Double     dPicRdCostBest = MAX_DOUBLE;
  UInt       uiQpIdxBest = 0;

  Double dFrameLambda;
#if FULL_NBIT
  Int    SHIFT_QP = 12 + 6 * (pcSlice->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA) - 8);
#else
  Int    SHIFT_QP = 12;
#endif

  // set frame lambda
  if (m_pcCfg->getGOPSize() > 1)
  {
    dFrameLambda = 0.68 * pow (2, (m_viRdPicQp[0]  - SHIFT_QP) / 3.0) * (pcSlice->isInterB()? 2 : 1);
  }
  else
  {
    dFrameLambda = 0.68 * pow (2, (m_viRdPicQp[0] - SHIFT_QP) / 3.0);
  }
  m_pcRdCost      ->setFrameLambda(dFrameLambda);

  // for each QP candidate
  for ( UInt uiQpIdx = 0; uiQpIdx < 2 * m_pcCfg->getDeltaQpRD() + 1; uiQpIdx++ )
  {
    pcSlice       ->setSliceQp             ( m_viRdPicQp    [uiQpIdx] );
#if ADAPTIVE_QP_SELECTION
    pcSlice       ->setSliceQpBase         ( m_viRdPicQp    [uiQpIdx] );
#endif
    setUpLambda(pcSlice, m_vdRdPicLambda[uiQpIdx], m_viRdPicQp    [uiQpIdx]);

    // try compress
    compressSlice   ( pcPic, true, m_pcCfg->getFastDeltaQp());

    UInt64 uiPicDist        = m_uiPicDist; // Distortion, as calculated by compressSlice.
    // NOTE: This distortion is the chroma-weighted SSE distortion for the slice.
    //       Previously a standard SSE distortion was calculated (for the entire frame).
    //       Which is correct?

    // TODO: Update loop filter, SAO and distortion calculation to work on one slice only.
    // m_pcGOPEncoder->preLoopFilterPicAll( pcPic, uiPicDist );

    // compute RD cost and choose the best
    Double dPicRdCost = m_pcRdCost->calcRdCost( (Double)m_uiPicTotalBits, (Double)uiPicDist, DF_SSE_FRAME);

    if ( dPicRdCost < dPicRdCostBest )
    {
      uiQpIdxBest    = uiQpIdx;
      dPicRdCostBest = dPicRdCost;
    }
  }

  // set best values
  pcSlice       ->setSliceQp             ( m_viRdPicQp    [uiQpIdxBest] );
#if ADAPTIVE_QP_SELECTION
  pcSlice       ->setSliceQpBase         ( m_viRdPicQp    [uiQpIdxBest] );
#endif
  setUpLambda(pcSlice, m_vdRdPicLambda[uiQpIdxBest], m_viRdPicQp    [uiQpIdxBest]);
}

Void TEncSlice::calCostSliceI(TComPic* pcPic) // TODO: this only analyses the first slice segment. What about the others?
{
  Double            iSumHadSlice      = 0;
  TComSlice * const pcSlice           = pcPic->getSlice(getSliceIdx());
  const TComSPS    &sps               = *(pcSlice->getSPS());
  const Int         shift             = sps.getBitDepth(CHANNEL_TYPE_LUMA)-8;
  const Int         offset            = (shift>0)?(1<<(shift-1)):0;

  pcSlice->setSliceSegmentBits(0);

  UInt startCtuTsAddr, boundingCtuTsAddr;
  xDetermineStartAndBoundingCtuTsAddr ( startCtuTsAddr, boundingCtuTsAddr, pcPic );

  for( UInt ctuTsAddr = startCtuTsAddr, ctuRsAddr = pcPic->getPicSym()->getCtuTsToRsAddrMap( startCtuTsAddr);
       ctuTsAddr < boundingCtuTsAddr;
       ctuRsAddr = pcPic->getPicSym()->getCtuTsToRsAddrMap(++ctuTsAddr) )
  {
    // initialize CU encoder
    TComDataCU* pCtu = pcPic->getCtu( ctuRsAddr );
    pCtu->initCtu( pcPic, ctuRsAddr );

    Int height  = min( sps.getMaxCUHeight(),sps.getPicHeightInLumaSamples() - ctuRsAddr / pcPic->getFrameWidthInCtus() * sps.getMaxCUHeight() );
    Int width   = min( sps.getMaxCUWidth(), sps.getPicWidthInLumaSamples()  - ctuRsAddr % pcPic->getFrameWidthInCtus() * sps.getMaxCUWidth() );

    Int iSumHad = m_pcCuEncoder->updateCtuDataISlice(pCtu, width, height);

    (m_pcRateCtrl->getRCPic()->getLCU(ctuRsAddr)).m_costIntra=(iSumHad+offset)>>shift;
    iSumHadSlice += (m_pcRateCtrl->getRCPic()->getLCU(ctuRsAddr)).m_costIntra;

  }
  m_pcRateCtrl->getRCPic()->setTotalIntraCost(iSumHadSlice);
}

Void TEncSlice::xSetPredFromPPS(Pel lastPalette[MAX_NUM_COMPONENT][MAX_PALETTE_PRED_SIZE], UChar lastPaletteSize[MAX_NUM_COMPONENT], TComSlice *pcSlice)
{
  TComPPS *pcPPS = m_pcGOPEncoder->getPPS(pcSlice->getPPSId());
  TComSPS *pcSPS = m_pcGOPEncoder->getSPS(pcPPS->getSPSId());
  pcSlice->setSPS(pcSPS);
  pcSlice->setPPS(pcPPS);
  UInt num = std::min(pcPPS->getPpsScreenExtension().getNumPalettePred(), pcSPS->getSpsScreenExtension().getPaletteMaxPredSize());
  if( !num )
  {
    memset(lastPaletteSize, 0, MAX_NUM_COMPONENT*sizeof(UChar));
    return;
  }

  for(int i=0; i<3; i++)
  {
    lastPaletteSize[i] = num;
    memcpy(lastPalette[i], pcPPS->getPpsScreenExtension().getPalettePred(i), num*sizeof(Pel));
  }
  for(Int ch=0; ch<MAX_NUM_CHANNEL_TYPE; ch++)
  {
    pcPPS->getPpsScreenExtension().setPalettePredictorBitDepth( ChannelType( ch ), pcSPS->getBitDepth( ChannelType( ch ) ) );
  }
  pcPPS->getPpsScreenExtension().setMonochromePaletteFlag( pcSPS->getChromaFormatIdc() == CHROMA_400 ? true : false );
}

Void TEncSlice::xSetPredFromSPS(Pel lastPalette[MAX_NUM_COMPONENT][MAX_PALETTE_PRED_SIZE], UChar lastPaletteSize[MAX_NUM_COMPONENT], TComSlice *pcSlice)
{
  TComPPS *pcPPS = m_pcGOPEncoder->getPPS(pcSlice->getPPSId());
  TComSPS *pcSPS = m_pcGOPEncoder->getSPS(pcPPS->getSPSId());
  UInt num = std::min(pcSPS->getSpsScreenExtension().getNumPalettePred(), pcSPS->getSpsScreenExtension().getPaletteMaxPredSize());
  if( !num )
  {
    memset(lastPaletteSize, 0, MAX_NUM_COMPONENT*sizeof(UChar));
    return;
  }
  for(int i=0; i<3; i++)
  {
    lastPaletteSize[i] = num;
    memcpy(lastPalette[i], pcSPS->getSpsScreenExtension().getPalettePred(i), num*sizeof(Pel));
  }
  pcSlice->setSPS(pcSPS);
}

Void TEncSlice::xSetPredDefault(Pel lastPalette[MAX_NUM_COMPONENT][MAX_PALETTE_PRED_SIZE], UChar lastPaletteSize[MAX_NUM_COMPONENT], TComSlice *pcSlice)
{
  const TComSPS *pcSPS = pcSlice->getSPS();
  pcSlice->setSPS(pcSPS);
  for(int i=0; i<3; i++)
  {
    lastPaletteSize[i] = 0;
    memset(lastPalette[i],0 , pcSPS->getSpsScreenExtension().getPaletteMaxSize()*sizeof(Pel));
  }
}

/** \param pcPic   picture class
 */
Void TEncSlice::compressSlice( TComPic* pcPic, const Bool bCompressEntireSlice, const Bool bFastDeltaQP )
{
  // if bCompressEntireSlice is true, then the entire slice (not slice segment) is compressed,
  //   effectively disabling the slice-segment-mode.

  UInt   startCtuTsAddr;
  UInt   boundingCtuTsAddr;
  TComSlice* const pcSlice            = pcPic->getSlice(getSliceIdx());
  pcSlice->setSliceSegmentBits(0);
  xDetermineStartAndBoundingCtuTsAddr ( startCtuTsAddr, boundingCtuTsAddr, pcPic );
  if (bCompressEntireSlice)
  {
    boundingCtuTsAddr = pcSlice->getSliceCurEndCtuTsAddr();
    pcSlice->setSliceSegmentCurEndCtuTsAddr(boundingCtuTsAddr);
  }

  // initialize cost values - these are used by precompressSlice (they should be parameters).
  m_uiPicTotalBits  = 0;
  m_dPicRdCost      = 0; // NOTE: This is a write-only variable!
  m_uiPicDist       = 0;

  m_pcEntropyCoder->setEntropyCoder   ( m_pppcRDSbacCoder[0][CI_CURR_BEST] );
  m_pcEntropyCoder->resetEntropy      ( pcSlice );

  TEncBinCABAC* pRDSbacCoder = (TEncBinCABAC *) m_pppcRDSbacCoder[0][CI_CURR_BEST]->getEncBinIf();
  pRDSbacCoder->setBinCountingEnableFlag( false );
  pRDSbacCoder->setBinsCoded( 0 );

  TComBitCounter  tempBitCounter;
  const UInt      frameWidthInCtus = pcPic->getPicSym()->getFrameWidthInCtus();
  
  m_pcCuEncoder->setFastDeltaQp(bFastDeltaQP);

  //------------------------------------------------------------------------------
  //  Weighted Prediction parameters estimation.
  //------------------------------------------------------------------------------
  // calculate AC/DC values for current picture
  if( pcSlice->getPPS()->getUseWP() || pcSlice->getPPS()->getWPBiPred() )
  {
    xCalcACDCParamSlice(pcSlice);
  }

  const Bool bWp_explicit = (pcSlice->getSliceType()==P_SLICE && pcSlice->getPPS()->getUseWP()) || (pcSlice->getSliceType()==B_SLICE && pcSlice->getPPS()->getWPBiPred());

  if ( bWp_explicit )
  {
    //------------------------------------------------------------------------------
    //  Weighted Prediction implemented at Slice level. SliceMode=2 is not supported yet.
    //------------------------------------------------------------------------------
    if ( pcSlice->getSliceMode()==FIXED_NUMBER_OF_BYTES || pcSlice->getSliceSegmentMode()==FIXED_NUMBER_OF_BYTES )
    {
      printf("Weighted Prediction is not supported with slice mode determined by max number of bins.\n"); exit(0);
    }

    xEstimateWPParamSlice( pcSlice, m_pcCfg->getWeightedPredictionMethod() );
    pcSlice->initWpScaling(pcSlice->getSPS());

    // check WP on/off
    xCheckWPEnable( pcSlice );
  }

#if ADAPTIVE_QP_SELECTION
  if( m_pcCfg->getUseAdaptQpSelect() && !(pcSlice->getDependentSliceSegmentFlag()))
  {
    // TODO: this won't work with dependent slices: they do not have their own QP. Check fix to mask clause execution with && !(pcSlice->getDependentSliceSegmentFlag())
    m_pcTrQuant->clearSliceARLCnt(); // TODO: this looks wrong for multiple slices - the results of all but the last slice will be cleared before they are used (all slices compressed, and then all slices encoded)
    if(pcSlice->getSliceType()!=I_SLICE)
    {
      Int qpBase = pcSlice->getSliceQpBase();
      pcSlice->setSliceQp(qpBase + m_pcTrQuant->getQpDelta(qpBase));
    }
  }
#endif

  UChar lastPaletteUsedSize[MAX_NUM_COMPONENT] = { PALETTE_SIZE_INVALID, PALETTE_SIZE_INVALID, PALETTE_SIZE_INVALID };
  UChar lastPaletteSize[MAX_NUM_COMPONENT] = { 0, 0, 0 };
  Pel lastPalette[MAX_NUM_COMPONENT][MAX_PALETTE_PRED_SIZE];
  for(UChar comp=0; comp < MAX_NUM_COMPONENT; comp++)
  {
    memset(lastPalette[comp], 0, sizeof(Pel) * pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize());
  }
  if ( m_pcCfg->getPalettePredInPPSEnabled() )
  {
    xSetPredFromPPS(lastPalette, lastPaletteSize, pcSlice);
  }
  else if (m_pcCfg->getPalettePredInSPSEnabled())
  {
    xSetPredFromSPS(lastPalette, lastPaletteSize, pcSlice);
  }
  else
  {
    xSetPredDefault(lastPalette, lastPaletteSize, pcSlice);
  }

  // Adjust initial state if this is the start of a dependent slice.
  {
    const UInt      ctuRsAddr               = pcPic->getPicSym()->getCtuTsToRsAddrMap( startCtuTsAddr);
    const UInt      currentTileIdx          = pcPic->getPicSym()->getTileIdxMap(ctuRsAddr);
    const TComTile *pCurrentTile            = pcPic->getPicSym()->getTComTile(currentTileIdx);
    const UInt      firstCtuRsAddrOfTile    = pCurrentTile->getFirstCtuRsAddr();
    if( pcSlice->getDependentSliceSegmentFlag() && ctuRsAddr != firstCtuRsAddrOfTile )
    {
      // This will only occur if dependent slice-segments (m_entropyCodingSyncContextState=true) are being used.
      if( pCurrentTile->getTileWidthInCtus() >= 2 || !m_pcCfg->getEntropyCodingSyncEnabledFlag() )
      {
        m_pppcRDSbacCoder[0][CI_CURR_BEST]->loadContexts( &m_lastSliceSegmentEndContextState );
        for ( UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++ )
        {
          lastPaletteSize[comp] = m_lastSliceSegmentEndPaletteState.lastPaletteSize[comp];
          for ( UInt idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++ )
          {
            lastPalette[comp][idx] = m_lastSliceSegmentEndPaletteState.lastPalette[comp][idx];
          }
        }
      }
    }
  }

  TComPPS *pcPPS = m_pcGOPEncoder->getPPS(pcSlice->getPPSId());
  TComSPS *pcSPS = m_pcGOPEncoder->getSPS(pcPPS->getPPSId());

  Bool refresh = false;
  if( !pcSlice->getSliceIdx() )
  {
    if( pcSlice->isOnlyCurrentPictureAsReference() ) m_numIDRs++;
    m_numFrames++;
    refresh = m_numIDRs > SCM_T0048_PALETTE_PRED_IN_PPS_REFRESH ||
              (m_numIDRs && m_numFrames > SCM_T0048_PALETTE_PRED_IN_PPS_REFRESH) ||
              m_numFrames > 5*m_pcCfg->getFrameRate();
  }
  if( pcSlice->getSPS()->getSpsScreenExtension().getUsePaletteMode() && (m_pcCfg->getPalettePredInPPSEnabled()||m_pcCfg->getPalettePredInSPSEnabled()) && refresh && !pcSlice->getSliceIdx() && !pcPic->getPOC() )
  {
    // for every CTU in image
    Int  srcCtu = -1;
    UInt numCtus = pcPic->getPicSym()->getNumberOfCtusInFrame(), numPreds = 0;

    m_pppcRDSbacCoder[0][CI_CURR_BEST]->resetEntropy( pcSlice );

    // Back-up before removing status
    SliceConstraint constraint = pcSlice->getSliceMode();
    UInt startCtuTsAddrSliceSegment    = pcSlice->getSliceSegmentCurStartCtuTsAddr();
    UInt boundingCtuTsAddrSliceSegment = pcSlice->getSliceSegmentCurEndCtuTsAddr();
    pcSlice->setSliceCurEndCtuTsAddr( numCtus - 1 );
    pcSlice->setSliceSegmentCurEndCtuTsAddr( numCtus - 1 );

    // Dependent slice
    Bool depend = pcSlice->getDependentSliceSegmentFlag();
    pcSlice->setDependentSliceSegmentFlag(false);
    
    // Analysis parameters
    UInt step = 1, stride = pcPic->getPicSym()->getFrameWidthInCtus(), count = 0, offset = 0;
    if( constraint != NO_SLICES )
    {
      step = pcPic->getPicSym()->getFrameHeightInCtus() < 4
           ? numCtus/8 : 2*(numCtus-2*stride-2) / (3*(pcPic->getPicSym()->getFrameHeightInCtus()-2));
      step = std::max(step, 1U);
      offset = stride+1;
    }
    TComSlice dummySlice;
    dummySlice.initSlice();
    dummySlice.setSliceIdx(1);
    //if (step > 1) pcPic->getPicYuvOrg()->copyToPic( pcPic->getPicYuvRec() );
    if( !pcSlice->getPOC() )
    {
      memset(lastPaletteUsedSize, PALETTE_SIZE_INVALID, sizeof(lastPaletteUsedSize));
      memset(lastPaletteSize, 0, sizeof(lastPaletteSize));
    }
    for( UInt ctuRsAddr = 0; ctuRsAddr < offset; ctuRsAddr++)
      pcPic->getCtu( ctuRsAddr )->initCtu( pcPic, ctuRsAddr );

    numPreds = 0;
    for( UInt ctuRsAddr = offset; ctuRsAddr < numCtus-offset; ctuRsAddr++)
    {
      // initialize CTU encoder
      TComDataCU* pCtu = pcPic->getCtu( ctuRsAddr );
      pCtu->initCtu( pcPic, ctuRsAddr );

      if( ctuRsAddr % step )
        continue;
      count++;
      if(step>1)
      {
        pcSlice->setSliceCurStartCtuTsAddr( ctuRsAddr );
        pcSlice->setSliceSegmentCurStartCtuTsAddr( ctuRsAddr );
        for( int i=0; i<4; i++)
        {
          TComDataCU *pcNeighbour;
          switch(i)
          {
          case 0: pcNeighbour = pCtu->getCtuLeft(); break;
          case 1: pcNeighbour = pCtu->getCtuAbove(); break;
          case 2: pcNeighbour = pCtu->getCtuAboveLeft(); break;
          case 3: pcNeighbour = pCtu->getCtuAboveRight(); break;
          }
          if( pcNeighbour )
            pcNeighbour->setSlice(&dummySlice);
        }
      }

      // Set last predictor
      for (UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++)
      {
        pCtu->setLastPaletteInLcuSizeFinal(comp, lastPaletteSize[comp]);
        memcpy(pCtu->getLastPaletteInLcuFinal(comp), lastPalette[comp], pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize()*sizeof(Pel));
      }

      // set go-on entropy coder (used for all trial encodings - the cu encoder and encoder search also have a copy of the same pointer)
      m_pcEntropyCoder->setEntropyCoder ( m_pcRDGoOnSbacCoder );
      m_pcEntropyCoder->setBitstream( &tempBitCounter );
      tempBitCounter.resetBits();
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[0][CI_CURR_BEST] ); // this copy is not strictly necessary here, but indicates that the GoOnSbacCoder
                                                                       // is reset to a known state before every decision process.

      ((TEncBinCABAC*)m_pcRDGoOnSbacCoder->getEncBinIf())->setBinCountingEnableFlag(false);

      // run CTU trial encoder
      m_pcCuEncoder->compressCtu( pCtu, lastPaletteSize, lastPaletteUsedSize, lastPalette );

      // All CTU decisions have now been made. Restore entropy coder to an initial stage, ready to make a true encode,
      // which will result in the state of the contexts being correct. It will also count up the number of bits coded,
      // which is used if there is a limit of the number of bytes per slice-segment.
      m_pcEntropyCoder->setEntropyCoder ( m_pppcRDSbacCoder[0][CI_CURR_BEST] );
      m_pcEntropyCoder->setBitstream( &tempBitCounter );
      m_pppcRDSbacCoder[0][CI_CURR_BEST]->resetBits();
      pRDSbacCoder->setBinsCoded( 0 );

      // encode CTU and calculate the true bit counters.
      m_pcCuEncoder->encodeCtu( pCtu );

      if (pCtu->getLastPaletteInLcuSizeFinal(COMPONENT_Y))
      {
        for (UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++)
        {
          lastPaletteSize[comp] = pCtu->getLastPaletteInLcuSizeFinal(comp);
          for (Int idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++)
            lastPalette[comp][idx] = pCtu->getLastPaletteInLcuFinal(comp, idx);
        }
      }

      if( pCtu->getLastPaletteInLcuSizeFinal(0) && numPreds <= pCtu->getLastPaletteInLcuSizeFinal(0) )
      {
        srcCtu = ctuRsAddr;
        if( pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize() <= numPreds && !constraint )
        {
          break;
        }
        else
        {
          numPreds = pCtu->getLastPaletteInLcuSizeFinal(0);
        }
      }
    }

    if( srcCtu == -1 || numPreds < 4 )
    {
      // refresh failed, wait a bit longer before retrying
      if( srcCtu != -1 )
      {
        //printf("Too few entries after %u/%u frames: %u vs %u\n", m_numIDRs, m_numFrames, numPreds, pcPPS->getNumPalettePred() );
      }
      m_numIDRs   = m_numIDRs>>1;
      m_numFrames = m_numFrames>>1;
    }
    else if( srcCtu != -1 )
    {
      if( pcSlice->getPOC() )
      {
        UInt ppsid = pcPPS->getPPSId()+1;
        pcPPS = m_pcGOPEncoder->copyToNewPPS(ppsid, pcPPS);
        pcSlice->setPPS(pcPPS);
      }
      if( !pcSlice->getSliceIdx() && !pcSPS->getSpsScreenExtension().getNumPalettePred() && !pcPic->getPOC()&&m_pcCfg->getPalettePredInSPSEnabled() )
      {
        numPreds = std::min(lastPaletteSize[0], (UChar)pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize());
        pcSPS->getSpsScreenExtension().setNumPalettePred(numPreds);
        for ( int i=0; i<3; i++ )
        {
          memcpy( pcSPS->getSpsScreenExtension().getPalettePred( i ), lastPalette[i], sizeof( Pel )*numPreds );
        }
      }
      if ( m_pcCfg->getPalettePredInPPSEnabled() )
      {
        numPreds = std::min(lastPaletteSize[0], (UChar)pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize());
        pcPPS->getPpsScreenExtension().setNumPalettePred(numPreds);
        pcPPS->getPpsScreenExtension().setMonochromePaletteFlag( pcSPS->getChromaFormatIdc() == CHROMA_400 ? true : false );
        //printf("PPS %u: %u palette entries from CTU %u/%u (%u analysed)\n", pcPPS->getPPSId(), numPreds, srcCtu, numCtus, count );
        for ( int i=0; i<3; i++ )
        {
          memcpy( pcPPS->getPpsScreenExtension().getPalettePred( i ), lastPalette[i], sizeof( Pel )*numPreds );
        }
      }
      m_numIDRs   = 0;
      m_numFrames = 0;
    }

    // restore predictor in any case
    if ( m_pcCfg->getPalettePredInPPSEnabled() )
    {
      xSetPredFromPPS(lastPalette, lastPaletteSize, pcSlice);
    }
    else if ( m_pcCfg->getPalettePredInSPSEnabled() )
    {
      xSetPredFromSPS(lastPalette, lastPaletteSize, pcSlice);
    }
    else
    {
      xSetPredDefault(lastPalette, lastPaletteSize, pcSlice);
    }

    // reset entropy
    TEncBinCABAC *pcCABAC = (TEncBinCABAC *) m_pppcRDSbacCoder[0][CI_CURR_BEST]->getEncBinIf();
    pcCABAC->setBinsCoded( 0 );
    pcCABAC->setBinCountingEnableFlag( false );
    tempBitCounter.resetBits();
    m_pppcRDSbacCoder[0][CI_CURR_BEST]->resetEntropy( pcSlice );
    m_pcEntropyCoder->resetEntropy( pcSlice );

    // reset pic level stuff
    if( m_pcCfg->getUseHashBasedIntraBCSearch() )
    {
      m_pcPredSearch->xClearIntraBCHashTable();
    }
    pcSlice->setDependentSliceSegmentFlag(depend);
    pcSlice->setSliceCurStartCtuTsAddr( startCtuTsAddr );
    pcSlice->setSliceCurEndCtuTsAddr( boundingCtuTsAddr );
    pcSlice->setSliceSegmentCurStartCtuTsAddr(startCtuTsAddrSliceSegment);
    pcSlice->setSliceSegmentCurEndCtuTsAddr( boundingCtuTsAddrSliceSegment );

    m_uiPicTotalBits = 0;
    m_dPicRdCost     = 0;
    m_uiPicDist      = 0;
  }
  if ( pcSlice->getSPS()->getSpsScreenExtension().getUsePaletteMode() && m_pcCfg->getPalettePredInPPSEnabled() && (pcPic->getPOC()||pcSlice->getSliceIdx()) && refresh )
  {
    UInt numPredsPOC=0;
    Int srcCtu=-1;
    numPredsPOC=m_pcGOPEncoder->getNumPalettePred();
    if (numPredsPOC) srcCtu=1;
    if(m_pcCfg->getPalettePredInPPSEnabled())
    {
      for ( UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++ )
      {
        lastPaletteSize[comp]=m_pcGOPEncoder->getNumPalettePred();
        memcpy( lastPalette[comp],m_pcGOPEncoder->getPalettePred(comp), sizeof( Pel )*numPredsPOC );
      }
    }
    if( srcCtu == -1 || numPredsPOC < 4 )
    {
      // refresh failed, wait a bit longer before retrying
      if( srcCtu != -1 )
      {
        //printf("Too few entries after %u/%u frames: %u vs %u\n", m_numIDRs, m_numFrames, numPreds, pcPPS->getNumPalettePred() );
      }
      m_numIDRs   = m_numIDRs>>1;
      m_numFrames = m_numFrames>>1;
    }
    else if( srcCtu != -1 )
    {
      if( pcSlice->getPOC() )
      {
        UInt ppsid = pcPPS->getPPSId()+1;
        pcPPS = m_pcGOPEncoder->copyToNewPPS(ppsid, pcPPS);
        pcSlice->setPPS(pcPPS);
      }
      numPredsPOC = std::min(lastPaletteSize[0], (UChar)pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize());
      pcPPS->getPpsScreenExtension().setNumPalettePred(numPredsPOC);
      pcPPS->getPpsScreenExtension().setMonochromePaletteFlag( pcSPS->getChromaFormatIdc() == CHROMA_400 ? true : false );
      //printf("PPS %u: %u palette entries from CTU %u/%u (%u analysed)\n", pcPPS->getPPSId(), numPreds, srcCtu, numCtus, count );
      for ( int i=0; i<3; i++ )
      {
        memcpy( pcPPS->getPpsScreenExtension().getPalettePred( i ), lastPalette[i], sizeof( Pel )*numPredsPOC );
      }
      m_numIDRs   = 0;
      m_numFrames = 0;
    }
    xSetPredFromPPS(lastPalette,lastPaletteSize,pcSlice);
  }

  // for every CTU in the slice segment (may terminate sooner if there is a byte limit on the slice-segment)

  for( UInt ctuTsAddr = startCtuTsAddr; ctuTsAddr < boundingCtuTsAddr; ++ctuTsAddr )
  {
    const UInt ctuRsAddr = pcPic->getPicSym()->getCtuTsToRsAddrMap(ctuTsAddr);
    // initialize CTU encoder
    TComDataCU* pCtu = pcPic->getCtu( ctuRsAddr );
    pCtu->initCtu( pcPic, ctuRsAddr );

    // update CABAC state
    const UInt firstCtuRsAddrOfTile = pcPic->getPicSym()->getTComTile(pcPic->getPicSym()->getTileIdxMap(ctuRsAddr))->getFirstCtuRsAddr();
    const UInt tileXPosInCtus = firstCtuRsAddrOfTile % frameWidthInCtus;
    const UInt ctuXPosInCtus  = ctuRsAddr % frameWidthInCtus;
    
    if (ctuRsAddr == firstCtuRsAddrOfTile)
    {
      m_pppcRDSbacCoder[0][CI_CURR_BEST]->resetEntropy(pcSlice);
    }
    else if ( ctuXPosInCtus == tileXPosInCtus && m_pcCfg->getEntropyCodingSyncEnabledFlag())
    {
      // reset and then update contexts to the state at the end of the top-right CTU (if within current slice and tile).
      m_pppcRDSbacCoder[0][CI_CURR_BEST]->resetEntropy(pcSlice);
      // Sync if the Top-Right is available.
      TComDataCU *pCtuUp = pCtu->getCtuAbove();
      if ( pCtuUp && ((ctuRsAddr%frameWidthInCtus+1) < frameWidthInCtus)  )
      {
        TComDataCU *pCtuTR = pcPic->getCtu( ctuRsAddr - frameWidthInCtus + 1 );
        if ( pCtu->CUIsFromSameSliceAndTile(pCtuTR) )
        {
          // Top-Right is available, we use it.
          m_pppcRDSbacCoder[0][CI_CURR_BEST]->loadContexts( &m_entropyCodingSyncContextState );
          for ( UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++ )
          {
            lastPaletteSize[comp] = m_entropyCodingSyncPaletteState.lastPaletteSize[comp];
            for ( UInt idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++ )
            {
              lastPalette[comp][idx] = m_entropyCodingSyncPaletteState.lastPalette[comp][idx];
            }
          }
        }
      }
    }

    if( ctuRsAddr == firstCtuRsAddrOfTile && ctuRsAddr != 0)
    {
      if( m_pcCfg->getUseHashBasedIntraBCSearch() )
      {
        m_pcPredSearch->xClearIntraBCHashTable();
      }
    }

    for (UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++)
    {
      pCtu->setLastPaletteInLcuSizeFinal(comp, lastPaletteSize[comp]);
      for ( UInt idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++ )
      {
        pCtu->setLastPaletteInLcuFinal(comp, lastPalette[comp][idx], idx);
      }
    }

    // set go-on entropy coder (used for all trial encodings - the cu encoder and encoder search also have a copy of the same pointer)
    m_pcEntropyCoder->setEntropyCoder ( m_pcRDGoOnSbacCoder );
    m_pcEntropyCoder->setBitstream( &tempBitCounter );
    tempBitCounter.resetBits();
    m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[0][CI_CURR_BEST] ); // this copy is not strictly necessary here, but indicates that the GoOnSbacCoder
                                                                     // is reset to a known state before every decision process.

    ((TEncBinCABAC*)m_pcRDGoOnSbacCoder->getEncBinIf())->setBinCountingEnableFlag(true);

    Double oldLambda = m_pcRdCost->getLambda();
    if ( m_pcCfg->getUseRateCtrl() )
    {
      Int estQP        = pcSlice->getSliceQp();
      Double estLambda = -1.0;
      Double bpp       = -1.0;

      if ( ( pcPic->getSlice( 0 )->getSliceType() == I_SLICE && m_pcCfg->getForceIntraQP() ) || !m_pcCfg->getLCULevelRC() )
      {
        estQP = pcSlice->getSliceQp();
      }
      else
      {
        bpp = m_pcRateCtrl->getRCPic()->getLCUTargetBpp(pcSlice->getSliceType());
        if ( pcPic->getSlice( 0 )->getSliceType() == I_SLICE)
        {
          estLambda = m_pcRateCtrl->getRCPic()->getLCUEstLambdaAndQP(bpp, pcSlice->getSliceQp(), &estQP);
        }
        else
        {
          estLambda = m_pcRateCtrl->getRCPic()->getLCUEstLambda( bpp );
          estQP     = m_pcRateCtrl->getRCPic()->getLCUEstQP    ( estLambda, pcSlice->getSliceQp() );
        }

        estQP     = Clip3( -pcSlice->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, estQP );

        m_pcRdCost->setLambda(estLambda, pcSlice->getSPS()->getBitDepths());

#if RDOQ_CHROMA_LAMBDA
        // set lambda for RDOQ
        const Double chromaLambda = estLambda / m_pcRdCost->getChromaWeight();
        const Double lambdaArray[MAX_NUM_COMPONENT] = { estLambda, chromaLambda, chromaLambda };
        m_pcTrQuant->setLambdas( lambdaArray );
#else
        m_pcTrQuant->setLambda( estLambda );
#endif
      }

      m_pcRateCtrl->setRCQP( estQP );
#if ADAPTIVE_QP_SELECTION
      pCtu->getSlice()->setSliceQpBase( estQP );
#endif
      if( pcSlice->getPPS()->getPpsScreenExtension().getUseColourTrans() && m_pcCfg->getRGBFormatFlag() && pcPic->getPicYuvCSC() )
      {
        pcPic->releaseCSCBuffer();
      }
    }

    // run CTU trial encoder
    m_pcCuEncoder->compressCtu( pCtu, lastPaletteSize, lastPaletteUsedSize, lastPalette );

    // All CTU decisions have now been made. Restore entropy coder to an initial stage, ready to make a true encode,
    // which will result in the state of the contexts being correct. It will also count up the number of bits coded,
    // which is used if there is a limit of the number of bytes per slice-segment.

    m_pcEntropyCoder->setEntropyCoder ( m_pppcRDSbacCoder[0][CI_CURR_BEST] );
    m_pcEntropyCoder->setBitstream( &tempBitCounter );
    pRDSbacCoder->setBinCountingEnableFlag( true );
    m_pppcRDSbacCoder[0][CI_CURR_BEST]->resetBits();
    pRDSbacCoder->setBinsCoded( 0 );

    // encode CTU and calculate the true bit counters.
    m_pcCuEncoder->encodeCtu( pCtu );


    pRDSbacCoder->setBinCountingEnableFlag( false );

    const Int numberOfWrittenBits = m_pcEntropyCoder->getNumberOfWrittenBits();

    // Calculate if this CTU puts us over slice bit size.
    // cannot terminate if current slice/slice-segment would be 0 Ctu in size,
    const UInt validEndOfSliceCtuTsAddr = ctuTsAddr + (ctuTsAddr == startCtuTsAddr ? 1 : 0);
    // Set slice end parameter
    if(pcSlice->getSliceMode()==FIXED_NUMBER_OF_BYTES && pcSlice->getSliceBits()+numberOfWrittenBits > (pcSlice->getSliceArgument()<<3))
    {
      pcSlice->setSliceSegmentCurEndCtuTsAddr(validEndOfSliceCtuTsAddr);
      pcSlice->setSliceCurEndCtuTsAddr(validEndOfSliceCtuTsAddr);
      boundingCtuTsAddr=validEndOfSliceCtuTsAddr;
    }
    else if((!bCompressEntireSlice) && pcSlice->getSliceSegmentMode()==FIXED_NUMBER_OF_BYTES && pcSlice->getSliceSegmentBits()+numberOfWrittenBits > (pcSlice->getSliceSegmentArgument()<<3))
    {
      pcSlice->setSliceSegmentCurEndCtuTsAddr(validEndOfSliceCtuTsAddr);
      boundingCtuTsAddr=validEndOfSliceCtuTsAddr;
    }

    if (boundingCtuTsAddr <= ctuTsAddr)
    {
      break;
    }

    pcSlice->setSliceBits( (UInt)(pcSlice->getSliceBits() + numberOfWrittenBits) );
    pcSlice->setSliceSegmentBits(pcSlice->getSliceSegmentBits()+numberOfWrittenBits);

    if (pCtu->getLastPaletteInLcuSizeFinal(COMPONENT_Y))
    {
      for (UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++)
      {
        lastPaletteSize[comp] = pCtu->getLastPaletteInLcuSizeFinal(comp);
        for (Int idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++)
        {
          lastPalette[comp][idx] = pCtu->getLastPaletteInLcuFinal(comp, idx);
        }
      }
    }

    // Store probabilities of second CTU in line into buffer - used only if wavefront-parallel-processing is enabled.
    if ( ctuXPosInCtus == tileXPosInCtus+1 && m_pcCfg->getEntropyCodingSyncEnabledFlag())
    {
      m_entropyCodingSyncContextState.loadContexts(m_pppcRDSbacCoder[0][CI_CURR_BEST]);
      for ( UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++ )
      {
        m_entropyCodingSyncPaletteState.lastPaletteSize[comp] = lastPaletteSize[comp];
        for ( UInt idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++ )
        {
          m_entropyCodingSyncPaletteState.lastPalette[comp][idx] = lastPalette[comp][idx];
        }
      }
    }


    if ( m_pcCfg->getUseRateCtrl() )
    {
      Int actualQP        = g_RCInvalidQPValue;
      Double actualLambda = m_pcRdCost->getLambda();
      Int actualBits      = pCtu->getTotalBits();
      Int numberOfEffectivePixels    = 0;
      for ( Int idx = 0; idx < pcPic->getNumPartitionsInCtu(); idx++ )
      {
        if ( pCtu->getPredictionMode( idx ) != NUMBER_OF_PREDICTION_MODES && ( !pCtu->isSkipped( idx ) ) )
        {
          numberOfEffectivePixels = numberOfEffectivePixels + 16;
          break;
        }
      }

      if ( numberOfEffectivePixels == 0 )
      {
        actualQP = g_RCInvalidQPValue;
      }
      else
      {
        actualQP = pCtu->getQP( 0 );
      }
      m_pcRdCost->setLambda(oldLambda, pcSlice->getSPS()->getBitDepths());
      m_pcRateCtrl->getRCPic()->updateAfterCTU( m_pcRateCtrl->getRCPic()->getLCUCoded(), actualBits, actualQP, actualLambda,
                                                pCtu->getSlice()->getSliceType() == I_SLICE ? 0 : m_pcCfg->getLCULevelRC() );
    }

    if( m_pcCfg->getUseHashBasedIntraBCSearch() )
    {
      m_pcPredSearch->xIntraBCHashTableUpdate(pCtu, false);
    }

    m_uiPicTotalBits += pCtu->getTotalBits();
    m_dPicRdCost     += pCtu->getTotalCost();
    m_uiPicDist      += pCtu->getTotalDistortion();
  }

  // store context state at the end of this slice-segment, in case the next slice is a dependent slice and continues using the CABAC contexts.
  if( pcSlice->getPPS()->getDependentSliceSegmentsEnabledFlag() )
  {
    m_lastSliceSegmentEndContextState.loadContexts( m_pppcRDSbacCoder[0][CI_CURR_BEST] );//ctx end of dep.slice
    for ( UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++ )
    {
      m_lastSliceSegmentEndPaletteState.lastPaletteSize[comp] = lastPaletteSize[comp];
      for ( UInt idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++ )
      {
        m_lastSliceSegmentEndPaletteState.lastPalette[comp][idx] = lastPalette[comp][idx];
      }
    }
  }

  if(m_pcCfg->getPalettePredInPPSEnabled() && pcSlice->getSPS()->getSpsScreenExtension().getUsePaletteMode())
  {
    m_pcGOPEncoder->setNumPalettePred( lastPaletteSize[COMPONENT_Y] );
    for ( UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++ )
    {
      memcpy( m_pcGOPEncoder->getPalettePred(comp),lastPalette[ comp ],  sizeof( Pel )*m_pcGOPEncoder->getNumPalettePred() );
    }
  }

  // stop use of temporary bit counter object.
  m_pppcRDSbacCoder[0][CI_CURR_BEST]->setBitstream(NULL);
  m_pcRDGoOnSbacCoder->setBitstream(NULL); // stop use of tempBitCounter.

  //clear the hash table used in Intra BC search
  if( m_pcCfg->getUseHashBasedIntraBCSearch() )
  {
    m_pcPredSearch->xClearIntraBCHashTable();
  }

  // TODO: optimise cabac_init during compress slice to improve multi-slice operation
  //if (pcSlice->getPPS()->getCabacInitPresentFlag() && !pcSlice->getPPS()->getDependentSliceSegmentsEnabledFlag())
  //{
  //  m_encCABACTableIdx = m_pcEntropyCoder->determineCabacInitIdx();
  //}
  //else
  //{
  //  m_encCABACTableIdx = pcSlice->getSliceType();
  //}
}

Void TEncSlice::encodeSlice   ( TComPic* pcPic, TComOutputBitstream* pcSubstreams, UInt &numBinsCoded )
{
  TComSlice *const pcSlice           = pcPic->getSlice(getSliceIdx());

  const UInt startCtuTsAddr          = pcSlice->getSliceSegmentCurStartCtuTsAddr();
  const UInt boundingCtuTsAddr       = pcSlice->getSliceSegmentCurEndCtuTsAddr();

  const UInt frameWidthInCtus        = pcPic->getPicSym()->getFrameWidthInCtus();
  const Bool depSliceSegmentsEnabled = pcSlice->getPPS()->getDependentSliceSegmentsEnabledFlag();
  const Bool wavefrontsEnabled       = pcSlice->getPPS()->getEntropyCodingSyncEnabledFlag();

  // initialise entropy coder for the slice
  m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
  m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder );
  m_pcEntropyCoder->resetEntropy    ( pcSlice );

  numBinsCoded = 0;
  m_pcBinCABAC->setBinCountingEnableFlag( true );
  m_pcBinCABAC->setBinsCoded(0);

#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceEnable;
#endif
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tPOC: " );
  DTRACE_CABAC_V( pcPic->getPOC() );
  DTRACE_CABAC_T( "\n" );
#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceDisable;
#endif

  UChar lastPaletteSize[3] = { 0, 0, 0 };
  Pel lastPalette[3][MAX_PALETTE_PRED_SIZE];
  if ( m_pcCfg->getPalettePredInPPSEnabled() )
  {
    xSetPredFromPPS(lastPalette, lastPaletteSize, pcSlice);
  }
  else if (m_pcCfg->getPalettePredInSPSEnabled())
  {
    xSetPredFromSPS(lastPalette, lastPaletteSize, pcSlice);
  }
  else
  {
    xSetPredDefault(lastPalette, lastPaletteSize, pcSlice);
  }

  if (depSliceSegmentsEnabled)
  {
    // modify initial contexts with previous slice segment if this is a dependent slice.
    const UInt ctuRsAddr        = pcPic->getPicSym()->getCtuTsToRsAddrMap( startCtuTsAddr );
    const UInt currentTileIdx=pcPic->getPicSym()->getTileIdxMap(ctuRsAddr);
    const TComTile *pCurrentTile=pcPic->getPicSym()->getTComTile(currentTileIdx);
    const UInt firstCtuRsAddrOfTile = pCurrentTile->getFirstCtuRsAddr();

    if( pcSlice->getDependentSliceSegmentFlag() && ctuRsAddr != firstCtuRsAddrOfTile )
    {
      if( pCurrentTile->getTileWidthInCtus() >= 2 || !wavefrontsEnabled )
      {
        m_pcSbacCoder->loadContexts(&m_lastSliceSegmentEndContextState);
        for ( UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++ )
        {
          lastPaletteSize[comp] = m_lastSliceSegmentEndPaletteState.lastPaletteSize[comp];
          for ( UInt idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++ )
          {
            lastPalette[comp][idx] = m_lastSliceSegmentEndPaletteState.lastPalette[comp][idx];
          }
        }
      }
    }
  }

  // for every CTU in the slice segment...

  for( UInt ctuTsAddr = startCtuTsAddr; ctuTsAddr < boundingCtuTsAddr; ++ctuTsAddr )
  {
    const UInt ctuRsAddr = pcPic->getPicSym()->getCtuTsToRsAddrMap(ctuTsAddr);
    const TComTile &currentTile = *(pcPic->getPicSym()->getTComTile(pcPic->getPicSym()->getTileIdxMap(ctuRsAddr)));
    const UInt firstCtuRsAddrOfTile = currentTile.getFirstCtuRsAddr();
    const UInt tileXPosInCtus       = firstCtuRsAddrOfTile % frameWidthInCtus;
    const UInt tileYPosInCtus       = firstCtuRsAddrOfTile / frameWidthInCtus;
    const UInt ctuXPosInCtus        = ctuRsAddr % frameWidthInCtus;
    const UInt ctuYPosInCtus        = ctuRsAddr / frameWidthInCtus;
    const UInt uiSubStrm=pcPic->getSubstreamForCtuAddr(ctuRsAddr, true, pcSlice);
    TComDataCU* pCtu = pcPic->getCtu( ctuRsAddr );

    m_pcEntropyCoder->setBitstream( &pcSubstreams[uiSubStrm] );

    // set up CABAC contexts' state for this CTU
    if (ctuRsAddr == firstCtuRsAddrOfTile)
    {
      if (ctuTsAddr != startCtuTsAddr) // if it is the first CTU, then the entropy coder has already been reset
      {
        m_pcEntropyCoder->resetEntropy(pcSlice);
      }
    }
    else if (ctuXPosInCtus == tileXPosInCtus && wavefrontsEnabled)
    {
      // Synchronize cabac probabilities with upper-right CTU if it's available and at the start of a line.
      if (ctuTsAddr != startCtuTsAddr) // if it is the first CTU, then the entropy coder has already been reset
      {
        m_pcEntropyCoder->resetEntropy(pcSlice);
      }
      TComDataCU *pCtuUp = pCtu->getCtuAbove();
      if ( pCtuUp && ((ctuRsAddr%frameWidthInCtus+1) < frameWidthInCtus)  )
    {
        TComDataCU *pCtuTR = pcPic->getCtu( ctuRsAddr - frameWidthInCtus + 1 );
        if ( pCtu->CUIsFromSameSliceAndTile(pCtuTR) )
        {
          // Top-right is available, so use it.
          m_pcSbacCoder->loadContexts( &m_entropyCodingSyncContextState );
          for ( UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++ )
          {
            lastPaletteSize[comp] = m_entropyCodingSyncPaletteState.lastPaletteSize[comp];
            for ( UInt idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++ )
            {
              lastPalette[comp][idx] = m_entropyCodingSyncPaletteState.lastPalette[comp][idx];
            }
          }
        }
      }
    }

    for (UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++)
    {
      pCtu->setLastPaletteInLcuSizeFinal(comp, lastPaletteSize[comp]);
      for ( UInt idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++ )
      {
        pCtu->setLastPaletteInLcuFinal(comp, lastPalette[comp][idx], idx);
      }
    }

    if ( pcSlice->getSPS()->getUseSAO() )
    {
      Bool bIsSAOSliceEnabled = false;
      Bool sliceEnabled[MAX_NUM_COMPONENT];
      for(Int comp=0; comp < MAX_NUM_COMPONENT; comp++)
      {
        ComponentID compId=ComponentID(comp);
        sliceEnabled[compId] = pcSlice->getSaoEnabledFlag(toChannelType(compId)) && (comp < pcPic->getNumberValidComponents());
        if (sliceEnabled[compId])
        {
          bIsSAOSliceEnabled=true;
        }
      }
      if (bIsSAOSliceEnabled)
      {
        SAOBlkParam& saoblkParam = (pcPic->getPicSym()->getSAOBlkParam())[ctuRsAddr];

        Bool leftMergeAvail = false;
        Bool aboveMergeAvail= false;
        //merge left condition
        Int rx = (ctuRsAddr % frameWidthInCtus);
        if(rx > 0)
        {
          leftMergeAvail = pcPic->getSAOMergeAvailability(ctuRsAddr, ctuRsAddr-1);
        }

        //merge up condition
        Int ry = (ctuRsAddr / frameWidthInCtus);
        if(ry > 0)
        {
          aboveMergeAvail = pcPic->getSAOMergeAvailability(ctuRsAddr, ctuRsAddr-frameWidthInCtus);
        }

        m_pcEntropyCoder->encodeSAOBlkParam(saoblkParam, pcPic->getPicSym()->getSPS().getBitDepths(), sliceEnabled, leftMergeAvail, aboveMergeAvail);
      }
    }

#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceEnable;
#endif
      m_pcCuEncoder->encodeCtu( pCtu );
#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceDisable;
#endif

    if (pCtu->getLastPaletteInLcuSizeFinal(COMPONENT_Y))
    {
      for (UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++)
      {
        lastPaletteSize[comp] = pCtu->getLastPaletteInLcuSizeFinal(comp);
        for (Int idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++)
        {
          lastPalette[comp][idx] = pCtu->getLastPaletteInLcuFinal(comp, idx);
        }
      }
    }


    //Store probabilities of second CTU in line into buffer
    if ( ctuXPosInCtus == tileXPosInCtus+1 && wavefrontsEnabled)
    {
      m_entropyCodingSyncContextState.loadContexts( m_pcSbacCoder );
      for ( UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++ )
      {
        m_entropyCodingSyncPaletteState.lastPaletteSize[comp] = lastPaletteSize[comp];
        for ( UInt idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++ )
        {
          m_entropyCodingSyncPaletteState.lastPalette[comp][idx] = lastPalette[comp][idx];
        }
      }
    }

    // terminate the sub-stream, if required (end of slice-segment, end of tile, end of wavefront-CTU-row):
    if (ctuTsAddr+1 == boundingCtuTsAddr ||
         (  ctuXPosInCtus + 1 == tileXPosInCtus + currentTile.getTileWidthInCtus() &&
          ( ctuYPosInCtus + 1 == tileYPosInCtus + currentTile.getTileHeightInCtus() || wavefrontsEnabled)
         )
       )
    {
      m_pcEntropyCoder->encodeTerminatingBit(1);
      m_pcEntropyCoder->encodeSliceFinish();
      // Byte-alignment in slice_data() when new tile
      pcSubstreams[uiSubStrm].writeByteAlignment();

      // write sub-stream size
      if (ctuTsAddr+1 != boundingCtuTsAddr)
      {
        pcSlice->addSubstreamSize( (pcSubstreams[uiSubStrm].getNumberOfWrittenBits() >> 3) + pcSubstreams[uiSubStrm].countStartCodeEmulations() );
      }
    }
  } // CTU-loop

  if( depSliceSegmentsEnabled )
  {
    m_lastSliceSegmentEndContextState.loadContexts( m_pcSbacCoder );//ctx end of dep.slice
    for ( UChar comp = 0; comp < MAX_NUM_COMPONENT; comp++ )
    {
      m_lastSliceSegmentEndPaletteState.lastPaletteSize[comp] = lastPaletteSize[comp];
      for ( UInt idx = 0; idx < pcSlice->getSPS()->getSpsScreenExtension().getPaletteMaxPredSize(); idx++ )
      {
        m_lastSliceSegmentEndPaletteState.lastPalette[comp][idx] = lastPalette[comp][idx];
      }
    }
  }

#if ADAPTIVE_QP_SELECTION
  if( m_pcCfg->getUseAdaptQpSelect() )
  {
    m_pcTrQuant->storeSliceQpNext(pcSlice); // TODO: this will only be storing the adaptive QP state of the very last slice-segment that is not dependent in the frame... Perhaps this should be moved to the compress slice loop.
  }
#endif

  if (pcSlice->getPPS()->getCabacInitPresentFlag() && !pcSlice->getPPS()->getDependentSliceSegmentsEnabledFlag())
  {
    m_encCABACTableIdx = m_pcEntropyCoder->determineCabacInitIdx(pcSlice);
  }
  else
  {
    m_encCABACTableIdx = pcSlice->getSliceType();
  }
  
  numBinsCoded = m_pcBinCABAC->getBinsCoded();
}

Void TEncSlice::calculateBoundingCtuTsAddrForSlice(UInt &startCtuTSAddrSlice, UInt &boundingCtuTSAddrSlice, Bool &haveReachedTileBoundary,
                                                   TComPic* pcPic, const Int sliceMode, const Int sliceArgument)
{
  TComSlice* pcSlice = pcPic->getSlice(getSliceIdx());
  const UInt numberOfCtusInFrame = pcPic->getNumberOfCtusInFrame();
  const TComPPS &pps=*(pcSlice->getPPS());
  boundingCtuTSAddrSlice=0;
  haveReachedTileBoundary=false;

  switch (sliceMode)
  {
    case FIXED_NUMBER_OF_CTU:
      {
        UInt ctuAddrIncrement    = sliceArgument;
        boundingCtuTSAddrSlice  = ((startCtuTSAddrSlice + ctuAddrIncrement) < numberOfCtusInFrame) ? (startCtuTSAddrSlice + ctuAddrIncrement) : numberOfCtusInFrame;
      }
      break;
    case FIXED_NUMBER_OF_BYTES:
      boundingCtuTSAddrSlice  = numberOfCtusInFrame; // This will be adjusted later if required.
      break;
    case FIXED_NUMBER_OF_TILES:
      {
        const UInt tileIdx        = pcPic->getPicSym()->getTileIdxMap( pcPic->getPicSym()->getCtuTsToRsAddrMap(startCtuTSAddrSlice) );
        const UInt tileTotalCount = (pcPic->getPicSym()->getNumTileColumnsMinus1()+1) * (pcPic->getPicSym()->getNumTileRowsMinus1()+1);
        UInt ctuAddrIncrement   = 0;

        for(UInt tileIdxIncrement = 0; tileIdxIncrement < sliceArgument; tileIdxIncrement++)
        {
          if((tileIdx + tileIdxIncrement) < tileTotalCount)
          {
            UInt tileWidthInCtus   = pcPic->getPicSym()->getTComTile(tileIdx + tileIdxIncrement)->getTileWidthInCtus();
            UInt tileHeightInCtus  = pcPic->getPicSym()->getTComTile(tileIdx + tileIdxIncrement)->getTileHeightInCtus();
            ctuAddrIncrement    += (tileWidthInCtus * tileHeightInCtus);
          }
        }

        boundingCtuTSAddrSlice  = ((startCtuTSAddrSlice + ctuAddrIncrement) < numberOfCtusInFrame) ? (startCtuTSAddrSlice + ctuAddrIncrement) : numberOfCtusInFrame;
      }
      break;
    default:
      boundingCtuTSAddrSlice    = numberOfCtusInFrame;
      break;
  }

  // Adjust for tiles and wavefronts.
  const Bool wavefrontsAreEnabled = pps.getEntropyCodingSyncEnabledFlag();

  if ((sliceMode == FIXED_NUMBER_OF_CTU || sliceMode == FIXED_NUMBER_OF_BYTES) &&
      (pps.getNumTileRowsMinus1() > 0 || pps.getNumTileColumnsMinus1() > 0))
  {
    const UInt ctuRSAddr                  = pcPic->getPicSym()->getCtuTsToRsAddrMap(startCtuTSAddrSlice);
    const UInt startTileIdx               = pcPic->getPicSym()->getTileIdxMap(ctuRSAddr);

    const TComTile *pStartingTile         = pcPic->getPicSym()->getTComTile(startTileIdx);
    const UInt tileStartTsAddr            = pcPic->getPicSym()->getCtuRsToTsAddrMap(pStartingTile->getFirstCtuRsAddr());
    const UInt tileStartWidth             = pStartingTile->getTileWidthInCtus();
    const UInt tileStartHeight            = pStartingTile->getTileHeightInCtus();
    const UInt tileLastTsAddr_excl        = tileStartTsAddr + tileStartWidth*tileStartHeight;
    const UInt tileBoundingCtuTsAddrSlice = tileLastTsAddr_excl;

    const UInt ctuColumnOfStartingTile    = ((startCtuTSAddrSlice-tileStartTsAddr)%tileStartWidth);
    if (wavefrontsAreEnabled && ctuColumnOfStartingTile!=0)
    {
      // WPP: if a slice does not start at the beginning of a CTB row, it must end within the same CTB row
      const UInt numberOfCTUsToEndOfRow            = tileStartWidth - ctuColumnOfStartingTile;
      const UInt wavefrontTileBoundingCtuAddrSlice = startCtuTSAddrSlice + numberOfCTUsToEndOfRow;
      if (wavefrontTileBoundingCtuAddrSlice < boundingCtuTSAddrSlice)
      {
        boundingCtuTSAddrSlice = wavefrontTileBoundingCtuAddrSlice;
      }
    }

    if (tileBoundingCtuTsAddrSlice < boundingCtuTSAddrSlice)
    {
      boundingCtuTSAddrSlice = tileBoundingCtuTsAddrSlice;
      haveReachedTileBoundary = true;
    }
  }
  else if ((sliceMode == FIXED_NUMBER_OF_CTU || sliceMode == FIXED_NUMBER_OF_BYTES) && wavefrontsAreEnabled && ((startCtuTSAddrSlice % pcPic->getFrameWidthInCtus()) != 0))
  {
    // Adjust for wavefronts (no tiles).
    // WPP: if a slice does not start at the beginning of a CTB row, it must end within the same CTB row
    boundingCtuTSAddrSlice = min(boundingCtuTSAddrSlice, startCtuTSAddrSlice - (startCtuTSAddrSlice % pcPic->getFrameWidthInCtus()) + (pcPic->getFrameWidthInCtus()));
  }
}

/** Determines the starting and bounding CTU address of current slice / dependent slice
 * \param [out] startCtuTsAddr
 * \param [out] boundingCtuTsAddr
 * \param [in]  pcPic

 * Updates startCtuTsAddr, boundingCtuTsAddr with appropriate CTU address
 */
Void TEncSlice::xDetermineStartAndBoundingCtuTsAddr  ( UInt& startCtuTsAddr, UInt& boundingCtuTsAddr, TComPic* pcPic )
{
  TComSlice* pcSlice                 = pcPic->getSlice(getSliceIdx());

  // Non-dependent slice
  UInt startCtuTsAddrSlice           = pcSlice->getSliceCurStartCtuTsAddr();
  Bool haveReachedTileBoundarySlice  = false;
  UInt boundingCtuTsAddrSlice;
  calculateBoundingCtuTsAddrForSlice(startCtuTsAddrSlice, boundingCtuTsAddrSlice, haveReachedTileBoundarySlice, pcPic,
                                     m_pcCfg->getSliceMode(), m_pcCfg->getSliceArgument());
  pcSlice->setSliceCurEndCtuTsAddr(   boundingCtuTsAddrSlice );
  pcSlice->setSliceCurStartCtuTsAddr( startCtuTsAddrSlice    );

  // Dependent slice
  UInt startCtuTsAddrSliceSegment          = pcSlice->getSliceSegmentCurStartCtuTsAddr();
  Bool haveReachedTileBoundarySliceSegment = false;
  UInt boundingCtuTsAddrSliceSegment;
  calculateBoundingCtuTsAddrForSlice(startCtuTsAddrSliceSegment, boundingCtuTsAddrSliceSegment, haveReachedTileBoundarySliceSegment, pcPic,
                                     m_pcCfg->getSliceSegmentMode(), m_pcCfg->getSliceSegmentArgument());
  if (boundingCtuTsAddrSliceSegment>boundingCtuTsAddrSlice)
  {
    boundingCtuTsAddrSliceSegment = boundingCtuTsAddrSlice;
  }
  pcSlice->setSliceSegmentCurEndCtuTsAddr( boundingCtuTsAddrSliceSegment );
  pcSlice->setSliceSegmentCurStartCtuTsAddr(startCtuTsAddrSliceSegment);

  // Make a joint decision based on reconstruction and dependent slice bounds
  startCtuTsAddr    = max(startCtuTsAddrSlice   , startCtuTsAddrSliceSegment   );
  boundingCtuTsAddr = boundingCtuTsAddrSliceSegment;
}

Double TEncSlice::xGetQPValueAccordingToLambda ( Double lambda )
{
  return 4.2005*log(lambda) + 13.7122;
}

//! \}
