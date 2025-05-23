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
 **************************************************************************************
 * \file
 *    slice.c
 * \brief
 *    generate the slice header, setup the bit buffer for slices,
 *    and generates the slice NALU(s)

 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Athanasios Leontaris            <aleon@dolby.com>
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 ***************************************************************************************
 */

#include "contributors.h"

#include <math.h>
#include <float.h>

#include "global.h"
#include "image.h"
#include "wp.h"
#include "list_reorder.h"
#if EXT3D
#include "configfile.h"
#endif

/*!
 ************************************************************************
 * \brief
 *    init_ref_pic_list_reordering initializations should go here
 ************************************************************************
 */
void init_ref_pic_list_reordering(Slice* currSlice, int refReorderMethod)
{
  currSlice->ref_pic_list_reordering_flag[LIST_0] = 0;
  currSlice->ref_pic_list_reordering_flag[LIST_1] = 0;

  UNREFERENCED_PARAMETER(refReorderMethod);

#if EXT3D
    currSlice->poc_ref_pic_reorder_frame=tlyr_ref_pic_reorder_frame_default;
#else
  currSlice->poc_ref_pic_reorder_frame = poc_ref_pic_reorder_frame_default;
  if(refReorderMethod == 2) 
    currSlice->poc_ref_pic_reorder_frame = tlyr_ref_pic_reorder_frame_default;
#endif

}

#if EXT3D
/*!
 ************************************************************************
 * \brief
 *    decide inter reference picture reordering for multi-view, Frame only
 ************************************************************************
 */
