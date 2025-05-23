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

#include "TLibCommon/TComBitCounter.h"
#include "TLibCommon/TComBitStream.h"
#include "TLibCommon/SEI.h"
#include "TLibCommon/TComSlice.h"
#include "TLibCommon/TComPicYuv.h"
#include "SEIwrite.h"

//! \ingroup TLibEncoder
//! \{

#if ENC_DEC_TRACE
Void  xTraceSEIHeader()
{
  fprintf( g_hTrace, "=========== SEI message ===========\n");
}

Void  xTraceSEIMessageType(SEI::PayloadType payloadType)
{
  fprintf( g_hTrace, "=========== %s SEI message ===========\n", SEI::getSEIMessageString(payloadType));
}
#endif

Void SEIWriter::xWriteSEIpayloadData(TComBitIf& bs, const SEI& sei, const TComSPS *sps)
{
  switch (sei.payloadType())
  {
  case SEI::USER_DATA_UNREGISTERED:
    xWriteSEIuserDataUnregistered(*static_cast<const SEIuserDataUnregistered*>(&sei));
    break;
  case SEI::ACTIVE_PARAMETER_SETS:
    xWriteSEIActiveParameterSets(*static_cast<const SEIActiveParameterSets*>(& sei));
    break;
  case SEI::DECODING_UNIT_INFO:
    xWriteSEIDecodingUnitInfo(*static_cast<const SEIDecodingUnitInfo*>(& sei), sps);
    break;
  case SEI::DECODED_PICTURE_HASH:
    xWriteSEIDecodedPictureHash(*static_cast<const SEIDecodedPictureHash*>(&sei));
    break;
  case SEI::BUFFERING_PERIOD:
    xWriteSEIBufferingPeriod(*static_cast<const SEIBufferingPeriod*>(&sei), sps);
    break;
  case SEI::PICTURE_TIMING:
    xWriteSEIPictureTiming(*static_cast<const SEIPictureTiming*>(&sei), sps);
    break;
  case SEI::RECOVERY_POINT:
    xWriteSEIRecoveryPoint(*static_cast<const SEIRecoveryPoint*>(&sei));
    break;
  case SEI::FRAME_PACKING:
    xWriteSEIFramePacking(*static_cast<const SEIFramePacking*>(&sei));
    break;
  case SEI::SEGM_RECT_FRAME_PACKING:
    xWriteSEISegmentedRectFramePacking(*static_cast<const SEISegmentedRectFramePacking*>(&sei));
    break;
  case SEI::DISPLAY_ORIENTATION:
    xWriteSEIDisplayOrientation(*static_cast<const SEIDisplayOrientation*>(&sei));
    break;
  case SEI::TEMPORAL_LEVEL0_INDEX:
    xWriteSEITemporalLevel0Index(*static_cast<const SEITemporalLevel0Index*>(&sei));
    break;
  case SEI::REGION_REFRESH_INFO:
    xWriteSEIGradualDecodingRefreshInfo(*static_cast<const SEIGradualDecodingRefreshInfo*>(&sei));
    break;
  case SEI::NO_DISPLAY:
    xWriteSEINoDisplay(*static_cast<const SEINoDisplay*>(&sei));
    break;
  case SEI::TONE_MAPPING_INFO:
    xWriteSEIToneMappingInfo(*static_cast<const SEIToneMappingInfo*>(&sei));
    break;
  case SEI::SOP_DESCRIPTION:
    xWriteSEISOPDescription(*static_cast<const SEISOPDescription*>(&sei));
    break;
  case SEI::SCALABLE_NESTING:
    xWriteSEIScalableNesting(bs, *static_cast<const SEIScalableNesting*>(&sei), sps);
    break;
  case SEI::CHROMA_RESAMPLING_FILTER_HINT:
    xWriteSEIChromaResamplingFilterHint(*static_cast<const SEIChromaResamplingFilterHint*>(&sei));
    break;
  case SEI::TEMP_MOTION_CONSTRAINED_TILE_SETS:
    xWriteSEITempMotionConstrainedTileSets(*static_cast<const SEITempMotionConstrainedTileSets*>(&sei));
    break;
  case SEI::TIME_CODE:
    xWriteSEITimeCode(*static_cast<const SEITimeCode*>(&sei));
    break;
  case SEI::KNEE_FUNCTION_INFO:
    xWriteSEIKneeFunctionInfo(*static_cast<const SEIKneeFunctionInfo*>(&sei));
    break;
  case SEI::COLOUR_REMAPPING_INFO:
    xWriteSEIColourRemappingInfo(*static_cast<const SEIColourRemappingInfo*>(&sei));
    break;
  case SEI::MASTERING_DISPLAY_COLOUR_VOLUME:
    xWriteSEIMasteringDisplayColourVolume(*static_cast<const SEIMasteringDisplayColourVolume*>(&sei));
    break;
#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
  case SEI::ALTERNATIVE_TRANSFER_CHARACTERISTICS:
    xWriteSEIAlternativeTransferCharacteristics(*static_cast<const SEIAlternativeTransferCharacteristics*>(&sei));
    break;
#endif

#if NH_MV
   case SEI::LAYERS_NOT_PRESENT:
       xWriteSEILayersNotPresent(*static_cast<const SEILayersNotPresent*>(&sei));
     break;
   case SEI::INTER_LAYER_CONSTRAINED_TILE_SETS:
     xWriteSEIInterLayerConstrainedTileSets(*static_cast<const SEIInterLayerConstrainedTileSets*>(&sei));
     break;
#if NH_MV_SEI_TBD
   case SEI::BSP_NESTING:
     xWriteSEIBspNesting(*static_cast<const SEIBspNesting*>(&sei));
     break;
   case SEI::BSP_INITIAL_ARRIVAL_TIME:
     xWriteSEIBspInitialArrivalTime(*static_cast<const SEIBspInitialArrivalTime*>(&sei));
     break;
#endif
   case SEI::SUB_BITSTREAM_PROPERTY:
     xWriteSEISubBitstreamProperty(*static_cast<const SEISubBitstreamProperty*>(&sei));
     break;
   case SEI::ALPHA_CHANNEL_INFO:
     xWriteSEIAlphaChannelInfo(*static_cast<const SEIAlphaChannelInfo*>(&sei));
     break;
   case SEI::OVERLAY_INFO:
     xWriteSEIOverlayInfo(*static_cast<const SEIOverlayInfo*>(&sei));
     break;
   case SEI::TEMPORAL_MV_PREDICTION_CONSTRAINTS:
     xWriteSEITemporalMvPredictionConstraints(*static_cast<const SEITemporalMvPredictionConstraints*>(&sei));
     break;
#if NH_MV_SEI_TBD
   case SEI::FRAME_FIELD_INFO:
     xWriteSEIFrameFieldInfo(*static_cast<const SEIFrameFieldInfo*>(&sei));
     break;
#endif
   case SEI::THREE_DIMENSIONAL_REFERENCE_DISPLAYS_INFO:
     xWriteSEIThreeDimensionalReferenceDisplaysInfo(*static_cast<const SEIThreeDimensionalReferenceDisplaysInfo*>(&sei));
     break;
   case SEI::DEPTH_REPRESENTATION_INFO:
       xWriteSEIDepthRepresentationInfo(*static_cast<const SEIDepthRepresentationInfo*>(&sei));
       break;
   case SEI::MULTIVIEW_SCENE_INFO:
     xWriteSEIMultiviewSceneInfo(*static_cast<const SEIMultiviewSceneInfo*>(&sei));
     break;
   case SEI::MULTIVIEW_ACQUISITION_INFO:
     xWriteSEIMultiviewAcquisitionInfo(*static_cast<const SEIMultiviewAcquisitionInfo*>(&sei));
     break;

   case SEI::MULTIVIEW_VIEW_POSITION:
     xWriteSEIMultiviewViewPosition(*static_cast<const SEIMultiviewViewPosition*>(&sei));
     break;
#if NH_3D
   case SEI::ALTERNATIVE_DEPTH_INFO:
     xWriteSEIAlternativeDepthInfo(*static_cast<const SEIAlternativeDepthInfo*>(&sei));
     break;
#endif
#endif


  default:
    assert(!"Trying to write unhandled SEI message");
    break;
  }
  xWriteByteAlign();
}

/**
 * marshal all SEI messages in provided list into one bitstream bs
 */
Void SEIWriter::writeSEImessages(TComBitIf& bs, const SEIMessages &seiList, const TComSPS *sps, Bool isNested)
{
#if ENC_DEC_TRACE
  if (g_HLSTraceEnable)
    xTraceSEIHeader();
#endif

  TComBitCounter bs_count;

  for (SEIMessages::const_iterator sei=seiList.begin(); sei!=seiList.end(); sei++)
  {
    // calculate how large the payload data is
    // TODO: this would be far nicer if it used vectored buffers
    bs_count.resetBits();
    setBitstream(&bs_count);

#if ENC_DEC_TRACE
    Bool traceEnable = g_HLSTraceEnable;
    g_HLSTraceEnable = false;
#endif
    xWriteSEIpayloadData(bs_count, **sei, sps);
#if ENC_DEC_TRACE
    g_HLSTraceEnable = traceEnable;
#endif
    UInt payload_data_num_bits = bs_count.getNumberOfWrittenBits();
    assert(0 == payload_data_num_bits % 8);

    setBitstream(&bs);
    UInt payloadType = (*sei)->payloadType();
    for (; payloadType >= 0xff; payloadType -= 0xff)
    {
      WRITE_CODE(0xff, 8, "payload_type");
    }
    WRITE_CODE(payloadType, 8, "payload_type");

    UInt payloadSize = payload_data_num_bits/8;
    for (; payloadSize >= 0xff; payloadSize -= 0xff)
    {
      WRITE_CODE(0xff, 8, "payload_size");
    }
    WRITE_CODE(payloadSize, 8, "payload_size");

    /* payloadData */
#if ENC_DEC_TRACE
    if (g_HLSTraceEnable)
      xTraceSEIMessageType((*sei)->payloadType());
#endif

    xWriteSEIpayloadData(bs, **sei, sps);
  }
  if (!isNested)
  {
    xWriteRbspTrailingBits();
  }
}

