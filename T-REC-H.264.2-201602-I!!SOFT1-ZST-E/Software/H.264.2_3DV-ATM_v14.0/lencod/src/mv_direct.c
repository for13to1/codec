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
 * \file mv_direct.c
 *
 * \brief
 *    Direct Motion Vector Generation
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Alexis Michael Tourapis         <alexismt@ieee.org>
 *
 *************************************************************************************
*/

#include "contributors.h"

#include <math.h>
#include <limits.h>
#include <time.h>



#include "global.h"



#include "image.h"
#include "mv_search.h"
#include "refbuf.h"
#include "memalloc.h"
#include "mb_access.h"
#include "macroblock.h"
#include "mc_prediction.h"
#include "conformance.h"
#include "mode_decision.h"

#if EXT3D
#include "configfile.h"
#endif

/*!
 ************************************************************************
 * \brief
 *    Calculate Temporal Direct Mode Motion Vectors
 ************************************************************************
 */
void Get_Direct_MV_Temporal (Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice; 
  int   block_x, block_y, pic_block_x, pic_block_y, opic_block_x, opic_block_y;
  MotionVector *****all_mvs;
  int   mv_scale;
  int refList;
  int ref_idx;
  VideoParameters *p_Vid = currMB->p_Vid;
  int list_offset = currMB->list_offset;

  StorablePicture **list1 = currSlice->listX[LIST_1 + list_offset];

  PicMotionParams colocated;

  //temporal direct mode copy from decoder
  for (block_y = 0; block_y < 4; block_y++)
  {
    pic_block_y  = currMB->block_y + block_y;
    opic_block_y = (currMB->opix_y >> 2) + block_y;

    for (block_x = 0; block_x < 4; block_x++)
    {
      pic_block_x  = currMB->block_x + block_x;
      opic_block_x = (currMB->pix_x>>2) + block_x;

      all_mvs = currSlice->all_mv;
      if (p_Vid->active_sps->direct_8x8_inference_flag)
      {
        colocated = list1[0]->mv_info[RSD(opic_block_y)][RSD(opic_block_x)];
        if(currSlice->mb_aff_frame_flag && currMB->mb_field && currSlice->listX[LIST_1][0]->coded_frame)
        {
          int iPosBlkY;
          if(currSlice->listX[LIST_1][0]->motion.mb_field[currMB->mbAddrX] )
            iPosBlkY = (RSD(opic_block_y)>>2)*8+4*(currMB->mbAddrX&1);
          else
            iPosBlkY = RSD(opic_block_y)*2;

          if(colocated.ref_idx[LIST_0]>=0)
            colocated.ref_pic[LIST_0] = list1[0]->frame->mv_info[iPosBlkY][RSD(opic_block_x)].ref_pic[LIST_0];
          if(colocated.ref_idx[LIST_1]>=0)
            colocated.ref_pic[LIST_1] = list1[0]->frame->mv_info[iPosBlkY][RSD(opic_block_x)].ref_pic[LIST_1];
        }        
      }
      else
        colocated = list1[0]->mv_info[opic_block_y][opic_block_x];
      if(currSlice->mb_aff_frame_flag)
      {
        if(!currMB->mb_field && ((currSlice->listX[LIST_1][0]->coded_frame && currSlice->listX[LIST_1][0]->motion.mb_field[currMB->mbAddrX]) ||
          (!currSlice->listX[LIST_1][0]->coded_frame)))
        {
          if (iabs(p_Vid->enc_picture->poc - currSlice->listX[LIST_1+4][0]->poc)> iabs(p_Vid->enc_picture->poc -currSlice->listX[LIST_1+2][0]->poc) )
          {
            colocated = p_Vid->active_sps->direct_8x8_inference_flag ? 
              currSlice->listX[LIST_1+2][0]->mv_info[RSD(opic_block_y)>>1][RSD(opic_block_x)] : currSlice->listX[LIST_1+2][0]->mv_info[(opic_block_y)>>1][opic_block_x];
            if(currSlice->listX[LIST_1][0]->coded_frame)
            {
              int iPosBlkY = (RSD(opic_block_y)>>3)*8 + ((RSD(opic_block_y)>>1) & 0x03);
              if(colocated.ref_idx[LIST_0] >=0) // && !colocated.ref_pic[LIST_0])
                colocated.ref_pic[LIST_0] = currSlice->listX[LIST_1+2][0]->frame->mv_info[iPosBlkY][RSD(opic_block_x)].ref_pic[LIST_0];
              if(colocated.ref_idx[LIST_1] >=0) // && !colocated.ref_pic[LIST_1])
                colocated.ref_pic[LIST_1] = currSlice->listX[LIST_1+2][0]->frame->mv_info[iPosBlkY][RSD(opic_block_x)].ref_pic[LIST_1];
            }
          }
          else
          {
            colocated = p_Vid->active_sps->direct_8x8_inference_flag ? 
              currSlice->listX[LIST_1+4][0]->mv_info[RSD(opic_block_y)>>1][RSD(opic_block_x)] : currSlice->listX[LIST_1+4][0]->mv_info[(opic_block_y)>>1][opic_block_x];
            if(currSlice->listX[LIST_1][0]->coded_frame)
            {
              int iPosBlkY = (RSD(opic_block_y)>>3)*8 + ((RSD(opic_block_y)>>1) & 0x03)+4;
              if(colocated.ref_idx[LIST_0] >=0) // && !colocated.ref_pic[LIST_0])
                colocated.ref_pic[LIST_0] = currSlice->listX[LIST_1+4][0]->frame->mv_info[iPosBlkY][RSD(opic_block_x)].ref_pic[LIST_0];
              if(colocated.ref_idx[LIST_1] >=0)// && !colocated.ref_pic[LIST_1])
                colocated.ref_pic[LIST_1] = currSlice->listX[LIST_1+4][0]->frame->mv_info[iPosBlkY][RSD(opic_block_x)].ref_pic[LIST_1];
            }
          }
        }
      }
      else if(!p_Vid->active_sps->frame_mbs_only_flag && !p_Vid->structure && !currSlice->listX[LIST_1][0]->coded_frame)
      {
        if (iabs(p_Vid->enc_picture->poc - list1[0]->bottom_field->poc)> iabs(p_Vid->enc_picture->poc -list1[0]->top_field->poc) )
        {
          colocated = p_Vid->active_sps->direct_8x8_inference_flag ? 
            list1[0]->top_field->mv_info[RSD(opic_block_y)>>1][RSD(opic_block_x)] : list1[0]->top_field->mv_info[(opic_block_y)>>1][opic_block_x];
        }
        else
        {
          colocated = p_Vid->active_sps->direct_8x8_inference_flag ? 
            list1[0]->bottom_field->mv_info[RSD(opic_block_y)>>1][RSD(opic_block_x)] : list1[0]->bottom_field->mv_info[(opic_block_y)>>1][opic_block_x];
        }
      }
      else if(!p_Vid->active_sps->frame_mbs_only_flag && p_Vid->structure && list1[0]->coded_frame)
      {
        int iPosBlkY; 
        int currentmb = 2*(list1[0]->size_x>>4) * (opic_block_y >> 2)+ (opic_block_x>>2)*2 + ((opic_block_y>>1) & 0x01);
        if(p_Vid->structure!=list1[0]->structure)
        {
          if (p_Vid->structure == TOP_FIELD)
          {
            colocated = p_Vid->active_sps->direct_8x8_inference_flag ? 
              list1[0]->frame->top_field->mv_info[RSD(opic_block_y)][RSD(opic_block_x)] : list1[0]->frame->top_field->mv_info[opic_block_y][opic_block_x];
          }
          else
          {
            colocated = p_Vid->active_sps->direct_8x8_inference_flag ? 
              list1[0]->frame->bottom_field->mv_info[RSD(opic_block_y)][RSD(opic_block_x)] : list1[0]->frame->bottom_field->mv_info[opic_block_y][opic_block_x];
          }
        }

        if(!currSlice->listX[LIST_1][0]->frame->mb_aff_frame_flag || !list1[0]->frame->motion.mb_field[currentmb])
          iPosBlkY = 2*(RSD(opic_block_y));
        else
          iPosBlkY = (RSD(opic_block_y)>>2)*8 + (RSD(opic_block_y) & 0x03)+4*(p_Vid->structure == BOTTOM_FIELD);
        if(colocated.ref_idx[LIST_0] >=0) // && !colocated.ref_pic[LIST_0])
          colocated.ref_pic[LIST_0] = list1[0]->frame->mv_info[iPosBlkY][RSD(opic_block_x)].ref_pic[LIST_0];
        if(colocated.ref_idx[LIST_1] >=0)// && !colocated.ref_pic[LIST_1])
          colocated.ref_pic[LIST_1] = list1[0]->frame->mv_info[iPosBlkY][RSD(opic_block_x)].ref_pic[LIST_1];
      }

      refList = (colocated.ref_idx[LIST_0] == -1 ? LIST_1 : LIST_0);
      ref_idx = colocated.ref_idx[refList];

      // next P is intra mode
      if (ref_idx == -1)
      {
        all_mvs[LIST_0][0][0][block_y][block_x] = zero_mv;
        all_mvs[LIST_1][0][0][block_y][block_x] = zero_mv;
        currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_0] = 0;
        currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_1] = 0;
        currSlice->direct_pdir[pic_block_y][pic_block_x] = 2;
      }
      // next P is skip or inter mode
      else
      {
        int mapped_idx=INVALIDINDEX;
        int iref;
        if( (currSlice->mb_aff_frame_flag && ( (currMB->mb_field && colocated.ref_pic[refList]->structure==FRAME) || 
          (!currMB->mb_field && colocated.ref_pic[refList]->structure!=FRAME))) ||
          (!currSlice->mb_aff_frame_flag && ((p_Vid->structure==FRAME && colocated.ref_pic[refList]->structure!=FRAME)||
          (p_Vid->structure!=FRAME && colocated.ref_pic[refList]->structure==FRAME))) )
        {
          //! Frame with field co-located
          for (iref = 0; iref < imin(currSlice->num_ref_idx_active[LIST_0], currSlice->listXsize[LIST_0 + list_offset]); iref++)
          {
            if (currSlice->listX[LIST_0 + list_offset][iref]->top_field == colocated.ref_pic[refList] ||
              currSlice->listX[LIST_0 + list_offset][iref]->bottom_field == colocated.ref_pic[refList] ||
              currSlice->listX[LIST_0 + list_offset][iref]->frame == colocated.ref_pic[refList] ) 
            {
              mapped_idx=iref;
              break;
            }
            else //! invalid index. Default to zero even though this case should not happen
              mapped_idx=INVALIDINDEX;
          }
        }
        else
        {
        for (iref = 0; iref < imin(currSlice->num_ref_idx_active[LIST_0], currSlice->listXsize[LIST_0 + list_offset]);iref++)
        {
          if(currSlice->listX[LIST_0 + list_offset][iref] == colocated.ref_pic[refList])
          {
            mapped_idx = iref;            
            break;
          }
          else //! invalid index. Default to zero even though this case should not happen
          {
            mapped_idx=INVALIDINDEX;
          }
        }
        }

        if (mapped_idx != INVALIDINDEX)
        {
          MotionVector mv = colocated.mv[refList];
          mv_scale = currSlice->mvscale[LIST_0 + list_offset][mapped_idx];

          if((currSlice->mb_aff_frame_flag && !currMB->mb_field && colocated.ref_pic[refList]->structure!=FRAME) ||
            (!currSlice->mb_aff_frame_flag && p_Vid->structure==FRAME && colocated.ref_pic[refList]->structure!=FRAME))
            mv.mv_y *= 2;
          else if((currSlice->mb_aff_frame_flag && currMB->mb_field && colocated.ref_pic[refList]->structure==FRAME) ||
            (!currSlice->mb_aff_frame_flag && p_Vid->structure!=FRAME && colocated.ref_pic[refList]->structure==FRAME))
            mv.mv_y /= 2;

          if (mv_scale==9999)
          {
            // forward
            all_mvs[LIST_0][0][0][block_y][block_x] = mv;
            // backward
            all_mvs[LIST_1][0][0][block_y][block_x] = zero_mv;
          }
          else
          {
            // forward
            all_mvs[LIST_0][mapped_idx][0][block_y][block_x].mv_x = (short) ((mv_scale * mv.mv_x + 128) >> 8);
            all_mvs[LIST_0][mapped_idx][0][block_y][block_x].mv_y = (short) ((mv_scale * mv.mv_y + 128) >> 8);
            // backward
            all_mvs[LIST_1][         0][0][block_y][block_x].mv_x = (short) (((mv_scale - 256) * mv.mv_x + 128) >> 8);
            all_mvs[LIST_1][         0][0][block_y][block_x].mv_y = (short) (((mv_scale - 256) * mv.mv_y + 128) >> 8);

          }

          // Test Level Limits if satisfied.
          if ( out_of_bounds_mvs(p_Vid, &all_mvs[LIST_0][mapped_idx][0][block_y][block_x])|| out_of_bounds_mvs(p_Vid, &all_mvs[LIST_1][0][0][block_y][block_x]))
          {
            currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_0] = -1;
            currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_1] = -1;
            currSlice->direct_pdir[pic_block_y][pic_block_x] = -1;
          }
          else
          {
            currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_0] = (char) mapped_idx;
            currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_1] = 0;
            currSlice->direct_pdir[pic_block_y][pic_block_x] = 2;
          }
        }
        else
        {
          currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_0] = -1;
          currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_1] = -1;
          currSlice->direct_pdir[pic_block_y][pic_block_x] = -1;
        }
      }

