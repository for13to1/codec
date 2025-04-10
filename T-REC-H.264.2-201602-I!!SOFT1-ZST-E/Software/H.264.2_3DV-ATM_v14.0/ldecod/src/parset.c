/*
 * Disclaimer of Warranty
 *
 * Copyright 2001-2015, International Telecommunication Union, Geneva
 *
 * These software programs are available to the user without any
 * license fee or royalty on an "as is" basis. The ITU disclaims
 * any and all warranties, whether express, implied, or statutory,
 * including any implied warranties of merchantability or of fitness
 * for a particular purpose.  In no event shall the ITU be liable for
 * any incidental, punitive, or consequential damages of any kind
 * whatsoever arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs
 * and the user's customers, employees, agents, transferees,
 * successors, and assigns.
 *
 * The ITU does not represent or warrant that the programs furnished
 * hereunder are free of infringement of any third-party patents.
 * Commercial implementations of ITU-T Recommendations, including
 * shareware, may be subject to royalty fees to patent holders.
 * Information regarding the ITU-T patent policy is available from the
 * ITU web site at http://www.itu.int.
 *
 * THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.
 *
 */



/*!
 ************************************************************************
 *  \file
 *     parset.c
 *  \brief
 *     Parameter Sets
 *  \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Stephan Wenger          <stewe@cs.tu-berlin.de>
 *
 ***********************************************************************
 */



#include "global.h"

#include "image.h"

#include "parsetcommon.h"
#include "parset.h"
#include "nalu.h"
#include "memalloc.h"
#include "fmo.h"
#include "cabac.h"
#include "vlc.h"
#include "mbuffer.h"
#include "erc_api.h"
#if EXT3D
#include "nonlinear_depth.h"
#include "3dv_acquisition.h"

extern int g_bFound_dec;
extern int g_NonlinearDepthNum_dec;
extern char g_NonlinearDepthPoints_dec[256];    
#endif

#if TRACE
#define SYMTRACESTRING(s) strncpy(sym->tracestring,s,TRACESTRING_SIZE)
#else
#define SYMTRACESTRING(s) // do nothing
#endif


extern void init_frext(VideoParameters *p_Vid);


// syntax for scaling list matrix values
void Scaling_List(int *scalingList, int sizeOfScalingList, Boolean *UseDefaultScalingMatrix, Bitstream *s)
{
  int j, scanj;
  int delta_scale, lastScale, nextScale;

  lastScale      = 8;
  nextScale      = 8;

  for(j=0; j<sizeOfScalingList; j++)
  {
    scanj = (sizeOfScalingList==16) ? ZZ_SCAN[j]:ZZ_SCAN8[j];

    if(nextScale!=0)
    {
      delta_scale = se_v (   "   : delta_sl   "                           , s);
      nextScale = (lastScale + delta_scale + 256) % 256;
      *UseDefaultScalingMatrix = (Boolean) (scanj==0 && nextScale==0);
    }

    scalingList[scanj] = (nextScale==0) ? lastScale:nextScale;
    lastScale = scalingList[scanj];
  }
}
// fill sps with content of p

int InterpretSPS (VideoParameters *p_Vid, DataPartition *p, seq_parameter_set_rbsp_t *sps)
{
  unsigned i;
  unsigned n_ScalingList;
  int reserved_zero;
  Bitstream *s = p->bitstream;

  assert (p != NULL);
  assert (p->bitstream != NULL);
  assert (p->bitstream->streamBuffer != 0);
  assert (sps != NULL);

  p_Dec->UsedBits = 0;

  sps->profile_idc                            = u_v  (8, "SPS: profile_idc"                           , s);

  if ((sps->profile_idc!=BASELINE       ) &&
      (sps->profile_idc!=MAIN           ) &&
      (sps->profile_idc!=EXTENDED       ) &&
      (sps->profile_idc!=FREXT_HP       ) &&
      (sps->profile_idc!=FREXT_Hi10P    ) &&
      (sps->profile_idc!=FREXT_Hi422    ) &&
      (sps->profile_idc!=FREXT_Hi444    ) &&
      (sps->profile_idc!=FREXT_CAVLC444 )
#if EXT3D
      && (sps->profile_idc!=MVC_HIGH)
      && (sps->profile_idc!=STEREO_HIGH)
      && (sps->profile_idc!=ThreeDV_HIGH)
      &&(sps->profile_idc!=ThreeDV_EXTEND_HIGH)
#else
#if (MVC_EXTENSION_ENABLE)
      && (sps->profile_idc!=MVC_HIGH)
      && (sps->profile_idc!=STEREO_HIGH)
#endif
#endif
      )
  {
    printf("Invalid Profile IDC (%d) encountered. \n", sps->profile_idc);
    return p_Dec->UsedBits;
  }

  sps->constrained_set0_flag                  = u_1  (   "SPS: constrained_set0_flag"                 , s);
  sps->constrained_set1_flag                  = u_1  (   "SPS: constrained_set1_flag"                 , s);
  sps->constrained_set2_flag                  = u_1  (   "SPS: constrained_set2_flag"                 , s);
  sps->constrained_set3_flag                  = u_1  (   "SPS: constrained_set3_flag"                 , s);
#if MVC_EXTENSION_ENABLE||EXT3D
  sps->constrained_set4_flag                  = u_1  (   "SPS: constrained_set4_flag"                 , s);
  reserved_zero                               = u_v  (3, "SPS: reserved_zero_3bits"                   , s);
#else
  reserved_zero                               = u_v  (4, "SPS: reserved_zero_4bits"                   , s);
#endif

#if EXT3D
  if (reserved_zero != 0)
     assert(0);
#else
  assert (reserved_zero==0);
#endif

  sps->level_idc                              = u_v  (8, "SPS: level_idc"                             , s);

  sps->seq_parameter_set_id                   = ue_v ("SPS: seq_parameter_set_id"                     , s);

  // Fidelity Range Extensions stuff
#if EXT3D
  sps->chroma_format_idc = (sps->profile_idc==ThreeDV_HIGH)? 0 : 1;
#else
  sps->chroma_format_idc = 1;
#endif
  sps->bit_depth_luma_minus8   = 0;
  sps->bit_depth_chroma_minus8 = 0;
  p_Vid->lossless_qpprime_flag   = 0;
  sps->separate_colour_plane_flag = 0;

  if((sps->profile_idc==FREXT_HP   ) ||
     (sps->profile_idc==FREXT_Hi10P) ||
     (sps->profile_idc==FREXT_Hi422) ||
     (sps->profile_idc==FREXT_Hi444) ||
     (sps->profile_idc==FREXT_CAVLC444)
#if EXT3D
     || (sps->profile_idc==MVC_HIGH)
     || (sps->profile_idc==STEREO_HIGH)
     || (sps->profile_idc==ThreeDV_HIGH)
     || (sps->profile_idc==ThreeDV_EXTEND_HIGH)
#else
#if (MVC_EXTENSION_ENABLE)
     || (sps->profile_idc==MVC_HIGH)
     || (sps->profile_idc==STEREO_HIGH)
#endif
#endif
     )
  {
    sps->chroma_format_idc                      = ue_v ("SPS: chroma_format_idc"                       , s);

    if(sps->chroma_format_idc == YUV444)
    {
      sps->separate_colour_plane_flag           = u_1  ("SPS: separate_colour_plane_flag"              , s);
    }

    sps->bit_depth_luma_minus8                  = ue_v ("SPS: bit_depth_luma_minus8"                   , s);
    sps->bit_depth_chroma_minus8                = ue_v ("SPS: bit_depth_chroma_minus8"                 , s);
    //checking;
    if((sps->bit_depth_luma_minus8+8 > sizeof(imgpel)*8) || (sps->bit_depth_chroma_minus8+8> sizeof(imgpel)*8))
      error ("Source picture has higher bit depth than imgpel data type. \nPlease recompile with larger data type for imgpel.", 500);

    p_Vid->lossless_qpprime_flag                  = u_1  ("SPS: lossless_qpprime_y_zero_flag"            , s);

    sps->seq_scaling_matrix_present_flag        = u_1  (   "SPS: seq_scaling_matrix_present_flag"       , s);

    if(sps->seq_scaling_matrix_present_flag)
    {
      n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
      for(i=0; i<n_ScalingList; i++)
      {
        sps->seq_scaling_list_present_flag[i]   = u_1  (   "SPS: seq_scaling_list_present_flag"         , s);
        if(sps->seq_scaling_list_present_flag[i])
        {
          if(i<6)
            Scaling_List(sps->ScalingList4x4[i], 16, &sps->UseDefaultScalingMatrix4x4Flag[i], s);
          else
            Scaling_List(sps->ScalingList8x8[i-6], 64, &sps->UseDefaultScalingMatrix8x8Flag[i-6], s);
        }
      }
    }
  }

  sps->log2_max_frame_num_minus4              = ue_v ("SPS: log2_max_frame_num_minus4"                , s);
  sps->pic_order_cnt_type                     = ue_v ("SPS: pic_order_cnt_type"                       , s);

  if (sps->pic_order_cnt_type == 0)
    sps->log2_max_pic_order_cnt_lsb_minus4 = ue_v ("SPS: log2_max_pic_order_cnt_lsb_minus4"           , s);
  else if (sps->pic_order_cnt_type == 1)
  {
    sps->delta_pic_order_always_zero_flag      = u_1  ("SPS: delta_pic_order_always_zero_flag"       , s);
    sps->offset_for_non_ref_pic                = se_v ("SPS: offset_for_non_ref_pic"                 , s);
    sps->offset_for_top_to_bottom_field        = se_v ("SPS: offset_for_top_to_bottom_field"         , s);
    sps->num_ref_frames_in_pic_order_cnt_cycle = ue_v ("SPS: num_ref_frames_in_pic_order_cnt_cycle"  , s);
    for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
      sps->offset_for_ref_frame[i]               = se_v ("SPS: offset_for_ref_frame[i]"              , s);
  }
  sps->num_ref_frames                        = ue_v ("SPS: num_ref_frames"                         , s);
  sps->gaps_in_frame_num_value_allowed_flag  = u_1  ("SPS: gaps_in_frame_num_value_allowed_flag"   , s);
  sps->pic_width_in_mbs_minus1               = ue_v ("SPS: pic_width_in_mbs_minus1"                , s);
  sps->pic_height_in_map_units_minus1        = ue_v ("SPS: pic_height_in_map_units_minus1"         , s);
  sps->frame_mbs_only_flag                   = u_1  ("SPS: frame_mbs_only_flag"                    , s);
  if (!sps->frame_mbs_only_flag)
  {
    sps->mb_adaptive_frame_field_flag        = u_1  ("SPS: mb_adaptive_frame_field_flag"           , s);
  }
  sps->direct_8x8_inference_flag             = u_1  ("SPS: direct_8x8_inference_flag"              , s);
  sps->frame_cropping_flag                   = u_1  ("SPS: frame_cropping_flag"                    , s);

  if (sps->frame_cropping_flag)
  {
    sps->frame_cropping_rect_left_offset      = ue_v ("SPS: frame_cropping_rect_left_offset"           , s);
    sps->frame_cropping_rect_right_offset     = ue_v ("SPS: frame_cropping_rect_right_offset"          , s);
    sps->frame_cropping_rect_top_offset       = ue_v ("SPS: frame_cropping_rect_top_offset"            , s);
    sps->frame_cropping_rect_bottom_offset    = ue_v ("SPS: frame_cropping_rect_bottom_offset"         , s);
  }
  sps->vui_parameters_present_flag           = (Boolean) u_1  ("SPS: vui_parameters_present_flag"      , s);

  InitVUI(sps);
  ReadVUI(p, sps);

  sps->Valid = TRUE;

  return p_Dec->UsedBits;
}

#if EXT3D
int InterpretDPS (VideoParameters *p_Vid, DataPartition *p, depth_parameter_set_rbsp_t *dps)
{
  int vsp_flag;
  Bitstream *s = p->bitstream;

  ThreeDVAcquisitionInfo* threeDV_acquisition_info = dps->acquisition_info;
  ThreeDVAcquisitionInfo* subsps_3dv_acquisition_info=p_Vid->DepParSet[0]->acquisition_info;
  ThreeDVAcquisitionInfo* forward_3dv_acquisition_info=threeDV_acquisition_info;
  ThreeDVAcquisitionInfo* backward_3dv_acquisition_info=threeDV_acquisition_info;

  int k;

  assert (p != NULL);
  assert (p->bitstream != NULL);
  assert (p->bitstream->streamBuffer != 0);
  assert (dps != NULL);

  p_Dec->UsedBits = 0;
  dps->dep_parameter_set_id = ue_v  ("DPS: dep_parameter_set_id"                           , s);
  if(dps->dep_parameter_set_id==2)
  {
    dps->dep_parameter_set_id = dps->dep_parameter_set_id;
  }
  threeDV_acquisition_info->pred_direction=u_v(2,"3dv_acquisition_info:pre_direcion",s);
  if(threeDV_acquisition_info->pred_direction==0 || threeDV_acquisition_info->pred_direction==1)
  {
    threeDV_acquisition_info->ref_element_id0=ue_v("3dv_acquisition_info:ref_element_id0",s);
    threeDV_acquisition_info->pred_weight0=64;
    if(threeDV_acquisition_info->ref_element_id0==0)
      forward_3dv_acquisition_info = subsps_3dv_acquisition_info;
    else
      forward_3dv_acquisition_info = p_Vid->DepParSet[threeDV_acquisition_info->ref_element_id0]->acquisition_info;
  }
  if(threeDV_acquisition_info->pred_direction==0)
  {
    threeDV_acquisition_info->ref_element_id1=ue_v("3dv_acquisition_info:ref_element_id1",s);
    threeDV_acquisition_info->pred_weight0=u_v(6,"3dv_acquisition_info:pred_weight0",s);
    if(threeDV_acquisition_info->ref_element_id0==0)
      backward_3dv_acquisition_info = subsps_3dv_acquisition_info;
    else
      backward_3dv_acquisition_info = p_Vid->DepParSet[threeDV_acquisition_info->ref_element_id1]->acquisition_info;
  }
  dps->num_views_minus1 = ue_v  ("DPS: num_views_minus1"                           , s);
  dps->acquisition_info->num_views = dps->num_views_minus1 + 1;
  decode_acquisition_info(threeDV_acquisition_info, subsps_3dv_acquisition_info, forward_3dv_acquisition_info, backward_3dv_acquisition_info, s, 0);
  vsp_flag = u_1  ("DPS: vsp_param_falg"  , s);
  if(vsp_flag)
    decode_acquisition_info(threeDV_acquisition_info, subsps_3dv_acquisition_info, forward_3dv_acquisition_info, backward_3dv_acquisition_info, s, 1);

  dps->depth_param_additional_extension_flag=u_1("sps_3dv_extension: depth_param_additional_extension_flag",                  s);    

  dps->NonlinearDepthNum = ue_v ("sps_3dv_extension: nonlinear_depth_num",                  s);      
  dps->NonlinearDepthPoints[0]=0;
  for (k=1; k<=dps->NonlinearDepthNum; ++k)    
  {
    dps->NonlinearDepthPoints[k] = (char)ue_v ("sps_3dv_extension: nonlinear_depth_coeff",                  s);
  }
  dps->NonlinearDepthPoints[dps->NonlinearDepthNum+1]=0;

  g_NonlinearDepthNum_dec=dps->NonlinearDepthNum;
  g_NonlinearDepthPoints_dec[0]=0;   // beginning
  for (k=1; k<=g_NonlinearDepthNum_dec; ++k)
  {
    g_NonlinearDepthPoints_dec[k]=dps->NonlinearDepthPoints[k];
  }
  g_NonlinearDepthPoints_dec[g_NonlinearDepthNum_dec+1]=0; // end

  dps->Valid = TRUE;
  return p_Dec->UsedBits;
}
#endif

