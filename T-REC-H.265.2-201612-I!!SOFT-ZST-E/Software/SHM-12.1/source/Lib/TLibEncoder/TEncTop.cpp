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

/** \file     TEncTop.cpp
    \brief    encoder class
*/

#include "TLibCommon/CommonDef.h"
#include "TEncTop.h"
#include "TEncPic.h"
#include "TLibCommon/TComChromaFormat.h"
#if FAST_BIT_EST
#include "TLibCommon/ContextModel.h"
#endif

//! \ingroup TLibEncoder
//! \{
#if SVC_EXTENSION  
Int TEncTop::m_iSPSIdCnt = 0;
Int TEncTop::m_iPPSIdCnt = 0;
TComVPS TEncCfg::m_cVPS;
#endif

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncTop::TEncTop()
{
  m_iPOCLast          = -1;
  m_iNumPicRcvd       =  0;
  m_uiNumAllPicCoded  =  0;
  m_pppcRDSbacCoder   =  NULL;
  m_pppcBinCoderCABAC =  NULL;
  m_cRDGoOnSbacCoder.init( &m_cRDGoOnBinCoderCABAC );
#if ENC_DEC_TRACE
  if (g_hTrace == NULL)
  {
    g_hTrace = fopen( "TraceEnc.txt", "wb" );
  }
  g_bJustDoIt = g_bEncDecTraceDisable;
  g_nSymbolCounter = 0;
#endif

  m_iMaxRefPicNum     = 0;

#if FAST_BIT_EST
  ContextModel::buildNextStateTable();
#endif

#if SVC_EXTENSION
  memset( m_cIlpPic, 0, sizeof(m_cIlpPic) );
  m_bMFMEnabledFlag = false;
  m_numRefLayerLocationOffsets = 0;
  m_pocAdjustmentValue     = 0;
#if NO_CLRAS_OUTPUT_FLAG
  m_noClrasOutputFlag          = false;
  m_layerInitializedFlag       = false;
  m_firstPicInLayerDecodedFlag = false;
  m_noOutputOfPriorPicsFlags   = false;
#endif
  m_pocDecrementedInDPBFlag    = false;
#endif //SVC_EXTENSION
}

TEncTop::~TEncTop()
{
#if ENC_DEC_TRACE
  if (g_hTrace != stdout)
  {
    fclose( g_hTrace );
  }
#endif
}

Void TEncTop::create ()
{
#if !SVC_EXTENSION
  // initialize global variables
  initROM();
#endif

  // create processing unit classes
#if SVC_EXTENSION
  m_cGOPEncoder.        create( m_layerId );
#else
  m_cGOPEncoder.        create( );
#endif
  m_cSliceEncoder.      create( getSourceWidth(), getSourceHeight(), m_chromaFormatIDC, m_maxCUWidth, m_maxCUHeight, m_maxTotalCUDepth );
  m_cCuEncoder.         create( m_maxTotalCUDepth, m_maxCUWidth, m_maxCUHeight, m_chromaFormatIDC );
  if (m_bUseSAO)
  {
    m_cEncSAO.create( getSourceWidth(), getSourceHeight(), m_chromaFormatIDC, m_maxCUWidth, m_maxCUHeight, m_maxTotalCUDepth, m_log2SaoOffsetScale[CHANNEL_TYPE_LUMA], m_log2SaoOffsetScale[CHANNEL_TYPE_CHROMA] );
    m_cEncSAO.createEncData(getSaoCtuBoundary());
  }
#if ADAPTIVE_QP_SELECTION
  if (m_bUseAdaptQpSelect)
  {
    m_cTrQuant.initSliceQpDelta();
  }
#endif

  m_cLoopFilter.create( m_maxTotalCUDepth );

  if ( m_RCEnableRateControl )
  {
    m_cRateCtrl.init( m_framesToBeEncoded, m_RCTargetBitrate, (Int)( (Double)m_iFrameRate/m_temporalSubsampleRatio + 0.5), m_iGOPSize, m_iSourceWidth, m_iSourceHeight,
                      m_maxCUWidth, m_maxCUHeight,m_RCKeepHierarchicalBit, m_RCUseLCUSeparateModel, m_GOPList );
  }

  m_pppcRDSbacCoder = new TEncSbac** [m_maxTotalCUDepth+1];
#if FAST_BIT_EST
  m_pppcBinCoderCABAC = new TEncBinCABACCounter** [m_maxTotalCUDepth+1];
#else
  m_pppcBinCoderCABAC = new TEncBinCABAC** [m_maxTotalCUDepth+1];
#endif

  for ( Int iDepth = 0; iDepth < m_maxTotalCUDepth+1; iDepth++ )
  {
    m_pppcRDSbacCoder[iDepth] = new TEncSbac* [CI_NUM];
#if FAST_BIT_EST
    m_pppcBinCoderCABAC[iDepth] = new TEncBinCABACCounter* [CI_NUM];
#else
    m_pppcBinCoderCABAC[iDepth] = new TEncBinCABAC* [CI_NUM];
#endif

    for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
    {
      m_pppcRDSbacCoder[iDepth][iCIIdx] = new TEncSbac;
#if FAST_BIT_EST
      m_pppcBinCoderCABAC [iDepth][iCIIdx] = new TEncBinCABACCounter;
#else
      m_pppcBinCoderCABAC [iDepth][iCIIdx] = new TEncBinCABAC;
#endif
      m_pppcRDSbacCoder   [iDepth][iCIIdx]->init( m_pppcBinCoderCABAC [iDepth][iCIIdx] );
    }
  }

#if LAYER_CTB
  memcpy(g_auiLayerZscanToRaster[m_layerId], g_auiZscanToRaster, sizeof( g_auiZscanToRaster ) );
  memcpy(g_auiLayerRasterToZscan[m_layerId], g_auiRasterToZscan, sizeof( g_auiRasterToZscan ) );
  memcpy(g_auiLayerRasterToPelX[m_layerId],  g_auiRasterToPelX,  sizeof( g_auiRasterToPelX ) );
  memcpy(g_auiLayerRasterToPelY[m_layerId],  g_auiRasterToPelY,  sizeof( g_auiRasterToPelY ) );
#endif
}

Void TEncTop::destroy ()
{
  // destroy processing unit classes
  m_cGOPEncoder.        destroy();
  m_cSliceEncoder.      destroy();
  m_cCuEncoder.         destroy();
  m_cEncSAO.            destroyEncData();
  m_cEncSAO.            destroy();
  m_cLoopFilter.        destroy();
  m_cRateCtrl.          destroy();
  m_cSearch.            destroy();
  Int iDepth;
  for ( iDepth = 0; iDepth < m_maxTotalCUDepth+1; iDepth++ )
  {
    for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
    {
      delete m_pppcRDSbacCoder[iDepth][iCIIdx];
      delete m_pppcBinCoderCABAC[iDepth][iCIIdx];
    }
  }

  for ( iDepth = 0; iDepth < m_maxTotalCUDepth+1; iDepth++ )
  {
    delete [] m_pppcRDSbacCoder[iDepth];
    delete [] m_pppcBinCoderCABAC[iDepth];
  }

  delete [] m_pppcRDSbacCoder;
  delete [] m_pppcBinCoderCABAC;

#if SVC_EXTENSION
  for(Int i=0; i<MAX_NUM_REF; i++)
  {
    if(m_cIlpPic[i])
    {
      m_cIlpPic[i]->destroy();
      delete m_cIlpPic[i];
      m_cIlpPic[i] = NULL;
    }
  }
#else
  // destroy ROM
  destroyROM();
#endif
  return;
}

Void TEncTop::init(Bool isFieldCoding)
{
  // initialize SPS
  xInitSPS();
  xInitVPS();

#if U0132_TARGET_BITS_SATURATION
  if (m_RCCpbSaturationEnabled)
  {
    m_cRateCtrl.initHrdParam(m_cSPS.getVuiParameters()->getHrdParameters(), m_iFrameRate, m_RCInitialCpbFullness);
  }
#endif
  m_cRdCost.setCostMode(m_costMode);

  // initialize PPS
  xInitPPS();
  xInitRPS(isFieldCoding);

  xInitPPSforTiles();

  // initialize processing unit classes
  m_cGOPEncoder.  init( this );
  m_cSliceEncoder.init( this );
  m_cCuEncoder.   init( this );

  // initialize transform & quantization class
  m_pcCavlcCoder = getCavlcCoder();

  m_cTrQuant.init( 1 << m_uiQuadtreeTULog2MaxSize,
                   m_useRDOQ,
                   m_useRDOQTS,
#if T0196_SELECTIVE_RDOQ
                   m_useSelectiveRDOQ,
#endif
                   true
                  ,m_useTransformSkipFast
#if ADAPTIVE_QP_SELECTION
                  ,m_bUseAdaptQpSelect
#endif
                  );

  // initialize encoder search class
  m_cSearch.init( this, &m_cTrQuant, m_iSearchRange, m_bipredSearchRange, m_motionEstimationSearchMethod, m_maxCUWidth, m_maxCUHeight, m_maxTotalCUDepth, &m_cEntropyCoder, &m_cRdCost, getRDSbacCoder(), getRDGoOnSbacCoder() );

  m_iMaxRefPicNum = 0;

  xInitScalingLists();

#if SVC_EXTENSION
  m_iSPSIdCnt ++;
  m_iPPSIdCnt ++;
  xInitILRP();
#endif
}

