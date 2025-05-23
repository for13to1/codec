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
 ***************************************************************************
 * \file mode_decision.c
 *
 * \brief
 *    Main macroblock mode decision functions and helpers
 *
 **************************************************************************
 */

#include <math.h>
#include <limits.h>
#include <float.h>

#include "global.h"
#include "image.h"
#include "ratectl.h"
#include "mode_decision.h"
#include "md_common.h"
#include "me_umhex.h"
#include "me_umhexsmp.h"
#include "macroblock.h"
#include "rdoq.h"
#include "q_around.h"
#include "slice.h"
#include "conformance.h"
#include "rdopt.h"


/*!
*************************************************************************************
* \brief
*    Reset Valid Modes
*************************************************************************************
*/
void reset_valid_modes(RD_PARAMS *enc_mb)
{
  memset(enc_mb->valid, 0, MAXMODE * sizeof(short));
}


/*!
*************************************************************************************
* \brief
*    Checks whether a primary SP slice macroblock was encoded as I16
*************************************************************************************
*/
static inline int check_for_SI16(int **lrec, int pix_x, int pix_y)
{
  int i,j;
  for (i = pix_y; i < pix_y + MB_BLOCK_SIZE; i++)
  {
    for (j = pix_x;j < pix_x + MB_BLOCK_SIZE; j++)
      if (lrec[i][j] != -16)
        return 0;
  }
  return 1;
}

/*!
*************************************************************************************
* \brief
*    Update parameters after encoding a macroblock
*************************************************************************************
*/
void end_encode_one_macroblock(Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;

  int bslice = (currSlice->slice_type == B_SLICE);

  update_qp_cbp(currMB);

  if ( (currSlice->mb_aff_frame_flag)
    && (currMB->mbAddrX & 0x01)
#if EXT3D
    && ((currMB->mb_type != 0 && currMB->mb_type != PVSP_SKIP) ? 0:((bslice) ? !currMB->cbp:1))  // bottom is skip
    && (((currMB->PrevMB)->mb_type != 0 && (currMB->PrevMB)->mb_type != PVSP_SKIP) ? 0:((bslice) ? !(currMB->PrevMB)->cbp:1))
#else
    && (currMB->mb_type ? 0:((bslice) ? !currMB->cbp:1))  // bottom is skip
    && ((currMB->PrevMB)->mb_type ? 0:((bslice) ? !(currMB->PrevMB)->cbp:1))
#endif
    && !(field_flag_inference(currMB) == currMB->mb_field)) // top is skip
  {
    currSlice->rddata->min_rdcost = 1e30;  // don't allow coding of a MB pair as skip if wrong inference
  }
  else
    currSlice->rddata->min_rdcost = (double)currMB->min_rdcost;

  currSlice->rddata->min_dcost  = (double)currMB->min_dcost;
  currSlice->rddata->min_rate   = (double)currMB->min_rate;

  if(p_Inp->SearchMode == UM_HEX)
  {
    UMHEX_skip_intrabk_SAD(currMB, currSlice->listXsize[currMB->list_offset]);
  }
  else if(p_Inp->SearchMode == UM_HEX_SIMPLE)
  {
    smpUMHEX_skip_intrabk_SAD(currMB);
  }

  //--- constrain intra prediction ---
  if(p_Inp->UseConstrainedIntraPred && (currSlice->slice_type == P_SLICE || currSlice->slice_type == B_SLICE))
  {
    p_Vid->intra_block[currMB->mbAddrX] = IS_INTRA(currMB);
  }
}

/*!
*************************************************************************************
* \brief
*    Initialize Encoding parameters for Macroblock
*************************************************************************************
*/
void init_enc_mb_params(Macroblock* currMB, RD_PARAMS *enc_mb, int intra)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;
  Slice *currSlice = currMB->p_Slice;
  int bslice = (currSlice->slice_type == B_SLICE);

  int l,k;

#if EXT3D 
  currMB->best_mode_changed = 0;