#if EXT3D
    if(((p_Vid->active_pps->weighted_bipred_idc == 1)||(currSlice->depth_range_wp))
      && currSlice->direct_pdir[pic_block_y][pic_block_x] == 2)
#else
      if (p_Vid->active_pps->weighted_bipred_idc == 1 && currSlice->direct_pdir[pic_block_y][pic_block_x] == 2)
#endif
      {
        int weight_sum, i;
        short l0_refX = currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_0];
        short l1_refX = currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_1];
        for (i=0;i< (p_Vid->active_sps->chroma_format_idc == YUV400 ? 1 : 3); i++)
        {
          weight_sum = currSlice->wbp_weight[0][l0_refX][l1_refX][i] + currSlice->wbp_weight[1][l0_refX][l1_refX][i];
          if (weight_sum < -128 ||  weight_sum > 127)
          {
            currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_0] = -1;
            currSlice->direct_ref_idx[pic_block_y][pic_block_x][LIST_1] = -1;
            currSlice->direct_pdir   [pic_block_y][pic_block_x]         = -1;
            break;
          }
        }
      }
    }
  }
}


static inline void set_direct_references(const PixelPos *mb, char *l0_rFrame, char *l1_rFrame, PicMotionParams **mv_info)
{
  if (mb->available)
  {
    char *ref_idx = mv_info[mb->pos_y][mb->pos_x].ref_idx;
    *l0_rFrame  = ref_idx[LIST_0];
    *l1_rFrame  = ref_idx[LIST_1];
  }
  else
  {
    *l0_rFrame  = -1;
    *l1_rFrame  = -1;
  }
}

