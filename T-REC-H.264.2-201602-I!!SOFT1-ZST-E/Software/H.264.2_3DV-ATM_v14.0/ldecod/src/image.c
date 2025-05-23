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
 ***********************************************************************
 * \file image.c
 *
 * \brief
 *    Decode a Slice
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
 *    - Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
 *    - Jani Lainema                    <jani.lainema@nokia.com>
 *    - Sebastian Purreiter             <sebastian.purreiter@mch.siemens.de>
 *    - Byeong-Moon Jeon                <jeonbm@lge.com>
 *    - Thomas Wedi                     <wedi@tnt.uni-hannover.de>
 *    - Gabi Blaettermann
 *    - Ye-Kui Wang                     <wyk@ieee.org>
 *    - Antti Hallapuro                 <antti.hallapuro@nokia.com>
 *    - Alexis Tourapis                 <alexismt@ieee.org>
 *    - Jill Boyce                      <jill.boyce@thomson.net>
 *    - Saurav K Bandyopadhyay          <saurav@ieee.org>
 *    - Zhenyu Wu                       <Zhenyu.Wu@thomson.net
 *    - Purvin Pandit                   <Purvin.Pandit@thomson.net>
 *
 ***********************************************************************
 */

#include "contributors.h"

#include <math.h>
#include <limits.h>

#include "global.h"
#include "image.h"
#include "fmo.h"
#include "annexb.h"
#include "nalu.h"
#include "parset.h"
#include "header.h"

#include "sei.h"
#include "output.h"
#include "mb_access.h"
#include "memalloc.h"
#include "macroblock.h"

#include "loopfilter.h"

#include "biaridecod.h"
#include "context_ini.h"
#include "cabac.h"
#include "vlc.h"
#include "quant.h"

#include "errorconcealment.h"
#include "erc_api.h"
#include "mbuffer_mvc.h"
#include "fast_memory.h"

#include "mc_prediction.h"

#if EXT3D
#include "resample.h"

extern int  g_bFound_dec;
extern int  g_NonlinearDepthNum_dec;
extern char g_NonlinearDepthPoints_dec[256];    
#endif

extern int testEndian(void);
void reorder_lists(Slice *currSlice);

static inline void reset_mbs(Macroblock *currMB)
{
  currMB->slice_nr = -1; 
  currMB->ei_flag  =  1;
  currMB->dpl_flag =  0;
}

static inline void reset_mv_info(PicMotionParams *mv_info)
{
  mv_info->ref_pic[LIST_0] = NULL;
  mv_info->ref_pic[LIST_1] = NULL;
  mv_info->mv[LIST_0] = zero_mv;
  mv_info->mv[LIST_1] = zero_mv;
  mv_info->ref_idx[LIST_0] = -1;
  mv_info->ref_idx[LIST_1] = -1;
}

/*!
 ************************************************************************
 * \brief
 *    init macroblock I and P frames
 ************************************************************************
 */
void init_all_macroblocks(StorablePicture *dec_picture)
{
  int j;
  PicMotionParams *mv_info = dec_picture->mv_info[0];

  // reset vectors and pred. modes
  for(j = 0; j < ((dec_picture->size_x * dec_picture->size_y) >> 4); ++j)
  {                        
    reset_mv_info(mv_info++);
  }
}


#if EXT3D
/*!
 ************************************************************************
 * \brief
 *    reset the picture's  according to its category (texture or depth)
 ************************************************************************
 */
static void reset_picture_size(VideoParameters *p_Vid, Slice *currSlice)
{
  int PicParsetId = currSlice->pic_parameter_set_id;  
  pic_parameter_set_rbsp_t *pps = &p_Vid->PicParSet[PicParsetId];
  seq_parameter_set_rbsp_t *sps = &p_Vid->SeqParSet[pps->seq_parameter_set_id];

  if(currSlice->svc_extension_flag != -1)
  {
    // Set SPS to the subset SPS parameters
    p_Vid->active_subset_sps = p_Vid->SubsetSeqParSet + pps->seq_parameter_set_id;
    sps = &(p_Vid->active_subset_sps->sps);

  }

  p_Vid->PicWidthInMbs = (sps->pic_width_in_mbs_minus1 +1);
  p_Vid->PicHeightInMapUnits = (sps->pic_height_in_map_units_minus1 +1);
  p_Vid->FrameHeightInMbs = ( 2 - sps->frame_mbs_only_flag ) * p_Vid->PicHeightInMapUnits;

  p_Vid->PicHeightInMbs = p_Vid->FrameHeightInMbs / ( 1 + currSlice->field_pic_flag );
  p_Vid->PicSizeInMbs   = p_Vid->PicWidthInMbs * p_Vid->PicHeightInMbs;
  p_Vid->FrameSizeInMbs = p_Vid->PicWidthInMbs * p_Vid->FrameHeightInMbs;
}

/*!
 ************************************************************************
 * \brief
 *  get the corresponding texture picture or depth map of current decoding depth map or texture picture
 ************************************************************************
 */
void get_dual_picture(VideoParameters* p_Vid, Slice* currSlice)
{
  DecodedPictureBuffer* p_dual_Dpb=p_Vid->p_Dpb[1^(currSlice->is_depth)];
  unsigned int i;

  p_Vid->p_dual_picture=NULL;
  if(p_dual_Dpb)
  {
    for(i=0;i<p_dual_Dpb->used_size;++i)
    {
#if ITRI_INTERLACE
    struct storable_picture* picture;
    if(currSlice->structure==FRAME)
    {
      if (p_dual_Dpb->fs[i]->frame)
      picture=p_dual_Dpb->fs[i]->frame;
      else
      picture=p_dual_Dpb->fs[i]->top_field;
    }
    else
    {
      if (p_dual_Dpb->fs[i]->top_field)
        picture=p_dual_Dpb->fs[i]->top_field;
      else
        picture=p_dual_Dpb->fs[i]->frame;
    }
#else
      struct storable_picture* picture=p_dual_Dpb->fs[i]->frame;
#endif // #if ITRI_INTERLACE

      if((picture->view_id==currSlice->view_id)&&(picture->frame_num==currSlice->frame_num)&&(picture->poc==currSlice->framepoc))
      {
        p_Vid->p_dual_picture=picture;
        break;
      }
    }
  }
}
#endif

/*!
 ************************************************************************
 * \brief
 *    Initializes the parameters for a new picture
 ************************************************************************
 */
static void init_picture(VideoParameters *p_Vid, Slice *currSlice, InputParameters *p_Inp)
{
  int i;
  int nplane;
  StorablePicture *dec_picture = NULL;
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
  DecodedPictureBuffer *p_Dpb = currSlice->p_Dpb;

#if EXT3D
  int voidx=GetVOIdx(p_Vid,currSlice->view_id);
  reset_picture_size(p_Vid,currSlice);

  active_sps->is_depth=currSlice->is_depth;
#endif

  p_Vid->bFrameInit = 1;
  if (p_Vid->dec_picture) // && p_Vid->num_dec_mb == p_Vid->PicSizeInMbs)
  {
    // this may only happen on slice loss
    exit_picture(p_Vid, &p_Vid->dec_picture);
  }
  if (p_Vid->recovery_point)
    p_Vid->recovery_frame_num = (currSlice->frame_num + p_Vid->recovery_frame_cnt) % p_Vid->MaxFrameNum;

  if (currSlice->idr_flag)
    p_Vid->recovery_frame_num = currSlice->frame_num;

  if (p_Vid->recovery_point == 0 &&
    currSlice->frame_num != p_Vid->pre_frame_num &&
    currSlice->frame_num != (p_Vid->pre_frame_num + 1) % p_Vid->MaxFrameNum)
  {
    if (active_sps->gaps_in_frame_num_value_allowed_flag == 0)
    {
      // picture error concealment
      if(p_Inp->conceal_mode !=0)
      {
        if((currSlice->frame_num) < ((p_Vid->pre_frame_num + 1) % p_Vid->MaxFrameNum))
        {
          /* Conceal lost IDR frames and any frames immediately
             following the IDR. Use frame copy for these since
             lists cannot be formed correctly for motion copy*/
          p_Vid->conceal_mode = 1;
          p_Vid->IDR_concealment_flag = 1;
          conceal_lost_frames(p_Dpb, currSlice);
          //reset to original concealment mode for future drops
          p_Vid->conceal_mode = p_Inp->conceal_mode;
        }
        else
        {
          //reset to original concealment mode for future drops
          p_Vid->conceal_mode = p_Inp->conceal_mode;

          p_Vid->IDR_concealment_flag = 0;
          conceal_lost_frames(p_Dpb, currSlice);
        }
      }
      else
      {   /* Advanced Error Concealment would be called here to combat unintentional loss of pictures. */
        error("An unintentional loss of pictures occurs! Exit\n", 100);
      }
    }
    if(p_Vid->conceal_mode == 0)
      fill_frame_num_gap(p_Vid, currSlice);
  }

  if(currSlice->nal_reference_idc)
  {
    p_Vid->pre_frame_num = currSlice->frame_num;
  }

  //p_Vid->num_dec_mb = 0;

  //calculate POC
  decode_poc(p_Vid, currSlice);

  if (p_Vid->recovery_frame_num == (int) currSlice->frame_num && p_Vid->recovery_poc == 0x7fffffff)
    p_Vid->recovery_poc = currSlice->framepoc;

  if(currSlice->nal_reference_idc)
    p_Vid->last_ref_pic_poc = currSlice->framepoc;

  //  dumppoc (p_Vid);

  if (currSlice->structure==FRAME ||currSlice->structure==TOP_FIELD)
  {
    gettime (&(p_Vid->start_time));             // start time
  }

  dec_picture = p_Vid->dec_picture = alloc_storable_picture (p_Vid, currSlice->structure, p_Vid->width, p_Vid->height, p_Vid->width_cr, p_Vid->height_cr);
  dec_picture->top_poc=currSlice->toppoc;
  dec_picture->bottom_poc=currSlice->bottompoc;
  dec_picture->frame_poc=currSlice->framepoc;
  dec_picture->qp = currSlice->qp;
  dec_picture->slice_qp_delta = currSlice->slice_qp_delta;
  dec_picture->chroma_qp_offset[0] = p_Vid->active_pps->chroma_qp_index_offset;
  dec_picture->chroma_qp_offset[1] = p_Vid->active_pps->second_chroma_qp_index_offset;
  dec_picture->iCodingType = currSlice->structure==FRAME? (currSlice->mb_aff_frame_flag? FRAME_MB_PAIR_CODING:FRAME_CODING): FIELD_CODING; //currSlice->slice_type;
#if MVC_EXTENSION_ENABLE || EXT3D
  dec_picture->view_id         = currSlice->view_id;
  dec_picture->inter_view_flag = currSlice->inter_view_flag;
  dec_picture->anchor_pic_flag = currSlice->anchor_pic_flag;
#endif

#if EXT3D
  assert(currSlice->is_depth==active_sps->is_depth);
  dec_picture->sps       =  active_sps;
  dec_picture->is_depth    =  currSlice->is_depth;
  dec_picture->p_out       = &(p_Vid->p_out_3dv[currSlice->is_depth][voidx]);

  if (dec_picture->is_depth)
  {

  if(p_Vid->profile_idc==ThreeDV_EXTEND_HIGH)
  {
    if(g_bFound_dec==1)
    {
      //if(andr_flag==1)
      if(p_Vid->andr_flag==1)
      {
        dec_picture->NonlinearDepthNum=g_NonlinearDepthNum_dec;
        dec_picture->NonlinearDepthPoints[0]=0;   // beginning
        for (i=1; i<=g_NonlinearDepthNum_dec; ++i)
        {
          dec_picture->NonlinearDepthPoints[i]=g_NonlinearDepthPoints_dec[i];
          //printf("  Depth Points_dec:%d  \n", dec_picture->NonlinearDepthPoints[i]);
        }
        dec_picture->NonlinearDepthPoints[g_NonlinearDepthNum_dec+1]=0; // end
      }
      else
      {
        dec_picture->NonlinearDepthNum=0;
        dec_picture->NonlinearDepthPoints[0]=0;   // beginning
        for (i=1; i<=g_NonlinearDepthNum_dec; ++i)
        {
          dec_picture->NonlinearDepthPoints[i]=0;
          //printf("  Depth Points_dec:%d  \n", dec_picture->NonlinearDepthPoints[i]);
        }
        dec_picture->NonlinearDepthPoints[g_NonlinearDepthNum_dec+1]=0; // end
      }

    }

    // In the beginning, the NDR variables are set.
    if(g_bFound_dec==0)
    {
      dec_picture->NonlinearDepthNum = p_Vid->NonlinearDepthNum;
      memcpy(dec_picture->NonlinearDepthPoints, p_Vid->NonlinearDepthPoints, sizeof(p_Vid->NonlinearDepthPoints));
      g_bFound_dec=1;

      //////////////////////////////////////////////////////////////////////////
      // Initialization
      if(p_Vid->andr_flag==0)
      {
        dec_picture->NonlinearDepthNum=0;
        dec_picture->NonlinearDepthPoints[0]=0;   // beginning
        for (i=1; i<=g_NonlinearDepthNum_dec; ++i)
        {
          dec_picture->NonlinearDepthPoints[i]=0;
          //printf("  Depth Points_dec:%d  \n", dec_picture->NonlinearDepthPoints[i]);
        }
        dec_picture->NonlinearDepthPoints[g_NonlinearDepthNum_dec+1]=0; // end
      }
    }
  }

  } else {
    dec_picture->NonlinearDepthNum = 0;    
    dec_picture->NonlinearDepthPoints[0]=0;
    dec_picture->NonlinearDepthPoints[1]=0;
  }

  //POZNAN_NTT_DEPTH_REPRESENTATION_SEI
  if (dec_picture->is_depth)
  {
    dec_picture->SEI_NonlinearDepthNum = p_Vid->SEI_NonlinearDepthNum[dec_picture->view_id];
    memcpy(dec_picture->SEI_NonlinearDepthPoints, p_Vid->SEI_NonlinearDepthPoints[dec_picture->view_id], sizeof(dec_picture->SEI_NonlinearDepthPoints));
  } else {
    dec_picture->SEI_NonlinearDepthNum = 0;    
    dec_picture->SEI_NonlinearDepthPoints[0]=0;
    dec_picture->SEI_NonlinearDepthPoints[1]=0;
  }

  dec_picture->reduced_res     = currSlice->is_depth?p_Vid->low_res_depth:0;
  dec_picture->upsampling_params=currSlice->is_depth?p_Vid->p_depth_upsample_params:NULL;

  if(p_Vid->SliceHeaderPred)
  {
#if FIX_SLICE_HEAD_PRED
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].slice_type      = currSlice->slice_type ;
#endif
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].colour_plane_id = (char)currSlice->colour_plane_id;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].start_mb_nr     = currSlice->start_mb_nr;

    p_Vid->slice_header_dual[voidx][currSlice->is_depth].slice_group_change_cycle = currSlice->slice_group_change_cycle;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].DFDisableIdc    = (char)currSlice->DFDisableIdc;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].DFAlphaC0Offset = (char)currSlice->DFAlphaC0Offset;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].DFBetaOffset    = (char)currSlice->DFBetaOffset;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].frame_num       = currSlice->frame_num;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].field_pic_flag    = currSlice->field_pic_flag;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].bottom_field_flag = currSlice->bottom_field_flag;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].structure         = currSlice->structure;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].mb_aff_frame_flag = currSlice->mb_aff_frame_flag;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].idr_pic_id        = currSlice->idr_pic_id;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].pic_order_cnt_lsb = currSlice->pic_order_cnt_lsb;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].delta_pic_order_cnt_bottom   = currSlice->delta_pic_order_cnt_bottom;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].delta_pic_order_cnt[0]       = currSlice->delta_pic_order_cnt[0];
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].delta_pic_order_cnt[1]       = currSlice->delta_pic_order_cnt[1];
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].redundant_pic_cnt            = currSlice->redundant_pic_cnt;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].redundant_slice_ref_idx      = currSlice->redundant_slice_ref_idx;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].direct_spatial_mv_pred_flag  = (char)currSlice->direct_spatial_mv_pred_flag;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].num_ref_idx_active[LIST_0]   = (char)currSlice->num_ref_idx_active[LIST_0];
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].num_ref_idx_active[LIST_1]   = (char)currSlice->num_ref_idx_active[LIST_1];
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].model_number                 = currSlice->model_number;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].sp_switch                    = currSlice->sp_switch;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].slice_qs_delta               = currSlice->slice_qs_delta;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].qs                           = currSlice->qs;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].no_output_of_prior_pics_flag    = currSlice->no_output_of_prior_pics_flag;
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].long_term_reference_flag        = currSlice->long_term_reference_flag;       
    p_Vid->slice_header_dual[voidx][currSlice->is_depth].adaptive_ref_pic_buffering_flag = currSlice->adaptive_ref_pic_buffering_flag;

    {
      DecRefPicMarking_t *tmp_drpm, *tmp_drpm2, *tmp;
      while (p_Vid->slice_header_dual[voidx][currSlice->is_depth].dec_ref_pic_marking_buffer)
      {
        tmp_drpm=p_Vid->slice_header_dual[voidx][currSlice->is_depth].dec_ref_pic_marking_buffer;
        p_Vid->slice_header_dual[voidx][currSlice->is_depth].dec_ref_pic_marking_buffer=tmp_drpm->Next;
        free (tmp_drpm);
      }
      for (tmp=currSlice->dec_ref_pic_marking_buffer; tmp!=NULL; tmp=tmp->Next)
      {
        tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t));
        memcpy(tmp_drpm, tmp, sizeof (DecRefPicMarking_t));
        tmp_drpm->Next=NULL;

        if (p_Vid->slice_header_dual[voidx][currSlice->is_depth].dec_ref_pic_marking_buffer==NULL)
        {
          p_Vid->slice_header_dual[voidx][currSlice->is_depth].dec_ref_pic_marking_buffer=tmp_drpm;
        }
        else
        {
          tmp_drpm2=p_Vid->slice_header_dual[voidx][currSlice->is_depth].dec_ref_pic_marking_buffer;
          while (tmp_drpm2->Next!=NULL) tmp_drpm2=tmp_drpm2->Next;
          tmp_drpm2->Next=tmp_drpm;
        }
      }
    }

    {
      //store the weighted prediction para
      int bDepth = currSlice->is_depth;
      int i, j, comp;
      p_Vid->slice_header_dual[voidx][bDepth].luma_log2_weight_denom = currSlice->luma_log2_weight_denom ;
      p_Vid->slice_header_dual[voidx][bDepth].wp_round_luma          = currSlice->wp_round_luma ;

      p_Vid->slice_header_dual[voidx][bDepth].chroma_log2_weight_denom = currSlice->chroma_log2_weight_denom;
      p_Vid->slice_header_dual[voidx][bDepth].wp_round_chroma          = currSlice->wp_round_chroma;
  
      for(j = LIST_0; j <= LIST_1; j ++)
      {
        if(currSlice->wp_weight[j])
        {
          for (i=0; i<(currSlice->num_ref_idx_active[j]); i++)
          {
            for (comp=0; comp<3; comp++)
            {
              p_Vid->slice_header_dual[voidx][bDepth].wp_weight[j][i][comp] = currSlice->wp_weight[j][i][comp];
              p_Vid->slice_header_dual[voidx][bDepth].wp_offset[j][i][comp] = currSlice->wp_offset[j][i][comp];
              //wbp_weight
            }
          }
        }
      }
    }

    {
      //store the reference picture list para
      int bDepth = currSlice->is_depth;
      int i, j;
      for(j = LIST_0; j <=LIST_1; j++)
      {
       p_Vid->slice_header_dual[voidx][bDepth].ref_pic_list_reordering_flag [j]   = currSlice->ref_pic_list_reordering_flag[j];
       if(currSlice->reordering_of_pic_nums_idc[j]!=NULL)
       {
          for (i=0; i<(currSlice->num_ref_idx_active[j] + 1); i++)
          {
            p_Vid->slice_header_dual[voidx][bDepth].reordering_of_pic_nums_idc[j][i] = currSlice->reordering_of_pic_nums_idc[j][i];
            p_Vid->slice_header_dual[voidx][bDepth].abs_diff_pic_num_minus1[j][i]    = currSlice->abs_diff_pic_num_minus1[j][i];
            p_Vid->slice_header_dual[voidx][bDepth].long_term_pic_idx[j][i]          = currSlice->long_term_pic_idx[j][i];
          }
        }
        if( currSlice->abs_diff_view_idx_minus1[j] )
        {
          for(i=0; i<(MAX_VIEWREFERENCE+1); i++)
          {
            p_Vid->slice_header_dual[voidx][bDepth].abs_diff_view_idx_minus1[j][i]        = currSlice->abs_diff_view_idx_minus1[j][i] ;
            //p_Vid->slice_header_dual[voidx][bDepth].reordering_of_interview_num_idc[j][i] = currSlice->reordering_of_interview_num_idc[j][i];
          }
        }
      }
    }
  }