void inter_ref_pic_reorder_frame_mvc(Slice*currSlice,int* reordering_of_pic_nums_idc,
                   int* abs_diff_pic_num_minus1,int list_no)
{
  //InputParameters*p_Inp=currSlice->p_Inp;
  VideoParameters*p_Vid=currSlice->p_Vid;
  int i=0,j=0;
  int CurrFrmNum=p_Vid->frame_num;
  int MaxFrmNum=p_Vid->max_frame_num;
  int Diff=0;
  int FrmNumLXPred=CurrFrmNum;

  if(currSlice->ref_pic_list_reordering_flag[list_no])
    return ;

  j=0;
  for(i=0;i<currSlice->listXsize[list_no];++i)
  {

    Diff = currSlice->listX[list_no][i]->frame_num-FrmNumLXPred;
    if (Diff <= 0)
    {
      reordering_of_pic_nums_idc[j] = 0;
      abs_diff_pic_num_minus1[j] = iabs(Diff)-1;
      if (abs_diff_pic_num_minus1[j] < 0)
        abs_diff_pic_num_minus1[j] = MaxFrmNum -1;
    }
    else
    {
      reordering_of_pic_nums_idc[j] = 1;
      abs_diff_pic_num_minus1[j] = iabs(Diff)-1;
    }
    FrmNumLXPred = currSlice->listX[list_no][j]->frame_num;
    ++j;

  }
  reordering_of_pic_nums_idc[j] = 3;
  currSlice->ref_pic_list_reordering_flag[list_no]=1;
}
/*!
************************************************************************
* \brief
*    decide interview reference picture reordering for multi-view, Frame only
************************************************************************
*/
void  interview_ref_pic_reorder_frame_mvc( Slice *currSlice, FrameStore ** fs, 
                          unsigned num_ref_idx_lX_active, int *reordering_of_interview_nums_idc, int *abs_diff_view_idx_minus1, int list_no )
{
  VideoParameters*p_Vid=currSlice->p_Vid;
  InputParameters*p_Inp=currSlice->p_Inp;
  //DecodedPictureBuffer*p_Dpb=currSlice->p_Dpb;
  //int currViewId=p_Vid->view_id;
  int isAnchor=p_Vid->anchor_pic_flag[0];
  int maxViewReference=0;
  int picViewIdxLXPred=-1;
  unsigned int i=0,j=0;
  int diff=0;
  int  voidx =GetVOIdx(p_Inp,p_Vid->view_id)  ;

  //int numOfViewReference=(list_no==0)?p_Dpb->listinterviewidx0:p_Dpb->listinterviewidx1;

  UNREFERENCED_PARAMETER(fs);

  if(isAnchor)
  {
    maxViewReference=(list_no==0)?p_Inp->NumOfFwdAnchorRefs[voidx]:p_Inp->NumOfBwdAnchorRefs[voidx];
  }
  else
  {
    maxViewReference=(list_no==0)?p_Inp->NumOfFwdNoAnchorRefs[voidx]:p_Inp->NumOfBwdNoAnchorRefs[voidx];
  }

  for(i=0;i<num_ref_idx_lX_active;i++)
  {
     diff=(int)i-(int)picViewIdxLXPred;
     if(diff<=0)
     {
       reordering_of_interview_nums_idc[j]=4;
       abs_diff_view_idx_minus1[j]=iabs(diff)-1;
       if(abs_diff_view_idx_minus1[j]<0)
         abs_diff_view_idx_minus1[j]+=maxViewReference;
       assert(abs_diff_view_idx_minus1[j]>=0)  ;
     }
     else
     {
       reordering_of_interview_nums_idc[j]=5;
       abs_diff_view_idx_minus1[j]=iabs(diff)-1;    
       if(abs_diff_view_idx_minus1[j]>=maxViewReference)
         abs_diff_view_idx_minus1[j]-=maxViewReference;
       assert(abs_diff_view_idx_minus1[j]>=0)  ;

     }
     picViewIdxLXPred=i;
     j++;
  }
  reordering_of_interview_nums_idc[j]=3;
  if(j)
  {
    currSlice->reordering_interview_flg[list_no]=1;
  }
}
/*!
************************************************************************
* \brief
*    decide temporal reference picture reordering for multi-view, Frame only
************************************************************************
*/
void poc_ref_pic_reorder_frame_mvc_default( Slice *currSlice, StorablePicture **list, 
                       unsigned num_ref_idx_lX_active, int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no )
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;
  StorablePicture *p_Enc_Pic = p_Vid->enc_picture;
  DecodedPictureBuffer *p_Dpb = currSlice->p_Dpb;

  unsigned int i,j;

  int currPicNum, picNumLXPred;

  int re_order[32];
  int list_sign[32];
  int no_reorder;
  int poc_diff[32];
  int tmp_value, diff;

  int abs_poc_dist;
  int maxPicNum;
  unsigned int num_refs;

  maxPicNum  = p_Vid->max_frame_num;
  currPicNum = p_Vid->frame_num;

  picNumLXPred = currPicNum;

  UNREFERENCED_PARAMETER(long_term_pic_idx);
  UNREFERENCED_PARAMETER(list);


  // Now access all references in buffer and assign them
  // to a potential reordering list. For each one of these
  // references compute the poc distance compared to current
  // frame.
  num_refs = currSlice->p_Dpb->ref_frames_in_buffer;
  for (i = 0; i < p_Dpb->ref_frames_in_buffer; i++)
  {
    poc_diff[i] = 0xFFFF;
    re_order[i] = p_Dpb->fs_ref[i]->frame->pic_num;

    if (p_Dpb->fs_ref[i]->is_used==3 && (p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term)
      && (p_Inp->NumOfViews==1 || p_Dpb->fs_ref[i]->view_id==p_Vid->view_id))
    {
      abs_poc_dist = iabs(p_Dpb->fs_ref[i]->frame->poc - p_Enc_Pic->poc) ;
      poc_diff[i] = abs_poc_dist;
      if (list_no == LIST_0)
      {
        list_sign[i] = (p_Enc_Pic->poc < p_Dpb->fs_ref[i]->frame->poc) ? +1 : -1;
      }
      else
      {
        list_sign[i] = (p_Enc_Pic->poc > p_Dpb->fs_ref[i]->frame->poc) ? +1 : -1;
      }
    }
  }

  // now sort these references based on poc (temporal) distance
  if(currSlice->slice_type==P_SLICE)
  {
    for (i = 0; i < (num_refs - 1); i++)
    {
      for (j = i + 1; j < num_refs; j++)
      {
        if (poc_diff[i]>poc_diff[j] || (poc_diff[i] == poc_diff[j] && list_sign[j] > list_sign[i]))
        {

          tmp_value = poc_diff[i];
          poc_diff[i] = poc_diff[j];
          poc_diff[j] = tmp_value;
          tmp_value  = re_order[i];
          re_order[i] = re_order[j];
          re_order[j] = tmp_value ;
          tmp_value  = list_sign[i];
          list_sign[i] = list_sign[j];
          list_sign[j] = tmp_value;
        }
      }
    }
  }

  // populate list with selections from the pre-analysis stage
  if ( p_Inp->WPMCPrecision 
    && p_Vid->pWPX->curr_wp_rd_pass->algorithm != WP_REGULAR
    && p_Vid->pWPX->num_wp_ref_list[list_no] )
  {
    for (i=0; i<num_ref_idx_lX_active; i++)
    {
      re_order[i] = p_Vid->pWPX->wp_ref_list[list_no][i].PicNum;
    }
  }

  // Check versus default list to see if any
  // change has happened
  no_reorder = 0;
  // If different, then signal reordering
  if (no_reorder==0)
  {
    for (i=0; i<num_ref_idx_lX_active; i++)
    {
      diff = re_order[i]-picNumLXPred;
      if (diff <= 0)
      {
        reordering_of_pic_nums_idc[i] = 0;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
        if (abs_diff_pic_num_minus1[i] < 0)
          abs_diff_pic_num_minus1[i] = maxPicNum -1;
      }
      else
      {
        reordering_of_pic_nums_idc[i] = 1;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
      }
      picNumLXPred = re_order[i];
    }
    reordering_of_pic_nums_idc[i] = 3;

    if (list_no==0)
    {
      currSlice->ref_pic_list_reordering_flag[LIST_0] = 1;
    }
    else
    {
      currSlice->ref_pic_list_reordering_flag[LIST_1] = 1;
    }
  }
}