static void set_direct_references_mb_field(const PixelPos *mb, char *l0_rFrame, char *l1_rFrame, PicMotionParams **mv_info, Macroblock *mb_data)
{
  if (mb->available)
  {
    char *ref_idx = mv_info[mb->pos_y][mb->pos_x].ref_idx;
    if (mb_data[mb->mb_addr].mb_field)
    {
      *l0_rFrame  = ref_idx[LIST_0];
      *l1_rFrame  = ref_idx[LIST_1];
    }
    else
    {
      *l0_rFrame  = (ref_idx[LIST_0] < 0) ? ref_idx[LIST_0] : ref_idx[LIST_0] * 2;
      *l1_rFrame  = (ref_idx[LIST_1] < 0) ? ref_idx[LIST_1] : ref_idx[LIST_1] * 2;
    }
  }
  else
  {
    *l0_rFrame  = -1;
    *l1_rFrame  = -1;
  }
}

static void set_direct_references_mb_frame(const PixelPos *mb, char *l0_rFrame, char *l1_rFrame, PicMotionParams **mv_info, Macroblock *mb_data)
{
  if (mb->available)
  {
    char *ref_idx = mv_info[mb->pos_y][mb->pos_x].ref_idx;
    if (mb_data[mb->mb_addr].mb_field)
    {
      *l0_rFrame  = (ref_idx[LIST_0] >> 1);
      *l1_rFrame  = (ref_idx[LIST_1] >> 1);
    }
    else
    {
      *l0_rFrame  = ref_idx[LIST_0];
      *l1_rFrame  = ref_idx[LIST_1];
    }
  }
  else
  {
    *l0_rFrame  = -1;
    *l1_rFrame  = -1;
  }
}

static void test_valid_direct(Slice *currSlice, seq_parameter_set_rbsp_t *active_sps, char  *direct_ref_idx, short l0_refX, short l1_refX, int pic_block_y, int pic_block_x)
{
  int weight_sum, i;
  Boolean invalid_wp = FALSE;
  for (i=0;i< (active_sps->chroma_format_idc == YUV400 ? 1 : 3); i++)
  {
    weight_sum = currSlice->wbp_weight[0][l0_refX][l1_refX][i] + currSlice->wbp_weight[1][l0_refX][l1_refX][i];
    if (weight_sum < -128 ||  weight_sum > 127)
    {
      invalid_wp = TRUE;
      break;
    }
  }
  if (invalid_wp == FALSE)
    currSlice->direct_pdir[pic_block_y][pic_block_x] = 2;
  else
  {
    direct_ref_idx[LIST_0] = -1;
    direct_ref_idx[LIST_1] = -1;
    currSlice->direct_pdir[pic_block_y][pic_block_x] = -1;
  }
}

/*!
*************************************************************************************
* \brief
*    Temporary function for colocated info when direct_inference is enabled. 
*
*************************************************************************************
*/
int get_colocated_info(Macroblock *currMB, StorablePicture *list1, int i, int j)
{
  if (list1->is_long_term)
    return 1;
  else
  {
    Slice *currSlice = currMB->p_Slice;
    VideoParameters *p_Vid = currMB->p_Vid;
    if( (currSlice->mb_aff_frame_flag) ||
      (!p_Vid->active_sps->frame_mbs_only_flag && ((!currSlice->structure && !list1->coded_frame) || (currSlice->structure!=list1->structure && list1->coded_frame))))
    {
      int jj = RSD(j);
      int ii = RSD(i);
      int jdiv = (jj>>1);
      int moving;
      PicMotionParams *fs = &list1->mv_info[jj][ii];

      if(currSlice->structure && currSlice->structure!=list1->structure && list1->coded_frame)
      {
         if(currSlice->structure == TOP_FIELD)
           fs = list1->top_field->mv_info[jj] + ii;
         else
           fs = list1->bottom_field->mv_info[jj] + ii;
      }
      else
      {
        if( (currSlice->mb_aff_frame_flag && ((!currMB->mb_field && list1->motion.mb_field[currMB->mbAddrX]) ||
          (!currMB->mb_field && !list1->coded_frame))) 
          || (!currSlice->mb_aff_frame_flag))
        {
          if (iabs(p_Vid->enc_picture->poc - list1->bottom_field->poc)> iabs(p_Vid->enc_picture->poc -list1->top_field->poc) )
          {
            fs = list1->top_field->mv_info[jdiv] + ii;
          }
          else
          {
            fs = list1->bottom_field->mv_info[jdiv] + ii;
          }
        }
      }
      moving = !((((fs->ref_idx[LIST_0] == 0)
        &&  (iabs(fs->mv[LIST_0].mv_x)>>1 == 0)
        &&  (iabs(fs->mv[LIST_0].mv_y)>>1 == 0)))
        || ((fs->ref_idx[LIST_0] == -1)
        &&  (fs->ref_idx[LIST_1] == 0)
        &&  (iabs(fs->mv[LIST_1].mv_x)>>1 == 0)
        &&  (iabs(fs->mv[LIST_1].mv_y)>>1 == 0)));
      return moving;
    }
    else
    {
      PicMotionParams *fs = &list1->mv_info[RSD(j)][RSD(i)];
      int moving;
      if(currMB->p_Vid->yuv_format == YUV444 && !currSlice->P444_joined)
        fs = &list1->JVmv_info[(int)(p_Vid->colour_plane_id)][RSD(j)][RSD(i)];
      moving= !((((fs->ref_idx[LIST_0] == 0)
        &&  (iabs(fs->mv[LIST_0].mv_x)>>1 == 0)
        &&  (iabs(fs->mv[LIST_0].mv_y)>>1 == 0)))
        || ((fs->ref_idx[LIST_0] == -1)
        &&  (fs->ref_idx[LIST_1] == 0)
        &&  (iabs(fs->mv[LIST_1].mv_x)>>1 == 0)
        &&  (iabs(fs->mv[LIST_1].mv_y)>>1 == 0)));

      return moving;  
    }
  }
}