#endif

  // reset all variables of the error concealment instance before decoding of every frame.
  // here the third parameter should, if perfectly, be equal to the number of slices per frame.
  // using little value is ok, the code will allocate more memory if the slice number is larger
  ercReset(p_Vid->erc_errorVar, p_Vid->PicSizeInMbs, p_Vid->PicSizeInMbs, dec_picture->size_x);
  p_Vid->erc_mvperMB = 0;

  switch (currSlice->structure )
  {
  case TOP_FIELD:
    {
      dec_picture->poc = currSlice->toppoc;
      p_Vid->number *= 2;
      break;
    }
  case BOTTOM_FIELD:
    {
      dec_picture->poc = currSlice->bottompoc;
      p_Vid->number = p_Vid->number * 2 + 1;
      break;
    }
  case FRAME:
    {
      dec_picture->poc = currSlice->framepoc;
      break;
    }
  default:
    error("p_Vid->structure not initialized", 235);
  }

  //p_Vid->current_slice_nr=0;

  if (p_Vid->type > SI_SLICE)
  {
    set_ec_flag(p_Vid, SE_PTYPE);
    p_Vid->type = P_SLICE;  // concealed element
  }

  // CAVLC init
  if (p_Vid->active_pps->entropy_coding_mode_flag == (Boolean)(CAVLC))
  {
    memset(p_Vid->nz_coeff[0][0][0], -1, p_Vid->PicSizeInMbs * 48 *sizeof(byte)); // 3 * 4 * 4
  }

  // Set the slice_nr member of each MB to -1, to ensure correct when packet loss occurs
  // TO set Macroblock Map (mark all MBs as 'have to be concealed')
  if( (p_Vid->separate_colour_plane_flag != 0) )
  {
    for( nplane=0; nplane<MAX_PLANE; ++nplane )
    {      
      Macroblock *currMB = p_Vid->mb_data_JV[nplane];
      char *intra_block = p_Vid->intra_block_JV[nplane];
      for(i=0; i<(int)p_Vid->PicSizeInMbs; ++i)
      {
        reset_mbs(currMB++);
      }
      fast_memset(p_Vid->ipredmode_JV[nplane][0], DC_PRED, 16 * p_Vid->FrameHeightInMbs * p_Vid->PicWidthInMbs * sizeof(char));
      if(p_Vid->active_pps->constrained_intra_pred_flag)
      {
        for (i=0; i<(int)p_Vid->PicSizeInMbs; ++i)
        {
          intra_block[i] = 1;
        }
      }
    }
  }
  else
  {
#if 0 //defined(OPENMP)
#pragma omp parallel for
    for(i=0; i<(int)p_Vid->PicSizeInMbs; ++i)
      reset_mbs(&p_Vid->mb_data[i]);
#else
    Macroblock *currMB = p_Vid->mb_data;
    for(i=0; i<(int)p_Vid->PicSizeInMbs; ++i)
      reset_mbs(currMB++);
#endif
    if(p_Vid->active_pps->constrained_intra_pred_flag)
    {
      for (i=0; i<(int)p_Vid->PicSizeInMbs; ++i)
      {
        p_Vid->intra_block[i] = 1;
      }
    }
    fast_memset(p_Vid->ipredmode[0], DC_PRED, 16 * p_Vid->FrameHeightInMbs * p_Vid->PicWidthInMbs * sizeof(char));
  }  

  dec_picture->slice_type = p_Vid->type;
  dec_picture->used_for_reference = (currSlice->nal_reference_idc != 0);
  dec_picture->idr_flag = currSlice->idr_flag;
  dec_picture->no_output_of_prior_pics_flag = currSlice->no_output_of_prior_pics_flag;
  dec_picture->long_term_reference_flag     = currSlice->long_term_reference_flag;
  dec_picture->adaptive_ref_pic_buffering_flag = currSlice->adaptive_ref_pic_buffering_flag;

  dec_picture->dec_ref_pic_marking_buffer = currSlice->dec_ref_pic_marking_buffer;
  currSlice->dec_ref_pic_marking_buffer   = NULL;

  dec_picture->mb_aff_frame_flag = currSlice->mb_aff_frame_flag;
  dec_picture->PicWidthInMbs     = p_Vid->PicWidthInMbs;

  p_Vid->get_mb_block_pos = dec_picture->mb_aff_frame_flag ? get_mb_block_pos_mbaff : get_mb_block_pos_normal;
  p_Vid->getNeighbour     = dec_picture->mb_aff_frame_flag ? getAffNeighbour : getNonAffNeighbour;

  dec_picture->pic_num   = currSlice->frame_num;
  dec_picture->frame_num = currSlice->frame_num;

  dec_picture->recovery_frame = (unsigned int) ((int) currSlice->frame_num == p_Vid->recovery_frame_num);

  dec_picture->coded_frame = (currSlice->structure==FRAME);

  dec_picture->chroma_format_idc = active_sps->chroma_format_idc;

  dec_picture->frame_mbs_only_flag = active_sps->frame_mbs_only_flag;
  dec_picture->frame_cropping_flag = active_sps->frame_cropping_flag;

  if (dec_picture->frame_cropping_flag)
  {
    dec_picture->frame_cropping_rect_left_offset   = active_sps->frame_cropping_rect_left_offset;
    dec_picture->frame_cropping_rect_right_offset  = active_sps->frame_cropping_rect_right_offset;
    dec_picture->frame_cropping_rect_top_offset    = active_sps->frame_cropping_rect_top_offset;
    dec_picture->frame_cropping_rect_bottom_offset = active_sps->frame_cropping_rect_bottom_offset;
  }

#if (ENABLE_OUTPUT_TONEMAPPING)
  // store the necessary tone mapping sei into StorablePicture structure
  if (p_Vid->seiToneMapping->seiHasTone_mapping)
  {
    int coded_data_bit_max = (1 << p_Vid->seiToneMapping->coded_data_bit_depth);
    dec_picture->seiHasTone_mapping    = 1;
    dec_picture->tone_mapping_model_id = p_Vid->seiToneMapping->model_id;
    dec_picture->tonemapped_bit_depth  = p_Vid->seiToneMapping->sei_bit_depth;
#if EXT3D
    dec_picture->tone_mapping_lut      = (imgpel*)malloc(coded_data_bit_max * sizeof(int));
#else
    dec_picture->tone_mapping_lut      = malloc(coded_data_bit_max * sizeof(int));
#endif
    if (NULL == dec_picture->tone_mapping_lut)
    {
      no_mem_exit("init_picture: tone_mapping_lut");
    }
    memcpy(dec_picture->tone_mapping_lut, p_Vid->seiToneMapping->lut, sizeof(imgpel) * coded_data_bit_max);
    update_tone_mapping_sei(p_Vid->seiToneMapping);
  }
  else
    dec_picture->seiHasTone_mapping = 0;
#endif

  if( (p_Vid->separate_colour_plane_flag != 0) )
  {
    p_Vid->dec_picture_JV[0] = p_Vid->dec_picture;
    p_Vid->dec_picture_JV[1] = alloc_storable_picture (p_Vid, (PictureStructure) currSlice->structure, p_Vid->width, p_Vid->height, p_Vid->width_cr, p_Vid->height_cr);
    copy_dec_picture_JV( p_Vid, p_Vid->dec_picture_JV[1], p_Vid->dec_picture_JV[0] );
    p_Vid->dec_picture_JV[2] = alloc_storable_picture (p_Vid, (PictureStructure) currSlice->structure, p_Vid->width, p_Vid->height, p_Vid->width_cr, p_Vid->height_cr);
    copy_dec_picture_JV( p_Vid, p_Vid->dec_picture_JV[2], p_Vid->dec_picture_JV[0] );
  }
}

static void update_mbaff_macroblock_data(imgpel **cur_img, imgpel (*temp)[16], int x0, int width, int height)
{
  imgpel (*temp_evn)[16] = temp;
  imgpel (*temp_odd)[16] = temp + height; 
  imgpel **temp_img = cur_img;
  int y;

  for (y = 0; y < 2 * height; ++y)
    memcpy(*temp++, (*temp_img++ + x0), width * sizeof(imgpel));

  for (y = 0; y < height; ++y)
  {
    memcpy((*cur_img++ + x0), *temp_evn++, width * sizeof(imgpel));
    memcpy((*cur_img++ + x0), *temp_odd++, width * sizeof(imgpel));
  }
}