Void TEncTop::xInitScalingLists()
{
  // Initialise scaling lists
  // The encoder will only use the SPS scaling lists. The PPS will never be marked present.
  const Int maxLog2TrDynamicRange[MAX_NUM_CHANNEL_TYPE] =
  {
      m_cSPS.getMaxLog2TrDynamicRange(CHANNEL_TYPE_LUMA),
      m_cSPS.getMaxLog2TrDynamicRange(CHANNEL_TYPE_CHROMA)
  };
  if(getUseScalingListId() == SCALING_LIST_OFF)
  {
    getTrQuant()->setFlatScalingList(maxLog2TrDynamicRange, m_cSPS.getBitDepths());
    getTrQuant()->setUseScalingList(false);
    m_cSPS.setScalingListPresentFlag(false);
    m_cPPS.setScalingListPresentFlag(false);
  }
  else if(getUseScalingListId() == SCALING_LIST_DEFAULT)
  {
#if SVC_EXTENSION
    // inferring of the scaling list can be moved to the config file
    UInt refLayerId = 0;
    if( m_layerId > 0 && !m_cVPS.getNonHEVCBaseLayerFlag() && m_cVPS.getRecursiveRefLayerFlag( m_layerId, refLayerId ) )
    {
      m_cSPS.setInferScalingListFlag( true );
      m_cSPS.setScalingListRefLayerId( refLayerId );
      m_cSPS.setScalingListPresentFlag( false );
      m_cPPS.setInferScalingListFlag( false );
      m_cPPS.setScalingListPresentFlag( false );

      // infer the scaling list from the reference layer
      getTrQuant()->setScalingList( &m_ppcTEncTop[m_cVPS.getLayerIdxInVps(refLayerId)]->getSPS()->getScalingList(), maxLog2TrDynamicRange, m_ppcTEncTop[m_cVPS.getLayerIdxInVps(refLayerId)]->getSPS()->getBitDepths() );
    }
    else
    {
#endif
    m_cSPS.getScalingList().setDefaultScalingList ();
    m_cSPS.setScalingListPresentFlag(false);
    m_cPPS.setScalingListPresentFlag(false);

    getTrQuant()->setScalingList(&(m_cSPS.getScalingList()), maxLog2TrDynamicRange, m_cSPS.getBitDepths());
#if SVC_EXTENSION
    }
#endif
    getTrQuant()->setUseScalingList(true);
  }
  else if(getUseScalingListId() == SCALING_LIST_FILE_READ)
  {
#if SVC_EXTENSION
    // inferring of the scaling list can be moved to the config file
    UInt refLayerId = 0;
    if( m_layerId > 0 && !m_cVPS.getNonHEVCBaseLayerFlag() && m_cVPS.getRecursiveRefLayerFlag( m_layerId, refLayerId ) )
    {
      m_cSPS.setInferScalingListFlag( true );
      m_cSPS.setScalingListRefLayerId( refLayerId );
      m_cSPS.setScalingListPresentFlag( false );
      m_cPPS.setInferScalingListFlag( false );
      m_cPPS.setScalingListPresentFlag( false );

      // infer the scaling list from the reference layer
      getTrQuant()->setScalingList( &m_ppcTEncTop[m_cVPS.getLayerIdxInVps(refLayerId)]->getSPS()->getScalingList(), maxLog2TrDynamicRange, m_ppcTEncTop[m_cVPS.getLayerIdxInVps(refLayerId)]->getSPS()->getBitDepths() );
    }
    else
    {
#endif
    m_cSPS.getScalingList().setDefaultScalingList ();
    if(m_cSPS.getScalingList().xParseScalingList(getScalingListFileName()))
    {
      Bool bParsedScalingList=false; // Use of boolean so that assertion outputs useful string
      assert(bParsedScalingList);
      exit(1);
    }
    m_cSPS.getScalingList().checkDcOfMatrix();
    m_cSPS.setScalingListPresentFlag(m_cSPS.getScalingList().checkDefaultScalingList());
    m_cPPS.setScalingListPresentFlag(false);
    getTrQuant()->setScalingList(&(m_cSPS.getScalingList()), maxLog2TrDynamicRange, m_cSPS.getBitDepths());
#if SVC_EXTENSION
    }
#endif
    getTrQuant()->setUseScalingList(true);
  }
  else
  {
    printf("error : ScalingList == %d not supported\n",getUseScalingListId());
    assert(0);
  }

  if (getUseScalingListId() != SCALING_LIST_OFF)
  {
    // Prepare delta's:
    for(UInt sizeId = 0; sizeId < SCALING_LIST_SIZE_NUM; sizeId++)
    {
      const Int predListStep = (sizeId == SCALING_LIST_32x32? (SCALING_LIST_NUM/NUMBER_OF_PREDICTION_MODES) : 1); // if 32x32, skip over chroma entries.

      for(UInt listId = 0; listId < SCALING_LIST_NUM; listId+=predListStep)
      {
        m_cSPS.getScalingList().checkPredMode( sizeId, listId );
      }
    }
  }
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncTop::deletePicBuffer()
{
  TComList<TComPic*>::iterator iterPic = m_cListPic.begin();
  Int iSize = Int( m_cListPic.size() );

  for ( Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);

    pcPic->destroy();
    delete pcPic;
    pcPic = NULL;
  }
}

/**
 - Application has picture buffer list with size of GOP + 1
 - Picture buffer list acts like as ring buffer
 - End of the list has the latest picture
 .
 \param   flush               cause encoder to encode a partial GOP
 \param   pcPicYuvOrg         original YUV picture
 \param   pcPicYuvTrueOrg     
 \param   snrCSC
 \retval  rcListPicYuvRecOut  list of reconstruction YUV pictures
 \retval  accessUnitsOut      list of output access units
 \retval  iNumEncoded         number of encoded pictures
 */
#if SVC_EXTENSION
Void TEncTop::encode( TComPicYuv* pcPicYuvOrg, const InputColourSpaceConversion snrCSC, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsOut, Int iPicIdInGOP )
{
  // compress GOP
#if !RC_SHVC_HARMONIZATION
  if ( m_RCEnableRateControl )
  {
    m_cRateCtrl.initRCGOP( m_iNumPicRcvd );
  }
#endif

  // compress GOP
  m_cGOPEncoder.compressGOP(iPicIdInGOP, m_iPOCLast, m_iNumPicRcvd, m_cListPic, rcListPicYuvRecOut, accessUnitsOut, false, false, snrCSC, m_printFrameMSE);

#if !RC_SHVC_HARMONIZATION
  if ( m_RCEnableRateControl )
  {
    m_cRateCtrl.destroyRCGOP();
  }
#endif
  
  m_uiNumAllPicCoded ++;
}

Void TEncTop::encodePrep( TComPicYuv* pcPicYuvOrg, TComPicYuv* pcPicYuvTrueOrg )
{
  if (pcPicYuvOrg != NULL)
  {
    // get original YUV
    TComPic* pcPicCurr = NULL;
    xGetNewPicBuffer( pcPicCurr );
    pcPicYuvOrg->copyToPic( pcPicCurr->getPicYuvOrg() );
    pcPicYuvTrueOrg->copyToPic( pcPicCurr->getPicYuvTrueOrg() );

    // compute image characteristics
    if ( getUseAdaptiveQP() )
    {
      m_cPreanalyzer.xPreanalyze( dynamic_cast<TEncPic*>( pcPicCurr ) );
    }
  }
}
#else
Void TEncTop::encode( Bool flush, TComPicYuv* pcPicYuvOrg, TComPicYuv* pcPicYuvTrueOrg, const InputColourSpaceConversion snrCSC, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsOut, Int& iNumEncoded )
{
  if (pcPicYuvOrg != NULL)
  {
    // get original YUV
    TComPic* pcPicCurr = NULL;

    xGetNewPicBuffer( pcPicCurr );
    pcPicYuvOrg->copyToPic( pcPicCurr->getPicYuvOrg() );
    pcPicYuvTrueOrg->copyToPic( pcPicCurr->getPicYuvTrueOrg() );

    // compute image characteristics
    if ( getUseAdaptiveQP() )
    {
      m_cPreanalyzer.xPreanalyze( dynamic_cast<TEncPic*>( pcPicCurr ) );
    }
  }

  if ((m_iNumPicRcvd == 0) || (!flush && (m_iPOCLast != 0) && (m_iNumPicRcvd != m_iGOPSize) && (m_iGOPSize != 0)))
  {
    iNumEncoded = 0;
    return;
  }

  if ( m_RCEnableRateControl )
  {
    m_cRateCtrl.initRCGOP( m_iNumPicRcvd );
  }

  // compress GOP
  m_cGOPEncoder.compressGOP(m_iPOCLast, m_iNumPicRcvd, m_cListPic, rcListPicYuvRecOut, accessUnitsOut, false, false, snrCSC, m_printFrameMSE);

  if ( m_RCEnableRateControl )
  {
    m_cRateCtrl.destroyRCGOP();
  }

  iNumEncoded         = m_iNumPicRcvd;
  m_iNumPicRcvd       = 0;
  m_uiNumAllPicCoded += iNumEncoded;
}
#endif

/**------------------------------------------------
 Separate interlaced frame into two fields
 -------------------------------------------------**/
Void separateFields(Pel* org, Pel* dstField, UInt stride, UInt width, UInt height, Bool isTop)
{
  if (!isTop)
  {
    org += stride;
  }
  for (Int y = 0; y < height>>1; y++)
  {
    for (Int x = 0; x < width; x++)
    {
      dstField[x] = org[x];
    }

    dstField += stride;
    org += stride*2;
  }

}

