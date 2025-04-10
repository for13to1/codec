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
* \file wp_dbr.c
*
* \brief
*    Estimate weights for depth range based WP in joint texture depth coding
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*     -Su Wenyi        <suwy@mail.ustc.edu.cn>
*     -Dmytro.Rusanovskyy  <Dmytro.Rusanovskyy@nokia.com>
*************************************************************************************
*/

#include "contributors.h"

#include "global.h"
#include "image.h"
#include "wp.h"

#if EXT3D
void ComputeExplicitWPParamsDRB(Slice* currSlice,
                short default_weight[3],
                short weight[6][MAX_REFERENCE_PICTURES][3],
                short offset[6][MAX_REFERENCE_PICTURES][3])
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  int clist, n, comp;
  StorablePicture* curr=p_Vid->enc_picture;
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

      curr_weight=(short)(( W_factora*W_factorb*W_factorc + (1<<(scale_w*3-currSlice->luma_log_weight_denom-1)) ) >> (scale_w*3-currSlice->luma_log_weight_denom));
      curr_weight=sClip3(-127,128,curr_weight);

      O_factora=( (curr_depth_near<<(scale_o))+(curr_ref_depth_far>>1) ) / curr_ref_depth_far;

      temp1 = curr_depth_far-curr_ref_depth_far;
      temp2 = curr_depth_far-curr_depth_near;
      sign= (temp1 < 0) ? -1 : 1;
      x = ( temp1 + sign * ( temp2 >> 1 ) ) / temp2;
      sign = ( (temp1 - x * temp2) < 0 ) ? -1 : 1;
      O_factorb = (x<<scale_o);
      O_factorb += ( ((temp1 - x * temp2) << scale_o) + sign * (temp2 >> 1) ) / temp2;

      curr_offset=(short)(( O_factora*O_factorb + (1<<(scale_o*2-8-1)) ) >> (scale_o*2-8));
      curr_offset=sClip3(-127,128,curr_offset);

      weight[clist][n][0]=curr_weight;
      offset[clist][n][0]=curr_offset;
    }
  }
}

void EstimateWPPSliceAlg3(Slice* currSlice, int select_offset)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;

  int  n;
  short default_weight[3];
  int list_offset   = ((currSlice->mb_aff_frame_flag) && (p_Vid->mb_data[p_Vid->current_mb_nr].mb_field))? (p_Vid->current_mb_nr & 0x01) ? 4 : 2 : 0;
  int clist;

  int cur_slice;
  short comp=0;

  short weight[2][MAX_REFERENCE_PICTURES][3];
  short offset[2][MAX_REFERENCE_PICTURES][3];

  UNREFERENCED_PARAMETER(select_offset);

  currSlice->luma_log_weight_denom   = 5;
  currSlice->chroma_log_weight_denom = 5;

  currSlice->wp_luma_round   = 1 << (currSlice->luma_log_weight_denom - 1);
  currSlice->wp_chroma_round = 1 << (currSlice->chroma_log_weight_denom - 1);

  default_weight[0]       = 1 << currSlice->luma_log_weight_denom;
  default_weight[1]       = default_weight[2] = 1 << currSlice->chroma_log_weight_denom;

  if(p_Inp->slice_mode == 1)
  {
    cur_slice = p_Vid->current_mb_nr / p_Inp->slice_argument; 
  }
  else
    cur_slice = 0;

  ComputeExplicitWPParamsDRB(currSlice, default_weight, weight, offset);

  for (clist = 0; clist < 2 + list_offset; clist++)
  {
    for (n = 0; n < currSlice->listXsize[clist]; n++)
    {
        for (comp=0; comp < 3; comp ++)
        {
          currSlice->wp_weight[clist][n][comp] = weight[clist][n][comp];
          currSlice->wp_offset[clist][n][comp] = offset[clist][n][comp];
          if(p_Vid->wp_weights)
            p_Vid->wp_weights[comp][clist][n][cur_slice] = weight[clist][n][comp];
          if(p_Vid->wp_offsets)
            p_Vid->wp_offsets[comp][clist][n][cur_slice] = offset[clist][n][comp];
        }
#if DEBUG_WP
        for(comp = 0; comp < 3; comp++)
          printf("slice %d: index %d component %d weight %d offset %d\n", cur_slice, n,comp,currSlice->wp_weight[clist][n][comp],currSlice->wp_offset[clist][n][comp]);
#endif
    }
  }
}