void get_mvc_lists(Slice* currSlice)
{
  //Wenyi >>

  //!<RPLR is necessary for multi-view coding
  //!<Inter reference picture->
  //!<Because of the temporal scalability, so we must use RPLCs to indicate the inter reference for decoder.
  //!<Interview reference pictures->
  //!<Interview reference pictures are added to the end of the end of the reference lists(LIST_0,LIST_1)
  //!<which means behind the short term and long term reference pictures, because of the temporal
  //!<scalability and the active reference list size, the interview reference pictures may not have the chance
  //!<to be chose as reference picture, so we need RPLCs to set the interview reference frames forward.
  //!<Especially in ViewFirst condition ,RPLCs are obvious needed to set the  interview reference   pictures at
  //!<the head of the reference lists.

  VideoParameters* p_Vid=currSlice->p_Vid;
  InputParameters* p_Inp=currSlice->p_Inp;
  DecodedPictureBuffer* p_Dpb=p_Vid->p_Dpb;
  int voidx=GetVOIdx(p_Inp,currSlice->view_id);

  int NumOfList0InterReference=currSlice->listXsize[LIST_0];
  int NumOfList1InterReference=currSlice->listXsize[LIST_1];
  int listinterviewidx0=p_Dpb->listinterviewidx0;
  int listinterviewidx1=p_Dpb->listinterviewidx1;

  StorablePicture **listX[2]={NULL,NULL};
  int list0size=0;
  int list1size=0;


  int i=0,j=0;

  int CmdLengthForReorder=(int)strlen(p_Vid->MVCReorder);



  if(NumOfList0InterReference)
    alloc_ref_pic_list_reordering_buffer(currSlice);
  if(listinterviewidx0||listinterviewidx1)
    alloc_interview_ref_pic_list_reordering_buffer(currSlice);

  if((currSlice->slice_type!=I_SLICE)&&(currSlice->slice_type!=SI_SLICE))
  {
    if(voidx)
      listX[0]=(StorablePicture**)calloc(MAX_LIST_SIZE,sizeof(StorablePicture*));
    if(NumOfList0InterReference>0)
    {
      currSlice->poc_ref_pic_reorder_frame(currSlice,currSlice->listX[LIST_0],
        currSlice->num_ref_idx_active[LIST_0],
        currSlice->reordering_of_pic_nums_idc[LIST_0],
        currSlice->abs_diff_pic_num_minus1[LIST_0],
        currSlice->long_term_pic_idx[LIST_0],
        LIST_0);
      reorder_ref_pic_list ( currSlice, LIST_0);
    }
    if(listinterviewidx0>0)
    {
      interview_ref_pic_reorder_frame_mvc(currSlice,p_Dpb->fs_listinterview0,listinterviewidx0,
        currSlice->reordering_of_interview_num_idc[0],currSlice->abs_diff_view_idx_minus1[0],0);
    }
  }
  if(currSlice->slice_type==B_SLICE)
  {  
    if(voidx)
      listX[1]=(StorablePicture**)calloc(MAX_LIST_SIZE,sizeof(StorablePicture*));
    if(NumOfList1InterReference>0)
    {
      currSlice->poc_ref_pic_reorder_frame(currSlice,currSlice->listX[LIST_1],
        currSlice->num_ref_idx_active[LIST_1],
        currSlice->reordering_of_pic_nums_idc[LIST_1],
        currSlice->abs_diff_pic_num_minus1[LIST_1],
        currSlice->long_term_pic_idx[LIST_1],
        LIST_1);
      reorder_ref_pic_list ( currSlice, LIST_1);
    }
    if(listinterviewidx1)
    {
      interview_ref_pic_reorder_frame_mvc(currSlice,p_Dpb->fs_listinterview1,listinterviewidx1,
        currSlice->reordering_of_interview_num_idc[1],currSlice->abs_diff_view_idx_minus1[1],1);
    }
  }

  if(voidx)
  {

  
    for(j=0;j<CmdLengthForReorder;++j)
    {
      switch(p_Vid->MVCReorder[j])
      {
      case '0':
        if((p_Vid->type!=I_SLICE)&&(p_Vid->type!=SI_SLICE))
        {
          for(i=0;i<currSlice->listXsize[0];++i)
          {
            listX[0][list0size++]=currSlice->listX[0][i];
          }
        }
        if(p_Vid->type==B_SLICE)
        {
          for(i=0;i<currSlice->listXsize[1];++i)
          {
            listX[1][list1size++]=currSlice->listX[1][i];
          }
        }
        break;
      case '1':
        if((p_Vid->type!=I_SLICE)&&(p_Vid->type!=SI_SLICE))
        {
          for(i=0;i<listinterviewidx0;++i)
          {
            listX[0][list0size++]=p_Dpb->fs_listinterview0[i]->frame;
          }
        }
        if(p_Vid->type==B_SLICE)
        {
          for(i=0;i<listinterviewidx1;++i)
          {
            listX[1][list1size++]=p_Dpb->fs_listinterview1[i]->frame;
          }
        }
        break;
#if EXT3D
      // @DT: Cmd '2' was to insert VSP ref into the ref picture lists
      case '2':
        printf("Warning: You may use a legacy configuration file. Set ReorderAtAnchor/ReorderAtNonAnchor as \"01\" instead of \"012\".\n");
        break;
#endif
      default:
        assert(0);
      }
    }

    for(i=0;i<list0size;++i)
      currSlice->listX[0][i]=listX[0][i];
    currSlice->listXsize[0]=(char)list0size;

    for(i=0;i<list1size;++i)
      currSlice->listX[1][i]=listX[1][i];
    currSlice->listXsize[1]=(char)list1size;


    //!<we always enable spatial direct for non-base views  
    currSlice->direct_spatial_mv_pred_flag=1;
  }
  if(listX[0])
    free(listX[0]);
  if(listX[1])
    free(listX[1]);

  currSlice->num_ref_idx_active[LIST_0] = currSlice->listXsize[LIST_0];
  currSlice->num_ref_idx_active[LIST_1] = currSlice->listXsize[LIST_1];

  if((p_Vid->type!=I_SLICE)&&(p_Vid->type!=SI_SLICE))
  {
    for(i=currSlice->listXsize[LIST_0];i<MAX_LIST_SIZE;++i)
      currSlice->listX[0][i]=NULL;
  }
  if(p_Vid->type==B_SLICE)
  {
    for(i=currSlice->listXsize[LIST_1];i<MAX_LIST_SIZE;++i)
      currSlice->listX[1][i]=NULL;
  }
}
#endif