#if SVC_EXTENSION
Void TEncTop::encodePrep( TComPicYuv* pcPicYuvOrg, TComPicYuv* pcPicYuvTrueOrg, Bool isTff )
{
  for (Int fieldNum=0; fieldNum<2; fieldNum++)
  {
    if (pcPicYuvOrg)
    {
      /* -- field initialization -- */
      const Bool isTopField=isTff==(fieldNum==0);

      TComPic *pcField;
      xGetNewPicBuffer( pcField );
      pcField->setReconMark (false);                     // where is this normally?
            
      pcField->getSlice(0)->setPOC( m_iPOCLast );        // superfluous?
#if !REDUCED_ENCODER_MEMORY
      pcField->getPicYuvRec()->setBorderExtension(false);// where is this normally?
#endif

      pcField->setTopField(isTopField);                  // interlaced requirement

      for (UInt componentIndex = 0; componentIndex < pcPicYuvOrg->getNumberValidComponents(); componentIndex++)
      {
        const ComponentID component = ComponentID(componentIndex);
        const UInt stride = pcPicYuvOrg->getStride(component);

        separateFields((pcPicYuvOrg->getBuf(component) + pcPicYuvOrg->getMarginX(component) + (pcPicYuvOrg->getMarginY(component) * stride)),
                       pcField->getPicYuvOrg()->getAddr(component),
                       pcPicYuvOrg->getStride(component),
                       pcPicYuvOrg->getWidth(component),
                       pcPicYuvOrg->getHeight(component),
                       isTopField);

        separateFields((pcPicYuvTrueOrg->getBuf(component) + pcPicYuvTrueOrg->getMarginX(component) + (pcPicYuvTrueOrg->getMarginY(component) * stride)),
                       pcField->getPicYuvTrueOrg()->getAddr(component),
                       pcPicYuvTrueOrg->getStride(component),
                       pcPicYuvTrueOrg->getWidth(component),
                       pcPicYuvTrueOrg->getHeight(component),
                       isTopField);
      }

      // compute image characteristics
      if ( getUseAdaptiveQP() )
      {
        m_cPreanalyzer.xPreanalyze( dynamic_cast<TEncPic*>( pcField ) );
      }
    }
  }
}

Void TEncTop::encode( TComPicYuv* pcPicYuvOrg, const InputColourSpaceConversion snrCSC, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsOut, Int iPicIdInGOP, Bool isTff )
{
  for (Int fieldNum=0; fieldNum<2; fieldNum++)
  {
    if (pcPicYuvOrg)
    {
      if (fieldNum==1)                                   // where is this normally?
      {
        TComPicYuv* rpcPicYuvRec;

        // org. buffer
        if ( rcListPicYuvRecOut.size() >= (UInt)m_iGOPSize+1 ) // need to maintain field 0 in list of RecOuts while processing field 1. Hence +1 on m_iGOPSize.
        {
          rpcPicYuvRec = rcListPicYuvRecOut.popFront();
        }
        else
        {
          rpcPicYuvRec = new TComPicYuv;
          rpcPicYuvRec->create( m_iSourceWidth, m_iSourceHeight, m_chromaFormatIDC, m_maxCUWidth, m_maxCUHeight, m_maxTotalCUDepth, true);
        }
        rcListPicYuvRecOut.pushBack( rpcPicYuvRec );
      }
    }

    // compress GOP
    m_cGOPEncoder.compressGOP(iPicIdInGOP, m_iPOCLast, m_iNumPicRcvd, m_cListPic, rcListPicYuvRecOut, accessUnitsOut, true, isTff, snrCSC, m_printFrameMSE);
  }

  m_uiNumAllPicCoded ++;
}
#else
Void TEncTop::encode(Bool flush, TComPicYuv* pcPicYuvOrg, TComPicYuv* pcPicYuvTrueOrg, const InputColourSpaceConversion snrCSC, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsOut, Int& iNumEncoded, Bool isTff)
{
  iNumEncoded = 0;

  for (Int fieldNum=0; fieldNum<2; fieldNum++)
  {
    if (pcPicYuvOrg)
    {

      /* -- field initialization -- */
      const Bool isTopField=isTff==(fieldNum==0);

      TComPic *pcField;
      xGetNewPicBuffer( pcField );
      pcField->setReconMark (false);                     // where is this normally?

      if (fieldNum==1)                                   // where is this normally?
      {
        TComPicYuv* rpcPicYuvRec;

        // org. buffer
        if ( rcListPicYuvRecOut.size() >= (UInt)m_iGOPSize+1 ) // need to maintain field 0 in list of RecOuts while processing field 1. Hence +1 on m_iGOPSize.
        {
          rpcPicYuvRec = rcListPicYuvRecOut.popFront();
        }
        else
        {
          rpcPicYuvRec = new TComPicYuv;
          rpcPicYuvRec->create( m_iSourceWidth, m_iSourceHeight, m_chromaFormatIDC, m_maxCUWidth, m_maxCUHeight, m_maxTotalCUDepth, true);
        }
        rcListPicYuvRecOut.pushBack( rpcPicYuvRec );
      }

      pcField->getSlice(0)->setPOC( m_iPOCLast );        // superfluous?
#if !REDUCED_ENCODER_MEMORY
      pcField->getPicYuvRec()->setBorderExtension(false);// where is this normally?
#endif

      pcField->setTopField(isTopField);                  // interlaced requirement

      for (UInt componentIndex = 0; componentIndex < pcPicYuvOrg->getNumberValidComponents(); componentIndex++)
      {
        const ComponentID component = ComponentID(componentIndex);
        const UInt stride = pcPicYuvOrg->getStride(component);

        separateFields((pcPicYuvOrg->getBuf(component) + pcPicYuvOrg->getMarginX(component) + (pcPicYuvOrg->getMarginY(component) * stride)),
                       pcField->getPicYuvOrg()->getAddr(component),
                       pcPicYuvOrg->getStride(component),
                       pcPicYuvOrg->getWidth(component),
                       pcPicYuvOrg->getHeight(component),
                       isTopField);

        separateFields((pcPicYuvTrueOrg->getBuf(component) + pcPicYuvTrueOrg->getMarginX(component) + (pcPicYuvTrueOrg->getMarginY(component) * stride)),
                       pcField->getPicYuvTrueOrg()->getAddr(component),
                       pcPicYuvTrueOrg->getStride(component),
                       pcPicYuvTrueOrg->getWidth(component),
                       pcPicYuvTrueOrg->getHeight(component),
                       isTopField);
      }

      // compute image characteristics
      if ( getUseAdaptiveQP() )
      {
        m_cPreanalyzer.xPreanalyze( dynamic_cast<TEncPic*>( pcField ) );
      }
    }

    if ( m_iNumPicRcvd && ((flush&&fieldNum==1) || (m_iPOCLast/2)==0 || m_iNumPicRcvd==m_iGOPSize ) )
    {
      // compress GOP
      m_cGOPEncoder.compressGOP(m_iPOCLast, m_iNumPicRcvd, m_cListPic, rcListPicYuvRecOut, accessUnitsOut, true, isTff, snrCSC, m_printFrameMSE);

      iNumEncoded += m_iNumPicRcvd;
      m_uiNumAllPicCoded += m_iNumPicRcvd;
      m_iNumPicRcvd = 0;
    }
  }
}
#endif

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

/**
 - Application has picture buffer list with size of GOP + 1
 - Picture buffer list acts like as ring buffer
 - End of the list has the latest picture
 .
 \retval rpcPic obtained picture buffer
 */
Void TEncTop::xGetNewPicBuffer ( TComPic*& rpcPic )
{
  // At this point, the SPS and PPS can be considered activated - they are copied to the new TComPic.

  TComSlice::sortPicList(m_cListPic);


  if (m_cListPic.size() >= (UInt)(m_iGOPSize + getMaxDecPicBuffering(MAX_TLAYER-1) + 2) )
  {
    TComList<TComPic*>::iterator iterPic  = m_cListPic.begin();
    Int iSize = Int( m_cListPic.size() );
    for ( Int i = 0; i < iSize; i++ )
    {
      rpcPic = *(iterPic++);
      if(rpcPic->getSlice(0)->isReferenced() == false)
      {
        break;
      }
    }
#if REDUCED_ENCODER_MEMORY
    rpcPic->releaseAllReconstructionData();
    rpcPic->prepareForEncoderSourcePicYuv();
#endif
  }
  else
  {
    if ( getUseAdaptiveQP() )
    {
      TEncPic* pcEPic = new TEncPic;

#if SVC_EXTENSION //Temporal solution, should be modified
      if( m_layerId > 0 )
      {
        for( UInt i = 0; i < m_cVPS.getNumDirectRefLayers( m_layerId ); i++ )
        {
          const Window scalEL = m_cPPS.getScaledRefLayerWindowForLayer(m_cVPS.getRefLayerId(m_layerId, i));
          const Window altRL = m_cPPS.getRefLayerWindowForLayer(m_cVPS.getRefLayerId(m_layerId, i));
          Bool equalOffsets = scalEL.hasEqualOffset(altRL);
          Bool zeroPhase = m_cPPS.hasZeroResamplingPhase(m_cVPS.getRefLayerId(m_layerId, i));

          TEncTop *pcEncTopBase = (TEncTop *)getRefLayerEnc( i );

          const Int bitDepthLuma = m_cSPS.getBitDepth(CHANNEL_TYPE_LUMA);
          const Int bitDepthChroma = m_cSPS.getBitDepth(CHANNEL_TYPE_CHROMA);
          const Int refBitDepthLuma = pcEncTopBase->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA);
          const Int refBitDepthChroma = pcEncTopBase->getSPS()->getBitDepth(CHANNEL_TYPE_CHROMA);

          Bool sameBitDepths = ( bitDepthLuma == refBitDepthLuma ) && ( bitDepthChroma == refBitDepthChroma );

          if( m_iSourceWidth == pcEncTopBase->getSourceWidth() && m_iSourceHeight == pcEncTopBase->getSourceHeight() && equalOffsets && zeroPhase )
          {
            pcEPic->setEqualPictureSizeAndOffsetFlag( i, true );
          }

          if( !pcEPic->equalPictureSizeAndOffsetFlag(i) || !sameBitDepths 
#if CGS_3D_ASYMLUT
            || m_cPPS.getCGSFlag() > 0
#endif
            )
          {
            pcEPic->setRequireResamplingFlag( i, true );

            //only for scalable extension
            assert( m_cVPS.getScalabilityMask( SCALABILITY_ID ) == true );
          }
        }
      }
#if REDUCED_ENCODER_MEMORY
      pcEPic->create( m_cVPS, m_cSPS, m_cPPS, m_cPPS.getMaxCuDQPDepth()+1, m_layerId);
#else
      pcEPic->create( m_cVPS, m_cSPS, m_cPPS, m_cPPS.getMaxCuDQPDepth()+1, false, m_layerId);
#endif
#else  //SVC_EXTENSION
#if REDUCED_ENCODER_MEMORY
      pcEPic->create( m_cSPS, m_cPPS, m_cPPS.getMaxCuDQPDepth()+1);
#else
      pcEPic->create( m_cSPS, m_cPPS, m_cPPS.getMaxCuDQPDepth()+1, false);
#endif
#endif //SVC_EXTENSION
      rpcPic = pcEPic;
    }
    else
    {
      rpcPic = new TComPic;

#if SVC_EXTENSION //Temporal solution, should be modified
      if( m_layerId > 0 )
      {
        for( UInt i = 0; i < m_cVPS.getNumDirectRefLayers( m_layerId ); i++ )
        {
          const Window scalEL = m_cPPS.getScaledRefLayerWindowForLayer(m_cVPS.getRefLayerId(m_layerId, i));
          const Window altRL = m_cPPS.getRefLayerWindowForLayer(m_cVPS.getRefLayerId(m_layerId, i));
          Bool equalOffsets = scalEL.hasEqualOffset(altRL);
          Bool zeroPhase = m_cPPS.hasZeroResamplingPhase(m_cVPS.getRefLayerId(m_layerId, i));

          TEncTop *pcEncTopBase = (TEncTop *)getRefLayerEnc( i );

          const Int bitDepthLuma = m_cSPS.getBitDepth(CHANNEL_TYPE_LUMA);
          const Int bitDepthChroma = m_cSPS.getBitDepth(CHANNEL_TYPE_CHROMA);
          const Int refBitDepthLuma = pcEncTopBase->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA);
          const Int refBitDepthChroma = pcEncTopBase->getSPS()->getBitDepth(CHANNEL_TYPE_CHROMA);

          Bool sameBitDepths = ( bitDepthLuma == refBitDepthLuma ) && ( bitDepthChroma == refBitDepthChroma );

          if( m_iSourceWidth != pcEncTopBase->getSourceWidth() || m_iSourceHeight != pcEncTopBase->getSourceHeight() || !sameBitDepths 
            || !equalOffsets 
            || !zeroPhase
#if CGS_3D_ASYMLUT
            || m_cPPS.getCGSFlag() > 0
#endif
            )
          {
            rpcPic->setRequireResamplingFlag( i, true );

            //only for scalable extension
            assert( m_cVPS.getScalabilityMask( SCALABILITY_ID ) == true );
          }
        }
      }

#if REDUCED_ENCODER_MEMORY
      rpcPic->create( m_cSPS, m_cPPS, true, false, m_layerId );
#else
      rpcPic->create( m_cSPS, m_cPPS, false, m_layerId );
#endif
#else  //SVC_EXTENSION
#if REDUCED_ENCODER_MEMORY
      rpcPic->create( m_cSPS, m_cPPS, true, false );
#else
      rpcPic->create( m_cSPS, m_cPPS, false );
#endif      
#endif //SVC_EXTENSION
    }

    m_cListPic.pushBack( rpcPic );
  }
  rpcPic->setReconMark (false);

  m_iPOCLast++;
  m_iNumPicRcvd++;

  rpcPic->getSlice(0)->setPOC( m_iPOCLast );
