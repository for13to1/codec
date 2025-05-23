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
 *************************************************************************************
 * \file header.c
 *
 * \brief
 *    H.264 Slice headers
 *
 *************************************************************************************
 */

#include "global.h"
#include "elements.h"
#include "defines.h"
#include "fmo.h"
#include "vlc.h"
#include "mbuffer.h"
#include "header.h"

#include "ctx_tables.h"
#if EXT3D
#include "image.h"
#endif

#if TRACE
#define SYMTRACESTRING(s) strncpy(sym.tracestring,s,TRACESTRING_SIZE)
#else
#define SYMTRACESTRING(s) // do nothing
#endif

static void ref_pic_list_reordering(Slice *currSlice);
static void pred_weight_table(Slice *currSlice);
#if MVC_EXTENSION_ENABLE||EXT3D
static void ref_pic_list_mvc_modification(Slice *currSlice);
#endif

#if EXT3D
int ViewCompOrder(VideoParameters *p_Vid, int depthFlag, int VOIdx)
{
  int k,num_of_views=p_Vid->num_of_views;

  for(k=0;k<num_of_views*2;++k)

  {
    if(p_Vid->ViewCompOrderDepthFlag[k]==depthFlag && p_Vid->ViewCompOrderVOIdx[k]==VOIdx)
      return k;
  }
  return -1;
}
#endif


/*!
 ************************************************************************
 * \brief
 *    calculate Ceil(Log2(uiVal))
 ************************************************************************
 */
unsigned CeilLog2( unsigned uiVal)
{
  unsigned uiTmp = uiVal-1;
  unsigned uiRet = 0;

  while( uiTmp != 0 )
  {
    uiTmp >>= 1;
    uiRet++;
  }
  return uiRet;
}

unsigned CeilLog2_sf( unsigned uiVal)
{
  unsigned uiTmp = uiVal-1;
  unsigned uiRet = 0;

  while( uiTmp > 0 )
  {
    uiTmp >>= 1;
    uiRet++;
  }
  return uiRet;
}

/*!
 ************************************************************************
 * \brief
 *    read the first part of the header (only the pic_parameter_set_id)
 * \return
 *    Length of the first part of the slice header (in bits)
 ************************************************************************
 */
int FirstPartOfSliceHeader(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  byte dP_nr = assignSE2partition[currSlice->dp_mode][SE_HEADER];
  DataPartition *partition = &(currSlice->partArr[dP_nr]);
  Bitstream *currStream = partition->bitstream;
  int tmp;

  p_Dec->UsedBits= partition->bitstream->frame_bitoffset; // was hardcoded to 31 for previous start-code. This is better.

  // Get first_mb_in_slice
  currSlice->start_mb_nr = ue_v ("SH: first_mb_in_slice", currStream);

  tmp = ue_v ("SH: slice_type", currStream);

  if (tmp > 4) tmp -= 5;

  p_Vid->type = currSlice->slice_type = (SliceType) tmp;

  currSlice->pic_parameter_set_id = ue_v ("SH: pic_parameter_set_id", currStream);

#if EXT3D
  if( p_Vid->SliceHeaderPred && currSlice->slice_header_pred_flag )
  {
    currSlice->pre_slice_header_src = u_v(2, "SH: pre_slice_header_src", currStream);
    assert(currSlice->pre_slice_header_src!=0);
    CopySliceHeader(currSlice, currSlice->pre_slice_header_src);
  }
  else
    currSlice->pre_slice_header_src = 0;
  if(!currSlice->pre_slice_header_src)
  {
#endif
  if( p_Vid->separate_colour_plane_flag )
    currSlice->colour_plane_id = u_v (2, "SH: colour_plane_id", currStream);
  else
    currSlice->colour_plane_id = PLANE_Y;
#if EXT3D
  }
#endif
  return p_Dec->UsedBits;
}

/*!
 ************************************************************************
 * \brief
 *    read the scond part of the header (without the pic_parameter_set_id
 * \return
 *    Length of the second part of the Slice header in bits
 ************************************************************************
 */