// fill subset_sps with content of p
#if MVC_EXTENSION_ENABLE||EXT3D
static int InterpretSubsetSPS (VideoParameters *p_Vid, DataPartition *p, int *curr_seq_set_id)
{
  subset_seq_parameter_set_rbsp_t *subset_sps;
#if EXT3D
  seq_parameter_set_3dv_extension *subset_sps_3dv_extension;
#endif
  unsigned int additional_extension2_flag;
#if EXT3D
  unsigned int additional_extension3_flag;
#endif
  Bitstream *s = p->bitstream;
  seq_parameter_set_rbsp_t *sps = AllocSPS();

  assert (p != NULL);
  assert (p->bitstream != NULL);
  assert (p->bitstream->streamBuffer != 0);

  InterpretSPS (p_Vid, p, sps);

  *curr_seq_set_id = sps->seq_parameter_set_id;
  subset_sps = p_Vid->SubsetSeqParSet + sps->seq_parameter_set_id;

  if(subset_sps->Valid || subset_sps->num_views_minus1>=0)
  {
    reset_subset_sps(subset_sps);
  }
#if EXT3D
  subset_sps_3dv_extension=p_Vid->SeqParSet3DVExtension+sps->seq_parameter_set_id;
  if(subset_sps_3dv_extension->valid)
    reset_sps_3dv_extension(subset_sps_3dv_extension);
#endif
  memcpy (&subset_sps->sps, sps, sizeof (seq_parameter_set_rbsp_t));


  assert (subset_sps != NULL);
  subset_sps->Valid = FALSE;


  /*if(subset_sps->sps.profile_idc == SCALABLE_BASELINE_PROFILE || subset_sps->sps.profile_idc == SCALABLE_HIGH_PROFILE)
  {
    printf("\nScalable profile is not supported yet!\n");
  }
  else*/
  if(subset_sps->sps.profile_idc == MVC_HIGH || subset_sps->sps.profile_idc == STEREO_HIGH)
  {
    subset_sps->bit_equal_to_one = u_1("bit_equal_to_one"                      , s);

    if(subset_sps->bit_equal_to_one !=1 )
    {
      printf("\nbit_equal_to_one is not equal to 1!\n");
      return p_Dec->UsedBits;
    }
    seq_parameter_set_mvc_extension(subset_sps, s);
  
    subset_sps->mvc_vui_parameters_present_flag = u_1("mvc_vui_parameters_present_flag"      , s);
    if(subset_sps->mvc_vui_parameters_present_flag)
      mvc_vui_parameters_extension(&(subset_sps->MVCVUIParams)                    , s);
  }

#if EXT3D
  else if((subset_sps->sps.profile_idc==ThreeDV_HIGH)||(subset_sps->sps.profile_idc==ThreeDV_EXTEND_HIGH))
  {
    subset_sps->bit_equal_to_one = u_1("bit_equal_to_one"                      , s);

    if(subset_sps->bit_equal_to_one !=1 )
    {
      printf("\nbit_equal_to_one is not equal to 1!\n");
      return p_Dec->UsedBits;
    }
    seq_parameter_set_mvc_extension(subset_sps, s);

    subset_sps->threedvc_vui_parameters_present_flag = u_1("3dvc_vui_parameters_present_flag"      , s);
    if(subset_sps->threedvc_vui_parameters_present_flag)
      mvcd_vui_parameters_extension(&(subset_sps->MVCVUIParams),s);

    if (subset_sps->sps.profile_idc==ThreeDV_HIGH || subset_sps->sps.profile_idc==ThreeDV_EXTEND_HIGH)
    {
      subset_sps->texture_vui_parameters_present_flag = u_1("texture_vui_parameters_present_flag"      , s);
      if(subset_sps->texture_vui_parameters_present_flag)
        mvc_vui_parameters_extension(&(subset_sps->TexVUIParams),s);
    }

    subset_sps_3dv_extension->subset_mvc_sps=subset_sps;
    if (sps->profile_idc==ThreeDV_EXTEND_HIGH)
      interpret_sps_3dv_extension(subset_sps_3dv_extension,subset_sps->num_views_minus1+1,s);

     if (p_Vid->DepParSet[0]!=NULL)
     {
       FreeDPS(p_Vid->DepParSet[0]);
       p_Vid->DepParSet[0]=NULL;
     }
      p_Vid->DepParSet[0]=AllocDPS();
      copy_3dv_acquisition_info(p_Vid->DepParSet[0]->acquisition_info, subset_sps_3dv_extension->threeDV_acquisition_in_subsps);
  }

  if(strlen(p_Vid->p_Inp->CameraFile))
  {
    p_Vid->dec_HHI_fast_vs_cam_file=fopen(p_Vid->p_Inp->CameraFile,"wb");
    fprintf(p_Vid->dec_HHI_fast_vs_cam_file,"#ViewNum    StartFrame   EndFrame        ZNear           ZFar\n");
  }

  p_Vid->SliceHeaderPred =subset_sps_3dv_extension->SliceHeaderPred;
  p_Vid->VSP_Enable=subset_sps_3dv_extension->VSP_Enable;

  p_Vid->AdaptiveLuminanceCompensation = subset_sps_3dv_extension->AdaptiveLuminanceCompensation;
  p_Vid->RLESkipTex = subset_sps_3dv_extension->RLESkip;

  if((subset_sps->sps.profile_idc==ThreeDV_HIGH)||(subset_sps->sps.profile_idc==ThreeDV_EXTEND_HIGH))
  {
    additional_extension3_flag = u_1("additional_extension3_flag"                    , s);
    if(additional_extension3_flag)
    {
      while (more_rbsp_data(s->streamBuffer, s->frame_bitoffset,s->bitstream_length))
        additional_extension3_flag = u_1("additional_extension3_flag"                , s);
    }
  }
  else
  {
#endif
  additional_extension2_flag = u_1("additional_extension2_flag"                    , s);
  if(additional_extension2_flag)
  {
    while (more_rbsp_data(s->streamBuffer, s->frame_bitoffset,s->bitstream_length))
      additional_extension2_flag = u_1("additional_extension2_flag"                , s);
  }
#if EXT3D
  }
#endif

  if (subset_sps->sps.Valid)
    subset_sps->Valid = TRUE;

  FreeSPS (sps);
  return p_Dec->UsedBits;

}
#endif

void InitVUI(seq_parameter_set_rbsp_t *sps)
{
  sps->vui_seq_parameters.matrix_coefficients = 2;
}