#if !REDUCED_ENCODER_MEMORY
  // mark it should be extended
  rpcPic->getPicYuvRec()->setBorderExtension(false);
#endif
}

Void TEncTop::xInitVPS()
{
  // The SPS must have already been set up.
  // set the VPS profile information.
#if SVC_EXTENSION
  m_cVPS.setVpsVuiPresentFlag( true );
#else
  *m_cVPS.getPTL() = *m_cSPS.getPTL();
  m_cVPS.setMaxOpSets(1);
#endif
  m_cVPS.getTimingInfo()->setTimingInfoPresentFlag       ( false );
  m_cVPS.setNumHrdParameters( 0 );

  m_cVPS.createHrdParamBuffer();
  for( UInt i = 0; i < m_cVPS.getNumHrdParameters(); i ++ )
  {
    m_cVPS.setHrdOpSetIdx( 0, i );
    m_cVPS.setCprmsPresentFlag( false, i );
    // Set up HrdParameters here.
  }
}

Void TEncTop::xInitSPS()
{
#if SVC_EXTENSION
  m_cSPS.setExtensionFlag( m_layerId > 0 ? true : false );
  m_cSPS.setNumDirectRefLayers(m_numDirectRefLayers);

  if( !m_numDirectRefLayers && m_numAddLayerSets )
  {
    m_cSPS.setLayerId(0); // layer ID 0 for independent layers
  }
  else
  {
    m_cSPS.setLayerId(m_layerId);
  }
#if VIEW_SCALABILITY 
  if( m_bUseDisparitySearchRangeRestriction )
  {
    m_cSPS.setInterViewMvVertConstraintFlag( true );
  }
  else
  {
    m_cSPS.setInterViewMvVertConstraintFlag( false );
  }
#endif
#endif //SVC_EXTENSION

  ProfileTierLevel& profileTierLevel = *m_cSPS.getPTL()->getGeneralPTL();
  profileTierLevel.setLevelIdc(m_level);
  profileTierLevel.setTierFlag(m_levelTier);
  profileTierLevel.setProfileIdc(m_profile);
  profileTierLevel.setProfileCompatibilityFlag(m_profile, 1);
  profileTierLevel.setProgressiveSourceFlag(m_progressiveSourceFlag);
  profileTierLevel.setInterlacedSourceFlag(m_interlacedSourceFlag);
  profileTierLevel.setNonPackedConstraintFlag(m_nonPackedConstraintFlag);
  profileTierLevel.setFrameOnlyConstraintFlag(m_frameOnlyConstraintFlag);
  profileTierLevel.setBitDepthConstraint(m_bitDepthConstraintValue);
  profileTierLevel.setChromaFormatConstraint(m_chromaFormatConstraintValue);
  profileTierLevel.setIntraConstraintFlag(m_intraConstraintFlag);
  profileTierLevel.setOnePictureOnlyConstraintFlag(m_onePictureOnlyConstraintFlag);
  profileTierLevel.setLowerBitRateConstraintFlag(m_lowerBitRateConstraintFlag);

  if ((m_profile == Profile::MAIN10) && (m_bitDepth[CHANNEL_TYPE_LUMA] == 8) && (m_bitDepth[CHANNEL_TYPE_CHROMA] == 8))
  {
    /* The above constraint is equal to Profile::MAIN */
    profileTierLevel.setProfileCompatibilityFlag(Profile::MAIN, 1);
  }
  if (m_profile == Profile::MAIN)
  {
    /* A Profile::MAIN10 decoder can always decode Profile::MAIN */
    profileTierLevel.setProfileCompatibilityFlag(Profile::MAIN10, 1);
  }
  /* XXX: should Main be marked as compatible with still picture? */
  /* XXX: may be a good idea to refactor the above into a function
   * that chooses the actual compatibility based upon options */

  m_cSPS.setPicWidthInLumaSamples  ( m_iSourceWidth      );
  m_cSPS.setPicHeightInLumaSamples ( m_iSourceHeight     );
  m_cSPS.setConformanceWindow      ( m_conformanceWindow );
  m_cSPS.setMaxCUWidth             ( m_maxCUWidth        );
  m_cSPS.setMaxCUHeight            ( m_maxCUHeight       );
  m_cSPS.setMaxTotalCUDepth        ( m_maxTotalCUDepth   );
  m_cSPS.setChromaFormatIdc( m_chromaFormatIDC);
  m_cSPS.setLog2DiffMaxMinCodingBlockSize(m_log2DiffMaxMinCodingBlockSize);

  Int minCUSize = m_cSPS.getMaxCUWidth() >> ( m_cSPS.getLog2DiffMaxMinCodingBlockSize() );
  Int log2MinCUSize = 0;
  while(minCUSize > 1)
  {
    minCUSize >>= 1;
    log2MinCUSize++;
  }

  m_cSPS.setLog2MinCodingBlockSize(log2MinCUSize);

#if SVC_EXTENSION
  m_cSPS.setSPSId         ( m_iSPSIdCnt       );
#endif

  m_cSPS.setPCMLog2MinSize (m_uiPCMLog2MinSize);
  m_cSPS.setUsePCM        ( m_usePCM           );
  m_cSPS.setPCMLog2MaxSize( m_pcmLog2MaxSize  );

  m_cSPS.setQuadtreeTULog2MaxSize( m_uiQuadtreeTULog2MaxSize );
  m_cSPS.setQuadtreeTULog2MinSize( m_uiQuadtreeTULog2MinSize );
  m_cSPS.setQuadtreeTUMaxDepthInter( m_uiQuadtreeTUMaxDepthInter    );
  m_cSPS.setQuadtreeTUMaxDepthIntra( m_uiQuadtreeTUMaxDepthIntra    );

  m_cSPS.setSPSTemporalMVPEnabledFlag((getTMVPModeId() == 2 || getTMVPModeId() == 1));

  m_cSPS.setMaxTrSize   ( 1 << m_uiQuadtreeTULog2MaxSize );

  m_cSPS.setUseAMP ( m_useAMP );

  for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
  {
#if SVC_EXTENSION
    m_cSPS.setBitDepth    (ChannelType(channelType), m_cVPS.getVpsRepFormat( m_cVPS.getVpsRepFormatIdx( m_cVPS.getLayerIdxInVps( m_layerId ) ) )->getBitDepthVps(ChannelType(channelType))            );    
#if O0043_BEST_EFFORT_DECODING
    m_cSPS.setStreamBitDepth(ChannelType(channelType), m_bitDepth[channelType] );
#endif
    m_cSPS.setQpBDOffset  (ChannelType(channelType), (6 * (m_cVPS.getVpsRepFormat( m_cVPS.getVpsRepFormatIdx( m_cVPS.getLayerIdxInVps( m_layerId ) ) )->getBitDepthVps(ChannelType(channelType)) - 8)));
#else
    m_cSPS.setBitDepth      (ChannelType(channelType), m_bitDepth[channelType] );
#if O0043_BEST_EFFORT_DECODING
    m_cSPS.setStreamBitDepth(ChannelType(channelType), m_bitDepth[channelType] );
#endif
    m_cSPS.setQpBDOffset  (ChannelType(channelType), (6 * (m_bitDepth[channelType] - 8)));
#endif
    m_cSPS.setPCMBitDepth (ChannelType(channelType), m_PCMBitDepth[channelType]         );
  }

  m_cSPS.setUseSAO( m_bUseSAO );

  m_cSPS.setMaxTLayers( m_maxTempLayer );
  m_cSPS.setTemporalIdNestingFlag( ( m_maxTempLayer == 1 ) ? true : false );

  for (Int i = 0; i < min(m_cSPS.getMaxTLayers(),(UInt) MAX_TLAYER); i++ )
  {
#if SVC_EXTENSION
    assert( i < MAX_TLAYER );
#endif
    m_cSPS.setMaxDecPicBuffering(m_maxDecPicBuffering[i], i);
    m_cSPS.setNumReorderPics(m_numReorderPics[i], i);
  }

  m_cSPS.setPCMFilterDisableFlag  ( m_bPCMFilterDisableFlag );
  m_cSPS.setScalingListFlag ( (m_useScalingListId == SCALING_LIST_OFF) ? 0 : 1 );
  m_cSPS.setUseStrongIntraSmoothing( m_useStrongIntraSmoothing );
  m_cSPS.setVuiParametersPresentFlag(getVuiParametersPresentFlag());

  if (m_cSPS.getVuiParametersPresentFlag())
  {
    TComVUI* pcVUI = m_cSPS.getVuiParameters();
    pcVUI->setAspectRatioInfoPresentFlag(getAspectRatioInfoPresentFlag());
    pcVUI->setAspectRatioIdc(getAspectRatioIdc());
    pcVUI->setSarWidth(getSarWidth());
    pcVUI->setSarHeight(getSarHeight());
    pcVUI->setOverscanInfoPresentFlag(getOverscanInfoPresentFlag());
    pcVUI->setOverscanAppropriateFlag(getOverscanAppropriateFlag());
    pcVUI->setVideoSignalTypePresentFlag(getVideoSignalTypePresentFlag());
    pcVUI->setVideoFormat(getVideoFormat());
    pcVUI->setVideoFullRangeFlag(getVideoFullRangeFlag());
    pcVUI->setColourDescriptionPresentFlag(getColourDescriptionPresentFlag());
    pcVUI->setColourPrimaries(getColourPrimaries());
    pcVUI->setTransferCharacteristics(getTransferCharacteristics());
    pcVUI->setMatrixCoefficients(getMatrixCoefficients());
    pcVUI->setChromaLocInfoPresentFlag(getChromaLocInfoPresentFlag());
    pcVUI->setChromaSampleLocTypeTopField(getChromaSampleLocTypeTopField());
    pcVUI->setChromaSampleLocTypeBottomField(getChromaSampleLocTypeBottomField());
    pcVUI->setNeutralChromaIndicationFlag(getNeutralChromaIndicationFlag());
    pcVUI->setDefaultDisplayWindow(getDefaultDisplayWindow());
    pcVUI->setFrameFieldInfoPresentFlag(getFrameFieldInfoPresentFlag());
    pcVUI->setFieldSeqFlag(false);
    pcVUI->setHrdParametersPresentFlag(false);
    pcVUI->getTimingInfo()->setPocProportionalToTimingFlag(getPocProportionalToTimingFlag());
    pcVUI->getTimingInfo()->setNumTicksPocDiffOneMinus1   (getNumTicksPocDiffOneMinus1()   );
    pcVUI->setBitstreamRestrictionFlag(getBitstreamRestrictionFlag());
    pcVUI->setTilesFixedStructureFlag(getTilesFixedStructureFlag());
    pcVUI->setMotionVectorsOverPicBoundariesFlag(getMotionVectorsOverPicBoundariesFlag());
    pcVUI->setMinSpatialSegmentationIdc(getMinSpatialSegmentationIdc());
    pcVUI->setMaxBytesPerPicDenom(getMaxBytesPerPicDenom());
    pcVUI->setMaxBitsPerMinCuDenom(getMaxBitsPerMinCuDenom());
    pcVUI->setLog2MaxMvLengthHorizontal(getLog2MaxMvLengthHorizontal());
    pcVUI->setLog2MaxMvLengthVertical(getLog2MaxMvLengthVertical());
  }
  m_cSPS.setNumLongTermRefPicSPS(NUM_LONG_TERM_REF_PIC_SPS);
  assert (NUM_LONG_TERM_REF_PIC_SPS <= MAX_NUM_LONG_TERM_REF_PICS);
  for (Int k = 0; k < NUM_LONG_TERM_REF_PIC_SPS; k++)
  {
    m_cSPS.setLtRefPicPocLsbSps(k, 0);
    m_cSPS.setUsedByCurrPicLtSPSFlag(k, 0);
  }

#if U0132_TARGET_BITS_SATURATION
  if( getPictureTimingSEIEnabled() || getDecodingUnitInfoSEIEnabled() || getCpbSaturationEnabled() )
#else
  if( getPictureTimingSEIEnabled() || getDecodingUnitInfoSEIEnabled() )
#endif
  {
    xInitHrdParameters();
  }
  if( getBufferingPeriodSEIEnabled() || getPictureTimingSEIEnabled() || getDecodingUnitInfoSEIEnabled() )
  {
    m_cSPS.getVuiParameters()->setHrdParametersPresentFlag( true );
  }

  // Set up SPS range extension settings
  m_cSPS.getSpsRangeExtension().setTransformSkipRotationEnabledFlag(m_transformSkipRotationEnabledFlag);
  m_cSPS.getSpsRangeExtension().setTransformSkipContextEnabledFlag(m_transformSkipContextEnabledFlag);
  for (UInt signallingModeIndex = 0; signallingModeIndex < NUMBER_OF_RDPCM_SIGNALLING_MODES; signallingModeIndex++)
  {
    m_cSPS.getSpsRangeExtension().setRdpcmEnabledFlag(RDPCMSignallingMode(signallingModeIndex), m_rdpcmEnabledFlag[signallingModeIndex]);
  }
  m_cSPS.getSpsRangeExtension().setExtendedPrecisionProcessingFlag(m_extendedPrecisionProcessingFlag);
  m_cSPS.getSpsRangeExtension().setIntraSmoothingDisabledFlag( m_intraSmoothingDisabledFlag );
  m_cSPS.getSpsRangeExtension().setHighPrecisionOffsetsEnabledFlag(m_highPrecisionOffsetsEnabledFlag);
  m_cSPS.getSpsRangeExtension().setPersistentRiceAdaptationEnabledFlag(m_persistentRiceAdaptationEnabledFlag);
  m_cSPS.getSpsRangeExtension().setCabacBypassAlignmentEnabledFlag(m_cabacBypassAlignmentEnabledFlag);

#if SVC_EXTENSION
  m_cSPS.inferSPS( m_layerId, &m_cVPS );
#endif
}