/**
 * marshal a user_data_unregistered SEI message sei, storing the marshalled
 * representation in bitstream bs.
 */
Void SEIWriter::xWriteSEIuserDataUnregistered(const SEIuserDataUnregistered &sei)
{
  for (UInt i = 0; i < ISO_IEC_11578_LEN; i++)
  {
    WRITE_CODE(sei.uuid_iso_iec_11578[i], 8 , "sei.uuid_iso_iec_11578[i]");
  }

  for (UInt i = 0; i < sei.userDataLength; i++)
  {
    WRITE_CODE(sei.userData[i], 8 , "user_data");
  }
}

/**
 * marshal a decoded picture hash SEI message, storing the marshalled
 * representation in bitstream bs.
 */
Void SEIWriter::xWriteSEIDecodedPictureHash(const SEIDecodedPictureHash& sei)
{
  const TChar *traceString="\0";
  switch (sei.method)
  {
    case HASHTYPE_MD5: traceString="picture_md5"; break;
    case HASHTYPE_CRC: traceString="picture_crc"; break;
    case HASHTYPE_CHECKSUM: traceString="picture_checksum"; break;
    default: assert(false); break;
  }

  if (traceString != 0) //use of this variable is needed to avoid a compiler error with G++ 4.6.1
  {
    WRITE_CODE(sei.method, 8, "hash_type");
    for(UInt i=0; i<UInt(sei.m_pictureHash.hash.size()); i++)
    {
      WRITE_CODE(sei.m_pictureHash.hash[i], 8, traceString);
    }
  }
}

Void SEIWriter::xWriteSEIActiveParameterSets(const SEIActiveParameterSets& sei)
{
  WRITE_CODE(sei.activeVPSId,     4,         "active_video_parameter_set_id");
  WRITE_FLAG(sei.m_selfContainedCvsFlag,     "self_contained_cvs_flag");
  WRITE_FLAG(sei.m_noParameterSetUpdateFlag, "no_parameter_set_update_flag");
  WRITE_UVLC(sei.numSpsIdsMinus1,            "num_sps_ids_minus1");

  assert (sei.activeSeqParameterSetId.size() == (sei.numSpsIdsMinus1 + 1));

  for (Int i = 0; i < sei.activeSeqParameterSetId.size(); i++)
  {
    WRITE_UVLC(sei.activeSeqParameterSetId[i], "active_seq_parameter_set_id");
  }
}

Void SEIWriter::xWriteSEIDecodingUnitInfo(const SEIDecodingUnitInfo& sei, const TComSPS *sps)
{
  const TComVUI *vui = sps->getVuiParameters();
  WRITE_UVLC(sei.m_decodingUnitIdx, "decoding_unit_idx");
  if(vui->getHrdParameters()->getSubPicCpbParamsInPicTimingSEIFlag())
  {
    WRITE_CODE( sei.m_duSptCpbRemovalDelay, (vui->getHrdParameters()->getDuCpbRemovalDelayLengthMinus1() + 1), "du_spt_cpb_removal_delay_increment");
  }
  WRITE_FLAG( sei.m_dpbOutputDuDelayPresentFlag, "dpb_output_du_delay_present_flag");
  if(sei.m_dpbOutputDuDelayPresentFlag)
  {
    WRITE_CODE(sei.m_picSptDpbOutputDuDelay, vui->getHrdParameters()->getDpbOutputDelayDuLengthMinus1() + 1, "pic_spt_dpb_output_du_delay");
  }
}

Void SEIWriter::xWriteSEIBufferingPeriod(const SEIBufferingPeriod& sei, const TComSPS *sps)
{
  Int i, nalOrVcl;
  const TComVUI *vui = sps->getVuiParameters();
  const TComHRD *hrd = vui->getHrdParameters();

  WRITE_UVLC( sei.m_bpSeqParameterSetId, "bp_seq_parameter_set_id" );
  if( !hrd->getSubPicCpbParamsPresentFlag() )
  {
    WRITE_FLAG( sei.m_rapCpbParamsPresentFlag, "irap_cpb_params_present_flag" );
  }
  if( sei.m_rapCpbParamsPresentFlag )
  {
    WRITE_CODE( sei.m_cpbDelayOffset, hrd->getCpbRemovalDelayLengthMinus1() + 1, "cpb_delay_offset" );
    WRITE_CODE( sei.m_dpbDelayOffset, hrd->getDpbOutputDelayLengthMinus1()  + 1, "dpb_delay_offset" );
  }
  WRITE_FLAG( sei.m_concatenationFlag, "concatenation_flag");
  WRITE_CODE( sei.m_auCpbRemovalDelayDelta - 1, ( hrd->getCpbRemovalDelayLengthMinus1() + 1 ), "au_cpb_removal_delay_delta_minus1" );
  for( nalOrVcl = 0; nalOrVcl < 2; nalOrVcl ++ )
  {
    if( ( ( nalOrVcl == 0 ) && ( hrd->getNalHrdParametersPresentFlag() ) ) ||
        ( ( nalOrVcl == 1 ) && ( hrd->getVclHrdParametersPresentFlag() ) ) )
    {
      for( i = 0; i < ( hrd->getCpbCntMinus1( 0 ) + 1 ); i ++ )
      {
        WRITE_CODE( sei.m_initialCpbRemovalDelay[i][nalOrVcl],( hrd->getInitialCpbRemovalDelayLengthMinus1() + 1 ) ,           "initial_cpb_removal_delay" );
        WRITE_CODE( sei.m_initialCpbRemovalDelayOffset[i][nalOrVcl],( hrd->getInitialCpbRemovalDelayLengthMinus1() + 1 ),      "initial_cpb_removal_delay_offset" );
        if( hrd->getSubPicCpbParamsPresentFlag() || sei.m_rapCpbParamsPresentFlag )
        {
          WRITE_CODE( sei.m_initialAltCpbRemovalDelay[i][nalOrVcl], ( hrd->getInitialCpbRemovalDelayLengthMinus1() + 1 ) ,     "initial_alt_cpb_removal_delay" );
          WRITE_CODE( sei.m_initialAltCpbRemovalDelayOffset[i][nalOrVcl], ( hrd->getInitialCpbRemovalDelayLengthMinus1() + 1 ),"initial_alt_cpb_removal_delay_offset" );
        }
      }
    }
  }
}
Void SEIWriter::xWriteSEIPictureTiming(const SEIPictureTiming& sei, const TComSPS *sps)
{
  Int i;
  const TComVUI *vui = sps->getVuiParameters();
  const TComHRD *hrd = vui->getHrdParameters();

  if( vui->getFrameFieldInfoPresentFlag() )
  {
    WRITE_CODE( sei.m_picStruct, 4,              "pic_struct" );
    WRITE_CODE( sei.m_sourceScanType, 2,         "source_scan_type" );
    WRITE_FLAG( sei.m_duplicateFlag ? 1 : 0,     "duplicate_flag" );
  }

  if( hrd->getCpbDpbDelaysPresentFlag() )
  {
    WRITE_CODE( sei.m_auCpbRemovalDelay - 1, ( hrd->getCpbRemovalDelayLengthMinus1() + 1 ),                                         "au_cpb_removal_delay_minus1" );
    WRITE_CODE( sei.m_picDpbOutputDelay, ( hrd->getDpbOutputDelayLengthMinus1() + 1 ),                                          "pic_dpb_output_delay" );
    if(hrd->getSubPicCpbParamsPresentFlag())
    {
      WRITE_CODE(sei.m_picDpbOutputDuDelay, hrd->getDpbOutputDelayDuLengthMinus1()+1, "pic_dpb_output_du_delay" );
    }
    if( hrd->getSubPicCpbParamsPresentFlag() && hrd->getSubPicCpbParamsInPicTimingSEIFlag() )
    {
      WRITE_UVLC( sei.m_numDecodingUnitsMinus1,     "num_decoding_units_minus1" );
      WRITE_FLAG( sei.m_duCommonCpbRemovalDelayFlag, "du_common_cpb_removal_delay_flag" );
      if( sei.m_duCommonCpbRemovalDelayFlag )
      {
        WRITE_CODE( sei.m_duCommonCpbRemovalDelayMinus1, ( hrd->getDuCpbRemovalDelayLengthMinus1() + 1 ),                       "du_common_cpb_removal_delay_minus1" );
      }
      for( i = 0; i <= sei.m_numDecodingUnitsMinus1; i ++ )
      {
        WRITE_UVLC( sei.m_numNalusInDuMinus1[ i ],  "num_nalus_in_du_minus1");
        if( ( !sei.m_duCommonCpbRemovalDelayFlag ) && ( i < sei.m_numDecodingUnitsMinus1 ) )
        {
          WRITE_CODE( sei.m_duCpbRemovalDelayMinus1[ i ], ( hrd->getDuCpbRemovalDelayLengthMinus1() + 1 ),                        "du_cpb_removal_delay_minus1" );
        }
      }
    }
  }
}
Void SEIWriter::xWriteSEIRecoveryPoint(const SEIRecoveryPoint& sei)
{
  WRITE_SVLC( sei.m_recoveryPocCnt,    "recovery_poc_cnt"    );
  WRITE_FLAG( sei.m_exactMatchingFlag, "exact_matching_flag" );
  WRITE_FLAG( sei.m_brokenLinkFlag,    "broken_link_flag"    );
}
Void SEIWriter::xWriteSEIFramePacking(const SEIFramePacking& sei)
{
  WRITE_UVLC( sei.m_arrangementId,                  "frame_packing_arrangement_id" );
  WRITE_FLAG( sei.m_arrangementCancelFlag,          "frame_packing_arrangement_cancel_flag" );

  if( sei.m_arrangementCancelFlag == 0 )
  {
    WRITE_CODE( sei.m_arrangementType, 7,           "frame_packing_arrangement_type" );

    WRITE_FLAG( sei.m_quincunxSamplingFlag,         "quincunx_sampling_flag" );
    WRITE_CODE( sei.m_contentInterpretationType, 6, "content_interpretation_type" );
    WRITE_FLAG( sei.m_spatialFlippingFlag,          "spatial_flipping_flag" );
    WRITE_FLAG( sei.m_frame0FlippedFlag,            "frame0_flipped_flag" );
    WRITE_FLAG( sei.m_fieldViewsFlag,               "field_views_flag" );
    WRITE_FLAG( sei.m_currentFrameIsFrame0Flag,     "current_frame_is_frame0_flag" );

    WRITE_FLAG( sei.m_frame0SelfContainedFlag,      "frame0_self_contained_flag" );
    WRITE_FLAG( sei.m_frame1SelfContainedFlag,      "frame1_self_contained_flag" );

    if(sei.m_quincunxSamplingFlag == 0 && sei.m_arrangementType != 5)
    {
      WRITE_CODE( sei.m_frame0GridPositionX, 4,     "frame0_grid_position_x" );
      WRITE_CODE( sei.m_frame0GridPositionY, 4,     "frame0_grid_position_y" );
      WRITE_CODE( sei.m_frame1GridPositionX, 4,     "frame1_grid_position_x" );
      WRITE_CODE( sei.m_frame1GridPositionY, 4,     "frame1_grid_position_y" );
    }

    WRITE_CODE( sei.m_arrangementReservedByte, 8,   "frame_packing_arrangement_reserved_byte" );
    WRITE_FLAG( sei.m_arrangementPersistenceFlag,   "frame_packing_arrangement_persistence_flag" );
  }

  WRITE_FLAG( sei.m_upsampledAspectRatio,           "upsampled_aspect_ratio" );
}