int RestOfSliceHeader(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

  byte dP_nr = assignSE2partition[currSlice->dp_mode][SE_HEADER];
  DataPartition *partition = &(currSlice->partArr[dP_nr]);
  Bitstream *currStream = partition->bitstream;
  int val, len;

#if EXT3D
  int voidx=GetVOIdx(p_Vid, (currSlice->svc_extension_flag == 1) ? currSlice->NaluHeader3dVCExt.view_id : currSlice->NaluHeaderMVCExt.view_id);
  currSlice->depth_based_mvp_flag=0;
  currSlice->Harmonize_VSP_IVP = 0;
  currSlice->depth_range_wp=0;

  currSlice->dep_parameter_set_id=0;

  if(p_Vid->SliceHeaderPred && currSlice->slice_header_pred_flag)
  {
    if(p_Vid->type==P_SLICE || p_Vid->type == SP_SLICE || p_Vid->type==B_SLICE)
    {
      currSlice->pre_ref_lists_src = u_v (2, "SH: pre_ref_lists_src", currStream);
      currSlice->num_ref_idx_active[LIST_0] = p_Vid->active_pps->num_ref_idx_l0_active_minus1 + 1;
      currSlice->num_ref_idx_active[LIST_1] = p_Vid->active_pps->num_ref_idx_l1_active_minus1 + 1;
      if(!currSlice->pre_ref_lists_src)
      {    
        if(p_Vid->type==P_SLICE || p_Vid->type == SP_SLICE || p_Vid->type==B_SLICE)
        {
          val = u_1 ("SH: num_ref_idx_override_flag", currStream);
          if (val)
          {
            currSlice->num_ref_idx_active[LIST_0] = 1 + ue_v ("SH: num_ref_idx_l0_active_minus1", currStream);
            if(p_Vid->type==B_SLICE)
              currSlice->num_ref_idx_active[LIST_1] = 1 + ue_v ("SH: num_ref_idx_l1_active_minus1", currStream);
          }
        }
      if (currSlice->svc_extension_flag == 0 || currSlice->svc_extension_flag == 1)
        ref_pic_list_mvc_modification(currSlice);
      else
        ref_pic_list_reordering(currSlice);
      }
      else
        CopySliceRefList(currSlice, currSlice->pre_ref_lists_src);
    }

    if (currSlice->slice_type!=B_SLICE)
        currSlice->num_ref_idx_active[LIST_1] = 0;
    currSlice->weighted_pred_flag = (unsigned short) ((currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE) 
      ? p_Vid->active_pps->weighted_pred_flag 
      : (currSlice->slice_type == B_SLICE && p_Vid->active_pps->weighted_bipred_idc == 1));
    currSlice->weighted_bipred_idc = (unsigned short) (currSlice->slice_type == B_SLICE && p_Vid->active_pps->weighted_bipred_idc > 0);
    if ((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE || p_Vid->type == SP_SLICE)) ||
      (p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE)))
    {
      currSlice->pre_pred_weight_table_src = u_v (2, "SH: pre_pred_weight_table_src", currStream);
      if(!currSlice->pre_pred_weight_table_src)
        pred_weight_table(currSlice);
      else
        CopySlicePredWeightTable(currSlice, currSlice->pre_pred_weight_table_src);
    }
    if (currSlice->nal_reference_idc)
    {
      currSlice->pre_dec_ref_pic_marking_src = u_v (2, "SH: pre_dec_ref_pic_marking_src",  currStream);
      if(!currSlice->pre_dec_ref_pic_marking_src)
        dec_ref_pic_marking(p_Vid, currStream, currSlice);
      else
        CopySliceDecRefPicMarking(currSlice, currSlice->pre_dec_ref_pic_marking_src);
    }
    currSlice->slice_qp_delta = val = se_v("SH: slice_qp_delta", currStream);
    currSlice->qp = 26 + p_Vid->active_pps->pic_init_qp_minus26 + val;
    if ((currSlice->qp < -p_Vid->bitdepth_luma_qp_scale) || (currSlice->qp > 51))
      error("slice_qp_delta makes slice_qp_y out of range", 500);  
  }
  else
  {
#endif 

  currSlice->frame_num = u_v (active_sps->log2_max_frame_num_minus4 + 4, "SH: frame_num", currStream);

  /* Tian Dong: frame_num gap processing, if found */
  if(currSlice->idr_flag) //if (p_Vid->idr_flag)
  {
    p_Vid->pre_frame_num = currSlice->frame_num;
    // picture error concealment
    p_Vid->last_ref_pic_poc = 0;
    assert(currSlice->frame_num == 0);
  }
  if (active_sps->frame_mbs_only_flag)
  {
    p_Vid->structure = FRAME;
    currSlice->field_pic_flag=0;
  }
  else
  {
    // field_pic_flag   u(1)
    currSlice->field_pic_flag = u_1("SH: field_pic_flag", currStream);
    if (currSlice->field_pic_flag)
    {
      // bottom_field_flag  u(1)
      currSlice->bottom_field_flag = (byte) u_1("SH: bottom_field_flag", currStream);
      p_Vid->structure = currSlice->bottom_field_flag ? BOTTOM_FIELD : TOP_FIELD;
    }
    else
    {
      p_Vid->structure = FRAME;
      currSlice->bottom_field_flag = FALSE;
    }
  }
  currSlice->structure = (PictureStructure) p_Vid->structure;
  currSlice->mb_aff_frame_flag = (active_sps->mb_adaptive_frame_field_flag && (currSlice->field_pic_flag==0));
  //currSlice->mb_aff_frame_flag = p_Vid->mb_aff_frame_flag;
  if (p_Vid->structure == FRAME       ) 
    assert (currSlice->field_pic_flag == 0);
  if (p_Vid->structure == TOP_FIELD   ) 
    assert (currSlice->field_pic_flag == 1 && (currSlice->bottom_field_flag == FALSE));
  if (p_Vid->structure == BOTTOM_FIELD) 
    assert (currSlice->field_pic_flag == 1 && (currSlice->bottom_field_flag == TRUE ));

  if (currSlice->idr_flag)
  {
    currSlice->idr_pic_id = ue_v("SH: idr_pic_id", currStream);
  }
  if (active_sps->pic_order_cnt_type == 0)
  {
    currSlice->pic_order_cnt_lsb = u_v(active_sps->log2_max_pic_order_cnt_lsb_minus4 + 4, "SH: pic_order_cnt_lsb", currStream);
    if( p_Vid->active_pps->bottom_field_pic_order_in_frame_present_flag  ==  1 &&  !currSlice->field_pic_flag )
      currSlice->delta_pic_order_cnt_bottom = se_v("SH: delta_pic_order_cnt_bottom", currStream);
    else
      currSlice->delta_pic_order_cnt_bottom = 0;
  }
  
  if( active_sps->pic_order_cnt_type == 1)
  {
    if ( !active_sps->delta_pic_order_always_zero_flag )
    {
      currSlice->delta_pic_order_cnt[ 0 ] = se_v("SH: delta_pic_order_cnt[0]", currStream);
      if( p_Vid->active_pps->bottom_field_pic_order_in_frame_present_flag  ==  1  &&  !currSlice->field_pic_flag )
        currSlice->delta_pic_order_cnt[ 1 ] = se_v("SH: delta_pic_order_cnt[1]", currStream);
      else
        currSlice->delta_pic_order_cnt[ 1 ] = 0;  // set to zero if not in stream
    }
    else
    {
      currSlice->delta_pic_order_cnt[ 0 ] = 0;
      currSlice->delta_pic_order_cnt[ 1 ] = 0;
    }
  }
  //! redundant_pic_cnt is missing here
  if (p_Vid->active_pps->redundant_pic_cnt_present_flag)
  {
    currSlice->redundant_pic_cnt = ue_v ("SH: redundant_pic_cnt", currStream);
  }
  if(currSlice->slice_type == B_SLICE)
  {
    currSlice->direct_spatial_mv_pred_flag = u_1 ("SH: direct_spatial_mv_pred_flag", currStream);
  }

  currSlice->num_ref_idx_active[LIST_0] = p_Vid->active_pps->num_ref_idx_l0_active_minus1 + 1;
  currSlice->num_ref_idx_active[LIST_1] = p_Vid->active_pps->num_ref_idx_l1_active_minus1 + 1;

  if(p_Vid->type==P_SLICE || p_Vid->type == SP_SLICE || p_Vid->type==B_SLICE)
  {
    val = u_1 ("SH: num_ref_idx_override_flag", currStream);
    if (val)
    {
      currSlice->num_ref_idx_active[LIST_0] = 1 + ue_v ("SH: num_ref_idx_l0_active_minus1", currStream);

      if(p_Vid->type==B_SLICE)
      {
        currSlice->num_ref_idx_active[LIST_1] = 1 + ue_v ("SH: num_ref_idx_l1_active_minus1", currStream);
      }
    }
  }
  if (currSlice->slice_type!=B_SLICE)
  {
    currSlice->num_ref_idx_active[LIST_1] = 0;
  }

#if MVC_EXTENSION_ENABLE||EXT3D
  if (currSlice->svc_extension_flag == 0 || currSlice->svc_extension_flag == 1)
    ref_pic_list_mvc_modification(currSlice);
  else
    ref_pic_list_reordering(currSlice);
#else
  ref_pic_list_reordering(currSlice);
#endif
  currSlice->weighted_pred_flag = (unsigned short) ((currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE) 
    ? p_Vid->active_pps->weighted_pred_flag 
    : (currSlice->slice_type == B_SLICE && p_Vid->active_pps->weighted_bipred_idc == 1));
  currSlice->weighted_bipred_idc = (unsigned short) (currSlice->slice_type == B_SLICE && p_Vid->active_pps->weighted_bipred_idc > 0);
  if ((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
      (p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE)))
  {
    pred_weight_table(currSlice);
  }
  if (currSlice->nal_reference_idc)
    dec_ref_pic_marking(p_Vid, currStream, currSlice);

  if (p_Vid->active_pps->entropy_coding_mode_flag && p_Vid->type!=I_SLICE && p_Vid->type!=SI_SLICE)
  {
    currSlice->model_number = ue_v("SH: cabac_init_idc", currStream);
  }
  else
  {
    currSlice->model_number = 0;
  }
  currSlice->slice_qp_delta = val = se_v("SH: slice_qp_delta", currStream);
  //currSlice->qp = p_Vid->qp = 26 + p_Vid->active_pps->pic_init_qp_minus26 + val;
  currSlice->qp = 26 + p_Vid->active_pps->pic_init_qp_minus26 + val;
  if ((currSlice->qp < -p_Vid->bitdepth_luma_qp_scale) || (currSlice->qp > 51))
    error ("slice_qp_delta makes slice_qp_y out of range", 500);

  if(p_Vid->type==SP_SLICE || p_Vid->type == SI_SLICE)
  {
    if(p_Vid->type==SP_SLICE)
    {
      currSlice->sp_switch = u_1 ("SH: sp_for_switch_flag", currStream);
    }
    currSlice->slice_qs_delta = val = se_v("SH: slice_qs_delta", currStream);
    currSlice->qs = 26 + p_Vid->active_pps->pic_init_qs_minus26 + val;    
    if ((currSlice->qs < 0) || (currSlice->qs > 51))
      error ("slice_qs_delta makes slice_qs_y out of range", 500);
  }
  if ( !HI_INTRA_ONLY_PROFILE || (HI_INTRA_ONLY_PROFILE && (p_Inp->intra_profile_deblocking == 1) ))
  //then read flags and parameters from bistream
  {
    if (p_Vid->active_pps->deblocking_filter_control_present_flag)
    {
      currSlice->DFDisableIdc = (short) ue_v ("SH: disable_deblocking_filter_idc", currStream);

      if (currSlice->DFDisableIdc!=1)
      {
        currSlice->DFAlphaC0Offset = (short) (2 * se_v("SH: slice_alpha_c0_offset_div2", currStream));
        currSlice->DFBetaOffset    = (short) (2 * se_v("SH: slice_beta_offset_div2", currStream));
      }
      else
      {
        currSlice->DFAlphaC0Offset = currSlice->DFBetaOffset = 0;
      }
    }
    else
    {
      currSlice->DFDisableIdc = currSlice->DFAlphaC0Offset = currSlice->DFBetaOffset = 0;
    }
  }
  else //By default the Loop Filter is Off
  { //444_TEMP_NOTE: change made below. 08/07/07
    //still need to parse the SEs (read flags and parameters from bistream) but will ignore
    if (p_Vid->active_pps->deblocking_filter_control_present_flag)
    {
      currSlice->DFDisableIdc = (short) ue_v ("SH: disable_deblocking_filter_idc", currStream);
      if (currSlice->DFDisableIdc!=1)
      {
        currSlice->DFAlphaC0Offset = (short) (2 * se_v("SH: slice_alpha_c0_offset_div2", currStream));
        currSlice->DFBetaOffset    = (short) (2 * se_v("SH: slice_beta_offset_div2", currStream));
      }
    }//444_TEMP_NOTE. the end of change. 08/07/07
    //Ignore the SEs, by default the Loop Filter is Off
    currSlice->DFDisableIdc =1;
    currSlice->DFAlphaC0Offset = currSlice->DFBetaOffset = 0;
  }
  if (p_Vid->active_pps->num_slice_groups_minus1>0 && p_Vid->active_pps->slice_group_map_type>=3 &&
      p_Vid->active_pps->slice_group_map_type<=5)
  {
    len = (active_sps->pic_height_in_map_units_minus1+1)*(active_sps->pic_width_in_mbs_minus1+1)/
          (p_Vid->active_pps->slice_group_change_rate_minus1+1);
    if (((active_sps->pic_height_in_map_units_minus1+1)*(active_sps->pic_width_in_mbs_minus1+1))%
          (p_Vid->active_pps->slice_group_change_rate_minus1+1))
          len +=1;

    len = CeilLog2(len+1);

    currSlice->slice_group_change_cycle = u_v (len, "SH: slice_group_change_cycle", currStream);
  }
#if EXT3D
  }

  if((currSlice->nal_unit_type==NALU_TYPE_3DV_EXT)&&(currSlice->slice_type!=SI_SLICE)&&(currSlice->slice_type!=I_SLICE))
  {
    if(currSlice->is_depth)
    {
      if(ThreeDV_EXTEND_HIGH==currSlice->p_Vid->active_sps->profile_idc)
        if(!(currSlice->weighted_pred_flag||currSlice->weighted_bipred_idc))
          currSlice->depth_range_wp=u_1("SH:depth_range_based_wp",currStream);
    }
    else
    {
      if (ThreeDV_EXTEND_HIGH==currSlice->p_Vid->active_sps->profile_idc)
      {
        currSlice->depth_based_mvp_flag=u_1("SH: depth_based_mvp_flag",currStream);
        if(p_Vid->VSP_Enable == 1)
        {
          currSlice->Harmonize_VSP_IVP = u_1("SH: Harmonize_VSP_IVP_flag",currStream);
        }
      }
    }
    if(p_Vid->acquisition_idx!=1 && (currSlice->depth_range_wp || currSlice->Harmonize_VSP_IVP || currSlice->depth_based_mvp_flag))
      currSlice->dep_parameter_set_id = ue_v("SH: dep_parameter_set_id" , currStream);
  }