#endif


  enc_mb->curr_mb_field = (short) ((currSlice->mb_aff_frame_flag)&&(currMB->mb_field));

  // Set valid modes  
  enc_mb->valid[I8MB]  = (short) ((!p_Inp->DisableIntraInInter || intra )?   p_Inp->Transform8x8Mode : 0);
  enc_mb->valid[I4MB]  = (short) ((!p_Inp->DisableIntraInInter || intra )? ((p_Inp->Transform8x8Mode == 2) ? 0 : 1) : 0);
  enc_mb->valid[I4MB]  = (short) ((!p_Inp->DisableIntra4x4  ) ? enc_mb->valid[I4MB] : 0);
  enc_mb->valid[I16MB] = (short) ((!p_Inp->DisableIntraInInter || intra )? 1 : 0);
  enc_mb->valid[I16MB] = (short) ((!p_Inp->DisableIntra16x16) ? enc_mb->valid[I16MB] : 0);
  enc_mb->valid[IPCM]  = (short) ((!p_Inp->DisableIntraInInter || intra )? p_Inp->EnableIPCM : 0);
  enc_mb->valid[SI4MB] = 0;
  //enc_mb->valid[SI4MB] = (short) (currSlice->slice_type == SI_SLICE);
  //enc_mb->valid[SI4MB] = (short) ( p_Inp->DisableIntra4x4 ? 0 : enc_mb->valid[SI4MB]);

  enc_mb->valid[0]     = (short) (!intra && p_Inp->InterSearch[bslice][0]);
  if(enc_mb->valid[0] && !p_Inp->direct_spatial_mv_pred_flag && currSlice->slice_type == B_SLICE && 
    (currSlice->mb_aff_frame_flag || (p_Vid->structure==FRAME && !currSlice->listX[LIST_1][0]->coded_frame) || (p_Vid->structure!=FRAME && currSlice->listX[LIST_1][0]->coded_frame)))
     enc_mb->valid[0] = 0;
  enc_mb->valid[1]     = (short) (!intra && p_Inp->InterSearch[bslice][1]);
  enc_mb->valid[2]     = (short) (!intra && p_Inp->InterSearch[bslice][2]);
  enc_mb->valid[3]     = (short) (!intra && p_Inp->InterSearch[bslice][3]);
  enc_mb->valid[4]     = (short) (!intra && p_Inp->InterSearch[bslice][4]);
  enc_mb->valid[5]     = (short) (!intra && p_Inp->InterSearch[bslice][5] && !(p_Inp->Transform8x8Mode==2));
  enc_mb->valid[6]     = (short) (!intra && p_Inp->InterSearch[bslice][6] && !(p_Inp->Transform8x8Mode==2));
  enc_mb->valid[7]     = (short) (!intra && p_Inp->InterSearch[bslice][7] && !(p_Inp->Transform8x8Mode==2));
  enc_mb->valid[P8x8]  = (short) (enc_mb->valid[4] || enc_mb->valid[5] || enc_mb->valid[6] || enc_mb->valid[7]);

#if EXT3D
  enc_mb->valid[PVSP_SKIP] = (short) 0;
  if(enc_mb->valid[0] && currSlice->bVspRefExist == TRUE)
    enc_mb->valid[PVSP_SKIP] = (short) 1;