/*!
************************************************************************
* \brief
*    decide reference picture reordering, Frame only
************************************************************************
*/
void poc_ref_pic_reorder_frame_default(Slice *currSlice, StorablePicture **list, 
                               unsigned num_ref_idx_lX_active, int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;
  StorablePicture *p_Enc_Pic = p_Vid->enc_picture;
  DecodedPictureBuffer *p_Dpb = currSlice->p_Dpb;

  unsigned int i,j,k;

  int currPicNum, picNumLXPred;

  int default_order[32];
  int re_order[32];
  int tmp_reorder[32];
  int list_sign[32];
#if !EXT3D
  int reorder_stop;
#endif
  int no_reorder;
  int poc_diff[32];
  int tmp_value, diff;

  int abs_poc_dist;
  int maxPicNum;
  unsigned int num_refs;

  UNREFERENCED_PARAMETER(long_term_pic_idx);

  maxPicNum  = p_Vid->max_frame_num;
  currPicNum = p_Vid->frame_num;

  picNumLXPred = currPicNum;

  // First assign default list order.
  for (i=0; i<num_ref_idx_lX_active; i++)
  {
    default_order[i] = list[i]->pic_num;
  }

  // Now access all references in buffer and assign them
  // to a potential reordering list. For each one of these
  // references compute the poc distance compared to current
  // frame.
  num_refs = currSlice->p_Dpb->ref_frames_in_buffer;
  for (i = 0; i < p_Dpb->ref_frames_in_buffer; i++)
  {
    poc_diff[i] = 0xFFFF;
    re_order[i] = p_Dpb->fs_ref[i]->frame->pic_num;
#if EXT3D
    if (p_Dpb->fs_ref[i]->is_used==3 && (p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term)
      && ( p_Dpb->fs_ref[i]->view_id==p_Vid->view_id))
#else
#if (MVC_EXTENSION_ENABLE)
    if (p_Dpb->fs_ref[i]->is_used==3 && (p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term)
      && (p_Inp->num_of_views==1 || p_Dpb->fs_ref[i]->view_id==p_Vid->view_id))
#else
    if (p_Dpb->fs_ref[i]->is_used==3 && (p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term))
#endif
#endif

    {
      abs_poc_dist = iabs(p_Dpb->fs_ref[i]->frame->poc - p_Enc_Pic->poc) ;
      poc_diff[i] = abs_poc_dist;
      if (list_no == LIST_0)
      {
        list_sign[i] = (p_Enc_Pic->poc < p_Dpb->fs_ref[i]->frame->poc) ? +1 : -1;
      }
      else
      {
        list_sign[i] = (p_Enc_Pic->poc > p_Dpb->fs_ref[i]->frame->poc) ? +1 : -1;
      }
    }
  }

  // now sort these references based on poc (temporal) distance
  for (i = 0; i < (num_refs - 1); i++)
  {
    for (j = i + 1; j < num_refs; j++)
    {
      if (poc_diff[i]>poc_diff[j] || (poc_diff[i] == poc_diff[j] && list_sign[j] > list_sign[i]))
      {

        tmp_value = poc_diff[i];
        poc_diff[i] = poc_diff[j];
        poc_diff[j] = tmp_value;
        tmp_value  = re_order[i];
        re_order[i] = re_order[j];
        re_order[j] = tmp_value ;
        tmp_value  = list_sign[i];
        list_sign[i] = list_sign[j];
        list_sign[j] = tmp_value;
      }
    }
  }

  // populate list with selections from the pre-analysis stage
  if ( p_Inp->WPMCPrecision 
    && p_Vid->pWPX->curr_wp_rd_pass->algorithm != WP_REGULAR
    && p_Vid->pWPX->num_wp_ref_list[list_no] )
  {
    for (i=0; i<num_ref_idx_lX_active; i++)
    {
      re_order[i] = p_Vid->pWPX->wp_ref_list[list_no][i].PicNum;
    }
  }

  // Check versus default list to see if any
  // change has happened
#if EXT3D
  //!Wenyi >>

  //!<<Force the RPLR for implicitly signaling the reference order for decoder.
  no_reorder = 0;
#else
  no_reorder = 1;

  for (i=0; i<num_ref_idx_lX_active; i++)
  {
    if (default_order[i] != re_order[i])
    {
      no_reorder = 0;
    }
  }
#endif

  // If different, then signal reordering
  if (no_reorder==0)
  {
    for (i=0; i<num_ref_idx_lX_active; i++)
    {
      diff = re_order[i]-picNumLXPred;
      if (diff <= 0)
      {
        reordering_of_pic_nums_idc[i] = 0;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
        if (abs_diff_pic_num_minus1[i] < 0)
          abs_diff_pic_num_minus1[i] = maxPicNum -1;
      }
      else
      {
        reordering_of_pic_nums_idc[i] = 1;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
      }
      picNumLXPred = re_order[i];
      tmp_reorder[i] = re_order[i];

      k = i;
      for (j=i; j<num_ref_idx_lX_active; j++)
      {
        if (default_order[j] != re_order[i])
        {
          ++k;
          tmp_reorder[k] = default_order[j];
        }
      }
#if(!EXT3D)
      reorder_stop = 1;
      for(j=i+1; j<num_ref_idx_lX_active; j++)
      {
        if (tmp_reorder[j] != re_order[j])
        {
          reorder_stop = 0;
          break;
        }
      }

      if (reorder_stop==1)
      {
        ++i;
        break;
      }
#endif
      memcpy ( default_order, tmp_reorder, num_ref_idx_lX_active * sizeof(int));

    }
    reordering_of_pic_nums_idc[i] = 3;

    memcpy ( default_order, tmp_reorder, num_ref_idx_lX_active * sizeof(int));

    if (list_no==0)
    {
      currSlice->ref_pic_list_reordering_flag[LIST_0] = 1;
    }
    else
    {
      currSlice->ref_pic_list_reordering_flag[LIST_1] = 1;
    }
  }
}