/*!
*************************************************************************************
* \brief
*    Colocated info <= direct_inference is disabled. 
*************************************************************************************
*/
int get_colocated_info_4x4(Macroblock *currMB, StorablePicture *list1, int i, int j)
{
  UNREFERENCED_PARAMETER(currMB);
  if (list1->is_long_term)
    return 1;
  else
  {
    PicMotionParams *fs = &list1->mv_info[j][i];

    int moving = !((((fs->ref_idx[LIST_0] == 0)
      &&  (iabs(fs->mv[LIST_0].mv_x)>>1 == 0)
      &&  (iabs(fs->mv[LIST_0].mv_y)>>1 == 0)))
      || ((fs->ref_idx[LIST_0] == -1)
      &&  (fs->ref_idx[LIST_1] == 0)
      &&  (iabs(fs->mv[LIST_1].mv_x)>>1 == 0)
      &&  (iabs(fs->mv[LIST_1].mv_y)>>1 == 0)));

    return moving;  
  }
}

#if EXT3D
int get_nbdv_temporal( Macroblock * currMB, short *disparity )
{
  int    curr_viewid = currMB->p_Slice->view_id;
  Slice* currSlice   = currMB->p_Slice;
  StorablePicture* colPic;
  int    i;

  for( i = 0; i < ( currSlice->slice_type == B_SLICE ? 2 : 1 ); i++ )
  {
    int list = currSlice->slice_type == B_SLICE ? 1-i : 0;
    colPic   = currSlice->listX[list][0];

    if( colPic != NULL )
    {
      if( colPic->size_x >= currMB->pix_x + BLOCK_SIZE + MB_BLOCK_SIZE && colPic->size_y >= currMB->pix_y + BLOCK_SIZE + MB_BLOCK_SIZE ) 
      {
        int opic_block_y = currMB->block_y + 4;
        int opic_block_x = currMB->block_x + 4;
        PicMotionParams temporal_mvinfo = colPic->mv_info[opic_block_y][opic_block_x];
        int ref_viewid = temporal_mvinfo.ref_idx[LIST_0] >=0 ? temporal_mvinfo.ref_pic[LIST_0]->view_id : -1;

        if( ref_viewid != -1 && ref_viewid != curr_viewid )
        {
          if( temporal_mvinfo.mv[LIST_0].mv_x != 0 )
          {        
            *disparity = temporal_mvinfo.mv[LIST_0].mv_x;
            return 1; 
          }
        }
      }
    }    
  }
  return 0;
}

int get_nbdv_spatial( Macroblock *currMB, short *disparity )
{
  int i;
  PixelPos spatial_cand; 
  int neighbor_ref_idx = -1;
  int neighbor_view_id = -1;
  Slice* currSlice          = currMB->p_Slice;
  PicMotionParams** mv_info = currMB->p_Vid->enc_picture->mv_info;
  int *mb_size = currMB->p_Vid->mb_size[IS_LUMA];
  const int spatial_cand_pos[4][2] = { { -1, 0 },{ 0, -1 },{ MB_BLOCK_SIZE, -1 },{ -1, -1 } };

  for( i = 0; i < 4; i++ ) 
  {
    spatial_cand.available = FALSE;
    get4x4Neighbour(currMB, spatial_cand_pos[i][0], spatial_cand_pos[i][1], mb_size, &spatial_cand );
    if( spatial_cand.available )
    {
      PicMotionParams spatial_mvinfo = mv_info[spatial_cand.pos_y][spatial_cand.pos_x];
      
      neighbor_ref_idx = spatial_mvinfo.ref_idx[LIST_0];

      if( neighbor_ref_idx < 0 || spatial_mvinfo.ref_pic[LIST_0] == NULL ) continue;

      neighbor_view_id = spatial_mvinfo.ref_pic[LIST_0]->view_id;

      if( neighbor_view_id >=0 && currSlice->view_id != neighbor_view_id )
      {
        if( spatial_mvinfo.mv[LIST_0].mv_x ) 
        {
          *disparity = spatial_mvinfo.mv[LIST_0].mv_x;
          return 1;
        }
      }
    }
  }
  return 0;
}

/*!
************************************************************************
* \brief
*    Derive the Inter-view MV for temporal prediction
************************************************************************
*/
void Get_InterViewMV_TemporalMVP(Macroblock *currMB, int mb_x, int mb_y, int offset_x, int offset_y, int list_idx, int view_id, MotionInfo *motion_interview, PixelPos *neighbors)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  //DecodedPictureBuffer *p_Dpb = p_Vid->p_Dpb;
  Slice *currSlice = currMB->p_Slice;
  PicMotionParams **motion = NULL;
  imgpel **p_DepthY;
  int point_x, point_y;
  int disparity_x, disparity_y;
  int pos_x, pos_y;
  //int ref_idx, ref_view_id;
  int i; // , k = -1;
  int hasinterview=0;

  UNREFERENCED_PARAMETER(neighbors);
  UNREFERENCED_PARAMETER(mb_x);
  UNREFERENCED_PARAMETER(mb_y);

  point_x = currMB->pix_x + ( ( offset_x ) >> 1 );
  point_y = currMB->pix_y + ( ( offset_y ) >> 1 );

  motion_interview[list_idx].ref_idx = -1;
  motion_interview[list_idx].mv.mv_x = 0;
  motion_interview[list_idx].mv.mv_y = 0;  

  //view_id==currMB->p_Inp->ViewCodingOrder[0]
  if( currSlice->view_id==view_id) 
  {
    return;
  }

  //find corresponding interview reference
  for( i = 0; i < currSlice->listXsize[0]; i++ )
  {
    if( ( currSlice->ThisPOC == currSlice->listX[0][i]->frame->poc ) && ( view_id == currSlice->listX[0][i]->frame->view_id) )
    {
      motion = currSlice->listX[0][i]->frame->mv_info;
      hasinterview=1;
      break;
    }
  }

  if( p_Vid->isTextureFirst )
  {
    hasinterview=currSlice->hasInterviewRef;
  }

  if(hasinterview==0)
  {
    return;
  }

  if(currSlice->is_depth)
  {
    p_DepthY = p_Vid->enc_picture->imgY;
  }
  else
  {
    p_DepthY = p_Vid->isTextureFirst ? NULL : p_Vid->p_dual_picture->imgY;  
  }

  if(currSlice->is_depth)
  {
    disparity_x = 0;
    disparity_y = 0;
  }
  else
  {  
    int j, start_y, start_x, end_y, end_x, blk_step_y, blk_step_x, dmax = 0;
    if(p_Vid->isTextureFirst)
    {
      disparity_x = currMB->iNBDV;
    }
    else
    {
      int curr_view_id=currMB->p_Slice->view_id;
      int grid_posx=p_Vid->grid_pos_x[curr_view_id];
      int grid_posy=p_Vid->grid_pos_y[curr_view_id];

      start_y = iClip3(p_Vid->DepthCropTopCoord,p_Vid->DepthCropBottomCoord,( currMB->pix_y +grid_posy) *p_Vid->depth_ver_mult>>p_Vid->depth_ver_rsh);
      start_x = iClip3(p_Vid->DepthCropLeftCoord,p_Vid->DepthCropRightCoord,( currMB->pix_x +grid_posx) *p_Vid->depth_hor_mult>>p_Vid->depth_hor_rsh);
      end_y   =  iClip3(p_Vid->DepthCropTopCoord,p_Vid->DepthCropBottomCoord,( currMB->pix_y +grid_posy+15) *p_Vid->depth_ver_mult>>p_Vid->depth_ver_rsh);
      end_x   = iClip3(p_Vid->DepthCropLeftCoord,p_Vid->DepthCropRightCoord,( currMB->pix_x +grid_posx+15) *p_Vid->depth_hor_mult>>p_Vid->depth_hor_rsh);

      blk_step_y = end_y-start_y;
      blk_step_x = end_x-start_x;

      for( j = start_y; j <=end_y; j += blk_step_y ) 
      {
        for( i = start_x; i <= end_x; i += blk_step_x ) 
        {
          if( p_DepthY[j][i] > dmax) dmax = p_DepthY[j][i];
          if (blk_step_x == 0)
            i++;
        }
        if (blk_step_y == 0)
          j++;
      }

      disparity_x = p_Vid->forward_disparity_table[dmax];
    }
    disparity_y = 0;
  }

  pos_x = iClip3( 0, p_Vid->width_blk - 1, ( point_x + ( disparity_x >> 2 ) ) >> 2 );
  pos_y = iClip3( 0, p_Vid->height_blk - 1, ( point_y + ( disparity_y >> 2 ) ) >> 2 );

  motion_interview[list_idx].ref_idx = motion[pos_y][pos_x].ref_idx[list_idx];
  motion_interview[list_idx].mv.mv_x = motion[pos_y][pos_x].mv[list_idx].mv_x;
  motion_interview[list_idx].mv.mv_y = motion[pos_y][pos_x].mv[list_idx].mv_y;  
}