#endif
  p_Vid->PicHeightInMbs = p_Vid->FrameHeightInMbs / ( 1 + currSlice->field_pic_flag );
  p_Vid->PicSizeInMbs   = p_Vid->PicWidthInMbs * p_Vid->PicHeightInMbs;
  p_Vid->FrameSizeInMbs = p_Vid->PicWidthInMbs * p_Vid->FrameHeightInMbs;

#if EXT3D
  if((!currSlice->is_depth)&&(!voidx))
  {
    p_Vid->base_view_frm_num=currSlice->frame_num;
  }

  if (1 == p_Vid->AdaptiveLuminanceCompensation &&  0 == currSlice->is_depth && voidx > 0 && P_SLICE == currSlice->slice_type)
    currSlice->alc_slice_flag=1;
  else
    currSlice->alc_slice_flag=0;

  //SAIT_INDR_C0094
  if(p_Vid->profile_idc==ThreeDV_EXTEND_HIGH)
  {
    if(currSlice->is_depth)
    {
      p_Vid->andr_flag = u_1("SH: adaptive_NDR_flag", currStream);
    }
  }
#endif

  return p_Dec->UsedBits;
}

#if EXT3D
void CopySliceHeader(Slice *pSlice, int src)
{
  int voidx               = GetVOIdx(pSlice->p_Vid, (pSlice->svc_extension_flag == 1) ? pSlice->NaluHeader3dVCExt.view_id : pSlice->NaluHeaderMVCExt.view_id);
  VideoParameters *p_Vid  = pSlice->p_Vid;
  int bDepth = 0;
  switch(src)
  {
  case 0:
    return;
    break;
  case 1:
    assert(voidx!=0);
    voidx = voidx - 1;
    bDepth = pSlice->is_depth;
    break;
  case 2:
    bDepth = 1 - pSlice->is_depth;
    break;
  case 3:
    bDepth = pSlice->is_depth;
    voidx = 0;
    break;
  }  
//  pSlice->start_mb_nr     = p_Vid->slice_header_dual[voidx][bDepth].start_mb_nr;
#if FIX_SLICE_HEAD_PRED
  if(pSlice->slice_type != p_Vid->slice_header_dual[voidx][bDepth].slice_type && ((pSlice->slice_type ==B_SLICE || p_Vid->slice_header_dual[voidx][bDepth].slice_type == B_SLICE)))
  {
    printf("slice header prediction is not allowed");
    exit(-1);
  }  
#endif
  pSlice->colour_plane_id         = p_Vid->slice_header_dual[voidx][bDepth].colour_plane_id;
  pSlice->frame_num                    = p_Vid->slice_header_dual[voidx][bDepth].frame_num             ;
  pSlice->field_pic_flag         = p_Vid->slice_header_dual[voidx][bDepth].field_pic_flag;
  pSlice->bottom_field_flag       = p_Vid->slice_header_dual[voidx][bDepth].bottom_field_flag;
  pSlice->idr_pic_id           = p_Vid->slice_header_dual[voidx][bDepth].idr_pic_id;

  pSlice->pic_order_cnt_lsb       = p_Vid->slice_header_dual[voidx][bDepth].pic_order_cnt_lsb;
  pSlice->delta_pic_order_cnt_bottom   = p_Vid->slice_header_dual[voidx][bDepth].delta_pic_order_cnt_bottom;
  pSlice->delta_pic_order_cnt[0]     = p_Vid->slice_header_dual[voidx][bDepth].delta_pic_order_cnt[0]   ;
  pSlice->delta_pic_order_cnt[1]     = p_Vid->slice_header_dual[voidx][bDepth].delta_pic_order_cnt[1]     ;

  pSlice->redundant_pic_cnt       = p_Vid->slice_header_dual[voidx][bDepth].redundant_pic_cnt;
  pSlice->redundant_slice_ref_idx     = p_Vid->slice_header_dual[voidx][bDepth].redundant_slice_ref_idx;

  pSlice->slice_group_change_cycle     = p_Vid->slice_header_dual[voidx][bDepth].slice_group_change_cycle ;
  pSlice->DFDisableIdc           = p_Vid->slice_header_dual[voidx][bDepth].DFDisableIdc         ;
  pSlice->DFAlphaC0Offset              = p_Vid->slice_header_dual[voidx][bDepth].DFAlphaC0Offset      ;
  pSlice->DFBetaOffset             = p_Vid->slice_header_dual[voidx][bDepth].DFBetaOffset;
  pSlice->structure           = p_Vid->slice_header_dual[voidx][bDepth].structure;
  pSlice->mb_aff_frame_flag       = p_Vid->slice_header_dual[voidx][bDepth].mb_aff_frame_flag;

  pSlice->direct_spatial_mv_pred_flag  = p_Vid->slice_header_dual[voidx][bDepth].direct_spatial_mv_pred_flag;
  pSlice->model_number                 = p_Vid->slice_header_dual[voidx][1-pSlice->is_depth].model_number           ;
  pSlice->sp_switch                    = p_Vid->slice_header_dual[voidx][1-pSlice->is_depth].sp_switch            ;
  pSlice->slice_qs_delta               = p_Vid->slice_header_dual[voidx][1-pSlice->is_depth].slice_qs_delta        ;
  pSlice->qs                           = p_Vid->slice_header_dual[voidx][1-pSlice->is_depth].qs              ;
}