#endif

  if (currSlice->UseRDOQuant && p_Inp->RDOQ_CP_Mode && (p_Vid->qp != p_Vid->masterQP) )  
    RDOQ_update_mode(currSlice, enc_mb);
  
  if(currSlice->slice_type == SP_SLICE || currSlice->slice_type == SI_SLICE)
  {
    if(currSlice->slice_type == SI_SLICE)
    {
      reset_valid_modes(enc_mb);
      if(check_for_SI16(p_Vid->lrec, currMB->pix_x, currMB->pix_y))
      {
        enc_mb->valid[I16MB] = 1;
      }
      else
      {
        enc_mb->valid[I4MB]  = 1;
      }
    }

    if(p_Vid->sp2_frame_indicator)
    {
      if(check_for_SI16(p_Vid->lrec, currMB->pix_x, currMB->pix_y))
      {
        reset_valid_modes(enc_mb);
        enc_mb->valid[I16MB] = 1;
      }
      else
      {
        enc_mb->valid[I8MB]  = 0;
        enc_mb->valid[IPCM]  = 0;
        enc_mb->valid[0]     = 0;
        //enc_mb->valid[I16MB] = 0;
      }
    }
  }

  //===== SET LAGRANGE PARAMETERS =====
  // Note that these are now computed at the slice level to reduce
  // computations and cleanup code.
  enc_mb->lambda_md = p_Vid->lambda_md[currSlice->slice_type][p_Vid->masterQP];
  //#if JCOST_CALC_SCALEUP
  enc_mb->lambda_mdfp = LAMBDA_FACTOR(enc_mb->lambda_md);
  //#endif

  enc_mb->lambda_me[F_PEL] = p_Vid->lambda_me[currSlice->slice_type][p_Vid->masterQP][F_PEL];
  enc_mb->lambda_me[H_PEL] = p_Vid->lambda_me[currSlice->slice_type][p_Vid->masterQP][H_PEL];
  enc_mb->lambda_me[Q_PEL] = p_Vid->lambda_me[currSlice->slice_type][p_Vid->masterQP][Q_PEL];

  enc_mb->lambda_mf[F_PEL] = p_Vid->lambda_mf[currSlice->slice_type][p_Vid->masterQP][F_PEL];
  enc_mb->lambda_mf[H_PEL] = p_Vid->lambda_mf[currSlice->slice_type][p_Vid->masterQP][H_PEL];
  enc_mb->lambda_mf[Q_PEL] = p_Vid->lambda_mf[currSlice->slice_type][p_Vid->masterQP][Q_PEL];

  if (!currSlice->mb_aff_frame_flag)
  {
    for (l = LIST_0; l < BI_PRED; l++)
    {
      for(k = 0; k < currSlice->listXsize[l]; k++)
      {
        if(currSlice->structure != currSlice->listX[l][k]->structure)
        {
          if (currSlice->structure == TOP_FIELD)
            currSlice->listX[l][k]->chroma_vector_adjustment = -2;
          else if (currSlice->structure == BOTTOM_FIELD)
            currSlice->listX[l][k]->chroma_vector_adjustment = 2;
          else
            currSlice->listX[l][k]->chroma_vector_adjustment= 0;
        }
        else
          currSlice->listX[l][k]->chroma_vector_adjustment= 0;
      }
    }
  }
  else
  {
    if (enc_mb->curr_mb_field)
    {
      for (l = currMB->list_offset; l <= currMB->list_offset + LIST_1; l++)
      {
        for(k = 0; k < currSlice->listXsize[l]; k++)
        {
          currSlice->listX[l][k]->chroma_vector_adjustment= 0;
          if((currMB->mbAddrX & 0x01) == 0 && currSlice->listX[l][k]->structure == BOTTOM_FIELD)
            currSlice->listX[l][k]->chroma_vector_adjustment = -2;
          if((currMB->mbAddrX & 0x01) == 1 && currSlice->listX[l][k]->structure == TOP_FIELD)
            currSlice->listX[l][k]->chroma_vector_adjustment = 2;
        }
      }
    }
    else
    {
      for (l = currMB->list_offset; l <= currMB->list_offset + LIST_1; l++)
      {
        for(k = 0; k < currSlice->listXsize[l]; k++)
          currSlice->listX[l][k]->chroma_vector_adjustment = 0;
      }
    }
  }

  // reset chroma intra predictor to default
  currMB->c_ipred_mode = DC_PRED_8;

  if(p_Inp->SearchMode == UM_HEX)
  {
    UMHEX_decide_intrabk_SAD(currMB);
    UMHEX_DefineThresholdMB(p_Vid, p_Inp);
  }
  else if (p_Inp->SearchMode == UM_HEX_SIMPLE)
  {
    smpUMHEX_decide_intrabk_SAD(currMB);
  }

}

