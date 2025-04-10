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
 * \file mv_prediction.c
 *
 * \brief
 *    Motion Vector Prediction Functions
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *      - Karsten Sühring          <suehring@hhi.de>
 *************************************************************************************
 */

#include "global.h"
#include "mbuffer.h"
#if EXT3D
#include "mv_prediction.h"
#endif

/*!
 ************************************************************************
 * \brief
 *    Get motion vector predictor
 ************************************************************************
 */
static void GetMotionVectorPredictorMBAFF (Macroblock *currMB, 
                                    PixelPos *block,        // <--> block neighbors
                                    MotionVector *pmv,
                                    short  ref_frame,
                                    PicMotionParams **mv_info,
                                    int    list,
                                    int    mb_x,
                                    int    mb_y,
                                    int    blockshape_x,
                                    int    blockshape_y
#if EXT3D
                                  , int    depth_based_mvp
#endif
)
{
  int mv_a, mv_b, mv_c, pred_vec=0;
  int mvPredType, rFrameL, rFrameU, rFrameUR;
  int hv;
  VideoParameters *p_Vid = currMB->p_Vid;

  mvPredType = MVPRED_MEDIAN;
#if EXT3D
  UNREFERENCED_PARAMETER(depth_based_mvp);
#endif

  if (currMB->mb_field)
  {
    rFrameL  = block[0].available
      ? (p_Vid->mb_data[block[0].mb_addr].mb_field
      ? mv_info[block[0].pos_y][block[0].pos_x].ref_idx[list]
    : mv_info[block[0].pos_y][block[0].pos_x].ref_idx[list] * 2) : -1;
    rFrameU  = block[1].available
      ? (p_Vid->mb_data[block[1].mb_addr].mb_field
      ? mv_info[block[1].pos_y][block[1].pos_x].ref_idx[list]
    : mv_info[block[1].pos_y][block[1].pos_x].ref_idx[list] * 2) : -1;
    rFrameUR = block[2].available
      ? (p_Vid->mb_data[block[2].mb_addr].mb_field
      ? mv_info[block[2].pos_y][block[2].pos_x].ref_idx[list]
    : mv_info[block[2].pos_y][block[2].pos_x].ref_idx[list] * 2) : -1;
  }
  else
  {
    rFrameL = block[0].available
      ? (p_Vid->mb_data[block[0].mb_addr].mb_field
      ? mv_info[block[0].pos_y][block[0].pos_x].ref_idx[list] >>1
      : mv_info[block[0].pos_y][block[0].pos_x].ref_idx[list]) : -1;
    rFrameU  = block[1].available
      ? (p_Vid->mb_data[block[1].mb_addr].mb_field
      ? mv_info[block[1].pos_y][block[1].pos_x].ref_idx[list] >>1
      : mv_info[block[1].pos_y][block[1].pos_x].ref_idx[list]) : -1;
    rFrameUR = block[2].available
      ? (p_Vid->mb_data[block[2].mb_addr].mb_field
      ? mv_info[block[2].pos_y][block[2].pos_x].ref_idx[list] >>1
      : mv_info[block[2].pos_y][block[2].pos_x].ref_idx[list]) : -1;
  }


  /* Prediction if only one of the neighbors uses the reference frame
  *  we are checking
  */
  if(rFrameL == ref_frame && rFrameU != ref_frame && rFrameUR != ref_frame)       
    mvPredType = MVPRED_L;
  else if(rFrameL != ref_frame && rFrameU == ref_frame && rFrameUR != ref_frame)  
    mvPredType = MVPRED_U;
  else if(rFrameL != ref_frame && rFrameU != ref_frame && rFrameUR == ref_frame)  
    mvPredType = MVPRED_UR;
  // Directional predictions
  if(blockshape_x == 8 && blockshape_y == 16)
  {
    if(mb_x == 0)
    {
      if(rFrameL == ref_frame)
        mvPredType = MVPRED_L;
    }
    else
    {
      if( rFrameUR == ref_frame)
        mvPredType = MVPRED_UR;
    }
  }
  else if(blockshape_x == 16 && blockshape_y == 8)
  {
    if(mb_y == 0)
    {
      if(rFrameU == ref_frame)
        mvPredType = MVPRED_U;
    }
    else
    {
      if(rFrameL == ref_frame)
        mvPredType = MVPRED_L;
    }
  }

  for (hv=0; hv < 2; hv++)
  {
    if (hv == 0)
    {
      mv_a = block[0].available ? mv_info[block[0].pos_y][block[0].pos_x].mv[list].mv_x : 0;
      mv_b = block[1].available ? mv_info[block[1].pos_y][block[1].pos_x].mv[list].mv_x : 0;
      mv_c = block[2].available ? mv_info[block[2].pos_y][block[2].pos_x].mv[list].mv_x : 0;
    }
    else
    {
      if (currMB->mb_field)
      {
        mv_a = block[0].available  ? p_Vid->mb_data[block[0].mb_addr].mb_field
          ? mv_info[block[0].pos_y][block[0].pos_x].mv[list].mv_y
        : mv_info[block[0].pos_y][block[0].pos_x].mv[list].mv_y / 2
          : 0;
        mv_b = block[1].available  ? p_Vid->mb_data[block[1].mb_addr].mb_field
          ? mv_info[block[1].pos_y][block[1].pos_x].mv[list].mv_y
        : mv_info[block[1].pos_y][block[1].pos_x].mv[list].mv_y / 2
          : 0;
        mv_c = block[2].available  ? p_Vid->mb_data[block[2].mb_addr].mb_field
          ? mv_info[block[2].pos_y][block[2].pos_x].mv[list].mv_y
        : mv_info[block[2].pos_y][block[2].pos_x].mv[list].mv_y / 2
          : 0;
      }
      else
      {
        mv_a = block[0].available  ? p_Vid->mb_data[block[0].mb_addr].mb_field
          ? mv_info[block[0].pos_y][block[0].pos_x].mv[list].mv_y * 2
          : mv_info[block[0].pos_y][block[0].pos_x].mv[list].mv_y
        : 0;
        mv_b = block[1].available  ? p_Vid->mb_data[block[1].mb_addr].mb_field
          ? mv_info[block[1].pos_y][block[1].pos_x].mv[list].mv_y * 2
          : mv_info[block[1].pos_y][block[1].pos_x].mv[list].mv_y
        : 0;
        mv_c = block[2].available  ? p_Vid->mb_data[block[2].mb_addr].mb_field
          ? mv_info[block[2].pos_y][block[2].pos_x].mv[list].mv_y * 2
          : mv_info[block[2].pos_y][block[2].pos_x].mv[list].mv_y
        : 0;
      }
    }

    switch (mvPredType)
    {
    case MVPRED_MEDIAN:
      if(!(block[1].available || block[2].available))
      {
        pred_vec = mv_a;
      }
      else
      {
        pred_vec = imedian(mv_a, mv_b, mv_c);
      }
      break;
    case MVPRED_L:
      pred_vec = mv_a;
      break;
    case MVPRED_U:
      pred_vec = mv_b;
      break;
    case MVPRED_UR:
      pred_vec = mv_c;
      break;
    default:
      break;
    }

    if (hv == 0)
      pmv->mv_x = (short) pred_vec;
    else
      pmv->mv_y = (short) pred_vec;
  }
}
#if EXT3D
/*!
************************************************************************
* \brief
*    Get motion vector predictor for inter-view reference
************************************************************************
*/
static void GetInterViewMotionVectorePredictionNormal(Macroblock*currMB,
  MotionVector*pmv,
  int mb_y,
  int mb_x,
  int blockshape_y,
  int blockshape_x,
  int* disparity_table)
{
  VideoParameters* p_Vid=currMB->p_Vid;
  imgpel**img_Y_depth = p_Vid->isTextureFirst ? NULL : p_Vid->p_dual_picture->imgY;
  int threshold=12;
  
  int viewid=currMB->p_Slice->view_id;
  int grid_posx=p_Vid->grid_pos_x[viewid];
  int grid_posy=p_Vid->grid_pos_y[viewid];

  int pix_y=iClip3(p_Vid->DepthCropTopCoord,p_Vid->DepthCropBottomCoord,(currMB->pix_y+mb_y+grid_posy) *p_Vid->depth_ver_mult>>p_Vid->depth_ver_rsh);
  int pix_x=iClip3(p_Vid->DepthCropLeftCoord,p_Vid->DepthCropRightCoord,(currMB->pix_x+mb_x+grid_posx) *p_Vid->depth_hor_mult>>p_Vid->depth_hor_rsh);
  int pix_yn=iClip3(p_Vid->DepthCropTopCoord,p_Vid->DepthCropBottomCoord,(currMB->pix_y+mb_y+blockshape_y-1+grid_posy) *p_Vid->depth_ver_mult>>p_Vid->depth_ver_rsh);
  int pix_xn=iClip3(p_Vid->DepthCropLeftCoord,p_Vid->DepthCropRightCoord,(currMB->pix_x+mb_x+blockshape_x-1+grid_posx) *p_Vid->depth_hor_mult>>p_Vid->depth_hor_rsh);

  int    Sum=0;
  int    Avg=0;
  int    sad=0;

  int j,i;

  blockshape_x= pix_xn-pix_x+1;
  blockshape_y= pix_yn-pix_y+1;

  if(p_Vid->isTextureFirst)
  {
    pmv->mv_x = currMB->iNBDV;
    pmv->mv_y = 0;
  }
  else
  {
  for(j=pix_y;j<pix_y+blockshape_y;++j)
    for(i=pix_x;i<pix_x+blockshape_x;++i)
      Sum+=img_Y_depth[j][i];
  Avg=(int)(Sum/(double)(blockshape_y*blockshape_x)+0.5);

  for(j=pix_y;j<pix_y+blockshape_y;++j)
    for(i=pix_x;i<pix_x+blockshape_x;++i)
      sad+=abs(img_Y_depth[j][i]-Avg);

  sad=sad/(blockshape_y*blockshape_x);

  if(sad<=threshold)
  {

    //int disparity= disparity_table[Avg];

    pmv->mv_x=(short)disparity_table[Avg];//!<1/4 sub-pixel precision
    pmv->mv_y=0;

  }
  else
    *pmv=zero_mv;
  }
}