Void SEIWriter::xWriteSEISegmentedRectFramePacking(const SEISegmentedRectFramePacking& sei)
{
  WRITE_FLAG( sei.m_arrangementCancelFlag,          "segmented_rect_frame_packing_arrangement_cancel_flag" );
  if( sei.m_arrangementCancelFlag == 0 )
  {
    WRITE_CODE( sei.m_contentInterpretationType, 2, "segmented_rect_content_interpretation_type" );
    WRITE_FLAG( sei.m_arrangementPersistenceFlag,   "segmented_rect_frame_packing_arrangement_persistence" );
  }
}

Void SEIWriter::xWriteSEIToneMappingInfo(const SEIToneMappingInfo& sei)
{
  Int i;
  WRITE_UVLC( sei.m_toneMapId,                    "tone_map_id" );
  WRITE_FLAG( sei.m_toneMapCancelFlag,            "tone_map_cancel_flag" );
  if( !sei.m_toneMapCancelFlag )
  {
    WRITE_FLAG( sei.m_toneMapPersistenceFlag,     "tone_map_persistence_flag" );
    WRITE_CODE( sei.m_codedDataBitDepth,    8,    "coded_data_bit_depth" );
    WRITE_CODE( sei.m_targetBitDepth,       8,    "target_bit_depth" );
    WRITE_UVLC( sei.m_modelId,                    "model_id" );
    switch(sei.m_modelId)
    {
    case 0:
      {
        WRITE_CODE( sei.m_minValue,  32,        "min_value" );
        WRITE_CODE( sei.m_maxValue, 32,         "max_value" );
        break;
      }
    case 1:
      {
        WRITE_CODE( sei.m_sigmoidMidpoint, 32,  "sigmoid_midpoint" );
        WRITE_CODE( sei.m_sigmoidWidth,    32,  "sigmoid_width"    );
        break;
      }
    case 2:
      {
        UInt num = 1u << sei.m_targetBitDepth;
        for(i = 0; i < num; i++)
        {
          WRITE_CODE( sei.m_startOfCodedInterval[i], (( sei.m_codedDataBitDepth + 7 ) >> 3 ) << 3,  "start_of_coded_interval" );
        }
        break;
      }
    case 3:
      {
        WRITE_CODE( sei.m_numPivots, 16,          "num_pivots" );
        for(i = 0; i < sei.m_numPivots; i++ )
        {
          WRITE_CODE( sei.m_codedPivotValue[i], (( sei.m_codedDataBitDepth + 7 ) >> 3 ) << 3,       "coded_pivot_value" );
          WRITE_CODE( sei.m_targetPivotValue[i], (( sei.m_targetBitDepth + 7 ) >> 3 ) << 3,         "target_pivot_value");
        }
        break;
      }
    case 4:
      {
        WRITE_CODE( sei.m_cameraIsoSpeedIdc,    8,    "camera_iso_speed_idc" );
        if( sei.m_cameraIsoSpeedIdc == 255) //Extended_ISO
        {
          WRITE_CODE( sei.m_cameraIsoSpeedValue,    32,    "camera_iso_speed_value" );
        }
        WRITE_CODE( sei.m_exposureIndexIdc,     8,    "exposure_index_idc" );
        if( sei.m_exposureIndexIdc == 255) //Extended_ISO
        {
          WRITE_CODE( sei.m_exposureIndexValue,     32,    "exposure_index_value" );
        }
        WRITE_FLAG( sei.m_exposureCompensationValueSignFlag,           "exposure_compensation_value_sign_flag" );
        WRITE_CODE( sei.m_exposureCompensationValueNumerator,     16,  "exposure_compensation_value_numerator" );
        WRITE_CODE( sei.m_exposureCompensationValueDenomIdc,      16,  "exposure_compensation_value_denom_idc" );
        WRITE_CODE( sei.m_refScreenLuminanceWhite,                32,  "ref_screen_luminance_white" );
        WRITE_CODE( sei.m_extendedRangeWhiteLevel,                32,  "extended_range_white_level" );
        WRITE_CODE( sei.m_nominalBlackLevelLumaCodeValue,         16,  "nominal_black_level_luma_code_value" );
        WRITE_CODE( sei.m_nominalWhiteLevelLumaCodeValue,         16,  "nominal_white_level_luma_code_value" );
        WRITE_CODE( sei.m_extendedWhiteLevelLumaCodeValue,        16,  "extended_white_level_luma_code_value" );
        break;
      }
    default:
      {
        assert(!"Undefined SEIToneMapModelId");
        break;
      }
    }//switch m_modelId
  }//if(!sei.m_toneMapCancelFlag)
}

Void SEIWriter::xWriteSEIDisplayOrientation(const SEIDisplayOrientation &sei)
{
  WRITE_FLAG( sei.cancelFlag,           "display_orientation_cancel_flag" );
  if( !sei.cancelFlag )
  {
    WRITE_FLAG( sei.horFlip,                   "hor_flip" );
    WRITE_FLAG( sei.verFlip,                   "ver_flip" );
    WRITE_CODE( sei.anticlockwiseRotation, 16, "anticlockwise_rotation" );
    WRITE_FLAG( sei.persistenceFlag,          "display_orientation_persistence_flag" );
  }
}

Void SEIWriter::xWriteSEITemporalLevel0Index(const SEITemporalLevel0Index &sei)
{
  WRITE_CODE( sei.tl0Idx, 8 , "tl0_idx" );
  WRITE_CODE( sei.rapIdx, 8 , "rap_idx" );
}

Void SEIWriter::xWriteSEIGradualDecodingRefreshInfo(const SEIGradualDecodingRefreshInfo &sei)
{
  WRITE_FLAG( sei.m_gdrForegroundFlag, "gdr_foreground_flag");
}

Void SEIWriter::xWriteSEINoDisplay(const SEINoDisplay& /*sei*/)
{
}

Void SEIWriter::xWriteSEISOPDescription(const SEISOPDescription& sei)
{
  WRITE_UVLC( sei.m_sopSeqParameterSetId,           "sop_seq_parameter_set_id"               );
  WRITE_UVLC( sei.m_numPicsInSopMinus1,             "num_pics_in_sop_minus1"               );
  for (UInt i = 0; i <= sei.m_numPicsInSopMinus1; i++)
  {
    WRITE_CODE( sei.m_sopDescVclNaluType[i], 6, "sop_desc_vcl_nalu_type" );
    WRITE_CODE( sei.m_sopDescTemporalId[i],  3, "sop_desc_temporal_id" );
    if (sei.m_sopDescVclNaluType[i] != NAL_UNIT_CODED_SLICE_IDR_W_RADL && sei.m_sopDescVclNaluType[i] != NAL_UNIT_CODED_SLICE_IDR_N_LP)
    {
      WRITE_UVLC( sei.m_sopDescStRpsIdx[i],           "sop_desc_st_rps_idx"               );
    }
    if (i > 0)
    {
      WRITE_SVLC( sei.m_sopDescPocDelta[i],           "sop_desc_poc_delta"               );
    }
  }
}

Void SEIWriter::xWriteSEIScalableNesting(TComBitIf& bs, const SEIScalableNesting& sei, const TComSPS *sps)
{
  WRITE_FLAG( sei.m_bitStreamSubsetFlag,             "bitstream_subset_flag"         );
  WRITE_FLAG( sei.m_nestingOpFlag,                   "nesting_op_flag      "         );
  if (sei.m_nestingOpFlag)
  {
    WRITE_FLAG( sei.m_defaultOpFlag,                 "default_op_flag"               );
    WRITE_UVLC( sei.m_nestingNumOpsMinus1,           "nesting_num_ops_minus1"        );
    for (UInt i = (sei.m_defaultOpFlag ? 1 : 0); i <= sei.m_nestingNumOpsMinus1; i++)
    {
      WRITE_CODE( sei.m_nestingMaxTemporalIdPlus1[i], 3,  "nesting_max_temporal_id_plus1" );
      WRITE_UVLC( sei.m_nestingOpIdx[i],                  "nesting_op_idx"                );
    }
  }
  else
  {
    WRITE_FLAG( sei.m_allLayersFlag,                      "all_layers_flag"               );
    if (!sei.m_allLayersFlag)
    {
      WRITE_CODE( sei.m_nestingNoOpMaxTemporalIdPlus1, 3, "nesting_no_op_max_temporal_id_plus1" );
      WRITE_UVLC( sei.m_nestingNumLayersMinus1,           "nesting_num_layers"                  );
      for (UInt i = 0; i <= sei.m_nestingNumLayersMinus1; i++)
      {
        WRITE_CODE( sei.m_nestingLayerId[i], 6,           "nesting_layer_id"              );
      }
    }
  }

  // byte alignment
  while ( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0 )
  {
    WRITE_FLAG( 0, "nesting_zero_bit" );
  }

  // write nested SEI messages
  writeSEImessages(bs, sei.m_nestedSEIs, sps, true);
}