#if U0132_TARGET_BITS_SATURATION
// calculate scale value of bitrate and initial delay
Int calcScale(Int x)
{
  if (x==0)
  {
    return 0;
  }
  UInt iMask = 0xffffffff;
  Int ScaleValue = 32;

  while ((x&iMask) != 0)
  {
    ScaleValue--;
    iMask = (iMask >> 1);
  }

  return ScaleValue;
}
#endif
Void TEncTop::xInitHrdParameters()
{
  Bool useSubCpbParams = (getSliceMode() > 0) || (getSliceSegmentMode() > 0);
  Int  bitRate         = getTargetBitrate();
  Bool isRandomAccess  = getIntraPeriod() > 0;
# if U0132_TARGET_BITS_SATURATION
  Int cpbSize          = getCpbSize();
  assert (cpbSize!=0);  // CPB size may not be equal to zero. ToDo: have a better default and check for level constraints
  if( !getVuiParametersPresentFlag() && !getCpbSaturationEnabled() )
#else
  if( !getVuiParametersPresentFlag() )
#endif
  {
    return;
  }

  TComVUI *vui = m_cSPS.getVuiParameters();
  TComHRD *hrd = vui->getHrdParameters();

  TimingInfo *timingInfo = vui->getTimingInfo();

#if SVC_EXTENSION
  if( m_layerId > 0 )
  {
    timingInfo->setTimingInfoPresentFlag( false );
    return;
  }
#endif

  timingInfo->setTimingInfoPresentFlag( true );
  switch( getFrameRate() )
  {
  case 24:
    timingInfo->setNumUnitsInTick( 1125000 );    timingInfo->setTimeScale    ( 27000000 );
    break;
  case 25:
    timingInfo->setNumUnitsInTick( 1080000 );    timingInfo->setTimeScale    ( 27000000 );
    break;
  case 30:
    timingInfo->setNumUnitsInTick( 900900 );     timingInfo->setTimeScale    ( 27000000 );
    break;
  case 50:
    timingInfo->setNumUnitsInTick( 540000 );     timingInfo->setTimeScale    ( 27000000 );
    break;
  case 60:
    timingInfo->setNumUnitsInTick( 450450 );     timingInfo->setTimeScale    ( 27000000 );
    break;
  default:
    timingInfo->setNumUnitsInTick( 1001 );       timingInfo->setTimeScale    ( 60000 );
    break;
  }

  if (getTemporalSubsampleRatio()>1)
  {
    UInt temporalSubsampleRatio = getTemporalSubsampleRatio();
    if ( Double(timingInfo->getNumUnitsInTick()) * temporalSubsampleRatio > std::numeric_limits<UInt>::max() )
    {
      timingInfo->setTimeScale( timingInfo->getTimeScale() / temporalSubsampleRatio );
    }
    else
    {
      timingInfo->setNumUnitsInTick( timingInfo->getNumUnitsInTick() * temporalSubsampleRatio );
    }
  }

  Bool rateCnt = ( bitRate > 0 );
  hrd->setNalHrdParametersPresentFlag( rateCnt );
  hrd->setVclHrdParametersPresentFlag( rateCnt );
  hrd->setSubPicCpbParamsPresentFlag( useSubCpbParams );

  if( hrd->getSubPicCpbParamsPresentFlag() )
  {
    hrd->setTickDivisorMinus2( 100 - 2 );                          //
    hrd->setDuCpbRemovalDelayLengthMinus1( 7 );                    // 8-bit precision ( plus 1 for last DU in AU )
    hrd->setSubPicCpbParamsInPicTimingSEIFlag( true );
    hrd->setDpbOutputDelayDuLengthMinus1( 5 + 7 );                 // With sub-clock tick factor of 100, at least 7 bits to have the same value as AU dpb delay
  }
  else
  {
    hrd->setSubPicCpbParamsInPicTimingSEIFlag( false );
  }

#if U0132_TARGET_BITS_SATURATION
  if (calcScale(bitRate) <= 6)
  {
    hrd->setBitRateScale(0);
  }
  else
  {
    hrd->setBitRateScale(calcScale(bitRate) - 6);
  }

  if (calcScale(cpbSize) <= 4)
  {
    hrd->setCpbSizeScale(0);
  }
  else
  {
    hrd->setCpbSizeScale(calcScale(cpbSize) - 4);
  }
#else
  hrd->setBitRateScale( 4 );                                       // in units of 2^( 6 + 4 ) = 1,024 bps
  hrd->setCpbSizeScale( 6 );                                       // in units of 2^( 4 + 6 ) = 1,024 bit
#endif

  hrd->setDuCpbSizeScale( 6 );                                     // in units of 2^( 4 + 6 ) = 1,024 bit

  hrd->setInitialCpbRemovalDelayLengthMinus1(15);                  // assuming 0.5 sec, log2( 90,000 * 0.5 ) = 16-bit
  if( isRandomAccess )
  {
    hrd->setCpbRemovalDelayLengthMinus1(5);                        // 32 = 2^5 (plus 1)
    hrd->setDpbOutputDelayLengthMinus1 (5);                        // 32 + 3 = 2^6
  }
  else
  {
    hrd->setCpbRemovalDelayLengthMinus1(9);                        // max. 2^10
    hrd->setDpbOutputDelayLengthMinus1 (9);                        // max. 2^10
  }

  // Note: parameters for all temporal layers are initialized with the same values
  Int i, j;
  UInt bitrateValue, cpbSizeValue;
  UInt duCpbSizeValue;
  UInt duBitRateValue = 0;

  for( i = 0; i < MAX_TLAYER; i ++ )
  {
    hrd->setFixedPicRateFlag( i, 1 );
    hrd->setPicDurationInTcMinus1( i, 0 );
    hrd->setLowDelayHrdFlag( i, 0 );
    hrd->setCpbCntMinus1( i, 0 );

    //! \todo check for possible PTL violations
    // BitRate[ i ] = ( bit_rate_value_minus1[ i ] + 1 ) * 2^( 6 + bit_rate_scale )
    bitrateValue = bitRate / (1 << (6 + hrd->getBitRateScale()) );      // bitRate is in bits, so it needs to be scaled down
    // CpbSize[ i ] = ( cpb_size_value_minus1[ i ] + 1 ) * 2^( 4 + cpb_size_scale )
#if U0132_TARGET_BITS_SATURATION
    cpbSizeValue = cpbSize / (1 << (4 + hrd->getCpbSizeScale()) );      // using bitRate results in 1 second CPB size
#else
    cpbSizeValue = bitRate / (1 << (4 + hrd->getCpbSizeScale()) );      // using bitRate results in 1 second CPB size
#endif


    // DU CPB size could be smaller (i.e. bitrateValue / number of DUs), but we don't know 
    // in how many DUs the slice segment settings will result 
    duCpbSizeValue = bitrateValue;
    duBitRateValue = cpbSizeValue;

    for( j = 0; j < ( hrd->getCpbCntMinus1( i ) + 1 ); j ++ )
    {
      hrd->setBitRateValueMinus1( i, j, 0, ( bitrateValue - 1 ) );
      hrd->setCpbSizeValueMinus1( i, j, 0, ( cpbSizeValue - 1 ) );
      hrd->setDuCpbSizeValueMinus1( i, j, 0, ( duCpbSizeValue - 1 ) );
      hrd->setDuBitRateValueMinus1( i, j, 0, ( duBitRateValue - 1 ) );
      hrd->setCbrFlag( i, j, 0, false );

      hrd->setBitRateValueMinus1( i, j, 1, ( bitrateValue - 1) );
      hrd->setCpbSizeValueMinus1( i, j, 1, ( cpbSizeValue - 1 ) );
      hrd->setDuCpbSizeValueMinus1( i, j, 1, ( duCpbSizeValue - 1 ) );
      hrd->setDuBitRateValueMinus1( i, j, 1, ( duBitRateValue - 1 ) );
      hrd->setCbrFlag( i, j, 1, false );
    }
  }
}

