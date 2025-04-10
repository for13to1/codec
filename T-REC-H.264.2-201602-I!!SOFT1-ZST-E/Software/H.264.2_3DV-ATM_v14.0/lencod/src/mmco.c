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
 * \file mmco.c
 *
 * \brief
 *    MMCO example operations.
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Alexis Michael Tourapis                     <alexismt@ieee.org>
 *     - Athanasios Leontaris                        <aleon@dolby.com>
 *************************************************************************************
 */

#include "contributors.h"

#include <ctype.h>
#include <limits.h>
#include "global.h"

#include "image.h"
#include "nalucommon.h"
#include "report.h"




void mmco_long_term(VideoParameters *p_Vid, int current_pic_num)
{
  DecRefPicMarking_t *tmp_drpm,*tmp_drpm2;

  if ( !p_Vid->currentPicture->idr_flag )
  {
    if (p_Vid->dec_ref_pic_marking_buffer!=NULL)
      return;

    if (NULL==(tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) 
      no_mem_exit("poc_based_ref_management: tmp_drpm");

    tmp_drpm->Next=NULL;

    tmp_drpm->memory_management_control_operation = 0;

    if (NULL==(tmp_drpm2=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) 
      no_mem_exit("poc_based_ref_management: tmp_drpm2");
    tmp_drpm2->Next=tmp_drpm;

    tmp_drpm2->memory_management_control_operation = 3;
    tmp_drpm2->long_term_frame_idx = current_pic_num;
    p_Vid->dec_ref_pic_marking_buffer = tmp_drpm2;
  }
  else
  {
    p_Vid->long_term_reference_flag = TRUE;
  }
}

/*!
************************************************************************
* \brief
*    POC-based reference management (FRAME)
************************************************************************
*/

void poc_based_ref_management_frame_pic(DecodedPictureBuffer *p_Dpb, int current_pic_num)
{
  VideoParameters *p_Vid = p_Dpb->p_Vid;
  unsigned i, pic_num = 0;

  int min_poc=INT_MAX;
  DecRefPicMarking_t *tmp_drpm,*tmp_drpm2;

  if (p_Vid->dec_ref_pic_marking_buffer!=NULL)
    return;

  if ( p_Vid->currentPicture->idr_flag )
    return;

  if ((p_Dpb->ref_frames_in_buffer + p_Dpb->ltref_frames_in_buffer)==0)
    return;

  for (i = 0; i < p_Dpb->used_size; i++)
  {
#if EXT3D // ADAPTIVE_MMCO_REORDER
    if (p_Dpb->fs[i]->view_id==p_Vid->view_id && p_Dpb->fs[i]->is_reference  && (!(p_Dpb->fs[i]->is_long_term)) && p_Dpb->fs[i]->poc < min_poc)
#else
    if (p_Dpb->fs[i]->is_reference  && (!(p_Dpb->fs[i]->is_long_term)) && p_Dpb->fs[i]->poc < min_poc)
#endif
    {
      min_poc = p_Dpb->fs[i]->frame->poc ;
      pic_num = p_Dpb->fs[i]->frame->pic_num;
    }
  }

  if (NULL==(tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) 
    no_mem_exit("poc_based_ref_management: tmp_drpm");
  tmp_drpm->Next=NULL;

  tmp_drpm->memory_management_control_operation = 0;

  if (NULL==(tmp_drpm2=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) 
    no_mem_exit("poc_based_ref_management: tmp_drpm2");
  tmp_drpm2->Next=tmp_drpm;

  tmp_drpm2->memory_management_control_operation = 1;
#if EXT3D
  tmp_drpm2->difference_of_pic_nums_minus1 = current_pic_num - pic_num - 1;
#else
#if MVC_EXTENSION_ENABLE
  if(p_Vid->active_sps->profile_idc < MULTIVIEW_HIGH)
    tmp_drpm2->difference_of_pic_nums_minus1 = current_pic_num - pic_num - 1;
  else
    tmp_drpm2->difference_of_pic_nums_minus1 = (current_pic_num - pic_num)/2 - 1;
#endif
#endif
  p_Vid->dec_ref_pic_marking_buffer = tmp_drpm2;
}

/*!
************************************************************************
* \brief
*    POC-based reference management (FIELD)
************************************************************************
*/

void poc_based_ref_management_field_pic(DecodedPictureBuffer *p_Dpb, int current_pic_num)
{
  VideoParameters *p_Vid = p_Dpb->p_Vid;
  unsigned int i, pic_num1 = 0, pic_num2 = 0;

  int min_poc=INT_MAX;
  DecRefPicMarking_t *tmp_drpm,*tmp_drpm2, *tmp_drpm3;

  if (p_Vid->dec_ref_pic_marking_buffer!=NULL)
    return;

  if ( p_Vid->currentPicture->idr_flag )
    return;

  if ((p_Dpb->ref_frames_in_buffer + p_Dpb->ltref_frames_in_buffer)==0)
    return;

  if ( p_Vid->structure == TOP_FIELD )
  {
    for (i=0; i<p_Dpb->used_size;i++)
    {
      if (p_Dpb->fs[i]->is_reference && (!(p_Dpb->fs[i]->is_long_term)) && p_Dpb->fs[i]->poc < min_poc)
      {      
        min_poc  = p_Dpb->fs[i]->poc;
        pic_num1 = p_Dpb->fs[i]->top_field->pic_num;
        pic_num2 = p_Dpb->fs[i]->bottom_field->pic_num;
      }
    }
  }

  if (NULL==(tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) no_mem_exit("poc_based_ref_management_field_pic: tmp_drpm");
  tmp_drpm->Next=NULL;
  tmp_drpm->memory_management_control_operation = 0;

  if ( p_Vid->structure == BOTTOM_FIELD )
  {
    p_Vid->dec_ref_pic_marking_buffer = tmp_drpm;
    return;
  }

  if (NULL==(tmp_drpm2=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) no_mem_exit("poc_based_ref_management_field_pic: tmp_drpm2");
  tmp_drpm2->Next=tmp_drpm;
  tmp_drpm2->memory_management_control_operation = 1;
  tmp_drpm2->difference_of_pic_nums_minus1 = current_pic_num - pic_num1 - 1;

  if (NULL==(tmp_drpm3=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) no_mem_exit("poc_based_ref_management_field_pic: tmp_drpm3");
  tmp_drpm3->Next=tmp_drpm2;
  tmp_drpm3->memory_management_control_operation = 1;
  tmp_drpm3->difference_of_pic_nums_minus1 = current_pic_num - pic_num2 - 1;

  p_Vid->dec_ref_pic_marking_buffer = tmp_drpm3;
}


#if EXT3D
/*!
************************************************************************
* \brief
*    the reference management (FRAME) for MVC
************************************************************************
*/
void mvc_based_ref_management_frame_pic(DecodedPictureBuffer *p_Dpb,Slice* currSlice, int current_pic_num)
{
  //Wenyi >>

  //!<In multi-view video coding, anchor picture, a coded picture in which all slices reference only slices with in the same access unit
  //!<i.e., no interprediction is used ,and all following codes pictures in ouput order do not use 
  //!<inter prediction from any picture prior to the coded picture in decoding order  , 
  //!<so these pictures which prior to the coded anchor picture should be set as non-reference frames

  VideoParameters*p_Vid=p_Dpb->p_Vid;                                                                   
  DecRefPicMarking_t *tmp_drpm=NULL,*current_drpm=NULL,*drpm=NULL;

  unsigned int i=0;
  int first=1;



  if(currSlice->start_mb_nr!=0)
    return;      //multi-slice
  else if(currSlice->idr_flag)
    return;     //idr ,MMCO is unnecessary




  for(i=0;i<p_Dpb->used_size;++i)
  {
    if((p_Dpb->fs[i]->view_id==p_Vid->view_id)&&(p_Dpb->fs[i]->is_reference)&&(!p_Dpb->fs[i]->is_long_term)&&((int)p_Dpb->fs[i]->frame_num!=p_Vid->last_mmco_frm_num))
    {
      if (NULL == (tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) 
        no_mem_exit("mvc_anchor_based_ref_management_frame_pic: tmp_drpm");
      tmp_drpm->memory_management_control_operation = 1;
      tmp_drpm->difference_of_pic_nums_minus1 =current_pic_num-p_Dpb->fs[i]->frame->pic_num - 1;

      if(first)
      {
        drpm = current_drpm = tmp_drpm;
        first = 0;
      }
      else
      {
        current_drpm->Next = tmp_drpm;
        current_drpm = current_drpm->Next;
      }
    }
  }

  if(first)
    return;
  if (NULL==(tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) 
    no_mem_exit("poc_based_ref_management: tmp_drpm");
  tmp_drpm->Next=NULL;

  tmp_drpm->memory_management_control_operation = 0;

  current_drpm->Next=tmp_drpm;

  p_Vid->dec_ref_pic_marking_buffer = drpm;

}
#endif

/*!
************************************************************************
* \brief
*    Temporal layer-based reference management (FRAME)
************************************************************************
*/

void tlyr_based_ref_management_frame_pic(VideoParameters *p_Vid, int current_pic_num)
{
  unsigned i, first = 1;
  DecRefPicMarking_t *drpm = NULL, *current_drpm = NULL, *tmp_drpm = NULL;
#if EXT3D
  InputParameters *p_Inp = p_Vid->p_Inp;
#endif

  if (p_Vid->dec_ref_pic_marking_buffer!=NULL)
    return;

  if ( p_Vid->currentPicture->idr_flag )
    return;

  if ((p_Vid->p_Dpb->ref_frames_in_buffer + p_Vid->p_Dpb->ltref_frames_in_buffer)==0)
    return;

  for (i = 0; i < p_Vid->p_Dpb->used_size; i++)
  {
#if EXT3D // ADAPTIVE_MMCO_REORDER
    if (p_Vid->p_Dpb->fs[i]->view_id==p_Vid->view_id && p_Vid->p_Dpb->fs[i]->is_reference && (!(p_Vid->p_Dpb->fs[i]->is_long_term)) &&
      (p_Vid->p_Dpb->fs[i]->frame->temporal_layer > p_Vid->enc_picture->temporal_layer||(p_Inp->TLYRMMCOMethod && !p_Vid->ShortGOP && p_Inp->HierarchicalCoding == 3 && p_Vid->p_Dpb->fs[i]->frame->temporal_layer == p_Vid->enc_picture->temporal_layer)))
#else
    if (p_Vid->p_Dpb->fs[i]->is_reference && (!(p_Vid->p_Dpb->fs[i]->is_long_term)) && p_Vid->p_Dpb->fs[i]->frame->temporal_layer > p_Vid->enc_picture->temporal_layer)
#endif
    {
      if (NULL == (tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) 
        no_mem_exit("poc_based_ref_management: tmp_drpm2");
      tmp_drpm->memory_management_control_operation = 1;
      tmp_drpm->difference_of_pic_nums_minus1 = current_pic_num - p_Vid->p_Dpb->fs[i]->frame->pic_num - 1;
      
      if (first) 
      {
        drpm = current_drpm = tmp_drpm;
        first = 0;
      }
      else 
      {
        current_drpm->Next = tmp_drpm;
        current_drpm = current_drpm->Next;
      }
    }
  }

  if (first)
    return;

  if (NULL==(tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) 
    no_mem_exit("poc_based_ref_management: tmp_drpm");
  tmp_drpm->Next=NULL;

  tmp_drpm->memory_management_control_operation = 0;

  current_drpm->Next=tmp_drpm;

  p_Vid->dec_ref_pic_marking_buffer = drpm;
}

#if EXT3D // ADAPTIVE_MMCO_REORDER
void layered_sliding_ref_management_frame_pic(DecodedPictureBuffer *p_Dpb, int current_pic_num)
{
  VideoParameters *p_Vid = p_Dpb->p_Vid;
  InputParameters *p_Inp = p_Vid->p_Inp;
  unsigned i, pic_num = 0;

  int min_poc=INT_MAX;
  int LayerRefNum=0;
  DecRefPicMarking_t *tmp_drpm,*tmp_drpm2;

  if (p_Vid->dec_ref_pic_marking_buffer!=NULL)
    return;

  if ( p_Vid->currentPicture->idr_flag )
    return;

  if ((p_Dpb->ref_frames_in_buffer + p_Dpb->ltref_frames_in_buffer)==0)
    return;

  if (p_Inp->NumReferenceTL[p_Vid->enc_picture->temporal_layer] == 0)
    return;

  for (i = 0; i < p_Dpb->used_size; i++)
  {
    if (p_Vid->p_Dpb->fs[i]->view_id==p_Vid->view_id && p_Dpb->fs[i]->is_reference  && (!(p_Dpb->fs[i]->is_long_term)) && p_Dpb->fs[i]->frame->temporal_layer == p_Vid->enc_picture->temporal_layer )
    {
      if (p_Dpb->fs[i]->poc < min_poc)
      {
        min_poc = p_Dpb->fs[i]->frame->poc ;
        pic_num = p_Dpb->fs[i]->frame->pic_num;
      }
      LayerRefNum++;
    }
  }

  if (LayerRefNum==0)
    return;

  if (p_Inp->NumReferenceTL[p_Vid->enc_picture->temporal_layer] > LayerRefNum)
    return;

  if (NULL==(tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) 
    no_mem_exit("poc_based_ref_management: tmp_drpm");
  tmp_drpm->Next=NULL;

  tmp_drpm->memory_management_control_operation = 0;

  if (NULL==(tmp_drpm2=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) 
    no_mem_exit("poc_based_ref_management: tmp_drpm2");
  tmp_drpm2->Next=tmp_drpm;

  tmp_drpm2->memory_management_control_operation = 1;

  tmp_drpm2->difference_of_pic_nums_minus1 = current_pic_num - pic_num - 1;

  p_Vid->dec_ref_pic_marking_buffer = tmp_drpm2;
}
#endif