/*!
************************************************************************
* \brief
*    Derive the Inter-view MVP for skip and direct mode
************************************************************************
*/
void get_inter_view_MV(Macroblock *currMB, int offset_x, int offset_y, int list_idx, int view_id, MotionInfo *motion_interview, PixelPos *neighbors)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  //DecodedPictureBuffer *p_Dpb = p_Vid->p_Dpb;
  Slice *currSlice = currMB->p_Slice;
  PicMotionParams **motion = NULL;
  imgpel **p_DepthY;
  int point_x = currMB->pix_x + offset_x;
  int point_y = currMB->pix_y + offset_y;
  int disparity_x, disparity_y;
  int pos_x, pos_y;
  int i; // k = -1;
  int hasinterview=0;

  int y, x, start_y, start_x, end_y, end_x, dmax = 0;

  int blk_step_y,blk_step_x;
  int curr_view_id=currMB->p_Slice->view_id;
  int grid_posx=p_Vid->grid_pos_x[curr_view_id];
  int grid_posy=p_Vid->grid_pos_y[curr_view_id];

  motion_interview[list_idx].ref_idx = -1;
  motion_interview[list_idx].mv.mv_x = 0;
  motion_interview[list_idx].mv.mv_y = 0;  

  //view_id==currMB->p_Inp->ViewCodingOrder[0]
  if( currSlice->view_id==view_id) 
  {
    return;
  }

  if( p_Vid->isTextureFirst )
  {
    hasinterview = currSlice->hasInterviewRef;
    if( hasinterview )
    {
      int list, voidx;
      for( list = 0; list < ( currSlice->slice_type == B_SLICE ? 2 : 1 ); list++)
      {
        for( i = 0; i < currSlice->listXsize[list]; i++ )
        {
          voidx = GetVOIdx(p_Vid->p_Inp,currSlice->listX[list][i]->view_id);
          if( voidx==0 )
          {
            motion = currSlice->listX[list][i]->frame->mv_info;
            break;
          }
        }
      }
    }
  }
  else
  {
    //find corresponding interview reference
    for( i = 0; i < currSlice->listXsize[0]; i++ )
    {
      if( ( currSlice->ThisPOC == currSlice->listX[0][i]->frame->poc ) && ( view_id == currSlice->listX[0][i]->frame->view_id) )
      {
        motion = currSlice->listX[0][i]->frame->mv_info;
        hasinterview=1;
        break;
      }
    }
  }

  if(hasinterview==0)
  {
    return;
  }

  if(currSlice->is_depth)
  {
    p_DepthY = p_Vid->enc_picture->imgY;
  }
  else
  {
    p_DepthY = p_Vid->isTextureFirst ? NULL : p_Vid->p_dual_picture->imgY;
  }


  if(p_Vid->isTextureFirst)
  {
    disparity_x = currMB->iNBDV;
    disparity_y = 0;
  }
  else
  {
    start_y = iClip3(p_Vid->DepthCropTopCoord,p_Vid->DepthCropBottomCoord,( currMB->pix_y +grid_posy) *p_Vid->depth_ver_mult>>p_Vid->depth_ver_rsh);
    start_x = iClip3(p_Vid->DepthCropLeftCoord,p_Vid->DepthCropRightCoord,( currMB->pix_x +grid_posx) *p_Vid->depth_hor_mult>>p_Vid->depth_hor_rsh); 
    end_y   = iClip3(p_Vid->DepthCropTopCoord,p_Vid->DepthCropBottomCoord,( currMB->pix_y +grid_posy+15) *p_Vid->depth_ver_mult>>p_Vid->depth_ver_rsh);
    end_x   = iClip3(p_Vid->DepthCropLeftCoord,p_Vid->DepthCropRightCoord,( currMB->pix_x +grid_posx+15) *p_Vid->depth_hor_mult>>p_Vid->depth_hor_rsh); 

    blk_step_y = end_y-start_y;  
    blk_step_x = end_x-start_x; 

    for( y = start_y; y <= end_y; y += blk_step_y ) 
    {
      for( x = start_x; x <= end_x; x += blk_step_x ) 
      {
        if( p_DepthY[y][x] > dmax) dmax = p_DepthY[y][x];
        if (blk_step_x == 0)
          x++;
      }
      if (blk_step_y==0)
        y++;
    }
  disparity_x = p_Vid->forward_disparity_table[dmax];
  disparity_y = 0;
  }

  pos_x = iClip3( 0, p_Vid->width_blk - 1, ( point_x + ( disparity_x >> 2 ) ) >> 2 );
  pos_y = iClip3( 0, p_Vid->height_blk - 1, ( point_y + ( disparity_y >> 2 ) ) >> 2 );

  if(motion[pos_y][pos_x].ref_idx[list_idx] >= currSlice->listXsize[list_idx]) 
  {
    return;
  }

  motion_interview[list_idx].ref_idx = motion[pos_y][pos_x].ref_idx[list_idx];
  motion_interview[list_idx].mv.mv_x = motion[pos_y][pos_x].mv[list_idx].mv_x;
  motion_interview[list_idx].mv.mv_y = motion[pos_y][pos_x].mv[list_idx].mv_y;  
}