Void SEIWriter::xWriteSEITempMotionConstrainedTileSets(const SEITempMotionConstrainedTileSets& sei)
{
  //UInt code;
  WRITE_FLAG((sei.m_mc_all_tiles_exact_sample_value_match_flag ? 1 : 0), "mc_all_tiles_exact_sample_value_match_flag");
  WRITE_FLAG((sei.m_each_tile_one_tile_set_flag                ? 1 : 0), "each_tile_one_tile_set_flag"               );

  if(!sei.m_each_tile_one_tile_set_flag)
  {
    WRITE_FLAG((sei.m_limited_tile_set_display_flag ? 1 : 0), "limited_tile_set_display_flag");
    WRITE_UVLC((sei.getNumberOfTileSets() - 1),               "num_sets_in_message_minus1"   );

    if(sei.getNumberOfTileSets() > 0)
    {
      for(Int i = 0; i < sei.getNumberOfTileSets(); i++)
      {
        WRITE_UVLC(sei.tileSetData(i).m_mcts_id, "mcts_id");

        if(sei.m_limited_tile_set_display_flag)
        {
          WRITE_FLAG((sei.tileSetData(i).m_display_tile_set_flag ? 1 : 0), "display_tile_set_flag");
        }

        WRITE_UVLC((sei.tileSetData(i).getNumberOfTileRects() - 1), "num_tile_rects_in_set_minus1");

        for(Int j = 0; j < sei.tileSetData(i).getNumberOfTileRects(); j++)
        {
          WRITE_UVLC(sei.tileSetData(i).topLeftTileIndex    (j), "top_left_tile_index");
          WRITE_UVLC(sei.tileSetData(i).bottomRightTileIndex(j), "bottom_right_tile_index");
        }

        if(!sei.m_mc_all_tiles_exact_sample_value_match_flag)
        {
          WRITE_FLAG((sei.tileSetData(i).m_exact_sample_value_match_flag ? 1 : 0), "exact_sample_value_match_flag");
        }

        WRITE_FLAG((sei.tileSetData(i).m_mcts_tier_level_idc_present_flag ? 1 : 0), "mcts_tier_level_idc_present_flag");

        if(sei.tileSetData(i).m_mcts_tier_level_idc_present_flag)
        {
          WRITE_FLAG((sei.tileSetData(i).m_mcts_tier_flag ? 1 : 0), "mcts_tier_flag");
          WRITE_CODE( sei.tileSetData(i).m_mcts_level_idc, 8,       "mcts_level_idc");
        }
      }
    }
  }
  else
  {
    WRITE_FLAG((sei.m_max_mcs_tier_level_idc_present_flag ? 1 : 0), "max_mcs_tier_level_idc_present_flag");

    if(sei.m_max_mcs_tier_level_idc_present_flag)
    {
      WRITE_FLAG((sei.m_max_mcts_tier_flag ? 1 : 0), "max_mcts_tier_flag");
      WRITE_CODE( sei.m_max_mcts_level_idc, 8,       "max_mcts_level_idc");
    }
  }
}

Void SEIWriter::xWriteSEITimeCode(const SEITimeCode& sei)
{
  WRITE_CODE(sei.numClockTs, 2, "num_clock_ts");
  for(Int i = 0; i < sei.numClockTs; i++)
  {
    const TComSEITimeSet &currentTimeSet = sei.timeSetArray[i];
    WRITE_FLAG(currentTimeSet.clockTimeStampFlag, "clock_time_stamp_flag");
    if(currentTimeSet.clockTimeStampFlag)
    {
      WRITE_FLAG(currentTimeSet.numUnitFieldBasedFlag, "units_field_based_flag");
      WRITE_CODE(currentTimeSet.countingType, 5, "counting_type");
      WRITE_FLAG(currentTimeSet.fullTimeStampFlag, "full_timestamp_flag");
      WRITE_FLAG(currentTimeSet.discontinuityFlag, "discontinuity_flag");
      WRITE_FLAG(currentTimeSet.cntDroppedFlag, "cnt_dropped_flag");
      WRITE_CODE(currentTimeSet.numberOfFrames, 9, "n_frames");
      if(currentTimeSet.fullTimeStampFlag)
      {
        WRITE_CODE(currentTimeSet.secondsValue, 6, "seconds_value");
        WRITE_CODE(currentTimeSet.minutesValue, 6, "minutes_value");
        WRITE_CODE(currentTimeSet.hoursValue, 5, "hours_value");
      }
      else
      {
        WRITE_FLAG(currentTimeSet.secondsFlag, "seconds_flag");
        if(currentTimeSet.secondsFlag)
        {
          WRITE_CODE(currentTimeSet.secondsValue, 6, "seconds_value");
          WRITE_FLAG(currentTimeSet.minutesFlag, "minutes_flag");
          if(currentTimeSet.minutesFlag)
          {
            WRITE_CODE(currentTimeSet.minutesValue, 6, "minutes_value");
            WRITE_FLAG(currentTimeSet.hoursFlag, "hours_flag");
            if(currentTimeSet.hoursFlag)
            {
              WRITE_CODE(currentTimeSet.hoursValue, 5, "hours_value");
            }
          }
        }
      }
      WRITE_CODE(currentTimeSet.timeOffsetLength, 5, "time_offset_length");
      if(currentTimeSet.timeOffsetLength > 0)
      {
        if(currentTimeSet.timeOffsetValue >= 0)
        {
          WRITE_CODE((UInt)currentTimeSet.timeOffsetValue, currentTimeSet.timeOffsetLength, "time_offset_value");
        }
        else
        {
          //  Two's complement conversion
          UInt offsetValue = ~(currentTimeSet.timeOffsetValue) + 1;
          offsetValue |= (1 << (currentTimeSet.timeOffsetLength-1));
          WRITE_CODE(offsetValue, currentTimeSet.timeOffsetLength, "time_offset_value");
        }
      }
    }
  }
}

Void SEIWriter::xWriteSEIChromaResamplingFilterHint(const SEIChromaResamplingFilterHint &sei)
{
  WRITE_CODE(sei.m_verChromaFilterIdc, 8, "ver_chroma_filter_idc");
  WRITE_CODE(sei.m_horChromaFilterIdc, 8, "hor_chroma_filter_idc");
  WRITE_FLAG(sei.m_verFilteringFieldProcessingFlag, "ver_filtering_field_processing_flag");
  if(sei.m_verChromaFilterIdc == 1 || sei.m_horChromaFilterIdc == 1)
  {
    WRITE_UVLC(sei.m_targetFormatIdc, "target_format_idc");
    if(sei.m_verChromaFilterIdc == 1)
    {
      const Int numVerticalFilter = (Int)sei.m_verFilterCoeff.size();
      WRITE_UVLC(numVerticalFilter, "num_vertical_filters");
      if(numVerticalFilter > 0)
      {
        for(Int i = 0; i < numVerticalFilter; i ++)
        {
          const Int verTapLengthMinus1 = (Int) sei.m_verFilterCoeff[i].size() - 1;
          WRITE_UVLC(verTapLengthMinus1, "ver_tap_length_minus_1");
          for(Int j = 0; j < (verTapLengthMinus1 + 1); j ++)
          {
            WRITE_SVLC(sei.m_verFilterCoeff[i][j], "ver_filter_coeff");
          }
        }
      }
    }
    if(sei.m_horChromaFilterIdc == 1)
    {
      const Int numHorizontalFilter = (Int) sei.m_horFilterCoeff.size();
      WRITE_UVLC(numHorizontalFilter, "num_horizontal_filters");
      if(numHorizontalFilter > 0)
      {
        for(Int i = 0; i < numHorizontalFilter; i ++)
        {
          const Int horTapLengthMinus1 = (Int) sei.m_horFilterCoeff[i].size() - 1;
          WRITE_UVLC(horTapLengthMinus1, "hor_tap_length_minus_1");
          for(Int j = 0; j < (horTapLengthMinus1 + 1); j ++)
          {
            WRITE_SVLC(sei.m_horFilterCoeff[i][j], "hor_filter_coeff");
          }
        }
      }
    }
  }
}

Void SEIWriter::xWriteSEIKneeFunctionInfo(const SEIKneeFunctionInfo &sei)
{
  WRITE_UVLC( sei.m_kneeId, "knee_function_id" );
  WRITE_FLAG( sei.m_kneeCancelFlag, "knee_function_cancel_flag" );
  if ( !sei.m_kneeCancelFlag )
  {
    WRITE_FLAG( sei.m_kneePersistenceFlag, "knee_function_persistence_flag" );
    WRITE_CODE( (UInt)sei.m_kneeInputDrange , 32,  "input_d_range" );
    WRITE_CODE( (UInt)sei.m_kneeInputDispLuminance, 32,  "input_disp_luminance" );
    WRITE_CODE( (UInt)sei.m_kneeOutputDrange, 32,  "output_d_range" );
    WRITE_CODE( (UInt)sei.m_kneeOutputDispLuminance, 32,  "output_disp_luminance" );
    WRITE_UVLC( sei.m_kneeNumKneePointsMinus1, "num_knee_points_minus1" );
    for(Int i = 0; i <= sei.m_kneeNumKneePointsMinus1; i++ )
    {
      WRITE_CODE( (UInt)sei.m_kneeInputKneePoint[i], 10,"input_knee_point" );
      WRITE_CODE( (UInt)sei.m_kneeOutputKneePoint[i], 10, "output_knee_point" );
    }
  }
}