void CopySliceRefList (Slice *pSlice, int src)
{
  int bDepth = 0, i = 0, j = 0;
  int voidx               = GetVOIdx(pSlice->p_Vid, (pSlice->svc_extension_flag == 1) ? pSlice->NaluHeader3dVCExt.view_id : pSlice->NaluHeaderMVCExt.view_id);
  VideoParameters *p_Vid                  = pSlice->p_Vid;
  switch(src)
  {
  case 0:
    return;
    break;
  case 1:
    assert(voidx!=0);
    voidx = voidx - 1;
    bDepth = pSlice->is_depth;
    break;
  case 2:
    bDepth = 1 - pSlice->is_depth;
    break;
  case 3:
    bDepth = pSlice->is_depth;
    voidx = 0;
    break;
  }
  for(j = LIST_0; j <=LIST_1; j++)
  {
    pSlice->ref_pic_list_reordering_flag[j] = p_Vid->slice_header_dual[voidx][bDepth].ref_pic_list_reordering_flag [j];
    pSlice->num_ref_idx_active[j]           = p_Vid->slice_header_dual[voidx][bDepth].num_ref_idx_active[j];
    if(p_Vid->slice_header_dual[voidx][bDepth].reordering_of_pic_nums_idc[j])
       {
      for (i=0; i<(pSlice->num_ref_idx_active[j] + 1); i++)
      {
        pSlice->reordering_of_pic_nums_idc[j][i] = p_Vid->slice_header_dual[voidx][bDepth].reordering_of_pic_nums_idc[j][i];
        pSlice->abs_diff_pic_num_minus1[j][i]    = p_Vid->slice_header_dual[voidx][bDepth].abs_diff_pic_num_minus1[j][i]   ;
        pSlice->long_term_pic_idx[j][i]          = p_Vid->slice_header_dual[voidx][bDepth].long_term_pic_idx[j][i];
      }
    }
    for(i=0; i<(MAX_VIEWREFERENCE+1); i++)
    {
      pSlice->abs_diff_view_idx_minus1[j][i]        = p_Vid->slice_header_dual[voidx][bDepth].abs_diff_view_idx_minus1[j][i];
      //pSlice->reordering_of_interview_num_idc[j][i] = p_Vid->slice_header_dual[voidx][bDepth].reordering_of_interview_num_idc[j][i];
    }
   }
}

void CopySlicePredWeightTable (Slice *pSlice, int src)
{
  int bDepth = 0, i = 0, comp = 0;
  int voidx               = GetVOIdx(pSlice->p_Vid, (pSlice->svc_extension_flag == 1) ? pSlice->NaluHeader3dVCExt.view_id : pSlice->NaluHeaderMVCExt.view_id);
  VideoParameters *p_Vid                  = pSlice->p_Vid;
  switch(src)
  {
  case 0:
    return;
    break;
  case 1:
    assert(voidx!=0);
    voidx = voidx - 1;
    bDepth = pSlice->is_depth;
    break;
  case 2:
    bDepth = 1 - pSlice->is_depth;
    break;
  case 3:
    bDepth = pSlice->is_depth;
    voidx = 0;
    break;
  }
  pSlice->luma_log2_weight_denom = p_Vid->slice_header_dual[voidx][bDepth].luma_log2_weight_denom;
  pSlice->wp_round_luma          = p_Vid->slice_header_dual[voidx][bDepth].wp_round_luma;

  if ( 0 != pSlice->active_sps->chroma_format_idc)
  {
    pSlice->chroma_log2_weight_denom = p_Vid->slice_header_dual[voidx][bDepth].chroma_log2_weight_denom;
    pSlice->wp_round_chroma          = p_Vid->slice_header_dual[voidx][bDepth].wp_round_chroma;
  }

  for (i=0; i<MAX_REFERENCE_PICTURES; i++)
  {
    for (comp=0; comp<3; comp++)
    {
      pSlice->wp_weight[0][i][comp] = p_Vid->slice_header_dual[voidx][bDepth].wp_weight[0][i][comp];
      pSlice->wp_weight[1][i][comp] = p_Vid->slice_header_dual[voidx][bDepth].wp_weight[1][i][comp];
      pSlice->wp_offset[0][i][comp] = p_Vid->slice_header_dual[voidx][bDepth].wp_offset[0][i][comp];
      pSlice->wp_offset[1][i][comp] = p_Vid->slice_header_dual[voidx][bDepth].wp_offset[1][i][comp];
    }
  }
}