/*!
*************************************************************************************
* \brief
*    computation of prediction list (including biprediction) cost
*************************************************************************************
*/
void list_prediction_cost(Macroblock *currMB, int list, int block, int mode, RD_PARAMS *enc_mb, distblk bmcost[5], char best_ref[2])
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;

  short ref;
  distblk mcost;
  int cur_list = list < BI_PRED ? currMB->list_offset + list : currMB->list_offset;
  int ref_lambda = (p_Inp->rdopt) ? enc_mb->lambda_mf[Q_PEL] :  enc_mb->lambda_mf[Q_PEL] >> 2;

  //--- get cost and reference frame for forward prediction ---
  if (list < BI_PRED)
  {
    for (ref=0; ref < currSlice->listXsize[cur_list]; ref++)
    {
      if (!p_Vid->checkref || list || ref==0 || (p_Inp->RestrictRef && CheckReliabilityOfRef (currMB, block, list, ref, mode)))
      {
        // limit the number of reference frames to 1 when switching SP frames are used
        if((!p_Inp->sp2_frame_indicator && !p_Inp->sp_output_indicator)||
          ((p_Inp->sp2_frame_indicator || p_Inp->sp_output_indicator) && (currSlice->slice_type != P_SLICE && currSlice->slice_type != SP_SLICE))||
          ((p_Inp->sp2_frame_indicator || p_Inp->sp_output_indicator) && ((currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE) &&(ref==0))))
        {

#if EXT3D 
            if (currMB->is_alc_mode)
            {
                mcost = 0;
            }
            else
            {
                mcost  = ref_cost(currSlice, ref_lambda, (short) ref, cur_list);
            }
#else
          mcost  = ref_cost(currSlice, ref_lambda, (short) ref, cur_list);
#endif

          mcost += p_Vid->motion_cost[mode][list][ref][block];

          if (mcost < bmcost[list])
          {
            bmcost[list]   = mcost;
            best_ref[list] = (char)ref;
          }
        }
      }
    }
  }
  else if (list == BI_PRED)
  {
#if EXT3D
  if((p_Vid->active_pps->weighted_bipred_idc == 1)||(currSlice->depth_range_wp))
#else
    if (p_Vid->active_pps->weighted_bipred_idc == 1)
#endif
    {
      int weight_sum = currSlice->wbp_weight[0][(int) best_ref[LIST_0]][(int) best_ref[LIST_1]][0] + currSlice->wbp_weight[1][(int) best_ref[LIST_0]][(int) best_ref[LIST_1]][0];

      if (weight_sum < -128 ||  weight_sum > 127)
      {
        bmcost[list] = DISTBLK_MAX;
      }
      else
      {
        bmcost[list]  = ref_cost(currSlice, ref_lambda, (short) best_ref[LIST_0], cur_list) +
          ref_cost(currSlice, ref_lambda, (short) best_ref[LIST_1], cur_list + LIST_1);
        bmcost[list] += BIDPartitionCost (currMB, mode, block, best_ref, enc_mb->lambda_mf[Q_PEL]);
      }
    }
    else
    {
      bmcost[list]  = ref_cost(currSlice, ref_lambda, (short) best_ref[LIST_0], cur_list) +
        ref_cost(currSlice, ref_lambda, (short) best_ref[LIST_1], cur_list + LIST_1);
      bmcost[list] += BIDPartitionCost (currMB, mode, block, best_ref, enc_mb->lambda_mf[Q_PEL]);
    }
  }
  else
  {
#if EXT3D
  if ((p_Vid->active_pps->weighted_bipred_idc == 1)||(currSlice->depth_range_wp))
#else
    if (p_Vid->active_pps->weighted_bipred_idc == 1)
#endif
    {
      int weight_sum = currSlice->wbp_weight[0][0][0][0] + currSlice->wbp_weight[1][0][0][0];

      if (weight_sum < -128 ||  weight_sum > 127)
      {
        bmcost[list] = DISTBLK_MAX;
      }
      else
      {
        bmcost[list]  = ref_cost(currSlice, ref_lambda, 0, cur_list) + ref_cost(currSlice, ref_lambda, 0, cur_list + LIST_1);
        bmcost[list] += BPredPartitionCost(currMB, mode, block, 0, 0, enc_mb->lambda_mf[Q_PEL], !(list&1));
      }
    }
    else
    {
      bmcost[list]  = ref_cost(currSlice, ref_lambda, 0, cur_list) + ref_cost(currSlice, ref_lambda, 0, cur_list + LIST_1);
      bmcost[list] += BPredPartitionCost(currMB, mode, block, 0, 0, enc_mb->lambda_mf[Q_PEL], !(list&1));
    }
  }
}

static inline distblk compute_ref_cost(Slice *currSlice, RD_PARAMS *enc_mb, int ref, int list)
{
  return weighted_cost(enc_mb->lambda_mf[Q_PEL],((currSlice->listXsize[list] <= 1)? 0 : currSlice->p_Vid->refbits[ref]));
}

/*!
*************************************************************************************
* \brief
*    Determination of prediction list based on simple distortion computation
*************************************************************************************
*/
void determine_prediction_list( distblk bmcost[5], Info8x8 *best, distblk *cost)
{
  int bestlist;  
  *cost += distblkminarray ( bmcost, 5, &bestlist);
  
  if (bestlist <= BI_PRED)  //LIST_0, LIST_1 & BI_DIR
  {
    best->pdir = (char) bestlist; 
    best->bipred= 0;
  }
  else                      //BI_PRED_L0 & BI_PRED_L1
  {
    best->pdir = 2;    
    best->bipred = (char) (bestlist - 2);
    best->ref[LIST_0] = 0;
    best->ref[LIST_1] = 0;
  }
}