int ReadVUI(DataPartition *p, seq_parameter_set_rbsp_t *sps)
{
  Bitstream *s = p->bitstream;
  if (sps->vui_parameters_present_flag)
  {
    sps->vui_seq_parameters.aspect_ratio_info_present_flag = u_1  ("VUI: aspect_ratio_info_present_flag"   , s);
    if (sps->vui_seq_parameters.aspect_ratio_info_present_flag)
    {
      sps->vui_seq_parameters.aspect_ratio_idc             = u_v  ( 8, "VUI: aspect_ratio_idc"              , s);
      if (255==sps->vui_seq_parameters.aspect_ratio_idc)
      {
        sps->vui_seq_parameters.sar_width                  = (unsigned short) u_v  (16, "VUI: sar_width"                     , s);
        sps->vui_seq_parameters.sar_height                 = (unsigned short) u_v  (16, "VUI: sar_height"                    , s);
      }
    }

    sps->vui_seq_parameters.overscan_info_present_flag     = u_1  ("VUI: overscan_info_present_flag"        , s);
    if (sps->vui_seq_parameters.overscan_info_present_flag)
    {
      sps->vui_seq_parameters.overscan_appropriate_flag    = u_1  ("VUI: overscan_appropriate_flag"         , s);
    }

    sps->vui_seq_parameters.video_signal_type_present_flag = u_1  ("VUI: video_signal_type_present_flag"    , s);
    if (sps->vui_seq_parameters.video_signal_type_present_flag)
    {
      sps->vui_seq_parameters.video_format                    = u_v  ( 3,"VUI: video_format"                      , s);
      sps->vui_seq_parameters.video_full_range_flag           = u_1  (   "VUI: video_full_range_flag"             , s);
      sps->vui_seq_parameters.colour_description_present_flag = u_1  (   "VUI: color_description_present_flag"    , s);
      if(sps->vui_seq_parameters.colour_description_present_flag)
      {
        sps->vui_seq_parameters.colour_primaries              = u_v  ( 8,"VUI: colour_primaries"                  , s);
        sps->vui_seq_parameters.transfer_characteristics      = u_v  ( 8,"VUI: transfer_characteristics"          , s);
        sps->vui_seq_parameters.matrix_coefficients           = u_v  ( 8,"VUI: matrix_coefficients"               , s);
      }
    }
    sps->vui_seq_parameters.chroma_location_info_present_flag = u_1  (   "VUI: chroma_loc_info_present_flag"      , s);
    if(sps->vui_seq_parameters.chroma_location_info_present_flag)
    {
      sps->vui_seq_parameters.chroma_sample_loc_type_top_field     = ue_v  ( "VUI: chroma_sample_loc_type_top_field"    , s);
      sps->vui_seq_parameters.chroma_sample_loc_type_bottom_field  = ue_v  ( "VUI: chroma_sample_loc_type_bottom_field" , s);
    }
    sps->vui_seq_parameters.timing_info_present_flag          = u_1  ("VUI: timing_info_present_flag"           , s);
    if (sps->vui_seq_parameters.timing_info_present_flag)
    {
      sps->vui_seq_parameters.num_units_in_tick               = u_v  (32,"VUI: num_units_in_tick"               , s);
      sps->vui_seq_parameters.time_scale                      = u_v  (32,"VUI: time_scale"                      , s);
      sps->vui_seq_parameters.fixed_frame_rate_flag           = u_1  (   "VUI: fixed_frame_rate_flag"           , s);
    }
    sps->vui_seq_parameters.nal_hrd_parameters_present_flag   = u_1  ("VUI: nal_hrd_parameters_present_flag"    , s);
    if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
    {
      ReadHRDParameters(p, &(sps->vui_seq_parameters.nal_hrd_parameters));
    }
    sps->vui_seq_parameters.vcl_hrd_parameters_present_flag   = u_1  ("VUI: vcl_hrd_parameters_present_flag"    , s);
    if (sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
    {
      ReadHRDParameters(p, &(sps->vui_seq_parameters.vcl_hrd_parameters));
    }
    if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag || sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
    {
      sps->vui_seq_parameters.low_delay_hrd_flag             =  u_1  ("VUI: low_delay_hrd_flag"                 , s);
    }
    sps->vui_seq_parameters.pic_struct_present_flag          =  u_1  ("VUI: pic_struct_present_flag   "         , s);
    sps->vui_seq_parameters.bitstream_restriction_flag       =  u_1  ("VUI: bitstream_restriction_flag"         , s);
    if (sps->vui_seq_parameters.bitstream_restriction_flag)
    {
      sps->vui_seq_parameters.motion_vectors_over_pic_boundaries_flag =  u_1  ("VUI: motion_vectors_over_pic_boundaries_flag", s);
      sps->vui_seq_parameters.max_bytes_per_pic_denom                 =  ue_v ("VUI: max_bytes_per_pic_denom"                , s);
      sps->vui_seq_parameters.max_bits_per_mb_denom                   =  ue_v ("VUI: max_bits_per_mb_denom"                  , s);
      sps->vui_seq_parameters.log2_max_mv_length_horizontal           =  ue_v ("VUI: log2_max_mv_length_horizontal"          , s);
      sps->vui_seq_parameters.log2_max_mv_length_vertical             =  ue_v ("VUI: log2_max_mv_length_vertical"            , s);
      sps->vui_seq_parameters.num_reorder_frames                      =  ue_v ("VUI: num_reorder_frames"                     , s);
      sps->vui_seq_parameters.max_dec_frame_buffering                 =  ue_v ("VUI: max_dec_frame_buffering"                , s);
    }
  }

  return 0;
}


int ReadHRDParameters(DataPartition *p, hrd_parameters_t *hrd)
{
  Bitstream *s = p->bitstream;
  unsigned int SchedSelIdx;

  hrd->cpb_cnt_minus1                                      = ue_v (   "VUI: cpb_cnt_minus1"                       , s);
  hrd->bit_rate_scale                                      = u_v  ( 4,"VUI: bit_rate_scale"                       , s);
  hrd->cpb_size_scale                                      = u_v  ( 4,"VUI: cpb_size_scale"                       , s);

  for( SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1; SchedSelIdx++ )
  {
    hrd->bit_rate_value_minus1[ SchedSelIdx ]             = ue_v  ( "VUI: bit_rate_value_minus1"                  , s);
    hrd->cpb_size_value_minus1[ SchedSelIdx ]             = ue_v  ( "VUI: cpb_size_value_minus1"                  , s);
    hrd->cbr_flag[ SchedSelIdx ]                          = u_1   ( "VUI: cbr_flag"                               , s);
  }

  hrd->initial_cpb_removal_delay_length_minus1            = u_v  ( 5,"VUI: initial_cpb_removal_delay_length_minus1" , s);
  hrd->cpb_removal_delay_length_minus1                    = u_v  ( 5,"VUI: cpb_removal_delay_length_minus1"         , s);
  hrd->dpb_output_delay_length_minus1                     = u_v  ( 5,"VUI: dpb_output_delay_length_minus1"          , s);
  hrd->time_offset_length                                 = u_v  ( 5,"VUI: time_offset_length"          , s);

  return 0;
}


int InterpretPPS (VideoParameters *p_Vid, DataPartition *p, pic_parameter_set_rbsp_t *pps)
{
  unsigned i;
  unsigned n_ScalingList;
  int chroma_format_idc;
  int NumberBitsPerSliceGroupId;
  Bitstream *s = p->bitstream;

  assert (p != NULL);
  assert (p->bitstream != NULL);
  assert (p->bitstream->streamBuffer != 0);
  assert (pps != NULL);

  p_Dec->UsedBits = 0;

  pps->pic_parameter_set_id                  = ue_v ("PPS: pic_parameter_set_id"                   , s);
  pps->seq_parameter_set_id                  = ue_v ("PPS: seq_parameter_set_id"                   , s);
  pps->entropy_coding_mode_flag              = u_1  ("PPS: entropy_coding_mode_flag"               , s);

  //! Note: as per JVT-F078 the following bit is unconditional.  If F078 is not accepted, then
  //! one has to fetch the correct SPS to check whether the bit is present (hopefully there is
  //! no consistency problem :-(
  //! The current encoder code handles this in the same way.  When you change this, don't forget
  //! the encoder!  StW, 12/8/02
  pps->bottom_field_pic_order_in_frame_present_flag                = u_1  ("PPS: bottom_field_pic_order_in_frame_present_flag"                 , s);

  pps->num_slice_groups_minus1               = ue_v ("PPS: num_slice_groups_minus1"                , s);

  // FMO stuff begins here
  if (pps->num_slice_groups_minus1 > 0)
  {
    pps->slice_group_map_type               = ue_v ("PPS: slice_group_map_type"                , s);
    if (pps->slice_group_map_type == 0)
    {
      for (i=0; i<=pps->num_slice_groups_minus1; i++)
        pps->run_length_minus1 [i]                  = ue_v ("PPS: run_length_minus1 [i]"              , s);
    }
    else if (pps->slice_group_map_type == 2)
    {
      for (i=0; i<pps->num_slice_groups_minus1; i++)
      {
        //! JVT-F078: avoid reference of SPS by using ue(v) instead of u(v)
        pps->top_left [i]                          = ue_v ("PPS: top_left [i]"                        , s);
        pps->bottom_right [i]                      = ue_v ("PPS: bottom_right [i]"                    , s);
      }
    }
    else if (pps->slice_group_map_type == 3 ||
             pps->slice_group_map_type == 4 ||
             pps->slice_group_map_type == 5)
    {
      pps->slice_group_change_direction_flag     = u_1  ("PPS: slice_group_change_direction_flag"      , s);
      pps->slice_group_change_rate_minus1        = ue_v ("PPS: slice_group_change_rate_minus1"         , s);
    }
    else if (pps->slice_group_map_type == 6)
    {
      if (pps->num_slice_groups_minus1+1 >4)
        NumberBitsPerSliceGroupId = 3;
      else if (pps->num_slice_groups_minus1+1 > 2)
        NumberBitsPerSliceGroupId = 2;
      else
        NumberBitsPerSliceGroupId = 1;
      pps->pic_size_in_map_units_minus1      = ue_v ("PPS: pic_size_in_map_units_minus1"               , s);
      if ((pps->slice_group_id = calloc (pps->pic_size_in_map_units_minus1+1, 1)) == NULL)
        no_mem_exit ("InterpretPPS: slice_group_id");
      for (i=0; i<=pps->pic_size_in_map_units_minus1; i++)
        pps->slice_group_id[i] = (byte) u_v (NumberBitsPerSliceGroupId, "slice_group_id[i]", s);
    }
  }

  // End of FMO stuff

  pps->num_ref_idx_l0_active_minus1          = ue_v ("PPS: num_ref_idx_l0_active_minus1"           , s);
  pps->num_ref_idx_l1_active_minus1          = ue_v ("PPS: num_ref_idx_l1_active_minus1"           , s);
  pps->weighted_pred_flag                    = u_1  ("PPS: weighted_pred_flag"                     , s);
  pps->weighted_bipred_idc                   = u_v  ( 2, "PPS: weighted_bipred_idc"                , s);
  pps->pic_init_qp_minus26                   = se_v ("PPS: pic_init_qp_minus26"                    , s);
  pps->pic_init_qs_minus26                   = se_v ("PPS: pic_init_qs_minus26"                    , s);

  pps->chroma_qp_index_offset                = se_v ("PPS: chroma_qp_index_offset"                 , s);

  pps->deblocking_filter_control_present_flag = u_1 ("PPS: deblocking_filter_control_present_flag" , s);
  pps->constrained_intra_pred_flag           = u_1  ("PPS: constrained_intra_pred_flag"            , s);
  pps->redundant_pic_cnt_present_flag        = u_1  ("PPS: redundant_pic_cnt_present_flag"         , s);

  if(more_rbsp_data(s->streamBuffer, s->frame_bitoffset,s->bitstream_length)) // more_data_in_rbsp()
  {
    //Fidelity Range Extensions Stuff
    pps->transform_8x8_mode_flag           = u_1  ("PPS: transform_8x8_mode_flag"                , s);
    pps->pic_scaling_matrix_present_flag   =  u_1  ("PPS: pic_scaling_matrix_present_flag"        , s);

    if(pps->pic_scaling_matrix_present_flag)
    {
      chroma_format_idc = p_Vid->SeqParSet[pps->seq_parameter_set_id].chroma_format_idc;
      n_ScalingList = 6 + ((chroma_format_idc != YUV444) ? 2 : 6) * pps->transform_8x8_mode_flag;
      for(i=0; i<n_ScalingList; i++)
      {
        pps->pic_scaling_list_present_flag[i]= u_1  ("PPS: pic_scaling_list_present_flag"          , s);

        if(pps->pic_scaling_list_present_flag[i])
        {
          if(i<6)
            Scaling_List(pps->ScalingList4x4[i], 16, &pps->UseDefaultScalingMatrix4x4Flag[i], s);
          else
            Scaling_List(pps->ScalingList8x8[i-6], 64, &pps->UseDefaultScalingMatrix8x8Flag[i-6], s);
        }
      }
    }
    pps->second_chroma_qp_index_offset      = se_v ("PPS: second_chroma_qp_index_offset"          , s);
  }
  else
  {
    pps->second_chroma_qp_index_offset      = pps->chroma_qp_index_offset;
  }

  pps->Valid = TRUE;
  return p_Dec->UsedBits;
}


void PPSConsistencyCheck (pic_parameter_set_rbsp_t *pps)
{
  UNREFERENCED_PARAMETER(pps);
  printf ("Consistency checking a picture parset, to be implemented\n");
//  if (pps->seq_parameter_set_id invalid then do something)
}

void SPSConsistencyCheck (seq_parameter_set_rbsp_t *sps)
{
  UNREFERENCED_PARAMETER(sps);
  printf ("Consistency checking a sequence parset, to be implemented\n");
}

#if MVC_EXTENSION_ENABLE||EXT3D
void SubsetSPSConsistencyCheck (subset_seq_parameter_set_rbsp_t *subset_sps)
{
  UNREFERENCED_PARAMETER(subset_sps);
  printf ("Consistency checking a subset sequence parset, to be implemented\n");
}
#endif

#if EXT3D
void DPSConsistencyCheck (depth_parameter_set_rbsp_t *dps)
{
  UNREFERENCED_PARAMETER(dps);
  printf ("Consistency checking a depth parset, to be implemented\n");
}
#endif

void MakePPSavailable (VideoParameters *p_Vid, int id, pic_parameter_set_rbsp_t *pps)
{
  assert (pps->Valid == TRUE);

  if (p_Vid->PicParSet[id].Valid == TRUE && p_Vid->PicParSet[id].slice_group_id != NULL)
    free (p_Vid->PicParSet[id].slice_group_id);

  memcpy (&p_Vid->PicParSet[id], pps, sizeof (pic_parameter_set_rbsp_t));

  // we can simply use the memory provided with the pps. the PPS is destroyed after this function
  // call and will not try to free if pps->slice_group_id == NULL
  p_Vid->PicParSet[id].slice_group_id = pps->slice_group_id;
  pps->slice_group_id          = NULL;
}

void CleanUpPPS(VideoParameters *p_Vid)
{
  int i;

  for (i=0; i<MAXPPS; i++)
  {
    if (p_Vid->PicParSet[i].Valid == TRUE && p_Vid->PicParSet[i].slice_group_id != NULL)
      free (p_Vid->PicParSet[i].slice_group_id);

    p_Vid->PicParSet[i].Valid = FALSE;
  }
}


void MakeSPSavailable (VideoParameters *p_Vid, int id, seq_parameter_set_rbsp_t *sps)
{
  assert (sps->Valid == TRUE);
  memcpy (&p_Vid->SeqParSet[id], sps, sizeof (seq_parameter_set_rbsp_t));
}

#if EXT3D
void MakeDPSavailable (VideoParameters *p_Vid, int id, depth_parameter_set_rbsp_t *dps)
{
  assert (dps->Valid == TRUE);
  p_Vid->DepParSet[id]=dps;
}
#endif

void ProcessSPS (VideoParameters *p_Vid, NALU_t *nalu)
{  
  DataPartition *dp = AllocPartition(1);
  seq_parameter_set_rbsp_t *sps = AllocSPS();

  memcpy (dp->bitstream->streamBuffer, &nalu->buf[1], nalu->len-1);
  dp->bitstream->code_len = dp->bitstream->bitstream_length = RBSPtoSODB (dp->bitstream->streamBuffer, nalu->len-1);
  dp->bitstream->ei_flag = 0;
  dp->bitstream->read_len = dp->bitstream->frame_bitoffset = 0;
  InterpretSPS (p_Vid, dp, sps);


#if MVC_EXTENSION_ENABLE||EXT3D
  get_max_dec_frame_buf_size(sps);
#endif


  if (sps->Valid)
  {
    if (p_Vid->active_sps)
    {
      if (sps->seq_parameter_set_id == p_Vid->active_sps->seq_parameter_set_id)
      {
        if (!sps_is_equal(sps, p_Vid->active_sps))
        {
          if (p_Vid->dec_picture) // && p_Vid->num_dec_mb == p_Vid->PicSizeInMbs) //?
          {
            // this may only happen on slice loss
            exit_picture(p_Vid, &p_Vid->dec_picture);
          }
          p_Vid->active_sps=NULL;
        }
      }
    }
    // SPSConsistencyCheck (pps);

#if EXT3D
    p_Vid->text_width=(sps->pic_width_in_mbs_minus1+1)*MB_BLOCK_SIZE;
    p_Vid->text_height=(sps->pic_height_in_map_units_minus1+1)*MB_BLOCK_SIZE;
    if (sps->chroma_format_idc == YUV420)
    {
      p_Vid->text_width_cr  = (p_Vid->text_width  >> 1);
      p_Vid->text_height_cr = (p_Vid->text_height >> 1);
    }
    else if (sps->chroma_format_idc == YUV422)
    {
      p_Vid->text_width_cr  = (p_Vid->text_width >> 1);
      p_Vid->text_height_cr = p_Vid->text_height;
    }
    else if (sps->chroma_format_idc== YUV444)
    {
      //YUV444
      p_Vid->text_width_cr = p_Vid->text_width;
      p_Vid->text_height_cr = p_Vid->text_height;
    }
#if ITRI_INTERLACE
    if ( (sps->is_depth==0) && (sps->profile_idc!=ThreeDV_HIGH) && (sps->profile_idc!=ThreeDV_EXTEND_HIGH))
      p_Vid->is_texture_frame = sps->frame_mbs_only_flag;
#endif
#endif

    MakeSPSavailable (p_Vid, sps->seq_parameter_set_id, sps);
    p_Vid->profile_idc = sps->profile_idc;
    p_Vid->separate_colour_plane_flag = sps->separate_colour_plane_flag;
    if( p_Vid->separate_colour_plane_flag )
    {
      p_Vid->ChromaArrayType = 0;
    }
    else
    {
      p_Vid->ChromaArrayType = sps->chroma_format_idc;
    }
  }

  FreePartition (dp, 1);
  FreeSPS (sps);
}

#if MVC_EXTENSION_ENABLE||EXT3D
void ProcessSubsetSPS (VideoParameters *p_Vid, NALU_t *nalu)
{
  DataPartition *dp = AllocPartition(1);
  subset_seq_parameter_set_rbsp_t *subset_sps;
  int curr_seq_set_id;

  memcpy (dp->bitstream->streamBuffer, &nalu->buf[1], nalu->len-1);
  dp->bitstream->code_len = dp->bitstream->bitstream_length = RBSPtoSODB (dp->bitstream->streamBuffer, nalu->len-1);
  dp->bitstream->ei_flag = 0;
  dp->bitstream->read_len = dp->bitstream->frame_bitoffset = 0;
  InterpretSubsetSPS (p_Vid, dp, &curr_seq_set_id);
  subset_sps = p_Vid->SubsetSeqParSet + curr_seq_set_id;
#if EXT3D
  p_Vid->num_of_views=subset_sps->num_views_minus1+1;
  if((subset_sps->is_depth)&&(p_Vid->num_of_views))
    if(subset_sps->num_views_minus1+1!=p_Vid->num_of_views)
      printf("The number of views for depth is different from texture\n");

#endif
  get_max_dec_frame_buf_size(&(subset_sps->sps));


  if (subset_sps->Valid)
  {
    // SubsetSPSConsistencyCheck (subset_sps);
    p_Vid->profile_idc = subset_sps->sps.profile_idc;
    p_Vid->separate_colour_plane_flag = subset_sps->sps.separate_colour_plane_flag;
    if( p_Vid->separate_colour_plane_flag )
    {
      p_Vid->ChromaArrayType = 0;
    }
    else
    {
      p_Vid->ChromaArrayType = subset_sps->sps.chroma_format_idc;
    }
  }

//  FreeSubsetSPS (subset_sps);
  FreePartition (dp, 1);  
}
#endif

#if EXT3D
void ProcessDPS (VideoParameters *p_Vid, NALU_t *nalu)
{  
  DataPartition *dp = AllocPartition(1);
  depth_parameter_set_rbsp_t *dps = AllocDPS();

  memcpy (dp->bitstream->streamBuffer, &nalu->buf[1], nalu->len-1);
  dp->bitstream->code_len = dp->bitstream->bitstream_length = RBSPtoSODB (dp->bitstream->streamBuffer, nalu->len-1);
  dp->bitstream->ei_flag = 0;
  dp->bitstream->read_len = dp->bitstream->frame_bitoffset = 0;
  InterpretDPS (p_Vid, dp, dps);

  if (dps->Valid)
  {
    if (p_Vid->active_dps)
    {
      if (dps->dep_parameter_set_id == p_Vid->active_dps->dep_parameter_set_id)
      {
        if (!dps_is_equal(dps, p_Vid->active_dps))
        {
          p_Vid->active_dps=NULL;
        }
      }
    }

    MakeDPSavailable (p_Vid, dps->dep_parameter_set_id, dps);

  }

  p_Vid->NonlinearDepthNum = dps->NonlinearDepthNum;
  memcpy(p_Vid->NonlinearDepthPoints, dps->NonlinearDepthPoints, sizeof(p_Vid->NonlinearDepthPoints));

  FreePartition (dp, 1);
}

extern void decode_acquisition_element(ThreeDVAcquisitionInfo* threeDV_acquisition_info,
                     ThreeDVAE* curr_3dv_ae,
                     ThreeDVAE* forward_3dv_ae,
                     ThreeDVAE* backward_3dv_ae,
                     Bitstream* bitstream)
{
  int32 k=0;
  int32 forward_weight=threeDV_acquisition_info->pred_weight0;
  int32 backward_weight=64-forward_weight;
  int32 prediction=threeDV_acquisition_info->pred_direction;

  int32 mantissa_prediction=0;
  int32 mantissa_diff=0;
  //int32 mantissa_size=0;

  int32 element_skip=0;
  int32 exponent_skip=0;
  int32 num_of_sets=threeDV_acquisition_info->num_views-curr_3dv_ae->delta_flag;

  for(k=0;k<curr_3dv_ae->delta_flag;++k)
    curr_3dv_ae->rec[k]=0;

  if(num_of_sets>1)
    curr_3dv_ae->element_equal=(byte)u_1("3dv_acquisition_info:element_equal",bitstream);
  else
    curr_3dv_ae->element_equal=0;

  if(curr_3dv_ae->element_equal)
    num_of_sets=1;

  if(prediction==2)
  {
    if(curr_3dv_ae->pred_mode==0)
    {
      curr_3dv_ae->mantissa_length=u_v(5,"3dv_acquisition_infO:matissa_len_minus1",bitstream);
      ++curr_3dv_ae->mantissa_length;
      //mantissa_size=curr_3dv_ae->mantissa_length;
    }
    else
    {
      curr_3dv_ae->precision=u_v(5,"3dv_acquisition_info:prec",bitstream);
    }
  }
  else
  {
    if(curr_3dv_ae->pred_mode==0)
      curr_3dv_ae->mantissa_length=forward_3dv_ae->mantissa_length;
    else
      curr_3dv_ae->precision=forward_3dv_ae->precision;
  }

  for(;k<num_of_sets+curr_3dv_ae->delta_flag;++k)
  {
    if(prediction==2)
    {
      curr_3dv_ae->element[k].sign=u_1("3dv_acquisition_info:sign",bitstream);
      curr_3dv_ae->element[k].exponent=u_v(curr_3dv_ae->exponent_size,"3dv_acquisition_info:exponent",bitstream);
      if(curr_3dv_ae->pred_mode)
      {
        if(curr_3dv_ae->element[k].exponent)
          curr_3dv_ae->mantissa_length=imin(imax(0,curr_3dv_ae->element[k].exponent+curr_3dv_ae->precision-31),30);
        else
          curr_3dv_ae->mantissa_length=imin(imax(0,curr_3dv_ae->precision-30),30);
      }
      curr_3dv_ae->element[k].mantissa=u_v(curr_3dv_ae->mantissa_length,"3dv_acquisition_info:mantissa",bitstream);
    }
    else
    {
      mantissa_prediction=(forward_weight*forward_3dv_ae->element[k].mantissa+
        backward_weight*backward_3dv_ae->element[k].mantissa+32)>>6;
      element_skip=u_1("3dv_acquisition_info:element_skip_flag",bitstream);
      if(element_skip)
      {
        curr_3dv_ae->element[k].sign=forward_3dv_ae->element[k].sign;
        curr_3dv_ae->element[k].exponent=forward_3dv_ae->element[k].exponent;
        mantissa_diff=0;
      }
      else
      {
        curr_3dv_ae->element[k].sign=u_1("3dv_acquisition_info:sign",bitstream);
        exponent_skip=u_1("3dv_acquisition_info:exponent_skip_flag",bitstream);
        if(exponent_skip)
          curr_3dv_ae->element[k].exponent=forward_3dv_ae->element[k].exponent;
        else
          curr_3dv_ae->element[k].exponent=u_v(curr_3dv_ae->exponent_size,"3dv_acquisition_info:exponent",bitstream);
        //decode_mantissa(&mantissa_diff,bitstream);
        mantissa_diff=se_v("3dv_acquisition_info:mantissa_diff",bitstream);

      }
      curr_3dv_ae->element[k].mantissa=mantissa_diff+mantissa_prediction;
    }
    get_rec_double_type(curr_3dv_ae,k);
  }
  if(curr_3dv_ae->element_equal)
  {
    for(k=curr_3dv_ae->delta_flag+1;k<threeDV_acquisition_info->num_views;++k)
    {
      curr_3dv_ae->element[k].sign=curr_3dv_ae->element[curr_3dv_ae->delta_flag].sign;
      curr_3dv_ae->element[k].exponent=curr_3dv_ae->element[curr_3dv_ae->delta_flag].exponent;
      curr_3dv_ae->element[k].mantissa=curr_3dv_ae->element[curr_3dv_ae->delta_flag].mantissa;
      curr_3dv_ae->rec[k]=curr_3dv_ae->rec[curr_3dv_ae->delta_flag];
    }
  }
}

static void decode_disparity_element(ThreeDVAcquisitionInfo* curr_3dv_acquisition_info,
  ThreeDVAcquisitionInfo* forward_3dv_acquisition_info,
  ThreeDVAcquisitionInfo* backward_3dv_acquisition_info,
  Bitstream* bitstream)
{
  int32 i,j;
  int32 num_of_views=curr_3dv_acquisition_info->num_views;
  int32 forward_weight=curr_3dv_acquisition_info->pred_weight0;
  int32 backward_weight=64-forward_weight;
  int32 prediction=curr_3dv_acquisition_info->pred_direction;
  int temp;

  for (i=0;i<num_of_views;i++)
  {
    for (j=0;j<i;j++)
    {
      curr_3dv_acquisition_info->i_disparity_scale_diff[j][i]=se_v("3dv_acquisition_info:scale parameter for disparity",bitstream);
      curr_3dv_acquisition_info->i_disparity_offset_diff[j][i]=se_v("3dv_acquisition_info:offset parameter for disparity",bitstream);
      curr_3dv_acquisition_info->i_disparity_scale_diff[i][j]=se_v("3dv_acquisition_info:scale parameter for disparity",bitstream);
      curr_3dv_acquisition_info->i_disparity_offset_diff[i][j]=se_v("3dv_acquisition_info:offset parameter for disparity",bitstream);
      if(prediction==2)
      {
        curr_3dv_acquisition_info->i_disparity_scale[j][i]=curr_3dv_acquisition_info->i_disparity_scale_diff[j][i];
        curr_3dv_acquisition_info->i_disparity_offset[j][i]=curr_3dv_acquisition_info->i_disparity_offset_diff[j][i];
        curr_3dv_acquisition_info->i_disparity_scale[i][j]=curr_3dv_acquisition_info->i_disparity_scale_diff[i][j]-curr_3dv_acquisition_info->i_disparity_scale[j][i];
        curr_3dv_acquisition_info->i_disparity_offset[i][j]=curr_3dv_acquisition_info->i_disparity_offset_diff[i][j]-curr_3dv_acquisition_info->i_disparity_offset[j][i];
      }
      else
      {
        temp=(forward_3dv_acquisition_info->i_disparity_scale[j][i]*forward_weight+
          backward_3dv_acquisition_info->i_disparity_scale[j][i]*backward_weight+32)>>6;
        curr_3dv_acquisition_info->i_disparity_scale[j][i]=curr_3dv_acquisition_info->i_disparity_scale_diff[j][i]+temp;

        temp=(forward_3dv_acquisition_info->i_disparity_offset[j][i]*forward_weight+
          backward_3dv_acquisition_info->i_disparity_offset[j][i]*backward_weight+32)>>6;
        curr_3dv_acquisition_info->i_disparity_offset[j][i]=curr_3dv_acquisition_info->i_disparity_offset_diff[j][i]+temp;

        temp=((forward_3dv_acquisition_info->i_disparity_scale[i][j]+forward_3dv_acquisition_info->i_disparity_scale[j][i])*forward_weight+
          (backward_3dv_acquisition_info->i_disparity_scale[i][j]+backward_3dv_acquisition_info->i_disparity_scale[j][i])*backward_weight+32)>>6;
        curr_3dv_acquisition_info->i_disparity_scale[i][j]=curr_3dv_acquisition_info->i_disparity_scale_diff[i][j]-curr_3dv_acquisition_info->i_disparity_scale[j][i]+temp;

        temp=((forward_3dv_acquisition_info->i_disparity_offset[i][j]+forward_3dv_acquisition_info->i_disparity_offset[j][i])*forward_weight+
          (backward_3dv_acquisition_info->i_disparity_offset[i][j]+backward_3dv_acquisition_info->i_disparity_offset[j][i])*backward_weight+32)>>6;
        curr_3dv_acquisition_info->i_disparity_offset[i][j]=curr_3dv_acquisition_info->i_disparity_offset_diff[i][j]-curr_3dv_acquisition_info->i_disparity_offset[j][i]+temp;
      }
    }
  }
}

extern void decode_acquisition_info(ThreeDVAcquisitionInfo* threeDV_acquisition_info, ThreeDVAcquisitionInfo* subsps_3dv_acquisition_info, ThreeDVAcquisitionInfo* forward_3dv_acquisition_info, ThreeDVAcquisitionInfo* backward_3dv_acquisition_info, Bitstream* bitstream, int is_vsp_param_flag)
{
  if(!is_vsp_param_flag){
    threeDV_acquisition_info->depth_near_ae->element_flag=(byte)u_1("3dv_acquisition_info:depth_near_flag",bitstream);
    threeDV_acquisition_info->depth_far_ae->element_flag=(byte)u_1("3dv_acquisition_info:depth_far_flag",bitstream);
    if(threeDV_acquisition_info->depth_near_ae->element_flag==0)
    {
      copy_acquisition_element(threeDV_acquisition_info->depth_near_ae,subsps_3dv_acquisition_info->depth_near_ae);
    }
    else
    {
      decode_acquisition_element(threeDV_acquisition_info,
        threeDV_acquisition_info->depth_near_ae,
        forward_3dv_acquisition_info->depth_near_ae,
        backward_3dv_acquisition_info->depth_near_ae,
        bitstream);
    }

    if(threeDV_acquisition_info->depth_far_ae->element_flag==0)
    {
      copy_acquisition_element(threeDV_acquisition_info->depth_far_ae,subsps_3dv_acquisition_info->depth_far_ae);
    }
    else
    {
      decode_acquisition_element(threeDV_acquisition_info,
        threeDV_acquisition_info->depth_far_ae,
        forward_3dv_acquisition_info->depth_far_ae,
        backward_3dv_acquisition_info->depth_far_ae,
        bitstream);
    }
  }
  else{
    threeDV_acquisition_info->disp_param_flag=(byte)u_1("3dv_acquisition_info:disp_param_flag",bitstream);
    if(threeDV_acquisition_info->disp_param_flag==0)
    {
      memcpy(threeDV_acquisition_info->i_disparity_scale[0],subsps_3dv_acquisition_info->i_disparity_scale[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
      memcpy(threeDV_acquisition_info->i_disparity_offset[0],subsps_3dv_acquisition_info->i_disparity_offset[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
    }
    else
    {
      decode_disparity_element(threeDV_acquisition_info,
        forward_3dv_acquisition_info,
        backward_3dv_acquisition_info,
        bitstream);
    }
  }
}

void copy_3dv_acquisition_info(ThreeDVAcquisitionInfo* dst_3dv_acquisition,ThreeDVAcquisitionInfo* src_3dv_acquisition)
{
  memcpy(dst_3dv_acquisition->focal_length_x_ae,src_3dv_acquisition->focal_length_x_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->focal_length_y_ae,src_3dv_acquisition->focal_length_y_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->principal_point_x_ae,src_3dv_acquisition->principal_point_x_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->principal_point_y_ae,src_3dv_acquisition->principal_point_y_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->translation_ae,src_3dv_acquisition->translation_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->depth_near_ae,src_3dv_acquisition->depth_near_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->depth_far_ae,src_3dv_acquisition->depth_far_ae,sizeof(ThreeDVAE));

  dst_3dv_acquisition->disp_param_flag=src_3dv_acquisition->disp_param_flag;
  memcpy(dst_3dv_acquisition->d_disparity_scale[0],src_3dv_acquisition->d_disparity_scale[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(double));
  memcpy(dst_3dv_acquisition->d_disparity_offset[0],src_3dv_acquisition->d_disparity_offset[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(double));
  memcpy(dst_3dv_acquisition->i_disparity_scale[0],src_3dv_acquisition->i_disparity_scale[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memcpy(dst_3dv_acquisition->i_disparity_offset[0],src_3dv_acquisition->i_disparity_offset[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memcpy(dst_3dv_acquisition->i_disparity_scale_diff[0],src_3dv_acquisition->i_disparity_scale_diff[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memcpy(dst_3dv_acquisition->i_disparity_offset_diff[0],src_3dv_acquisition->i_disparity_offset_diff[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
}

extern void copy_acquisition_element(ThreeDVAE* dst_ae, ThreeDVAE* src_ae)
{
  memcpy(dst_ae,src_ae,sizeof(ThreeDVAE));
}
#endif

void ProcessPPS (VideoParameters *p_Vid, NALU_t *nalu)
{
  DataPartition *dp = AllocPartition(1);
  pic_parameter_set_rbsp_t *pps = AllocPPS();

  memcpy (dp->bitstream->streamBuffer, &nalu->buf[1], nalu->len-1);
  dp->bitstream->code_len = dp->bitstream->bitstream_length = RBSPtoSODB (dp->bitstream->streamBuffer, nalu->len-1);
  dp->bitstream->ei_flag = 0;
  dp->bitstream->read_len = dp->bitstream->frame_bitoffset = 0;
  InterpretPPS (p_Vid, dp, pps);
  // PPSConsistencyCheck (pps);

  if (p_Vid->active_pps)
  {
    if (pps->pic_parameter_set_id == p_Vid->active_pps->pic_parameter_set_id)
    {
      if(!pps_is_equal(pps, p_Vid->active_pps))
      {
        //copy to next PPS;
        memcpy(p_Vid->pNextPPS, p_Vid->active_pps, sizeof (pic_parameter_set_rbsp_t));
        {
          if (p_Vid->dec_picture) // && p_Vid->num_dec_mb == p_Vid->PicSizeInMbs)
          {
            // this may only happen on slice loss
            exit_picture(p_Vid, &p_Vid->dec_picture);
          }
          p_Vid->active_pps = NULL;
        }
      }
    }
  }
  MakePPSavailable (p_Vid, pps->pic_parameter_set_id, pps);
  FreePartition (dp, 1);
  FreePPS (pps);
}

/*!
 ************************************************************************
 * \brief
 *    Updates images max values
 *
 ************************************************************************
 */
static void updateMaxValue(FrameFormat *format)
{
  format->max_value[0] = (1 << format->bit_depth[0]) - 1;
  format->max_value_sq[0] = format->max_value[0] * format->max_value[0];
  format->max_value[1] = (1 << format->bit_depth[1]) - 1;
  format->max_value_sq[1] = format->max_value[1] * format->max_value[1];
  format->max_value[2] = (1 << format->bit_depth[2]) - 1;
  format->max_value_sq[2] = format->max_value[2] * format->max_value[2];
}

/*!
 ************************************************************************
 * \brief
 *    Reset format information
 *
 ************************************************************************
 */
void reset_format_info(seq_parameter_set_rbsp_t *sps, VideoParameters *p_Vid, FrameFormat *source, FrameFormat *output)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  static const int SubWidthC  [4]= { 1, 2, 2, 1};
  static const int SubHeightC [4]= { 1, 2, 1, 1};

  int crop_left, crop_right;
  int crop_top, crop_bottom;

  // cropping for luma
  if (sps->frame_cropping_flag)
  {
    crop_left   = SubWidthC [sps->chroma_format_idc] * sps->frame_cropping_rect_left_offset;
    crop_right  = SubWidthC [sps->chroma_format_idc] * sps->frame_cropping_rect_right_offset;
    crop_top    = SubHeightC[sps->chroma_format_idc] * ( 2 - sps->frame_mbs_only_flag ) *  sps->frame_cropping_rect_top_offset;
    crop_bottom = SubHeightC[sps->chroma_format_idc] * ( 2 - sps->frame_mbs_only_flag ) *  sps->frame_cropping_rect_bottom_offset;
  }
  else
  {
    crop_left = crop_right = crop_top = crop_bottom = 0;
  }

  source->width[0] = p_Vid->width - crop_left - crop_right;
  source->height[0] = p_Vid->height - crop_top - crop_bottom;

  // cropping for chroma
  if (sps->frame_cropping_flag)
  {
    crop_left   = sps->frame_cropping_rect_left_offset;
    crop_right  = sps->frame_cropping_rect_right_offset;
    crop_top    = ( 2 - sps->frame_mbs_only_flag ) *  sps->frame_cropping_rect_top_offset;
    crop_bottom = ( 2 - sps->frame_mbs_only_flag ) *  sps->frame_cropping_rect_bottom_offset;
  }
  else
  {
    crop_left = crop_right = crop_top = crop_bottom = 0;
  }

#if EXT3D
  if ((sps->chroma_format_idc==YUV400) && p_Inp->write_uv[sps->is_depth])
#else
  if ((sps->chroma_format_idc==YUV400) && p_Inp->write_uv)
#endif
  {
    source->width[1]  = (source->width[0] >> 1);
    source->width[2]  = source->width[1];
    source->height[1] = (source->height[0] >> 1);
    source->height[2] = source->height[1];
  }
  else
  {
    source->width[1]  = p_Vid->width_cr - crop_left - crop_right;
    source->width[2]  = source->width[1];
    source->height[1] = p_Vid->height_cr - crop_top - crop_bottom;
    source->height[2] = source->height[1];
  }

  output->width[0]  = p_Vid->width;
#if EXT3D
  output->width[1] = p_Vid->width_cr;
  output->width[2] = p_Vid->width_cr;
#else
  source->width[1]  = p_Vid->width_cr;
  source->width[2]  = p_Vid->width_cr;
#endif
  output->height[0] = p_Vid->height;
  output->height[1] = p_Vid->height_cr;
  output->height[2] = p_Vid->height_cr;

  source->size_cmp[0] = source->width[0] * source->height[0];
  source->size_cmp[1] = source->width[1] * source->height[1];
  source->size_cmp[2] = source->size_cmp[1];
  source->size        = source->size_cmp[0] + source->size_cmp[1] + source->size_cmp[2];
  source->mb_width    = source->width[0]  / MB_BLOCK_SIZE;
  source->mb_height   = source->height[0] / MB_BLOCK_SIZE;

  // output size (excluding padding)
  output->size_cmp[0] = output->width[0] * output->height[0];
  output->size_cmp[1] = output->width[1] * output->height[1];
  output->size_cmp[2] = output->size_cmp[1];
  output->size        = output->size_cmp[0] + output->size_cmp[1] + output->size_cmp[2];
  output->mb_width    = output->width[0]  / MB_BLOCK_SIZE;
  output->mb_height   = output->height[0] / MB_BLOCK_SIZE;


  output->bit_depth[0] = source->bit_depth[0] = p_Vid->bitdepth_luma;
  output->bit_depth[1] = source->bit_depth[1] = p_Vid->bitdepth_chroma;
  output->bit_depth[2] = source->bit_depth[2] = p_Vid->bitdepth_chroma;  
  output->pic_unit_size_on_disk = (imax(output->bit_depth[0], output->bit_depth[1]) > 8) ? 16 : 8;
  output->pic_unit_size_shift3 = output->pic_unit_size_on_disk >> 3;

  output->frame_rate  = source->frame_rate;
  output->color_model = source->color_model;
  output->yuv_format  = source->yuv_format = (ColorFormat) sps->chroma_format_idc;

  output->auto_crop_bottom    = crop_bottom;
  output->auto_crop_right     = crop_right;
  output->auto_crop_bottom_cr = (crop_bottom * p_Vid->mb_cr_size_y) / MB_BLOCK_SIZE;
  output->auto_crop_right_cr  = (crop_right * p_Vid->mb_cr_size_x) / MB_BLOCK_SIZE;

  source->auto_crop_bottom    = output->auto_crop_bottom;
  source->auto_crop_right     = output->auto_crop_right;
  source->auto_crop_bottom_cr = output->auto_crop_bottom_cr;
  source->auto_crop_right_cr  = output->auto_crop_right_cr;

  updateMaxValue(source);
  updateMaxValue(output);
}

/*!
 ************************************************************************
 * \brief
 *    Activate Sequence Parameter Sets
 *
 ************************************************************************
 */
#if EXT3D
void activate_sps (VideoParameters *p_Vid, seq_parameter_set_rbsp_t *sps,int is_depth)
#else
void activate_sps (VideoParameters *p_Vid, seq_parameter_set_rbsp_t *sps)
#endif
{
  InputParameters *p_Inp = p_Vid->p_Inp;  
#if EXT3D
  if (sps->is_depth!=is_depth)
  {
    sps->is_depth=is_depth;
  }
#endif

  if (p_Vid->active_sps != sps)
  {
    if (p_Vid->dec_picture)
    {
      // this may only happen on slice loss
      exit_picture(p_Vid, &p_Vid->dec_picture);
    }
    p_Vid->active_sps = sps;

#if EXT3D
    p_Vid->profile_idc=sps->profile_idc;
    p_Vid->separate_colour_plane_flag=sps->separate_colour_plane_flag;
    if( p_Vid->separate_colour_plane_flag )
    {
      p_Vid->ChromaArrayType = 0;
    }
    else
    {
      p_Vid->ChromaArrayType = sps->chroma_format_idc;
    }
#endif

    p_Vid->bitdepth_chroma = 0;
    p_Vid->width_cr        = 0;
    p_Vid->height_cr       = 0;

    // maximum vertical motion vector range in luma quarter pixel units
    if (p_Vid->active_sps->level_idc <= 10)
    {
      p_Vid->max_vmv_r = 64 * 4;
    }
    else if (p_Vid->active_sps->level_idc <= 20)
    {
      p_Vid->max_vmv_r = 128 * 4;
    }
    else if (p_Vid->active_sps->level_idc <= 30)
    {
      p_Vid->max_vmv_r = 256 * 4;
    }
    else
    {
      p_Vid->max_vmv_r = 512 * 4; // 512 pixels in quarter pixels
    }

    // Fidelity Range Extensions stuff (part 1)
    p_Vid->bitdepth_luma       = (short) (sps->bit_depth_luma_minus8 + 8);
    p_Vid->bitdepth_scale[0]   = 1 << sps->bit_depth_luma_minus8;
    if (sps->chroma_format_idc != YUV400)
    {
      p_Vid->bitdepth_chroma   = (short) (sps->bit_depth_chroma_minus8 + 8);
      p_Vid->bitdepth_scale[1] = 1 << sps->bit_depth_chroma_minus8;
    }

    p_Vid->MaxFrameNum = 1<<(sps->log2_max_frame_num_minus4+4);
    p_Vid->PicWidthInMbs = (sps->pic_width_in_mbs_minus1 +1);
    p_Vid->PicHeightInMapUnits = (sps->pic_height_in_map_units_minus1 +1);
    p_Vid->FrameHeightInMbs = ( 2 - sps->frame_mbs_only_flag ) * p_Vid->PicHeightInMapUnits;
    p_Vid->FrameSizeInMbs = p_Vid->PicWidthInMbs * p_Vid->FrameHeightInMbs;

#if EXT3D
    if (is_depth==0)
    {
      p_Vid->text_width=p_Vid->PicWidthInMbs*MB_BLOCK_SIZE;
      p_Vid->text_height=p_Vid->FrameHeightInMbs*MB_BLOCK_SIZE;
      if (sps->chroma_format_idc == YUV420)
      {
        p_Vid->text_width_cr  = (p_Vid->text_width  >> 1);
        p_Vid->text_height_cr = (p_Vid->text_height >> 1);
      }
      else if (sps->chroma_format_idc == YUV422)
      {
        p_Vid->text_width_cr  = (p_Vid->text_width >> 1);
        p_Vid->text_height_cr = p_Vid->text_height;
      }
      else if (sps->chroma_format_idc== YUV444)
      {
        //YUV444
        p_Vid->text_width_cr = p_Vid->text_width;
        p_Vid->text_height_cr = p_Vid->text_height;
      }
      if (sps->profile_idc==ThreeDV_EXTEND_HIGH)
        memcpy(p_Vid->text_view_coding_order,p_Vid->active_subset_sps->view_id,(p_Vid->active_subset_sps->num_views_minus1+1)*sizeof(int));
#if ITRI_INTERLACE
      if ( (sps->profile_idc!=ThreeDV_HIGH) && (sps->profile_idc!=ThreeDV_EXTEND_HIGH) )
        p_Vid->is_texture_frame = sps->frame_mbs_only_flag;
#endif
    }
    else
    {
      int SubWidthC  [4]= { 1, 2, 2, 1};
      int SubHeightC [4]= { 1, 2, 1, 1};

      if (sps->profile_idc==ThreeDV_EXTEND_HIGH && p_Vid->active_sps_3dv_extension->reduced_resolution)
      {
        p_Vid->PicWidthInMbs = (p_Vid->active_sps_3dv_extension->depth_pic_width_in_mbs_minus1 +1);
        p_Vid->PicHeightInMapUnits = (p_Vid->active_sps_3dv_extension->depth_pic_height_in_map_units_minus1 +1);
        p_Vid->FrameHeightInMbs = ( 2 - sps->frame_mbs_only_flag ) * p_Vid->PicHeightInMapUnits;
        p_Vid->FrameSizeInMbs = p_Vid->PicWidthInMbs * p_Vid->FrameHeightInMbs;

      }

      p_Vid->depth_width  = p_Vid->PicWidthInMbs * MB_BLOCK_SIZE ;
      p_Vid->depth_height = p_Vid->FrameHeightInMbs * MB_BLOCK_SIZE ;


      if (sps->profile_idc==ThreeDV_EXTEND_HIGH)
      {
        p_Vid->depth_hor_mult=p_Vid->active_sps_3dv_extension->depth_hor_mult_minus1 + 1;
        p_Vid->depth_ver_mult=p_Vid->active_sps_3dv_extension->depth_ver_mult_minus1 + 1;
        p_Vid->depth_ver_rsh=p_Vid->active_sps_3dv_extension->depth_ver_rsh;
        p_Vid->depth_hor_rsh=p_Vid->active_sps_3dv_extension->depth_hor_rsh;

        p_Vid->depth_frame_cropping_flag=p_Vid->active_sps_3dv_extension->depth_frame_cropping_flag;
      }

      if (sps->profile_idc==ThreeDV_EXTEND_HIGH && p_Vid->active_sps_3dv_extension->depth_frame_cropping_flag)
      {
        sps->frame_cropping_flag=1;
        sps->frame_cropping_rect_left_offset=p_Vid->active_sps_3dv_extension->depth_frame_crop_left_offset;
        sps->frame_cropping_rect_right_offset=p_Vid->active_sps_3dv_extension->depth_frame_crop_right_offset;
        sps->frame_cropping_rect_top_offset=p_Vid->active_sps_3dv_extension->depth_frame_crop_top_offset;
        sps->frame_cropping_rect_bottom_offset=p_Vid->active_sps_3dv_extension->depth_frame_crop_bottom_offset;

        p_Vid->depth_frame_crop_left_offset=p_Vid->active_sps_3dv_extension->depth_frame_crop_left_offset;
        p_Vid->depth_frame_crop_right_offset=p_Vid->active_sps_3dv_extension->depth_frame_crop_right_offset;
        p_Vid->depth_frame_crop_top_offset=p_Vid->active_sps_3dv_extension->depth_frame_crop_top_offset;
        p_Vid->depth_frame_crop_bottom_offset=p_Vid->active_sps_3dv_extension->depth_frame_crop_bottom_offset;

        p_Vid->DepthCropLeftCoord=p_Vid->depth_frame_crop_left_offset * SubWidthC[sps->chroma_format_idc];
        p_Vid->DepthCropTopCoord=p_Vid->depth_frame_crop_top_offset * SubHeightC[sps->chroma_format_idc] *(2- sps->frame_mbs_only_flag);
        p_Vid->DepthCropRightCoord= p_Vid->PicWidthInMbs *16 - 1 - p_Vid->depth_frame_crop_right_offset * SubWidthC[sps->chroma_format_idc];
        p_Vid->DepthCropBottomCoord= p_Vid->FrameHeightInMbs *16 - 1- p_Vid->depth_frame_crop_bottom_offset * SubHeightC[sps->chroma_format_idc] *(2- sps->frame_mbs_only_flag);
      }else if (sps->frame_cropping_flag)
      {
        p_Vid->DepthCropLeftCoord=sps->frame_cropping_rect_left_offset * SubWidthC[sps->chroma_format_idc];
        p_Vid->DepthCropTopCoord=sps->frame_cropping_rect_top_offset * SubHeightC[sps->chroma_format_idc] *(2- sps->frame_mbs_only_flag);
        p_Vid->DepthCropRightCoord=  p_Vid->PicWidthInMbs *16 - 1 - sps->frame_cropping_rect_right_offset * SubWidthC[sps->chroma_format_idc];
        p_Vid->DepthCropBottomCoord= p_Vid->FrameHeightInMbs *16 - 1- sps->frame_cropping_rect_bottom_offset * SubHeightC[sps->chroma_format_idc] *(2- sps->frame_mbs_only_flag);
      }
      else
      {
        p_Vid->DepthCropLeftCoord=p_Vid->DepthCropTopCoord=0;
        p_Vid->DepthCropRightCoord=p_Vid->PicWidthInMbs*16-1;
        p_Vid->DepthCropBottomCoord=p_Vid->FrameHeightInMbs*16-1;
      }

      if (sps->profile_idc==ThreeDV_EXTEND_HIGH)
      {
        p_Vid->grid_pos_num_views=p_Vid->active_sps_3dv_extension->grid_pos_num_views;
        memcpy(p_Vid->grid_pos_x,p_Vid->active_sps_3dv_extension->grid_pos_x,sizeof(p_Vid->grid_pos_x));
        memcpy(p_Vid->grid_pos_y,p_Vid->active_sps_3dv_extension->grid_pos_y,sizeof(p_Vid->grid_pos_y));
      }

      p_Vid->enable_mvd_format=1;
      p_Vid->max_depth_map_value=1<<(p_Vid->active_subset_sps->sps.bit_depth_luma_minus8+8);

      if (sps->chroma_format_idc == YUV420)
      {
        p_Vid->depth_width_cr  = (p_Vid->depth_width  >> 1);
        p_Vid->depth_height_cr = (p_Vid->depth_height >> 1);
      }
      else if (sps->chroma_format_idc == YUV422)
      {
        p_Vid->depth_width_cr  = (p_Vid->depth_width >> 1);
        p_Vid->depth_height_cr = p_Vid->depth_height;
      }
      else if (sps->chroma_format_idc== YUV444)
      {
        //YUV444
        p_Vid->depth_width_cr = p_Vid->depth_width;
        p_Vid->depth_height_cr = p_Vid->depth_height;
      }

      memcpy(p_Vid->depth_view_coding_order,p_Vid->active_subset_sps->view_id,(p_Vid->active_subset_sps->num_views_minus1+1)*sizeof(int));
    }
#endif

    p_Vid->yuv_format=sps->chroma_format_idc;

    p_Vid->width = p_Vid->PicWidthInMbs * MB_BLOCK_SIZE;
    p_Vid->height = p_Vid->FrameHeightInMbs * MB_BLOCK_SIZE;



    
    if (sps->chroma_format_idc == YUV420)
    {
      p_Vid->width_cr  = (p_Vid->width  >> 1);
      p_Vid->height_cr = (p_Vid->height >> 1);
    }
    else if (sps->chroma_format_idc == YUV422)
    {
      p_Vid->width_cr  = (p_Vid->width >> 1);
      p_Vid->height_cr = p_Vid->height;
      p_Vid->iChromaPadY = MCBUF_CHROMA_PAD_Y*2;
    }
    else if (sps->chroma_format_idc == YUV444)
    {
      //YUV444
      p_Vid->width_cr = p_Vid->width;
      p_Vid->height_cr = p_Vid->height;
      p_Vid->iChromaPadX = p_Vid->iLumaPadX;
      p_Vid->iChromaPadY = p_Vid->iLumaPadY;
    }

#if EXT3D
    if ((!is_depth)&&(p_Vid->last_pic_width_in_mbs_minus1 != (int)p_Vid->active_sps->pic_width_in_mbs_minus1
      || p_Vid->last_pic_height_in_map_units_minus1 != (int)p_Vid->active_sps->pic_height_in_map_units_minus1
      || p_Vid->last_max_dec_frame_buffering != GetMaxDecFrameBuffering(p_Vid)
      || (p_Vid->last_profile_idc != (int)p_Vid->active_sps->profile_idc &&p_Vid->active_sps->profile_idc < MVC_HIGH && p_Vid->last_profile_idc< MVC_HIGH)))
    {
      assert(!is_depth);  //!<base view for texture
      init_frext(p_Vid);
      init_text_global_buffers(p_Vid);

      if (!p_Vid->no_output_of_prior_pics_flag)
      {
        flush_dpb(p_Vid->p_Dpb[0], -1);
        flush_dpb(p_Vid->p_Dpb[1], -1);
      }

      init_dpb(p_Vid, p_Vid->p_Dpb[is_depth]);

      p_Vid->last_pic_width_in_mbs_minus1 = p_Vid->active_sps->pic_width_in_mbs_minus1;  
      p_Vid->last_pic_height_in_map_units_minus1 = p_Vid->active_sps->pic_height_in_map_units_minus1;
      p_Vid->last_max_dec_frame_buffering = GetMaxDecFrameBuffering(p_Vid);
      p_Vid->last_profile_idc = p_Vid->active_sps->profile_idc;

      p_Vid->height_ori= (p_Vid->active_sps->pic_height_in_map_units_minus1+1)*MB_BLOCK_SIZE;
      p_Vid->width_ori = (p_Vid->active_sps->pic_width_in_mbs_minus1+1)*MB_BLOCK_SIZE;
#if ITRI_INTERLACE
    if ( (p_Vid->active_sps->is_depth==0) && (p_Vid->active_sps->profile_idc!=ThreeDV_HIGH) && (p_Vid->active_sps->profile_idc!=ThreeDV_EXTEND_HIGH) )
      p_Vid->is_texture_frame = sps->frame_mbs_only_flag;
#endif
    }
    else if(p_Vid->last_profile_idc != (int)p_Vid->active_sps->profile_idc && (p_Vid->last_profile_idc>=MVC_HIGH || p_Vid->active_sps->profile_idc >= MVC_HIGH)&& (!is_depth)&&p_Vid->p_Dpb[is_depth]->init_done)
    {
      //!<non-base view for texture
      re_init_dpb(p_Vid, p_Vid->p_Dpb[is_depth]);
      p_Vid->last_profile_idc = p_Vid->active_sps->profile_idc;
    }
    else if(is_depth&&(!p_Vid->p_Dpb[is_depth]->init_done))
    {
      //!<all views for depth
      init_frext(p_Vid);
      init_depth_global_buffers(p_Vid);
      init_dpb(p_Vid,p_Vid->p_Dpb[is_depth]);
    }
    else
    {
      init_frext(p_Vid);
    }

    p_Vid->p_Dpb[is_depth]->num_ref_frames = p_Vid->active_sps->num_ref_frames;
    set_picture_global_buffers(p_Vid);
#else
#if (MVC_EXTENSION_ENABLE)
    if (p_Vid->last_pic_width_in_mbs_minus1 != p_Vid->active_sps->pic_width_in_mbs_minus1
      || p_Vid->last_pic_height_in_map_units_minus1 != p_Vid->active_sps->pic_height_in_map_units_minus1
      || p_Vid->last_max_dec_frame_buffering != GetMaxDecFrameBuffering(p_Vid)
      || (p_Vid->last_profile_idc != p_Vid->active_sps->profile_idc && p_Vid->active_sps->profile_idc < MVC_HIGH && p_Vid->last_profile_idc< MVC_HIGH))
    {
      init_frext(p_Vid);
      init_global_buffers(p_Vid);

      if (!p_Vid->no_output_of_prior_pics_flag)
      {
        flush_dpb(p_Vid->p_Dpb, -1);
      }
      
      init_dpb(p_Vid, p_Vid->p_Dpb);

      p_Vid->last_pic_width_in_mbs_minus1 = p_Vid->active_sps->pic_width_in_mbs_minus1;  
      p_Vid->last_pic_height_in_map_units_minus1 = p_Vid->active_sps->pic_height_in_map_units_minus1;
      p_Vid->last_max_dec_frame_buffering = GetMaxDecFrameBuffering(p_Vid);
      p_Vid->last_profile_idc = p_Vid->active_sps->profile_idc;
    }
    else if(p_Vid->last_profile_idc != p_Vid->active_sps->profile_idc && (p_Vid->last_profile_idc>=MVC_HIGH || p_Vid->active_sps->profile_idc >= MVC_HIGH)&& p_Vid->p_Dpb->init_done /*&& !(p_Vid->init_bl_done)*/)
    {
      re_init_dpb(p_Vid, p_Vid->p_Dpb);
      p_Vid->last_profile_idc = p_Vid->active_sps->profile_idc;
    }

    p_Vid->p_Dpb->num_ref_frames = p_Vid->active_sps->num_ref_frames;
#else
    init_frext(p_Vid);
    init_global_buffers(p_Vid);

    if (!p_Vid->no_output_of_prior_pics_flag)
    {
      flush_dpb(p_Vid->p_Dpb);
    }
    init_dpb(p_Vid, p_Vid->p_Dpb);

#endif
#endif


    ercInit(p_Vid, p_Vid->width, p_Vid->height, 1);
    if(p_Vid->dec_picture)
    {

      ercReset(p_Vid->erc_errorVar, p_Vid->PicSizeInMbs, p_Vid->PicSizeInMbs, p_Vid->dec_picture->size_x);
      p_Vid->erc_mvperMB = 0;
    }
  }

  reset_format_info(sps, p_Vid, &p_Inp->source, &p_Inp->output);

}

void activate_pps(VideoParameters *p_Vid, pic_parameter_set_rbsp_t *pps)
{  
  if (p_Vid->active_pps != pps)
  {
    if (p_Vid->dec_picture) // && p_Vid->num_dec_mb == p_Vid->pi)
    {
      // this may only happen on slice loss
      exit_picture(p_Vid, &p_Vid->dec_picture);
    }

    p_Vid->active_pps = pps;
  }
}

#if EXT3D
void activate_dps(VideoParameters *p_Vid, depth_parameter_set_rbsp_t *dps) 
{   
  unsigned int i,j; 
  int voidx; 
  struct decoded_picture_buffer *p_Dpb; 
//  seq_parameter_set_3dv_extension *active_3dv_subset_sps = p_Vid->active_sps_3dv_extension; 
  p_Vid->active_dps = dps; 
  get_camera_depth_range_info(p_Vid);

  for(i=0;i<2;++i) 
  { 
    p_Dpb = p_Vid->p_Dpb[i]; 
    for(j=0;j<p_Vid->p_Dpb[0]->used_size;++j)
    {
      if(p_Dpb->fs[j]->poc==p_Vid->ThisPOC && p_Vid->p_Dpb[i]->fs[j]->view_id!=-1)
      { 
        voidx=GetVOIdx(p_Vid,p_Dpb->fs[j]->view_id);
        p_Dpb->fs[j]->frame->depth_near     = p_Vid->dec_depth_near[voidx]; 
        p_Dpb->fs[j]->frame->depth_far      = p_Vid->dec_depth_far[voidx]; 
      } 
    } 
  }
}

void UseDepParameterSet (Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  int DepParSetId;  
  depth_parameter_set_rbsp_t *dps;

  if(p_Vid->acquisition_idx!=1 && (currSlice->depth_based_mvp_flag || currSlice->Harmonize_VSP_IVP || currSlice->depth_range_wp))
  {
    if(currSlice->dep_parameter_set_id==0){
      if(p_Vid->active_dps!=NULL){
        p_Vid->active_dps = NULL;
        get_camera_depth_range_info(p_Vid);
      }
    }

    else
    {
      DepParSetId = currSlice->dep_parameter_set_id;
      dps = p_Vid->DepParSet[DepParSetId];
      if (dps->Valid != TRUE)
        printf ("Trying to use an invalid (uninitialized) Depth Range Parameter Set with ID %d, expect the unexpected...\n", DepParSetId);

      if((p_Vid->active_dps != dps)){
        activate_dps(p_Vid, dps);
      }
    }
  }
}
#endif

void UseParameterSet (Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  int PicParsetId = currSlice->pic_parameter_set_id;  
  pic_parameter_set_rbsp_t *pps = &p_Vid->PicParSet[PicParsetId];
  seq_parameter_set_rbsp_t *sps = &p_Vid->SeqParSet[pps->seq_parameter_set_id];
  int i;

  if (pps->Valid != TRUE)
    printf ("Trying to use an invalid (uninitialized) Picture Parameter Set with ID %d, expect the unexpected...\n", PicParsetId);
#if MVC_EXTENSION_ENABLE||EXT3D
  if((currSlice->svc_extension_flag == -1))
  {
    if (sps->Valid != TRUE)
      printf ("PicParset %d references an invalid (uninitialized) Sequence Parameter Set with ID %d, expect the unexpected...\n", 
      PicParsetId, (int) pps->seq_parameter_set_id);
  }
  else
  {
    // Set SPS to the subset SPS parameters
    p_Vid->active_subset_sps = p_Vid->SubsetSeqParSet + pps->seq_parameter_set_id;

    sps = &(p_Vid->active_subset_sps->sps);
    if (p_Vid->SubsetSeqParSet[pps->seq_parameter_set_id].Valid != TRUE)
      printf ("PicParset %d references an invalid (uninitialized) Subset Sequence Parameter Set with ID %d, expect the unexpected...\n", 
      PicParsetId, (int) pps->seq_parameter_set_id);    
#if EXT3D
    p_Vid->active_sps_3dv_extension=p_Vid->SeqParSet3DVExtension+pps->seq_parameter_set_id;
    if(!p_Vid->active_sps_3dv_extension->valid)
      p_Vid->active_sps_3dv_extension=NULL;
#endif

  }
#else
  if (sps->Valid != TRUE)
    printf ("PicParset %d references an invalid (uninitialized) Sequence Parameter Set with ID %d, expect the unexpected...\n", 
    PicParsetId, (int) pps->seq_parameter_set_id);
#endif

  // In theory, and with a well-designed software, the lines above
  // are everything necessary.  In practice, we need to patch many values
  // in p_Vid-> (but no more in p_Inp-> -- these have been taken care of)

  // Set Sequence Parameter Stuff first
  //  printf ("Using Picture Parameter set %d and associated Sequence Parameter Set %d\n", PicParsetId, pps->seq_parameter_set_id);
  if ((int) sps->pic_order_cnt_type < 0 || sps->pic_order_cnt_type > 2)  // != 1
  {
    printf ("invalid sps->pic_order_cnt_type = %d\n", (int) sps->pic_order_cnt_type);
    error ("pic_order_cnt_type != 1", -1000);
  }

  if (sps->pic_order_cnt_type == 1)
  {
    if(sps->num_ref_frames_in_pic_order_cnt_cycle >= MAXnum_ref_frames_in_pic_order_cnt_cycle)
    {
      error("num_ref_frames_in_pic_order_cnt_cycle too large",-1011);
    }
  }
#if EXT3D
  activate_sps(p_Vid, sps,currSlice->is_depth);
#else
  activate_sps(p_Vid, sps);
#endif
  activate_pps(p_Vid, pps);

  // currSlice->dp_mode is set by read_new_slice (NALU first byte available there)
  if (pps->entropy_coding_mode_flag == (Boolean)(CAVLC))
  {
    currSlice->nal_startcode_follows = uvlc_startcode_follows;
    for (i=0; i<3; i++)
    {
      currSlice->partArr[i].readSyntaxElement = readSyntaxElement_UVLC;      
    }
  }
  else
  {
    currSlice->nal_startcode_follows = cabac_startcode_follows;
    for (i=0; i<3; i++)
    {
      currSlice->partArr[i].readSyntaxElement = readSyntaxElement_CABAC;
    }
  }
}

#if MVC_EXTENSION_ENABLE||EXT3D
void seq_parameter_set_mvc_extension(subset_seq_parameter_set_rbsp_t *subset_sps, Bitstream *s)
{
  int i, j, num_views;

  subset_sps->num_views_minus1 = ue_v("num_views_minus1"                                  , s);
  num_views = 1+subset_sps->num_views_minus1;
  if( num_views >0)
  {
    if ((subset_sps->view_id = (int*) calloc(num_views, sizeof(int))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->view_id");
#if EXT3D // @DT: Unpaired MVD
    if ((subset_sps->depth_view_present_flag = (int*) calloc(num_views, sizeof(int))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->depth_view_present_flag");
    if ((subset_sps->texture_view_present_flag = (int*) calloc(num_views, sizeof(int))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->texture_view_present_flag");
#endif
    if ((subset_sps->num_anchor_refs_l0 = (int*) calloc(num_views, sizeof(int))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->num_anchor_refs_l0");
    if ((subset_sps->num_anchor_refs_l1 = (int*) calloc(num_views, sizeof(int))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->num_anchor_refs_l1");
    if ((subset_sps->anchor_ref_l0 = (int**) calloc(num_views, sizeof(int*))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->anchor_ref_l0");
    if ((subset_sps->anchor_ref_l1 = (int**) calloc(num_views, sizeof(int*))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->anchor_ref_l1");
    if ((subset_sps->num_non_anchor_refs_l0 = (int*) calloc(num_views, sizeof(int))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->num_non_anchor_refs_l0");      
    if ((subset_sps->num_non_anchor_refs_l1 = (int*) calloc(num_views, sizeof(int))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->num_non_anchor_refs_l1");
    if ((subset_sps->non_anchor_ref_l0 = (int**) calloc(num_views, sizeof(int*))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->non_anchor_ref_l0");
    if ((subset_sps->non_anchor_ref_l1 = (int**) calloc(num_views, sizeof(int*))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->non_anchor_ref_l1");

  }
#if EXT3D
  subset_sps->num_coded_depth_view = 0;
#endif
  for(i=0; i<num_views; i++)
  {
    subset_sps->view_id[i] = ue_v("view_id"                                        , s);
#if EXT3D // @DT: Unpaired MVD
    if (subset_sps->sps.profile_idc==ThreeDV_HIGH)
    {
      subset_sps->depth_view_present_flag[i] = u_1 ("depth_view_present_flag"          , s);
      subset_sps->num_coded_depth_view += subset_sps->depth_view_present_flag[i];
      subset_sps->texture_view_present_flag[i] = u_1 ("texture_view_present_flag"          , s);
    }
#endif
  }
  for(i=1; i<num_views; i++)
  {

#if EXT3D // @DT: Unpaired MVD
    if (subset_sps->sps.profile_idc==ThreeDV_HIGH && subset_sps->depth_view_present_flag[i]==0) 
      continue;
#endif
    subset_sps->num_anchor_refs_l0[i] = ue_v("num_anchor_refs_l0"                            , s);
    if(subset_sps->num_anchor_refs_l0[i]>0)
    {
      if ((subset_sps->anchor_ref_l0[i] = (int*) calloc(subset_sps->num_anchor_refs_l0[i], sizeof(int))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->anchor_ref_l0[i]");
      for(j=0; j<subset_sps->num_anchor_refs_l0[i]; j++)
        subset_sps->anchor_ref_l0[i][j] = ue_v("anchor_ref_l0"                            , s);
    }

    subset_sps->num_anchor_refs_l1[i] = ue_v("num_anchor_refs_l1"                            , s);
    if(subset_sps->num_anchor_refs_l1[i]>0)
    {
      if ((subset_sps->anchor_ref_l1[i] = (int*) calloc(subset_sps->num_anchor_refs_l1[i], sizeof(int))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->anchor_ref_l1[i]");
      for(j=0; j<subset_sps->num_anchor_refs_l1[i]; j++)
        subset_sps->anchor_ref_l1[i][j] = ue_v("anchor_ref_l1"                            , s);
    }
  }
  for(i=1; i<num_views; i++)
  {

#if EXT3D // @DT: Unpaired MVD
    if (subset_sps->sps.profile_idc==ThreeDV_HIGH && subset_sps->depth_view_present_flag[i]==0) 
      continue;
#endif
    subset_sps->num_non_anchor_refs_l0[i] = ue_v("num_non_anchor_refs_l0"                        , s);
    if(subset_sps->num_non_anchor_refs_l0[i]>0)
    {
      if ((subset_sps->non_anchor_ref_l0[i] = (int*) calloc(subset_sps->num_non_anchor_refs_l0[i], sizeof(int))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->non_anchor_ref_l0[i]");
      for(j=0; j<subset_sps->num_non_anchor_refs_l0[i]; j++)
        subset_sps->non_anchor_ref_l0[i][j] = ue_v("non_anchor_ref_l0"                        , s);
    }
    subset_sps->num_non_anchor_refs_l1[i] = ue_v("num_non_anchor_refs_l1"                        , s);
    if(subset_sps->num_non_anchor_refs_l1[i]>0)
    {
      if ((subset_sps->non_anchor_ref_l1[i] = (int*) calloc(subset_sps->num_non_anchor_refs_l1[i], sizeof(int))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->non_anchor_ref_l1[i]");
      for(j=0; j<subset_sps->num_non_anchor_refs_l1[i]; j++)
        subset_sps->non_anchor_ref_l1[i][j] = ue_v("non_anchor_ref_l1"                        , s);
    }
  }


  subset_sps->num_level_values_signalled_minus1 = ue_v("num_level_values_signalled_minus1"                , s);
  if(subset_sps->num_level_values_signalled_minus1 >=0)
  {
    i = 1+ subset_sps->num_level_values_signalled_minus1;
    if ((subset_sps->level_idc = (int*) calloc(i, sizeof(int))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->level_idc");
    if ((subset_sps->num_applicable_ops_minus1 = (int*) calloc(i, sizeof(int))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->num_applicable_ops_minus1");
    if ((subset_sps->applicable_op_temporal_id = (int**) calloc(i, sizeof(int*))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_temporal_id");
    if ((subset_sps->applicable_op_num_target_views_minus1 = (int**) calloc(i, sizeof(int*))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_num_target_views_minus1");
    if ((subset_sps->applicable_op_target_view_id = (int***) calloc(i, sizeof(int**))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_target_view_id");
#if EXT3D
    if ((subset_sps->applicable_op_depth_flag = (int***) calloc(i, sizeof(int**))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_target_view_id");
    if ((subset_sps->applicable_op_texture_flag = (int***) calloc(i, sizeof(int**))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_target_view_id");
    if (subset_sps->sps.profile_idc==ThreeDV_HIGH || subset_sps->sps.profile_idc==ThreeDV_EXTEND_HIGH)
    {
      if ((subset_sps->applicable_op_num_texture_views_minus1 = (int**) calloc(i, sizeof(int*))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_num_texture_views_minus1");
      if ((subset_sps->applicable_op_num_depth_views = (int**) calloc(i, sizeof(int*))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_num_depth_views");
    }
    else
#endif
    if ((subset_sps->applicable_op_num_views_minus1 = (int**) calloc(i, sizeof(int*))) == NULL)
      no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_num_views_minus1");
  }
  for(i=0; i<=subset_sps->num_level_values_signalled_minus1; i++)
  {
    subset_sps->level_idc[i] = u_v(8, "level_idc"                                    , s);
    subset_sps->num_applicable_ops_minus1[i] = ue_v("num_applicable_ops_minus1"                      , s);
    if(subset_sps->num_applicable_ops_minus1[i]>=0)
    {
      if ((subset_sps->applicable_op_temporal_id[i] = (int*) calloc(1+subset_sps->num_applicable_ops_minus1[i], sizeof(int))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_temporal_id[i]");
      if ((subset_sps->applicable_op_num_target_views_minus1[i] = (int*) calloc(1+subset_sps->num_applicable_ops_minus1[i], sizeof(int))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_num_target_views_minus1[i]");
      if ((subset_sps->applicable_op_target_view_id[i] = (int**) calloc(1+subset_sps->num_applicable_ops_minus1[i], sizeof(int *))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_target_view_id[i]");
#if EXT3D
      if ((subset_sps->applicable_op_depth_flag[i] = (int**) calloc(1+subset_sps->num_applicable_ops_minus1[i], sizeof(int *))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_target_view_id[i]");
      if ((subset_sps->applicable_op_texture_flag[i] = (int**) calloc(1+subset_sps->num_applicable_ops_minus1[i], sizeof(int *))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_target_view_id[i]");
      if (subset_sps->sps.profile_idc==ThreeDV_HIGH || subset_sps->sps.profile_idc==ThreeDV_EXTEND_HIGH)
      {
        if ((subset_sps->applicable_op_num_texture_views_minus1[i] = (int*) calloc(1+subset_sps->num_applicable_ops_minus1[i], sizeof(int))) == NULL)
          no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_num_texture_views_minus1[i]");
        if ((subset_sps->applicable_op_num_depth_views[i] = (int*) calloc(1+subset_sps->num_applicable_ops_minus1[i], sizeof(int))) == NULL)
          no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_num_depth_views[i]");
      }
      else
#endif
      if ((subset_sps->applicable_op_num_views_minus1[i] = (int*) calloc(1+subset_sps->num_applicable_ops_minus1[i], sizeof(int))) == NULL)
        no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_num_views_minus1[i]");

      for(j=0; j<=subset_sps->num_applicable_ops_minus1[i]; j++)
      {
        int k;
        subset_sps->applicable_op_temporal_id[i][j] = u_v(3, "applicable_op_temporal_id"              , s);
        subset_sps->applicable_op_num_target_views_minus1[i][j] = ue_v("applicable_op_num_target_views_minus1"    , s);
        if(subset_sps->applicable_op_num_target_views_minus1[i][j]>=0)
        {
#if EXT3D
            if ((subset_sps->applicable_op_target_view_id[i][j] = (int*) calloc(1+subset_sps->applicable_op_num_target_views_minus1[i][j], sizeof(int))) == NULL)
              no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_target_view_id[i][j]");
            if ((subset_sps->applicable_op_depth_flag[i][j] = (int*) calloc(1+subset_sps->applicable_op_num_target_views_minus1[i][j], sizeof(int))) == NULL)
              no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_target_view_id[i][j]");
            if ((subset_sps->applicable_op_texture_flag[i][j] = (int*) calloc(1+subset_sps->applicable_op_num_target_views_minus1[i][j], sizeof(int))) == NULL)
              no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_target_view_id[i][j]");
            for(k = 0; k <= subset_sps->applicable_op_num_target_views_minus1[i][j]; k++)
            {
              subset_sps->applicable_op_target_view_id[i][j][k] = ue_v("applicable_op_target_view_id"        , s);
              if (subset_sps->sps.profile_idc==ThreeDV_EXTEND_HIGH || subset_sps->sps.profile_idc==ThreeDV_HIGH)
              {
                subset_sps->applicable_op_depth_flag[i][j][k] = u_1("applicable_op_depth_flag"        , s);
                subset_sps->applicable_op_texture_flag[i][j][k] = u_1("applicable_op_texture_flag"        , s);
              }
            }
#else
          if ((subset_sps->applicable_op_target_view_id[i][j] = (int*) calloc(1+subset_sps->applicable_op_num_target_views_minus1[i][j], sizeof(int))) == NULL)
            no_mem_exit("init_subset_seq_parameter_set: subset_sps->applicable_op_target_view_id[i][j]");
          for(k = 0; k <= subset_sps->applicable_op_num_target_views_minus1[i][j]; k++)
            subset_sps->applicable_op_target_view_id[i][j][k] = ue_v("applicable_op_target_view_id"        , s);
#endif
        }
#if EXT3D
        if (subset_sps->sps.profile_idc==ThreeDV_HIGH || subset_sps->sps.profile_idc==ThreeDV_EXTEND_HIGH)
        {
          subset_sps->applicable_op_num_texture_views_minus1[i][j] = ue_v("applicable_op_num_texture_views_minus1"          , s);
          subset_sps->applicable_op_num_depth_views[i][j] = ue_v("applicable_op_num_depth_views"          , s);
        }
        else
#endif
        subset_sps->applicable_op_num_views_minus1[i][j] = ue_v("applicable_op_num_views_minus1"          , s);
      }
    }
  }
}

int MemAlloc1D(void** ppBuf, int iEleSize, int iNum)
{
  if(iEleSize*iNum <=0)
    return 1;

  *ppBuf = calloc(iNum, iEleSize);
  return (*ppBuf == NULL);
}

void hrd_parameters(MVCVUI_t *pMVCVUI, Bitstream *s)
{
  int i;

  pMVCVUI->cpb_cnt_minus1 = (char) ue_v("cpb_cnt_minus1"                                , s);
  assert(pMVCVUI->cpb_cnt_minus1<=31);
  pMVCVUI->bit_rate_scale = (char) u_v(4, "bit_rate_scale"                              , s);
  pMVCVUI->cpb_size_scale = (char) u_v(4, "cpb_size_scale"                              , s);
  for(i=0; i<=pMVCVUI->cpb_cnt_minus1; i++)
  {
    pMVCVUI->bit_rate_value_minus1[i] = ue_v("bit_rate_value_minus1"                    , s);
    pMVCVUI->cpb_size_value_minus1[i] = ue_v("cpb_size_value_minus1"                    , s);
    pMVCVUI->cbr_flag[i]              = (char) u_1 ("cbr_flag"                                 , s);
  }
  pMVCVUI->initial_cpb_removal_delay_length_minus1 = (char) u_v(5, "initial_cpb_removal_delay_length_minus1"      , s);
  pMVCVUI->cpb_removal_delay_length_minus1         = (char) u_v(5, "cpb_removal_delay_length_minus1"              , s);
  pMVCVUI->dpb_output_delay_length_minus1          = (char) u_v(5, "dpb_output_delay_length_minus1"              , s);
  pMVCVUI->time_offset_length                      = (char) u_v(5, "time_offset_length"                          , s);

}

void mvc_vui_parameters_extension(MVCVUI_t *pMVCVUI, Bitstream *s)
{
  int i, j, iNumOps;

  pMVCVUI->num_ops_minus1 = ue_v("vui_mvc_num_ops_minus1"                            , s);
  iNumOps = 1+ pMVCVUI->num_ops_minus1;
  if(iNumOps > 0)
  {
    MemAlloc1D((void **)&(pMVCVUI->temporal_id), sizeof(pMVCVUI->temporal_id[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->num_target_output_views_minus1), sizeof(pMVCVUI->num_target_output_views_minus1[0]), iNumOps);
    if ((pMVCVUI->view_id = (int**) calloc(iNumOps, sizeof(int*))) == NULL)
      no_mem_exit("mvc_vui_parameters_extension: pMVCVUI->view_id");
    MemAlloc1D((void **)&(pMVCVUI->timing_info_present_flag), sizeof(pMVCVUI->timing_info_present_flag[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->num_units_in_tick), sizeof(pMVCVUI->num_units_in_tick[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->time_scale), sizeof(pMVCVUI->time_scale[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->fixed_frame_rate_flag), sizeof(pMVCVUI->fixed_frame_rate_flag[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->nal_hrd_parameters_present_flag), sizeof(pMVCVUI->nal_hrd_parameters_present_flag[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->vcl_hrd_parameters_present_flag), sizeof(pMVCVUI->vcl_hrd_parameters_present_flag[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->low_delay_hrd_flag), sizeof(pMVCVUI->low_delay_hrd_flag[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->pic_struct_present_flag), sizeof(pMVCVUI->pic_struct_present_flag[0]), iNumOps);

    for(i=0; i<iNumOps; i++)
    {
      pMVCVUI->temporal_id[i] = (char) u_v(3, "vui_mvc_temporal_id"                          , s);
      pMVCVUI->num_target_output_views_minus1[i] = ue_v("vui_mvc_num_target_output_views_minus1"        , s);
      if(pMVCVUI->num_target_output_views_minus1[i] >= 0)
        MemAlloc1D((void **)&(pMVCVUI->view_id[i]), sizeof(pMVCVUI->view_id[0][0]), pMVCVUI->num_target_output_views_minus1[i]+1);
      for(j=0; j<=pMVCVUI->num_target_output_views_minus1[i]; j++)
        pMVCVUI->view_id[i][j] = ue_v("vui_mvc_view_id"                            , s);
      pMVCVUI->timing_info_present_flag[i] = (char) u_1("vui_mvc_timing_info_present_flag", s);
      if(pMVCVUI->timing_info_present_flag[i])
      {
        pMVCVUI->num_units_in_tick[i]     = u_v(32, "vui_mvc_num_units_in_tick"    , s); 
        pMVCVUI->time_scale[i]            = u_v(32, "vui_mvc_time_scale"          , s); 
        pMVCVUI->fixed_frame_rate_flag[i] = (char) u_1("vui_mvc_fixed_frame_rate_flag"    , s);
      }
      pMVCVUI->nal_hrd_parameters_present_flag[i] = (char) u_1("vui_mvc_nal_hrd_parameters_present_flag"        , s);
      if(pMVCVUI->nal_hrd_parameters_present_flag[i])
        hrd_parameters(pMVCVUI, s);
      pMVCVUI->vcl_hrd_parameters_present_flag[i] = (char) u_1("vcl_hrd_parameters_present_flag"            , s);
      if(pMVCVUI->vcl_hrd_parameters_present_flag[i])
        hrd_parameters(pMVCVUI, s);
      if(pMVCVUI->nal_hrd_parameters_present_flag[i]||pMVCVUI->vcl_hrd_parameters_present_flag[i])
        pMVCVUI->low_delay_hrd_flag[i]    = (char) u_1("vui_mvc_low_delay_hrd_flag"                  , s);
      pMVCVUI->pic_struct_present_flag[i] = (char) u_1("vui_mvc_pic_struct_present_flag"                , s);
    }
  }
}

#if EXT3D
void mvcd_vui_parameters_extension(MVCVUI_t *pMVCVUI, Bitstream *s)
{
  int i, j, iNumOps;
  
  pMVCVUI->is_mvcd_vui=1;

  pMVCVUI->num_ops_minus1 = ue_v("vui_mvc_num_ops_minus1"														, s);
  iNumOps = 1+ pMVCVUI->num_ops_minus1;
  if(iNumOps > 0)
    {
    MemAlloc1D((void **)&(pMVCVUI->temporal_id), sizeof(pMVCVUI->temporal_id[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->num_target_output_views_minus1), sizeof(pMVCVUI->num_target_output_views_minus1[0]), iNumOps);
    if ((pMVCVUI->view_id = (int**) calloc(iNumOps, sizeof(int*))) == NULL)
      no_mem_exit("mvc_vui_parameters_extension: pMVCVUI->view_id");
    if ((pMVCVUI->depth_flag = (int**) calloc(iNumOps, sizeof(int*))) == NULL)
      no_mem_exit("mvc_vui_parameters_extension: pMVCVUI->view_id");
    if ((pMVCVUI->texture_flag = (int**) calloc(iNumOps, sizeof(int*))) == NULL)
      no_mem_exit("mvc_vui_parameters_extension: pMVCVUI->view_id");

    MemAlloc1D((void **)&(pMVCVUI->timing_info_present_flag), sizeof(pMVCVUI->timing_info_present_flag[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->num_units_in_tick), sizeof(pMVCVUI->num_units_in_tick[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->time_scale), sizeof(pMVCVUI->time_scale[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->fixed_frame_rate_flag), sizeof(pMVCVUI->fixed_frame_rate_flag[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->nal_hrd_parameters_present_flag), sizeof(pMVCVUI->nal_hrd_parameters_present_flag[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->vcl_hrd_parameters_present_flag), sizeof(pMVCVUI->vcl_hrd_parameters_present_flag[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->low_delay_hrd_flag), sizeof(pMVCVUI->low_delay_hrd_flag[0]), iNumOps);
    MemAlloc1D((void **)&(pMVCVUI->pic_struct_present_flag), sizeof(pMVCVUI->pic_struct_present_flag[0]), iNumOps);

    for(i=0; i<iNumOps; i++)
      {
      pMVCVUI->temporal_id[i] = (char) u_v(3, "vui_mvc_temporal_id"													, s);
      pMVCVUI->num_target_output_views_minus1[i] = ue_v("vui_mvc_num_target_output_views_minus1"				, s);
      if(pMVCVUI->num_target_output_views_minus1[i] >= 0)
        {
        MemAlloc1D((void **)&(pMVCVUI->view_id[i]), sizeof(pMVCVUI->view_id[0][0]), pMVCVUI->num_target_output_views_minus1[i]+1);
        MemAlloc1D((void **)&(pMVCVUI->depth_flag[i]), sizeof(pMVCVUI->depth_flag[0][0]), pMVCVUI->num_target_output_views_minus1[i]+1);
        MemAlloc1D((void **)&(pMVCVUI->texture_flag[i]), sizeof(pMVCVUI->texture_flag[0][0]), pMVCVUI->num_target_output_views_minus1[i]+1);
        }

      for(j=0; j<=pMVCVUI->num_target_output_views_minus1[i]; j++)
        {
        pMVCVUI->view_id[i][j] = ue_v("vui_mvc_view_id"														, s);
        pMVCVUI->depth_flag[i][j] = u_1("vui_mvc_depth_flag"														, s);
        pMVCVUI->texture_flag[i][j] = u_1("vui_mvc_texture_flag"														, s);
        }

      pMVCVUI->timing_info_present_flag[i] = (char) u_1("vui_mvc_timing_info_present_flag", s);
      if(pMVCVUI->timing_info_present_flag[i])
        {
        pMVCVUI->num_units_in_tick[i]     = u_v(32, "vui_mvc_num_units_in_tick"		, s); 
        pMVCVUI->time_scale[i]            = u_v(32, "vui_mvc_time_scale"          , s); 
        pMVCVUI->fixed_frame_rate_flag[i] = (char) u_1("vui_mvc_fixed_frame_rate_flag"		, s);
        }
      pMVCVUI->nal_hrd_parameters_present_flag[i] = (char) u_1("vui_mvc_nal_hrd_parameters_present_flag"				, s);
      if(pMVCVUI->nal_hrd_parameters_present_flag[i])
        hrd_parameters(pMVCVUI, s);
      pMVCVUI->vcl_hrd_parameters_present_flag[i] = (char) u_1("vcl_hrd_parameters_present_flag"						, s);
      if(pMVCVUI->vcl_hrd_parameters_present_flag[i])
        hrd_parameters(pMVCVUI, s);
      if(pMVCVUI->nal_hrd_parameters_present_flag[i]||pMVCVUI->vcl_hrd_parameters_present_flag[i])
        pMVCVUI->low_delay_hrd_flag[i]    = (char) u_1("vui_mvc_low_delay_hrd_flag"									, s);
      pMVCVUI->pic_struct_present_flag[i] = (char) u_1("vui_mvc_pic_struct_present_flag"								, s);
      }
    }
  }
#endif

void init_subset_sps_list(subset_seq_parameter_set_rbsp_t *subset_sps_list, int iSize)
{
  int i;
  memset(subset_sps_list, 0, iSize*sizeof(subset_sps_list[0]));
  for(i=0; i<iSize; i++)
  {
    subset_sps_list[i].sps.seq_parameter_set_id = (unsigned int) -1;
    subset_sps_list[i].num_views_minus1 = -1;
    subset_sps_list[i].num_level_values_signalled_minus1 = -1;
    subset_sps_list[i].MVCVUIParams.num_ops_minus1 = -1;
#if EXT3D
    subset_sps_list[i].is_depth=0;
#endif
  }
}

#if EXT3D
void init_sps_3dv_extension_list(seq_parameter_set_3dv_extension *sps_3dv_extension_list, int iSize)
{
  int i=0;
  for(i=0;i<iSize;++i)
  {
    memset(&(sps_3dv_extension_list[i]),0,sizeof(seq_parameter_set_3dv_extension));
    get_mem_acquisition_info(&(sps_3dv_extension_list[i].threeDV_acquisition_in_subsps));
    init_acquisition_info(sps_3dv_extension_list[i].threeDV_acquisition_in_subsps);
  }
  
}
void reset_sps_3dv_extension(seq_parameter_set_3dv_extension *sps_3dv_extension)
{
    free_mem_acquisition_info(sps_3dv_extension->threeDV_acquisition_in_subsps);
}

void interpret_sps_3dv_extension(seq_parameter_set_3dv_extension *sps_3dv_extension,int num_of_views, Bitstream *s)
{
  int k=0;
  sps_3dv_extension->acquisition_idx=u_v(2,"3dv_acquisition_info:2dv_acquisition_idx",s);

    if(sps_3dv_extension->acquisition_idx)
    {
      init_acquisition_info(sps_3dv_extension->threeDV_acquisition_in_subsps);
      sps_3dv_extension->threeDV_acquisition_in_subsps->num_views=num_of_views;
      sps_3dv_extension->threeDV_acquisition_in_subsps->pred_direction=2;

      for(k=0;k<num_of_views;++k)
        sps_3dv_extension->camera_order[k]=ue_v("sps_3dv_extension:camera_order",s);

      sps_3dv_extension->disparity_param_prec=ue_v("sps_3dv_extension:disparity parameter precision",s);

      decode_acquisition_info(sps_3dv_extension->threeDV_acquisition_in_subsps,sps_3dv_extension->threeDV_acquisition_in_subsps,
        sps_3dv_extension->threeDV_acquisition_in_subsps, sps_3dv_extension->threeDV_acquisition_in_subsps, s, 0);
      decode_acquisition_info(sps_3dv_extension->threeDV_acquisition_in_subsps,sps_3dv_extension->threeDV_acquisition_in_subsps,
        sps_3dv_extension->threeDV_acquisition_in_subsps, sps_3dv_extension->threeDV_acquisition_in_subsps, s, 1);
    }
  

  if(ThreeDV_EXTEND_HIGH==sps_3dv_extension->subset_mvc_sps->sps.profile_idc)
  {
    int i;

    sps_3dv_extension->reduced_resolution=u_1("sps_3dv_extension: reduced_resolution",s);

    if (sps_3dv_extension->reduced_resolution)
    {
      sps_3dv_extension->depth_pic_width_in_mbs_minus1=ue_v("SPS: depth_pic_width_in_mbs_minus1",s);
      sps_3dv_extension->depth_pic_height_in_map_units_minus1=ue_v("SPS: depth_pic_height_in_map_units_minus1",s);
      sps_3dv_extension->depth_hor_mult_minus1=ue_v("SPS: depth_hor_mult_minus1",s);
      sps_3dv_extension->depth_ver_mult_minus1=ue_v("SPS: depth_ver_mult_minus1",s);
      sps_3dv_extension->depth_hor_rsh=ue_v("SPS: depth_hor_rsh",s);
      sps_3dv_extension->depth_ver_rsh=ue_v("SPS: depth_ver_rsh",s);
    }

    sps_3dv_extension->depth_frame_cropping_flag=u_1("sps_3dv_extension: depth_frame_cropping_flag",s);
    if (sps_3dv_extension->depth_frame_cropping_flag)
    {
      sps_3dv_extension->depth_frame_crop_left_offset=ue_v("SPS: depth_frame_crop_left_offset",s);
      sps_3dv_extension->depth_frame_crop_right_offset=ue_v("SPS: depth_frame_crop_right_offset",s);
      sps_3dv_extension->depth_frame_crop_top_offset=ue_v("SPS: depth_frame_crop_top_offset",s);
      sps_3dv_extension->depth_frame_crop_bottom_offset=ue_v("SPS: depth_frame_crop_bottom_offset",s);
    }else
    {
      sps_3dv_extension->depth_frame_crop_left_offset=0;
      sps_3dv_extension->depth_frame_crop_right_offset=0;
      sps_3dv_extension->depth_frame_crop_top_offset=0;
      sps_3dv_extension->depth_frame_crop_bottom_offset=0;
    }

    memset(sps_3dv_extension->grid_pos_x,0,sizeof(sps_3dv_extension->grid_pos_x));
    memset(sps_3dv_extension->grid_pos_y,0,sizeof(sps_3dv_extension->grid_pos_y));

    sps_3dv_extension->grid_pos_num_views=ue_v("SPS: grid_pos_num_views",s);
    for(i=0;i<sps_3dv_extension->grid_pos_num_views;i++)
    {
      int viewid;

      viewid=ue_v("SPS: grid_pos_view_id",s);
      sps_3dv_extension->grid_pos_x[viewid]=se_v("SPS: grid_pos_x",s);
      sps_3dv_extension->grid_pos_y[viewid]=se_v("SPS: grid_pos_y",s);
    }

    sps_3dv_extension->SliceHeaderPred = u_1("sps_3dv_extension: slice_header_prediction",          s);

    sps_3dv_extension->VSP_Enable=u_1("sps_3dv_extension: seq_view_synthesis_flag ",                  s);
  }

  if (ThreeDV_EXTEND_HIGH==sps_3dv_extension->subset_mvc_sps->sps.profile_idc)
  {
    sps_3dv_extension->AdaptiveLuminanceCompensation = u_1("sps_3dv_extension: enable_alc_encoder_flag",          s);
  }
  if (ThreeDV_EXTEND_HIGH==sps_3dv_extension->subset_mvc_sps->sps.profile_idc)
  {
    sps_3dv_extension->RLESkip = u_1("sps_3dv_extension: enable RLESkip flag",          s);
  }

  sps_3dv_extension->valid=1;
}
#endif

void reset_subset_sps(subset_seq_parameter_set_rbsp_t *subset_sps)
{
  int i, j;

  if(subset_sps && subset_sps->num_views_minus1>=0)
  {
    subset_sps->sps.seq_parameter_set_id = (unsigned int) -1;

    FREEPTR(subset_sps->view_id);
#if EXT3D
    FREEPTR(subset_sps->depth_view_present_flag);
    FREEPTR(subset_sps->texture_view_present_flag);
#endif
    for(i=0; i<=subset_sps->num_views_minus1; i++)
    {
      FREEPTR(subset_sps->anchor_ref_l0[i]);
      FREEPTR(subset_sps->anchor_ref_l1[i]);
    }
    FREEPTR(subset_sps->anchor_ref_l0);
    FREEPTR(subset_sps->anchor_ref_l1);
    FREEPTR(subset_sps->num_anchor_refs_l0);
    FREEPTR(subset_sps->num_anchor_refs_l1);

    for(i=0; i<=subset_sps->num_views_minus1; i++)
    {
      FREEPTR(subset_sps->non_anchor_ref_l0[i]);
      FREEPTR(subset_sps->non_anchor_ref_l1[i]);
    }
    FREEPTR(subset_sps->non_anchor_ref_l0);
    FREEPTR(subset_sps->non_anchor_ref_l1);
    FREEPTR(subset_sps->num_non_anchor_refs_l0);
    FREEPTR(subset_sps->num_non_anchor_refs_l1);

    if(subset_sps->num_level_values_signalled_minus1 >= 0)
    {
      FREEPTR(subset_sps->level_idc);
      for(i=0; i<=subset_sps->num_level_values_signalled_minus1; i++)
      {
        for(j=0; j<=subset_sps->num_applicable_ops_minus1[i]; j++)
        {
          FREEPTR(subset_sps->applicable_op_target_view_id[i][j]);
#if EXT3D
          FREEPTR(subset_sps->applicable_op_depth_flag[i][j]);
          FREEPTR(subset_sps->applicable_op_texture_flag[i][j]);
#endif
        }
        FREEPTR(subset_sps->applicable_op_target_view_id[i]);
#if EXT3D
        FREEPTR(subset_sps->applicable_op_depth_flag[i]);
        FREEPTR(subset_sps->applicable_op_texture_flag[i]);
#endif
        FREEPTR(subset_sps->applicable_op_temporal_id[i]);
        FREEPTR(subset_sps->applicable_op_num_target_views_minus1[i]);
#if EXT3D
        if (subset_sps->sps.profile_idc==ThreeDV_HIGH || subset_sps->sps.profile_idc==ThreeDV_EXTEND_HIGH)
        {
          FREEPTR(subset_sps->applicable_op_num_texture_views_minus1[i]);
          FREEPTR(subset_sps->applicable_op_num_depth_views[i]);
        }
        else
#endif
        FREEPTR(subset_sps->applicable_op_num_views_minus1[i]);
      }
      FREEPTR(subset_sps->applicable_op_target_view_id);
#if EXT3D
      FREEPTR(subset_sps->applicable_op_depth_flag);
      FREEPTR(subset_sps->applicable_op_texture_flag);
#endif
      FREEPTR(subset_sps->applicable_op_temporal_id);
      FREEPTR(subset_sps->applicable_op_num_target_views_minus1);
#if EXT3D
      if (subset_sps->sps.profile_idc==ThreeDV_HIGH || subset_sps->sps.profile_idc==ThreeDV_EXTEND_HIGH)
      {
        FREEPTR(subset_sps->applicable_op_num_texture_views_minus1);
        FREEPTR(subset_sps->applicable_op_num_depth_views);
      }
      else
#endif
      FREEPTR(subset_sps->applicable_op_num_views_minus1);      
      FREEPTR(subset_sps->num_applicable_ops_minus1);

      subset_sps->num_level_values_signalled_minus1 = -1;
    }    

    //end;
    subset_sps->num_views_minus1 = -1;
  }

  if(subset_sps && subset_sps->mvc_vui_parameters_present_flag)
  {
    MVCVUI_t *pMVCVUI = &(subset_sps->MVCVUIParams);
    if(pMVCVUI->num_ops_minus1 >=0)
    {
      FREEPTR(pMVCVUI->temporal_id);
      FREEPTR(pMVCVUI->num_target_output_views_minus1);
      for(i=0; i<=pMVCVUI->num_ops_minus1; i++)
#if EXT3D
      {
          FREEPTR(pMVCVUI->view_id[i]);
          if (pMVCVUI->is_mvcd_vui)
          {
            FREEPTR(pMVCVUI->depth_flag[i]);
            FREEPTR(pMVCVUI->texture_flag[i]);
          }
      }
#else
        FREEPTR(pMVCVUI->view_id[i]);
#endif
      FREEPTR(pMVCVUI->view_id);
#if EXT3D
      if (pMVCVUI->is_mvcd_vui)
      {
        FREEPTR(pMVCVUI->depth_flag);
        FREEPTR(pMVCVUI->texture_flag);
      }
#endif
      FREEPTR(pMVCVUI->timing_info_present_flag);
      FREEPTR(pMVCVUI->num_units_in_tick);
      FREEPTR(pMVCVUI->time_scale);
      FREEPTR(pMVCVUI->fixed_frame_rate_flag);
      FREEPTR(pMVCVUI->nal_hrd_parameters_present_flag);
      FREEPTR(pMVCVUI->vcl_hrd_parameters_present_flag);
      FREEPTR(pMVCVUI->low_delay_hrd_flag);
      FREEPTR(pMVCVUI->pic_struct_present_flag);

      pMVCVUI->num_ops_minus1 = -1;
    }    
    subset_sps->mvc_vui_parameters_present_flag = 0;
  }
}

int GetBaseViewId(VideoParameters *p_Vid, subset_seq_parameter_set_rbsp_t **subset_sps)
{
  subset_seq_parameter_set_rbsp_t *curr_subset_sps;
  int i, iBaseViewId=0; //-1;

  *subset_sps = NULL;
  curr_subset_sps = p_Vid->SubsetSeqParSet;
  for(i=0; i<MAXSPS; i++)
  {
    if(curr_subset_sps->num_views_minus1>=0 && curr_subset_sps->sps.Valid) // && curr_subset_sps->sps.seq_parameter_set_id < MAXSPS)
    {
      iBaseViewId = curr_subset_sps->view_id[BASE_VIEW_IDX];
      break;
    }
    curr_subset_sps++;
  }

  if(i<MAXSPS)
    *subset_sps = curr_subset_sps;
  return iBaseViewId;
}

void get_max_dec_frame_buf_size(seq_parameter_set_rbsp_t *sps)
{
  int pic_size = (sps->pic_width_in_mbs_minus1 + 1) * (sps->pic_height_in_map_units_minus1 + 1) * (sps->frame_mbs_only_flag?1:2) * 384;

  int size = 0;

  switch (sps->level_idc)
  {
  case 9:
    size = 152064;
    break;
  case 10:
    size = 152064;
    break;
  case 11:
    if (!IS_FREXT_PROFILE(sps->profile_idc) && (sps->constrained_set3_flag == 1))
      size = 152064;
    else
      size = 345600;
    break;
  case 12:
    size = 912384;
    break;
  case 13:
    size = 912384;
    break;
  case 20:
    size = 912384;
    break;
  case 21:
    size = 1824768;
    break;
  case 22:
    size = 3110400;
    break;
  case 30:
    size = 3110400;
    break;
  case 31:
    size = 6912000;
    break;
  case 32:
    size = 7864320;
    break;
  case 40:
    size = 12582912;
    break;
  case 41:
    size = 12582912;
    break;
  case 42:
    size = 13369344;
    break;
  case 50:
    size = 42393600;
    break;
  case 51:
    size = 70778880;
    break;
  default:
    error ("undefined level", 500);
    break;
  }

  size /= pic_size;
  size = imin( size, 16);
  sps->max_dec_frame_buffering = size;
}
#endif