/*!
************************************************************************
* \brief
*    decide reference picture reordering, Field only
************************************************************************
*/
void poc_ref_pic_reorder_field(Slice *currSlice, StorablePicture **list, unsigned num_ref_idx_lX_active, int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  //InputParameters *p_Inp = currSlice->p_Inp;
  StorablePicture *p_Enc_Pic = p_Vid->enc_picture;
  DecodedPictureBuffer *p_Dpb = currSlice->p_Dpb;


  unsigned int i,j,k;

  int currPicNum, picNumLXPred;

  int default_order[32];
  int re_order[32];
  int tmp_reorder[32];
  int list_sign[32];  
  int poc_diff[32];
  int fld_type[32];

#if !EXT3D
  int reorder_stop;
#endif
  int no_reorder;
  int tmp_value, diff;

  int abs_poc_dist;
  int maxPicNum;
  unsigned int num_refs;

  int field_used[2] = {1, 2};
  int fld, idx, num_flds;

  unsigned int top_idx = 0;
  unsigned int bot_idx = 0;
  unsigned int list_size = 0;

  StorablePicture *pField[2]; // 0: TOP_FIELD, 1: BOTTOM_FIELD
  FrameStore      *pFrameStore; 

  UNREFERENCED_PARAMETER(long_term_pic_idx);


  maxPicNum  = 2 * p_Vid->max_frame_num;
  currPicNum = 2 * p_Vid->frame_num + 1;

  picNumLXPred = currPicNum;

  // First assign default list order.
  for (i=0; i<num_ref_idx_lX_active; i++)
  {
    default_order[i] = list[i]->pic_num;
  }

  // Now access all references in buffer and assign them
  // to a potential reordering list. For each one of these
  // references compute the poc distance compared to current
  // frame.  
  // look for eligible fields
  idx = 0;

  for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
  {
    pFrameStore = p_Dpb->fs_ref[i];
    pField[0]   = pFrameStore->top_field;
    pField[1]   = pFrameStore->bottom_field;
    num_flds    = (currSlice->structure == BOTTOM_FIELD && (p_Enc_Pic->poc == (pField[0]->poc + 1) ) ) ? 1 : 2;

    poc_diff[2*i    ] = 0xFFFF;
    poc_diff[2*i + 1] = 0xFFFF;

    for ( fld = 0; fld < num_flds; fld++ )
    {
#if (MVC_EXTENSION_ENABLE)
      if ( (pFrameStore->is_used & field_used[fld]) && pField[fld]->used_for_reference && !(pField[fld]->is_long_term) 
        && (p_Vid->p_Inp->num_of_views==1 || pField[fld]->view_id==p_Vid->view_id))
#else
      if ( (pFrameStore->is_used & field_used[fld]) && pField[fld]->used_for_reference && !(pField[fld]->is_long_term))
#endif
      {
        abs_poc_dist = iabs(pField[fld]->poc - p_Enc_Pic->poc) ;
        poc_diff[idx] = abs_poc_dist;
        re_order[idx] = pField[fld]->pic_num;
        fld_type[idx] = fld + 1;

        if (list_no == LIST_0)
        {
          list_sign[idx] = (p_Enc_Pic->poc < pField[fld]->poc) ? +1 : -1;
        }
        else
        {
          list_sign[idx] = (p_Enc_Pic->poc > pField[fld]->poc) ? +1 : -1;
        }
        idx++;
      }
    }
  }
  num_refs = idx;

  // now sort these references based on poc (temporal) distance
  for (i=0; i < num_refs-1; i++)
  {
    for (j = (i + 1); j < num_refs; j++)
    {
      if (poc_diff[i] > poc_diff[j] || (poc_diff[i] == poc_diff[j] && list_sign[j] > list_sign[i]))
      {
        // poc_diff
        tmp_value   = poc_diff[i];
        poc_diff[i] = poc_diff[j];
        poc_diff[j] = tmp_value;
        // re_order (PicNum)
        tmp_value   = re_order[i];
        re_order[i] = re_order[j];
        re_order[j] = tmp_value;
        // list_sign
        tmp_value    = list_sign[i];
        list_sign[i] = list_sign[j];
        list_sign[j] = tmp_value;
        // fld_type
        tmp_value   = fld_type[i];
        fld_type[i] = fld_type[j];
        fld_type[j] = tmp_value ;
      }
    }
  }

  if (currSlice->structure == TOP_FIELD)
  {
    while ((top_idx < num_refs)||(bot_idx < num_refs))
    {
      for ( ; top_idx < num_refs; top_idx++)
      {
        if ( fld_type[top_idx] == TOP_FIELD )
        {
          tmp_reorder[list_size] = re_order[top_idx];
          list_size++;
          top_idx++;
          break;
        }
      }
      for ( ; bot_idx < num_refs; bot_idx++)
      {
        if ( fld_type[bot_idx] == BOTTOM_FIELD )
        {
          tmp_reorder[list_size] = re_order[bot_idx];
          list_size++;
          bot_idx++;
          break;
        }
      }
    }
  }
  if (currSlice->structure == BOTTOM_FIELD)
  {
    while ((top_idx < num_refs)||(bot_idx < num_refs))
    {
      for ( ; bot_idx < num_refs; bot_idx++)
      {
        if ( fld_type[bot_idx] == BOTTOM_FIELD )
        {
          tmp_reorder[list_size] = re_order[bot_idx];
          list_size++;
          bot_idx++;
          break;
        }
      }
      for ( ; top_idx < num_refs; top_idx++)
      {
        if ( fld_type[top_idx] == TOP_FIELD )
        {
          tmp_reorder[list_size] = re_order[top_idx];
          list_size++;
          top_idx++;
          break;
        }
      }
    }
  }

  // copy to final matrix
  list_size = imin( list_size, 32 );
  for ( i = 0; i < list_size; i++ )
  {
    re_order[i] = tmp_reorder[i];
  }

  // Check versus default list to see if any
  // change has happened
#if EXT3D
  //!<force the RPLR
  no_reorder=0;
#else
  no_reorder = 1;
  for (i=0; i<num_ref_idx_lX_active; i++)
  {
    if (default_order[i] != re_order[i])
    {
      no_reorder = 0;
    }
  }
#endif

  // If different, then signal reordering
  if (no_reorder == 0)
  {
    for (i=0; i<num_ref_idx_lX_active; i++)
    {
      diff = re_order[i] - picNumLXPred;
      if (diff <= 0)
      {
        reordering_of_pic_nums_idc[i] = 0;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
        if (abs_diff_pic_num_minus1[i] < 0)
          abs_diff_pic_num_minus1[i] = maxPicNum -1;
      }
      else
      {
        reordering_of_pic_nums_idc[i] = 1;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
      }
      picNumLXPred = re_order[i];

      tmp_reorder[i] = re_order[i];

      k = i;
      for (j = i; j < num_ref_idx_lX_active; j++)
      {
        if (default_order[j] != re_order[i])
        {
          ++k;
          tmp_reorder[k] = default_order[j];
        }
      }
#if(!EXT3D)
      reorder_stop = 1;
      for(j=i+1; j<num_ref_idx_lX_active; j++)
      {
        if (tmp_reorder[j] != re_order[j])
        {
          reorder_stop = 0;
          break;
        }
      }

      if (reorder_stop==1)
      {
        ++i;
        break;
      }
#endif

      memcpy ( default_order, tmp_reorder, num_ref_idx_lX_active * sizeof(int));
    }
    reordering_of_pic_nums_idc[i] = 3;

    memcpy ( default_order, tmp_reorder, num_ref_idx_lX_active * sizeof(int));

    if (list_no==0)
    {
      currSlice->ref_pic_list_reordering_flag[LIST_0] = 1;
    }
    else
    {
      currSlice->ref_pic_list_reordering_flag[LIST_1] = 1;
    }
  }
}