static void MbAffPostProc(VideoParameters *p_Vid)
{

  imgpel temp_buffer[32][16];

  StorablePicture *dec_picture = p_Vid->dec_picture;
  imgpel ** imgY  = dec_picture->imgY;
  imgpel ***imgUV = dec_picture->imgUV;

  short i, x0, y0;

  for (i=0; i<(int)dec_picture->PicSizeInMbs; i+=2)
  {
    if (dec_picture->motion.mb_field[i])
    {
      get_mb_pos(p_Vid, i, p_Vid->mb_size[IS_LUMA], &x0, &y0);
      update_mbaff_macroblock_data(imgY + y0, temp_buffer, x0, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

      if (dec_picture->chroma_format_idc != YUV400)
      {
        x0 = (short) ((x0 * p_Vid->mb_cr_size_x) >> 4);
        y0 = (short) ((y0 * p_Vid->mb_cr_size_y) >> 4);

        update_mbaff_macroblock_data(imgUV[0] + y0, temp_buffer, x0, p_Vid->mb_cr_size_x, p_Vid->mb_cr_size_y);
        update_mbaff_macroblock_data(imgUV[1] + y0, temp_buffer, x0, p_Vid->mb_cr_size_x, p_Vid->mb_cr_size_y);
      }
    }
  }
}

#if EXT3D
static void get_drb_wp_params(Slice* currSlice,
                short default_weight[3],
                short weight[6][MAX_REFERENCE_PICTURES][3],
                short offset[6][MAX_REFERENCE_PICTURES][3])
{
  //!<get weight and offset for depth based weight in B Slice
  VideoParameters *p_Vid = currSlice->p_Vid;
  int clist, n, comp;
  StorablePicture* curr=p_Vid->dec_picture;
  StorablePicture* curr_ref=NULL;

  short curr_weight=32;
  short curr_offset=0;

  int32 scale_w=8;
  int32 scale_o=8;

  int32 W_factora,W_factorb,W_factorc;
  int32 O_factora,O_factorb;

  int32 curr_depth_near=(int)curr->depth_near;
  int32 curr_depth_far=(int)curr->depth_far;
  int32 curr_ref_depth_near=0;
  int32 curr_ref_depth_far=0;
  int32 x,sign,temp1,temp2;

  for (clist=0; clist< 2; clist++)
  {
    for (n = 0; n < currSlice->listXsize[clist]; n++)
    {
      for (comp = 0; comp < 3; comp++)
      {
        weight[clist][n][comp] = default_weight[comp];
        offset[clist][n][comp] = 0;
      }
    }
  }
  for (clist = 0; clist < 2; clist++)
  {
    for (n = 0; n < currSlice->listXsize[clist]; n++)
    {
      curr_ref = currSlice->listX[clist][n];

      curr_ref_depth_near=(int)curr_ref->depth_near;
      curr_ref_depth_far=(int)curr_ref->depth_far;

      temp1 = curr_ref_depth_far - curr_ref_depth_near;
      temp2 = curr_ref_depth_far;
      x = ( temp1 + ( temp2 >> 1 ) ) / temp2;
      sign = ( (temp1 - x * temp2) < 0 ) ? -1 : 1;
      W_factora = (x<<scale_w);
      W_factora += ( ((temp1 - x * temp2) << scale_w) + sign * (temp2 >> 1) ) / temp2;

      temp1 = curr_depth_far;
      temp2 = curr_depth_far - curr_depth_near;
      x = ( temp1 + ( temp2 >> 1 ) ) / temp2;
      sign = ( (temp1 - x * temp2) < 0 ) ? -1 : 1;
      W_factorb = (x<<scale_w);
      W_factorb += ( ((temp1 - x * temp2) << scale_w) + sign * (temp2 >> 1) ) / temp2;

      temp1 = curr_depth_near;
      temp2 = curr_ref_depth_near;
      x = ( temp1 + ( temp2 >> 1 ) ) / temp2;
      sign = ( (temp1 - x * temp2) < 0 ) ? -1 : 1;
      W_factorc = (x<<scale_w);
      W_factorc += ( ((temp1 - x * temp2) << scale_w) + sign * (temp2 >> 1) ) / temp2;

      curr_weight=(short)( ( W_factora*W_factorb*W_factorc + (1<<(scale_w*3-currSlice->luma_log2_weight_denom-1)) ) >> (scale_w*3-currSlice->luma_log2_weight_denom) );
      curr_weight=sClip3(-127,128,curr_weight);

      O_factora=( (curr_depth_near<<(scale_o))+(curr_ref_depth_far>>1) ) / curr_ref_depth_far;

      temp1 = curr_depth_far-curr_ref_depth_far;
      temp2 = curr_depth_far-curr_depth_near;
      sign= (temp1 < 0) ? -1 : 1;
      x = ( temp1 + sign * ( temp2 >> 1 ) ) / temp2;
      sign = ( (temp1 - x * temp2) < 0 ) ? -1 : 1;
      O_factorb = (x<<scale_o);
      O_factorb += ( ((temp1 - x * temp2) << scale_o) + sign * (temp2 >> 1) ) / temp2;

      curr_offset=(short)( ( O_factora*O_factorb + (1<<(scale_o*2-8-1)) ) >> (scale_o*2-8) );
      curr_offset=sClip3(-127,128,curr_offset);

      weight[clist][n][0]=curr_weight;
      offset[clist][n][0]=curr_offset;
    }
  }
}
#endif

static void fill_wp_params(Slice *currSlice)
{
  if (currSlice->slice_type == B_SLICE)
  {
    int i, j, k;
    int comp;
    int log_weight_denom;
    int tb, td;  
    int tx,DistScaleFactor;

    int max_l0_ref = currSlice->num_ref_idx_active[LIST_0];
    int max_l1_ref = currSlice->num_ref_idx_active[LIST_1];
#if EXT3D
    short weight[6][MAX_REFERENCE_PICTURES][3];
    short offset[6][MAX_REFERENCE_PICTURES][3];
    if(currSlice->depth_range_wp)
    { 
      short default_weight[3];
      assert(currSlice->active_pps->weighted_bipred_idc==0);
      assert(currSlice->is_depth);
      currSlice->luma_log2_weight_denom = 5;
      currSlice->chroma_log2_weight_denom = 5;
      currSlice->wp_round_luma   = 16;
      currSlice->wp_round_chroma = 16;
      default_weight[0]       = 1 << currSlice->luma_log2_weight_denom;
      default_weight[1]       = default_weight[2] = 1 << currSlice->chroma_log2_weight_denom;
      get_drb_wp_params(currSlice,default_weight,weight,offset);

      for (i=0; i<MAX_REFERENCE_PICTURES; ++i)
      {
        for (comp=0; comp<3; ++comp)
        {
          log_weight_denom = (comp == 0) ? currSlice->luma_log2_weight_denom : currSlice->chroma_log2_weight_denom;
          currSlice->wp_weight[0][i][comp] =weight[0][i][comp];
          currSlice->wp_weight[1][i][comp] =weight[1][i][comp];
          currSlice->wp_offset[0][i][comp] = offset[0][i][comp];
          currSlice->wp_offset[1][i][comp] = offset[1][i][comp];
        }
      }
    }
#endif


    if (currSlice->active_pps->weighted_bipred_idc == 2)
    {
      currSlice->luma_log2_weight_denom = 5;
      currSlice->chroma_log2_weight_denom = 5;
      currSlice->wp_round_luma   = 16;
      currSlice->wp_round_chroma = 16;

      for (i=0; i<MAX_REFERENCE_PICTURES; ++i)
      {
        for (comp=0; comp<3; ++comp)
        {
          log_weight_denom = (comp == 0) ? currSlice->luma_log2_weight_denom : currSlice->chroma_log2_weight_denom;
          currSlice->wp_weight[0][i][comp] = 1 << log_weight_denom;
          currSlice->wp_weight[1][i][comp] = 1 << log_weight_denom;
          currSlice->wp_offset[0][i][comp] = 0;
          currSlice->wp_offset[1][i][comp] = 0;
        }
      }
    }

    for (i=0; i<max_l0_ref; ++i)
    {
      for (j=0; j<max_l1_ref; ++j)
      {
        for (comp = 0; comp<3; ++comp)
        {
          log_weight_denom = (comp == 0) ? currSlice->luma_log2_weight_denom : currSlice->chroma_log2_weight_denom;
#if EXT3D
          if((currSlice->active_pps->weighted_bipred_idc == 1)||((currSlice->is_depth&&currSlice->depth_range_wp)&&(0==currSlice->active_pps->weighted_bipred_idc)))
#else
          if (currSlice->active_pps->weighted_bipred_idc == 1)
#endif
          {
            currSlice->wbp_weight[0][i][j][comp] =  currSlice->wp_weight[0][i][comp];
            currSlice->wbp_weight[1][i][j][comp] =  currSlice->wp_weight[1][j][comp];
          }
          else if (currSlice->active_pps->weighted_bipred_idc == 2)
          {
            td = iClip3(-128,127,currSlice->listX[LIST_1][j]->poc - currSlice->listX[LIST_0][i]->poc);
            if (td == 0 || currSlice->listX[LIST_1][j]->is_long_term || currSlice->listX[LIST_0][i]->is_long_term)
            {
              currSlice->wbp_weight[0][i][j][comp] = 32;
              currSlice->wbp_weight[1][i][j][comp] = 32;
            }
            else
            {
              tb = iClip3(-128,127,currSlice->ThisPOC - currSlice->listX[LIST_0][i]->poc);

              tx = (16384 + iabs(td/2))/td;
              DistScaleFactor = iClip3(-1024, 1023, (tx*tb + 32 )>>6);

              currSlice->wbp_weight[1][i][j][comp] = DistScaleFactor >> 2;
              currSlice->wbp_weight[0][i][j][comp] = 64 - currSlice->wbp_weight[1][i][j][comp];
              if (currSlice->wbp_weight[1][i][j][comp] < -64 || currSlice->wbp_weight[1][i][j][comp] > 128)
              {
                currSlice->wbp_weight[0][i][j][comp] = 32;
                currSlice->wbp_weight[1][i][j][comp] = 32;
                currSlice->wp_offset[0][i][comp] = 0;
                currSlice->wp_offset[1][j][comp] = 0;
              }
            }
          }

        }
      }
    }

    if (currSlice->mb_aff_frame_flag)
    {
      for (i=0; i<2*max_l0_ref; ++i)
      {
        for (j=0; j<2*max_l1_ref; ++j)
        {
          for (comp = 0; comp<3; ++comp)
          {
            for (k=2; k<6; k+=2)
            {
              currSlice->wp_offset[k+0][i][comp] = currSlice->wp_offset[0][i>>1][comp];
              currSlice->wp_offset[k+1][j][comp] = currSlice->wp_offset[1][j>>1][comp];

              log_weight_denom = (comp == 0) ? currSlice->luma_log2_weight_denom : currSlice->chroma_log2_weight_denom;
#if EXT3D
              if ((currSlice->active_pps->weighted_bipred_idc == 1)||
                ((currSlice->active_pps->weighted_bipred_idc==0)&&(currSlice->depth_range_wp&&currSlice->is_depth)))
#else
              if (currSlice->active_pps->weighted_bipred_idc == 1)
#endif
              {
                currSlice->wbp_weight[k+0][i][j][comp] =  currSlice->wp_weight[0][i>>1][comp];
                currSlice->wbp_weight[k+1][i][j][comp] =  currSlice->wp_weight[1][j>>1][comp];
              }
              else if (currSlice->active_pps->weighted_bipred_idc == 2)
              {
                td = iClip3(-128, 127, currSlice->listX[k+LIST_1][j]->poc - currSlice->listX[k+LIST_0][i]->poc);
                if (td == 0 || currSlice->listX[k+LIST_1][j]->is_long_term || currSlice->listX[k+LIST_0][i]->is_long_term)
                {
                  currSlice->wbp_weight[k+0][i][j][comp] =   32;
                  currSlice->wbp_weight[k+1][i][j][comp] =   32;
                }
                else
                {
                  tb = iClip3(-128,127,((k==2)?currSlice->toppoc:currSlice->bottompoc) - currSlice->listX[k+LIST_0][i]->poc);

                  tx = (16384 + iabs(td/2))/td;
                  DistScaleFactor = iClip3(-1024, 1023, (tx*tb + 32 )>>6);

                  currSlice->wbp_weight[k+1][i][j][comp] = DistScaleFactor >> 2;
                  currSlice->wbp_weight[k+0][i][j][comp] = 64 - currSlice->wbp_weight[k+1][i][j][comp];
                  if (currSlice->wbp_weight[k+1][i][j][comp] < -64 || currSlice->wbp_weight[k+1][i][j][comp] > 128)
                  {
                    currSlice->wbp_weight[k+1][i][j][comp] = 32;
                    currSlice->wbp_weight[k+0][i][j][comp] = 32;
                    currSlice->wp_offset[k+0][i][comp] = 0;
                    currSlice->wp_offset[k+1][j][comp] = 0;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

#if EXT3D
  if((currSlice->slice_type==P_SLICE)&&(currSlice->depth_range_wp))
  {
    
    int i;
    int comp;
    int max_l0_ref = currSlice->num_ref_idx_active[LIST_0];
    short weight[6][MAX_REFERENCE_PICTURES][3];
    short offset[6][MAX_REFERENCE_PICTURES][3];

    assert(0==currSlice->weighted_pred_flag);
    assert(currSlice->is_depth);

    if(currSlice->depth_range_wp)
    { 
      short default_weight[3];
      assert(currSlice->active_pps->weighted_bipred_idc==0);
      currSlice->luma_log2_weight_denom = 5;
      currSlice->chroma_log2_weight_denom = 5;
      currSlice->wp_round_luma   = 16;
      currSlice->wp_round_chroma = 16;
      default_weight[0]       = 1 << currSlice->luma_log2_weight_denom;
      default_weight[1]       = default_weight[2] = 1 << currSlice->chroma_log2_weight_denom;
      get_drb_wp_params(currSlice,default_weight,weight,offset);

      for (i=0; i<max_l0_ref; ++i)
      {
        for (comp=0; comp<3; ++comp)
        {
          currSlice->wp_weight[0][i][comp] =weight[0][i][comp];
          currSlice->wp_offset[0][i][comp] = offset[0][i][comp];
        }
      }
    }
  }
#endif
}



static void init_picture_decoding(VideoParameters *p_Vid)
{
  Slice *pSlice = p_Vid->ppSliceList[0];
  int j, i, iSliceNo, iDeblockMode=1;
  Macroblock *pMBData;
#if EXT3D
  int is_depth=pSlice->is_depth;
  int voidx=GetVOIdx(p_Vid,pSlice->view_id);

  reset_picture_size(p_Vid,pSlice);
#endif

  if(p_Vid->iSliceNumOfCurrPic >= MAX_NUM_SLICES)
  {
    error ("Maximum number of supported slices exceeded. \nPlease recompile with increased value for MAX_NUM_SLICES", 200);
  }

  if(p_Vid->pNextPPS->Valid && (int) p_Vid->pNextPPS->pic_parameter_set_id == pSlice->pic_parameter_set_id)
  {
    pic_parameter_set_rbsp_t tmpPPS;
    memcpy(&tmpPPS, p_Vid->active_pps, sizeof (pic_parameter_set_rbsp_t));
    p_Vid->active_pps->slice_group_id = NULL;
    MakePPSavailable (p_Vid, p_Vid->pNextPPS->pic_parameter_set_id, p_Vid->pNextPPS);
    memcpy(p_Vid->pNextPPS, &tmpPPS, sizeof (pic_parameter_set_rbsp_t));
    tmpPPS.slice_group_id = NULL;
  }

  UseParameterSet (pSlice);
  if(pSlice->idr_flag)
    p_Vid->number=0;

#if EXT3D
  get_dual_picture(p_Vid,pSlice);

  // here we don't use the function "viewcomporder" to determine whether texture-first or not
  //because we don't transmit "texture_voidx_delta"  anymore
  p_Vid->isTextureFirst=p_Vid->p_dual_picture==NULL ? 1 : 0 ;

  if((!pSlice->is_depth)&&(0==voidx))
  {
    for(i=0;i<MAX_CODEVIEW;++i)
    {
      p_Vid->vs_ok[0][i]=p_Vid->vs_ok[1][i]=0;
      p_Vid->decode_ok[0][i]=p_Vid->decode_ok[1][i]=0;

    }
  }

  if(voidx&&(!pSlice->is_depth))
  {
    //!<we calculate the disparity table for texture pictures at non-base view
    if(p_Vid->max_depth_map_value && p_Vid->active_sps->profile_idc==ThreeDV_EXTEND_HIGH)
      get_disparity_table(p_Vid);
  }
  p_Vid->dec_picture->depth_near     = p_Vid->dec_depth_near[voidx];
  p_Vid->dec_picture->depth_far      = p_Vid->dec_depth_far[voidx];
#endif

  p_Vid->PicHeightInMbs = p_Vid->FrameHeightInMbs / ( 1 + pSlice->field_pic_flag );
  p_Vid->PicSizeInMbs   = p_Vid->PicWidthInMbs * p_Vid->PicHeightInMbs;
  p_Vid->FrameSizeInMbs = p_Vid->PicWidthInMbs * p_Vid->FrameHeightInMbs;
  p_Vid->structure = pSlice->structure;

  fmo_init (p_Vid, pSlice);

#if EXT3D
  update_ref_list(p_Vid->p_Dpb[is_depth], pSlice->view_id);
  update_ltref_list(p_Vid->p_Dpb[is_depth], pSlice->view_id);
  update_pic_num(pSlice);
  i = pSlice->view_id;
#else
#if (MVC_EXTENSION_ENABLE)
  update_ref_list(p_Vid->p_Dpb, pSlice->view_id);
  update_ltref_list(p_Vid->p_Dpb, pSlice->view_id);
  update_pic_num(pSlice);
  i = pSlice->view_id;
#else
  update_pic_num(pSlice);
  i = 0;
#endif
#endif
  init_Deblock(p_Vid, pSlice->mb_aff_frame_flag);
  //init mb_data;

  for(j=0; j<p_Vid->iSliceNumOfCurrPic; j++)
  {
    if(p_Vid->ppSliceList[j]->DFDisableIdc == 0)
      iDeblockMode=0;
#if MVC_EXTENSION_ENABLE||EXT3D
    assert(p_Vid->ppSliceList[j]->view_id == i);
#endif
  }
  p_Vid->iDeblockMode = iDeblockMode;

  if(p_Vid->iDeblockMode == 1)
  {
    for(j=0; j<p_Vid->iSliceNumOfCurrPic; j++)
    {
      pSlice = p_Vid->ppSliceList[j];
      iSliceNo = pSlice->current_slice_nr;
      if((p_Vid->separate_colour_plane_flag != 0))
        pMBData = p_Vid->mb_data_JV[pSlice->colour_plane_id];
      else
        pMBData = p_Vid->mb_data;
      for(i = pSlice->start_mb_nr * (1 + pSlice->mb_aff_frame_flag); i < pSlice->end_mb_nr_plus1 * (1 + pSlice->mb_aff_frame_flag); i++)
        pMBData[i].slice_nr = (short) iSliceNo;
    }
  }
}

void init_slice(VideoParameters *p_Vid, Slice *currSlice)
{
  int i;
  p_Vid->active_sps = currSlice->active_sps;
  p_Vid->active_pps = currSlice->active_pps;
#if EXT3D
  if(currSlice->depth_based_mvp_flag || currSlice->depth_range_wp || currSlice->Harmonize_VSP_IVP)
    p_Vid->active_dps = currSlice->active_dps;
#endif


#if MVC_EXTENSION_ENABLE||EXT3D
  //update_ref_list(p_Vid->p_Dpb, currSlice->view_id);
  //update_ltref_list(p_Vid->p_Dpb, currSlice->view_id);
  //update_pic_num(currSlice);

  currSlice->init_lists(currSlice);

  if (currSlice->svc_extension_flag == 0 || currSlice->svc_extension_flag == 1)
    reorder_lists_mvc (currSlice, currSlice->ThisPOC);
  else
    reorder_lists (currSlice);

  if (currSlice->fs_listinterview0)
  {
    free(currSlice->fs_listinterview0);
    currSlice->fs_listinterview0 = NULL;
  }
  if (currSlice->fs_listinterview1)
  {
    free(currSlice->fs_listinterview1);
    currSlice->fs_listinterview1 = NULL;
  }
#else
  //update_pic_num(currSlice);
  currSlice->init_lists (currSlice);
  reorder_lists (currSlice);
#endif

#if EXT3D
  {
    int list, numlist=0;
    currSlice->hasInterviewRef = 0;

    if( currSlice->slice_type==P_SLICE || currSlice->slice_type==SP_SLICE )
    {
      numlist = 1;
    }
    else if( currSlice->slice_type==B_SLICE )
    {
      numlist = 2;
    }

    for( list = 0; list < numlist; list++)
    {
      for( i = 0; i < currSlice->listXsize[list]; i++ )
      {
        if( ( currSlice->framepoc == currSlice->listX[list][i]->frame_poc ) && ( currSlice->view_id != currSlice->listX[list][i]->view_id ) )
        {
          currSlice->hasInterviewRef=1;
          break;
        }
      }
    }
  }

  {
    int currPoc    = currSlice->framepoc; // ThisPOC?
    int currViewId = currSlice->view_id;
    int currLayer  = currSlice->is_depth;
    int listSize;
    StorablePicture* pElem;
    int iRefIdxTest;
    currSlice->bVspRefExist = FALSE;
    currSlice->idxVspRef[0] = currSlice->idxVspRef[1] = -1;
    if (currSlice->slice_type != I_SLICE && currSlice->slice_type != SI_SLICE )
    {
      listSize = currSlice->listXsize[0]; 
      for(iRefIdxTest = 0; iRefIdxTest < listSize; iRefIdxTest++ ) 
      {
        pElem = currSlice->listX[0][iRefIdxTest];
        if ((p_Vid->profile_idc==ThreeDV_EXTEND_HIGH) && (pElem->frame_poc == currPoc && pElem->is_depth == currLayer && pElem->view_id != currViewId && currLayer==0))
        {
          currSlice->bVspRefExist = TRUE;
          currSlice->idxVspRef[0] = iRefIdxTest;
          break;
        }
      }
      if (currSlice->slice_type == B_SLICE)
      {
        listSize = currSlice->listXsize[1]; 
        for( iRefIdxTest = 0; iRefIdxTest < listSize; iRefIdxTest++ ) 
        {
          pElem = currSlice->listX[1][iRefIdxTest];
          if ((p_Vid->profile_idc==ThreeDV_EXTEND_HIGH)&& (pElem->frame_poc == currPoc && pElem->is_depth == currLayer && pElem->view_id != currViewId && currLayer==0))
          {
            currSlice->bVspRefExist = TRUE;
            currSlice->idxVspRef[1] = iRefIdxTest;
            break;
          }
        }
      }
    }

    if (currSlice->is_depth==0)
    {
      if (currSlice->Harmonize_VSP_IVP==0) 
      {
        currSlice->bVspRefExist=FALSE;
      }
    }
  }
#endif

  if (currSlice->structure==FRAME)
  {
    init_mbaff_lists(p_Vid, currSlice);
  }
  //p_Vid->recovery_point = 0;

  // update reference flags and set current p_Vid->ref_flag
  if(!(currSlice->redundant_pic_cnt != 0 && p_Vid->previous_frame_num == currSlice->frame_num))
  {
    for(i=16;i>0;i--)
    {
      currSlice->ref_flag[i] = currSlice->ref_flag[i-1];
    }
  }
  currSlice->ref_flag[0] = currSlice->redundant_pic_cnt==0 ? p_Vid->Is_primary_correct : p_Vid->Is_redundant_correct;
  //p_Vid->previous_frame_num = currSlice->frame_num; //p_Vid->frame_num;

  if((currSlice->active_sps->chroma_format_idc==0)||(currSlice->active_sps->chroma_format_idc==3))
  {
    currSlice->linfo_cbp_intra = linfo_cbp_intra_other;
    currSlice->linfo_cbp_inter = linfo_cbp_inter_other;
  }
  else
  {
    currSlice->linfo_cbp_intra = linfo_cbp_intra_normal;
    currSlice->linfo_cbp_inter = linfo_cbp_inter_normal;
  }

#if EXT3D 
  if (0 == currSlice->is_depth && GetVOIdx(p_Vid, currSlice->view_id) > 0)
  {
    currSlice->AdaptiveLuminanceCompensation = p_Vid->AdaptiveLuminanceCompensation;
    // The ref pic lists are prepared, update the interview ref list
    {
      int currPoc    = currSlice->framepoc;
      int currViewId = currSlice->view_id;
      int currLayer  = currSlice->is_depth;
      int listSize;
      StorablePicture* pElem;
      int iRefIdxTest;
      currSlice->idxInterviewRef[0] = currSlice->idxInterviewRef[1] = -1;
      currSlice->bIVRefExist = FALSE;
      if (currSlice->slice_type == P_SLICE && !currLayer)
      {
        listSize = currSlice->listXsize[0]; 
        for(iRefIdxTest = 0; iRefIdxTest < listSize; iRefIdxTest++ ) 
        {
          pElem = currSlice->listX[0][iRefIdxTest];
          if (pElem->frame_poc == currPoc && !pElem->is_depth && pElem->view_id != currViewId)
          {
            currSlice->bIVRefExist = TRUE;
            currSlice->idxInterviewRef[0] = iRefIdxTest;
            break;
          }
        }
      }
    }
  }
  else
    currSlice->AdaptiveLuminanceCompensation = 0;

  if(currSlice->slice_type == B_SLICE && GetVOIdx(p_Vid, currSlice->view_id) > 0 && p_Vid->RLESkipTex && !currSlice->is_depth)
    currSlice->RLCSkip_enabled = 1;
  else
    currSlice->RLCSkip_enabled = 0;

  if(currSlice->depth_based_mvp_flag || currSlice->Harmonize_VSP_IVP ) 
  {   // camera array - [0]:center / [1]:left / [2]:right for 3 view config.
    int voidx=GetVOIdx(p_Vid,currSlice->view_id);
    currSlice->disparity_coeff=iabs(p_Vid->DepParSet[currSlice->dep_parameter_set_id]->acquisition_info->i_disparity_scale[voidx][0]);

    currSlice->series_7skip_len = 0;
  }
#endif
}

void decode_slice(Slice *currSlice, int current_header)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  int iScale = (1+currSlice->mb_aff_frame_flag);
  if (currSlice->active_pps->entropy_coding_mode_flag)
  {
    init_contexts  (currSlice);
    cabac_new_slice(currSlice);
  }

#if EXT3D
  if ( (((currSlice->active_pps->weighted_bipred_idc > 0)||(currSlice->is_depth&&currSlice->depth_range_wp))  && (currSlice->slice_type == B_SLICE)) 
    || ((currSlice->active_pps->weighted_pred_flag ||(currSlice->is_depth&&currSlice->depth_range_wp))&& currSlice->slice_type !=I_SLICE))
#else
  if ( (currSlice->active_pps->weighted_bipred_idc > 0  && (currSlice->slice_type == B_SLICE)) || (currSlice->active_pps->weighted_pred_flag && currSlice->slice_type !=I_SLICE))
#endif
    fill_wp_params(currSlice);

  //printf("frame picture %d %d %d\n",currSlice->structure,currSlice->ThisPOC,currSlice->direct_spatial_mv_pred_flag);

  // decode main slice information
  if ((current_header == SOP || current_header == SOS) && currSlice->ei_flag == 0)
    decode_one_slice(currSlice);

  // setMB-Nr in case this slice was lost
  // if(currSlice->ei_flag)
  //   p_Vid->current_mb_nr = currSlice->last_mb_nr + 1;

  //deblocking for frame or field
  if(p_Vid->iDeblockMode && (p_Vid->bDeblockEnable & (1<<(p_Vid->dec_picture->used_for_reference))))
  {
    if((p_Vid->separate_colour_plane_flag != 0) )
    {
      change_plane_JV(p_Vid, currSlice->colour_plane_id, currSlice);
      DeblockPicturePartially(p_Vid, currSlice->dec_picture, currSlice->start_mb_nr*iScale, currSlice->end_mb_nr_plus1*iScale);
    }
    else
    {
      DeblockPicturePartially(p_Vid, currSlice->dec_picture, currSlice->start_mb_nr*iScale, currSlice->end_mb_nr_plus1*iScale);
    }
  }
}


/*!
 ************************************************************************
 * \brief
 *    Error tracking: if current frame is lost or any reference frame of
 *                    current frame is lost, current frame is incorrect.
 ************************************************************************
 */
static void Error_tracking(VideoParameters *p_Vid, Slice *currSlice)
{
  int i;

  if(currSlice->redundant_pic_cnt == 0)
  {
    p_Vid->Is_primary_correct = p_Vid->Is_redundant_correct = 1;
  }

  if(currSlice->redundant_pic_cnt == 0 && p_Vid->type != I_SLICE)
  {
    for(i=0;i<currSlice->num_ref_idx_active[LIST_0];++i)
    {
      if(currSlice->ref_flag[i] == 0)  // any reference of primary slice is incorrect
      {
        p_Vid->Is_primary_correct = 0; // primary slice is incorrect
      }
    }
  }
  else if(currSlice->redundant_pic_cnt != 0 && p_Vid->type != I_SLICE)
  {
    if(currSlice->ref_flag[currSlice->redundant_slice_ref_idx] == 0)  // reference of redundant slice is incorrect
    {
      p_Vid->Is_redundant_correct = 0;  // redundant slice is incorrect
    }
  }
}

static void CopyPOC(Slice *pSlice0, Slice *currSlice)
{
  currSlice->framepoc  = pSlice0->framepoc;
  currSlice->toppoc    = pSlice0->toppoc;
  currSlice->bottompoc = pSlice0->bottompoc;  
  currSlice->ThisPOC   = pSlice0->ThisPOC;
}



/*!
 ***********************************************************************
 * \brief
 *    decodes one I- or P-frame
 *
 ***********************************************************************
 */
int decode_one_frame(DecoderParams *pDecoder)
{
  VideoParameters *p_Vid = pDecoder->p_Vid;
  InputParameters *p_Inp = p_Vid->p_Inp;
  int current_header, iRet;
  Slice *currSlice; // = p_Vid->currentSlice;
  Slice **ppSliceList = p_Vid->ppSliceList;
  int iSliceNo;
  
  //read one picture first;
  p_Vid->iSliceNumOfCurrPic=0;
  current_header=0;
  p_Vid->iNumOfSlicesDecoded=0;
  p_Vid->num_dec_mb = 0;

  if(p_Vid->newframe)
  {
    if(p_Vid->pNextPPS->Valid && (int) p_Vid->pNextPPS->pic_parameter_set_id == p_Vid->pNextSlice->pic_parameter_set_id)
    {
      MakePPSavailable (p_Vid, p_Vid->pNextPPS->pic_parameter_set_id, p_Vid->pNextPPS);
      p_Vid->pNextPPS->Valid=0;
    }

    //get the first slice from currentslice;
    assert(ppSliceList[p_Vid->iSliceNumOfCurrPic]);
    currSlice = ppSliceList[p_Vid->iSliceNumOfCurrPic];
    ppSliceList[p_Vid->iSliceNumOfCurrPic] = p_Vid->pNextSlice;

    p_Vid->pNextSlice = currSlice;
    assert(ppSliceList[p_Vid->iSliceNumOfCurrPic]->current_slice_nr == 0);

    currSlice = ppSliceList[p_Vid->iSliceNumOfCurrPic];

    UseParameterSet (currSlice);

    init_picture(p_Vid, currSlice, p_Inp);

    p_Vid->iSliceNumOfCurrPic++;
    current_header = SOS;
  }
  while(current_header != SOP && current_header !=EOS)
  {
    //no pending slices;
    assert(p_Vid->iSliceNumOfCurrPic < p_Vid->iNumOfSlicesAllocated);
    if(!ppSliceList[p_Vid->iSliceNumOfCurrPic])
    {
      ppSliceList[p_Vid->iSliceNumOfCurrPic] = malloc_slice(p_Inp, p_Vid);
    }
    currSlice = ppSliceList[p_Vid->iSliceNumOfCurrPic];

    //p_Vid->currentSlice = currSlice;
    currSlice->p_Vid = p_Vid;
    currSlice->p_Inp = p_Inp;
#if EXT3D
    currSlice->p_Dpb = p_Vid->p_Dpb[0];//!< we have to reset this value according to its corresponding  category (depth or texture)
#else
    currSlice->p_Dpb = p_Vid->p_Dpb;
#endif
    currSlice->next_header = -8888;
    currSlice->num_dec_mb = 0;
    currSlice->coeff_ctr = -1;
    currSlice->pos       =  0;
    currSlice->is_reset_coeff = FALSE;

    current_header = read_new_slice(currSlice);
    //init;
    currSlice->current_header = current_header;

    // error tracking of primary and redundant slices.
    Error_tracking(p_Vid, currSlice);
    // If primary and redundant are received and primary is correct, discard the redundant
    // else, primary slice will be replaced with redundant slice.
    if(currSlice->frame_num == p_Vid->previous_frame_num && currSlice->redundant_pic_cnt !=0
      && p_Vid->Is_primary_correct !=0 && current_header != EOS)
    {
      continue;
    }

    if((current_header != SOP && current_header !=EOS) || (p_Vid->iSliceNumOfCurrPic==0 && current_header == SOP))
    {
       currSlice->current_slice_nr = (short) p_Vid->iSliceNumOfCurrPic;
       p_Vid->dec_picture->max_slice_id = (short) imax(currSlice->current_slice_nr, p_Vid->dec_picture->max_slice_id);
       if(p_Vid->iSliceNumOfCurrPic >0)
       {
         CopyPOC(*ppSliceList, currSlice);
         ppSliceList[p_Vid->iSliceNumOfCurrPic-1]->end_mb_nr_plus1 = currSlice->start_mb_nr;
       }
       p_Vid->iSliceNumOfCurrPic++;

       if(p_Vid->iSliceNumOfCurrPic >= p_Vid->iNumOfSlicesAllocated)
       {
         Slice **tmpSliceList = (Slice **)realloc(p_Vid->ppSliceList, (p_Vid->iNumOfSlicesAllocated+MAX_NUM_DECSLICES)*sizeof(Slice*));
         if(!tmpSliceList)
         {
           tmpSliceList = calloc((p_Vid->iNumOfSlicesAllocated+MAX_NUM_DECSLICES), sizeof(Slice*));
           memcpy(tmpSliceList, p_Vid->ppSliceList, p_Vid->iSliceNumOfCurrPic*sizeof(Slice*));
           //free;
           free(p_Vid->ppSliceList);
           ppSliceList = p_Vid->ppSliceList = tmpSliceList;
         }
         else
         {
           //assert(tmpSliceList == p_Vid->ppSliceList);
           ppSliceList = p_Vid->ppSliceList = tmpSliceList;
           memset(p_Vid->ppSliceList+p_Vid->iSliceNumOfCurrPic, 0, sizeof(Slice*)*MAX_NUM_DECSLICES);
         }
         p_Vid->iNumOfSlicesAllocated += MAX_NUM_DECSLICES;
       }
       current_header = SOS;       
    }
    else
    {
#if EXT3D
      seq_parameter_set_rbsp_t *active_sps = ppSliceList[p_Vid->iSliceNumOfCurrPic-1]->active_sps;
      int FrameSizeInMbs = (active_sps->pic_width_in_mbs_minus1+1)*(active_sps->pic_height_in_map_units_minus1+1);
      if(ppSliceList[p_Vid->iSliceNumOfCurrPic-1]->mb_aff_frame_flag)
       ppSliceList[p_Vid->iSliceNumOfCurrPic-1]->end_mb_nr_plus1 = FrameSizeInMbs/2;
      else
        ppSliceList[p_Vid->iSliceNumOfCurrPic-1]->end_mb_nr_plus1 = FrameSizeInMbs/(1+ppSliceList[p_Vid->iSliceNumOfCurrPic-1]->field_pic_flag);
#else
      if(ppSliceList[p_Vid->iSliceNumOfCurrPic-1]->mb_aff_frame_flag)
       ppSliceList[p_Vid->iSliceNumOfCurrPic-1]->end_mb_nr_plus1 = p_Vid->FrameSizeInMbs/2;
      else
       ppSliceList[p_Vid->iSliceNumOfCurrPic-1]->end_mb_nr_plus1 = p_Vid->FrameSizeInMbs/(1+ppSliceList[p_Vid->iSliceNumOfCurrPic-1]->field_pic_flag);
#endif
       p_Vid->newframe = 1;
       currSlice->current_slice_nr = 0;
       //keep it in currentslice;
       ppSliceList[p_Vid->iSliceNumOfCurrPic] = p_Vid->pNextSlice;
       p_Vid->pNextSlice = currSlice; 
#if EXT3D
       CopySliceInfo(currSlice, p_Vid->old_slice);
#endif
    }
#if !EXT3D
    CopySliceInfo(currSlice, p_Vid->old_slice);
#endif
  }
  iRet = current_header;
    init_picture_decoding(p_Vid);

    {
      for(iSliceNo=0; iSliceNo<p_Vid->iSliceNumOfCurrPic; iSliceNo++)
      {
        currSlice = ppSliceList[iSliceNo];
        current_header = currSlice->current_header;
        //p_Vid->currentSlice = currSlice;

        assert(current_header != EOS);
        assert(currSlice->current_slice_nr == iSliceNo);

        init_slice(p_Vid, currSlice);
        decode_slice(currSlice, current_header);

        p_Vid->iNumOfSlicesDecoded++;
        p_Vid->num_dec_mb += currSlice->num_dec_mb;
        p_Vid->erc_mvperMB += currSlice->erc_mvperMB;
      }
    }
    exit_picture(p_Vid, &p_Vid->dec_picture);
#if EXT3D
  {
    int voidx=GetVOIdx(p_Vid,ppSliceList[0]->view_id);
    p_Vid->decode_ok[ppSliceList[0]->is_depth][voidx]=1;
  }
#endif
  p_Vid->previous_frame_num = ppSliceList[0]->frame_num;
#if EXT3D
  if(p_Vid->RefDispSEIUpdated == 1 && p_Vid->frame_no != p_Vid->RefDispSEIPOC)
  {
    if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
      fprintf(p_Vid->RefDispFile,"for POC %d ####\n\n",p_Vid->frame_no);
    p_Vid->RefDispSEIUpdated = 0;
  }
#endif
  return (iRet);
}

/*!
 ************************************************************************
 * \brief
 *    Convert file read buffer to source picture structure
 * \param imgX
 *    Pointer to image plane
 * \param buf
 *    Buffer for file output
 * \param size_x
 *    horizontal image size in pixel
 * \param size_y
 *    vertical image size in pixel
 * \param symbol_size_in_bytes
 *    number of bytes used per pel
 ************************************************************************
 */
void buffer2img (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes)
{
  int i,j;

  uint16 tmp16, ui16;
  unsigned long  tmp32, ui32;

  if (symbol_size_in_bytes> sizeof(imgpel))
  {
    error ("Source picture has higher bit depth than imgpel data type. \nPlease recompile with larger data type for imgpel.", 500);
  }

#if (IMGTYPE == 0)
  if (sizeof(char) == symbol_size_in_bytes)
  {
    // imgpel == pixel_in_file == 1 byte -> simple copy
    fast_memcpy(&imgX[0][0], buf, size_x * size_y);
  }
  else
#endif
  {
    // sizeof (imgpel) > sizeof(char)
    if (testEndian())
    {
      // big endian
      switch (symbol_size_in_bytes)
      {
      case 1:
        {
          for(j = 0; j < size_y; ++j)

            for(i = 0; i < size_x; ++i)
            {
              imgX[j][i]= buf[i+j*size_x];
            }
          break;
        }
      case 2:
        {
          for(j=0;j<size_y;++j)
            for(i=0;i<size_x;++i)
            {
              memcpy(&tmp16, buf+((i+j*size_x)*2), 2);
              ui16  = (uint16) ((tmp16 >> 8) | ((tmp16&0xFF)<<8));
              imgX[j][i] = (imgpel) ui16;
            }
          break;
        }
      case 4:
        {
          for(j=0;j<size_y;++j)
            for(i=0;i<size_x;++i)
            {
              memcpy(&tmp32, buf+((i+j*size_x)*4), 4);
              ui32  = ((tmp32&0xFF00)<<8) | ((tmp32&0xFF)<<24) | ((tmp32&0xFF0000)>>8) | ((tmp32&0xFF000000)>>24);
              imgX[j][i] = (imgpel) ui32;
            }
        }
      default:
        {
           error ("reading only from formats of 8, 16 or 32 bit allowed on big endian architecture", 500);
           break;
        }
      }

    }
    else
    {
      // little endian
      if (symbol_size_in_bytes == 1)
      {
        for (j=0; j < size_y; ++j)
        {
          for (i=0; i < size_x; ++i)
          {
            imgX[j][i]=*(buf++);
          }
        }
      }
      else
      {
        for (j=0; j < size_y; ++j)
        {
          int jpos = j*size_x;
          for (i=0; i < size_x; ++i)
          {
            imgX[j][i]=0;
            memcpy(&(imgX[j][i]), buf +((i+jpos)*symbol_size_in_bytes), symbol_size_in_bytes);
          }
        }
      }

    }
  }
}


/*!
 ***********************************************************************
 * \brief
 *    compute generic SSE
 ***********************************************************************
 */
int64 compute_SSE(imgpel **imgRef, imgpel **imgSrc, int xRef, int xSrc, int ySize, int xSize)
{
  int i, j;
  imgpel *lineRef, *lineSrc;
  int64 distortion = 0;

  for (j = 0; j < ySize; j++)
  {
    lineRef = &imgRef[j][xRef];    
    lineSrc = &imgSrc[j][xSrc];

    for (i = 0; i < xSize; i++)
      distortion += iabs2( *lineRef++ - *lineSrc++ );
  }
  return distortion;
}

/*!
 ************************************************************************
 * \brief
 *    Calculate the value of frame_no
 ************************************************************************
*/
void calculate_frame_no(VideoParameters *p_Vid, StorablePicture *p)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  // calculate frame number
  int  psnrPOC = p_Vid->active_sps->mb_adaptive_frame_field_flag ? p->poc /(p_Inp->poc_scale) : p->poc/(p_Inp->poc_scale);

  if (psnrPOC==0)// && p_Vid->psnr_number)
  {
    p_Vid->idr_psnr_number = p_Vid->number*p_Vid->ref_poc_gap/(p_Inp->poc_scale);
  }
  p_Vid->psnr_number=imax(p_Vid->psnr_number,p_Vid->idr_psnr_number+psnrPOC);

  p_Vid->frame_no = p_Vid->idr_psnr_number + psnrPOC;
}


/*!
************************************************************************
* \brief
*    Find PSNR for all three components.Compare decoded frame with
*    the original sequence. Read p_Inp->jumpd frames to reflect frame skipping.
* \param p_Vid
*      video encoding parameters for current picture
* \param p
*      picture to be compared
* \param p_ref
*      file pointer piont to reference YUV reference file
************************************************************************
*/
void find_snr(VideoParameters *p_Vid, 
              StorablePicture *p,
              int *p_ref)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
#if EXT3D
  int   voidx=GetVOIdx(p_Vid,p->view_id)  ;
  SNRParameters   *snr   = p_Vid->snr_3dv[p->is_depth][voidx];
#else
  SNRParameters   *snr   = p_Vid->snr;
#endif

  int k;
  int ret;
  int64 diff_comp[3] = {0};
  int64  status;
  int symbol_size_in_bytes = (p_Vid->pic_unit_bitsize_on_disk >> 3);
  int comp_size_x[3], comp_size_y[3];
  int64 framesize_in_bytes;

  unsigned int max_pix_value_sqd[3];

  Boolean rgb_output = (Boolean) (p_Vid->active_sps->vui_seq_parameters.matrix_coefficients==0);
  unsigned char *buf;
  imgpel **cur_ref [3];
  imgpel **cur_comp[3]; 
  // picture error concealment
  char yuv_types[4][6]= {"4:0:0","4:2:0","4:2:2","4:4:4"};

  max_pix_value_sqd[0] = iabs2(p_Vid->max_pel_value_comp[0]);
  max_pix_value_sqd[1] = iabs2(p_Vid->max_pel_value_comp[1]);
  max_pix_value_sqd[2] = iabs2(p_Vid->max_pel_value_comp[2]);

  cur_ref[0]  = p_Vid->imgY_ref;
  cur_ref[1]  = p->chroma_format_idc != YUV400 ? p_Vid->imgUV_ref[0] : NULL;
  cur_ref[2]  = p->chroma_format_idc != YUV400 ? p_Vid->imgUV_ref[1] : NULL;

  cur_comp[0] = p->imgY;
  cur_comp[1] = p->chroma_format_idc != YUV400 ? p->imgUV[0]  : NULL;
  cur_comp[2] =  p->chroma_format_idc!= YUV400 ? p->imgUV[1]  : NULL; 

  comp_size_x[0] = p_Inp->source.width[0];
  comp_size_y[0] = p_Inp->source.height[0];
  comp_size_x[1] = comp_size_x[2] = p_Inp->source.width[1];
  comp_size_y[1] = comp_size_y[2] = p_Inp->source.height[1];

  framesize_in_bytes = (((int64) comp_size_x[0] * comp_size_y[0]) + ((int64) comp_size_x[1] * comp_size_y[1] ) * 2) * symbol_size_in_bytes;

  // KS: this buffer should actually be allocated only once, but this is still much faster than the previous version
  buf = malloc ( comp_size_x[0] * comp_size_y[0] * symbol_size_in_bytes );

  if (NULL == buf)
  {
    no_mem_exit("find_snr: buf");
  }

  status = lseek (*p_ref, framesize_in_bytes * p_Vid->frame_no, SEEK_SET);
  if (status == -1)
  {
    fprintf(stderr, "Warning: Could not seek to frame number %d in reference file. Shown PSNR might be wrong.\n", p_Vid->frame_no);
    free (buf);
    return;
  }

  if(rgb_output)
    lseek (*p_ref, framesize_in_bytes/3, SEEK_CUR);

  for (k = 0; k < ((p->chroma_format_idc != YUV400) ? 3 : 1); ++k)
  {

    if(rgb_output && k == 2)
      lseek (*p_ref, -framesize_in_bytes, SEEK_CUR);

    ret = read(*p_ref, buf, comp_size_x[k] * comp_size_y[k] * symbol_size_in_bytes);
    if (ret != comp_size_x[k] * comp_size_y[k] * symbol_size_in_bytes)
    {
      printf ("Warning: could not read from reconstructed file\n");
      memset (buf, 0, comp_size_x[k] * comp_size_y[k] * symbol_size_in_bytes);
      close(*p_ref);
      *p_ref = -1;
      break;
    }
    buffer2img(cur_ref[k], buf, comp_size_x[k], comp_size_y[k], symbol_size_in_bytes);

    // Compute SSE
    diff_comp[k] = compute_SSE(cur_ref[k], cur_comp[k], 0, 0, comp_size_y[k], comp_size_x[k]);

    // Collecting SNR statistics
    snr->snr[k] = psnr( max_pix_value_sqd[k], comp_size_x[k] * comp_size_y[k], (float) diff_comp[k]);   

    if (snr->frame_ctr == 0) // first
    {
      snr->snra[k] = snr->snr[k];                                                        // keep luma snr for first frame
    }
    else
    {
      snr->snra[k] = (float)(snr->snra[k]*(snr->frame_ctr)+snr->snr[k])/(snr->frame_ctr + 1); // average snr chroma for all frames
    }
  }

  if(rgb_output)
    lseek (*p_ref, framesize_in_bytes * 2 / 3, SEEK_CUR);


  free (buf);

  // picture error concealment
  if(p->concealed_pic)
  {
    fprintf(stdout,"%04d(P)  %8d %5d %5d %7.4f %7.4f %7.4f  %s %5d\n",
      p_Vid->frame_no, p->frame_poc, p->pic_num, p->qp,
      snr->snr[0], snr->snr[1], snr->snr[2], yuv_types[p->chroma_format_idc], 0);
  }
}


void reorder_lists(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;

  if ((currSlice->slice_type != I_SLICE)&&(currSlice->slice_type != SI_SLICE))
  {
    if (currSlice->ref_pic_list_reordering_flag[LIST_0])
    {
      reorder_ref_pic_list(currSlice, LIST_0);
    }
#if EXT3D
    if (p_Vid->no_reference_picture[currSlice->is_depth] == currSlice->listX[0][currSlice->num_ref_idx_active[LIST_0] - 1])
#else
    if (p_Vid->no_reference_picture == currSlice->listX[0][currSlice->num_ref_idx_active[LIST_0] - 1])
#endif
    {
      if (p_Vid->non_conforming_stream)
        printf("RefPicList0[ num_ref_idx_l0_active_minus1 ] is equal to 'no reference picture'\n");
      else
        error("RefPicList0[ num_ref_idx_l0_active_minus1 ] is equal to 'no reference picture', invalid bitstream",500);
    }
    // that's a definition
    currSlice->listXsize[0] = (char) currSlice->num_ref_idx_active[LIST_0];
  }
  if (currSlice->slice_type == B_SLICE)
  {
    if (currSlice->ref_pic_list_reordering_flag[LIST_1])
    {
      reorder_ref_pic_list(currSlice, LIST_1);
    }
#if EXT3D
    if (p_Vid->no_reference_picture[currSlice->is_depth] == currSlice->listX[1][currSlice->num_ref_idx_active[LIST_1]-1])
#else
    if (p_Vid->no_reference_picture == currSlice->listX[1][currSlice->num_ref_idx_active[LIST_1]-1])
#endif
    {
      if (p_Vid->non_conforming_stream)
        printf("RefPicList1[ num_ref_idx_l1_active_minus1 ] is equal to 'no reference picture'\n");
      else
        error("RefPicList1[ num_ref_idx_l1_active_minus1 ] is equal to 'no reference picture', invalid bitstream",500);
    }
    // that's a definition
    currSlice->listXsize[1] = (char) currSlice->num_ref_idx_active[LIST_1];
  }

  free_ref_pic_list_reordering_buffer(currSlice);

  if ( currSlice->slice_type == P_SLICE )
  {
#if PRINTREFLIST
    unsigned int i;
#if MVC_EXTENSION_ENABLE||EXT3D
    // print out for debug purpose
#if EXT3D
    if( currSlice->current_slice_nr==0)
#else
    if((p_Vid->profile_idc == MVC_HIGH || p_Vid->profile_idc == STEREO_HIGH) && currSlice->current_slice_nr==0)
#endif
    {
      if(currSlice->listXsize[0]>0)
      {
        printf("\n");
        printf(" ** (CurViewID:%d) %s Ref Pic List 0 ****\n", currSlice->view_id, currSlice->structure==FRAME ? "FRM":(currSlice->structure==TOP_FIELD ? "TOP":"BOT"));
        for(i=0; i<(unsigned int)(currSlice->listXsize[0]); i++)  //ref list 0
        {
          printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d\n", i, currSlice->listX[0][i]->poc, currSlice->listX[0][i]->pic_num, currSlice->listX[0][i]->view_id);
        }
      }
    }
#endif
#endif
  }
  else if ( currSlice->slice_type == B_SLICE )
  {
#if PRINTREFLIST
    unsigned int i;
#if MVC_EXTENSION_ENABLE||EXT3D
    // print out for debug purpose
#if EXT3D
    if(currSlice->current_slice_nr==0)
#else
    if((p_Vid->profile_idc == MVC_HIGH || p_Vid->profile_idc == STEREO_HIGH) && currSlice->current_slice_nr==0)
#endif
    {
      if((currSlice->listXsize[0]>0) || (currSlice->listXsize[1]>0))
        printf("\n");
      if(currSlice->listXsize[0]>0)
      {
        printf(" ** (CurViewID:%d) %s Ref Pic List 0 ****\n", currSlice->view_id, currSlice->structure==FRAME ? "FRM":(currSlice->structure==TOP_FIELD ? "TOP":"BOT"));
        for(i=0; i<(unsigned int)(currSlice->listXsize[0]); i++)  //ref list 0
        {
          printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d\n", i, currSlice->listX[0][i]->poc, currSlice->listX[0][i]->pic_num, currSlice->listX[0][i]->view_id);
        }
      }
      if(currSlice->listXsize[1]>0)
      {
        printf(" ** (CurViewID:%d) %s Ref Pic List 1 ****\n", currSlice->view_id, currSlice->structure==FRAME ? "FRM":(currSlice->structure==TOP_FIELD ? "TOP":"BOT"));
        for(i=0; i<(unsigned int)(currSlice->listXsize[1]); i++)  //ref list 1
        {
          printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d\n", i, currSlice->listX[1][i]->poc, currSlice->listX[1][i]->pic_num, currSlice->listX[1][i]->view_id);
        }
      }
    }
#endif

#endif
  }
}




/*!
 ************************************************************************
 * \brief
 *    Reads new slice from bit_stream
 ************************************************************************
 */
int read_new_slice(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;

  NALU_t *nalu = p_Vid->nalu; 
  int current_header = 0;
  int BitsUsedByHeader;
  Bitstream *currStream = NULL;

  int slice_id_a, slice_id_b, slice_id_c;

  for (;;)
  {
#if EXT3D
    currSlice->svc_extension_flag = -1;
    currSlice->is_depth=0;
#elif MVC_EXTENSION_ENABLE
    currSlice->svc_extension_flag = -1;
#endif
    if (0 == read_next_nalu(p_Vid, nalu))
      return EOS;

#if MVC_EXTENSION_ENABLE||EXT3D

#if EXT3D
    if(p_Inp->DecodeAllLayers == 1 && (nalu->nal_unit_type == NALU_TYPE_PREFIX || nalu->nal_unit_type == NALU_TYPE_SLC_EXT || nalu->nal_unit_type == NALU_TYPE_3DV_EXT))
#else
    if(p_Inp->DecodeAllLayers == 1 && (nalu->nal_unit_type == NALU_TYPE_PREFIX || nalu->nal_unit_type == NALU_TYPE_SLC_EXT))
#endif
    {
      currStream = currSlice->partArr[0].bitstream;
      currStream->ei_flag = 0;
      currStream->frame_bitoffset = currStream->read_len = 0;
      fast_memcpy (currStream->streamBuffer, &nalu->buf[1], nalu->len-1);
      currStream->code_len = currStream->bitstream_length = RBSPtoSODB(currStream->streamBuffer, nalu->len-1);

      currSlice->svc_extension_flag = u_1 ("svc_extension_flag"       , currStream);
#if EXT3D
      if(currSlice->svc_extension_flag&&(nalu->nal_unit_type == NALU_TYPE_PREFIX||nalu->nal_unit_type == NALU_TYPE_SLC_EXT))
#else
      if(currSlice->svc_extension_flag)
#endif
      {
        nal_unit_header_svc_extension();
      }
#if EXT3D
    else if(currSlice->svc_extension_flag&&nalu->nal_unit_type == NALU_TYPE_3DV_EXT)
    {
      nal_unit_header_3dvc_extension(&currSlice->NaluHeader3dVCExt, currStream);
    }
#endif
      else
      {
        nal_unit_header_mvc_extension(&currSlice->NaluHeaderMVCExt, currStream);
        currSlice->NaluHeaderMVCExt.iPrefixNALU = (nalu->nal_unit_type == NALU_TYPE_PREFIX);
      }

      if(nalu->nal_unit_type == NALU_TYPE_SLC_EXT)
      {        
        if(currSlice->svc_extension_flag)
        {
          //to be implemented for Annex G;
        }
        else 
        {
          nalu->nal_unit_type = currSlice->NaluHeaderMVCExt.non_idr_flag==0? NALU_TYPE_IDR: NALU_TYPE_SLICE;
        }
      }
#if EXT3D
    currSlice->is_depth=(nalu->nal_unit_type!=NALU_TYPE_3DV_EXT)?0:(currSlice->svc_extension_flag?currSlice->NaluHeader3dVCExt.depth_flag:1);
#endif
    }
#endif

process_nalu:
    switch (nalu->nal_unit_type)
    {
    case NALU_TYPE_SLICE:
    case NALU_TYPE_IDR:
#if EXT3D
    case NALU_TYPE_3DV_EXT:

      if((nalu->nal_unit_type==NALU_TYPE_3DV_EXT)&&(p_Inp->DecodeAllLayers == 0 ))
      {
        if(p_Inp->silent == FALSE)
          printf ("Found 3DV extension NALU (%d). Ignoring.\n", (int) nalu->nal_unit_type);
        break;
      }
      currSlice->nal_unit_type=nalu->nal_unit_type;
      currSlice->p_Dpb=p_Vid->p_Dpb[currSlice->is_depth];
#endif

      if (p_Vid->recovery_point || nalu->nal_unit_type == NALU_TYPE_IDR)
      {
        if (p_Vid->recovery_point_found == 0)
        {
          if (nalu->nal_unit_type != NALU_TYPE_IDR)
          {
            printf("Warning: Decoding does not start with an IDR picture.\n");
            p_Vid->non_conforming_stream = 1;
          }
          else
            p_Vid->non_conforming_stream = 0;
        }
        p_Vid->recovery_point_found = 1;
      }


      if (p_Vid->recovery_point_found == 0)
        break;

#if EXT3D
    if(currSlice->svc_extension_flag == 1)
      currSlice->idr_flag=(currSlice->NaluHeader3dVCExt.non_idr_flag==0);
    else if(currSlice->svc_extension_flag == 0)
        currSlice->idr_flag=(currSlice->NaluHeaderMVCExt.non_idr_flag==0);
    else
        currSlice->idr_flag=(nalu->nal_unit_type == NALU_TYPE_IDR);
#else
      currSlice->idr_flag = (nalu->nal_unit_type == NALU_TYPE_IDR);
#endif

      currSlice->nal_reference_idc = nalu->nal_reference_idc;
      currSlice->dp_mode = PAR_DP_1;
      currSlice->max_part_nr = 1;

#if EXT3D
      if (currSlice->svc_extension_flag ==-1)
      {
        //
        currStream = currSlice->partArr[0].bitstream;
        currStream->ei_flag = 0;
        currStream->frame_bitoffset = currStream->read_len = 0;
        fast_memcpy (currStream->streamBuffer, &nalu->buf[1], nalu->len-1);
        currStream->code_len = currStream->bitstream_length = RBSPtoSODB(currStream->streamBuffer, nalu->len-1);
      }
#elif MVC_EXTENSION_ENABLE
      if (currSlice->svc_extension_flag != 0)
      {
        currStream = currSlice->partArr[0].bitstream;
        currStream->ei_flag = 0;
        currStream->frame_bitoffset = currStream->read_len = 0;
        fast_memcpy (currStream->streamBuffer, &nalu->buf[1], nalu->len-1);
        currStream->code_len = currStream->bitstream_length = RBSPtoSODB(currStream->streamBuffer, nalu->len-1);
      }
#else   
      currStream = currSlice->partArr[0].bitstream;
      currStream->ei_flag = 0;
      currStream->frame_bitoffset = currStream->read_len = 0;
      memcpy (currStream->streamBuffer, &nalu->buf[1], nalu->len-1);
      currStream->code_len = currStream->bitstream_length = RBSPtoSODB(currStream->streamBuffer, nalu->len-1);
#endif

#if EXT3D
      // Some syntax of the Slice Header depends on the parameter set, which depends on
      // the parameter set ID of the SLice header.  Hence, read the pic_parameter_set_id
      // of the slice header first, then setup the active parameter sets, and then read
      // the rest of the slice header
      if(p_Vid->SliceHeaderPred)
      {
        //check whether this slice header is predicted or not
        int voidx=GetVOIdx(p_Vid, (currSlice->svc_extension_flag == 1) ? currSlice->NaluHeader3dVCExt.view_id : currSlice->NaluHeaderMVCExt.view_id) ;
#if FIX_SLICE_HEAD_PRED
        if(!(voidx==0 && !currSlice->is_depth))
#else
        if( p_Vid->dec_view_flag[voidx][1-currSlice->is_depth] && (!(voidx==0 && !currSlice->is_depth)) )
#endif
          currSlice->slice_header_pred_flag = 1;
        else
          currSlice->slice_header_pred_flag = 0;
      }
      else
        currSlice->slice_header_pred_flag = 0;
#endif
      BitsUsedByHeader = FirstPartOfSliceHeader(currSlice);
      UseParameterSet (currSlice);

      currSlice->active_sps = p_Vid->active_sps;
      currSlice->active_pps = p_Vid->active_pps;
      currSlice->Transform8x8Mode = p_Vid->active_pps->transform_8x8_mode_flag;
      currSlice->is_not_independent = (p_Vid->active_sps->chroma_format_idc==YUV444)&&((p_Vid->separate_colour_plane_flag == 0));
      BitsUsedByHeader += RestOfSliceHeader (currSlice);

#if EXT3D
      UseDepParameterSet (currSlice);
      if(currSlice->depth_based_mvp_flag || currSlice->Harmonize_VSP_IVP || currSlice->depth_range_wp)
        currSlice->active_dps = p_Vid->active_dps;

      if(p_Vid->SliceHeaderPred)
      {
          int voidx=GetVOIdx(p_Vid, (currSlice->svc_extension_flag == 1) ? currSlice->NaluHeader3dVCExt.view_id : currSlice->NaluHeaderMVCExt.view_id) ;
          p_Vid->dec_view_flag[voidx][currSlice->is_depth] = 1;
          if(!voidx && !currSlice->is_depth)
          {
            int i;
            p_Vid->dec_view_flag[0][1] = 0; 

            p_Vid->slice_header_dual[0][0].dec_ref_pic_marking_buffer = NULL;
            p_Vid->slice_header_dual[0][1].dec_ref_pic_marking_buffer = NULL;

            for(i=1; i< p_Vid->num_of_views; i++)
            {
              p_Vid->dec_view_flag[i][0] = 0;
              p_Vid->dec_view_flag[i][1] = 0;

              p_Vid->slice_header_dual[i][0].dec_ref_pic_marking_buffer = NULL;
              p_Vid->slice_header_dual[i][1].dec_ref_pic_marking_buffer = NULL;
            }
          }
      }

      if((currSlice->svc_extension_flag == 0)||(nalu->nal_unit_type==NALU_TYPE_3DV_EXT))
      {
        //!<MVC texture or 3DV texture and depth
        if(currSlice->svc_extension_flag)
        {
          currSlice->view_id = currSlice->NaluHeader3dVCExt.view_id;
          currSlice->inter_view_flag = currSlice->NaluHeader3dVCExt.inter_view_flag;
          currSlice->anchor_pic_flag = currSlice->NaluHeader3dVCExt.anchor_pic_flag;
          currSlice->temporal_id=currSlice->NaluHeader3dVCExt.temporal_id;
        }
        else
        {
          currSlice->view_id = currSlice->NaluHeaderMVCExt.view_id;
          currSlice->inter_view_flag = currSlice->NaluHeaderMVCExt.inter_view_flag;
          currSlice->anchor_pic_flag = currSlice->NaluHeaderMVCExt.anchor_pic_flag;
          currSlice->temporal_id=currSlice->NaluHeaderMVCExt.temporal_id;
        }
      }
      else if(currSlice->svc_extension_flag == -1) //SVC and the normal AVC;
      {
        if(p_Vid->active_subset_sps == NULL)
        {
          currSlice->view_id = GetBaseViewId(p_Vid, &p_Vid->active_subset_sps);
          if(currSlice->NaluHeaderMVCExt.iPrefixNALU >0)
          {
            assert(currSlice->view_id == (int)currSlice->NaluHeaderMVCExt.view_id);
            currSlice->inter_view_flag = currSlice->NaluHeaderMVCExt.inter_view_flag;
            currSlice->anchor_pic_flag = currSlice->NaluHeaderMVCExt.anchor_pic_flag;
            currSlice->temporal_id=currSlice->NaluHeaderMVCExt.temporal_id;
          }
          else
          {
            currSlice->inter_view_flag = 1;
            currSlice->anchor_pic_flag = currSlice->idr_flag;
            currSlice->temporal_id=0;
          }
        }
        else
        {
          assert(p_Vid->active_subset_sps->num_views_minus1 >=0);
          // prefix NALU available
          if(currSlice->NaluHeaderMVCExt.iPrefixNALU >0)
          {
            currSlice->view_id = currSlice->NaluHeaderMVCExt.view_id;
            currSlice->inter_view_flag = currSlice->NaluHeaderMVCExt.inter_view_flag;
            currSlice->anchor_pic_flag = currSlice->NaluHeaderMVCExt.anchor_pic_flag;
            currSlice->temporal_id=currSlice->NaluHeaderMVCExt.temporal_id;
          }
          else
          { //no prefix NALU;
            currSlice->view_id = p_Vid->active_subset_sps->view_id[0];
            currSlice->inter_view_flag = 1;
            currSlice->anchor_pic_flag = currSlice->idr_flag;
            currSlice->temporal_id=0;
          }
        }
      }
#elif MVC_EXTENSION_ENABLE
      if(currSlice->svc_extension_flag == 0)
      {  //MVC          
        currSlice->view_id = currSlice->NaluHeaderMVCExt.view_id;
        currSlice->inter_view_flag = currSlice->NaluHeaderMVCExt.inter_view_flag;
        currSlice->anchor_pic_flag = currSlice->NaluHeaderMVCExt.anchor_pic_flag;
      }
      else if(currSlice->svc_extension_flag == -1) //SVC and the normal AVC;
      {          
        if(p_Vid->active_subset_sps == NULL)
        {
          currSlice->view_id = GetBaseViewId(p_Vid, &p_Vid->active_subset_sps);
          if(currSlice->NaluHeaderMVCExt.iPrefixNALU >0)
          {
            assert(currSlice->view_id == currSlice->NaluHeaderMVCExt.view_id);
            currSlice->inter_view_flag = currSlice->NaluHeaderMVCExt.inter_view_flag;
            currSlice->anchor_pic_flag = currSlice->NaluHeaderMVCExt.anchor_pic_flag;
          }
          else
          {
            currSlice->inter_view_flag = 1;
            currSlice->anchor_pic_flag = currSlice->idr_flag;

          }
        }
        else
        {                
          assert(p_Vid->active_subset_sps->num_views_minus1 >=0);
          // prefix NALU available
          if(currSlice->NaluHeaderMVCExt.iPrefixNALU >0)
          {
            currSlice->view_id = currSlice->NaluHeaderMVCExt.view_id;
            currSlice->inter_view_flag = currSlice->NaluHeaderMVCExt.inter_view_flag;
            currSlice->anchor_pic_flag = currSlice->NaluHeaderMVCExt.anchor_pic_flag;
          }
          else
          { //no prefix NALU;
            currSlice->view_id = p_Vid->active_subset_sps->view_id[0];
            currSlice->inter_view_flag = 1;
            currSlice->anchor_pic_flag = currSlice->idr_flag;
          }
        }      
      }
#endif

      //fmo_init (p_Vid, currSlice);
      //currSlice->frame_num  = p_Vid->frame_num;        
      //currSlice->active_sps = p_Vid->active_sps;
      //currSlice->active_pps = p_Vid->active_pps;

      assign_quant_params (currSlice);        

      // if primary slice is replaced with redundant slice, set the correct image type
      if(currSlice->redundant_pic_cnt && p_Vid->Is_primary_correct==0 && p_Vid->Is_redundant_correct)
      {
        p_Vid->dec_picture->slice_type = p_Vid->type;
      }

      if(is_new_picture(p_Vid->dec_picture, currSlice, p_Vid->old_slice))
      {
        if(p_Vid->iSliceNumOfCurrPic==0)
          init_picture(p_Vid, currSlice, p_Inp);

#if EXT3D // SAIT_INDR_C0094
    //////////////////////////////////////////////////////////////////////////
    // EHP Profile(ThreeDV_EXTEND_HIGH)
    if(p_Vid->andr_flag==1)
    {
      int i;
      p_Vid->NonlinearDepthNum=g_NonlinearDepthNum_dec;
      p_Vid->NonlinearDepthPoints[0]=0;   // beginning
      for (i=1; i<=g_NonlinearDepthNum_dec; ++i)
      {
        p_Vid->NonlinearDepthPoints[i]=g_NonlinearDepthPoints_dec[i];
      }
      p_Vid->NonlinearDepthPoints[g_NonlinearDepthNum_dec+1]=0; // end
    }
    else
    {
      int i;
      p_Vid->NonlinearDepthNum=0;
      p_Vid->NonlinearDepthPoints[0]=0;   // beginning
      for (i=1; i<=g_NonlinearDepthNum_dec; ++i)
      {
        p_Vid->NonlinearDepthPoints[i]=0;
      }
      p_Vid->NonlinearDepthPoints[g_NonlinearDepthNum_dec+1]=0; // end
    }
#endif
        current_header = SOP;
        //check zero_byte if it is also the first NAL unit in the access unit
        CheckZeroByteVCL(p_Vid, nalu);
      }
      else
        current_header = SOS;

      setup_slice_methods(currSlice);

      // From here on, p_Vid->active_sps, p_Vid->active_pps and the slice header are valid
      if (currSlice->mb_aff_frame_flag)
        currSlice->current_mb_nr = currSlice->start_mb_nr << 1;
      else
        currSlice->current_mb_nr = currSlice->start_mb_nr;

      if (p_Vid->active_pps->entropy_coding_mode_flag)
      {
        int ByteStartPosition = currStream->frame_bitoffset/8;
        if (currStream->frame_bitoffset%8 != 0)
        {
          ++ByteStartPosition;
        }
        arideco_start_decoding (&currSlice->partArr[0].de_cabac, currStream->streamBuffer, ByteStartPosition, &currStream->read_len);
      }
      // printf ("read_new_slice: returning %s\n", current_header == SOP?"SOP":"SOS");
      //FreeNALU(nalu);
      p_Vid->recovery_point = 0;
      return current_header;
      break;
    case NALU_TYPE_DPA:
      // read DP_A
      currSlice->dpB_NotPresent =1; 
      currSlice->dpC_NotPresent =1; 

      currSlice->idr_flag          = FALSE;
      currSlice->nal_reference_idc = nalu->nal_reference_idc;
      currSlice->dp_mode     = PAR_DP_3;
      currSlice->max_part_nr = 3;
      currSlice->ei_flag     = 0;
      currStream             = currSlice->partArr[0].bitstream;
      currStream->ei_flag    = 0;
      currStream->frame_bitoffset = currStream->read_len = 0;
      memcpy (currStream->streamBuffer, &nalu->buf[1], nalu->len-1);
      currStream->code_len = currStream->bitstream_length = RBSPtoSODB(currStream->streamBuffer, nalu->len-1);

      BitsUsedByHeader     = FirstPartOfSliceHeader(currSlice);
      UseParameterSet (currSlice);
      BitsUsedByHeader += RestOfSliceHeader (currSlice);

      fmo_init (p_Vid, currSlice);

      if(is_new_picture(p_Vid->dec_picture, currSlice, p_Vid->old_slice))
      {
        init_picture(p_Vid, currSlice, p_Inp);
        current_header = SOP;
        CheckZeroByteVCL(p_Vid, nalu);
      }
      else
        current_header = SOS;

      update_pic_num(currSlice);
      currSlice->init_lists(currSlice);
      reorder_lists (currSlice);

      if (p_Vid->structure==FRAME)
      {
        init_mbaff_lists(p_Vid, currSlice);
      }

      // From here on, p_Vid->active_sps, p_Vid->active_pps and the slice header are valid
      if (currSlice->mb_aff_frame_flag)
        currSlice->current_mb_nr = currSlice->start_mb_nr << 1;
      else
        currSlice->current_mb_nr = currSlice->start_mb_nr;

      // Now I need to read the slice ID, which depends on the value of
      // redundant_pic_cnt_present_flag

      slice_id_a  = ue_v("NALU: DP_A slice_id", currStream);

      if (p_Vid->active_pps->entropy_coding_mode_flag)
        error ("received data partition with CABAC, this is not allowed", 500);

      // continue with reading next DP
      if (0 == read_next_nalu(p_Vid, nalu))
        return current_header;

      if ( NALU_TYPE_DPB == nalu->nal_unit_type)
      {
        // we got a DPB
        currStream             = currSlice->partArr[1].bitstream;
        currStream->ei_flag    = 0;
        currStream->frame_bitoffset = currStream->read_len = 0;

        memcpy (currStream->streamBuffer, &nalu->buf[1], nalu->len-1);
        currStream->code_len = currStream->bitstream_length = RBSPtoSODB(currStream->streamBuffer, nalu->len-1);

        slice_id_b  = ue_v("NALU: DP_B slice_id", currStream);

        currSlice->dpB_NotPresent = 0; 

        if ((slice_id_b != slice_id_a) || (nalu->lost_packets))
        {
          printf ("Waning: got a data partition B which does not match DP_A (DP loss!)\n");
          currSlice->dpB_NotPresent =1; 
          currSlice->dpC_NotPresent =1; 
        }
        else
        {
          if (p_Vid->active_pps->redundant_pic_cnt_present_flag)
            ue_v("NALU: DP_B redudant_pic_cnt", currStream);

          // we're finished with DP_B, so let's continue with next DP
          if (0 == read_next_nalu(p_Vid, nalu))
            return current_header;
        }
      }
      else
      {
        currSlice->dpB_NotPresent =1; 
      }

      // check if we got DP_C
      if ( NALU_TYPE_DPC == nalu->nal_unit_type)
      {
        currStream             = currSlice->partArr[2].bitstream;
        currStream->ei_flag    = 0;
        currStream->frame_bitoffset = currStream->read_len = 0;

        memcpy (currStream->streamBuffer, &nalu->buf[1], nalu->len-1);
        currStream->code_len = currStream->bitstream_length = RBSPtoSODB(currStream->streamBuffer, nalu->len-1);

        currSlice->dpC_NotPresent = 0;

        slice_id_c  = ue_v("NALU: DP_C slice_id", currStream);
        if ((slice_id_c != slice_id_a)|| (nalu->lost_packets))
        {
          printf ("Warning: got a data partition C which does not match DP_A(DP loss!)\n");
          //currSlice->dpB_NotPresent =1;
          currSlice->dpC_NotPresent =1;
        }

        if (p_Vid->active_pps->redundant_pic_cnt_present_flag)
          ue_v("NALU:SLICE_C redudand_pic_cnt", currStream);
      }
      else
      {
        currSlice->dpC_NotPresent =1;
      }

      // check if we read anything else than the expected partitions
      if ((nalu->nal_unit_type != NALU_TYPE_DPB) && (nalu->nal_unit_type != NALU_TYPE_DPC))
      {
        // we have a NALI that we can't process here, so restart processing
        goto process_nalu;
        // yes, "goto" should not be used, but it's really the best way here before we restructure the decoding loop
        // (which should be taken care of anyway)
      }

      //FreeNALU(nalu);
      return current_header;

      break;
    case NALU_TYPE_DPB:
      if (p_Inp->silent == FALSE)
      {
        printf ("found data partition B without matching DP A, discarding\n");
      }
      break;
    case NALU_TYPE_DPC:
      if (p_Inp->silent == FALSE)
      {
        printf ("found data partition C without matching DP A, discarding\n");
      }
      break;
    case NALU_TYPE_SEI:
      //printf ("read_new_slice: Found NALU_TYPE_SEI, len %d\n", nalu->len);
      InterpretSEIMessage(nalu->buf,nalu->len,p_Vid, currSlice);
      break;
    case NALU_TYPE_PPS:
      //printf ("Found NALU_TYPE_PPS\n");
      ProcessPPS(p_Vid, nalu);
      break;
    case NALU_TYPE_SPS:
      //printf ("Found NALU_TYPE_SPS\n");
      ProcessSPS(p_Vid, nalu);
      break;
    case NALU_TYPE_AUD:
      //printf ("Found NALU_TYPE_AUD\n");
      //        printf ("read_new_slice: Found 'Access Unit Delimiter' NAL unit, len %d, ignored\n", nalu->len);
      break;
    case NALU_TYPE_EOSEQ:
      //        printf ("read_new_slice: Found 'End of Sequence' NAL unit, len %d, ignored\n", nalu->len);
      break;
    case NALU_TYPE_EOSTREAM:
      //        printf ("read_new_slice: Found 'End of Stream' NAL unit, len %d, ignored\n", nalu->len);
      break;
    case NALU_TYPE_FILL:
      if (p_Inp->silent == FALSE)
      {
        printf ("read_new_slice: Found NALU_TYPE_FILL, len %d\n", (int) nalu->len);
        printf ("Skipping these filling bits, proceeding w/ next NALU\n");
      }
      break;
#if MVC_EXTENSION_ENABLE||EXT3D
    case NALU_TYPE_VDRD:
      //printf ("Found NALU_TYPE_VDRD\n");
      //        printf ("read_new_slice: Found 'View and Dependency Representation Delimiter' NAL unit, len %d, ignored\n", nalu->len);
      break;
    case NALU_TYPE_PREFIX:
      //printf ("Found NALU_TYPE_PREFIX\n");
      if(currSlice->svc_extension_flag==1)
        prefix_nal_unit_svc();
      break;  
    case NALU_TYPE_SUB_SPS:
      //printf ("Found NALU_TYPE_SUB_SPS\n");
      if (p_Inp->DecodeAllLayers== 1)
        ProcessSubsetSPS(p_Vid, nalu);
      else
      {
        if (p_Inp->silent == FALSE)
          printf ("Found Subsequence SPS NALU. Ignoring.\n");
      }
      break;
    case NALU_TYPE_SLC_EXT:
      //printf ("Found NALU_TYPE_SLC_EXT\n");
      if (p_Inp->DecodeAllLayers == 0 &&  (p_Inp->silent == FALSE))
        printf ("Found SVC extension NALU (%d). Ignoring.\n", (int) nalu->nal_unit_type);
      break;
#if EXT3D
    case NALU_TYPE_DPS:
      //printf("Found NALU_TYPE_DPS\n");
      ProcessDPS(p_Vid, nalu);
      break;
#endif
#endif

    default:
      {
        if (p_Inp->silent == FALSE)
          printf ("Found NALU type %d, len %d undefined, ignore NALU, moving on\n", (int) nalu->nal_unit_type, (int) nalu->len);
      }
      break;
    }
  }
}

void pad_buf(imgpel *pImgBuf, int iWidth, int iHeight, int iStride, int iPadX, int iPadY)
{
  int j;
  imgpel *pLine0 = pImgBuf - iPadX, *pLine;
#if (IMGTYPE==0)
  int pad_width = iPadX + iWidth;
  fast_memset(pImgBuf - iPadX, *pImgBuf, iPadX * sizeof(imgpel));
  fast_memset(pImgBuf + iWidth, *(pImgBuf + iWidth - 1), iPadX * sizeof(imgpel));

  pLine = pLine0 - iPadY * iStride;
  for(j = -iPadY; j < 0; j++)
  {
    fast_memcpy(pLine, pLine0, iStride * sizeof(imgpel));
    pLine += iStride;
  }

  for(j = 1; j < iHeight; j++)
  {
    pLine += iStride;
    fast_memset(pLine, *(pLine + iPadX), iPadX * sizeof(imgpel));
    fast_memset(pLine + pad_width, *(pLine + pad_width - 1), iPadX * sizeof(imgpel));
  }

  pLine0 = pLine + iStride;

  for(j = iHeight; j < iHeight + iPadY; j++)
  {
    fast_memcpy(pLine0,  pLine, iStride * sizeof(imgpel));
    pLine0 += iStride;
  }
#else
  int i;
  for(i=-iPadX; i<0; i++)
    pImgBuf[i] = *pImgBuf;
  for(i=0; i<iPadX; i++)
    pImgBuf[i+iWidth] = *(pImgBuf+iWidth-1);

  for(j=-iPadY; j<0; j++)
    memcpy(pLine0+j*iStride, pLine0, iStride*sizeof(imgpel));
  for(j=1; j<iHeight; j++)
  {
    pLine = pLine0 + j*iStride;
    for(i=0; i<iPadX; i++)
      pLine[i] = pLine[iPadX];
    pLine += iPadX+iWidth-1;
    for(i=1; i<iPadX+1; i++)
      pLine[i] = *pLine;
  }
  pLine = pLine0 + (iHeight-1)*iStride;
  for(j=iHeight; j<iHeight+iPadY; j++)
    memcpy(pLine0+j*iStride,  pLine, iStride*sizeof(imgpel));
#endif
}

void pad_dec_picture(VideoParameters *p_Vid, StorablePicture *dec_picture)
{
  int iPadX = p_Vid->iLumaPadX;
  int iPadY = p_Vid->iLumaPadY;
  int iWidth = dec_picture->size_x;
  int iHeight = dec_picture->size_y;
  int iStride = dec_picture->iLumaStride;

  pad_buf(*dec_picture->imgY, iWidth, iHeight, iStride, iPadX, iPadY);

  if(dec_picture->chroma_format_idc != YUV400) 
  {
    iPadX = p_Vid->iChromaPadX;
    iPadY = p_Vid->iChromaPadY;
    iWidth = dec_picture->size_x_cr;
    iHeight = dec_picture->size_y_cr;
    iStride = dec_picture->iChromaStride;
    pad_buf(*dec_picture->imgUV[0], iWidth, iHeight, iStride, iPadX, iPadY);
    pad_buf(*dec_picture->imgUV[1], iWidth, iHeight, iStride, iPadX, iPadY);
  }
}

/*!
 ************************************************************************
 * \brief
 *    finish decoding of a picture, conceal errors and store it
 *    into the DPB
 ************************************************************************
 */
void exit_picture(VideoParameters *p_Vid, StorablePicture **dec_picture)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
#if EXT3D
  //int  ViewId=(*dec_picture)->view_id;
  int is_depth=(*dec_picture)->is_depth;
  int   voidx=GetVOIdx(p_Vid,(*dec_picture)->view_id) ;
  SNRParameters   *snr   = p_Vid->snr_3dv[is_depth][voidx];
  
#else
  SNRParameters   *snr   = p_Vid->snr;
#endif
  char yuv_types[4][6]= {"4:0:0","4:2:0","4:2:2","4:4:4"};
#if (DISABLE_ERC == 0)
  int ercStartMB;
  int ercSegment;
  frame recfr;
#endif
#if EXT3D
  int structure, frame_poc, slice_type, refpic, qp, pic_num, chroma_format_idc, is_idr;// , top_poc, bottom_poc;
#else
  int structure, frame_poc, slice_type, refpic, qp, pic_num, chroma_format_idc, is_idr, top_poc, bottom_poc;
#endif
  int64 tmp_time;                   // time used by decoding the last frame
  char   yuvFormat[10];


  // return if the last picture has already been finished
  if (*dec_picture==NULL || (p_Vid->num_dec_mb != p_Vid->PicSizeInMbs && (p_Vid->yuv_format != YUV444 || !p_Vid->separate_colour_plane_flag)))
  {
    return;
  }

#if (DISABLE_ERC == 0)
  recfr.p_Vid = p_Vid;
  recfr.yptr = &(*dec_picture)->imgY[0][0];
  if ((*dec_picture)->chroma_format_idc != YUV400)
  {
    recfr.uptr = &(*dec_picture)->imgUV[0][0][0];
    recfr.vptr = &(*dec_picture)->imgUV[1][0][0];
  }

  //! this is always true at the beginning of a picture
  ercStartMB = 0;
  ercSegment = 0;
#endif
  
  //! mark the start of the first segment
#if (DISABLE_ERC == 0)
  if (!(*dec_picture)->mb_aff_frame_flag)
  {
    int i;
    ercStartSegment(0, ercSegment, 0 , p_Vid->erc_errorVar);
    //! generate the segments according to the macroblock map
    for(i = 1; i<(*dec_picture)->PicSizeInMbs; ++i)
    {
      if(p_Vid->mb_data[i].ei_flag != p_Vid->mb_data[i-1].ei_flag)
      {
        ercStopSegment(i-1, ercSegment, 0, p_Vid->erc_errorVar); //! stop current segment

        //! mark current segment as lost or OK
        if(p_Vid->mb_data[i-1].ei_flag)
          ercMarkCurrSegmentLost((*dec_picture)->size_x, p_Vid->erc_errorVar);
        else
          ercMarkCurrSegmentOK((*dec_picture)->size_x, p_Vid->erc_errorVar);

        ++ercSegment;  //! next segment
        ercStartSegment(i, ercSegment, 0 , p_Vid->erc_errorVar); //! start new segment
        ercStartMB = i;//! save start MB for this segment
      }
    }
    //! mark end of the last segment
    ercStopSegment((*dec_picture)->PicSizeInMbs-1, ercSegment, 0, p_Vid->erc_errorVar);
    if(p_Vid->mb_data[i-1].ei_flag)
      ercMarkCurrSegmentLost((*dec_picture)->size_x, p_Vid->erc_errorVar);
    else
      ercMarkCurrSegmentOK((*dec_picture)->size_x, p_Vid->erc_errorVar);

    //! call the right error concealment function depending on the frame type.
    p_Vid->erc_mvperMB /= (*dec_picture)->PicSizeInMbs;

    p_Vid->erc_img = p_Vid;

    if((*dec_picture)->slice_type == I_SLICE || (*dec_picture)->slice_type == SI_SLICE) // I-frame
      ercConcealIntraFrame(p_Vid, &recfr, (*dec_picture)->size_x, (*dec_picture)->size_y, p_Vid->erc_errorVar);
    else
      ercConcealInterFrame(&recfr, p_Vid->erc_object_list, (*dec_picture)->size_x, (*dec_picture)->size_y, p_Vid->erc_errorVar, (*dec_picture)->chroma_format_idc);
  }
#endif

  if(!p_Vid->iDeblockMode && (p_Vid->bDeblockEnable & (1<<(*dec_picture)->used_for_reference)))
  {
    //deblocking for frame or field
    if( (p_Vid->separate_colour_plane_flag != 0) )
    {
      int nplane;
      int colour_plane_id = p_Vid->ppSliceList[0]->colour_plane_id;
      for( nplane=0; nplane<MAX_PLANE; ++nplane )
      {
        p_Vid->ppSliceList[0]->colour_plane_id = nplane;
        change_plane_JV( p_Vid, nplane, NULL );
        DeblockPicture( p_Vid, *dec_picture );
      }
      p_Vid->ppSliceList[0]->colour_plane_id = colour_plane_id;
      make_frame_picture_JV(p_Vid);
    }
    else
    {
      DeblockPicture( p_Vid, *dec_picture );
    }
  }
  else
  {
    if( (p_Vid->separate_colour_plane_flag != 0) )
    {
      make_frame_picture_JV(p_Vid);
    }
  }

  if ((*dec_picture)->mb_aff_frame_flag)
    MbAffPostProc(p_Vid);

  if (p_Vid->structure == FRAME)         // buffer mgt. for frame mode
    frame_postprocessing(p_Vid);
  else
    field_postprocessing(p_Vid);   // reset all interlaced variables

#if MVC_EXTENSION_ENABLE||EXT3D
  if((*dec_picture)->used_for_reference || ((*dec_picture)->inter_view_flag == 1))
    pad_dec_picture(p_Vid, *dec_picture);
#else
  if((*dec_picture)->used_for_reference)
    pad_dec_picture(p_Vid, *dec_picture);
#endif
  structure  = (*dec_picture)->structure;
  slice_type = (*dec_picture)->slice_type;
  frame_poc  = (*dec_picture)->frame_poc;
#if !EXT3D
  top_poc    = (*dec_picture)->top_poc;
  bottom_poc = (*dec_picture)->bottom_poc;
#endif
  refpic     = (*dec_picture)->used_for_reference;
  qp         = (*dec_picture)->qp;
  pic_num    = (*dec_picture)->pic_num;
  is_idr     = (*dec_picture)->idr_flag;

  chroma_format_idc = (*dec_picture)->chroma_format_idc;

#if EXT3D
  store_picture_in_dpb(p_Vid->p_Dpb[is_depth], *dec_picture);
#else
  store_picture_in_dpb(p_Vid->p_Dpb, *dec_picture);
#endif
  *dec_picture=NULL;

#if EXT3D
  if (p_Vid->last_has_mmco_5[is_depth])
#else
  if (p_Vid->last_has_mmco_5)
#endif
  {
    p_Vid->pre_frame_num = 0;
  }

  if (p_Inp->silent == FALSE)
  {
    if (structure==TOP_FIELD || structure==FRAME)
    {
      if(slice_type == I_SLICE && is_idr) // IDR picture
        strcpy(p_Vid->cslice_type,"IDR");
      else if(slice_type == I_SLICE) // I picture
        strcpy(p_Vid->cslice_type," I ");
      else if(slice_type == P_SLICE) // P pictures
        strcpy(p_Vid->cslice_type," P ");
      else if(slice_type == SP_SLICE) // SP pictures
        strcpy(p_Vid->cslice_type,"SP ");
      else if (slice_type == SI_SLICE)
        strcpy(p_Vid->cslice_type,"SI ");
      else if(refpic) // stored B pictures
        strcpy(p_Vid->cslice_type," B ");
      else // B pictures
        strcpy(p_Vid->cslice_type," b ");

      if (structure==FRAME)
      {
        strncat(p_Vid->cslice_type,")       ",8-strlen(p_Vid->cslice_type));
      }
    }
    else if (structure==BOTTOM_FIELD)
    {
      if(slice_type == I_SLICE && is_idr) // IDR picture
        strncat(p_Vid->cslice_type,"|IDR)",8-strlen(p_Vid->cslice_type));
      else if(slice_type == I_SLICE) // I picture
        strncat(p_Vid->cslice_type,"| I )",8-strlen(p_Vid->cslice_type));
      else if(slice_type == P_SLICE) // P pictures
        strncat(p_Vid->cslice_type,"| P )",8-strlen(p_Vid->cslice_type));
      else if(slice_type == SP_SLICE) // SP pictures
        strncat(p_Vid->cslice_type,"|SP )",8-strlen(p_Vid->cslice_type));
      else if (slice_type == SI_SLICE)
        strncat(p_Vid->cslice_type,"|SI )",8-strlen(p_Vid->cslice_type));
      else if(refpic) // stored B pictures
        strncat(p_Vid->cslice_type,"| B )",8-strlen(p_Vid->cslice_type));
      else // B pictures
        strncat(p_Vid->cslice_type,"| b )",8-strlen(p_Vid->cslice_type));   
    }
  }

  if ((structure==FRAME)||structure==BOTTOM_FIELD)
  {
    gettime (&(p_Vid->end_time));              // end time

    tmp_time  = timediff(&(p_Vid->start_time), &(p_Vid->end_time));
    p_Vid->tot_time += tmp_time;
    tmp_time  = timenorm(tmp_time);
    sprintf(yuvFormat,"%s", yuv_types[chroma_format_idc]);

    if (p_Inp->silent == FALSE)
    {
#if EXT3D
      SNRParameters   *snr = p_Vid->snr_3dv[is_depth][voidx];
#else
      SNRParameters   *snr = p_Vid->snr;
#endif
#if EXT3D
      if(p_Vid->p_ref_3dv[is_depth][voidx])
#else
      if (p_Vid->p_ref != -1)
#endif
        fprintf(stdout,"%05d(%s%5d %5d %5d %8.4f %8.4f %8.4f  %s %7d\n",
        p_Vid->frame_no, p_Vid->cslice_type, frame_poc, pic_num, qp, snr->snr[0], snr->snr[1], snr->snr[2], yuvFormat, (int) tmp_time);
      else
        fprintf(stdout,"%05d(%s%5d %5d %5d                             %s %7d\n",
        p_Vid->frame_no, p_Vid->cslice_type, frame_poc, pic_num, qp, yuvFormat, (int)tmp_time);
    }
    else
      fprintf(stdout,"Completed Decoding frame %05d.\n",snr->frame_ctr);

    fflush(stdout);

    if(slice_type == I_SLICE || slice_type == SI_SLICE || slice_type == P_SLICE || refpic)   // I or P pictures
    {
#if MVC_EXTENSION_ENABLE||EXT3D
      if((p_Vid->ppSliceList[0])->view_id!=0)
#endif
        ++(p_Vid->number);
    }
    else
      ++(p_Vid->Bframe_ctr);    // B pictures
    ++(snr->frame_ctr);

    ++(p_Vid->g_nFrame);
  }

  //p_Vid->currentSlice->current_mb_nr = -4712;   // impossible value for debugging, StW
  //p_Vid->currentSlice->current_slice_nr = 0;
}

/*!
 ************************************************************************
 * \brief
 *    write the encoding mode and motion vectors of current
 *    MB to the buffer of the error concealment module.
 ************************************************************************
 */

void ercWriteMBMODEandMV(Macroblock *currMB)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  int i, ii, jj, currMBNum = currMB->mbAddrX; //p_Vid->currentSlice->current_mb_nr;
  StorablePicture *dec_picture = p_Vid->dec_picture;
  int mbx = xPosMB(currMBNum, dec_picture->size_x), mby = yPosMB(currMBNum, dec_picture->size_x);
  objectBuffer_t *currRegion, *pRegion;

  currRegion = p_Vid->erc_object_list + (currMBNum<<2);

  if(p_Vid->type != B_SLICE) //non-B frame
  {
    for (i=0; i<4; ++i)
    {
      pRegion             = currRegion + i;
      pRegion->regionMode = (currMB->mb_type  ==I16MB  ? REGMODE_INTRA      :
        currMB->b8mode[i]==IBLOCK ? REGMODE_INTRA_8x8  :
        currMB->b8mode[i]==0      ? REGMODE_INTER_COPY :
        currMB->b8mode[i]==1      ? REGMODE_INTER_PRED : REGMODE_INTER_PRED_8x8);
      if (currMB->b8mode[i]==0 || currMB->b8mode[i]==IBLOCK)  // INTRA OR COPY
      {
        pRegion->mv[0]    = 0;
        pRegion->mv[1]    = 0;
        pRegion->mv[2]    = 0;
      }
      else
      {
        ii              = 4*mbx + (i & 0x01)*2;// + BLOCK_SIZE;
        jj              = 4*mby + (i >> 1  )*2;
        if (currMB->b8mode[i]>=5 && currMB->b8mode[i]<=7)  // SMALL BLOCKS
        {
          pRegion->mv[0]  = (dec_picture->mv_info[jj][ii].mv[LIST_0].mv_x + dec_picture->mv_info[jj][ii + 1].mv[LIST_0].mv_x + dec_picture->mv_info[jj + 1][ii].mv[LIST_0].mv_x + dec_picture->mv_info[jj + 1][ii + 1].mv[LIST_0].mv_x + 2)/4;
          pRegion->mv[1]  = (dec_picture->mv_info[jj][ii].mv[LIST_0].mv_y + dec_picture->mv_info[jj][ii + 1].mv[LIST_0].mv_y + dec_picture->mv_info[jj + 1][ii].mv[LIST_0].mv_y + dec_picture->mv_info[jj + 1][ii + 1].mv[LIST_0].mv_y + 2)/4;
        }
        else // 16x16, 16x8, 8x16, 8x8
        {
          pRegion->mv[0]  = dec_picture->mv_info[jj][ii].mv[LIST_0].mv_x;
          pRegion->mv[1]  = dec_picture->mv_info[jj][ii].mv[LIST_0].mv_y;
          //          pRegion->mv[0]  = dec_picture->motion.mv[LIST_0][4*mby+(i/2)*2][4*mbx+(i%2)*2+BLOCK_SIZE][0];
          //          pRegion->mv[1]  = dec_picture->motion.mv[LIST_0][4*mby+(i/2)*2][4*mbx+(i%2)*2+BLOCK_SIZE][1];
        }
        currMB->p_Slice->erc_mvperMB      += iabs(pRegion->mv[0]) + iabs(pRegion->mv[1]);
        pRegion->mv[2]    = dec_picture->mv_info[jj][ii].ref_idx[LIST_0];
      }
    }
  }
  else  //B-frame
  {
    for (i=0; i<4; ++i)
    {
      ii                  = 4*mbx + (i%2)*2;// + BLOCK_SIZE;
      jj                  = 4*mby + (i/2)*2;
      pRegion             = currRegion + i;
      pRegion->regionMode = (currMB->mb_type  ==I16MB  ? REGMODE_INTRA      :
        currMB->b8mode[i]==IBLOCK ? REGMODE_INTRA_8x8  : REGMODE_INTER_PRED_8x8);
      if (currMB->mb_type==I16MB || currMB->b8mode[i]==IBLOCK)  // INTRA
      {
        pRegion->mv[0]    = 0;
        pRegion->mv[1]    = 0;
        pRegion->mv[2]    = 0;
      }
      else
      {
        int idx = (dec_picture->mv_info[jj][ii].ref_idx[0] < 0) ? 1 : 0;
        //        int idx = (currMB->b8mode[i]==0 && currMB->b8pdir[i]==2 ? LIST_0 : currMB->b8pdir[i]==1 ? LIST_1 : LIST_0);
        //        int idx = currMB->b8pdir[i]==0 ? LIST_0 : LIST_1;
        pRegion->mv[0]    = (dec_picture->mv_info[jj][ii].mv[idx].mv_x + 
          dec_picture->mv_info[jj][ii+1].mv[idx].mv_x + 
          dec_picture->mv_info[jj+1][ii].mv[idx].mv_x + 
          dec_picture->mv_info[jj+1][ii+1].mv[idx].mv_x + 2)/4;
        pRegion->mv[1]    = (dec_picture->mv_info[jj][ii].mv[idx].mv_y + 
          dec_picture->mv_info[jj][ii+1].mv[idx].mv_y + 
          dec_picture->mv_info[jj+1][ii].mv[idx].mv_y + 
          dec_picture->mv_info[jj+1][ii+1].mv[idx].mv_y + 2)/4;
        currMB->p_Slice->erc_mvperMB      += iabs(pRegion->mv[0]) + iabs(pRegion->mv[1]);

        pRegion->mv[2]  = (dec_picture->mv_info[jj][ii].ref_idx[idx]);
        /*
        if (currMB->b8pdir[i]==0 || (currMB->b8pdir[i]==2 && currMB->b8mode[i]!=0)) // forward or bidirect
        {
        pRegion->mv[2]  = (dec_picture->motion.ref_idx[LIST_0][jj][ii]);
        ///???? is it right, not only "p_Vid->fw_refFrArr[jj][ii-4]"
        }
        else
        {
        pRegion->mv[2]  = (dec_picture->motion.ref_idx[LIST_1][jj][ii]);
        //          pRegion->mv[2]  = 0;
        }
        */
      }
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    set defaults for old_slice
 *    NAL unit of a picture"
 ************************************************************************
 */
void init_old_slice(OldSliceParams *p_old_slice)
{
  p_old_slice->field_pic_flag = 0;

  p_old_slice->pps_id = INT_MAX;

  p_old_slice->frame_num = INT_MAX;

  p_old_slice->nal_ref_idc = INT_MAX;

  p_old_slice->idr_flag = FALSE;

  p_old_slice->pic_oder_cnt_lsb          = UINT_MAX;
  p_old_slice->delta_pic_oder_cnt_bottom = INT_MAX;

  p_old_slice->delta_pic_order_cnt[0] = INT_MAX;
  p_old_slice->delta_pic_order_cnt[1] = INT_MAX;
#if EXT3D
  p_old_slice->is_depth=0;
#endif
}


void CopySliceInfo(Slice *currSlice, OldSliceParams *p_old_slice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;

  p_old_slice->pps_id         = currSlice->pic_parameter_set_id;
  p_old_slice->frame_num      = currSlice->frame_num; //p_Vid->frame_num;
  p_old_slice->field_pic_flag = currSlice->field_pic_flag; //p_Vid->field_pic_flag;

  if(currSlice->field_pic_flag)
  {
    p_old_slice->bottom_field_flag = currSlice->bottom_field_flag;
  }

  p_old_slice->nal_ref_idc = currSlice->nal_reference_idc;
  p_old_slice->idr_flag    = (byte) currSlice->idr_flag;

  if (currSlice->idr_flag)
  {
    p_old_slice->idr_pic_id = currSlice->idr_pic_id;
  }

  if (p_Vid->active_sps->pic_order_cnt_type == 0)
  {
    p_old_slice->pic_oder_cnt_lsb          = currSlice->pic_order_cnt_lsb;
    p_old_slice->delta_pic_oder_cnt_bottom = currSlice->delta_pic_order_cnt_bottom;
  }

  if (p_Vid->active_sps->pic_order_cnt_type == 1)
  {
    p_old_slice->delta_pic_order_cnt[0] = currSlice->delta_pic_order_cnt[0];
    p_old_slice->delta_pic_order_cnt[1] = currSlice->delta_pic_order_cnt[1];
  }
#if MVC_EXTENSION_ENABLE||EXT3D
  p_old_slice->view_id = currSlice->view_id;
  p_old_slice->inter_view_flag = currSlice->inter_view_flag; 
  p_old_slice->anchor_pic_flag = currSlice->anchor_pic_flag;
#endif
#if EXT3D
  p_old_slice->is_depth=currSlice->is_depth;
#endif
}

/*!
 ************************************************************************
 * \brief
 *    detect if current slice is "first VCL NAL unit of a picture"
 ************************************************************************
 */
int is_new_picture(StorablePicture *dec_picture, Slice *currSlice, OldSliceParams *p_old_slice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;

  int result=0;

  result |= (NULL==dec_picture);

  result |= (p_old_slice->pps_id != currSlice->pic_parameter_set_id);

  result |= (p_old_slice->frame_num != currSlice->frame_num);

  result |= (p_old_slice->field_pic_flag != currSlice->field_pic_flag);

  if(currSlice->field_pic_flag && p_old_slice->field_pic_flag)
  {
    result |= (p_old_slice->bottom_field_flag != currSlice->bottom_field_flag);
  }

  result |= (p_old_slice->nal_ref_idc != currSlice->nal_reference_idc) && ((p_old_slice->nal_ref_idc == 0) || (currSlice->nal_reference_idc == 0));
  result |= (p_old_slice->idr_flag    != currSlice->idr_flag);

  if (currSlice->idr_flag && p_old_slice->idr_flag)
  {
    result |= (p_old_slice->idr_pic_id != currSlice->idr_pic_id);
  }

  if (p_Vid->active_sps->pic_order_cnt_type == 0)
  {
    result |= (p_old_slice->pic_oder_cnt_lsb          != currSlice->pic_order_cnt_lsb);
    if( p_Vid->active_pps->bottom_field_pic_order_in_frame_present_flag  ==  1 &&  !currSlice->field_pic_flag )
    {
      result |= (p_old_slice->delta_pic_oder_cnt_bottom != currSlice->delta_pic_order_cnt_bottom);
    }
  }

  if (p_Vid->active_sps->pic_order_cnt_type == 1)
  {
    if (!p_Vid->active_sps->delta_pic_order_always_zero_flag)
    {
      result |= (p_old_slice->delta_pic_order_cnt[0] != currSlice->delta_pic_order_cnt[0]);
      if( p_Vid->active_pps->bottom_field_pic_order_in_frame_present_flag  ==  1 &&  !currSlice->field_pic_flag )
      {
        result |= (p_old_slice->delta_pic_order_cnt[1] != currSlice->delta_pic_order_cnt[1]);
      }
    }
  }

#if MVC_EXTENSION_ENABLE||EXT3D
  result |= (currSlice->view_id != p_old_slice->view_id);
  result |= (currSlice->inter_view_flag != p_old_slice->inter_view_flag);
  result |= (currSlice->anchor_pic_flag != p_old_slice->anchor_pic_flag);
#endif

  return result;
}


/*!
 ************************************************************************
 * \brief
 *    Prepare field and frame buffer after frame decoding
 ************************************************************************
 */
void frame_postprocessing(VideoParameters *p_Vid)
{

}

/*!
 ************************************************************************
 * \brief
 *    Prepare field and frame buffer after field decoding
 ************************************************************************
 */
void field_postprocessing(VideoParameters *p_Vid)
{
  p_Vid->number /= 2;
}



/*!
 ************************************************************************
 * \brief
 *    copy StorablePicture *src -> StorablePicture *dst
 *    for 4:4:4 Independent mode
 ************************************************************************
 */
void copy_dec_picture_JV( VideoParameters *p_Vid, StorablePicture *dst, StorablePicture *src )
{
  dst->top_poc              = src->top_poc;
  dst->bottom_poc           = src->bottom_poc;
  dst->frame_poc            = src->frame_poc;
  dst->qp                   = src->qp;
  dst->slice_qp_delta       = src->slice_qp_delta;
  dst->chroma_qp_offset[0]  = src->chroma_qp_offset[0];
  dst->chroma_qp_offset[1]  = src->chroma_qp_offset[1];

  dst->poc                  = src->poc;

  dst->slice_type           = src->slice_type;
  dst->used_for_reference   = src->used_for_reference;
  dst->idr_flag             = src->idr_flag;
  dst->no_output_of_prior_pics_flag = src->no_output_of_prior_pics_flag;
  dst->long_term_reference_flag = src->long_term_reference_flag;
  dst->adaptive_ref_pic_buffering_flag = src->adaptive_ref_pic_buffering_flag;

  dst->dec_ref_pic_marking_buffer = src->dec_ref_pic_marking_buffer;

  dst->mb_aff_frame_flag    = src->mb_aff_frame_flag;
  dst->PicWidthInMbs        = src->PicWidthInMbs;
  dst->pic_num              = src->pic_num;
  dst->frame_num            = src->frame_num;
  dst->recovery_frame       = src->recovery_frame;
  dst->coded_frame          = src->coded_frame;

  dst->chroma_format_idc    = src->chroma_format_idc;

  dst->frame_mbs_only_flag  = src->frame_mbs_only_flag;
  dst->frame_cropping_flag  = src->frame_cropping_flag;

  dst->frame_cropping_rect_left_offset   = src->frame_cropping_rect_left_offset;
  dst->frame_cropping_rect_right_offset  = src->frame_cropping_rect_right_offset;
  dst->frame_cropping_rect_top_offset    = src->frame_cropping_rect_top_offset;
  dst->frame_cropping_rect_bottom_offset = src->frame_cropping_rect_bottom_offset;

#if (ENABLE_OUTPUT_TONEMAPPING)
  // store the necessary tone mapping sei into StorablePicture structure
  dst->seiHasTone_mapping = src->seiHasTone_mapping;

  dst->seiHasTone_mapping    = src->seiHasTone_mapping;
  dst->tone_mapping_model_id = src->tone_mapping_model_id;
  dst->tonemapped_bit_depth  = src->tonemapped_bit_depth;
  if( src->tone_mapping_lut )
  {
    int coded_data_bit_max = (1 << p_Vid->seiToneMapping->coded_data_bit_depth);
    dst->tone_mapping_lut      = malloc(sizeof(int) * coded_data_bit_max);
    if (NULL == dst->tone_mapping_lut)
    {
      no_mem_exit("copy_dec_picture_JV: tone_mapping_lut");
    }
    memcpy(dst->tone_mapping_lut, src->tone_mapping_lut, sizeof(imgpel) * coded_data_bit_max);
  }
#endif
}


// this is intended to make get_block_luma faster by doing this at a more appropriate level
// i.e. per slice rather than per MB
static void init_cur_imgy(Slice *currSlice, VideoParameters *p_Vid)
{
  int i,j;
  if ((p_Vid->separate_colour_plane_flag != 0))  {
#if EXT3D
    StorablePicture *vidref = p_Vid->no_reference_picture[currSlice->is_depth];
#else
    StorablePicture *vidref = p_Vid->no_reference_picture;
#endif
    int noref = (currSlice->framepoc < p_Vid->recovery_poc);
    switch(currSlice->colour_plane_id) {
    case 0:
      for (j = 0; j < 6; j++) {  //for (j = 0; j < (currSlice->slice_type==B_SLICE?2:1); j++) { 
        for (i = 0; i < MAX_LIST_SIZE; i++) {
          StorablePicture *curr_ref = currSlice->listX[j][i];
          if (curr_ref) {
            curr_ref->no_ref = noref && (curr_ref == vidref);
            curr_ref->cur_imgY = curr_ref->imgY;
          }
        }
      }
      break;
#if 0
    case 1:
      for (j = 0; j < 6; j++) { //for (j = 0; j < (currSlice->slice_type==B_SLICE?2:1); j++) { //
        for (i = 0; i < MAX_LIST_SIZE; i++) {
          StorablePicture *curr_ref = currSlice->listX[j][i];
          if (curr_ref) {
            curr_ref->no_ref = noref && (curr_ref == vidref);
            curr_ref->cur_imgY = curr_ref->imgUV[0];
          }
        }
      }
      break;
    case 2:
      for (j = 0; j < 6; j++) { //for (j = 0; j < (currSlice->slice_type==B_SLICE?2:1); j++) { //
        for (i = 0; i < MAX_LIST_SIZE; i++) {
          StorablePicture *curr_ref = currSlice->listX[j][i];
          if (curr_ref) {
            curr_ref->no_ref = noref && (curr_ref == vidref);
            curr_ref->cur_imgY = curr_ref->imgUV[1];
          }
        }
      }
      break;
#endif
    }
  }
  else
  {
#if EXT3D
    StorablePicture *vidref = p_Vid->no_reference_picture[currSlice->is_depth];
#else
    StorablePicture *vidref = p_Vid->no_reference_picture;
#endif
    int noref = (currSlice->framepoc < p_Vid->recovery_poc);
    int total_lists = currSlice->mb_aff_frame_flag ? 6 : (currSlice->slice_type==B_SLICE ? 2 : 1);
    //    for (j = 0; j < 6; j++) {  //for (j = 0; j < (currSlice->slice_type==B_SLICE?2:1); j++) { 
    for (j = 0; j < total_lists; j++) 
    {
      // note that if we always set this to MAX_LIST_SIZE, we avoid crashes with invalid ref_idx being set
      // since currently this is done at the slice level, it seems safe to do so.
      // Note for some reason I get now a mismatch between version 12 and this one in cabac. I wonder why.
      //for (i = 0; i < currSlice->listXsize[j]; i++) 
      for (i = 0; i < MAX_LIST_SIZE; i++) 
      {
        StorablePicture *curr_ref = currSlice->listX[j][i];
        if (curr_ref) {
          curr_ref->no_ref = noref && (curr_ref == vidref);
          curr_ref->cur_imgY = curr_ref->imgY;
        }
      }
    }
  }
}



/*!
 ************************************************************************
 * \brief
 *    decodes one slice
 ************************************************************************
 */
void decode_one_slice(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  Boolean end_of_slice = FALSE;
  Macroblock *currMB = NULL;
  currSlice->cod_counter=-1;

  if( (p_Vid->separate_colour_plane_flag != 0) )
  {
    change_plane_JV( p_Vid, currSlice->colour_plane_id, currSlice );
  }
  else
  {
    currSlice->mb_data = p_Vid->mb_data;
    currSlice->dec_picture = p_Vid->dec_picture;
    currSlice->siblock = p_Vid->siblock;
    currSlice->ipredmode = p_Vid->ipredmode;
    currSlice->intra_block = p_Vid->intra_block;
  }

  if (currSlice->slice_type == B_SLICE)
  {
    compute_colocated(currSlice, currSlice->listX);
  }

  if (currSlice->slice_type != I_SLICE && currSlice->slice_type != SI_SLICE)
    init_cur_imgy(currSlice,p_Vid); 

  //reset_ec_flags(p_Vid);

  while (end_of_slice == FALSE) // loop over macroblocks
  {

#if TRACE
#if EXT3D
    fprintf(p_Dec->p_trace,"\n*********** POC: %i (I/P) MB: %i Slice: %i Type %d %2d**********\n", currSlice->ThisPOC, currSlice->current_mb_nr, currSlice->current_slice_nr, currSlice->slice_type, currSlice->partArr[0].de_cabac.Drange);
#else
    fprintf(p_Dec->p_trace,"\n*********** POC: %i (I/P) MB: %i Slice: %i Type %d **********\n", currSlice->ThisPOC, currSlice->current_mb_nr, currSlice->current_slice_nr, currSlice->slice_type);
#endif
#endif

    // Initializes the current macroblock
    start_macroblock(currSlice, &currMB);
    // Get the syntax elements from the NAL
    currSlice->read_one_macroblock(currMB);

    decode_one_macroblock(currMB, currSlice->dec_picture);

    if(currSlice->mb_aff_frame_flag && currMB->mb_field)
    {
      currSlice->num_ref_idx_active[LIST_0] >>= 1;
      currSlice->num_ref_idx_active[LIST_1] >>= 1;
    }

#if (DISABLE_ERC == 0)
    ercWriteMBMODEandMV(currMB);
#endif

    end_of_slice = exit_macroblock(currSlice, (!currSlice->mb_aff_frame_flag|| currSlice->current_mb_nr%2));
  }

  //reset_ec_flags(p_Vid);
}

#if MVC_EXTENSION_ENABLE||EXT3D
int GetVOIdx(VideoParameters *p_Vid, int iViewId)
{
#if EXT3D
  int iVOIdx = 0;
#else
  int iVOIdx = -1;
#endif
  int *piViewIdMap;
  if(p_Vid->active_subset_sps)
  {
    piViewIdMap = p_Vid->active_subset_sps->view_id;
    for(iVOIdx = p_Vid->active_subset_sps->num_views_minus1; iVOIdx>=0; iVOIdx--)
      if(piViewIdMap[iVOIdx] == iViewId)
        break;
  }

  return iVOIdx;
}


int get_maxViewIdx (VideoParameters *p_Vid, int view_id, int anchor_pic_flag, int listidx)
{
  int VOIdx;
  int maxViewIdx = 0;

  VOIdx = GetVOIdx(p_Vid, view_id);
  if(VOIdx >= 0)
  {
    if(anchor_pic_flag)
      maxViewIdx = listidx? p_Vid->active_subset_sps->num_anchor_refs_l1[VOIdx] : p_Vid->active_subset_sps->num_anchor_refs_l0[VOIdx];
    else
      maxViewIdx = listidx? p_Vid->active_subset_sps->num_non_anchor_refs_l1[VOIdx] : p_Vid->active_subset_sps->num_non_anchor_refs_l0[VOIdx];
  }

  return maxViewIdx;  
}


#if EXT3D
/*!
 ************************************************************************
 * \brief
 *    get the left reference id for view synthesized picture produced by Visbd
 ************************************************************************
 */
static int get_left_reference(Slice* pSlice,int curr_view_id)
{
  VideoParameters* p_Vid=pSlice->p_Vid;
  subset_seq_parameter_set_rbsp_t* active_subset_sps=p_Vid->active_subset_sps;

  int curr_voidx=GetVOIdx(p_Vid,curr_view_id);

  ThreeDVAcquisitionInfo* curr_3dv_acquisition=p_Vid->DepParSet[pSlice->dep_parameter_set_id]->acquisition_info;

  double min_view_distance=MAX_VALUE;
  int left_ref_view_id=-1;
  int i=0;
  int texture=(1^(pSlice->is_depth));

  for(i=0;i<=active_subset_sps->num_views_minus1;++i)
  {
    int encoded=0;
    if(texture)
    {
      if(p_Vid->decode_ok[0][i]&&p_Vid->decode_ok[1][i])
        encoded=1;
    }
    else
    {
      //!<view synthesis for depth
      if(p_Vid->decode_ok[1][i])
        encoded=1;
    }
    if((i==curr_voidx)||(encoded==0))
      continue;
    if((curr_3dv_acquisition->i_disparity_scale[curr_voidx][i]>0)
      &&abs(curr_3dv_acquisition->i_disparity_scale[curr_voidx][i])<min_view_distance)
    {
      min_view_distance=abs(curr_3dv_acquisition->i_disparity_scale[curr_voidx][i]);
      left_ref_view_id=active_subset_sps->view_id[i];
    }
  }
  return left_ref_view_id;
}

/*!
 ************************************************************************
 * \brief
 *    get the right reference id for view synthesized picture produced by Visbd
 ************************************************************************
 */
static int get_right_reference(Slice* pSlice,int curr_view_id)
{
  VideoParameters* p_Vid=pSlice->p_Vid;
  subset_seq_parameter_set_rbsp_t* active_subset_sps=p_Vid->active_subset_sps;

  int curr_voidx=GetVOIdx(p_Vid,curr_view_id);

  ThreeDVAcquisitionInfo* curr_3dv_acquisition=p_Vid->DepParSet[pSlice->dep_parameter_set_id]->acquisition_info;

  double min_view_distance=MAX_VALUE;
  int right_ref_view_id=-1;
  int i=0;
  int texture=(1^(pSlice->is_depth));


  for(i=0;i<=active_subset_sps->num_views_minus1;++i)
  {
    int encoded=0;
    if(texture)
    {
      if(p_Vid->decode_ok[0][i]&&p_Vid->decode_ok[1][i])
        encoded=1;
    }
    else
    {
      if(p_Vid->decode_ok[1][i])
        encoded=1;
    }
    if((i==curr_voidx)||(encoded==0))
      continue;

    if((curr_3dv_acquisition->i_disparity_scale[curr_voidx][i]<0)
      &&abs(curr_3dv_acquisition->i_disparity_scale[curr_voidx][i])<min_view_distance)
    {
      min_view_distance=abs(curr_3dv_acquisition->i_disparity_scale[curr_voidx][i]);
      right_ref_view_id=active_subset_sps->view_id[i];
    }
  }
  return right_ref_view_id;
}

/*!
 ************************************************************************
 * \brief
 *    Prepare the parameters for view synthesized picture produced by Visbd
 ************************************************************************
 */
static int prepare_viewsyn_picture(Slice* pSlice,int curr_view_id,
                  int left_ref_view_id,int right_ref_view_id)
{

  VideoParameters* p_Vid=pSlice->p_Vid;
  DecodedPictureBuffer* p_Dpb=p_Vid->p_Dpb[pSlice->is_depth];
  DecodedPictureBuffer* p_DualDpb=p_Vid->p_Dpb[1^(pSlice->is_depth)];

  FrameStore* fs=NULL;
  int is_texture=(1^pSlice->is_depth);
  unsigned int frm_number=pSlice->frame_num;
  int poc=pSlice->framepoc;
  unsigned int i=0;

  for(i=0;i<p_Dpb->used_size;++i)
  {
    fs=p_Dpb->fs[i];
    if((fs->frame_num==frm_number)&&(fs->poc==poc)&&(fs->view_id==left_ref_view_id))
    {
      p_Vid->left_refer_pic=fs->frame;
      if(!is_texture)
        p_Vid->left_refer_depth=fs->frame;
    }
    if((fs->frame_num==frm_number)&&(fs->poc==poc)&&(fs->view_id==right_ref_view_id))
    {
      p_Vid->right_refer_pic=fs->frame;
      if(!is_texture)
        p_Vid->right_refer_depth=fs->frame;
    }
  }
  if(is_texture)
  {
    for(i=0;i<p_DualDpb->used_size;++i)
    {
      fs=p_DualDpb->fs[i];
      if((fs->frame_num==frm_number)&&(fs->poc==poc)&&(fs->view_id==left_ref_view_id))
      {
        p_Vid->left_refer_depth=fs->frame;
      }
      if((fs->frame_num==frm_number)&&(fs->poc==poc)&&(fs->view_id==right_ref_view_id))
      {
        p_Vid->right_refer_depth=fs->frame;
      }
    }
  }

  return 1;
}

int get_viewsyn_picture(Slice* pSlice)
{
  VideoParameters* p_Vid=pSlice->p_Vid;
  int curr_view_id=pSlice->view_id;
  int curr_voidx=GetVOIdx(p_Vid,curr_view_id);

  int left_ref_view_id=get_left_reference(pSlice,curr_view_id);
  int right_ref_view_id=get_right_reference(pSlice,curr_view_id);

  p_Vid->left_refer_pic=NULL;
  p_Vid->right_refer_pic=NULL;

  assert((left_ref_view_id!=-1)||(right_ref_view_id!=-1));

  if(left_ref_view_id==-1)
  {
    left_ref_view_id=right_ref_view_id;
  }
  else if(right_ref_view_id==-1)
  {
    right_ref_view_id=left_ref_view_id;
  }

  prepare_viewsyn_picture(pSlice,curr_view_id,left_ref_view_id,right_ref_view_id);

  p_Vid->vs_ok[pSlice->is_depth][curr_voidx]=1;

  return 1;
}
#endif

#endif