void CopySliceDecRefPicMarking(Slice *pSlice, int src)
{
  int voidx                               = GetVOIdx(pSlice->p_Vid, (pSlice->svc_extension_flag == 1) ? pSlice->NaluHeader3dVCExt.view_id : pSlice->NaluHeaderMVCExt.view_id);
  VideoParameters *p_Vid                  = pSlice->p_Vid;
  DecRefPicMarking_t *tmp_drpm, *tmp_drpm2, *tmp;
  int bDepth = 0;
  switch(src)
  {
  case 0:
    return;
    break;
  case 1:
    assert(voidx!=0);
    voidx = voidx - 1;
    bDepth = pSlice->is_depth;
    break;
  case 2:
    bDepth = 1 - pSlice->is_depth;
    break;
  case 3:
    bDepth = pSlice->is_depth;
    voidx = 0;
    break;
  }
  pSlice->no_output_of_prior_pics_flag    = p_Vid->slice_header_dual[voidx][bDepth].no_output_of_prior_pics_flag;
  pSlice->long_term_reference_flag        = p_Vid->slice_header_dual[voidx][bDepth].long_term_reference_flag    ;        
  pSlice->adaptive_ref_pic_buffering_flag = p_Vid->slice_header_dual[voidx][bDepth].adaptive_ref_pic_buffering_flag;
  
  while (pSlice->dec_ref_pic_marking_buffer)
  {
    tmp_drpm = pSlice->dec_ref_pic_marking_buffer;
    pSlice->dec_ref_pic_marking_buffer = tmp_drpm->Next;
    free (tmp_drpm);
  }
  for (tmp = p_Vid->slice_header_dual[voidx][bDepth].dec_ref_pic_marking_buffer; tmp!=NULL; tmp=tmp->Next)
  {
    tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t));
    memcpy(tmp_drpm, tmp, sizeof (DecRefPicMarking_t));
    tmp_drpm->Next=NULL;

    if (pSlice->dec_ref_pic_marking_buffer==NULL)
    {
      pSlice->dec_ref_pic_marking_buffer=tmp_drpm;
    }
    else
    {
      tmp_drpm2=pSlice->dec_ref_pic_marking_buffer;
      while (tmp_drpm2->Next!=NULL) tmp_drpm2=tmp_drpm2->Next;
      tmp_drpm2->Next=tmp_drpm;
    }
  }
}
#endif

/*!
 ************************************************************************
 * \brief
 *    read the reference picture reordering information
 ************************************************************************
 */
static void ref_pic_list_reordering(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  byte dP_nr = assignSE2partition[currSlice->dp_mode][SE_HEADER];
  DataPartition *partition = &(currSlice->partArr[dP_nr]);
  Bitstream *currStream = partition->bitstream;
  int i, val;

  alloc_ref_pic_list_reordering_buffer(currSlice);

  if (p_Vid->type!=I_SLICE && p_Vid->type!=SI_SLICE)
  {
    val = currSlice->ref_pic_list_reordering_flag[LIST_0] = u_1 ("SH: ref_pic_list_reordering_flag_l0", currStream);

    if (val)
    {
      i=0;
      do
      {
        val = currSlice->reordering_of_pic_nums_idc[LIST_0][i] = ue_v("SH: reordering_of_pic_nums_idc_l0", currStream);
        if (val==0 || val==1)
        {
          currSlice->abs_diff_pic_num_minus1[LIST_0][i] = ue_v("SH: abs_diff_pic_num_minus1_l0", currStream);
        }
        else
        {
          if (val==2)
          {
            currSlice->long_term_pic_idx[LIST_0][i] = ue_v("SH: long_term_pic_idx_l0", currStream);
          }
        }
        i++;
        // assert (i>currSlice->num_ref_idx_active[LIST_0]);
      } while (val != 3);
    }
  }

  if (p_Vid->type==B_SLICE)
  {
    val = currSlice->ref_pic_list_reordering_flag[LIST_1] = u_1 ("SH: ref_pic_list_reordering_flag_l1", currStream);

    if (val)
    {
      i=0;
      do
      {
        val = currSlice->reordering_of_pic_nums_idc[LIST_1][i] = ue_v("SH: reordering_of_pic_nums_idc_l1", currStream);
        if (val==0 || val==1)
        {
          currSlice->abs_diff_pic_num_minus1[LIST_1][i] = ue_v("SH: abs_diff_pic_num_minus1_l1", currStream);
        }
        else
        {
          if (val==2)
          {
            currSlice->long_term_pic_idx[LIST_1][i] = ue_v("SH: long_term_pic_idx_l1", currStream);
          }
        }
        i++;
        // assert (i>currSlice->num_ref_idx_active[LIST_1]);
      } while (val != 3);
    }
  }

  // set reference index of redundant slices.
  if(currSlice->redundant_pic_cnt && (p_Vid->type != I_SLICE) )
  {
    currSlice->redundant_slice_ref_idx = currSlice->abs_diff_pic_num_minus1[LIST_0][0] + 1;
  }
}

/*!
 ************************************************************************
 * \brief
 *    read the MVC reference picture reordering information
 ************************************************************************
 */
#if MVC_EXTENSION_ENABLE||EXT3D
static void ref_pic_list_mvc_modification(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  byte dP_nr = assignSE2partition[currSlice->dp_mode][SE_HEADER];
  DataPartition *partition = &(currSlice->partArr[dP_nr]);
  Bitstream *currStream = partition->bitstream;
  int i, val;

  alloc_ref_pic_list_reordering_buffer(currSlice);

  if ((p_Vid->type % 5) != I_SLICE && (p_Vid->type % 5) != SI_SLICE)
  {
    val = currSlice->ref_pic_list_reordering_flag[LIST_0] = u_1 ("SH: ref_pic_list_modification_flag_l0", currStream);

    if (val)
    {
      i=0;
      do
      {
        val = currSlice->reordering_of_pic_nums_idc[LIST_0][i] = ue_v("SH: modification_of_pic_nums_idc_l0", currStream);
        if (val==0 || val==1)
        {
          currSlice->abs_diff_pic_num_minus1[LIST_0][i] = ue_v("SH: abs_diff_pic_num_minus1_l0", currStream);
        }
        else
        {
          if (val==2)
          {
            currSlice->long_term_pic_idx[LIST_0][i] = ue_v("SH: long_term_pic_idx_l0", currStream);
          }
          else if (val==4 || val==5)
          {
            currSlice->abs_diff_view_idx_minus1[LIST_0][i] = ue_v("SH: abs_diff_view_idx_minus1_l0", currStream);
          }
        }
        i++;
        // assert (i>img->num_ref_idx_l0_active);
      } while (val != 3);
    }
  }

  if ((p_Vid->type % 5) == B_SLICE)
  {
    val = currSlice->ref_pic_list_reordering_flag[LIST_1] = u_1 ("SH: ref_pic_list_reordering_flag_l1", currStream);

    if (val)
    {
      i=0;
      do
      {
        val = currSlice->reordering_of_pic_nums_idc[LIST_1][i] = ue_v("SH: modification_of_pic_nums_idc_l1", currStream);
        if (val==0 || val==1)
        {
          currSlice->abs_diff_pic_num_minus1[LIST_1][i] = ue_v("SH: abs_diff_pic_num_minus1_l1", currStream);
        }
        else
        {
          if (val==2)
          {
            currSlice->long_term_pic_idx[LIST_1][i] = ue_v("SH: long_term_pic_idx_l1", currStream);
          }
          else if (val==4 || val==5)
          {
            currSlice->abs_diff_view_idx_minus1[LIST_1][i] = ue_v("SH: abs_diff_view_idx_minus1_l1", currStream);
          }
        }
        i++;
        // assert (i>img->num_ref_idx_l1_active);
      } while (val != 3);
    }
  }

  // set reference index of redundant slices.
  if(currSlice->redundant_pic_cnt && (p_Vid->type != I_SLICE) )
  {
    currSlice->redundant_slice_ref_idx = currSlice->abs_diff_pic_num_minus1[LIST_0][0] + 1;
  }
}
#endif