/*!
************************************************************************
* \brief
*    Get motion vector predictor for inter-view reference
************************************************************************
*/
static void GetInterViewMotionVectorePredictionNormalMax(Macroblock*currMB,
                               MotionVector*pmv,
                               int mb_x,
                               int mb_y,
                               int blockshape_x,
                               int blockshape_y,
                               int* disparity_table)
{
  VideoParameters* p_Vid=currMB->p_Vid;
  imgpel**img_Y_depth=p_Vid->isTextureFirst ? NULL : p_Vid->p_dual_picture->imgY;
  int viewid=currMB->p_Slice->view_id;
  int grid_posx=p_Vid->grid_pos_x[viewid];
  int grid_posy=p_Vid->grid_pos_y[viewid];

  int pix_y=iClip3(p_Vid->DepthCropTopCoord,p_Vid->DepthCropBottomCoord,(currMB->pix_y+mb_y+grid_posy) *p_Vid->depth_ver_mult>>p_Vid->depth_ver_rsh);
  int pix_x=iClip3(p_Vid->DepthCropLeftCoord,p_Vid->DepthCropRightCoord,(currMB->pix_x+mb_x+grid_posx) *p_Vid->depth_hor_mult>>p_Vid->depth_hor_rsh);
  int pix_yn=iClip3(p_Vid->DepthCropTopCoord,p_Vid->DepthCropBottomCoord,(currMB->pix_y+mb_y+(blockshape_y-1)+grid_posy) *p_Vid->depth_ver_mult>>p_Vid->depth_ver_rsh);
  int pix_xn=iClip3(p_Vid->DepthCropLeftCoord,p_Vid->DepthCropRightCoord,(currMB->pix_x+mb_x+(blockshape_x-1)+grid_posx) *p_Vid->depth_hor_mult>>p_Vid->depth_hor_rsh);

  int depth_MB_y=pix_yn-pix_y+1;
  int depth_MB_x=pix_xn-pix_x+1;
  int max=0;
  int j,i;

  UNREFERENCED_PARAMETER(mb_x);
  UNREFERENCED_PARAMETER(mb_y);
  UNREFERENCED_PARAMETER(blockshape_x);
  UNREFERENCED_PARAMETER(blockshape_y);

  if(p_Vid->isTextureFirst)
  {
    pmv->mv_x = currMB->iNBDV;
    pmv->mv_y = 0; 
  }
  else
  {
    for(j=pix_y;j<pix_y+depth_MB_y; j+= (depth_MB_y-1)) { 
      for(i=pix_x;i<pix_x+depth_MB_x; i+=(depth_MB_x-1)) {  
        if(img_Y_depth[j][i]>max) max=img_Y_depth[j][i];
        if (depth_MB_x == 1)
          i++;
      }
      if (depth_MB_y == 1)
        j++;
    }

  pmv->mv_x=(short)disparity_table[max];
  pmv->mv_y=0;
  }
}
#endif