Void SEIWriter::xWriteSEIColourRemappingInfo(const SEIColourRemappingInfo& sei)
{
  WRITE_UVLC( sei.m_colourRemapId,                             "colour_remap_id" );
  WRITE_FLAG( sei.m_colourRemapCancelFlag,                     "colour_remap_cancel_flag" );
  if( !sei.m_colourRemapCancelFlag ) 
  {
    WRITE_FLAG( sei.m_colourRemapPersistenceFlag,              "colour_remap_persistence_flag" );
    WRITE_FLAG( sei.m_colourRemapVideoSignalInfoPresentFlag,   "colour_remap_video_signal_info_present_flag" );
    if ( sei.m_colourRemapVideoSignalInfoPresentFlag )
    {
      WRITE_FLAG( sei.m_colourRemapFullRangeFlag,              "colour_remap_full_range_flag" );
      WRITE_CODE( sei.m_colourRemapPrimaries,               8, "colour_remap_primaries" );
      WRITE_CODE( sei.m_colourRemapTransferFunction,        8, "colour_remap_transfer_function" );
      WRITE_CODE( sei.m_colourRemapMatrixCoefficients,      8, "colour_remap_matrix_coefficients" );
    }
    WRITE_CODE( sei.m_colourRemapInputBitDepth,             8, "colour_remap_input_bit_depth" );
    WRITE_CODE( sei.m_colourRemapBitDepth,                  8, "colour_remap_bit_depth" );
    for( Int c=0 ; c<3 ; c++ )
    {
      WRITE_CODE( sei.m_preLutNumValMinus1[c],              8, "pre_lut_num_val_minus1[c]" );
      if( sei.m_preLutNumValMinus1[c]>0 )
      {
        for( Int i=0 ; i<=sei.m_preLutNumValMinus1[c] ; i++ )
        {
          WRITE_CODE( sei.m_preLut[c][i].codedValue,  (( sei.m_colourRemapInputBitDepth + 7 ) >> 3 ) << 3, "pre_lut_coded_value[c][i]" );
          WRITE_CODE( sei.m_preLut[c][i].targetValue, (( sei.m_colourRemapBitDepth      + 7 ) >> 3 ) << 3, "pre_lut_target_value[c][i]" );
        }
      }
    }
    WRITE_FLAG( sei.m_colourRemapMatrixPresentFlag,            "colour_remap_matrix_present_flag" );
    if( sei.m_colourRemapMatrixPresentFlag )
    {
      WRITE_CODE( sei.m_log2MatrixDenom,                    4, "log2_matrix_denom" );
      for( Int c=0 ; c<3 ; c++ )
      {
        for( Int i=0 ; i<3 ; i++ )
        {
          WRITE_SVLC( sei.m_colourRemapCoeffs[c][i],           "colour_remap_coeffs[c][i]" );
        }
      }
    }

    for( Int c=0 ; c<3 ; c++ )
    {
      WRITE_CODE( sei.m_postLutNumValMinus1[c],             8, "m_postLutNumValMinus1[c]" );
      if( sei.m_postLutNumValMinus1[c]>0 )
      {
        for( Int i=0 ; i<=sei.m_postLutNumValMinus1[c] ; i++ )
        {
          WRITE_CODE( sei.m_postLut[c][i].codedValue, (( sei.m_colourRemapBitDepth + 7 ) >> 3 ) << 3, "post_lut_coded_value[c][i]" );
          WRITE_CODE( sei.m_postLut[c][i].targetValue, (( sei.m_colourRemapBitDepth + 7 ) >> 3 ) << 3, "post_lut_target_value[c][i]" );
        }
      }
    }
  }
}

Void SEIWriter::xWriteSEIMasteringDisplayColourVolume(const SEIMasteringDisplayColourVolume& sei)
{
  WRITE_CODE( sei.values.primaries[0][0],  16,  "display_primaries_x[0]" );
  WRITE_CODE( sei.values.primaries[0][1],  16,  "display_primaries_y[0]" );

  WRITE_CODE( sei.values.primaries[1][0],  16,  "display_primaries_x[1]" );
  WRITE_CODE( sei.values.primaries[1][1],  16,  "display_primaries_y[1]" );

  WRITE_CODE( sei.values.primaries[2][0],  16,  "display_primaries_x[2]" );
  WRITE_CODE( sei.values.primaries[2][1],  16,  "display_primaries_y[2]" );

  WRITE_CODE( sei.values.whitePoint[0],    16,  "white_point_x" );
  WRITE_CODE( sei.values.whitePoint[1],    16,  "white_point_y" );
    
  WRITE_CODE( sei.values.maxLuminance,     32,  "max_display_mastering_luminance" );
  WRITE_CODE( sei.values.minLuminance,     32,  "min_display_mastering_luminance" );
}


Void SEIWriter::xWriteByteAlign()
{
  if( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0)
  {
    WRITE_FLAG( 1, "payload_bit_equal_to_one" );
    while( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0 )
    {
      WRITE_FLAG( 0, "payload_bit_equal_to_zero" );
    }
  }
}

#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
Void SEIWriter::xWriteSEIAlternativeTransferCharacteristics(const SEIAlternativeTransferCharacteristics& sei)
{
  WRITE_CODE(sei.m_preferredTransferCharacteristics, 8, "preferred_transfer_characteristics");
}
#endif

#if NH_MV
Void SEIWriter::xWriteSEILayersNotPresent(const SEILayersNotPresent& sei)
{
  WRITE_CODE( sei.m_lnpSeiActiveVpsId, 4, "lnp_sei_active_vps_id" );
  for( Int i = 0; i  <  sei.m_lnpSeiMaxLayers; i++ )
  {
    WRITE_FLAG( ( sei.m_layerNotPresentFlag[i] ? 1 : 0 ), "layer_not_present_flag" );
  }
};

Void SEIWriter::xWriteSEIInterLayerConstrainedTileSets( const SEIInterLayerConstrainedTileSets& sei)
{
  WRITE_FLAG( ( sei.m_ilAllTilesExactSampleValueMatchFlag ? 1 : 0 ), "il_all_tiles_exact_sample_value_match_flag" );
  WRITE_FLAG( ( sei.m_ilOneTilePerTileSetFlag ? 1 : 0 ),             "il_one_tile_per_tile_set_flag" );
  if( !sei.m_ilOneTilePerTileSetFlag )
  {
    WRITE_UVLC( sei.m_ilNumSetsInMessageMinus1, "il_num_sets_in_message_minus1" );
    if( sei.m_ilNumSetsInMessageMinus1 )
    {
      WRITE_FLAG( ( sei.m_skippedTileSetPresentFlag ? 1 : 0 ), "skipped_tile_set_present_flag" );
    }
    Int numSignificantSets = sei.m_ilNumSetsInMessageMinus1 - sei.m_skippedTileSetPresentFlag + 1;
    for( Int i = 0; i < numSignificantSets; i++ )
    {
      WRITE_UVLC( sei.m_ilctsId[i],                   "ilcts_id" );
      WRITE_UVLC( sei.m_ilNumTileRectsInSetMinus1[i], "il_num_tile_rects_in_set_minus1" );
      for( Int j = 0; j  <= sei.m_ilNumTileRectsInSetMinus1[ i ]; j++ )
      {
        WRITE_UVLC( sei.m_ilTopLeftTileIndex[i][j],     "il_top_left_tile_index" );
        WRITE_UVLC( sei.m_ilBottomRightTileIndex[i][j], "il_bottom_right_tile_index" );
      }
      WRITE_CODE( sei.m_ilcIdc[i], 2, "ilc_idc" );
      if ( !sei.m_ilAllTilesExactSampleValueMatchFlag )
      {
        WRITE_FLAG( ( sei.m_ilExactSampleValueMatchFlag[i] ? 1 : 0 ), "il_exact_sample_value_match_flag" );
      }
    }
  }
  else
  {
    WRITE_CODE( sei.m_allTilesIlcIdc, 2, "all_tiles_ilc_idc" );
  }
};

#if NH_MV_SEI_TBD
Void SEIWriter::xWriteSEIBspNesting( const SEIBspNesting& sei)
{
  WRITE_UVLC( sei.m_seiOlsIdx, "sei_ols_idx" );
  WRITE_UVLC( sei.m_seiPartitioningSchemeIdx, "sei_partitioning_scheme_idx" );
  WRITE_UVLC( sei.m_bspIdx, "bsp_idx" );
  while( !ByteaLigned(() ) );
  {
    WRITE_CODE( sei.m_bspNestingZeroBit, *equalto0*/u1, "bsp_nesting_zero_bit" );
  }
  WRITE_UVLC( sei.m_numSeisInBspMinus1, "num_seis_in_bsp_minus1" );
  for( Int i = 0; i  <=  NumSeisInBspMinus1( ); i++ )
  {
    SeiMessage(() );
  }
};