/*!
************************************************************************
* \brief
*    Get motion vector predictor for direct mode
************************************************************************
*/
void prepare_mvp_direct_params(Macroblock *currMB)
{
  //int i,list_idx;
  PixelPos mb[4];
  VideoParameters *p_Vid = currMB->p_Vid;
  PicMotionParams **motion = p_Vid->enc_picture->mv_info;
  Boolean mvp_available=FALSE;
  MotionInfo motion_interview[2];
  int view_id;

  char l0_refX, l1_refX;

  get_neighbors(currMB, mb, 0, 0, 16);
  currMB->best_ref[0]=currMB->best_ref[1]=-1;
  currMB->best_mvp[0]=currMB->best_mvp[1]=zero_mv;
  view_id = currMB->p_Inp->ViewCodingOrder[0];

  get_inter_view_MV(currMB, 7, 7, LIST_0, view_id, motion_interview, mb);
  if( motion_interview[LIST_0].ref_idx != -1 )
  {
    currMB->best_ref[0]=motion_interview[LIST_0].ref_idx;
    currMB->best_mvp[0]=motion_interview[LIST_0].mv;
    mvp_available=TRUE;
  }

  get_inter_view_MV(currMB, 7, 7, LIST_1, view_id, motion_interview, mb);
  if( motion_interview[LIST_1].ref_idx != -1 )
  {
    currMB->best_ref[1]=motion_interview[LIST_1].ref_idx;
    currMB->best_mvp[1]=motion_interview[LIST_1].mv;    
    mvp_available=TRUE;
  }

  if(!mvp_available) 
  {
    l0_refX=l1_refX=0;

    if(l0_refX>=0)
      currMB->GetMVPredictor(currMB, mb, &currMB->best_mvp[0], (short)l0_refX, motion, LIST_0, 0, 0, 16, 16, currMB->p_Slice->depth_based_mvp_flag);
    if(l1_refX>=0)
      currMB->GetMVPredictor(currMB, mb, &currMB->best_mvp[1], (short)l1_refX, motion, LIST_1, 0, 0, 16, 16, currMB->p_Slice->depth_based_mvp_flag);
    currMB->best_ref[0]=(int)l0_refX;
    currMB->best_ref[1]=(int)l1_refX;

    if(currMB->best_ref[0]>=0 || currMB->best_ref[1]>=0)
      mvp_available=TRUE;
  }
}

/*!
************************************************************************
* \brief
*    Get Spatial Direct Mode Motion Vectors 
************************************************************************
*/
void Get_Direct_MV_Spatial_MVP (Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice; 
  VideoParameters *p_Vid = currMB->p_Vid;
  //InputParameters* p_Inp=currMB->p_Inp;
  int   block_x, block_y, pic_block_x, pic_block_y, opic_block_x, opic_block_y;
  MotionVector *****all_mvs;
  StorablePicture**list1 = currSlice->listX[LIST_1];
  //int i=0,j=0;
  char    *direct_ref_idx;

  char best_ref[2]={-1,-1};
  MotionVector best_mvp[2];

  prepare_mvp_direct_params(currMB);

  best_ref[0]=(char)currMB->best_ref[0];
  best_mvp[0]=currMB->best_mvp[0];

  best_ref[1]=(char)currMB->best_ref[1];
  best_mvp[1]=currMB->best_mvp[1];

  if (best_ref[0] == -1 && best_ref[1] == -1)
  {
    for (block_y=0; block_y<4; block_y++)
    {
      pic_block_y  = currMB->block_y + block_y;
      for (block_x=0; block_x<4; block_x++)
      {
        pic_block_x  = currMB->block_x + block_x;
        direct_ref_idx = currSlice->direct_ref_idx[pic_block_y][pic_block_x];

        currSlice->all_mv[LIST_0][0][0][block_y][block_x] = zero_mv;
        currSlice->all_mv[LIST_1][0][0][block_y][block_x] = zero_mv;

        direct_ref_idx[LIST_0] = direct_ref_idx[LIST_1] = 0;
        if ((p_Vid->active_pps->weighted_bipred_idc == 1)||(currSlice->depth_range_wp))
          test_valid_direct(currSlice, currSlice->active_sps, direct_ref_idx, 0, 0, pic_block_y, pic_block_x);
        else
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 2;
      }
    }
  }
  else if (best_ref[0] == 0 || best_ref[1] == 0)
  {
    int (*get_colocated)(Macroblock *currMB, StorablePicture *list1, int i, int j) = 
      p_Vid->active_sps->direct_8x8_inference_flag ? get_colocated_info : get_colocated_info_4x4;

    int is_moving_block;
    for (block_y = 0; block_y < 4; block_y++)
    {
      pic_block_y  = currMB->block_y + block_y;
      opic_block_y = (currMB->opix_y >> 2) + block_y;

      for (block_x=0; block_x<4; block_x++)
      {
        pic_block_x    = currMB->block_x + block_x;
        direct_ref_idx = currSlice->direct_ref_idx[pic_block_y][pic_block_x];
        opic_block_x   = (currMB->pix_x >> 2) + block_x;

        all_mvs = currSlice->all_mv;

        is_moving_block = (get_colocated(currMB, list1[0], opic_block_x, opic_block_y) == 0);
        is_moving_block = 0;

        if (best_ref[0] < 0)
        {
          all_mvs[LIST_0][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_0] = -1;
        }
        else if ((best_ref[0] == 0) && is_moving_block)
        {
          all_mvs[LIST_0][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_0] = 0;
        }
        else
        {
          all_mvs[LIST_0][(short) best_ref[0]][0][block_y][block_x] = best_mvp[0];
          direct_ref_idx[LIST_0] = (char)best_ref[0];
        }

        if (best_ref[1] < 0)
        {
          all_mvs[LIST_1][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_1] = -1;
        }
        else if((best_ref[1] == 0) && is_moving_block)
        {
          all_mvs[LIST_1][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_1] = 0;
        }
        else
        {
          all_mvs[LIST_1][(short) best_ref[1]][0][block_y][block_x] = best_mvp[1];
          direct_ref_idx[LIST_1] = (char)best_ref[1];
        }

        if      (direct_ref_idx[LIST_1] == -1)
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 0;
        else if (direct_ref_idx[LIST_0] == -1)
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 1;
        else 
          if ((p_Vid->active_pps->weighted_bipred_idc == 1)||(currSlice->depth_range_wp))
            test_valid_direct(currSlice, currSlice->active_sps, direct_ref_idx, best_ref[0], best_ref[1], pic_block_y, pic_block_x);
          else
            currSlice->direct_pdir[pic_block_y][pic_block_x] = 2;
      }
    }
  }
  else
  {
    for (block_y=0; block_y<4; block_y++)
    {
      pic_block_y  = currMB->block_y + block_y;

      for (block_x=0; block_x<4; block_x++)
      {
        pic_block_x  = currMB->block_x + block_x;
        direct_ref_idx = currSlice->direct_ref_idx[pic_block_y][pic_block_x];

        all_mvs = currSlice->all_mv;

        if (best_ref[0] > 0)
        {
          all_mvs[LIST_0][(short) best_ref[0]][0][block_y][block_x] = best_mvp[0];
          direct_ref_idx[LIST_0]= (char)best_ref[0];          
        }
        else
        {
          all_mvs[LIST_0][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_0]=-1;
        }

        if (best_ref[1] > 0)
        {
          all_mvs[LIST_1][(short) best_ref[1]][0][block_y][block_x] = best_mvp[1];
          direct_ref_idx[LIST_1] = (char)best_ref[1];
        }
        else
        {
          all_mvs[LIST_1][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_1] = -1;
        }

        if      (direct_ref_idx[LIST_1] == -1)
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 0;
        else if (direct_ref_idx[LIST_0] == -1)
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 1;
        else
          if ((p_Vid->active_pps->weighted_bipred_idc == 1)||(currSlice->depth_range_wp))
            test_valid_direct(currSlice, currSlice->active_sps, direct_ref_idx, best_ref[0], best_ref[1], pic_block_y, pic_block_x);
          else
            currSlice->direct_pdir[pic_block_y][pic_block_x] = 2;
      }
    }
  }
}
#endif