Void TEncTop::xInitPPS()
{
  m_cPPS.setConstrainedIntraPred( m_bUseConstrainedIntraPred );
  Bool bUseDQP = (getMaxCuDQPDepth() > 0)? true : false;

  if((getMaxDeltaQP() != 0 )|| getUseAdaptiveQP())
  {
    bUseDQP = true;
  }

  if (m_costMode==COST_SEQUENCE_LEVEL_LOSSLESS || m_costMode==COST_LOSSLESS_CODING)
  {
    bUseDQP=false;
  }


  if ( m_RCEnableRateControl )
  {
    m_cPPS.setUseDQP(true);
    m_cPPS.setMaxCuDQPDepth( 0 );
  }
  else if(bUseDQP)
  {
    m_cPPS.setUseDQP(true);
    m_cPPS.setMaxCuDQPDepth( m_iMaxCuDQPDepth );
  }
  else
  {
    m_cPPS.setUseDQP(false);
    m_cPPS.setMaxCuDQPDepth( 0 );
  }

  if ( m_diffCuChromaQpOffsetDepth >= 0 )
  {
    m_cPPS.getPpsRangeExtension().setDiffCuChromaQpOffsetDepth(m_diffCuChromaQpOffsetDepth);
    m_cPPS.getPpsRangeExtension().clearChromaQpOffsetList();
    m_cPPS.getPpsRangeExtension().setChromaQpOffsetListEntry(1, 6, 6);
    /* todo, insert table entries from command line (NB, 0 should not be touched) */
  }
  else
  {
    m_cPPS.getPpsRangeExtension().setDiffCuChromaQpOffsetDepth(0);
    m_cPPS.getPpsRangeExtension().clearChromaQpOffsetList();
  }
  m_cPPS.getPpsRangeExtension().setCrossComponentPredictionEnabledFlag(m_crossComponentPredictionEnabledFlag);
  m_cPPS.getPpsRangeExtension().setLog2SaoOffsetScale(CHANNEL_TYPE_LUMA,   m_log2SaoOffsetScale[CHANNEL_TYPE_LUMA  ]);
  m_cPPS.getPpsRangeExtension().setLog2SaoOffsetScale(CHANNEL_TYPE_CHROMA, m_log2SaoOffsetScale[CHANNEL_TYPE_CHROMA]);

  m_cPPS.setQpOffset(COMPONENT_Cb, m_chromaCbQpOffset );
  m_cPPS.setQpOffset(COMPONENT_Cr, m_chromaCrQpOffset );
#if W0038_CQP_ADJ
  Bool bChromaDeltaQPEnabled = false;
  {
    bChromaDeltaQPEnabled = ( m_sliceChromaQpOffsetIntraOrPeriodic[0] || m_sliceChromaQpOffsetIntraOrPeriodic[1] );
    if( !bChromaDeltaQPEnabled )
    {
      for( Int i=0; i<m_iGOPSize; i++ )
      {
        if( m_GOPList[i].m_CbQPoffset || m_GOPList[i].m_CrQPoffset )
        {
          bChromaDeltaQPEnabled = true;
          break;
        }
      }
    }
  }
  m_cPPS.setSliceChromaQpFlag(bChromaDeltaQPEnabled);
#endif

  m_cPPS.setEntropyCodingSyncEnabledFlag( m_entropyCodingSyncEnabledFlag );
  m_cPPS.setTilesEnabledFlag( (m_iNumColumnsMinus1 > 0 || m_iNumRowsMinus1 > 0) );
  m_cPPS.setUseWP( m_useWeightedPred );
  m_cPPS.setWPBiPred( m_useWeightedBiPred );
  m_cPPS.setOutputFlagPresentFlag( false );
  m_cPPS.setSignDataHidingEnabledFlag(getSignDataHidingEnabledFlag());

  if ( getDeblockingFilterMetric() )
  {
    m_cPPS.setDeblockingFilterOverrideEnabledFlag(true);
    m_cPPS.setPPSDeblockingFilterDisabledFlag(false);
  }
  else
  {
    m_cPPS.setDeblockingFilterOverrideEnabledFlag( !getLoopFilterOffsetInPPS() );
    m_cPPS.setPPSDeblockingFilterDisabledFlag( getLoopFilterDisable() );
  }

  if (! m_cPPS.getPPSDeblockingFilterDisabledFlag())
  {
    m_cPPS.setDeblockingFilterBetaOffsetDiv2( getLoopFilterBetaOffset() );
    m_cPPS.setDeblockingFilterTcOffsetDiv2( getLoopFilterTcOffset() );
  }
  else
  {
    m_cPPS.setDeblockingFilterBetaOffsetDiv2(0);
    m_cPPS.setDeblockingFilterTcOffsetDiv2(0);
  }

  // deblockingFilterControlPresentFlag is true if any of the settings differ from the inferred values:
  const Bool deblockingFilterControlPresentFlag = m_cPPS.getDeblockingFilterOverrideEnabledFlag() ||
                                                  m_cPPS.getPPSDeblockingFilterDisabledFlag()      ||
                                                  m_cPPS.getDeblockingFilterBetaOffsetDiv2() != 0 ||
                                                  m_cPPS.getDeblockingFilterTcOffsetDiv2() != 0;

  m_cPPS.setDeblockingFilterControlPresentFlag(deblockingFilterControlPresentFlag);

  m_cPPS.setLog2ParallelMergeLevelMinus2   (m_log2ParallelMergeLevelMinus2 );
  m_cPPS.setCabacInitPresentFlag(CABAC_INIT_PRESENT_FLAG);
  m_cPPS.setLoopFilterAcrossSlicesEnabledFlag( m_bLFCrossSliceBoundaryFlag );


  Int histogram[MAX_NUM_REF + 1];
  for( Int i = 0; i <= MAX_NUM_REF; i++ )
  {
    histogram[i]=0;
  }
  for( Int i = 0; i < getGOPSize(); i++)
  {
    assert(getGOPEntry(i).m_numRefPicsActive >= 0 && getGOPEntry(i).m_numRefPicsActive <= MAX_NUM_REF);
    histogram[getGOPEntry(i).m_numRefPicsActive]++;
  }

  Int maxHist=-1;
  Int bestPos=0;
  for( Int i = 0; i <= MAX_NUM_REF; i++ )
  {
    if(histogram[i]>maxHist)
    {
      maxHist=histogram[i];
      bestPos=i;
    }
  }
  assert(bestPos <= 15);
  m_cPPS.setNumRefIdxL0DefaultActive(bestPos);
  m_cPPS.setNumRefIdxL1DefaultActive(bestPos);
  m_cPPS.setTransquantBypassEnabledFlag(getTransquantBypassEnabledFlag());
  m_cPPS.setUseTransformSkip( m_useTransformSkip );
  m_cPPS.getPpsRangeExtension().setLog2MaxTransformSkipBlockSize( m_log2MaxTransformSkipBlockSize  );

  if (m_sliceSegmentMode != NO_SLICES)
  {
    m_cPPS.setDependentSliceSegmentsEnabledFlag( true );
  }
  
#if SVC_EXTENSION
  m_cPPS.setLayerId( m_layerId );

  if( !m_numDirectRefLayers && m_numAddLayerSets )
  {
    m_cPPS.setLayerId(0); // layer ID 0 for independent layers
  }

  if( m_layerId > 0 )
  {
    m_cPPS.setListsModificationPresentFlag(true);
    m_cPPS.setExtensionFlag(true);
  }
  else
  {
    m_cPPS.setListsModificationPresentFlag(false);
    m_cPPS.setExtensionFlag(false);
  }

  m_cPPS.setPPSId( m_iPPSIdCnt );
  m_cPPS.setSPSId( m_iSPSIdCnt );

  if( m_crossLayerBLAFlag )
  {
    m_cPPS.setNumExtraSliceHeaderBits( 3 );
  }

  m_cPPS.setNumRefLayerLocationOffsets(m_numRefLayerLocationOffsets);
  for(Int i = 0; i < m_cPPS.getNumRefLayerLocationOffsets(); i++)
  {
    m_cPPS.setRefLocationOffsetLayerId(i, m_refLocationOffsetLayerId[i]);
    m_cPPS.getScaledRefLayerWindow(i) = m_scaledRefLayerWindow[i];
    m_cPPS.getRefLayerWindow(i) = m_refLayerWindow[i];
    m_cPPS.setScaledRefLayerOffsetPresentFlag( i, m_scaledRefLayerOffsetPresentFlag[i] );
    m_cPPS.setRefRegionOffsetPresentFlag( i, m_refRegionOffsetPresentFlag[i] );
    m_cPPS.setResamplePhaseSetPresentFlag( i, m_resamplePhaseSetPresentFlag[i] );
    m_cPPS.setPhaseHorLuma( m_refLocationOffsetLayerId[i], m_phaseHorLuma[i] );
    m_cPPS.setPhaseVerLuma( m_refLocationOffsetLayerId[i], m_phaseVerLuma[i] );
    m_cPPS.setPhaseHorChroma( m_refLocationOffsetLayerId[i], m_phaseHorChroma[i] );
    m_cPPS.setPhaseVerChroma( m_refLocationOffsetLayerId[i], m_phaseVerChroma[i] );
  }
#if CGS_3D_ASYMLUT
  m_cPPS.setCGSFlag( m_nCGSFlag );
#endif
  m_cPPS.setPocResetInfoPresentFlag( true );
  m_cPPS.setExtensionFlag( true );
  m_cPPS.setSliceHeaderExtensionPresentFlag( true );
#endif //SVC_EXTENSION
}