Void SEIWriter::xWriteSEIBspInitialArrivalTime( const SEIBspInitialArrivalTime& sei)
{
  psIdx = SeiPartitioningSchemeIdx();
  if( nalInitialArrivalDelayPresent )
  {
    for( Int i = 0; i < BspSchedCnt( SeiOlsIdx(), psIdx, MaxTemporalId( 0 ) ); i++ )
    {
      WRITE_CODE( sei.m_nalInitialArrivalDelay[i], getNalInitialArrivalDelayLen ), "nal_initial_arrival_delay" );
    }
  }
  if( vclInitialArrivalDelayPresent )
  {
    for( Int i = 0; i < BspSchedCnt( SeiOlsIdx(), psIdx, MaxTemporalId( 0 ) ); i++ )
    {
      WRITE_CODE( sei.m_vclInitialArrivalDelay[i], getVclInitialArrivalDelayLen ), "vcl_initial_arrival_delay" );
    }
  }
};
#endif
Void SEIWriter::xWriteSEISubBitstreamProperty( const SEISubBitstreamProperty& sei)
{
  WRITE_CODE( sei.m_sbPropertyActiveVpsId, 4,      "sb_property_active_vps_id"         );
  WRITE_UVLC( sei.m_numAdditionalSubStreamsMinus1, "num_additional_sub_streams_minus1" );
  for( Int i = 0; i  <=  sei.m_numAdditionalSubStreamsMinus1; i++ )
  {
    WRITE_CODE( sei.m_subBitstreamMode    [i], 2,  "sub_bitstream_mode"       );
    WRITE_UVLC( sei.m_olsIdxToVps         [i],     "ols_idx_to_vps"           );
    WRITE_CODE( sei.m_highestSublayerId   [i], 3,  "highest_sublayer_id"      );
    WRITE_CODE( sei.m_avgSbPropertyBitRate[i], 16, "avg_sb_property_bit_rate" );
    WRITE_CODE( sei.m_maxSbPropertyBitRate[i], 16, "max_sb_property_bit_rate" );
  }
};

Void SEIWriter::xWriteSEIAlphaChannelInfo( const SEIAlphaChannelInfo& sei)
{
  WRITE_FLAG( ( sei.m_alphaChannelCancelFlag ? 1 : 0 ), "alpha_channel_cancel_flag" );
  if( !sei.m_alphaChannelCancelFlag )
  {
    WRITE_CODE( sei.m_alphaChannelUseIdc, 3, "alpha_channel_use_idc" );
    WRITE_CODE( sei.m_alphaChannelBitDepthMinus8, 3, "alpha_channel_bit_depth_minus8" );
    WRITE_CODE( sei.m_alphaTransparentValue, sei.m_alphaChannelBitDepthMinus8+9, "alpha_transparent_value" );
    WRITE_CODE( sei.m_alphaOpaqueValue, sei.m_alphaChannelBitDepthMinus8+9, "alpha_opaque_value" );
    WRITE_FLAG( ( sei.m_alphaChannelIncrFlag ? 1 : 0 ), "alpha_channel_incr_flag" );
    WRITE_FLAG( ( sei.m_alphaChannelClipFlag ? 1 : 0 ), "alpha_channel_clip_flag" );
    if( sei.m_alphaChannelClipFlag )
    {
      WRITE_FLAG( ( sei.m_alphaChannelClipTypeFlag ? 1 : 0 ), "alpha_channel_clip_type_flag" );
    }
  }
};

Void SEIWriter::xWriteSEIOverlayInfo( const SEIOverlayInfo& sei)
{
  WRITE_FLAG( ( sei.m_overlayInfoCancelFlag ? 1 : 0 ), "overlay_info_cancel_flag" );
  if( !sei.m_overlayInfoCancelFlag )
  {
    WRITE_UVLC( sei.m_overlayContentAuxIdMinus128, "overlay_content_aux_id_minus128" );
    WRITE_UVLC( sei.m_overlayLabelAuxIdMinus128, "overlay_label_aux_id_minus128" );
    WRITE_UVLC( sei.m_overlayAlphaAuxIdMinus128, "overlay_alpha_aux_id_minus128" );
    WRITE_UVLC( sei.m_overlayElementLabelValueLengthMinus8, "overlay_element_label_value_length_minus8" );
    WRITE_UVLC( sei.m_numOverlaysMinus1, "num_overlays_minus1" );
    for( Int i = 0; i  <=  sei.m_numOverlaysMinus1 ; i++ )
    {
      WRITE_UVLC( sei.m_overlayIdx[i], "overlay_idx" );
      WRITE_FLAG( ( sei.m_languageOverlayPresentFlag[i] ? 1 : 0 ), "language_overlay_present_flag" );
      WRITE_CODE( sei.m_overlayContentLayerId[i], 6, "overlay_content_layer_id" );
      WRITE_FLAG( ( sei.m_overlayLabelPresentFlag[i] ? 1 : 0 ), "overlay_label_present_flag" );
      if( sei.m_overlayLabelPresentFlag[i] )
      {
        WRITE_CODE( sei.m_overlayLabelLayerId[i], 6, "overlay_label_layer_id" );
      }
      WRITE_FLAG( ( sei.m_overlayAlphaPresentFlag[i] ? 1 : 0 ), "overlay_alpha_present_flag" );
      if( sei.m_overlayAlphaPresentFlag[i] )
      {
        WRITE_CODE( sei.m_overlayAlphaLayerId[i], 6, "overlay_alpha_layer_id" );
      }
      if( sei.m_overlayLabelPresentFlag[i] )
      {
        WRITE_UVLC( sei.m_numOverlayElementsMinus1[i], "num_overlay_elements_minus1" );
        for( Int j = 0; j  <=  sei.m_numOverlayElementsMinus1[i]; j++ )
        {
          WRITE_CODE( sei.m_overlayElementLabelMin[i][j], (sei.m_overlayElementLabelValueLengthMinus8 + 8), "overlay_element_label_min" );
          WRITE_CODE( sei.m_overlayElementLabelMax[i][j], (sei.m_overlayElementLabelValueLengthMinus8 + 8), "overlay_element_label_max" );
        }
      }
    }

    // byte alignment
    while ( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0 )
    {
      WRITE_FLAG( 0, "overlay_zero_bit" );
    }

    UChar* stmp;
    UInt ilength;
    for( Int i = 0; i  <=  sei.m_numOverlaysMinus1; i++ )
    {
      if( sei.m_languageOverlayPresentFlag[i] )
      {
        stmp = (UChar*) strdup( sei.m_overlayLanguage[i].c_str() );
        ilength = (UInt) sei.m_overlayLanguage[i].size();
        WRITE_STRING( stmp, ilength, "overlay_language" );
        free(stmp);
      }
      stmp = (UChar*) strdup( sei.m_overlayName[i].c_str() );
      ilength = (UInt) sei.m_overlayName[i].size();
      WRITE_STRING( stmp, ilength, "overlay_name" );
      free(stmp);
      if( sei.m_overlayLabelPresentFlag[i] )
      {
        for( Int j = 0; j  <=  sei.m_numOverlayElementsMinus1[i]; j++ )
        {
          stmp = (UChar*) strdup( sei.m_overlayElementName[i][j].c_str() );
          ilength = (UInt) sei.m_overlayElementName[i][j].size();
          WRITE_STRING( stmp, ilength, "overlay_element_name" );
          free(stmp);
        }
      }
    }
    WRITE_FLAG( ( sei.m_overlayInfoPersistenceFlag ? 1 : 0 ), "overlay_info_persistence_flag" );
  }
};


Void SEIWriter::xWriteSEITemporalMvPredictionConstraints( const SEITemporalMvPredictionConstraints& sei)
{
  WRITE_FLAG( ( sei.m_prevPicsNotUsedFlag    ? 1 : 0 ), "prev_pics_not_used_flag"     );
  WRITE_FLAG( ( sei.m_noIntraLayerColPicFlag ? 1 : 0 ), "no_intra_layer_col_pic_flag" );
};

#if NH_MV_SEI_TBD
Void SEIWriter::xWriteSEIFrameFieldInfo( const SEIFrameFieldInfo& sei)
{
  WRITE_CODE( sei.m_ffinfoPicStruct, 4, "ffinfo_pic_struct" );
  WRITE_CODE( sei.m_ffinfoSourceScanType, 2, "ffinfo_source_scan_type" );
  WRITE_FLAG( ( sei.m_ffinfoDuplicateFlag ? 1 : 0 ), "ffinfo_duplicate_flag" );
};
#endif

Void SEIWriter::xWriteSEIThreeDimensionalReferenceDisplaysInfo( const SEIThreeDimensionalReferenceDisplaysInfo& sei)
{
  WRITE_UVLC( sei.m_precRefDisplayWidth, "prec_ref_display_width" );
  WRITE_FLAG( ( sei.m_refViewingDistanceFlag ? 1 : 0 ), "ref_viewing_distance_flag" );
  if( sei.m_refViewingDistanceFlag )
  {
    WRITE_UVLC( sei.m_precRefViewingDist, "prec_ref_viewing_dist" );
  }
  WRITE_UVLC( sei.m_numRefDisplaysMinus1, "num_ref_displays_minus1" );
  for( Int i = 0; i  <=  sei.getNumRefDisplaysMinus1( ); i++ )
  {
    WRITE_UVLC( sei.m_leftViewId[i], "left_view_id" );
    WRITE_UVLC( sei.m_rightViewId[i], "right_view_id" );
    WRITE_CODE( sei.m_exponentRefDisplayWidth[i], 6, "exponent_ref_display_width" );
    WRITE_CODE( sei.m_mantissaRefDisplayWidth[i], sei.getMantissaReferenceDisplayWidthLen(i) , "mantissa_ref_display_width" );
    if( sei.m_refViewingDistanceFlag )
    {
      WRITE_CODE( sei.m_exponentRefViewingDistance[i], 6, "exponent_ref_viewing_distance" );
      WRITE_CODE( sei.m_mantissaRefViewingDistance[i], sei.getMantissaReferenceViewingDistanceLen(i), "mantissa_ref_viewing_distance" );
    }
    WRITE_FLAG( ( sei.m_additionalShiftPresentFlag[i] ? 1 : 0 ), "additional_shift_present_flag" );
    if( sei.m_additionalShiftPresentFlag[i] )
    {
      WRITE_CODE( sei.m_numSampleShiftPlus512[i], 10, "num_sample_shift_plus512" );
    }
  }
  WRITE_FLAG( ( sei.m_threeDimensionalReferenceDisplaysExtensionFlag ? 1 : 0 ), "three_dimensional_reference_displays_extension_flag" );
};