/*!
************************************************************************
* \brief
*    Calculate Spatial Direct Mode Motion Vectors 
************************************************************************
*/
void Get_Direct_MV_Spatial_Normal (Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice; 
  VideoParameters *p_Vid = currMB->p_Vid;
  PicMotionParams **mv_info = p_Vid->enc_picture->mv_info;
  char l0_refA, l0_refB, l0_refC;
  char l1_refA, l1_refB, l1_refC;
  char l0_refX,l1_refX;
  MotionVector pmvfw = zero_mv, pmvbw = zero_mv;

  int   block_x, block_y, pic_block_x, pic_block_y, opic_block_x, opic_block_y;
  MotionVector *****all_mvs;
  char  *direct_ref_idx;
  StorablePicture **list1 = currSlice->listX[LIST_1];



  PixelPos mb[4];  


  get_neighbors(currMB, mb, 0, 0, 16);


  set_direct_references(&mb[0], &l0_refA,  &l1_refA,  mv_info);
  set_direct_references(&mb[1], &l0_refB,  &l1_refB,  mv_info);
  set_direct_references(&mb[2], &l0_refC,  &l1_refC,  mv_info);

  l0_refX = (char) imin(imin((unsigned char) l0_refA, (unsigned char) l0_refB), (unsigned char) l0_refC);
  l1_refX = (char) imin(imin((unsigned char) l1_refA, (unsigned char) l1_refB), (unsigned char) l1_refC);

#if EXT3D
  if (l0_refX >= 0)
    currMB->GetMVPredictor (currMB, mb, &pmvfw, l0_refX, mv_info, LIST_0, 0, 0, 16, 16,0);

  if (l1_refX >= 0)
    currMB->GetMVPredictor (currMB, mb, &pmvbw, l1_refX, mv_info, LIST_1, 0, 0, 16, 16,0);
#else
  if (l0_refX >= 0)
    currMB->GetMVPredictor (currMB, mb, &pmvfw, l0_refX, mv_info, LIST_0, 0, 0, 16, 16);

  if (l1_refX >= 0)
    currMB->GetMVPredictor (currMB, mb, &pmvbw, l1_refX, mv_info, LIST_1, 0, 0, 16, 16);
#endif

  if (l0_refX == -1 && l1_refX == -1)
  {
    for (block_y=0; block_y<4; block_y++)
    {
      pic_block_y  = currMB->block_y + block_y;
      for (block_x=0; block_x<4; block_x++)
      {
        pic_block_x  = currMB->block_x + block_x;
        direct_ref_idx = currSlice->direct_ref_idx[pic_block_y][pic_block_x];

        currSlice->all_mv[LIST_0][0][0][block_y][block_x] = zero_mv;
        currSlice->all_mv[LIST_1][0][0][block_y][block_x] = zero_mv;

        direct_ref_idx[LIST_0] = direct_ref_idx[LIST_1] = 0;
#if EXT3D
    if ((p_Vid->active_pps->weighted_bipred_idc == 1)||(currSlice->depth_range_wp))
#else
        if (p_Vid->active_pps->weighted_bipred_idc == 1)
#endif
          test_valid_direct(currSlice, currSlice->active_sps, direct_ref_idx, 0, 0, pic_block_y, pic_block_x);
        else
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 2;
      }
    }
  }
  else if (l0_refX == 0 || l1_refX == 0)
  {
    int (*get_colocated)(Macroblock *currMB, StorablePicture *list1, int i, int j) = 
      p_Vid->active_sps->direct_8x8_inference_flag ? get_colocated_info : get_colocated_info_4x4;

    int is_moving_block;
    for (block_y = 0; block_y < 4; block_y++)
    {
      pic_block_y  = currMB->block_y + block_y;
      opic_block_y = (currMB->opix_y >> 2) + block_y;

      for (block_x=0; block_x<4; block_x++)
      {
        pic_block_x    = currMB->block_x + block_x;
        direct_ref_idx = currSlice->direct_ref_idx[pic_block_y][pic_block_x];
        opic_block_x   = (currMB->pix_x >> 2) + block_x;

        all_mvs = currSlice->all_mv;
        is_moving_block = (get_colocated(currMB, list1[0], opic_block_x, opic_block_y) == 0);

        if (l0_refX < 0)
        {
          all_mvs[LIST_0][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_0] = -1;
        }
        else if ((l0_refX == 0) && is_moving_block)
        {
          all_mvs[LIST_0][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_0] = 0;
        }
        else
        {
          all_mvs[LIST_0][(short) l0_refX][0][block_y][block_x] = pmvfw;
          direct_ref_idx[LIST_0] = (char)l0_refX;
        }
        
        if (l1_refX < 0)
        {
          all_mvs[LIST_1][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_1] = -1;
        }
        else if((l1_refX == 0) && is_moving_block)
        {
          all_mvs[LIST_1][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_1] = 0;
        }
        else
        {
          all_mvs[LIST_1][(short) l1_refX][0][block_y][block_x] = pmvbw;
          direct_ref_idx[LIST_1] = (char)l1_refX;
        }

        if      (direct_ref_idx[LIST_1] == -1)
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 0;
        else if (direct_ref_idx[LIST_0] == -1)
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 1;
#if EXT3D
        else if ((p_Vid->active_pps->weighted_bipred_idc == 1)||(currSlice->depth_range_wp))
#else
        else if (p_Vid->active_pps->weighted_bipred_idc == 1)
#endif
          test_valid_direct(currSlice, currSlice->active_sps, direct_ref_idx, l0_refX, l1_refX, pic_block_y, pic_block_x);
        else
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 2;
      }
    }
  }
  else
  {
    for (block_y=0; block_y<4; block_y++)
    {
      pic_block_y  = currMB->block_y + block_y;

      for (block_x=0; block_x<4; block_x++)
      {
        pic_block_x  = currMB->block_x + block_x;
        direct_ref_idx = currSlice->direct_ref_idx[pic_block_y][pic_block_x];

        all_mvs = currSlice->all_mv;

        if (l0_refX > 0)
        {
          all_mvs[LIST_0][(short) l0_refX][0][block_y][block_x] = pmvfw;
          direct_ref_idx[LIST_0]= (char)l0_refX;          
        }
        else
        {
          all_mvs[LIST_0][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_0]=-1;
        }

        if (l1_refX > 0)
        {
          all_mvs[LIST_1][(short) l1_refX][0][block_y][block_x] = pmvbw;
          direct_ref_idx[LIST_1] = (char)l1_refX;
        }
        else
        {
          all_mvs[LIST_1][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_1] = -1;
        }

        if      (direct_ref_idx[LIST_1] == -1)
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 0;
        else if (direct_ref_idx[LIST_0] == -1)
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 1;
#if EXT3D
        else if ((p_Vid->active_pps->weighted_bipred_idc == 1)||(currSlice->depth_range_wp))
#else
        else if (p_Vid->active_pps->weighted_bipred_idc == 1)
#endif
          test_valid_direct(currSlice, currSlice->active_sps, direct_ref_idx, l0_refX, l1_refX, pic_block_y, pic_block_x);
        else
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 2;
      }
    }
  }
}