/*!
************************************************************************
* \brief
*    decide reference picture reordering based on temporal layer, Frame only
************************************************************************
*/
void tlyr_ref_pic_reorder_frame_default(Slice *currSlice, StorablePicture **list, 
                               unsigned num_ref_idx_lX_active, int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;
  StorablePicture *p_Enc_Pic = p_Vid->enc_picture;

  unsigned int i,j,k;

  int currPicNum, picNumLXPred;

  int default_order[32];
  int re_order[32];
  int tmp_reorder[32];
  int list_sign[32];
#if !EXT3D
  int reorder_stop;
#endif
  int no_reorder;
  int poc_diff[32];

  int temporal_layer[32];

  int diff;

  int abs_poc_dist;
  int maxPicNum;
  unsigned int num_refs;

  int tmp_poc_diff, tmp_re_order, tmp_list_sign, tmp_temporal_layer;
  unsigned int beginIdx = 0;
  char newSize = 0;

  UNREFERENCED_PARAMETER(long_term_pic_idx);


  maxPicNum  = p_Vid->max_frame_num;
  currPicNum = p_Vid->frame_num;

  picNumLXPred = currPicNum;

  // First assign default list order.
  for (i=0; i<num_ref_idx_lX_active; i++)
  {
    default_order[i] = list[i]->pic_num;
  }

  // Now access all references in buffer and assign them
  // to a potential reordering list. For each one of these
  // references compute the poc distance compared to current
  // frame.
  num_refs = p_Vid->p_Dpb->ref_frames_in_buffer;
  for (i=0; i<p_Vid->p_Dpb->ref_frames_in_buffer; i++)
  {
    poc_diff[i] = 0xFFFF;
    re_order[i] = p_Vid->p_Dpb->fs_ref[i]->frame->pic_num;

    temporal_layer[i] = p_Vid->p_Dpb->fs_ref[i]->frame->temporal_layer; 

    if (p_Vid->p_Dpb->fs_ref[i]->is_used==3 && (p_Vid->p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Vid->p_Dpb->fs_ref[i]->frame->is_long_term))
    {
      abs_poc_dist = iabs(p_Vid->p_Dpb->fs_ref[i]->frame->poc - p_Enc_Pic->poc) ;
      poc_diff[i] = abs_poc_dist;
      if (list_no == LIST_0)
      {
        list_sign[i] = (p_Enc_Pic->poc < p_Vid->p_Dpb->fs_ref[i]->frame->poc) ? +1 : -1;
      }
      else
      {
        list_sign[i] = (p_Enc_Pic->poc > p_Vid->p_Dpb->fs_ref[i]->frame->poc) ? +1 : -1;
      }
    }
  }

  // first, sort the references based on poc (temporal) distance 
  for (i = 0; i < (num_refs - 1); i++)
  {
    for (j = i + 1; j < num_refs; j++)
    {

#if EXT3D
      if((list_sign[i]>list_sign[j])||(list_sign[i] == list_sign[j] && poc_diff[i] > poc_diff[j]))
#else
      if (list_sign[i] == list_sign[j] && poc_diff[i] > poc_diff[j])
#endif
      {
        tmp_poc_diff = poc_diff[i];
        tmp_re_order = re_order[i];
        tmp_list_sign = list_sign[i];
        tmp_temporal_layer = temporal_layer[i];
        poc_diff[i] = poc_diff[j];
        re_order[i] = re_order[j];
        list_sign[i] = list_sign[j];
        temporal_layer[i] = temporal_layer[j];
        poc_diff[j] = tmp_poc_diff;
        re_order[j] = tmp_re_order ;
        list_sign[j] = tmp_list_sign;
        temporal_layer[j] = tmp_temporal_layer;
      }
    }
  }

  // use the closest valid reference frame + 3 temporal layer 0 frames
  for (i = 0; i < num_refs; i++)
  {  
    /*
    if ( (i == 0 && temporal_layer[i] <= p_Enc_Pic->temporal_layer) ||
         (i && !temporal_layer[i]) ) // valid reference pic
    */
#if EXT3D // ADAPTIVE_MMCO_REORDER
    if (p_Inp->DisableTLRefReorder || temporal_layer[i]==0 || temporal_layer[i] < p_Enc_Pic->temporal_layer || ( p_Inp->TLRefReorderMethod==0 && temporal_layer[i] == p_Enc_Pic->temporal_layer))
#else
    if (temporal_layer[i] <= p_Enc_Pic->temporal_layer) 
#endif
    {
      tmp_poc_diff = poc_diff[i];
      tmp_re_order = re_order[i];
      tmp_list_sign = list_sign[i];
      tmp_temporal_layer = temporal_layer[i];
      for (j = i; j > beginIdx; j--) 
      {
        poc_diff[j] = poc_diff[j-1];
        re_order[j] = re_order[j-1];
        list_sign[j] = list_sign[j-1];
        temporal_layer[j] = temporal_layer[j-1];
      }                   
      poc_diff[beginIdx] = tmp_poc_diff;
      re_order[beginIdx] = tmp_re_order;
      list_sign[beginIdx] = tmp_list_sign;
      temporal_layer[beginIdx] = tmp_temporal_layer;
      beginIdx += 1;

      newSize++;
    }
  }
#if EXT3D
  num_ref_idx_lX_active=currSlice->listXsize[list_no]=(char)imin(newSize,num_ref_idx_lX_active);
#else
  currSlice->listXsize[0] = newSize; // update with valid size
#endif

  // populate list with selections from the pre-analysis stage
  if ( p_Inp->WPMCPrecision 
    && p_Vid->pWPX->curr_wp_rd_pass->algorithm != WP_REGULAR
    && p_Vid->pWPX->num_wp_ref_list[list_no] )
  {
    for (i=0; i<num_ref_idx_lX_active; i++)
    {
      re_order[i] = p_Vid->pWPX->wp_ref_list[list_no][i].PicNum;
    }
  }

  // Check versus default list to see if any
  // change has happened
#if EXT3D
  no_reorder = 0;
#else
  no_reorder = 1;

  for (i=0; i<num_ref_idx_lX_active; i++)
  {
    if (default_order[i] != re_order[i])
    {
      no_reorder = 0;
    }
  }
#endif

  // If different, then signal reordering
  if (no_reorder==0)
  {
    for (i=0; i<num_ref_idx_lX_active; i++)
    {
      diff = re_order[i]-picNumLXPred;
      if (diff <= 0)
      {
        reordering_of_pic_nums_idc[i] = 0;

        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
        if (abs_diff_pic_num_minus1[i] < 0)
          abs_diff_pic_num_minus1[i] = maxPicNum -1;
      }
      else
      {
        reordering_of_pic_nums_idc[i] = 1;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
      }
      picNumLXPred = re_order[i];

      tmp_reorder[i] = re_order[i];

      k = i;
      for (j=i; j<num_ref_idx_lX_active; j++)
      {
        if (default_order[j] != re_order[i])
        {
          ++k;
          tmp_reorder[k] = default_order[j];
        }
      }
#if(!EXT3D)
      reorder_stop = 1;
      for(j=i+1; j<num_ref_idx_lX_active; j++)
      {
        if (tmp_reorder[j] != re_order[j])
        {
          reorder_stop = 0;
          break;
        }
      }

      if (reorder_stop==1)
      {
        ++i;
        break;
      }
#endif
      memcpy ( default_order, tmp_reorder, num_ref_idx_lX_active * sizeof(int));

    }
    reordering_of_pic_nums_idc[i] = 3;

    memcpy ( default_order, tmp_reorder, num_ref_idx_lX_active * sizeof(int));

    if (list_no==0)
    {
      currSlice->ref_pic_list_reordering_flag[LIST_0] = 1;
    }
    else
    {
      currSlice->ref_pic_list_reordering_flag[LIST_1] = 1;
    }
  }
}