/*!
 ************************************************************************
 * \brief
 *    Get motion vector predictor
 ************************************************************************
 */
static void GetMotionVectorPredictorNormal (Macroblock *currMB, 
                                            PixelPos *block,      // <--> block neighbors
                                            MotionVector *pmv,
                                            short  ref_frame,
                                            PicMotionParams **mv_info,
                                            int    list,
                                            int    mb_x,
                                            int    mb_y,
                                            int    blockshape_x,
                                            int    blockshape_y
#if EXT3D
                                          , int    depth_based_mvp
#endif
)
{
  int mvPredType = MVPRED_MEDIAN;

 

#if EXT3D
  VideoParameters*p_Vid       = currMB->p_Vid;
  int ref_view_id             = currMB->p_Slice->listX[list][ref_frame]->view_id;
  int is_interview_prediction = (ref_view_id!=currMB->p_Slice->view_id);
#endif

  int rFrameL    = block[0].available ? mv_info[block[0].pos_y][block[0].pos_x].ref_idx[list] : -1;
  int rFrameU    = block[1].available ? mv_info[block[1].pos_y][block[1].pos_x].ref_idx[list] : -1;
  int rFrameUR   = block[2].available ? mv_info[block[2].pos_y][block[2].pos_x].ref_idx[list] : -1;

  /* Prediction if only one of the neighbors uses the reference frame
  *  we are checking
  */
  if(rFrameL == ref_frame && rFrameU != ref_frame && rFrameUR != ref_frame)       
    mvPredType = MVPRED_L;
  else if(rFrameL != ref_frame && rFrameU == ref_frame && rFrameUR != ref_frame)  
    mvPredType = MVPRED_U;
  else if(rFrameL != ref_frame && rFrameU != ref_frame && rFrameUR == ref_frame)  
    mvPredType = MVPRED_UR;

  // Directional predictions
  if(blockshape_x == 8 && blockshape_y == 16)
  {
    if(mb_x == 0)
    {
      if(rFrameL == ref_frame)
        mvPredType = MVPRED_L;
    }
    else
    {
      if(rFrameUR == ref_frame)
        mvPredType = MVPRED_UR;
    }
  }
  else if(blockshape_x == 16 && blockshape_y == 8)
  {
    if(mb_y == 0)
    {
      if(rFrameU == ref_frame)
        mvPredType = MVPRED_U;
    }
    else
    {
      if(rFrameL == ref_frame)
        mvPredType = MVPRED_L;
    }
  }

  switch (mvPredType)
  {
  case MVPRED_MEDIAN:
#if EXT3D  
    if(depth_based_mvp&&is_interview_prediction)
    {
      MotionVector mv_a;
      MotionVector mv_b;
      MotionVector mv_c;

      if(block[0].available && rFrameL == ref_frame) mv_a=mv_info[block[0].pos_y][block[0].pos_x].mv[list];
      else currMB->GetInterviewMVPredictionMax(currMB,&mv_a,0,0,16,16,
        (list==0)?p_Vid->forward_disparity_table:p_Vid->backward_disparity_table);

      if(block[1].available && rFrameU == ref_frame) mv_b=mv_info[block[1].pos_y][block[1].pos_x].mv[list];
      else currMB->GetInterviewMVPredictionMax(currMB,&mv_b,0,0,16,16,
        (list==0)?p_Vid->forward_disparity_table:p_Vid->backward_disparity_table);

      if(block[2].available && rFrameUR == ref_frame) mv_c=mv_info[block[2].pos_y][block[2].pos_x].mv[list];
      else currMB->GetInterviewMVPredictionMax(currMB,&mv_c,0,0,16,16,
        (list==0)?p_Vid->forward_disparity_table:p_Vid->backward_disparity_table);

      pmv->mv_x = (short) imedian(mv_a.mv_x, mv_b.mv_x, mv_c.mv_x);
      pmv->mv_y = (short) imedian(mv_a.mv_y, mv_b.mv_y, mv_c.mv_y);

      break;
    }

    if(depth_based_mvp&&!is_interview_prediction) 
    {
      MotionInfo motion_interview[2];
      MotionVector mv_a;
      MotionVector mv_b;
      MotionVector mv_c;

      if(block[0].available && rFrameL == ref_frame) mv_a=mv_info[block[0].pos_y][block[0].pos_x].mv[list];
      else 
      {
        Get_InterViewMV_TemporalMVP(currMB, mb_x, mb_y, blockshape_x, blockshape_y, list, currMB->base_view_id, motion_interview, block);
        if(ref_frame==motion_interview[list].ref_idx) mv_a=motion_interview[list].mv;
        else mv_a=zero_mv;
      }

      if(block[1].available && rFrameU == ref_frame) mv_b=mv_info[block[1].pos_y][block[1].pos_x].mv[list];
      else 
      {
        Get_InterViewMV_TemporalMVP(currMB, mb_x, mb_y, blockshape_x, blockshape_y, list, currMB->base_view_id, motion_interview, block);
        if(ref_frame==motion_interview[list].ref_idx) mv_b=motion_interview[list].mv;
        else mv_b=zero_mv;
      }

      if(block[2].available && rFrameUR == ref_frame) mv_c=mv_info[block[2].pos_y][block[2].pos_x].mv[list];
      else 
      {
        Get_InterViewMV_TemporalMVP(currMB, mb_x, mb_y, blockshape_x, blockshape_y, list, currMB->base_view_id, motion_interview, block);
        if(ref_frame==motion_interview[list].ref_idx) mv_c=motion_interview[list].mv;
        else mv_c=zero_mv;
      }

      pmv->mv_x = (short) imedian(mv_a.mv_x, mv_b.mv_x, mv_c.mv_x);
      pmv->mv_y = (short) imedian(mv_a.mv_y, mv_b.mv_y, mv_c.mv_y);

      break;
    }
#endif

    if(!(block[1].available || block[2].available))
    {
      if (block[0].available)
      {
        *pmv = mv_info[block[0].pos_y][block[0].pos_x].mv[list];
      }
      else
      {
        *pmv = zero_mv;
      }        
    }
    else
    {

      MotionVector *mv_a = block[0].available ? &mv_info[block[0].pos_y][block[0].pos_x].mv[list] : (MotionVector *) &zero_mv;
      MotionVector *mv_b = block[1].available ? &mv_info[block[1].pos_y][block[1].pos_x].mv[list] : (MotionVector *) &zero_mv;
      MotionVector *mv_c = block[2].available ? &mv_info[block[2].pos_y][block[2].pos_x].mv[list] : (MotionVector *) &zero_mv;


      pmv->mv_x = (short) imedian(mv_a->mv_x, mv_b->mv_x, mv_c->mv_x);
      pmv->mv_y = (short) imedian(mv_a->mv_y, mv_b->mv_y, mv_c->mv_y);

    }    
    break;
  case MVPRED_L:
    if (block[0].available)
    {
      *pmv = mv_info[block[0].pos_y][block[0].pos_x].mv[list];
    }
    else
    {
      *pmv = zero_mv;
    }
    break;
  case MVPRED_U:
    if (block[1].available)
    {
      *pmv = mv_info[block[1].pos_y][block[1].pos_x].mv[list];
    }
    else
    {
      *pmv = zero_mv;
    }
    break;
  case MVPRED_UR:
    if (block[2].available)
    {
      *pmv = mv_info[block[2].pos_y][block[2].pos_x].mv[list];
    }
    else
    {
      *pmv = zero_mv;
    }
    break;
  default:
    break;
  }
}

void init_motion_vector_prediction(Macroblock *currMB, int mb_aff_frame_flag)
{
#if EXT3D
  if(mb_aff_frame_flag)
    currMB->GetMVPredictor=GetMotionVectorPredictorMBAFF;
  else
  {
    currMB->GetMVPredictor=GetMotionVectorPredictorNormal;

    if((!currMB->p_Slice->is_depth)&&(currMB->p_Slice->depth_based_mvp_flag || currMB->p_Slice->Harmonize_VSP_IVP))
      currMB->GetInterviewMVPrediction=GetInterViewMotionVectorePredictionNormal;
    else
      currMB->GetInterviewMVPrediction=NULL;

    if((!currMB->p_Slice->is_depth)&&(currMB->p_Slice->depth_based_mvp_flag || currMB->p_Slice->Harmonize_VSP_IVP))
      currMB->GetInterviewMVPredictionMax=GetInterViewMotionVectorePredictionNormalMax;
    else
      currMB->GetInterviewMVPredictionMax=NULL;
  }
#else
  if (mb_aff_frame_flag)
    currMB->GetMVPredictor = GetMotionVectorPredictorMBAFF;
  else
    currMB->GetMVPredictor = GetMotionVectorPredictorNormal;
#endif
}