/*!
************************************************************************
* \brief
*    Calculate Spatial Direct Mode Motion Vectors 
************************************************************************
*/
void Get_Direct_MV_Spatial_MBAFF (Macroblock *currMB)
{
  char l0_refA, l0_refB, l0_refC;
  char l1_refA, l1_refB, l1_refC;
  short l0_refX, l1_refX;
  MotionVector pmvfw = zero_mv, pmvbw = zero_mv;

  int   block_x, block_y, pic_block_x, pic_block_y, opic_block_x, opic_block_y;
  MotionVector *****all_mvs;
  char  *direct_ref_idx;
  int is_moving_block;
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  PicMotionParams **mv_info = p_Vid->enc_picture->mv_info;
  StorablePicture **list1 = currSlice->listX[LIST_1 + currMB->list_offset];



  int (*get_colocated)(Macroblock *currMB, StorablePicture *list1, int i, int j) = 
    p_Vid->active_sps->direct_8x8_inference_flag ? get_colocated_info : get_colocated_info_4x4;

  PixelPos mb[4];  

#if EXT3D
  int ref_view_id=-1;
  //int ref_voidx=-1;
  PixelPos mb_tmp[4];
#endif

  get_neighbors(currMB, mb, 0, 0, 16);


  if (currMB->mb_field)
  {
    set_direct_references_mb_field(&mb[0], &l0_refA, &l1_refA, mv_info, p_Vid->mb_data);
    set_direct_references_mb_field(&mb[1], &l0_refB, &l1_refB, mv_info, p_Vid->mb_data);
    set_direct_references_mb_field(&mb[2], &l0_refC, &l1_refC, mv_info, p_Vid->mb_data);
  }
  else
  {
    set_direct_references_mb_frame(&mb[0], &l0_refA, &l1_refA, mv_info, p_Vid->mb_data);
    set_direct_references_mb_frame(&mb[1], &l0_refB, &l1_refB, mv_info, p_Vid->mb_data);
    set_direct_references_mb_frame(&mb[2], &l0_refC, &l1_refC, mv_info, p_Vid->mb_data);
  }

  l0_refX = (char) imin(imin((unsigned char) l0_refA, (unsigned char) l0_refB), (unsigned char) l0_refC);
  l1_refX = (char) imin(imin((unsigned char) l1_refA, (unsigned char) l1_refB), (unsigned char) l1_refC);


#if EXT3D
  memcpy(mb_tmp,mb,4*sizeof(PixelPos));
  if(l0_refX>=0)
  {
    check_neighbors(currMB,mb,mv_info,LIST_0,l0_refX);
    ref_view_id=currSlice->listX[l0_refX][0]->view_id;
    // ref_voidx=
    GetVOIdx(p_Vid->p_Inp,ref_view_id);
    currMB->GetMVPredictor (currMB, mb, &pmvfw, (short)l0_refX, mv_info, LIST_0, 0, 0, 16, 16,
      currMB->p_Slice->depth_based_mvp_flag);
  }
  memcpy(mb,mb_tmp,4*sizeof(PixelPos));
  if(l1_refX>=0)
  {
    check_neighbors(currMB,mb,mv_info,LIST_1,l1_refX);
    ref_view_id=currSlice->listX[l1_refX][0]->view_id;
    // ref_voidx=
    GetVOIdx(p_Vid->p_Inp,ref_view_id);
    currMB->GetMVPredictor (currMB, mb, &pmvbw, (short)l1_refX, mv_info, LIST_1, 0, 0, 16, 16,
      currMB->p_Slice->depth_based_mvp_flag);
  }
#else
  if (l0_refX >=0)
    currMB->GetMVPredictor (currMB, mb, &pmvfw, l0_refX, mv_info, LIST_0, 0, 0, 16, 16);

  if (l1_refX >=0)
    currMB->GetMVPredictor (currMB, mb, &pmvbw, l1_refX, mv_info, LIST_1, 0, 0, 16, 16);
#endif

  for (block_y=0; block_y<4; block_y++)
  {
    pic_block_y  = currMB->block_y + block_y;
    opic_block_y = (currMB->opix_y >> 2) + block_y;

    for (block_x=0; block_x<4; block_x++)
    {
      pic_block_x  = currMB->block_x + block_x;
      direct_ref_idx = currSlice->direct_ref_idx[pic_block_y][pic_block_x];
      opic_block_x = (currMB->pix_x >> 2) + block_x;
      is_moving_block = (get_colocated(currMB, list1[0], opic_block_x, opic_block_y) == 0);

      all_mvs = currSlice->all_mv;

      if (l0_refX >=0)
      {
        if (!l0_refX  && is_moving_block)
        {
          all_mvs[LIST_0][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_0] = 0;
        }
        else
        {
          all_mvs[LIST_0][(short) l0_refX][0][block_y][block_x] = pmvfw;
          direct_ref_idx[LIST_0] = (char)l0_refX;
        }
      }
      else
      {
        all_mvs[LIST_0][0][0][block_y][block_x] = zero_mv;
        direct_ref_idx[LIST_0] = -1;
      }

      if (l1_refX >=0)
      {
        if(l1_refX==0 && is_moving_block)
        {
          all_mvs[LIST_1][0][0][block_y][block_x] = zero_mv;
          direct_ref_idx[LIST_1] = (char)l1_refX;
        }
        else
        {
          all_mvs[LIST_1][(short) l1_refX][0][block_y][block_x] = pmvbw;
          direct_ref_idx[LIST_1] = (char)l1_refX;
        }
      }
      else
      {
        all_mvs[LIST_1][0][0][block_y][block_x] = zero_mv;
        direct_ref_idx[LIST_1] = -1;
      }

     // Test Level Limits if satisfied.

      // Test Level Limits if satisfied.
      if ((out_of_bounds_mvs(p_Vid, &all_mvs[LIST_0][l0_refX < 0? 0 : l0_refX][0][block_y][block_x])
        ||  out_of_bounds_mvs(p_Vid, &all_mvs[LIST_1][l1_refX < 0? 0 : l1_refX][0][block_y][block_x])))
      {
        direct_ref_idx[LIST_0] = -1;
        direct_ref_idx[LIST_1] = -1;
        currSlice->direct_pdir   [pic_block_y][pic_block_x]         = -1;
      }     
      else
      {
        if (l0_refX < 0 && l1_refX < 0)
        {
          direct_ref_idx[LIST_0] = direct_ref_idx[LIST_1] = 0;
          l0_refX = 0;
          l1_refX = 0;
        }

        if      (direct_ref_idx[LIST_1] == -1)
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 0;
        else if (direct_ref_idx[LIST_0] == -1)
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 1;
#if EXT3D
        else if ((p_Vid->active_pps->weighted_bipred_idc == 1)||(currSlice->depth_range_wp))
#else
        else if (p_Vid->active_pps->weighted_bipred_idc == 1)
#endif
          test_valid_direct(currSlice, currSlice->active_sps, direct_ref_idx, l0_refX, l1_refX, pic_block_y, pic_block_x);
        else
          currSlice->direct_pdir[pic_block_y][pic_block_x] = 2;
      }
    }
  }
}