Void SEIWriter::xWriteSEIDepthRepresentationInfo( const SEIDepthRepresentationInfo& sei)
{

    assert(sei.m_currLayerID>=0);

    WRITE_FLAG( ( sei.m_zNearFlag[sei.m_currLayerID] ? 1 : 0 ), "z_near_flag" );
    WRITE_FLAG( ( sei.m_zFarFlag[sei.m_currLayerID] ? 1 : 0 ), "z_far_flag" );
    WRITE_FLAG( ( sei.m_dMinFlag[sei.m_currLayerID] ? 1 : 0 ), "d_min_flag" );
    WRITE_FLAG( ( sei.m_dMaxFlag[sei.m_currLayerID] ? 1 : 0 ), "d_max_flag" );
    WRITE_UVLC( sei.m_depthRepresentationType[sei.m_currLayerID][0], "depth_representation_type" );
    if( sei.m_dMinFlag[sei.m_currLayerID]  ||  sei.m_dMaxFlag[sei.m_currLayerID] )
    {
        WRITE_UVLC( sei.m_disparityRefViewId[sei.m_currLayerID][0], "disparity_ref_view_id" );
    }
    if( sei.m_zNearFlag[sei.m_currLayerID] )
    {
        xWriteSEIDepthRepInfoElement(sei.m_zNear[sei.m_currLayerID][0]);
    }
    if( sei.m_zFarFlag[sei.m_currLayerID] )
    {
        xWriteSEIDepthRepInfoElement(sei.m_zFar[sei.m_currLayerID][0]);
    }
    if( sei.m_dMinFlag[sei.m_currLayerID] )
    {
        xWriteSEIDepthRepInfoElement(sei.m_dMin[sei.m_currLayerID][0]);
    }
    if( sei.m_dMaxFlag[sei.m_currLayerID] )
    {
        xWriteSEIDepthRepInfoElement(sei.m_dMax[sei.m_currLayerID][0]);
    }

    if (sei.m_depthRepresentationType[sei.m_currLayerID][0] == 3)
    {
        WRITE_UVLC( sei.m_depthNonlinearRepresentationNumMinus1[sei.m_currLayerID][0], "depth_nonlinear_representation_num_minus1" );
        for( Int i = 1; i  <=  sei.m_depthNonlinearRepresentationNumMinus1[sei.m_currLayerID][0] + 1; i++ )
        {
            WRITE_UVLC(sei.m_depth_nonlinear_representation_model[sei.m_currLayerID][i-1],"depth_nonlinear_representation_model[ i ]");
        }
    }
}

Void SEIWriter::xWriteSEIDepthRepInfoElement( double f)
{
    UInt x_sign, x_exp, x_mantissa,x_mantissa_len;
    if (f < 0)
    {
        f = f * (-1);
        x_sign = 1;
    }
    else
    {
        x_sign = 0;
    }
    int exponent=0;
    if(f >= 1)
    {
        while(f>=2)
        {
            exponent++;
            f = f/2;
        }
    }
    else
    {
        while (f<1)
        {
            exponent++;
            f = f*2;
        }
        exponent=-exponent;
    }

    int i;
    f = f -1;
    double s = 1;
    char s_mantissa[32];
    double thr=1.0/(4.0*(1<<30));

    if (f>=thr)
    {
        for(i=0;i<32;i++)
        {
            s /= 2;
            if(f>=s)
            {
                f = f-s;
                s_mantissa[i]=1;

                if (f<thr)
                    break;
            }
            else
            {
                s_mantissa[i]=0;
            }
        }

        if (i<32)
            x_mantissa_len=i+1;
        else
            x_mantissa_len=32;

        x_mantissa=0;

        for(i=0;i<x_mantissa_len;i++)
        {
            if (s_mantissa[i]==1)
                x_mantissa += (1u)<<(x_mantissa_len-1-i) ;
        }

    }
    else
    {
        x_mantissa=0;
        x_mantissa_len=1;
    }

    assert(exponent>=-31 && exponent<= (1<<7)-32);
    x_exp=exponent+31;

    WRITE_FLAG(  x_sign,                         "da_sign_flag" );
    WRITE_CODE(  x_exp, 7 ,                      "da_exponent" );
    WRITE_CODE( x_mantissa_len-1, 5 ,            "da_mantissa_len_minus1" );
    WRITE_CODE( x_mantissa, x_mantissa_len ,     "da_mantissa" );

};

Void SEIWriter::xWriteSEIMultiviewSceneInfo( const SEIMultiviewSceneInfo& sei)
{
  WRITE_SVLC( sei.m_minDisparity     , "min_disparity"       );
  WRITE_UVLC( sei.m_maxDisparityRange, "max_disparity_range" );
};


Void SEIWriter::xWriteSEIMultiviewAcquisitionInfo( const SEIMultiviewAcquisitionInfo& sei)
{
  WRITE_FLAG( ( sei.m_intrinsicParamFlag ? 1 : 0 ), "intrinsic_param_flag" );
  WRITE_FLAG( ( sei.m_extrinsicParamFlag ? 1 : 0 ), "extrinsic_param_flag" );
  if( sei.m_intrinsicParamFlag )
  {
    WRITE_FLAG( ( sei.m_intrinsicParamsEqualFlag ? 1 : 0 ), "intrinsic_params_equal_flag" );
    WRITE_UVLC(   sei.m_precFocalLength                   , "prec_focal_length"           );
    WRITE_UVLC(   sei.m_precPrincipalPoint                , "prec_principal_point"        );
    WRITE_UVLC(   sei.m_precSkewFactor                    , "prec_skew_factor"            );

    for( Int i = 0; i  <=  ( sei.m_intrinsicParamsEqualFlag ? 0 : sei.getNumViewsMinus1() ); i++ )
    {
      WRITE_FLAG( ( sei.m_signFocalLengthX       [i] ? 1 : 0 ),                                         "sign_focal_length_x"        );
      WRITE_CODE(   sei.m_exponentFocalLengthX   [i]          , 6                                  ,    "exponent_focal_length_x"    );
      WRITE_CODE(   sei.m_mantissaFocalLengthX   [i]          , sei.getMantissaFocalLengthXLen( i ),    "mantissa_focal_length_x"    );
      WRITE_FLAG( ( sei.m_signFocalLengthY       [i] ? 1 : 0 ),                                         "sign_focal_length_y"        );
      WRITE_CODE(   sei.m_exponentFocalLengthY   [i]          , 6                                  ,    "exponent_focal_length_y"    );
      WRITE_CODE(   sei.m_mantissaFocalLengthY   [i]          , sei.getMantissaFocalLengthYLen( i ),    "mantissa_focal_length_y"    );
      WRITE_FLAG( ( sei.m_signPrincipalPointX    [i] ? 1 : 0 ),                                         "sign_principal_point_x"     );
      WRITE_CODE(   sei.m_exponentPrincipalPointX[i]          , 6,                                      "exponent_principal_point_x" );
      WRITE_CODE(   sei.m_mantissaPrincipalPointX[i]          , sei.getMantissaPrincipalPointXLen( i ), "mantissa_principal_point_x" );
      WRITE_FLAG( ( sei.m_signPrincipalPointY    [i] ? 1 : 0 ),                                         "sign_principal_point_y"     );
      WRITE_CODE(   sei.m_exponentPrincipalPointY[i]          , 6,                                      "exponent_principal_point_y" );
      WRITE_CODE(   sei.m_mantissaPrincipalPointY[i]          , sei.getMantissaPrincipalPointYLen( i ), "mantissa_principal_point_y" );
      WRITE_FLAG( ( sei.m_signSkewFactor         [i] ? 1 : 0 ),                                         "sign_skew_factor"           );
      WRITE_CODE(   sei.m_exponentSkewFactor     [i]          , 6,                                      "exponent_skew_factor"       );
      WRITE_CODE(   sei.m_mantissaSkewFactor     [i]          , sei.getMantissaSkewFactorLen( i )  ,    "mantissa_skew_factor"       );
    }
  }
  if( sei.m_extrinsicParamFlag )
  {
    WRITE_UVLC( sei.m_precRotationParam   , "prec_rotation_param"    );
    WRITE_UVLC( sei.m_precTranslationParam, "prec_translation_param" );
    for( Int i = 0; i  <=  sei.getNumViewsMinus1(); i++ )
    {
      for( Int j = 0; j  <=  2; j++ )  /* row */
      {
        for( Int k = 0; k  <=  2; k++ )  /* column */
        {
          WRITE_FLAG( ( sei.m_signR    [i][j][k] ? 1 : 0 ),                                "sign_r"     );
          WRITE_CODE(   sei.m_exponentR[i][j][k]          , 6,                             "exponent_r" );
          WRITE_CODE(   sei.m_mantissaR[i][j][k]          , sei.getMantissaRLen( i,j,k ) , "mantissa_r" );
        }
        WRITE_FLAG( ( sei.m_signT    [i][j] ? 1 : 0 ),                          "sign_t"     );
        WRITE_CODE(   sei.m_exponentT[i][j]          , 6,                       "exponent_t" );
        WRITE_CODE(   sei.m_mantissaT[i][j]          , sei.getMantissaTLen( i,j ),"mantissa_t" );
      }
    }
  }
};

Void SEIWriter::xWriteSEIMultiviewViewPosition( const SEIMultiviewViewPosition& sei)
{
  WRITE_UVLC( sei.m_numViewsMinus1, "num_views_minus1" );
  for( Int i = 0; i  <=  sei.m_numViewsMinus1; i++ )
  {
    WRITE_UVLC( sei.m_viewPosition[i], "view_position" );
  }
};