//Function for initializing m_RPSList, a list of TComReferencePictureSet, based on the GOPEntry objects read from the config file.
Void TEncTop::xInitRPS(Bool isFieldCoding)
{
  TComReferencePictureSet*      rps;

  m_cSPS.createRPSList(getGOPSize() + m_extraRPSs + 1);
  TComRPSList* rpsList = m_cSPS.getRPSList();

  for( Int i = 0; i < getGOPSize()+m_extraRPSs; i++)
  {
    GOPEntry ge = getGOPEntry(i);
    rps = rpsList->getReferencePictureSet(i);
    rps->setNumberOfPictures(ge.m_numRefPics);
    rps->setNumRefIdc(ge.m_numRefIdc);
    Int numNeg = 0;
    Int numPos = 0;
    for( Int j = 0; j < ge.m_numRefPics; j++)
    {
      rps->setDeltaPOC(j,ge.m_referencePics[j]);
      rps->setUsed(j,ge.m_usedByCurrPic[j]);
      if(ge.m_referencePics[j]>0)
      {
        numPos++;
      }
      else
      {
        numNeg++;
      }
    }
    rps->setNumberOfNegativePictures(numNeg);
    rps->setNumberOfPositivePictures(numPos);

    // handle inter RPS intialization from the config file.
    rps->setInterRPSPrediction(ge.m_interRPSPrediction > 0);  // not very clean, converting anything > 0 to true.
    rps->setDeltaRIdxMinus1(0);                               // index to the Reference RPS is always the previous one.
    TComReferencePictureSet*     RPSRef = i>0 ? rpsList->getReferencePictureSet(i-1): NULL;  // get the reference RPS

    if (ge.m_interRPSPrediction == 2)  // Automatic generation of the inter RPS idc based on the RIdx provided.
    {
      assert (RPSRef!=NULL);
      Int deltaRPS = getGOPEntry(i-1).m_POC - ge.m_POC;  // the ref POC - current POC
      Int numRefDeltaPOC = RPSRef->getNumberOfPictures();

      rps->setDeltaRPS(deltaRPS);           // set delta RPS
      rps->setNumRefIdc(numRefDeltaPOC+1);  // set the numRefIdc to the number of pictures in the reference RPS + 1.
      Int count=0;
      for (Int j = 0; j <= numRefDeltaPOC; j++ ) // cycle through pics in reference RPS.
      {
        Int RefDeltaPOC = (j<numRefDeltaPOC)? RPSRef->getDeltaPOC(j): 0;  // if it is the last decoded picture, set RefDeltaPOC = 0
        rps->setRefIdc(j, 0);
        for (Int k = 0; k < rps->getNumberOfPictures(); k++ )  // cycle through pics in current RPS.
        {
          if (rps->getDeltaPOC(k) == ( RefDeltaPOC + deltaRPS))  // if the current RPS has a same picture as the reference RPS.
          {
              rps->setRefIdc(j, (rps->getUsed(k)?1:2));
              count++;
              break;
          }
        }
      }
      if (count != rps->getNumberOfPictures())
      {
        printf("Warning: Unable fully predict all delta POCs using the reference RPS index given in the config file.  Setting Inter RPS to false for this RPS.\n");
        rps->setInterRPSPrediction(0);
      }
    }
    else if (ge.m_interRPSPrediction == 1)  // inter RPS idc based on the RefIdc values provided in config file.
    {
      assert (RPSRef!=NULL);
      rps->setDeltaRPS(ge.m_deltaRPS);
      rps->setNumRefIdc(ge.m_numRefIdc);
      for (Int j = 0; j < ge.m_numRefIdc; j++ )
      {
        rps->setRefIdc(j, ge.m_refIdc[j]);
      }
      // the following code overwrite the deltaPOC and Used by current values read from the config file with the ones
      // computed from the RefIdc.  A warning is printed if they are not identical.
      numNeg = 0;
      numPos = 0;
      TComReferencePictureSet      RPSTemp;  // temporary variable

      for (Int j = 0; j < ge.m_numRefIdc; j++ )
      {
        if (ge.m_refIdc[j])
        {
          Int deltaPOC = ge.m_deltaRPS + ((j < RPSRef->getNumberOfPictures())? RPSRef->getDeltaPOC(j) : 0);
          RPSTemp.setDeltaPOC((numNeg+numPos),deltaPOC);
          RPSTemp.setUsed((numNeg+numPos),ge.m_refIdc[j]==1?1:0);
          if (deltaPOC<0)
          {
            numNeg++;
          }
          else
          {
            numPos++;
          }
        }
      }
      if (numNeg != rps->getNumberOfNegativePictures())
      {
        printf("Warning: number of negative pictures in RPS is different between intra and inter RPS specified in the config file.\n");
        rps->setNumberOfNegativePictures(numNeg);
        rps->setNumberOfPictures(numNeg+numPos);
      }
      if (numPos != rps->getNumberOfPositivePictures())
      {
        printf("Warning: number of positive pictures in RPS is different between intra and inter RPS specified in the config file.\n");
        rps->setNumberOfPositivePictures(numPos);
        rps->setNumberOfPictures(numNeg+numPos);
      }
      RPSTemp.setNumberOfPictures(numNeg+numPos);
      RPSTemp.setNumberOfNegativePictures(numNeg);
      RPSTemp.sortDeltaPOC();     // sort the created delta POC before comparing
      // check if Delta POC and Used are the same
      // print warning if they are not.
      for (Int j = 0; j < ge.m_numRefIdc; j++ )
      {
        if (RPSTemp.getDeltaPOC(j) != rps->getDeltaPOC(j))
        {
          printf("Warning: delta POC is different between intra RPS and inter RPS specified in the config file.\n");
          rps->setDeltaPOC(j,RPSTemp.getDeltaPOC(j));
        }
        if (RPSTemp.getUsed(j) != rps->getUsed(j))
        {
          printf("Warning: Used by Current in RPS is different between intra and inter RPS specified in the config file.\n");
          rps->setUsed(j,RPSTemp.getUsed(j));
        }
      }
    }
  }
  //In case of field coding, we need to set special parameters for the first bottom field of the sequence, since it is not specified in the cfg file.
  //The position = GOPSize + extraRPSs which is (a priori) unused is reserved for this field in the RPS.
  if (isFieldCoding)
  {
    rps = rpsList->getReferencePictureSet(getGOPSize()+m_extraRPSs);
    rps->setNumberOfPictures(1);
    rps->setNumberOfNegativePictures(1);
    rps->setNumberOfPositivePictures(0);
    rps->setNumberOfLongtermPictures(0);
    rps->setDeltaPOC(0,-1);
    rps->setPOC(0,0);
    rps->setUsed(0,true);
    rps->setInterRPSPrediction(false);
    rps->setDeltaRIdxMinus1(0);
    rps->setDeltaRPS(0);
    rps->setNumRefIdc(0);
  }
}

   // This is a function that
   // determines what Reference Picture Set to use
   // for a specific slice (with POC = POCCurr)