/*!
************************************************************************
* \brief
*    Reorder lists
************************************************************************
*/

void reorder_lists( Slice *currSlice )
{
  InputParameters *p_Inp = currSlice->p_Inp;

  // Perform reordering based on poc distances for HierarchicalCoding
  if ( p_Inp->ReferenceReorder && currSlice->slice_type == P_SLICE )
  {
    int i, num_ref;

    alloc_ref_pic_list_reordering_buffer(currSlice);

    for (i = 0; i < currSlice->num_ref_idx_active[LIST_0] + 1; i++)
    {
      currSlice->reordering_of_pic_nums_idc[LIST_0][i] = 3;
      currSlice->abs_diff_pic_num_minus1[LIST_0][i] = 0;
      currSlice->long_term_pic_idx[LIST_0][i] = 0;
    }

    num_ref = currSlice->num_ref_idx_active[LIST_0];

    if ( p_Inp->ReferenceReorder == 2 ) 
    { // temporal layer based
      if ( currSlice->structure == FRAME )
        currSlice->poc_ref_pic_reorder_frame(currSlice, currSlice->listX[LIST_0], num_ref,
        currSlice->reordering_of_pic_nums_idc[LIST_0],
        currSlice->abs_diff_pic_num_minus1[LIST_0],
        currSlice->long_term_pic_idx[LIST_0], LIST_0);
    }
    else
    {
    if ( currSlice->structure == FRAME )
      currSlice->poc_ref_pic_reorder_frame(currSlice, currSlice->listX[LIST_0], num_ref,
      currSlice->reordering_of_pic_nums_idc[LIST_0],
      currSlice->abs_diff_pic_num_minus1[LIST_0],
      currSlice->long_term_pic_idx[LIST_0], LIST_0);
    else
    {
      poc_ref_pic_reorder_field(currSlice, currSlice->listX[LIST_0], num_ref,
        currSlice->reordering_of_pic_nums_idc[LIST_0],
        currSlice->abs_diff_pic_num_minus1[LIST_0],
        currSlice->long_term_pic_idx[LIST_0], LIST_0);
    }
    } 

    currSlice->num_ref_idx_active[LIST_0] = currSlice->listXsize[0]; 
    //reference picture reordering
    reorder_ref_pic_list ( currSlice, LIST_0);
  }
   currSlice->num_ref_idx_active[LIST_0] = currSlice->listXsize[0]; 
}