/*!
*************************************************************************************
* \brief
*    RD decision process
*************************************************************************************
*/
void compute_mode_RD_cost(Macroblock *currMB,
                          RD_PARAMS *enc_mb,
                          short mode,
                          short *inter_skip)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;
  int terminate_16x16 = 0, terminate_trans = 0, ctr16x16 = 0;
  Slice *currSlice = currMB->p_Slice;
  RDOPTStructure  *p_RDO = currSlice->p_RDO;   
  int bslice = (currSlice->slice_type == B_SLICE);

  //--- transform size ---
  currMB->luma_transform_size_8x8_flag = (byte) (p_Inp->Transform8x8Mode==2
    ?  (mode >= 1 && mode <= 3)
    || (mode == 0 && bslice && p_Vid->active_sps->direct_8x8_inference_flag)
    || ((mode == P8x8) && (enc_mb->valid[4]))
    :  0);

  currSlice->set_modes_and_refs_for_blocks (currMB, (short) mode);
  //memset( currSlice->cofAC[0][0][0], 0, 2080 * sizeof(int)); // 4 * 4 * 2 * 65

  // Encode with coefficients
  currSlice->NoResidueDirect = 0;

  if ((p_Inp->FastCrIntraDecision ) || (currMB->c_ipred_mode == DC_PRED_8 || (IS_INTRA(currMB) )))
  {
    do
    {
      // This seems to have a problem since we are not properly copying the right b8x8info (transform based)
      terminate_16x16 = bslice_16x16_termination_control(p_Inp, p_Vid->b8x8info, &ctr16x16, mode, bslice);
      do
      {
        RD_8x8DATA *t8x8_data = currMB->luma_transform_size_8x8_flag ? p_RDO->tr8x8 : p_RDO->tr4x4;

        if (mode == P8x8 ) //&& !(p_Inp->RDOQ_QP_Num > 1 && currSlice->dQPCnt > 0))
        {
          p_Vid->b8x8info->best[P8x8][0] = t8x8_data->part[0];
          p_Vid->b8x8info->best[P8x8][1] = t8x8_data->part[1];
          p_Vid->b8x8info->best[P8x8][2] = t8x8_data->part[2];
          p_Vid->b8x8info->best[P8x8][3] = t8x8_data->part[3];
        }

        // check if prediction parameters are in valid range.
        if (CheckPredictionParams(currMB, p_Vid->b8x8info, mode) == TRUE)
        {
          if (RDCost_for_macroblocks (currMB, enc_mb->lambda_mdfp, mode))
          {
            //Rate control
            if (p_Inp->RCEnable)
            {
              if(mode == P8x8)
              {
                rc_store_diff(currSlice->diffy, &p_Vid->pCurImg[currMB->opix_y], currMB->pix_x, t8x8_data->mpr8x8);
              }
              else
                rc_store_diff(currSlice->diffy, &p_Vid->pCurImg[currMB->opix_y], currMB->pix_x, p_RDO->pred);
            }

            store_macroblock_parameters (currMB, mode);

#if EXT3D
            if(p_Inp->rdopt == 2 && (mode == 0 || mode == PVSP_SKIP) && p_Inp->EarlySkipEnable)
#else
            if(p_Inp->rdopt == 2 && mode == 0 && p_Inp->EarlySkipEnable)
#endif
            {
              // check transform quantized coeff.
              if(currMB->cbp == 0)
                *inter_skip = 1;
            }
          }        
        }
        // This code needs to be fixed - BUG? ATOUR

        terminate_trans = transform_termination_control(currMB, mode);

      }while (!terminate_trans);
    }while (!terminate_16x16);

    // Encode with no coefficients. Currently only for direct. This could be extended to all other modes as in example.
    //if (bslice && mode < P8x8 && (*inter_skip == 0) && enc_mb->valid[mode] && currMB->cbp && (currMB->cbp&15) != 15 && !p_Inp->nobskip
#if EXT3D
    if (bslice && (mode == 0 || mode == BVSP_DIRECT) && (*inter_skip == 0) && enc_mb->valid[mode] && currMB->cbp && (currMB->cbp&15) != 15 && !p_Inp->nobskip
    && !(currMB->qp_scaled[0] == 0 && p_Vid->lossless_qpprime_flag==1) )
#else
    if (bslice && mode == 0 && (*inter_skip == 0) && enc_mb->valid[mode] && currMB->cbp && (currMB->cbp&15) != 15 && !p_Inp->nobskip
    && !(currMB->qp_scaled[0] == 0 && p_Vid->lossless_qpprime_flag==1) )
#endif
    {
      currSlice->NoResidueDirect = 1;

      if (CheckPredictionParams(currMB, p_Vid->b8x8info, mode) == TRUE)
      {
        if (RDCost_for_macroblocks (currMB, enc_mb->lambda_mdfp, mode))
        {
          //Rate control
          if (p_Inp->RCEnable)
            rc_store_diff(currSlice->diffy, &p_Vid->pCurImg[currMB->opix_y], currMB->pix_x, p_RDO->pred);

          if (p_Vid->AdaptiveRounding)
            reset_adaptive_rounding_direct(p_Vid);

          store_macroblock_parameters (currMB, mode);
        }
      }
    }

    //modes 0 and 1 of a B frame 
#if EXT3D
    if (p_Vid->AdaptiveRounding && bslice && (mode <= 1 || mode == BVSP_DIRECT))
#else
    if (p_Vid->AdaptiveRounding && bslice && mode <= 1)
#endif
    { 
      if (currMB->temp_transform_size_8x8_flag)
        update_adaptive_rounding_16x16( p_Vid, p_Vid->ARCofAdj8x8, mode);
      else
        update_adaptive_rounding_16x16( p_Vid, p_Vid->ARCofAdj4x4, mode);
    }
  }
}