#if NH_3D
Void SEIWriter::xWriteSEIAlternativeDepthInfo( const SEIAlternativeDepthInfo& sei)
{
  WRITE_FLAG( ( sei.m_alternativeDepthInfoCancelFlag ? 1 : 0 ), "alternative_depth_info_cancel_flag" );
  if( sei.m_alternativeDepthInfoCancelFlag  ==  0 )
  {
    WRITE_CODE( sei.m_depthType, 2, "depth_type" );

    if( sei.m_depthType  ==  0 )
    {
      WRITE_UVLC( sei.m_numConstituentViewsGvdMinus1, "num_constituent_views_gvd_minus1" );
      WRITE_FLAG( ( sei.m_depthPresentGvdFlag ? 1 : 0 ), "depth_present_gvd_flag" );
      WRITE_FLAG( ( sei.m_zGvdFlag ? 1 : 0 ), "z_gvd_flag" );
      WRITE_FLAG( ( sei.m_intrinsicParamGvdFlag ? 1 : 0 ), "intrinsic_param_gvd_flag" );
      WRITE_FLAG( ( sei.m_rotationGvdFlag ? 1 : 0 ), "rotation_gvd_flag" );
      WRITE_FLAG( ( sei.m_translationGvdFlag ? 1 : 0 ), "translation_gvd_flag" );
      if( sei.m_zGvdFlag )
      {
        for( Int i = 0, j = 0; j  <=  sei.m_numConstituentViewsGvdMinus1 + 1; j++ )
        {
          WRITE_FLAG( ( sei.m_signGvdZNearFlag[i][j] ? 1 : 0 ), "sign_gvd_z_near_flag" );
          WRITE_CODE( sei.m_expGvdZNear[i][j], 7, "exp_gvd_z_near" );
          WRITE_CODE( sei.m_manLenGvdZNearMinus1[i][j], 5, "man_len_gvd_z_near_minus1" );
          WRITE_CODE( sei.m_manGvdZNear[i][j], sei.m_manLenGvdZNearMinus1[i][j] + 1, "man_gvd_z_near" );
          WRITE_FLAG( ( sei.m_signGvdZFarFlag[i][j] ? 1 : 0 ), "sign_gvd_z_far_flag" );
          WRITE_CODE( sei.m_expGvdZFar[i][j], 7, "exp_gvd_z_far" );
          WRITE_CODE( sei.m_manLenGvdZFarMinus1[i][j], 5, "man_len_gvd_z_far_minus1" );
          WRITE_CODE( sei.m_manGvdZFar[i][j], sei.m_manLenGvdZFarMinus1[i][j] + 1, "man_gvd_z_far" );
        }
      }
      if( sei.m_intrinsicParamGvdFlag )
      {
        WRITE_UVLC( sei.m_precGvdFocalLength, "prec_gvd_focal_length" );
        WRITE_UVLC( sei.m_precGvdPrincipalPoint, "prec_gvd_principal_point" );
      }
      if( sei.m_rotationGvdFlag )
      {
        WRITE_UVLC( sei.m_precGvdRotationParam, "prec_gvd_rotation_param" );
      }
      if( sei.m_translationGvdFlag )
      {
        WRITE_UVLC( sei.m_precGvdTranslationParam, "prec_gvd_translation_param" );
      }
      for( Int i = 0, j = 0; j  <=  sei.m_numConstituentViewsGvdMinus1 + 1; j++ )
      {
        if( sei.m_intrinsicParamGvdFlag )
        {
          WRITE_FLAG( ( sei.m_signGvdFocalLengthX[i][j] ? 1 : 0 ), "sign_gvd_focal_length_x" );
          WRITE_CODE( sei.m_expGvdFocalLengthX[i][j], 6, "exp_gvd_focal_length_x" );
          WRITE_CODE( sei.m_manGvdFocalLengthX[i][j], sei.getManGvdFocalLengthXLen(i,j), "man_gvd_focal_length_x" );
          WRITE_FLAG( ( sei.m_signGvdFocalLengthY[i][j] ? 1 : 0 ), "sign_gvd_focal_length_y" );
          WRITE_CODE( sei.m_expGvdFocalLengthY[i][j], 6, "exp_gvd_focal_length_y" );
          WRITE_CODE( sei.m_manGvdFocalLengthY[i][j], sei.getManGvdFocalLengthYLen(i,j), "man_gvd_focal_length_y" );
          WRITE_FLAG( ( sei.m_signGvdPrincipalPointX[i][j] ? 1 : 0 ), "sign_gvd_principal_point_x" );
          WRITE_CODE( sei.m_expGvdPrincipalPointX[i][j], 6, "exp_gvd_principal_point_x" );
          WRITE_CODE( sei.m_manGvdPrincipalPointX[i][j], sei.getManGvdPrincipalPointXLen(i,j), "man_gvd_principal_point_x" );
          WRITE_FLAG( ( sei.m_signGvdPrincipalPointY[i][j] ? 1 : 0 ), "sign_gvd_principal_point_y" );
          WRITE_CODE( sei.m_expGvdPrincipalPointY[i][j], 6, "exp_gvd_principal_point_y" );
          WRITE_CODE( sei.m_manGvdPrincipalPointY[i][j], sei.getManGvdPrincipalPointYLen(i,j), "man_gvd_principal_point_y" );
        }
        if( sei.m_rotationGvdFlag )
        {
          WRITE_FLAG( ( sei.m_signGvdR00[i][j] ? 1 : 0 ), "sign_gvd_r00" );
          WRITE_CODE( sei.m_expGvdR00[i][j], 6, "exp_gvd_r00" );
          WRITE_CODE( sei.m_manGvdR00[i][j], sei.m_precGvdRotationParam, "man_gvd_r00" );
          WRITE_FLAG( ( sei.m_signGvdR01[i][j] ? 1 : 0 ), "sign_gvd_r01" );
          WRITE_CODE( sei.m_expGvdR01[i][j], 6, "exp_gvd_r01" );
          WRITE_CODE( sei.m_manGvdR01[i][j], sei.m_precGvdRotationParam, "man_gvd_r01" );
          WRITE_FLAG( ( sei.m_signGvdR02[i][j] ? 1 : 0 ), "sign_gvd_r02" );
          WRITE_CODE( sei.m_expGvdR02[i][j], 6, "exp_gvd_r02" );
          WRITE_CODE( sei.m_manGvdR02[i][j], sei.m_precGvdRotationParam, "man_gvd_r02" );
          WRITE_FLAG( ( sei.m_signGvdR10[i][j] ? 1 : 0 ), "sign_gvd_r10" );
          WRITE_CODE( sei.m_expGvdR10[i][j], 6, "exp_gvd_r10" );
          WRITE_CODE( sei.m_manGvdR10[i][j], sei.m_precGvdRotationParam, "man_gvd_r10" );
          WRITE_FLAG( ( sei.m_signGvdR11[i][j] ? 1 : 0 ), "sign_gvd_r11" );
          WRITE_CODE( sei.m_expGvdR11[i][j], 6, "exp_gvd_r11" );
          WRITE_CODE( sei.m_manGvdR11[i][j], sei.m_precGvdRotationParam, "man_gvd_r11" );
          WRITE_FLAG( ( sei.m_signGvdR12[i][j] ? 1 : 0 ), "sign_gvd_r12" );
          WRITE_CODE( sei.m_expGvdR12[i][j], 6, "exp_gvd_r12" );
          WRITE_CODE( sei.m_manGvdR12[i][j], sei.m_precGvdRotationParam, "man_gvd_r12" );
          WRITE_FLAG( ( sei.m_signGvdR20[i][j] ? 1 : 0 ), "sign_gvd_r20" );
          WRITE_CODE( sei.m_expGvdR20[i][j], 6, "exp_gvd_r20" );
          WRITE_CODE( sei.m_manGvdR20[i][j], sei.m_precGvdRotationParam, "man_gvd_r20" );
          WRITE_FLAG( ( sei.m_signGvdR21[i][j] ? 1 : 0 ), "sign_gvd_r21" );
          WRITE_CODE( sei.m_expGvdR21[i][j], 6, "exp_gvd_r21" );
          WRITE_CODE( sei.m_manGvdR21[i][j], sei.m_precGvdRotationParam, "man_gvd_r21" );
          WRITE_FLAG( ( sei.m_signGvdR22[i][j] ? 1 : 0 ), "sign_gvd_r22" );
          WRITE_CODE( sei.m_expGvdR22[i][j], 6, "exp_gvd_r22" );
          WRITE_CODE( sei.m_manGvdR22[i][j], sei.m_precGvdRotationParam, "man_gvd_r22" );
        }
        if( sei.m_translationGvdFlag )
        {
          WRITE_FLAG( ( sei.m_signGvdTX[i][j] ? 1 : 0 ), "sign_gvd_t_x" );
          WRITE_CODE( sei.m_expGvdTX[i][j], 6, "exp_gvd_t_x" );
          WRITE_CODE( sei.m_manGvdTX[i][j], sei.getManGvdTXLen(i,j), "man_gvd_t_x" );
        }
      }
    }

    if( sei.m_depthType  ==  1 )
    {
      WRITE_SVLC( sei.m_minOffsetXInt, "min_offset_x_int" );
      WRITE_CODE( sei.m_minOffsetXFrac, 8, "min_offset_x_frac" );
      WRITE_SVLC( sei.m_maxOffsetXInt, "max_offset_x_int" );
      WRITE_CODE( sei.m_maxOffsetXFrac, 8, "max_offset_x_frac" );
      WRITE_FLAG( ( sei.m_offsetYPresentFlag ? 1 : 0 ), "offset_y_present_flag" );
      if( sei.m_offsetYPresentFlag )
      {
        WRITE_SVLC( sei.m_minOffsetYInt, "min_offset_y_int" );
        WRITE_CODE( sei.m_minOffsetYFrac, 8, "min_offset_y_frac" );
        WRITE_SVLC( sei.m_maxOffsetYInt, "max_offset_y_int" );
        WRITE_CODE( sei.m_maxOffsetYFrac, 8, "max_offset_y_frac" );
      }
      WRITE_FLAG( ( sei.m_warpMapSizePresentFlag ? 1 : 0 ), "warp_map_size_present_flag" );
      if( sei.m_warpMapSizePresentFlag )
      {
        WRITE_UVLC( sei.m_warpMapWidthMinus2, "warp_map_width_minus2" );
        WRITE_UVLC( sei.m_warpMapHeightMinus2, "warp_map_height_minus2" );
      }
    }
  }
};
#endif
#endif

//! \}