/*!
************************************************************************
* \brief
*    Reorder lists
************************************************************************
*/

void wp_mcprec_reorder_lists( Slice *currSlice )
{

  int i, num_ref;

  wpxModifyRefPicList( currSlice );

  alloc_ref_pic_list_reordering_buffer(currSlice);

  for (i = 0; i < currSlice->num_ref_idx_active[LIST_0] + 1; i++)
  {
    currSlice->reordering_of_pic_nums_idc[LIST_0][i] = 3;
    currSlice->abs_diff_pic_num_minus1[LIST_0][i] = 0;
    currSlice->long_term_pic_idx[LIST_0][i] = 0;
  }

  if (currSlice->slice_type == B_SLICE) // type should be part of currSlice not p_Vid
  {
    for (i = 0; i < currSlice->num_ref_idx_active[LIST_1] + 1; i++)
    {
      currSlice->reordering_of_pic_nums_idc[LIST_1][i] = 3;
      currSlice->abs_diff_pic_num_minus1[LIST_1][i] = 0;
      currSlice->long_term_pic_idx[LIST_1][i] = 0;
    }
  }

  // LIST_0
  num_ref = currSlice->num_ref_idx_active[LIST_0];

  currSlice->poc_ref_pic_reorder_frame(currSlice, currSlice->listX[LIST_0], num_ref,
    currSlice->reordering_of_pic_nums_idc[LIST_0],
    currSlice->abs_diff_pic_num_minus1[LIST_0],
    currSlice->long_term_pic_idx[LIST_0], LIST_0);
  // reference picture reordering
  reorder_ref_pic_list ( currSlice, LIST_0);

  if ( currSlice->slice_type == B_SLICE )
  {
    // LIST_1
    num_ref = currSlice->num_ref_idx_active[LIST_1];

    currSlice->poc_ref_pic_reorder_frame(currSlice, currSlice->listX[LIST_1], num_ref,
      currSlice->reordering_of_pic_nums_idc[LIST_1],
      currSlice->abs_diff_pic_num_minus1[LIST_1],
      currSlice->long_term_pic_idx[LIST_1], LIST_1);
    // reference picture reordering
    reorder_ref_pic_list ( currSlice, LIST_1);
  }
}