Void TEncTop::selectReferencePictureSet(TComSlice* slice, Int POCCurr, Int GOPid )
{
  slice->setRPSidx(GOPid);

  for(Int extraNum=m_iGOPSize; extraNum<m_extraRPSs+m_iGOPSize; extraNum++)
  {
    if(m_uiIntraPeriod > 0 && getDecodingRefreshType() > 0)
    {
      Int POCIndex = POCCurr%m_uiIntraPeriod;
      if(POCIndex == 0)
      {
        POCIndex = m_uiIntraPeriod;
      }
      if(POCIndex == m_GOPList[extraNum].m_POC)
      {
        slice->setRPSidx(extraNum);
      }
    }
    else
    {
      if(POCCurr==m_GOPList[extraNum].m_POC)
      {
        slice->setRPSidx(extraNum);
      }
    }
  }

  if(POCCurr == 1 && slice->getPic()->isField())
  {
    slice->setRPSidx(m_iGOPSize+m_extraRPSs);
  }

  const TComReferencePictureSet *rps = (slice->getSPS()->getRPSList()->getReferencePictureSet(slice->getRPSidx()));
  slice->setRPS(rps);
}

Int TEncTop::getReferencePictureSetIdxForSOP(Int POCCurr, Int GOPid )
{
  Int rpsIdx = GOPid;

  for(Int extraNum=m_iGOPSize; extraNum<m_extraRPSs+m_iGOPSize; extraNum++)
  {
    if(m_uiIntraPeriod > 0 && getDecodingRefreshType() > 0)
    {
      Int POCIndex = POCCurr%m_uiIntraPeriod;
      if(POCIndex == 0)
      {
        POCIndex = m_uiIntraPeriod;
      }
      if(POCIndex == m_GOPList[extraNum].m_POC)
      {
        rpsIdx = extraNum;
      }
    }
    else
    {
      if(POCCurr==m_GOPList[extraNum].m_POC)
      {
        rpsIdx = extraNum;
      }
    }
  }

  return rpsIdx;
}

Void  TEncTop::xInitPPSforTiles()
{
  m_cPPS.setTileUniformSpacingFlag( m_tileUniformSpacingFlag );
  m_cPPS.setNumTileColumnsMinus1( m_iNumColumnsMinus1 );
  m_cPPS.setNumTileRowsMinus1( m_iNumRowsMinus1 );
  if( !m_tileUniformSpacingFlag )
  {
    m_cPPS.setTileColumnWidth( m_tileColumnWidth );
    m_cPPS.setTileRowHeight( m_tileRowHeight );
  }
  m_cPPS.setLoopFilterAcrossTilesEnabledFlag( m_loopFilterAcrossTilesEnabledFlag );

  // # substreams is "per tile" when tiles are independent.
}

Void  TEncCfg::xCheckGSParameters()
{
  Int   iWidthInCU = ( m_iSourceWidth%m_maxCUWidth ) ? m_iSourceWidth/m_maxCUWidth + 1 : m_iSourceWidth/m_maxCUWidth;
  Int   iHeightInCU = ( m_iSourceHeight%m_maxCUHeight ) ? m_iSourceHeight/m_maxCUHeight + 1 : m_iSourceHeight/m_maxCUHeight;
  UInt  uiCummulativeColumnWidth = 0;
  UInt  uiCummulativeRowHeight = 0;

  //check the column relative parameters
  if( m_iNumColumnsMinus1 >= (1<<(LOG2_MAX_NUM_COLUMNS_MINUS1+1)) )
  {
    printf( "The number of columns is larger than the maximum allowed number of columns.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumColumnsMinus1 >= iWidthInCU )
  {
    printf( "The current picture can not have so many columns.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumColumnsMinus1 && !m_tileUniformSpacingFlag )
  {
    for(Int i=0; i<m_iNumColumnsMinus1; i++)
    {
      uiCummulativeColumnWidth += m_tileColumnWidth[i];
    }

    if( uiCummulativeColumnWidth >= iWidthInCU )
    {
      printf( "The width of the column is too large.\n" );
      exit( EXIT_FAILURE );
    }
  }

  //check the row relative parameters
  if( m_iNumRowsMinus1 >= (1<<(LOG2_MAX_NUM_ROWS_MINUS1+1)) )
  {
    printf( "The number of rows is larger than the maximum allowed number of rows.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumRowsMinus1 >= iHeightInCU )
  {
    printf( "The current picture can not have so many rows.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumRowsMinus1 && !m_tileUniformSpacingFlag )
  {
    for(Int i=0; i<m_iNumRowsMinus1; i++)
    {
      uiCummulativeRowHeight += m_tileRowHeight[i];
    }

    if( uiCummulativeRowHeight >= iHeightInCU )
    {
      printf( "The height of the row is too large.\n" );
      exit( EXIT_FAILURE );
    }
  }
}

#if SVC_EXTENSION
TEncTop* TEncTop::getRefLayerEnc( UInt refLayerIdx )
{
  if( m_ppcTEncTop[m_cVPS.getLayerIdxInVps(m_layerId)]->getNumDirectRefLayers() <= 0 )
  {
    return (TEncTop *)getLayerEnc( 0 );
  }

  return (TEncTop *)getLayerEnc( m_cVPS.getLayerIdxInVps(m_cVPS.getRefLayerId( m_layerId, refLayerIdx )) );
}

Void TEncTop::xInitILRP()
{
  if( m_layerId > 0 )
  {
    if( m_cIlpPic[0] == NULL )
    {
      for (Int j=0; j < m_numDirectRefLayers; j++)
      {
        m_cIlpPic[j] = new TComPic;
#if REDUCED_ENCODER_MEMORY
        m_cIlpPic[j]->create(m_cSPS, m_cPPS, false, true, m_layerId);
#else
        m_cIlpPic[j]->create(m_cSPS, m_cPPS, true, m_layerId);
#endif
        for (Int i=0; i<m_cIlpPic[j]->getPicSym()->getNumberOfCtusInFrame(); i++)
        {
          m_cIlpPic[j]->getPicSym()->getCtu(i)->initCtu(m_cIlpPic[j], i);
        }
      }
    }

    if( m_cVPS.getNumRefLayers( m_layerId ) == 0 )
    {
      UInt layerIdx = m_cVPS.getLayerIdxInVps( m_layerId );
      RepFormat* repFmt = m_cVPS.getVpsRepFormat(m_cVPS.getVpsRepFormatIdx(layerIdx));
      
      if( m_cPPS.getLayerId() == 0 && 
          m_cSPS.getLayerId() == 0 &&
          repFmt->getChromaFormatVpsIdc() == m_cSPS.getChromaFormatIdc() &&
          repFmt->getSeparateColourPlaneVpsFlag() == 0 &&
          repFmt->getPicHeightVpsInLumaSamples() == m_cSPS.getPicHeightInLumaSamples() &&
          repFmt->getPicWidthVpsInLumaSamples()  == m_cSPS.getPicWidthInLumaSamples() &&
          repFmt->getBitDepthVpsLuma()   == m_cSPS.getBitDepth(CHANNEL_TYPE_LUMA) &&
          repFmt->getBitDepthVpsChroma() == m_cSPS.getBitDepth(CHANNEL_TYPE_CHROMA) &&
          repFmt->getConformanceWindowVps().getWindowLeftOffset()   == m_cSPS.getConformanceWindow().getWindowLeftOffset() &&
          repFmt->getConformanceWindowVps().getWindowRightOffset()  == m_cSPS.getConformanceWindow().getWindowRightOffset() &&
          repFmt->getConformanceWindowVps().getWindowTopOffset()    == m_cSPS.getConformanceWindow().getWindowTopOffset() &&
          repFmt->getConformanceWindowVps().getWindowBottomOffset() == m_cSPS.getConformanceWindow().getWindowBottomOffset() )
      {
        m_cVPS.setBaseLayerPSCompatibilityFlag(layerIdx, 1);
      }
      else
      {
        m_cVPS.setBaseLayerPSCompatibilityFlag(layerIdx, 0);
      }
    }
  }
}

Window TEncTop::getScaledRefLayerWindowForLayer(Int layerId)
{
  for (Int i = 0; i < m_numRefLayerLocationOffsets; i++)
  {
    if (layerId == m_refLocationOffsetLayerId[i])
    {
      return m_scaledRefLayerWindow[i];
    }
  }

  return Window();
}

Window TEncTop::getRefLayerWindowForLayer(Int layerId)
{
  for (Int i = 0; i < m_numRefLayerLocationOffsets; i++)
  {
    if (layerId == m_refLayerId[i])
    {
      return m_refLayerWindow[i];
    }
  }

  return Window();
}
#endif //SVC_EXTENSION
//! \}