static void reset_wp_params(Slice *currSlice)
{
  int i,comp;
  int log_weight_denom;

  for (i=0; i<MAX_REFERENCE_PICTURES; i++)
  {
    for (comp=0; comp<3; comp++)
    {
      log_weight_denom = (comp == 0) ? currSlice->luma_log2_weight_denom : currSlice->chroma_log2_weight_denom;
      currSlice->wp_weight[0][i][comp] = 1 << log_weight_denom;
      currSlice->wp_weight[1][i][comp] = 1 << log_weight_denom;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    read the weighted prediction tables
 ************************************************************************
 */
static void pred_weight_table(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
  byte dP_nr = assignSE2partition[currSlice->dp_mode][SE_HEADER];
  DataPartition *partition = &(currSlice->partArr[dP_nr]);
  Bitstream *currStream = partition->bitstream;
  int luma_weight_flag_l0, luma_weight_flag_l1, chroma_weight_flag_l0, chroma_weight_flag_l1;
  int i,j;

  currSlice->luma_log2_weight_denom = (unsigned short) ue_v ("SH: luma_log2_weight_denom", currStream);
  currSlice->wp_round_luma = currSlice->luma_log2_weight_denom ? 1<<(currSlice->luma_log2_weight_denom - 1): 0;

  if ( 0 != active_sps->chroma_format_idc)
  {
    currSlice->chroma_log2_weight_denom = (unsigned short) ue_v ("SH: chroma_log2_weight_denom", currStream);
    currSlice->wp_round_chroma = currSlice->chroma_log2_weight_denom ? 1<<(currSlice->chroma_log2_weight_denom - 1): 0;
  }

  reset_wp_params(currSlice);

  for (i=0; i<currSlice->num_ref_idx_active[LIST_0]; i++)
  {
    luma_weight_flag_l0 = u_1("SH: luma_weight_flag_l0", currStream);

    if (luma_weight_flag_l0)
    {
      currSlice->wp_weight[0][i][0] = se_v ("SH: luma_weight_l0", currStream);
      currSlice->wp_offset[0][i][0] = se_v ("SH: luma_offset_l0", currStream);
      currSlice->wp_offset[0][i][0] = currSlice->wp_offset[0][i][0]<<(p_Vid->bitdepth_luma - 8);
    }
    else
    {
      currSlice->wp_weight[0][i][0] = 1 << currSlice->luma_log2_weight_denom;
      currSlice->wp_offset[0][i][0] = 0;
    }

    if (active_sps->chroma_format_idc != 0)
    {
      chroma_weight_flag_l0 = u_1 ("SH: chroma_weight_flag_l0", currStream);

      for (j=1; j<3; j++)
      {
        if (chroma_weight_flag_l0)
        {
          currSlice->wp_weight[0][i][j] = se_v("SH: chroma_weight_l0", currStream);
          currSlice->wp_offset[0][i][j] = se_v("SH: chroma_offset_l0", currStream);
          currSlice->wp_offset[0][i][j] = currSlice->wp_offset[0][i][j]<<(p_Vid->bitdepth_chroma-8);
        }
        else
        {
          currSlice->wp_weight[0][i][j] = 1<<currSlice->chroma_log2_weight_denom;
          currSlice->wp_offset[0][i][j] = 0;
        }
      }
    }
  }
  if ((p_Vid->type == B_SLICE) && p_Vid->active_pps->weighted_bipred_idc == 1)
  {
    for (i=0; i<currSlice->num_ref_idx_active[LIST_1]; i++)
    {
      luma_weight_flag_l1 = u_1("SH: luma_weight_flag_l1", currStream);

      if (luma_weight_flag_l1)
      {
        currSlice->wp_weight[1][i][0] = se_v ("SH: luma_weight_l1", currStream);
        currSlice->wp_offset[1][i][0] = se_v ("SH: luma_offset_l1", currStream);
        currSlice->wp_offset[1][i][0] = currSlice->wp_offset[1][i][0]<<(p_Vid->bitdepth_luma-8);
      }
      else
      {
        currSlice->wp_weight[1][i][0] = 1<<currSlice->luma_log2_weight_denom;
        currSlice->wp_offset[1][i][0] = 0;
      }

      if (active_sps->chroma_format_idc != 0)
      {
        chroma_weight_flag_l1 = u_1 ("SH: chroma_weight_flag_l1", currStream);

        for (j=1; j<3; j++)
        {
          if (chroma_weight_flag_l1)
          {
            currSlice->wp_weight[1][i][j] = se_v("SH: chroma_weight_l1", currStream);
            currSlice->wp_offset[1][i][j] = se_v("SH: chroma_offset_l1", currStream);
            currSlice->wp_offset[1][i][j] = currSlice->wp_offset[1][i][j]<<(p_Vid->bitdepth_chroma-8);
          }
          else
          {
            currSlice->wp_weight[1][i][j] = 1<<currSlice->chroma_log2_weight_denom;
            currSlice->wp_offset[1][i][j] = 0;
          }
        }
      }
    }
  }
}


/*!
 ************************************************************************
 * \brief
 *    read the memory control operations
 ************************************************************************
 */
void dec_ref_pic_marking(VideoParameters *p_Vid, Bitstream *currStream, Slice *pSlice)
{
  int val;

  DecRefPicMarking_t *tmp_drpm,*tmp_drpm2;

  // free old buffer content
  while (pSlice->dec_ref_pic_marking_buffer)
  {
    tmp_drpm=pSlice->dec_ref_pic_marking_buffer;

    pSlice->dec_ref_pic_marking_buffer=tmp_drpm->Next;
    free (tmp_drpm);
  }

  if (pSlice->idr_flag)
  {
    pSlice->no_output_of_prior_pics_flag = u_1("SH: no_output_of_prior_pics_flag", currStream);
    p_Vid->no_output_of_prior_pics_flag = pSlice->no_output_of_prior_pics_flag;
    pSlice->long_term_reference_flag = u_1("SH: long_term_reference_flag", currStream);
  }
  else
  {
    pSlice->adaptive_ref_pic_buffering_flag = u_1("SH: adaptive_ref_pic_buffering_flag", currStream);
    if (pSlice->adaptive_ref_pic_buffering_flag)
    {
      // read Memory Management Control Operation
      do
      {
        tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t));
        tmp_drpm->Next=NULL;

        val = tmp_drpm->memory_management_control_operation = ue_v("SH: memory_management_control_operation", currStream);

        if ((val==1)||(val==3))
        {
          tmp_drpm->difference_of_pic_nums_minus1 = ue_v("SH: difference_of_pic_nums_minus1", currStream);
        }
        if (val==2)
        {
          tmp_drpm->long_term_pic_num = ue_v("SH: long_term_pic_num", currStream);
        }

        if ((val==3)||(val==6))
        {
          tmp_drpm->long_term_frame_idx = ue_v("SH: long_term_frame_idx", currStream);
        }
        if (val==4)
        {
          tmp_drpm->max_long_term_frame_idx_plus1 = ue_v("SH: max_long_term_pic_idx_plus1", currStream);
        }

        // add command
        if (pSlice->dec_ref_pic_marking_buffer==NULL)
        {
          pSlice->dec_ref_pic_marking_buffer=tmp_drpm;
        }
        else
        {
          tmp_drpm2=pSlice->dec_ref_pic_marking_buffer;
          while (tmp_drpm2->Next!=NULL) tmp_drpm2=tmp_drpm2->Next;
          tmp_drpm2->Next=tmp_drpm;
        }

      }
      while (val != 0);
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    To calculate the poc values
 *        based upon JVT-F100d2
 *  POC200301: Until Jan 2003, this function will calculate the correct POC
 *    values, but the management of POCs in buffered pictures may need more work.
 * \return
 *    none
 ************************************************************************
 */
void decode_poc(VideoParameters *p_Vid, Slice *pSlice)
{
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
  int i;
  // for POC mode 0:
  unsigned int MaxPicOrderCntLsb = (1<<(active_sps->log2_max_pic_order_cnt_lsb_minus4+4));

  switch ( active_sps->pic_order_cnt_type )
  {
  case 0: // POC MODE 0
    // 1st
    if(pSlice->idr_flag)
    {
      p_Vid->PrevPicOrderCntMsb = 0;
      p_Vid->PrevPicOrderCntLsb = 0;
    }
    else
    {
#if EXT3D
      if (p_Vid->last_has_mmco_5[pSlice->is_depth])
#else
      if (p_Vid->last_has_mmco_5)
#endif
      {
        if (p_Vid->last_pic_bottom_field)
        {
          p_Vid->PrevPicOrderCntMsb = 0;
          p_Vid->PrevPicOrderCntLsb = 0;
        }
        else
        {
          p_Vid->PrevPicOrderCntMsb = 0;
          p_Vid->PrevPicOrderCntLsb = pSlice->toppoc;
        }
      }
    }
    // Calculate the MSBs of current picture
    if( pSlice->pic_order_cnt_lsb  <  p_Vid->PrevPicOrderCntLsb  &&
      ( p_Vid->PrevPicOrderCntLsb - pSlice->pic_order_cnt_lsb )  >=  ( MaxPicOrderCntLsb / 2 ) )
      pSlice->PicOrderCntMsb = p_Vid->PrevPicOrderCntMsb + MaxPicOrderCntLsb;
    else if ( pSlice->pic_order_cnt_lsb  >  p_Vid->PrevPicOrderCntLsb  &&
      ( pSlice->pic_order_cnt_lsb - p_Vid->PrevPicOrderCntLsb )  >  ( MaxPicOrderCntLsb / 2 ) )
      pSlice->PicOrderCntMsb = p_Vid->PrevPicOrderCntMsb - MaxPicOrderCntLsb;
    else
      pSlice->PicOrderCntMsb = p_Vid->PrevPicOrderCntMsb;

    // 2nd

    if(pSlice->field_pic_flag==0)
    {           //frame pix
      pSlice->toppoc = pSlice->PicOrderCntMsb + pSlice->pic_order_cnt_lsb;
      pSlice->bottompoc = pSlice->toppoc + pSlice->delta_pic_order_cnt_bottom;
      pSlice->ThisPOC = pSlice->framepoc = (pSlice->toppoc < pSlice->bottompoc)? pSlice->toppoc : pSlice->bottompoc; // POC200301
    }
    else if (pSlice->bottom_field_flag == FALSE)
    {  //top field
      pSlice->ThisPOC= pSlice->toppoc = pSlice->PicOrderCntMsb + pSlice->pic_order_cnt_lsb;
    }
    else
    {  //bottom field
      pSlice->ThisPOC= pSlice->bottompoc = pSlice->PicOrderCntMsb + pSlice->pic_order_cnt_lsb;
    }
    pSlice->framepoc = pSlice->ThisPOC;

    p_Vid->ThisPOC = pSlice->ThisPOC;

    if ( pSlice->frame_num != p_Vid->PreviousFrameNum)
      p_Vid->PreviousFrameNum = pSlice->frame_num;

    if(pSlice->nal_reference_idc)
    {
      p_Vid->PrevPicOrderCntLsb = pSlice->pic_order_cnt_lsb;
      p_Vid->PrevPicOrderCntMsb = pSlice->PicOrderCntMsb;
    }

    break;

  case 1: // POC MODE 1
    // 1st
    if(pSlice->idr_flag)
    {
      p_Vid->FrameNumOffset=0;     //  first pix of IDRGOP,
      if(pSlice->frame_num)
        error("frame_num not equal to zero in IDR picture", -1020);
    }
    else
    {
#if EXT3D
      if (p_Vid->last_has_mmco_5[pSlice->is_depth])
#else
      if (p_Vid->last_has_mmco_5)
#endif
      {
        p_Vid->PreviousFrameNumOffset = 0;
        p_Vid->PreviousFrameNum = 0;
      }
      if (pSlice->frame_num<p_Vid->PreviousFrameNum)
      {             //not first pix of IDRGOP
        p_Vid->FrameNumOffset = p_Vid->PreviousFrameNumOffset + p_Vid->MaxFrameNum;
      }
      else
      {
        p_Vid->FrameNumOffset = p_Vid->PreviousFrameNumOffset;
      }
    }

    // 2nd
    if(active_sps->num_ref_frames_in_pic_order_cnt_cycle)
      pSlice->AbsFrameNum = p_Vid->FrameNumOffset+pSlice->frame_num;
    else
      pSlice->AbsFrameNum=0;
    if( (!pSlice->nal_reference_idc) && pSlice->AbsFrameNum > 0)
      pSlice->AbsFrameNum--;

    // 3rd
    p_Vid->ExpectedDeltaPerPicOrderCntCycle=0;

    if(active_sps->num_ref_frames_in_pic_order_cnt_cycle)
    for(i=0;i<(int) active_sps->num_ref_frames_in_pic_order_cnt_cycle;i++)
      p_Vid->ExpectedDeltaPerPicOrderCntCycle += active_sps->offset_for_ref_frame[i];

    if(pSlice->AbsFrameNum)
    {
      p_Vid->PicOrderCntCycleCnt = (pSlice->AbsFrameNum-1)/active_sps->num_ref_frames_in_pic_order_cnt_cycle;
      p_Vid->FrameNumInPicOrderCntCycle = (pSlice->AbsFrameNum-1)%active_sps->num_ref_frames_in_pic_order_cnt_cycle;
      p_Vid->ExpectedPicOrderCnt = p_Vid->PicOrderCntCycleCnt*p_Vid->ExpectedDeltaPerPicOrderCntCycle;
      for(i=0;i<=(int)p_Vid->FrameNumInPicOrderCntCycle;i++)
        p_Vid->ExpectedPicOrderCnt += active_sps->offset_for_ref_frame[i];
    }
    else
      p_Vid->ExpectedPicOrderCnt=0;

    if(!pSlice->nal_reference_idc)
      p_Vid->ExpectedPicOrderCnt += active_sps->offset_for_non_ref_pic;

    if(pSlice->field_pic_flag==0)
    {           //frame pix
      pSlice->toppoc = p_Vid->ExpectedPicOrderCnt + pSlice->delta_pic_order_cnt[0];
      pSlice->bottompoc = pSlice->toppoc + active_sps->offset_for_top_to_bottom_field + pSlice->delta_pic_order_cnt[1];
      pSlice->ThisPOC = pSlice->framepoc = (pSlice->toppoc < pSlice->bottompoc)? pSlice->toppoc : pSlice->bottompoc; // POC200301
    }
    else if (pSlice->bottom_field_flag == FALSE)
    {  //top field
      pSlice->ThisPOC = pSlice->toppoc = p_Vid->ExpectedPicOrderCnt + pSlice->delta_pic_order_cnt[0];
    }
    else
    {  //bottom field
      pSlice->ThisPOC = pSlice->bottompoc = p_Vid->ExpectedPicOrderCnt + active_sps->offset_for_top_to_bottom_field + pSlice->delta_pic_order_cnt[0];
    }
    pSlice->framepoc=pSlice->ThisPOC;

    p_Vid->PreviousFrameNum=pSlice->frame_num;
    p_Vid->PreviousFrameNumOffset=p_Vid->FrameNumOffset;

    break;


  case 2: // POC MODE 2
    if(pSlice->idr_flag) // IDR picture
    {
      p_Vid->FrameNumOffset=0;     //  first pix of IDRGOP,
      pSlice->ThisPOC = pSlice->framepoc = pSlice->toppoc = pSlice->bottompoc = 0;
      if(pSlice->frame_num)
        error("frame_num not equal to zero in IDR picture", -1020);
    }
    else
    {
#if EXT3D
      if (p_Vid->last_has_mmco_5[pSlice->is_depth])
#else
      if (p_Vid->last_has_mmco_5)
#endif
      {
        p_Vid->PreviousFrameNum = 0;
        p_Vid->PreviousFrameNumOffset = 0;
      }
      if (pSlice->frame_num<p_Vid->PreviousFrameNum)
        p_Vid->FrameNumOffset = p_Vid->PreviousFrameNumOffset + p_Vid->MaxFrameNum;
      else
        p_Vid->FrameNumOffset = p_Vid->PreviousFrameNumOffset;


      pSlice->AbsFrameNum = p_Vid->FrameNumOffset+pSlice->frame_num;
      if(!pSlice->nal_reference_idc)
        pSlice->ThisPOC = (2*pSlice->AbsFrameNum - 1);
      else
        pSlice->ThisPOC = (2*pSlice->AbsFrameNum);

      if (pSlice->field_pic_flag==0)
        pSlice->toppoc = pSlice->bottompoc = pSlice->framepoc = pSlice->ThisPOC;
      else if (pSlice->bottom_field_flag == FALSE)
         pSlice->toppoc = pSlice->framepoc = pSlice->ThisPOC;
      else 
        pSlice->bottompoc = pSlice->framepoc = pSlice->ThisPOC;
    }

    p_Vid->PreviousFrameNum=pSlice->frame_num;
    p_Vid->PreviousFrameNumOffset=p_Vid->FrameNumOffset;
    break;


  default:
    //error must occurs
    assert( 1==0 );
    break;
  }
}

/*!
 ************************************************************************
 * \brief
 *    A little helper for the debugging of POC code
 * \return
 *    none
 ************************************************************************
 */
int dumppoc(VideoParameters *p_Vid) 
{
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

  printf ("\nPOC locals...\n");
  printf ("toppoc                                %d\n", (int) p_Vid->ppSliceList[0]->toppoc);
  printf ("bottompoc                             %d\n", (int) p_Vid->ppSliceList[0]->bottompoc);
  printf ("frame_num                             %d\n", (int) p_Vid->ppSliceList[0]->frame_num);
  printf ("field_pic_flag                        %d\n", (int) p_Vid->ppSliceList[0]->field_pic_flag);
  printf ("bottom_field_flag                     %d\n", (int) p_Vid->ppSliceList[0]->bottom_field_flag);
  printf ("POC SPS\n");
  printf ("log2_max_frame_num_minus4             %d\n", (int) active_sps->log2_max_frame_num_minus4);         // POC200301
  printf ("log2_max_pic_order_cnt_lsb_minus4     %d\n", (int) active_sps->log2_max_pic_order_cnt_lsb_minus4);
  printf ("pic_order_cnt_type                    %d\n", (int) active_sps->pic_order_cnt_type);
  printf ("num_ref_frames_in_pic_order_cnt_cycle %d\n", (int) active_sps->num_ref_frames_in_pic_order_cnt_cycle);
  printf ("delta_pic_order_always_zero_flag      %d\n", (int) active_sps->delta_pic_order_always_zero_flag);
  printf ("offset_for_non_ref_pic                %d\n", (int) active_sps->offset_for_non_ref_pic);
  printf ("offset_for_top_to_bottom_field        %d\n", (int) active_sps->offset_for_top_to_bottom_field);
  printf ("offset_for_ref_frame[0]               %d\n", (int) active_sps->offset_for_ref_frame[0]);
  printf ("offset_for_ref_frame[1]               %d\n", (int) active_sps->offset_for_ref_frame[1]);
  printf ("POC in SLice Header\n");
  printf ("bottom_field_pic_order_in_frame_present_flag                %d\n", (int) p_Vid->active_pps->bottom_field_pic_order_in_frame_present_flag);
  printf ("delta_pic_order_cnt[0]                %d\n", (int) p_Vid->ppSliceList[0]->delta_pic_order_cnt[0]);
  printf ("delta_pic_order_cnt[1]                %d\n", (int) p_Vid->ppSliceList[0]->delta_pic_order_cnt[1]);
  printf ("idr_flag                              %d\n", (int) p_Vid->ppSliceList[0]->idr_flag);
  printf ("MaxFrameNum                           %d\n", (int) p_Vid->MaxFrameNum);

  return 0;
}

/*!
 ************************************************************************
 * \brief
 *    return the poc of p_Vid as per (8-1) JVT-F100d2
 *  POC200301
 ************************************************************************
 */
int picture_order( Slice *pSlice )
{
  if (pSlice->field_pic_flag==0) // is a frame
    return pSlice->framepoc;
  else if (pSlice->bottom_field_flag == FALSE) // top field
    return pSlice->toppoc;
  else // bottom field
    return pSlice->bottompoc;
}