void get_initial_mb16x16_cost(Macroblock* currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  RDOPTStructure *p_RDO = currSlice->p_RDO;
  if (currMB->mb_left && currMB->mb_up)
  {
    p_Vid->mb16x16_cost = (p_Vid->mb16x16_cost_frame[currMB->mbAddrX - 1] +
      p_Vid->mb16x16_cost_frame[currMB->mbAddrX - (p_Vid->width>>4)] + 1)/2.0;
  }
  else if (currMB->mb_left)
  {
    p_Vid->mb16x16_cost = p_Vid->mb16x16_cost_frame[currMB->mbAddrX - 1];
  }
  else if (currMB->mb_up)
  {
    p_Vid->mb16x16_cost = p_Vid->mb16x16_cost_frame[currMB->mbAddrX - (p_Vid->width>>4)];
  }
  else
  {
    p_Vid->mb16x16_cost = CALM_MF_FACTOR_THRESHOLD;
  }

  p_RDO->lambda_mf_factor = p_Vid->mb16x16_cost < CALM_MF_FACTOR_THRESHOLD ? 1.0 : sqrt(p_Vid->mb16x16_cost / (CALM_MF_FACTOR_THRESHOLD * p_Vid->lambda_mf_factor[currSlice->slice_type][p_Vid->qp]));
}

void adjust_mb16x16_cost(Macroblock *currMB, distblk cost)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  RDOPTStructure  *p_RDO = currSlice->p_RDO;

  p_Vid->mb16x16_cost = (double) cost;
  p_Vid->mb16x16_cost_frame[currMB->mbAddrX] = p_Vid->mb16x16_cost;
#if JCOST_CALC_SCALEUP
  p_RDO->lambda_mf_factor = (p_Vid->mb16x16_cost < CALM_MF_FACTOR_THRESHOLD*(1<<LAMBDA_ACCURACY_BITS))
  ? 1.0
  : sqrt(p_Vid->mb16x16_cost / (CALM_MF_FACTOR_THRESHOLD*(1<<LAMBDA_ACCURACY_BITS) * p_Vid->lambda_mf_factor[currSlice->slice_type][p_Vid->qp]));
#else
  p_RDO->lambda_mf_factor = (p_Vid->mb16x16_cost < CALM_MF_FACTOR_THRESHOLD)
  ? 1.0
  : sqrt(p_Vid->mb16x16_cost / (CALM_MF_FACTOR_THRESHOLD * p_Vid->lambda_mf_factor[currSlice->slice_type][p_Vid->qp]));
#endif
}