void EstimateWPBSliceAlg3(Slice* currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;
  int i, j, n;

  int comp;

  short default_weight[3];
  int list_offset   = ((currSlice->mb_aff_frame_flag) && (p_Vid->mb_data[p_Vid->current_mb_nr].mb_field))? (p_Vid->current_mb_nr & 0x01) ? 4 : 2 : 0;
  int clist;
  int cur_slice = 0;

  short im_weight[6][MAX_REFERENCE_PICTURES][MAX_REFERENCE_PICTURES][3];
  short wp_weight[6][MAX_REFERENCE_PICTURES][3];
  short wp_offset[6][MAX_REFERENCE_PICTURES][3];

  //double curr_depth_near=0.0;
  //double curr_depth_far=0.0;

  if (p_Vid->active_pps->weighted_bipred_idc == 2) //! implicit mode. Values are fixed and it is important to show it here
  {
    currSlice->luma_log_weight_denom = 5;
    currSlice->chroma_log_weight_denom = 5;
  }
  else                                     //! explicit mode. Values can be changed for higher precision.
  {
    currSlice->luma_log_weight_denom = 5;
    currSlice->chroma_log_weight_denom = 5;
  }

  currSlice->wp_luma_round   = 1 << (currSlice->luma_log_weight_denom - 1);
  currSlice->wp_chroma_round = 1 << (currSlice->chroma_log_weight_denom - 1);
  default_weight[0] = 1 << currSlice->luma_log_weight_denom;
  default_weight[1] = 1 << currSlice->chroma_log_weight_denom;
  default_weight[2] = 1 << currSlice->chroma_log_weight_denom;

  if(p_Inp->slice_mode == 1)
  {
    cur_slice = p_Vid->current_mb_nr / p_Inp->slice_argument ; 
  }
  else
    cur_slice = 0;

  ComputeExplicitWPParamsDRB(currSlice,
    default_weight,
    wp_weight,
    wp_offset);

  if(p_Vid->active_pps->weighted_bipred_idc==3)
    ComputeImplicitWeights(currSlice, default_weight, im_weight);


  for (clist=0; clist<2 + list_offset; clist++)
  {
    for (n = 0; n < currSlice->listXsize[clist]; n++)
    {
      for (comp=0; comp < 3; comp ++)
      {
        currSlice->wp_weight[clist][n][comp] = wp_weight[clist][n][comp];
        currSlice->wp_offset[clist][n][comp] = wp_offset[clist][n][comp];
        if(p_Vid->wp_weights)
          p_Vid->wp_weights[comp][clist][n][cur_slice] = wp_weight[clist][n][comp];
        if(p_Vid->wp_offsets)
          p_Vid->wp_offsets[comp][clist][n][cur_slice] = wp_offset[clist][n][comp];
#if DEBUG_WP
        printf("slice %d: index %d component %d weight %d offset %d\n", clist, n,comp,currSlice->wp_weight[clist][n][comp],currSlice->wp_offset[clist][n][comp]);
#endif
      }
    }
  }


  //curr_depth_near=p_Vid->enc_picture->depth_near;
  //curr_depth_far=p_Vid->enc_picture->depth_far;

  for (i = 0; i < currSlice->listXsize[LIST_0]; i++)
  {
    for (j = 0; j < currSlice->listXsize[LIST_1]; j++)
    {
      //double depth_near_list0=currSlice->listX[LIST_0][i]->depth_near;
      //double depth_far_list0=currSlice->listX[LIST_0][i]->depth_far;
      //double depth_near_list1=currSlice->listX[LIST_1][j]->depth_near;
      //double depth_far_list1=currSlice->listX[LIST_1][j]->depth_far;
      //if((p_Vid->active_pps->weighted_bipred_idc==3)&&(fabs(curr_depth_near-depth_near_list0)<=1e-3)&&(fabs(curr_depth_near-depth_near_list1)<=1e-3)
      //  &&(fabs(curr_depth_far-depth_far_list0)<=1e-3)&&(fabs(curr_depth_far-depth_far_list1)))
      //{
      //  for(comp=0;comp<3;comp++)
      //  {
      //    currSlice->wbp_weight[0][i][j][comp]=im_weight[0][i][j][comp];
      //    currSlice->wbp_weight[1][i][j][comp]=im_weight[1][i][j][comp];
      //    currSlice->wp_offset[0+list_offset][i][comp]=0;
      //    currSlice->wp_offset[1+list_offset][j][comp]=1;
      //    //!<int this conditions, the offset will be equal to zero 
      //  }
      //}
      for (comp = 0; comp < 3; comp++)
      {
        currSlice->wbp_weight[0][i][j][comp] = currSlice->wp_weight[0][i][comp];
        currSlice->wbp_weight[1][i][j][comp] = currSlice->wp_weight[1][j][comp];
      }
    }
  }

}
#endif