void update_lambda_costs(Macroblock *currMB, RD_PARAMS *enc_mb, int lambda_mf[3])
{
  InputParameters *p_Inp = currMB->p_Inp;
  RDOPTStructure  *p_RDO = currMB->p_Slice->p_RDO;

  int MEPos;
  for (MEPos = 0; MEPos < 3; MEPos ++)
  {
    lambda_mf[MEPos] = p_Inp->CtxAdptLagrangeMult == 0 ? enc_mb->lambda_mf[MEPos] : (int)(enc_mb->lambda_mf[MEPos] * sqrt(p_RDO->lambda_mf_factor));
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Return array's minimum and its index
 *************************************************************************************
 */
int iminarray ( int arr[], int size, int *minind )
{
  int i; 
  int mincand = arr[0];
  *minind = 0;
  for ( i = 1; i < size; i++ )
  {
    if (arr[i] < mincand)
    {
      mincand = arr[i];
      *minind = i;
    }
  }
  return mincand;
} 

distblk distblkminarray ( distblk arr[], int size, int *minind )
{
  int i; 
  distblk mincand = arr[0];
  *minind = 0;
  for ( i = 1; i < size; i++ )
  {
    if (arr[i] < mincand)
    {
      mincand = arr[i];
      *minind = i;
    }
  }
  return mincand;
} 

/*!
 *************************************************************************************
 * \brief
 *    Determines whether bi prediction is enabaled for current mode
 *************************************************************************************
 */
int is_bipred_enabled(VideoParameters *p_Vid, int mode) 
{
  return p_Vid->bipred_enabled[(mode == P8x8) ? 4: mode];
}

/*!
 *************************************************************************************
 * \brief
 *    Decides whether to perform tranform 8x8 for this mode
 *************************************************************************************
 */
int transform_termination_control(Macroblock* currMB, int mode) 
{  
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;

  // Go through transform modes.
  // Note that if currMB->cbp is 0 one could choose to skip 8x8 mode
  // although this could be due to deadzoning decisions.
  //if (p_Inp->Transform8x8Mode==1 && currMB->cbp!=0)
  if (p_Inp->Transform8x8Mode == 1)
  {
    int bslice = currSlice->slice_type == B_SLICE;
    //=========== try the 8x8 transform with mb_types 16x16,16x8, 8x16, 8x8, and DIRECT 16x16 ===========
    if (currMB->luma_transform_size_8x8_flag == FALSE && 
      ((mode >= 1 && mode <= 3) || (bslice && mode == 0 && p_Vid->active_sps->direct_8x8_inference_flag) || (mode == P8x8)))
        //if (currMB->luma_transform_size_8x8_flag == FALSE && 
      //((mode >= 1 && mode <= 3) || (bslice && mode == 0 && active_sps->direct_8x8_inference_flag)))
    {
      //try with 8x8 transform size
      currMB->luma_transform_size_8x8_flag = TRUE;
      return 0;
    }
    else
    {
      currMB->luma_transform_size_8x8_flag = FALSE;
      return 1;
    }
  }
  else
  {
    return 1;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Update prediction direction for mode P16x16 to check all prediction directions
 *************************************************************************************
 */
int bslice_16x16_termination_control(InputParameters *p_Inp, Block8x8Info *b8x8info, int *ctr16x16, int mode, int bslice)
{
  int lastcheck = 1;
  //--- for INTER16x16 in BSLICEs check all prediction directions ---
  if (mode == 1 && bslice)
  {
    char pdir = 0;
    short i;
    char bipred_me = 0;
 
    switch (*ctr16x16)
    {
    case 0:
      pdir = 0;
      lastcheck = 0;
      break;
    case 1:
      pdir = 1;
      lastcheck = 0;
      break;
    case 2:
      pdir = 2;
      if (p_Inp->BiPredMotionEstimation)
      {
        lastcheck = 0;
      }
      break;
    case 3:
      pdir = 2;
      bipred_me = 1;
      lastcheck = 0;
      break;
    case 4:
      pdir = 2;
      bipred_me = 2;
      break;
    default:
      error("invalid 'ctr16x16' value", -1);
      break;
    }
    for (i = 0; i< 4; i++)
    {
      b8x8info->best[1][i].bipred = bipred_me;
      b8x8info->best[1][i].pdir = pdir;
    }
    (*ctr16x16)++;
  }
  return lastcheck;
}

/*!
**************************************************************************************
* \brief
*     Compute bipred costs
**************************************************************************************
*/
void get_bipred_cost(Macroblock *currMB, int mode, int block, int i, int j, Info8x8  *best, RD_PARAMS *enc_mb, distblk bmcost[5])
{
  Slice *currSlice = currMB->p_Slice;
  MotionVector   *bi0_mv_l0    = &currSlice->bipred_mv[0][LIST_0][0][mode][j][i];
  MotionVector   *bi0_mv_l1    = &currSlice->bipred_mv[0][LIST_1][0][mode][j][i];
  MotionVector   *bi1_mv_l0    = &currSlice->bipred_mv[1][LIST_0][0][mode][j][i];
  MotionVector   *bi1_mv_l1    = &currSlice->bipred_mv[1][LIST_1][0][mode][j][i];
  distblk  MaxVal = DISTBLK_MAX;
  if (best->ref[0] == 0 && best->ref[1] == 0)
  {
    MotionVector *single_mv_l0 = &currSlice->all_mv [LIST_0][0][mode][j][i];
    MotionVector *single_mv_l1 = &currSlice->all_mv [LIST_1][0][mode][j][i];

    if ((single_mv_l0->mv_x != bi0_mv_l0->mv_x) || (single_mv_l0->mv_y != bi0_mv_l0->mv_y) ||
      (single_mv_l1->mv_x != bi0_mv_l1->mv_x) || (single_mv_l1->mv_y != bi0_mv_l1->mv_y))
      list_prediction_cost(currMB, BI_PRED_L0, block, mode, enc_mb, bmcost, 0);
    else
      bmcost[BI_PRED_L0] = MaxVal;

    if ((single_mv_l0->mv_x != bi1_mv_l0->mv_x) || (single_mv_l0->mv_y != bi1_mv_l0->mv_y) ||
      (single_mv_l1->mv_x != bi1_mv_l1->mv_x) || (single_mv_l1->mv_y != bi1_mv_l1->mv_y))
    {
      if ((bi0_mv_l0->mv_x != bi1_mv_l0->mv_x) || (bi0_mv_l0->mv_y != bi1_mv_l0->mv_y) ||
        (bi0_mv_l1->mv_x != bi1_mv_l1->mv_x) || (bi0_mv_l1->mv_y != bi1_mv_l1->mv_y))
        list_prediction_cost(currMB, BI_PRED_L1, block, mode, enc_mb, bmcost, 0);
      else
        bmcost[BI_PRED_L1] = MaxVal;
    }
    else
      bmcost[BI_PRED_L1] = MaxVal;
  }
  else
  {
    list_prediction_cost(currMB, BI_PRED_L0, block, mode, enc_mb, bmcost, 0);
    if ((bi0_mv_l0->mv_x != bi1_mv_l0->mv_x) || (bi0_mv_l0->mv_y != bi1_mv_l0->mv_y) ||
      (bi0_mv_l1->mv_x != bi1_mv_l1->mv_x) || (bi0_mv_l1->mv_y != bi1_mv_l1->mv_y))
      list_prediction_cost(currMB, BI_PRED_L1, block, mode, enc_mb, bmcost, 0);
    else
      bmcost[BI_PRED_L1] = MaxVal;
  }
}

#if EXT3D 
/*!
**************************************************************************************
* \brief
*     chooses coding modes for checking when alc is enabled
**************************************************************************************
*/
void init_alc_enc_mb_params(Macroblock* currMB, RD_PARAMS *enc_mb)
{
    // Set valid modes
    // Disable all INTRA modes
    enc_mb->valid[I4MB  ] = 0;
    enc_mb->valid[I16MB ] = 0;
    enc_mb->valid[IBLOCK] = 0;
    enc_mb->valid[SI4MB ] = 0;
    enc_mb->valid[I8MB  ] = 0;
    enc_mb->valid[IPCM  ] = 0;

  if(currMB->p_Slice->check_alc_skip)
  {
    enc_mb->valid[1]=enc_mb->valid[2]=enc_mb->valid[3]=enc_mb->valid[4]=
    enc_mb->valid[5]=enc_mb->valid[6]=enc_mb->valid[7]=enc_mb->valid[P8x8]=0;
  }
  else
  {
    enc_mb->valid[0]      = 0;
    enc_mb->valid[4]      = 0;
    enc_mb->valid[5]      = 0;
    enc_mb->valid[6]      = 0;
    enc_mb->valid[7]      = 0;
    enc_mb->valid[P8x8]   = 0;
  }

  enc_mb->valid[PVSP_SKIP] = 0;

}
#endif
