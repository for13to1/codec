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
 * \file mv_search.c
 *
 * \brief
 *    Motion Vector Search, unified for B and P Pictures
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Stephan Wenger                  <stewe@cs.tu-berlin.de>
 *      - Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
 *      - Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
 *      - Stephan Wenger                  <stewe@cs.tu-berlin.de>
 *      - Jani Lainema                    <jani.lainema@nokia.com>
 *      - Detlev Marpe                    <marpe@hhi.de>
 *      - Thomas Wedi                     <wedi@tnt.uni-hannover.de>
 *      - Heiko Schwarz                   <hschwarz@hhi.de>
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

// Motion estimation distortion header file
#include "me_distortion.h"

// Motion estimation search algorithms
#include "me_epzs.h"
#include "me_epzs_int.h"
#include "me_fullfast.h"
#include "me_fullsearch.h"
#include "me_umhex.h"
#include "me_umhexsmp.h"
#include "rdoq.h"
#if EXT3D
#include "configfile.h"
#endif


static const short bx0[5][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,2,0,0}, {0,2,0,2}};
static const short by0[5][4] = {{0,0,0,0}, {0,0,0,0}, {0,2,0,0}, {0,0,0,0}, {0,0,2,2}};


static distblk GetSkipCostMB          (Macroblock *currMB, int lambda);
static distblk BiPredBlockMotionSearch(Macroblock *currMB, MEBlock *, MotionVector*, int, int , int*);

/*!
 ************************************************************************
 * \brief
 *    Set search range. This needs to be changed to provide 2D support
 ************************************************************************
 */
void get_search_range(MEBlock *mv_block, InputParameters *p_Inp, short ref, int blocktype)
{
  SearchWindow *searchRange = &mv_block->searchRange;

  *searchRange = mv_block->p_Vid->searchRange;
  //----- set search range ---
  if (p_Inp->full_search == 1)
  {
    int scale = (imin(ref,1)+1);
    searchRange->min_x /= scale;
    searchRange->max_x /= scale;
    searchRange->min_y /= scale;
    searchRange->max_y /= scale;
  }
  else if  (p_Inp->full_search != 2)
  {
    int scale = ((imin(ref,1)+1) * imin(2,blocktype));
    searchRange->min_x /= scale;
    searchRange->max_x /= scale;
    searchRange->min_y /= scale;
    searchRange->max_y /= scale;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Set search range. This needs to be changed to provide 2D support
 ************************************************************************
 */
#if EXT3D
static inline void set_me_parameters( PicMotionParams **motion, const MotionVector *all_mv, int list, char ref, int step_h, int step_v, int pic_block_y, int pic_block_x , short isVSPRef)
#else
static inline void set_me_parameters( PicMotionParams **motion, const MotionVector *all_mv, int list, char ref, int step_h, int step_v, int pic_block_y, int pic_block_x)
#endif
{
  int i, j;

  // Set first line
  for (j = pic_block_y; j < pic_block_y + step_v; j++)
  {
    for (i=pic_block_x; i<pic_block_x + step_h; i++)
    {
      motion[j][i].mv[list] = *all_mv;
      motion[j][i].ref_idx[list] = ref;
#if EXT3D
      motion[j][i].isVSPRef[list]=isVSPRef;
#endif
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Set ME access method
 ************************************************************************
 */
void set_access_method(int *access_method, MotionVector *blk, int min_x, int min_y, int max_x, int max_y)
{
  if ( (blk->mv_x > min_x) && (blk->mv_x < max_x) && (blk->mv_y > min_y) && (blk->mv_y < max_y))
  {
    *access_method = FAST_ACCESS;
  }
  else
  {
    *access_method = UMV_ACCESS;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Initialize ME engine
 ************************************************************************
 */
void init_ME_engine(Macroblock *currMB)
{
  InputParameters *p_Inp = currMB->p_Inp;
  switch (p_Inp->SearchMode)
  {
   case EPZS:
     currMB->IntPelME = (p_Inp->EPZSSubPelGrid) ? EPZSIntPelBlockMotionSearch : EPZSPelBlockMotionSearch;
     currMB->BiPredME = (p_Inp->EPZSSubPelGrid) ? EPZSIntBiPredBlockMotionSearch : EPZSBiPredBlockMotionSearch;
     currMB->SubPelME       = (p_Inp->EPZSSubPelME) ? EPZSSubPelBlockMotionSearch : SubPelBlockMotionSearch;
     currMB->SubPelBiPredME = (p_Inp->EPZSSubPelMEBiPred) ? EPZSSubPelBlockSearchBiPred : SubPelBlockSearchBiPred;
     break;
   case UM_HEX:
     currMB->IntPelME       = UMHEXIntegerPelBlockMotionSearch;
     currMB->BiPredME       = UMHEXBipredIntegerPelBlockMotionSearch;
     currMB->SubPelBiPredME = SubPelBlockSearchBiPred;
     currMB->SubPelME       = UMHEXSubPelBlockME;
     break;
   case UM_HEX_SIMPLE:
     currMB->IntPelME       = smpUMHEXIntegerPelBlockMotionSearch;
     currMB->BiPredME       = smpUMHEXBipredIntegerPelBlockMotionSearch;
     currMB->SubPelBiPredME = SubPelBlockSearchBiPred;
     currMB->SubPelME       = smpUMHEXSubPelBlockME;
     break;
   case FULL_SEARCH:
     currMB->IntPelME       = FullPelBlockMotionSearch;
     currMB->BiPredME       = FullPelBlockMotionBiPred;
     currMB->SubPelBiPredME = SubPelBlockSearchBiPred;
     currMB->SubPelME       = SubPelBlockMotionSearch;
     break;
   case FAST_FULL_SEARCH:
   default:
     currMB->IntPelME       = FastFullPelBlockMotionSearch;
     currMB->BiPredME       = FullPelBlockMotionBiPred;
     currMB->SubPelBiPredME = SubPelBlockSearchBiPred;
     currMB->SubPelME       = SubPelBlockMotionSearch;
     break;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Prepare Motion Estimation parameters for single list ME
 ************************************************************************
 */
void PrepareMEParams(Slice *currSlice, MEBlock *mv_block, int ChromaMEEnable, int list, int ref)
{
  if (mv_block->apply_weights)
  {
    mv_block->weight_luma = currSlice->wp_weight[list][ref][0];
    mv_block->offset_luma = currSlice->wp_offset[list][ref][0];

    if ( ChromaMEEnable)
    {
      mv_block->weight_cr[0] = currSlice->wp_weight[list][ref][1];
      mv_block->weight_cr[1] = currSlice->wp_weight[list][ref][2];
      mv_block->offset_cr[0] = currSlice->wp_offset[list][ref][1];
      mv_block->offset_cr[1] = currSlice->wp_offset[list][ref][2];
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Prepare Motion Estimation parameters for bipred list ME
 ************************************************************************
 */
void PrepareBiPredMEParams(Slice *currSlice, MEBlock *mv_block, int ChromaMEEnable, int list, int list_offset, int ref)
{
  if (mv_block->apply_weights)
  {
    if (list == LIST_0)
    {
      mv_block->weight1  = currSlice->wbp_weight[list_offset         ][ref][0][0];
      mv_block->weight2  = currSlice->wbp_weight[list_offset + LIST_1][ref][0][0];
      mv_block->offsetBi = (currSlice->wp_offset[list_offset         ][ref][0] + currSlice->wp_offset[list_offset + LIST_1][ref][0] + 1)>>1;

      if ( ChromaMEEnable)
      {
        mv_block->weight1_cr[0] = currSlice->wbp_weight[list_offset         ][ref][0][1];
        mv_block->weight1_cr[1] = currSlice->wbp_weight[list_offset         ][ref][0][2];
        mv_block->weight2_cr[0] = currSlice->wbp_weight[list_offset + LIST_1][ref][0][1];
        mv_block->weight2_cr[1] = currSlice->wbp_weight[list_offset + LIST_1][ref][0][2];

        mv_block->offsetBi_cr[0] = (currSlice->wp_offset[list_offset        ][ref][1] + currSlice->wp_offset[list_offset + LIST_1][ref][1] + 1) >> 1;
        mv_block->offsetBi_cr[1] = (currSlice->wp_offset[list_offset        ][ref][2] + currSlice->wp_offset[list_offset + LIST_1][ref][2] + 1) >> 1;
      }
    }
    else
    {
      mv_block->weight1  = currSlice->wbp_weight[list_offset + LIST_1][0  ][ref][0];
      mv_block->weight2  = currSlice->wbp_weight[list_offset         ][0  ][ref][0];
      mv_block->offsetBi = (currSlice->wp_offset[list_offset + LIST_1][0][0] + currSlice->wp_offset[list_offset][0][0] + 1)>>1;

      if ( ChromaMEEnable)
      {
        mv_block->weight1_cr[0] = currSlice->wbp_weight[list_offset + LIST_1][0  ][ref][1];
        mv_block->weight1_cr[1] = currSlice->wbp_weight[list_offset + LIST_1][0  ][ref][2];
        mv_block->weight2_cr[0] = currSlice->wbp_weight[list_offset         ][0  ][ref][1];
        mv_block->weight2_cr[1] = currSlice->wbp_weight[list_offset         ][0  ][ref][2];

        mv_block->offsetBi_cr[0] = (currSlice->wp_offset[list_offset + LIST_1][0  ][1] + currSlice->wp_offset[list_offset         ][0  ][1] + 1) >> 1;
        mv_block->offsetBi_cr[1] = (currSlice->wp_offset[list_offset + LIST_1][0  ][2] + currSlice->wp_offset[list_offset         ][0  ][2] + 1) >> 1;
      }
    }
  }
  else
  {
    mv_block->weight1 = (short) (1 << currSlice->luma_log_weight_denom);
    mv_block->weight2 = (short) (1 << currSlice->luma_log_weight_denom);
    mv_block->offsetBi = 0;
    if ( ChromaMEEnable)
    {
      mv_block->weight1_cr[0] = 1<<currSlice->chroma_log_weight_denom;
      mv_block->weight1_cr[1] = 1<<currSlice->chroma_log_weight_denom;
      mv_block->weight2_cr[0] = 1<<currSlice->chroma_log_weight_denom;
      mv_block->weight2_cr[1] = 1<<currSlice->chroma_log_weight_denom;
      mv_block->offsetBi_cr[0] = 0;
      mv_block->offsetBi_cr[1] = 0;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Get current block spatial neighbors
 ************************************************************************
 */

void get_neighbors(Macroblock *currMB,       // <--  current Macroblock
                   PixelPos   *block,        // <--> neighbor blocks
                   int         mb_x,         // <--  block x position
                   int         mb_y,         // <--  block y position
                   int         blockshape_x  // <--  block width
                   )
{
  VideoParameters *p_Vid = currMB->p_Vid;
  int *mb_size = p_Vid->mb_size[IS_LUMA];

  get4x4Neighbour(currMB, mb_x - 1,            mb_y    , mb_size, &block[0]);
  get4x4Neighbour(currMB, mb_x,                mb_y - 1, mb_size, &block[1]);
  get4x4Neighbour(currMB, mb_x + blockshape_x, mb_y - 1, mb_size, &block[2]);
  get4x4Neighbour(currMB, mb_x - 1,            mb_y - 1, mb_size, &block[3]);

  if (mb_y > 0)
  {
    if (mb_x < 8)  // first column of 8x8 blocks
    {
      if (mb_y == 8 )
      {
        if (blockshape_x == MB_BLOCK_SIZE)      
          block[2].available  = 0;
      }
      else if (mb_x + blockshape_x == 8)
      {
        block[2].available = 0;
      }
    }
    else if (mb_x + blockshape_x == MB_BLOCK_SIZE)
    {
      block[2].available = 0;
    }
  }

  if (!block[2].available)
  {
    block[2] = block[3];
  }
}



/*!
************************************************************************
* \brief
*    Initialize the motion search
************************************************************************
*/
void Init_Motion_Search_Module (VideoParameters *p_Vid, InputParameters *p_Inp)
{
  int bits;
  int i_min, i_max,k;
  int i, l;
  int search_range               = p_Inp->search_range;
  int max_search_points          = imax(9, (2 * search_range + 1) * (2 * search_range + 1));
  int max_ref_bits               = 1 + 2 * (int)floor(log(imax(16, p_Vid->max_num_references + 1)) / log(2) + 1e-10);
  int max_ref                    = (1<<((max_ref_bits>>1)+1))-1;
  int number_of_subpel_positions = 4 * (2*search_range+3);
  int max_mv_bits                = 3 + 2 * (int)ceil (log(number_of_subpel_positions + 1) / log(2) + 1e-10);
  int max_mvd                    = p_Inp->UseMVLimits? imax(4*imax(p_Inp->SetMVXLimit, p_Inp->SetMVYLimit), ((1<<( max_mv_bits >>1) ) - 1)): ((1<<( max_mv_bits >>1)) - 1);

  p_Vid->max_mvd = max_mvd;
  p_Vid->imgpel_abs_range          = (imax(p_Vid->max_pel_value_comp[0],p_Vid->max_pel_value_comp[1]) + 1) * 64;

  //=====   CREATE ARRAYS   =====
  //-----------------------------
  if ((p_Vid->spiral_search = (MotionVector*)calloc(max_search_points, sizeof(MotionVector))) == NULL)
    no_mem_exit("Init_Motion_Search_Module: p_Vid->spiral_search");
  if ((p_Vid->spiral_hpel_search = (MotionVector*)calloc(max_search_points, sizeof(MotionVector))) == NULL)
    no_mem_exit("Init_Motion_Search_Module: p_Vid->spiral_hpel_search");
  if ((p_Vid->spiral_qpel_search = (MotionVector*)calloc(max_search_points, sizeof(MotionVector))) == NULL)
    no_mem_exit("Init_Motion_Search_Module: p_Vid->spiral_qpel_search");


  if ((p_Vid->mvbits = (int*)calloc(2 * max_mvd + 1, sizeof(int))) == NULL)
    no_mem_exit("Init_Motion_Search_Module: p_Vid->mvbits");

  if ((p_Vid->refbits = (int*)calloc(max_ref, sizeof(int))) == NULL)
    no_mem_exit("Init_Motion_Search_Module: p_Vid->refbits");

#if (JM_MEM_DISTORTION)
  if ((p_Vid->imgpel_abs = (int*)calloc(p_Vid->imgpel_abs_range, sizeof(int))) == NULL)
    no_mem_exit("Init_Motion_Search_Module: p_Vid->imgpel_abs");
  if ((p_Vid->imgpel_quad = (int*)calloc(p_Vid->imgpel_abs_range, sizeof(int))) == NULL)
    no_mem_exit("Init_Motion_Search_Module: p_Vid->imgpel_quad");
  p_Vid->imgpel_abs  += p_Vid->imgpel_abs_range / 2;
  p_Vid->imgpel_quad += p_Vid->imgpel_abs_range / 2;
#endif

  if (p_Vid->max_num_references)
    get_mem4Ddistblk (&p_Vid->motion_cost, 8, 2, p_Vid->max_num_references, 4);

  //--- set array offsets ---
  p_Vid->mvbits      += max_mvd;

  //=====   INIT ARRAYS   =====
  //---------------------------
  //--- init array: motion vector bits ---
  p_Vid->mvbits[0] = 1;
  for (bits = 3; bits <= max_mv_bits; bits += 2)
  {
    i_max = (short) (1 << (bits >> 1));
    i_min = i_max >> 1;

    for (i = i_min; i < i_max; i++)
      p_Vid->mvbits[-i] = p_Vid->mvbits[i] = bits;
  }

  //--- init array: reference frame bits ---
  p_Vid->refbits[0] = 1;
  for (bits=3; bits<=max_ref_bits; bits+=2)
  {
    i_max = (short) (1 << ((bits >> 1) + 1)) - 1;
    i_min = i_max >> 1;

    for (i = i_min; i < i_max; i++)
      p_Vid->refbits[i] = bits;
  }

#if (JM_MEM_DISTORTION)
  //--- init array: absolute value ---
  p_Vid->imgpel_abs[0] = 0;

  for (i=1; i<p_Vid->imgpel_abs_range / 2; i++)
  {
    p_Vid->imgpel_abs[i] = p_Vid->imgpel_abs[-i] = i;
  }

  //--- init array: square value ---
  p_Vid->imgpel_quad[0] = 0;

  for (i=1; i<p_Vid->imgpel_abs_range / 2; i++)
  {
    p_Vid->imgpel_quad[i] = p_Vid->imgpel_quad[-i] = i * i;
  }
#endif

  //--- init array: search pattern ---
  p_Vid->spiral_search[0].mv_x = p_Vid->spiral_search[0].mv_y = 0;
  p_Vid->spiral_hpel_search[0].mv_x = p_Vid->spiral_hpel_search[0].mv_y = 0;
  p_Vid->spiral_qpel_search[0].mv_x = p_Vid->spiral_qpel_search[0].mv_y = 0;

  for (k=1, l=1; l <= imax(1,search_range); l++)
  {
    for (i=-l+1; i< l; i++)
    {
      p_Vid->spiral_search[k].mv_x =     (short)  i;
      p_Vid->spiral_search[k].mv_y =     (short) -l;
      p_Vid->spiral_hpel_search[k].mv_x =   (short) (i<<1);
      p_Vid->spiral_hpel_search[k].mv_y =   (short) -(l<<1);
      p_Vid->spiral_qpel_search[k].mv_x =   (short)  (i<<2);
      p_Vid->spiral_qpel_search[k++].mv_y = (short) -(l<<2);
      p_Vid->spiral_search[k].mv_x =     (short)  i;
      p_Vid->spiral_search[k].mv_y =     (short)  l;
      p_Vid->spiral_hpel_search[k].mv_x =   (short) (i<<1);
      p_Vid->spiral_hpel_search[k].mv_y =   (short) (l<<1);
      p_Vid->spiral_qpel_search[k].mv_x =   (short) (i<<2);
      p_Vid->spiral_qpel_search[k++].mv_y = (short) (l<<2);
    }
    for (i=-l;   i<=l; i++)
    {
      p_Vid->spiral_search[k].mv_x =     (short) -l;
      p_Vid->spiral_search[k].mv_y =     (short)  i;
      p_Vid->spiral_hpel_search[k].mv_x =   (short) -(l<<1);
      p_Vid->spiral_hpel_search[k].mv_y =   (short)  (i<<1);
      p_Vid->spiral_qpel_search[k].mv_x =   (short) -(l<<2);
      p_Vid->spiral_qpel_search[k++].mv_y = (short)  (i<<2);
      p_Vid->spiral_search[k].mv_x =     (short)  l;
      p_Vid->spiral_search[k].mv_y =     (short)  i;
      p_Vid->spiral_hpel_search[k].mv_x =   (short) (l<<1);
      p_Vid->spiral_hpel_search[k].mv_y =   (short) (i<<1);
      p_Vid->spiral_qpel_search[k].mv_x =   (short) (l<<2);
      p_Vid->spiral_qpel_search[k++].mv_y = (short) (i<<2);
    }
  }

  // set global variable prior to ME
  p_Vid->start_me_refinement_hp = (p_Inp->ChromaMEEnable == 1 || p_Inp->MEErrorMetric[F_PEL] != p_Inp->MEErrorMetric[H_PEL] ) ? 0 : 1;
  p_Vid->start_me_refinement_qp = (p_Inp->ChromaMEEnable == 1 || p_Inp->MEErrorMetric[H_PEL] != p_Inp->MEErrorMetric[Q_PEL] ) ? 0 : 1;

  select_distortion(p_Vid, p_Inp);

  // Setup Distortion Metrics depending on refinement level
  for (i=0; i<3; i++)
  {
    switch(p_Inp->MEErrorMetric[i])
    {
    case ERROR_SAD:
      p_Vid->computeUniPred[i] = computeSAD;
      p_Vid->computeUniPred[i + 3] = computeSADWP;
      p_Vid->computeBiPred1[i] = computeBiPredSAD1;
      p_Vid->computeBiPred2[i] = computeBiPredSAD2;
      break;
    case ERROR_SSE:
      p_Vid->computeUniPred[i] = computeSSE;
      p_Vid->computeUniPred[i + 3] = computeSSEWP;
      p_Vid->computeBiPred1[i] = computeBiPredSSE1;
      p_Vid->computeBiPred2[i] = computeBiPredSSE2;
      break;
    case ERROR_SATD :
    default:
      p_Vid->computeUniPred[i] = computeSATD;
      p_Vid->computeUniPred[i + 3] = computeSATDWP;
      p_Vid->computeBiPred1[i] = computeBiPredSATD1;
      p_Vid->computeBiPred2[i] = computeBiPredSATD2;
      break;
    }
  }
  if (!p_Inp->IntraProfile)
  {
    if(p_Inp->SearchMode == FAST_FULL_SEARCH)
      InitializeFastFullIntegerSearch (p_Vid, p_Inp);

    if (p_Inp->SearchMode == UM_HEX)
      UMHEX_DefineThreshold(p_Vid);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Free memory used by motion search
 ************************************************************************
 */
void
Clear_Motion_Search_Module (VideoParameters *p_Vid, InputParameters *p_Inp)
{
  //int search_range               = p_Inp->search_range;
  //int number_of_subpel_positions = 4 * (2*search_range+3);
  //int max_mv_bits                = 3 + 2 * (int)ceil (log(number_of_subpel_positions + 1) / log(2) + 1e-10);
  int max_mvd                    = p_Vid->max_mvd; //(1<<( max_mv_bits >>1)   ) - 1;


  //--- correct array offset ---
  p_Vid->mvbits      -= max_mvd;
#if (JM_MEM_DISTORTION)
  p_Vid->imgpel_abs  -= p_Vid->imgpel_abs_range / 2;
  p_Vid->imgpel_quad -= p_Vid->imgpel_abs_range / 2;
#endif

  //--- delete arrays ---
  free (p_Vid->spiral_search);
  free (p_Vid->spiral_hpel_search);
  free (p_Vid->spiral_qpel_search);
  free (p_Vid->mvbits);
  free (p_Vid->refbits);

#if (JM_MEM_DISTORTION)
  free (p_Vid->imgpel_abs);
  free (p_Vid->imgpel_quad);
#endif


  if (p_Vid->motion_cost)
    free_mem4Ddistblk (p_Vid->motion_cost);

  if ((p_Inp->SearchMode == FAST_FULL_SEARCH) && (!p_Inp->IntraProfile) )
    ClearFastFullIntegerSearch (p_Vid);
}
static inline int mv_bits_cost(VideoParameters *p_Vid, short ***all_mv, short ***p_mv, int by, int bx, int step_v0, int step_v, int step_h0, int step_h, int mvd_bits)
{
  int v, h;
  for (v=by; v<by + step_v0; v+=step_v)
  {
    for (h=bx; h<bx + step_h0; h+=step_h)
    {
      mvd_bits += (int) p_Vid->mvbits[ all_mv[v][h][0] - p_mv[v][h][0] ];
      mvd_bits += (int) p_Vid->mvbits[ all_mv[v][h][1] - p_mv[v][h][1] ];
    }
  }
  return mvd_bits;
}

static inline int mv_bit_cost(Macroblock *currMB, MotionVector **all_mv, int cur_list, short cur_ref, int by, int bx, int step_v0, int step_v, int step_h0, int step_h, int mvd_bits)
{
  int v, h;
  MotionVector predMV;
  PixelPos block[4];  // neighbor blocks
  VideoParameters *p_Vid = currMB->p_Vid;

  for (v=by; v<by + step_v0; v+=step_v)
  {
    for (h=bx; h<bx + step_h0; h+=step_h)
    {


#if EXT3D
      get_neighbors(currMB, block, h<<2, v<<2, step_h<<2);
      if(currMB->p_Slice->depth_based_mvp_flag)
        check_neighbors(currMB,block,p_Vid->enc_picture->mv_info,cur_list,cur_ref);
      currMB->GetMVPredictor(currMB, block, &predMV, cur_ref, p_Vid->enc_picture->mv_info, cur_list, h<<2, v<<2, step_h<<2, step_v<<2,
        currMB->p_Slice->depth_based_mvp_flag);
#else
      get_neighbors(currMB, block, h, v, step_h);
      // Lets recompute MV predictor. This should avoid any problems with alterations of the motion vectors after ME
      currMB->GetMVPredictor (currMB, block, &predMV, cur_ref, p_Vid->enc_picture->mv_info, cur_list, h, v, step_h, step_v);
#endif

      mvd_bits += p_Vid->mvbits[ all_mv[v][h].mv_x - predMV.mv_x ];
      mvd_bits += p_Vid->mvbits[ all_mv[v][h].mv_y - predMV.mv_y ];
    }
  }

  return mvd_bits;
}

/*!
 ***********************************************************************
 * \brief
 *    Motion Cost for Bidirectional modes
 ***********************************************************************
 */
distblk BPredPartitionCost (Macroblock *currMB,
                        int   blocktype,
                        int   block8x8,
                        short ref_l0,
                        short ref_l1,
                        int   lambda_factor,
                        int   list)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  Slice *currSlice = currMB->p_Slice;
  imgpel **cur_img = p_Vid->pCurImg;

  short pic_pix_x, pic_pix_y;
  short  v, h;
  int i, j;
  distblk mcost;

  int   mvd_bits  = 0;

  short parttype  = (short) (blocktype < 4 ? blocktype : 4);
  short step_h0   = (part_size[ parttype][0]);
  short step_v0   = (part_size[ parttype][1]);
  short step_h    = (part_size[blocktype][0]);
  short step_v    = (part_size[blocktype][1]);  
  short by0_part  = by0[parttype][block8x8];
  short bx0_part  = bx0[parttype][block8x8];
  short block_size_x = block_size[blocktype][0];
  short block_size_y = block_size[blocktype][1];

  MotionVector  **all_mv_l0 = currSlice->bipred_mv[list][LIST_0][ref_l0][blocktype]; 
  MotionVector  **all_mv_l1 = currSlice->bipred_mv[list][LIST_1][ref_l1][blocktype]; 
  imgpel  **mb_pred    = currSlice->mb_pred[0];

  // List0 
  mvd_bits = mv_bit_cost(currMB, all_mv_l0, LIST_0, ref_l0, by0_part, bx0_part, step_v0, step_v, step_h0, step_h, mvd_bits);
  // List1
  mvd_bits = mv_bit_cost(currMB, all_mv_l1, LIST_1, ref_l1, by0_part, bx0_part, step_v0, step_v, step_h0, step_h, mvd_bits);

  mcost = weighted_cost (lambda_factor, mvd_bits);

  // Get prediction  
  for (v = by0_part << 2; v < (by0_part + step_v0) << 2; v += block_size_y)
  {
    for (h = (bx0_part << 2); h < (bx0_part + step_h0) << 2; h += block_size_x)
    {      
      luma_prediction_bi (currMB, h, v, block_size_x, block_size_y, blocktype, blocktype, ref_l0, ref_l1, list);
    }
  }

  //----- cost of residual signal -----
  if ((!currSlice->p_Inp->Transform8x8Mode) || (blocktype>4))
  {
    short diff16[16];
    short *diff;

    pic_pix_y = currMB->opix_y;
    pic_pix_x = currMB->pix_x;
    for (v = by0_part << 2; v < (by0_part + step_v0) << 2; v += 4)
    {            
      for (h = (bx0_part << 2); h < (bx0_part + step_h0) << 2; h += 4)
      {
        diff = diff16;
        for (j = v; j < v + 4; j++)
        {
          for (i = h; i < h + 4; i++)
            *diff++ = cur_img[pic_pix_y+j][pic_pix_x+i] - mb_pred[j][i];
        }
        mcost += p_Vid->distortion4x4 (diff16, DISTBLK_MAX);
      }
    }
  }
  else
  {
  short diff64[64];
  short *diff;

    pic_pix_y = currMB->opix_y;
    pic_pix_x = currMB->pix_x;
    for (v = by0_part << 2; v < (by0_part + step_v0) << 2; v += 8)
    {
      for (h = (bx0_part << 2); h < (bx0_part + step_h0) << 2; h += 8)
      {
        diff = diff64;
        for (j = v; j < v + 8; j++)
    {      
          for (i = h; i < h + 8; i++)
            *diff++ = cur_img[pic_pix_y+j][pic_pix_x+i] - mb_pred[j][i];
        }
        mcost += p_Vid->distortion8x8(diff64, DISTBLK_MAX);
      }
    }
  }

  return mcost;
}

void update_mv_block(Macroblock *currMB, MEBlock *mv_block, int h, int v)
{
  mv_block->block_x      = (short) h;
  mv_block->block_y      = (short) v;
  mv_block->pos_x        = (short) (currMB->pix_x  + (h << 2));
  mv_block->pos_y        = (short) (currMB->opix_y + (v << 2));
  mv_block->pos_x2       = (short) (mv_block->pos_x >> 2);
  mv_block->pos_y2       = (short) (mv_block->pos_y >> 2);
  mv_block->pos_x_padded = (short) (mv_block->pos_x << 2);
  mv_block->pos_y_padded = (short) (mv_block->pos_y << 2);

  mv_block->pos_cr_x     = (short) (mv_block->pos_x >> currMB->p_Vid->shift_cr_x);
  mv_block->pos_cr_y     = (short) (mv_block->pos_y >> currMB->p_Vid->shift_cr_y);
}

/*!
 ***********************************************************************
 * \brief
 *    Init motion vector block
 ***********************************************************************
 */
void init_mv_block(Macroblock *currMB, MEBlock *mv_block, short blocktype, int list, char ref_idx, short mb_x, short mb_y)
{
  InputParameters *p_Inp = currMB->p_Inp;
  VideoParameters *p_Vid = currMB->p_Vid;
  Slice *currSlice = currMB->p_Slice;
  mv_block->blocktype         = blocktype;
  mv_block->blocksize_x       = block_size[blocktype][0];  // horizontal block size
  mv_block->blocksize_y       = block_size[blocktype][1];  // vertical block size
  // update position info
  update_mv_block(currMB, mv_block, mb_x, mb_y);

  mv_block->list              = (char) list;
  mv_block->ref_idx           = ref_idx;

  mv_block->mv[LIST_0].mv_x   = 0;
  mv_block->mv[LIST_0].mv_y   = 0;
  mv_block->mv[LIST_1].mv_x   = 0;
  mv_block->mv[LIST_1].mv_y   = 0;
  // Init WP parameters
  mv_block->p_Vid             = p_Vid;
  mv_block->p_Slice           = currSlice;
  mv_block->cost              = INT_MAX;
  mv_block->search_pos2       = 9;
  mv_block->search_pos4       = 9;

  if (p_Inp->ChromaMEEnable)
    get_mem2Dpel(&mv_block->orig_pic, 3, mv_block->blocksize_x * mv_block->blocksize_y);
  else
    get_mem2Dpel(&mv_block->orig_pic, 1, mv_block->blocksize_x * mv_block->blocksize_y);
  
  mv_block->ChromaMEEnable = p_Inp->ChromaMEEnable;

#if EXT3D
  mv_block->apply_bi_weights = p_Inp->UseWeightedReferenceME &&
    ((currSlice->slice_type == B_SLICE) && ((p_Vid->active_pps->weighted_bipred_idc != 0)||(p_Inp->DepthRangeBasedWP)));
  mv_block->apply_weights    = p_Inp->UseWeightedReferenceME && 
    (( currSlice->weighted_prediction != 0 )||(p_Inp->DepthRangeBasedWP));
#else
  mv_block->apply_bi_weights = p_Inp->UseWeightedReferenceME && ((currSlice->slice_type == B_SLICE) && p_Vid->active_pps->weighted_bipred_idc != 0);
  mv_block->apply_weights    = p_Inp->UseWeightedReferenceME && ( currSlice->weighted_prediction != 0 );
#endif

  if (p_Inp->ChromaMEEnable)
  {    
    mv_block->blocksize_cr_x = (short) (mv_block->blocksize_x >> p_Vid->shift_cr_x);
    mv_block->blocksize_cr_y = (short) (mv_block->blocksize_y >> p_Vid->shift_cr_y);

    mv_block->ChromaMEWeight = p_Inp->ChromaMEWeight;
  }

  if (mv_block->apply_weights)
  {
    // If implicit WP, single list weights are always non weighted
    if ((currSlice->slice_type == B_SLICE) && (p_Vid->active_pps->weighted_bipred_idc == 2))
    {
      mv_block->computePredFPel   = p_Vid->computeUniPred[F_PEL];
      mv_block->computePredHPel   = p_Vid->computeUniPred[H_PEL];
      mv_block->computePredQPel   = p_Vid->computeUniPred[Q_PEL];
    }
    else
    {
      mv_block->computePredFPel   = p_Vid->computeUniPred[F_PEL + 3];
      mv_block->computePredHPel   = p_Vid->computeUniPred[H_PEL + 3];
      mv_block->computePredQPel   = p_Vid->computeUniPred[Q_PEL + 3];
    }
    mv_block->computeBiPredFPel = p_Vid->computeBiPred2[F_PEL];
    mv_block->computeBiPredHPel = p_Vid->computeBiPred2[H_PEL];
    mv_block->computeBiPredQPel = p_Vid->computeBiPred2[Q_PEL];    
  }
  else
  {
    mv_block->computePredFPel   = p_Vid->computeUniPred[F_PEL];
    mv_block->computePredHPel   = p_Vid->computeUniPred[H_PEL];
    mv_block->computePredQPel   = p_Vid->computeUniPred[Q_PEL];
    mv_block->computeBiPredFPel = p_Vid->computeBiPred1[F_PEL];
    mv_block->computeBiPredHPel = p_Vid->computeBiPred1[H_PEL];
    mv_block->computeBiPredQPel = p_Vid->computeBiPred1[Q_PEL];
  }
}

/*!
 ***********************************************************************
 * \brief
 *    free motion vector block
 ***********************************************************************
 */
void free_mv_block(MEBlock *mv_block)
{
  if (mv_block->orig_pic)
  {
    free_mem2Dpel(mv_block->orig_pic);
  }
}


void get_original_block(VideoParameters *p_Vid, MEBlock *mv_block)
{
  //==================================
  //=====   GET ORIGINAL BLOCK   =====
  //==================================
  imgpel *orig_pic_tmp = mv_block->orig_pic[0];
  int   bsx       = mv_block->blocksize_x;
  int   pic_pix_x = mv_block->pos_x;
  int   i, j;
  imgpel **cur_img = &p_Vid->pCurImg[mv_block->pos_y];

  for (j = 0; j < mv_block->blocksize_y; j++)
  {
    memcpy(orig_pic_tmp,&cur_img[j][pic_pix_x], bsx * sizeof(imgpel));
    orig_pic_tmp += bsx;
  }

  if ( p_Vid->p_Inp->ChromaMEEnable )
  {
    bsx       = mv_block->blocksize_cr_x;
    pic_pix_x = mv_block->pos_cr_x;

    // copy the original cmp1 and cmp2 data to the orig_pic matrix
    for ( i = 1; i<=2; i++)
    {
      cur_img = &p_Vid->pImgOrg[i][mv_block->pos_cr_y];
      orig_pic_tmp = mv_block->orig_pic[i];
      for (j = 0; j < mv_block->blocksize_cr_y; j++)
      {
        memcpy(orig_pic_tmp, &(cur_img[j][pic_pix_x]), bsx * sizeof(imgpel));
        orig_pic_tmp += bsx;
      }
    }
  }
}

void CheckSearchRange(VideoParameters *p_Vid, MotionVector *pPredMV, MotionVector *pSWC, MEBlock *mv_block)
{
  int iMaxMVD = p_Vid->max_mvd - 2;
  SearchWindow *searchRange = &mv_block->searchRange;
  int left  = pSWC->mv_x + searchRange->min_x;
  int right = pSWC->mv_x + searchRange->max_x;
  int top   = pSWC->mv_y + searchRange->min_y;
  int down  = pSWC->mv_y + searchRange->max_y;
  
  left  = iClip3(pPredMV->mv_x - iMaxMVD, pPredMV->mv_x + iMaxMVD, left);
  right = iClip3(pPredMV->mv_x - iMaxMVD, pPredMV->mv_x + iMaxMVD, right);
  top   = iClip3(pPredMV->mv_y - iMaxMVD, pPredMV->mv_y + iMaxMVD, top); 
  down  = iClip3(pPredMV->mv_y - iMaxMVD, pPredMV->mv_y + iMaxMVD, down);

  if(left<right && top<down)
  {
    pSWC->mv_x = (short) ((left + right)>>1);
    pSWC->mv_y = (short) ((top + down)>>1);
    searchRange->min_x = left - pSWC->mv_x;
    searchRange->max_x = imin(pSWC->mv_x-left, right-pSWC->mv_x);
    searchRange->min_y = top - pSWC->mv_y;
    searchRange->max_y = imin(pSWC->mv_y-top, down-pSWC->mv_y);
  }
  else
  {
    *pSWC = *pPredMV;
  }
}

/*!
 ***********************************************************************
 * \brief
 *    Block motion search
 ***********************************************************************
 */
distblk                                         //!< minimum motion cost after search
BlockMotionSearch (Macroblock *currMB,      //!< Current Macroblock
                   MEBlock   *mv_block,     //!< Motion estimation information block
                   int       mb_x,          //!< x-coordinate inside macroblock
                   int       mb_y,          //!< y-coordinate inside macroblock
                   int*      lambda_factor) //!< lagrangian parameter for determining motion cost
{
  // each 48-pel line stores the 16 luma pels (at 0) followed by 8 or 16 crcb[0] (at 16) and crcb[1] (at 32) pels
  // depending on the type of chroma subsampling used: YUV 4:4:4, 4:2:2, and 4:2:0
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;

  int   i, j;
  distblk   max_value = DISTBLK_MAX;
  distblk   min_mcost = max_value;
  int   block_x   = (mb_x>>2);
  int   block_y   = (mb_y>>2);

  int   bsx       = mv_block->blocksize_x;
  int   bsy       = mv_block->blocksize_y;

  short pic_pix_x = (short) (currMB->pix_x + mb_x);

  int  blocktype = mv_block->blocktype;
  int  list = mv_block->list;
  short ref = mv_block->ref_idx;
  MotionVector *mv = &mv_block->mv[list], pred; 

  MotionVector **all_mv = &currSlice->all_mv[list][ref][blocktype][block_y];

  distblk *prevSad = (p_Inp->SearchMode == EPZS)? currSlice->p_EPZS->distortion[list + currMB->list_offset][blocktype - 1]: NULL;


  get_neighbors(currMB, mv_block->block, mb_x, mb_y, bsx);

#if EXT3D
  if(currSlice->depth_based_mvp_flag)
    check_neighbors(currMB,mv_block->block,p_Vid->enc_picture->mv_info,list,ref);
#endif

  PrepareMEParams(currSlice, mv_block, p_Inp->ChromaMEEnable, list + currMB->list_offset, ref);

  //==================================
  //=====   GET ORIGINAL BLOCK   =====
  //==================================
  if (blocktype > 4)
    get_original_block(p_Vid, mv_block);

  //===========================================
  //=====   GET MOTION VECTOR PREDICTOR   =====
  //===========================================
  if (p_Inp->SearchMode == UM_HEX)
  {
    p_Vid->p_UMHex->UMHEX_blocktype = blocktype;
    p_Vid->p_UMHex->bipred_flag = 0;
    UMHEXSetMotionVectorPredictor(currMB, &pred, p_Vid->enc_picture->mv_info, ref, list, mb_x, mb_y, bsx, bsy, mv_block);
  }
  else if (p_Inp->SearchMode == UM_HEX_SIMPLE)
  {
    smpUMHEX_setup(currMB, ref, list, block_y, block_x, blocktype, currSlice->all_mv );
#if EXT3D
    currMB->GetMVPredictor (currMB, mv_block->block, &pred, ref, p_Vid->enc_picture->mv_info, list, mb_x, mb_y, bsx, bsy,
      currSlice->depth_based_mvp_flag);
#else
    currMB->GetMVPredictor (currMB, mv_block->block, &pred, ref, p_Vid->enc_picture->mv_info, list, mb_x, mb_y, bsx, bsy);
#endif
  }
  else
  {
#if EXT3D
    currMB->GetMVPredictor (currMB, mv_block->block, &pred, ref, p_Vid->enc_picture->mv_info, list, mb_x, mb_y, bsx, bsy,
      currSlice->depth_based_mvp_flag);
#else
    currMB->GetMVPredictor (currMB, mv_block->block, &pred, ref, p_Vid->enc_picture->mv_info, list, mb_x, mb_y, bsx, bsy);
#endif
  }



  //==================================
  //=====   INTEGER-PEL SEARCH   =====
  //==================================
  if (p_Inp->EPZSSubPelGrid)
  {
    *mv = pred;
  }
  else
  {
#if (JM_INT_DIVIDE)
    mv->mv_x = (short) (((pred.mv_x  + 2) >> 2) * 4);
    mv->mv_y = (short) (((pred.mv_y  + 2) >> 2) * 4);
#else
    mv->mv_x = (short) ((pred.mv_x / 4) * 4);
    mv->mv_y = (short) ((pred.mv_y / 4) * 4);
#endif
  }

  if (!p_Inp->rdopt)
  {
    MotionVector center = *mv;
    //--- adjust search center so that the (0,0)-vector is inside ---
    mv->mv_x = (short) iClip3 (mv_block->searchRange.min_x, mv_block->searchRange.max_x, mv->mv_x);
    mv->mv_y = (short) iClip3 (mv_block->searchRange.min_y, mv_block->searchRange.max_y, mv->mv_y);
    //mvbits overflow checking;
    if((mv->mv_x != center.mv_x) || (mv->mv_y != center.mv_y))
      CheckSearchRange(p_Vid, &center, mv, mv_block);
  }

  // valid search range limits could be precomputed once during the initialization process
  clip_mv_range(p_Vid, 0, mv, Q_PEL);

  //--- perform motion search ---
  min_mcost = currMB->IntPelME (currMB, &pred, mv_block, min_mcost, lambda_factor[F_PEL]);

  //==============================
  //=====   SUB-PEL SEARCH   =====
  //============================== 
  mv_block->ChromaMEEnable = (p_Inp->ChromaMEEnable == (Boolean) ME_YUV_FP_SP ) ? TRUE : FALSE; // set it externally

#if EXT3D
  if (!p_Inp->DisableSubpelME && !mv_block->isVSPRef[list])
#else
  if (!p_Inp->DisableSubpelME)
#endif
  {
    if (p_Inp->SearchMode != EPZS || (ref == 0 || currSlice->structure != FRAME || (ref > 0 && min_mcost < 3.5 * prevSad[pic_pix_x >> 2])))
    {
      if ( !p_Vid->start_me_refinement_hp )
      {
        min_mcost = max_value;
      }
      min_mcost =  currMB->SubPelME (currMB, &pred, mv_block, min_mcost, lambda_factor);
    }
  }

  // clip mvs after me is performed (is not exactly the best)
  // better solution is to modify search window appropriately
  clip_mv_range(p_Vid, 0, mv, Q_PEL);

  if (!p_Inp->rdopt)
  {
    // Get the skip mode cost
    if (blocktype == 1 && (currSlice->slice_type == P_SLICE|| (currSlice->slice_type == SP_SLICE) ))
    {
      distblk cost;
      FindSkipModeMotionVector (currMB);

      cost  = GetSkipCostMB (currMB, lambda_factor[Q_PEL]);
      if (cost < min_mcost)
      {
        min_mcost = cost;
        *mv = currSlice->all_mv [0][0][0][0][0];
      }
    } 
  }

  //===============================================
  //=====   SET MV'S AND RETURN MOTION COST   =====
  //===============================================

  // Set first line
  for (i=block_x; i < block_x + (bsx>>2); i++)
  {
    all_mv[0][i] = *mv;
  }

  // set all other lines
  for (j=1; j < (bsy>>2); j++)
  {
    memcpy(&all_mv[j][block_x], &all_mv[0][block_x], (bsx>>2) * sizeof(MotionVector));
  }


  // Bipred ME consideration: returns minimum bipred cost
  if (is_bipred_enabled(p_Vid, blocktype) && (ref == 0)) 
  {
    BiPredBlockMotionSearch(currMB, mv_block, &pred, mb_x, mb_y, lambda_factor);
  }

  return min_mcost;
}


/*!
 ***********************************************************************
 * \brief
 *    Bi-predictive motion search
 ***********************************************************************
 */
static distblk BiPredBlockMotionSearch(Macroblock *currMB,      //!< Current Macroblock
                                   MEBlock  *mv_block,
                                   MotionVector *pred_mv,     //!< current list motion vector predictor
                                   int       mb_x,            //!< x-coordinate inside macroblock
                                   int       mb_y,            //!< y-coordinate inside macroblock
                                   int*      lambda_factor)   //!< lagrangian parameter for determining motion cost
{
  VideoParameters *p_Vid     = currMB->p_Vid;
  InputParameters *p_Inp     = currMB->p_Inp;
  Slice           *currSlice = currMB->p_Slice;
  int         list = mv_block->list;
  int         i, j;
  short       bipred_type = list ? 0 : 1;
  MotionVector ***** bipred_mv = currSlice->bipred_mv[bipred_type];
  distblk     min_mcostbi = DISTBLK_MAX;
  MotionVector *mv = &mv_block->mv[list];
  MotionVector bimv, tempmv;
  MotionVector pred_mv1, pred_mv2, pred_bi;
  MotionVector *bi_mv1 = NULL, *bi_mv2 = NULL;
  short       iterlist = (short) list;  
  int         block_x   = (mb_x>>2);
  int         block_y   = (mb_y>>2);
  int  blocktype = mv_block->blocktype;
  int         bsx       = mv_block->blocksize_x;
  int         bsy       = mv_block->blocksize_y;
  //PixelPos    block[4];  // neighbor blocks
  
#if EXT3D
  MotionVector imp_mv1, imp_mv2;
  int RefViewId=-1;
  //int RefVOIdx=-1;

  get_neighbors(currMB, mv_block->block, mb_x, mb_y, mv_block->blocksize_x);
  if(currMB->p_Slice->depth_based_mvp_flag)
    check_neighbors(currMB,mv_block->block,p_Vid->enc_picture->mv_info,list^1,0);
#endif
  //get_neighbors(currMB, mv_block->block, mb_x, mb_y, bsx);

  if (p_Inp->SearchMode == UM_HEX)
  {
    p_Vid->p_UMHex->bipred_flag = 1;
    UMHEXSetMotionVectorPredictor(currMB, &pred_bi, p_Vid->enc_picture->mv_info, 0, list ^ 1, mb_x, mb_y, bsx, bsy, mv_block);
  }
  else
#if EXT3D
  {
    RefViewId=currSlice->listX[list^1][0]->view_id;
    // RefVOIdx=
    GetVOIdx(p_Inp,RefViewId);
    currMB->GetMVPredictor (currMB, mv_block->block, &pred_bi, 0, p_Vid->enc_picture->mv_info, list ^ 1, mb_x, mb_y, bsx, bsy,
      currSlice->depth_based_mvp_flag);
  }
#else
    currMB->GetMVPredictor (currMB, mv_block->block, &pred_bi, 0, p_Vid->enc_picture->mv_info, list ^ 1, mb_x, mb_y, bsx, bsy);
#endif

  if ((p_Inp->SearchMode != EPZS) || (p_Inp->EPZSSubPelGrid == 0))
  {
    mv->mv_x = ((mv->mv_x  + 2) >> 2) * 4;
    mv->mv_y = ((mv->mv_y  + 2) >> 2) * 4;
    bimv.mv_x = ((pred_bi.mv_x  + 2) >> 2) * 4;
    bimv.mv_y = ((pred_bi.mv_y  + 2) >> 2) * 4;
  }
  else
  {
    bimv = pred_bi;
  }

#if EXT3D
  if (mv_block->isVSPRef[(list)])  // if MV in current lsit is marked for implicit signaling, set predictor to equal to MV
  {
    for (j=block_y; j < block_y + (bsy>>2); j++)
    {
      for (i=block_x ; i < block_x + (bsx>>2); i++)
      {
        currMB->GetInterviewMVPredictionMax(currMB,&imp_mv1, (mv_block->block_x)<<2,(mv_block->block_y)<<2,mv_block->blocksize_x,mv_block->blocksize_y,((list)==0)?p_Vid->forward_disparity_table:p_Vid->backward_disparity_table);
        currMB->GetInterviewMVPredictionMax(currMB,&imp_mv2, (mv_block->block_x)<<2,(mv_block->block_y)<<2,mv_block->blocksize_x,mv_block->blocksize_y,((list ^ 1)==0)?p_Vid->forward_disparity_table:p_Vid->backward_disparity_table);
        clip_mv_range(p_Vid, 0, &imp_mv1, Q_PEL);
        clip_mv_range(p_Vid, 0, &imp_mv1, Q_PEL);
        currSlice->bipred_mv_implicit[bipred_type][list    ][(short) mv_block->ref_idx][blocktype][j][i] = imp_mv1;
        currSlice->bipred_mv_implicit[bipred_type][list ^ 1][(short) mv_block->ref_idx][blocktype][j][i] = imp_mv2;
      }
    }
  }
#endif

  //Bi-predictive motion Refinements
  for (mv_block->iteration_no = 0; mv_block->iteration_no <= p_Inp->BiPredMERefinements; mv_block->iteration_no++)
  {
    if (mv_block->iteration_no & 0x01)
    {
      pred_mv1  = *pred_mv;
      pred_mv2  = pred_bi;
      bi_mv1    = mv;
      bi_mv2    = &bimv;
      iterlist  = (short) list;
    }
    else
    {
      pred_mv1  = pred_bi;
      pred_mv2  = *pred_mv;
      bi_mv1    = &bimv;
      bi_mv2    = mv;
      iterlist = (short) (list ^ 1);
#if EXT3D
      if (mv_block->isVSPRef[(list)])  // if MV in current lsit is marked for implicit signaling, set predictor to equal to MV
      {
        pred_mv2  = *mv;  // Implicity signaling assumed for VSPRef==1, thus asign disparity vector
      }
      if (mv_block->isVSPRef[(list ^ 1)])  // another list is marked as using implicit signaling, thus no search is possible, set candidate 1 and predictors 1 equal to DV
      {
        currMB->GetInterviewMVPredictionMax(currMB,&pred_mv1, (mv_block->block_x)<<2,(mv_block->block_y)<<2,mv_block->blocksize_x,mv_block->blocksize_y,((list ^ 1)==0)?p_Vid->forward_disparity_table:p_Vid->backward_disparity_table);
        bi_mv1    = &pred_mv1;  // Implicity signaling assumed for VSPRef==1, thus asign disparity vector
      }
#endif
    }

    tempmv = *bi_mv1;

    PrepareBiPredMEParams(currSlice, mv_block, mv_block->ChromaMEEnable, iterlist, currMB->list_offset, mv_block->ref_idx);
    // Get bipred mvs for list iterlist given previously computed mvs from other list
    min_mcostbi = currMB->BiPredME (currMB, iterlist, 
      &pred_mv1, &pred_mv2, bi_mv1, bi_mv2, mv_block, 
      (p_Inp->BiPredMESearchRange <<2)>>mv_block->iteration_no, min_mcostbi, lambda_factor[F_PEL]);

    if (mv_block->iteration_no > 0 && (tempmv.mv_x == bi_mv1->mv_x) && (tempmv.mv_y == bi_mv1->mv_y))
    {
      break;
    }
#if EXT3D
    if (mv_block->isVSPRef[list])  
      break;          // Search around current disparity vector is avoided, due to implicit signaling
#endif
  }

  if (!p_Inp->DisableSubpelME)
  {
    if (p_Inp->BiPredMESubPel)
    {
      if ( !p_Vid->start_me_refinement_hp )
        min_mcostbi = DISTBLK_MAX;
      PrepareBiPredMEParams(currSlice, mv_block, mv_block->ChromaMEEnable, iterlist, currMB->list_offset, mv_block->ref_idx);

      min_mcostbi =  currMB->SubPelBiPredME (currMB, mv_block, iterlist, &pred_mv1, &pred_mv2, bi_mv1, bi_mv2, min_mcostbi, lambda_factor);
    }

    if (p_Inp->BiPredMESubPel==2)
    {
      if ( !p_Vid->start_me_refinement_qp )
        min_mcostbi = DISTBLK_MAX;
      PrepareBiPredMEParams(currSlice, mv_block, mv_block->ChromaMEEnable, iterlist ^ 1, currMB->list_offset, mv_block->ref_idx);

      min_mcostbi =  currMB->SubPelBiPredME (currMB, mv_block, iterlist ^ 1, &pred_mv2, &pred_mv1, bi_mv2, bi_mv1, min_mcostbi, lambda_factor);
    }
  }

  clip_mv_range(p_Vid, 0, bi_mv1, Q_PEL);
  clip_mv_range(p_Vid, 0, bi_mv2, Q_PEL);

  for (j=block_y; j < block_y + (bsy>>2); j++)
  {
    for (i=block_x ; i < block_x + (bsx>>2); i++)
    {
      bipred_mv[iterlist    ][(short) mv_block->ref_idx][blocktype][j][i] = *bi_mv1;
      bipred_mv[iterlist ^ 1][(short) mv_block->ref_idx][blocktype][j][i] = *bi_mv2;
    }
  }

#if EXT3D
  mv_block->biVSPCost[mv_block->isVSPRef[(list)]] = min_mcostbi;
#endif

  return min_mcostbi;
}

/*!
 ***********************************************************************
 * \brief
 *    Motion Cost for Bidirectional modes
 ***********************************************************************
 */
distblk BIDPartitionCost (Macroblock *currMB, 
                      int   blocktype,
                      int   block8x8,
                      char  cur_ref[2],
                      int   lambda_factor)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  Slice *currSlice = currMB->p_Slice;
  imgpel **cur_img = p_Vid->pCurImg;

  short pic_pix_x, pic_pix_y;
  int   v, h, i, j;
  distblk mcost;

  int   mvd_bits  = 0;

  int   parttype  = (blocktype < 4 ? blocktype : 4);
  int   step_h0   = part_size[ parttype][0];
  int   step_v0   = part_size[ parttype][1];
  int   step_h    = part_size[blocktype][0];
  int   step_v    = part_size[blocktype][1];
  int   bx        = bx0[parttype][block8x8];
  int   by        = by0[parttype][block8x8];
  short block_size_x = block_size[blocktype][0];
  short block_size_y = block_size[blocktype][1];

  MotionVector **all_mv_l0 = currSlice->all_mv [LIST_0][(int) cur_ref[LIST_0]][blocktype];
  MotionVector **all_mv_l1 = currSlice->all_mv [LIST_1][(int) cur_ref[LIST_1]][blocktype];
  short bipred_me =  0; //no bipred for this case 
  imgpel  **mb_pred = currSlice->mb_pred[0];
  int   list_mode[2];
  list_mode[0] = blocktype;
  list_mode[1] = blocktype;

  //----- cost for motion vector bits -----
  // Should write a separate, small function to do this processing
  // List0 
  // mvd_bits = mv_bits_cost(p_Vid, all_mv_l0, p_mv_l0, by, bx, step_v0, step_v, step_h0, step_h, mvd_bits);
  mvd_bits = mv_bit_cost(currMB, all_mv_l0, LIST_0, cur_ref[LIST_0], by, bx, step_v0, step_v, step_h0, step_h, mvd_bits);
  // List1
  // mvd_bits = mv_bits_cost(p_Vid, all_mv_l1, p_mv_l1, by, bx, step_v0, step_v, step_h0, step_h, mvd_bits);
  mvd_bits = mv_bit_cost(currMB, all_mv_l1, LIST_1, cur_ref[LIST_1], by, bx, step_v0, step_v, step_h0, step_h, mvd_bits);

  mcost = weighted_cost (lambda_factor, mvd_bits);

  // Get prediction
  for (v = by << 2; v < (by + step_v0) << 2; v += block_size_y)
  {
    for (h = (bx << 2); h < (bx + step_h0) << 2; h += block_size_x)
    {
      luma_prediction (currMB, h, v, block_size_x, block_size_y, 2, list_mode, cur_ref, bipred_me);
    }
  }

  //----- cost of residual signal -----
  if ((!currSlice->p_Inp->Transform8x8Mode) || (blocktype>4))
  {
    short diff16[16];
    short *diff;

    pic_pix_y = (short) currMB->opix_y;
    pic_pix_x = (short) currMB->pix_x;
    for (v= (by << 2); v < (by + step_v0) << 2; v += 4)
    {
      for (h = (bx << 2); h < (bx + step_h0) << 2; h += 4)
      {        
       diff = diff16;
        for (j = v; j < v + 4; j++)
        {
          for (i = h; i < h + 4; i++)
            *diff++ = cur_img[pic_pix_y+j][pic_pix_x+i] - mb_pred[j][i];
        }

        mcost += p_Vid->distortion4x4 (diff16, DISTBLK_MAX);
      }
    }
  }
  else
  {
    short diff64[64];
    short *diff;

    pic_pix_y = (short) currMB->opix_y;
    pic_pix_x = (short) currMB->pix_x;
    for (v= (by << 2); v < (by + step_v0) << 2; v += 8)
    {
      for (h = (bx << 2); h < (bx + step_h0) << 2; h += 8)
      {
        diff = diff64;
        for (j = v; j < v + 8; j++)
    {
          for (i = h; i < h + 8; i++)
            *diff++ = cur_img[pic_pix_y + j][pic_pix_x + i] - mb_pred[j][i];
      }
        mcost += p_Vid->distortion8x8(diff64, DISTBLK_MAX);
      }
    }
  }

  return mcost;
}

/*!
 ************************************************************************
 * \brief
 *    Get cost for skip mode for an macroblock
 ************************************************************************
 */
static distblk GetSkipCostMB (Macroblock *currMB, int lambda)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;  
  distblk cost = 0;

  int block;
  imgpel  **mb_pred = currSlice->mb_pred[0];
  char  cur_ref[2] = {0, 0};
  int   list_mode[2] = {0, 0};

  //===== prediction of 16x16 skip block =====
  luma_prediction (currMB, 0, 0, MB_BLOCK_SIZE, MB_BLOCK_SIZE, 0, list_mode, cur_ref, 0);

  if (p_Inp->Transform8x8Mode == 0)
  {
    short diff16[16];
    short *diff;
    int block_y, block_x, i, j;
    int mb_x, mb_y;

    int pic_pix_y = currMB->opix_y;
    int pic_pix_x = currMB->pix_x;

    for(block = 0;block < 4; block++)
    {
      mb_y = (block >>   1)<<3;
      mb_x = (block & 0x01)<<3;
      for (block_y = mb_y; block_y < mb_y + 8; block_y += 4)
      {
        for (block_x = mb_x; block_x < mb_x + 8; block_x += 4)
        {
          diff = diff16;
          //===== get displaced frame difference ======
          for (j = block_y; j < 4 + block_y; j++)
          {
            for (i = block_x; i < 4 + block_x; i++)
            {
              *diff++ = p_Vid->pCurImg[pic_pix_y + j][pic_pix_x + i] - mb_pred[j][i];
            }
          }
          cost += p_Vid->distortion4x4 (diff16, DISTBLK_MAX);
        }
      }
    }
  }
  else
  {
    short diff64[64];
    short *diff;
    int i, j;
    int mb_x, mb_y;

    int pic_pix_y = currMB->opix_y;
    int pic_pix_x = currMB->pix_x;

    for(block = 0;block < 4;block++)
    {
      mb_y = (block >>   1)<<3;
      mb_x = (block & 0x01)<<3;

      //===== get displaced frame difference ======
      diff = diff64;
      for (j = mb_y; j < mb_y + 8; j++)
      {
        for (i = mb_x; i < mb_x + 8; i++)
        {
          *diff++ = p_Vid->pCurImg[pic_pix_y + j][pic_pix_x + i] - mb_pred[j][i];
        }
      }
      cost += p_Vid->distortion8x8 (diff64, DISTBLK_MAX);
    }
  }

  //cost -= ((lambda_factor[Q_PEL] + 4096) >> 13);
  cost -= weight_cost(lambda, 8);

  return cost;
}

#if EXT3D
/*!
************************************************************************
* \brief
*    Prepare motion vector predictor for skip mode
************************************************************************
*/
void prepare_mvp_skip_params(Macroblock* currMB)
{

  //int i;
  PixelPos mb[4];
  VideoParameters *p_Vid = currMB->p_Vid;
  PicMotionParams **motion = p_Vid->enc_picture->mv_info;
  Boolean mvp_available=FALSE;
  MotionInfo motion_interview[2];
  int view_id;

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

  if(!mvp_available)
  {
    currMB->best_ref[0]=0;
    currMB->GetMVPredictor(currMB, mb, &currMB->best_mvp[0], 0, motion, LIST_0, 0, 0, 16, 16, currMB->p_Slice->depth_based_mvp_flag);
    mvp_available=TRUE;
  }
}

void FindSkipModeMotionVector_MVP(Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice;
  //VideoParameters *p_Vid = currMB->p_Vid;
  // InputParameters* p_Inp =currMB->p_Inp;
  int by,bx;

  prepare_mvp_skip_params(currMB);
  currMB->best_mvp[0]=(currMB->best_ref[0]!=-1)?currMB->best_mvp[0]:zero_mv;  // Dong: TODO: Bug fix. Move it one line above!!!!!
  currMB->best_ref[0]=(currMB->best_ref[0]>=0)?currMB->best_ref[0]:0;

  for (by = 0;by < 4;by++)
  {
    for (bx = 0;bx < 4;bx++)
    {
      currSlice->all_mv[LIST_0][currMB->best_ref[0]][0][by][bx] = currMB->best_mvp[0];
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Find motion vector for the ALC skip mode
 ************************************************************************
 */
void FindSkipModeMotionVector_4ALC(Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  PicMotionParams **motion = p_Vid->enc_picture->mv_info;
  int   bx, by;
  int idxInterviewRef = currSlice->idxInterviewRef[0];
  MotionVector **all_mv = currSlice->all_mv[0][idxInterviewRef][0];

  MotionVector pmv;

  PixelPos mb[4];

  get_neighbors(currMB, mb, 0, 0, 16);
  currMB->best_ref[0]=currMB->best_ref[1]=-1;
  currMB->best_mvp[0]=currMB->best_mvp[1]=zero_mv;

  currMB->GetMVPredictor (currMB, mb, &pmv, (short)idxInterviewRef, motion, LIST_0, 0, 0, 16, 16, currSlice->depth_based_mvp_flag);
  currMB->best_ref[0] = idxInterviewRef;
  currMB->best_mvp[0] = pmv;

  for (by = 0;by < 4;by++)
    for (bx = 0;bx < 4;bx++)
    {
      all_mv [by][bx] = pmv;
      if(currSlice->slice_type == B_SLICE)
      {
        currSlice->direct_pdir[currMB->block_y + by][currMB->block_x + bx] = 0;
        currSlice->direct_ref_idx[currMB->block_y + by][currMB->block_x + bx][0] = (char)idxInterviewRef;
        currSlice->direct_ref_idx[currMB->block_y + by][currMB->block_x + bx][1] = -1;
      }
    }


}
#endif
/*!
 ************************************************************************
 * \brief
 *    Find motion vector for the Skip mode
 ************************************************************************
 */
void FindSkipModeMotionVector (Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  PicMotionParams **motion = p_Vid->enc_picture->mv_info;
  int   bx, by;
  MotionVector **all_mv = currSlice->all_mv[0][0][0];

  MotionVector pmv;

  int zeroMotionAbove;
  int zeroMotionLeft;
  PixelPos mb[4];
  int      a_mv_y = 0;
  int      a_ref_idx = 0;
  int      b_mv_y = 0;
  int      b_ref_idx = 0;

  get_neighbors(currMB, mb, 0, 0, 16);

  if (mb[0].available)
  {
    a_mv_y    = motion[mb[0].pos_y][mb[0].pos_x].mv[LIST_0].mv_y;
    a_ref_idx = motion[mb[0].pos_y][mb[0].pos_x].ref_idx[LIST_0];

    if (currMB->mb_field && !p_Vid->mb_data[mb[0].mb_addr].mb_field)
    {
      a_mv_y    /=2;
      a_ref_idx *=2;
    }
    if (!currMB->mb_field && p_Vid->mb_data[mb[0].mb_addr].mb_field)
    {
      a_mv_y    *= 2;
      a_ref_idx >>=1;
    }
  }

  if (mb[1].available)
  {
    b_mv_y    = motion[mb[1].pos_y][mb[1].pos_x].mv[LIST_0].mv_y;
    b_ref_idx = motion[mb[1].pos_y][mb[1].pos_x].ref_idx[LIST_0];

    if (currMB->mb_field && !p_Vid->mb_data[mb[1].mb_addr].mb_field)
    {
      b_mv_y    /=2;
      b_ref_idx *=2;
    }
    if (!currMB->mb_field && p_Vid->mb_data[mb[1].mb_addr].mb_field)
    {
      b_mv_y    *=2;
      b_ref_idx >>=1;
    }
  }

  zeroMotionLeft  = !mb[0].available ? 1 : a_ref_idx==0 && motion[mb[0].pos_y][mb[0].pos_x].mv[LIST_0].mv_x ==0 && a_mv_y==0 ? 1 : 0;
  zeroMotionAbove = !mb[1].available ? 1 : b_ref_idx==0 && motion[mb[1].pos_y][mb[1].pos_x].mv[LIST_0].mv_x ==0 && b_mv_y==0 ? 1 : 0;

  if (zeroMotionAbove || zeroMotionLeft)
  {
    memset(&all_mv [0][0], 0, 16 * sizeof(MotionVector)); // 4 * 4
  }
  else
  {
#if EXT3D
    currMB->GetMVPredictor(currMB, mb, &pmv, 0, motion, LIST_0, 0, 0, 16, 16, 0);
#else
    currMB->GetMVPredictor (currMB, mb, &pmv, 0, motion, LIST_0, 0, 0, 16, 16);
#endif

    for (by = 0;by < 4;by++)
    for (bx = 0;bx < 4;bx++)
    {
      all_mv [by][bx] = pmv;
    }
  }
}

#if EXT3D
/*!
************************************************************************
* \brief
*    Find motion vector for the VSP Skip mode
************************************************************************
*/
void FindVSPSkipModeMotionVector (Macroblock *currMB, Block8x8Info *b8x8info)
{
  Slice *currSlice = currMB->p_Slice;
  MotionVector **all_mv = NULL;
  int idxVspRef;
  int block;
  int list;

  MotionVector disparity_mv;
  VideoParameters *p_Vid = currMB->p_Vid;
  int i,j,k,l;

  // LIST0
  list = 0;
  idxVspRef = currSlice->idxVspRef[list];
  if (idxVspRef >= 0)
  {
    all_mv = currSlice->all_mv[list][idxVspRef][10];

    for(j=0;j<16;j+=BLOCK_VSP_T)
    {
      for(i=0;i<16;i+=BLOCK_VSP_T)
      {
        currMB->GetInterviewMVPredictionMax(currMB,&(disparity_mv),i,j,BLOCK_VSP_T,BLOCK_VSP_T,p_Vid->forward_disparity_table);
        for(k=j;k<j+BLOCK_VSP_T;k+=4)
        {
          for(l=i;l<i+BLOCK_VSP_T;l+=4)
          {
            if(p_Vid->isTextureFirst)
            {
              all_mv[k>>2][l>>2].mv_x = currMB->iDoNBDV[3][j/BLOCK_VSP_T][i/BLOCK_VSP_T];
              all_mv[k>>2][l>>2].mv_y=0;
            }
            else
              all_mv[k>>2][l>>2]=disparity_mv;
          }
        }
      }
    }

    for (block = 0; block < 4; block++)
    {
      b8x8info->best[PVSP_SKIP][block].pdir = (char)list;
      b8x8info->best[PVSP_SKIP][block].mode = 10; // this is correct (in accordiance with how these mdoes are defined in ATM) - fixed for SMRC changes onlys
      b8x8info->best[PVSP_SKIP][block].ref[list]  = (char)idxVspRef;
      b8x8info->best[PVSP_SKIP][block].bipred = (char)list;
    }
  }
}


/*!
************************************************************************
* \brief
*    Calculate Spatial Direct Mode Motion Vectors 
************************************************************************
*/
void FindVSPDirectModeMotionVector(Macroblock *currMB, Block8x8Info *b8x8info)
{
  Slice *currSlice = currMB->p_Slice; 
  //VideoParameters *p_Vid = currMB->p_Vid;
  //InputParameters* p_Inp=currMB->p_Inp;
  int   block_x, block_y; //, pic_block_x, pic_block_y;
  MotionVector *****all_mvs;
  //StorablePicture**list1 = currSlice->listX[LIST_1];
  //int i=0,j=0;
  int idxVspRef, block;

  MotionVector disparity_mv;

  char best_ref[2]={-1,-1};
  MotionVector best_mvp[2];

  best_ref[0]=(char)currSlice->idxVspRef[0];
  best_mvp[0]=zero_mv;

  best_ref[1]=(char)currSlice->idxVspRef[1];
  best_mvp[1]=best_mvp[0];

  if (best_ref[0] == -1 && best_ref[1] == -1)
  {
    for (block_y=0; block_y<4; block_y++)
    {
      //pic_block_y  = currMB->block_y + block_y;
      for (block_x=0; block_x<4; block_x++)
      {
        //pic_block_x  = currMB->block_x + block_x;
        currSlice->all_mv[LIST_0][0][10][block_y][block_x] = zero_mv;
        currSlice->all_mv[LIST_1][0][10][block_y][block_x] = zero_mv;
      }
    }
  }
  else //if (best_ref[0] != -1 && best_ref[1] != -1)
  {
    for (block_y=0; block_y<4; block_y++)
    {
      //pic_block_y  = currMB->block_y + block_y;
      for (block_x=0; block_x<4; block_x++)
      {
        //pic_block_x  = currMB->block_x + block_x;
        all_mvs = currSlice->all_mv;

        if (best_ref[0] >= 0) // @DT: Bi-VSP support. not ">"
        {
          //direct
          currMB->GetInterviewMVPredictionMax(currMB,&(disparity_mv),(4*block_x)/BLOCK_VSP_T*BLOCK_VSP_T,(4*block_y)/BLOCK_VSP_T*BLOCK_VSP_T
            ,BLOCK_VSP_T,BLOCK_VSP_T,currMB->p_Vid->forward_disparity_table);
          if(currMB->p_Vid->isTextureFirst)
          {
            all_mvs[LIST_0][(short) best_ref[0]][10][block_y][block_x].mv_x=currMB->iDoNBDV[3][block_y/(BLOCK_VSP_T/BLOCK_SIZE)][block_x/(BLOCK_VSP_T/BLOCK_SIZE)];
            all_mvs[LIST_0][(short) best_ref[0]][10][block_y][block_x].mv_y=0;
          }
          else
            all_mvs[LIST_0][(short) best_ref[0]][10][block_y][block_x] = disparity_mv;
        }
        else
          all_mvs[LIST_0][0][10][block_y][block_x] = zero_mv;

        if (best_ref[1] >= 0) // @DT: Bi-VSP support. not ">"
        {
          currMB->GetInterviewMVPredictionMax(currMB,&(disparity_mv),(4*block_x)/BLOCK_VSP_T*BLOCK_VSP_T,(4*block_y)/BLOCK_VSP_T*BLOCK_VSP_T
            ,BLOCK_VSP_T,BLOCK_VSP_T,currMB->p_Vid->backward_disparity_table);
          if(currMB->p_Vid->isTextureFirst)
          {
            all_mvs[LIST_1][(short) best_ref[1]][10][block_y][block_x].mv_x=currMB->iDoNBDV[3][block_y/(BLOCK_VSP_T/BLOCK_SIZE)][block_x/(BLOCK_VSP_T/BLOCK_SIZE)];
            all_mvs[LIST_1][(short) best_ref[1]][10][block_y][block_x].mv_y=0;
          }
          else
            all_mvs[LIST_1][(short) best_ref[1]][10][block_y][block_x] = disparity_mv;
        }
        else
          all_mvs[LIST_1][0][10][block_y][block_x] = zero_mv;

      }
    }

    if (best_ref[0] != -1 && best_ref[1] == -1)
    {
      idxVspRef = currSlice->idxVspRef[0];
      for (block = 0; block < 4; block++)
      {
        b8x8info->best[BVSP_DIRECT][block].pdir = 0;
        b8x8info->best[BVSP_DIRECT][block].mode = 10;
        b8x8info->best[BVSP_DIRECT][block].ref[0]  = (char)idxVspRef;
        b8x8info->best[BVSP_DIRECT][block].bipred = 0;
      }
    }
    else if (best_ref[0] == -1 && best_ref[1] != -1)
    {
      idxVspRef = currSlice->idxVspRef[1];
      for (block = 0; block < 4; block++)
      {
        b8x8info->best[BVSP_DIRECT][block].pdir = 1;
        b8x8info->best[BVSP_DIRECT][block].mode = 10;
        b8x8info->best[BVSP_DIRECT][block].ref[1]  = (char)idxVspRef;
        b8x8info->best[BVSP_DIRECT][block].bipred = 0;
      }
    }
    else
    {
      for (block = 0; block < 4; block++)
      {
        b8x8info->best[BVSP_DIRECT][block].pdir = 2;

        b8x8info->best[BVSP_DIRECT][block].mode = 10;
        b8x8info->best[BVSP_DIRECT][block].ref[0] = (char)currSlice->idxVspRef[0];
        b8x8info->best[BVSP_DIRECT][block].ref[1] = (char)currSlice->idxVspRef[1];
        b8x8info->best[BVSP_DIRECT][block].bipred = 1;
      }
    }
  }
}
#endif

/*!
 ************************************************************************
 * \brief
 *    Get cost for direct mode for an 8x8 block
 ************************************************************************
 */
distblk GetDirectCost8x8 (Macroblock *currMB, int block, distblk *cost8x8)
{
  Slice *currSlice = currMB->p_Slice; 
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;
  int pic_pix_y, pic_pix_x, i, j;

  distblk cost  = 0;
  int mb_y  = (block >> 1)<<3;
  int mb_x  = (block & 0x01)<<3;
  imgpel  **mb_pred = currSlice->mb_pred[0];
  int   list_mode[2] = {0, 0};

  // Check if valid
  for (j=(currMB->opix_y + mb_y) >> 2; j < (currMB->opix_y + mb_y + 8) >> 2; j++)
  {
    for (i=(currMB->pix_x + mb_x) >> 2; i < (currMB->pix_x + mb_x + 8) >> 2; i++)
    {
      if (currSlice->direct_pdir[j][i] < 0)
      {
        *cost8x8 = DISTBLK_MAX;
        return DISTBLK_MAX; //mode not allowed
      }
    }
  }

  //===== Generate direct prediction =====
  for (j = mb_y; j < mb_y + 8; j += 4)
  {
    pic_pix_y = (currMB->opix_y + j) >> 2;
    for (i = mb_x; i < mb_x + 8; i += 4)
    {
      pic_pix_x = (currMB->pix_x + i) >> 2;
      luma_prediction (currMB, i, j, 4, 4, currSlice->direct_pdir[pic_pix_y][pic_pix_x], 
        list_mode, currSlice->direct_ref_idx[pic_pix_y][pic_pix_x], 0);
    }
  }

  if(p_Inp->Transform8x8Mode)
  {
    short diff16[4][16];
    short diff64[64];
    short *tmp64 = diff64;
    short *tmp16[4]; //{diff16[0], diff16[1], diff16[2], diff16[3]};
    int index;

    tmp16[0] = diff16[0];
    tmp16[1] = diff16[1];
    tmp16[2] = diff16[2];
    tmp16[3] = diff16[3];

    pic_pix_y = currMB->opix_y;
    pic_pix_x = currMB->pix_x;
    //===== get displaced frame difference ======
    for (j = mb_y; j < 8 + mb_y; j++)
    {
      for (i = mb_x; i < 8 + mb_x; i++)
      {
        index = 2 * ((j - mb_y)> 3) + ((i - mb_x)> 3);
        *tmp64++ = *(tmp16[index])++ = (short) (p_Vid->pCurImg[pic_pix_y + j][pic_pix_x + i] - mb_pred[j][i]);
      }
    }

    cost += p_Vid->distortion4x4 (diff16[0], DISTBLK_MAX);
    cost += p_Vid->distortion4x4 (diff16[1], DISTBLK_MAX);
    cost += p_Vid->distortion4x4 (diff16[2], DISTBLK_MAX);
    cost += p_Vid->distortion4x4 (diff16[3], DISTBLK_MAX);
    *cost8x8 += p_Vid->distortion8x8 (diff64, DISTBLK_MAX);
  }
  else
  {
    int block_y, block_x;
    short diff16[16];
    short *diff;

    for (block_y=mb_y; block_y < mb_y + 8; block_y += 4)
    {
      pic_pix_y = currMB->opix_y + block_y;

      for (block_x=mb_x; block_x<mb_x+8; block_x+=4)
      {
        pic_pix_x = currMB->pix_x + block_x;
        diff = diff16;

        //===== get displaced frame difference ======
        for (j=0; j<4; j++)
          for (i=0; i<4; i++)
          {
            *diff++ = (short) (p_Vid->pCurImg[pic_pix_y+j][pic_pix_x+i] - mb_pred[j+block_y][i+block_x]);
          }
          cost += p_Vid->distortion4x4 (diff16, DISTBLK_MAX);
      }
    }
  }

  return cost;
}



/*!

 ************************************************************************
 * \brief
 *    Get cost for direct mode for an macroblock
 ************************************************************************
 */
distblk GetDirectCostMB (Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice; 
  InputParameters *p_Inp = currSlice->p_Inp;
  int i;
  distblk cost = 0;
  distblk cost8x8 = 0;
  int bslice = currSlice->slice_type == B_SLICE;

  for (i=0; i<4; i++)
  {
    cost += GetDirectCost8x8 (currMB, i, &cost8x8);
    if (cost8x8 == DISTBLK_MAX) return DISTBLK_MAX;
  }

  switch(p_Inp->Transform8x8Mode)
  {
  case 1: // Mixture of 8x8 & 4x4 transform
    if((cost8x8 < cost)||
      !(p_Inp->InterSearch[bslice][5] &&
      p_Inp->InterSearch[bslice][6] &&
      p_Inp->InterSearch[bslice][7])
      )
    {
      cost = cost8x8; //return 8x8 cost
    }
    break;
  case 2: // 8x8 Transform only
    cost = cost8x8;
    break;
  default: // 4x4 Transform only
    break;
  }

  return cost;
}

/*!
 ************************************************************************
 * \brief
 *    Motion search for a macroblock partition
 ************************************************************************
 */
void PartitionMotionSearch (Macroblock *currMB,
                            int    blocktype,
                            int    block8x8,
                            int    *lambda_factor)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  Slice *currSlice = currMB->p_Slice;

#if GET_METIME
  TIME_T me_time_start;
  TIME_T me_time_end;
  int64 me_tmp_time;
  gettime( &me_time_start );    // start time ms
#endif

  if (currSlice->rdoq_motion_copy == 1)
  {
    PicMotionParams **motion = p_Vid->enc_picture->mv_info;
    short by = by0[blocktype][block8x8];
    short bx = bx0[blocktype][block8x8];
    short step_h = (part_size[blocktype][0]);
    short step_v = (part_size[blocktype][1]);

    short pic_block_y = currMB->block_y + by;
    short pic_block_x = currMB->block_x + bx;
    int   list_offset = currMB->list_offset;
    int   numlists    = (currSlice->slice_type == B_SLICE) ? 2 : 1;
    distblk *m_cost;

    short list = LIST_0;
    short ref = 0;

    //===== LOOP OVER REFERENCE FRAMES =====
    for (list = 0; list < numlists; list++)
    {
      for (ref=0; ref < currSlice->listXsize[list+list_offset]; ref++)
      {
        m_cost = &p_Vid->motion_cost[blocktype][list][ref][block8x8];

        //===== LOOP OVER SUB MACRO BLOCK partitions
        updateMV_mp(currMB, m_cost, ref, list, bx, by, blocktype, block8x8);
#if EXT3D
        set_me_parameters(motion, &currSlice->all_mv[list][ref][blocktype][by][bx], list, (char) ref, step_h, step_v, pic_block_y, pic_block_x,
          currSlice->isVSPRef[list][ref][blocktype][by][bx]);
#else
        set_me_parameters(motion, &currSlice->all_mv[list][ref][blocktype][by][bx], list, (char) ref, step_h, step_v, pic_block_y, pic_block_x);
#endif
      }
    }
  }
  else
  {
    InputParameters *p_Inp = currMB->p_Inp;
    short by = by0[blocktype][block8x8];
    short bx = bx0[blocktype][block8x8];
    short step_h    = (part_size[blocktype][0]);
    short step_v    = (part_size[blocktype][1]);

    short pic_block_y = currMB->block_y + by;
    short pic_block_x = currMB->block_x + bx;
    int   list_offset = currMB->list_offset;
    int   numlists  = (currSlice->slice_type == B_SLICE) ? 2 : 1;
    short list = LIST_0;
    short ref = 0;
    MEBlock  mv_block;
    distblk *m_cost;


    PicMotionParams **motion = p_Vid->enc_picture->mv_info;  

    // Set flag for 8x8 Hadamard consideration for SATD (only used when 8x8 integer transform is used for encoding)
    mv_block.test8x8 = p_Inp->Transform8x8Mode;

    init_mv_block(currMB, &mv_block, (short) blocktype, list, (char) ref, bx, by);

    if (p_Inp->SearchMode == EPZS)
    {
      if (p_Inp->EPZSSubPelGrid)
        currMB->IntPelME = EPZSIntPelBlockMotionSearch;
      else
        currMB->IntPelME = EPZSPelBlockMotionSearch;
    }

    get_original_block(p_Vid, &mv_block);

    //--- motion search for block ---   
    {
      //===== LOOP OVER REFERENCE FRAMES =====
      for (list = 0; list < numlists; list++)
      {
        //----- set arrays -----
        mv_block.list = (char) list;
        for (ref=0; ref < currSlice->listXsize[list+list_offset]; ref++) 
        {
#if EXT3D
            if (currSlice->slice_type == B_SLICE)
            {
              mv_block.biVSPCost[0] = DISTBLK_MAX;
              mv_block.biVSPCost[1] = DISTBLK_MAX;
            }

            // skip non-interview ref fremes for alc
            if ( currMB->is_alc_mode && (p_Vid->view_id == currSlice->listX[list][ref]->view_id || currSlice->listX[list][ref]->view_id != currSlice->listX[list][currSlice->idxInterviewRef[0]]->view_id))
            {
               mv_block.ref_idx = (char)ref;
               m_cost = &p_Vid->motion_cost[blocktype][list][ref][block8x8];
               *m_cost = DISTBLK_MAX/2;
               continue;
            }
#endif
            mv_block.ref_idx = (char) ref;
#if EXT3D
            mv_block.isVSPRef[0] = 0;
            mv_block.isVSPRef[1] = 0;
#endif
            m_cost = &p_Vid->motion_cost[blocktype][list][ref][block8x8];

            {
              //----- set search range ---
              get_search_range(&mv_block, p_Inp, ref, blocktype);

              //===== LOOP OVER MACROBLOCK partitions        
              *m_cost = BlockMotionSearch (currMB, &mv_block, bx<<2, by<<2, lambda_factor);     
            }
#if EXT3D
          if((currSlice->Harmonize_VSP_IVP) && (p_Vid->view_id != currSlice->listX[list][ref]->view_id) && (numlists==1) // @DT: Bi-VSP support. add numlists == 1
            && !( currMB->is_alc_mode && p_Vid->isTextureFirst ) )
          {
            int ref_lambda = (p_Inp->rdopt) ? lambda_factor[Q_PEL] :  lambda_factor[Q_PEL] >> 2;
            distblk disVSP=0;
            int selectVSP = 0;

            int j,i;

            int   bsx       = mv_block.blocksize_x;
            int   bsy       = mv_block.blocksize_y;
            MotionVector motion_EPZS = currSlice->p_EPZS?
              currSlice->p_EPZS->p_motion[list][ref][blocktype - 1][mv_block.block_y][mv_block.pos_x2]:zero_mv;
            distblk  distortion_EPZS = currSlice->p_EPZS?
              currSlice->p_EPZS->distortion[list][blocktype - 1][mv_block.pos_x2]:0;

            memcpy(&(currSlice->all_mv_backup[0][0][0][0][0]),&(currSlice->all_mv[0][0][0][0][0]),currSlice->mv_backup_size);

            if (currSlice->slice_type == B_SLICE)
              memcpy(&(currSlice->bipred_mv_backup[0][0][0][0][0][0]),&(currSlice->bipred_mv[0][0][0][0][0][0]),currSlice->bi_mv_backup_size); // copy bipred_mv to backup

            mv_block.isVSPRef[list] = 1;
            disVSP = BlockMotionSearch (currMB, &mv_block, bx<<2, by<<2, lambda_factor);
            disVSP += p_Vid->refbits[1]*ref_lambda;

            *m_cost += p_Vid->refbits[0]*ref_lambda;
            selectVSP = (disVSP<*m_cost)?1:0;

            if (currSlice->slice_type == B_SLICE)
            {
              if (mv_block.biVSPCost[1] > mv_block.biVSPCost[0]) // compare costs for bi-predictions with VSP=1 and VSP=0
              {
                memcpy(&(currSlice->bipred_mv[0][0][0][0][0][0]),&(currSlice->bipred_mv_backup[0][0][0][0][0][0]), currSlice->bi_mv_backup_size); // copy backup to bipred_mv
              }
            }
            
            for(j=by;j<by+(bsy>>2);++j)
            {
              for(i=bx;i<bx+(bsx>>2);++i)
              {
                currSlice->isVSPRef[list][ref][blocktype][j][i] = (short)selectVSP;
              }
            }
            if(0==selectVSP)
            {
              for(j=by;j<by+(bsy>>2);++j)
              {
                for(i=bx;i<bx+(bsx>>2);++i)
                {
                  currSlice->all_mv[list][ref][blocktype][j][i] = currSlice->all_mv_backup[list][ref][blocktype][j][i];
                }
              }
              if(currSlice->p_EPZS)
              {
                currSlice->p_EPZS->p_motion[list][ref][blocktype - 1][mv_block.block_y][mv_block.pos_x2] = motion_EPZS;
                currSlice->p_EPZS->distortion[list][blocktype - 1][mv_block.pos_x2] = distortion_EPZS;
              }
            }

            if(p_Vid->isTextureFirst&&selectVSP)
            {
              for (j = pic_block_y; j < pic_block_y + step_v; j++)
              {
                for (i = pic_block_x; i < pic_block_x + step_h; i++)
                {
                  assert(blocktype==P16x16||blocktype==P16x8||blocktype==P8x16||blocktype==P8x8);
                  currSlice->all_mv[list][ref][blocktype][j-currMB->block_y][i-currMB->block_x].mv_x = currMB->iDoNBDV[blocktype==P8x8 ? 3 : (blocktype-1)][(j-currMB->block_y)/(BLOCK_VSP_T/BLOCK_SIZE)][(i-currMB->block_x)/(BLOCK_VSP_T/BLOCK_SIZE)];
                  currSlice->all_mv[list][ref][blocktype][j-currMB->block_y][i-currMB->block_x].mv_y = 0;
                  motion[j][i].mv[list] = currSlice->all_mv[list][ref][blocktype][j-currMB->block_y][i-currMB->block_x];
                  motion[j][i].ref_idx[list] = ref;
                  motion[j][i].isVSPRef[list] = (short)selectVSP;
                }
              }
            }
            else
            {
              set_me_parameters(motion, &currSlice->all_mv[list][ref][blocktype][by][bx], list, (char) ref, step_h, step_v, pic_block_y, pic_block_x, (short)selectVSP);
            }

            mv_block.isVSPRef[list] = 0;
          }
          else
          {
            int j,i;

            int   bsx       = mv_block.blocksize_x;
            int   bsy       = mv_block.blocksize_y;

            set_me_parameters(motion, &currSlice->all_mv[list][ref][blocktype][by][bx], list, (char) ref, step_h, step_v, pic_block_y, pic_block_x, 0);

            for(j=by;j<by+(bsy>>2);++j)
            {
              for(i=bx;i<bx+(bsx>>2);++i)
              {
                currSlice->isVSPRef[list][ref][blocktype][j][i] = 0;
              }
            }
          }
#else
            //--- set motion vectors and reference frame ---            
            set_me_parameters(motion, &currSlice->all_mv[list][ref][blocktype][by][bx], list, (char) ref, step_h, step_v, pic_block_y, pic_block_x);
#endif
        }
      }
    }

    free_mv_block(&mv_block);
  }

#if GET_METIME
  gettime(&me_time_end);   // end time ms
  me_tmp_time = timediff (&me_time_start, &me_time_end);
  p_Vid->me_tot_time += me_tmp_time;
  p_Vid->me_time += me_tmp_time;
#endif
}

/*!
 ************************************************************************
 * \brief
 *    Motion search for a submacroblock partition
 ************************************************************************
 */
void SubPartitionMotionSearch (Macroblock *currMB,
                               int    blocktype,
                               int    block8x8,
                               int    *lambda_factor)

{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;  

#if GET_METIME
  TIME_T me_time_start;
  TIME_T me_time_end;
  int64 me_tmp_time;
  gettime( &me_time_start );    // start time ms
#endif

  if (currSlice->rdoq_motion_copy == 1)
  {
    int   parttype  = 4;
    PicMotionParams **motion = p_Vid->enc_picture->mv_info;
    short by = by0[parttype][block8x8];
    short bx = bx0[parttype][block8x8];
    short step_h    = (part_size[blocktype][0]);
    short step_v    = (part_size[blocktype][1]);
    int   list_offset = currMB->list_offset;
    int   numlists  = (currSlice->slice_type == B_SLICE) ? 2 : 1;
    distblk *m_cost;
    MotionVector *all_mv;
    short list = LIST_0;
    short ref = 0;

    short step_h0   = (part_size[ parttype][0]);
    short step_v0   = (part_size[ parttype][1]);

    int   v, h;
    int   pic_block_y;

    //===== LOOP OVER REFERENCE FRAMES =====
    for (list = 0; list < numlists; list++)
    {
      for (ref=0; ref < currSlice->listXsize[list+list_offset]; ref++)
      {
        m_cost = &p_Vid->motion_cost[blocktype][list][ref][block8x8];

        //===== LOOP OVER SUB MACRO BLOCK partitions
        for (v=by; v<by + step_v0; v += step_v)
        {
          pic_block_y = currMB->block_y + v;
          for (h=bx; h<bx+step_h0; h+=step_h)
          {
            all_mv = &currSlice->all_mv[list][ref][blocktype][v][h];

            updateMV_mp(currMB, m_cost, ref, list, h, v, blocktype, block8x8);

            //--- set motion vectors and reference frame (for motion vector prediction) ---
#if EXT3D
            set_me_parameters(motion, all_mv, list, (char) ref, step_h, step_v, pic_block_y, currMB->block_x + h,currSlice->isVSPRef[list][ref][blocktype][by][bx]);
#else
            set_me_parameters(motion, all_mv, list, (char) ref, step_h, step_v, pic_block_y, currMB->block_x + h);
#endif
          } // h
        } // v
      }
    }
  }
  else
  {
    InputParameters *p_Inp = currMB->p_Inp;
    PicMotionParams **motion = p_Vid->enc_picture->mv_info;
    int   parttype  = 4;
    short by = by0[parttype][block8x8];
    short bx = bx0[parttype][block8x8];
    short step_h0   = (part_size[ parttype][0]);
    short step_v0   = (part_size[ parttype][1]);
    short step_h    = (part_size[blocktype][0]);
    short step_v    = (part_size[blocktype][1]);
    int   list_offset = currMB->list_offset;
    int   numlists  = (currSlice->slice_type == B_SLICE) ? 2 : 1;
    MotionVector *all_mv;
    short list = LIST_0;
    short ref = 0;
    MEBlock  mv_block;
    distblk *m_cost;
    distblk  mcost; 
    int   v, h;
    int   pic_block_y;

    // Set if 8x8 transform will be used if SATD is used
    mv_block.test8x8 = p_Inp->Transform8x8Mode && blocktype == 4;

    if (p_Inp->SearchMode == EPZS)
    {
      if (p_Inp->EPZSSubPelGrid)
      {
        if (blocktype > 4)
          currMB->IntPelME = EPZSIntPelBlockMotionSearchSubMB;
        else
          currMB->IntPelME = EPZSIntPelBlockMotionSearch;
      }
      else
      {
        if (blocktype > 4)
          currMB->IntPelME = EPZSPelBlockMotionSearchSubMB;
        else
          currMB->IntPelME = EPZSPelBlockMotionSearch;
      }
    }

    init_mv_block(currMB, &mv_block, (short) blocktype, list, (char) ref, bx, by);

    if (blocktype == 4)
      get_original_block(p_Vid, &mv_block);

    //===== LOOP OVER REFERENCE FRAMES =====
    for (list=0; list<numlists;list++)
    {
      mv_block.list = (char) list;
      for (ref=0; ref < currSlice->listXsize[list+list_offset]; ref++)
      {
#if EXT3D 
          if (currSlice->slice_type == B_SLICE)
          {
            mv_block.biVSPCost[0] = DISTBLK_MAX;
            mv_block.biVSPCost[0] = DISTBLK_MAX;
          }

          // skip non-interview ref fremes for alc
          if ( currMB->is_alc_mode && (p_Vid->view_id == currSlice->listX[list][ref]->view_id || currSlice->listX[list][ref]->view_id != currSlice->listX[list][currSlice->idxInterviewRef[0]]->view_id))
          {
              mv_block.ref_idx = (char) ref;
              m_cost = &p_Vid->motion_cost[blocktype][list][ref][block8x8];
              *m_cost = DISTBLK_MAX/2;
              continue;
          }
#endif
          mv_block.ref_idx = (char) ref;
          m_cost = &p_Vid->motion_cost[blocktype][list][ref][block8x8];
          //----- set search range ---
          get_search_range(&mv_block, p_Inp, ref, blocktype);

          //----- init motion cost -----
          *m_cost = 0;

          //===== LOOP OVER SUB MACRO BLOCK partitions
          for (v=by; v<by + step_v0; v += step_v)
          {
            pic_block_y = currMB->block_y + v;

            for (h=bx; h<bx+step_h0; h+=step_h)
            {
              all_mv = &currSlice->all_mv[list][ref][blocktype][v][h];

#if EXT3D
              mv_block.isVSPRef[0] = 0;
              mv_block.isVSPRef[1] = 0;
#endif
              //--- motion search for block ---          
              update_mv_block(currMB, &mv_block, h, v);
              {
                //----- set search range ---
                get_search_range(&mv_block, p_Inp, ref, blocktype);

                mcost = BlockMotionSearch (currMB, &mv_block, h<<2, v<<2, lambda_factor);

                *m_cost += mcost;
              }
#if EXT3D
              if((currSlice->Harmonize_VSP_IVP)&&(p_Vid->view_id != currSlice->listX[list][ref]->view_id) && (numlists==1) // @DT: Bi-VSP support. add numlists==1
                && !( currMB->is_alc_mode && p_Vid->isTextureFirst ) && blocktype <=SMB8x8 )
              {
                int ref_lambda = (p_Inp->rdopt) ? lambda_factor[Q_PEL] :  lambda_factor[Q_PEL] >> 2;
                distblk disVSP=0;
                int selectVSP = 0;

                int j,i;

                int   bsx       = mv_block.blocksize_x;
                int   bsy       = mv_block.blocksize_y;

                MotionVector motion_EPZS = currSlice->p_EPZS?
                  currSlice->p_EPZS->p_motion[list][ref][blocktype - 1][mv_block.block_y][mv_block.pos_x2]:zero_mv;
                distblk  distortion_EPZS = currSlice->p_EPZS?
                  currSlice->p_EPZS->distortion[list][blocktype - 1][mv_block.pos_x2]:0;

                memcpy(&(currSlice->all_mv_backup[0][0][0][0][0]),&(currSlice->all_mv[0][0][0][0][0]),currSlice->mv_backup_size);

                if (currSlice->slice_type == B_SLICE)
                  memcpy(&(currSlice->bipred_mv_backup[0][0][0][0][0][0]),&(currSlice->bipred_mv[0][0][0][0][0][0]),currSlice->bi_mv_backup_size); // copy bipred_mv to backup

                mv_block.isVSPRef[list] = 1;
                disVSP = BlockMotionSearch (currMB, &mv_block, h<<2, v<<2, lambda_factor);
                disVSP += p_Vid->refbits[1]*ref_lambda;

                *m_cost += p_Vid->refbits[0]*ref_lambda;
                selectVSP = (disVSP<*m_cost)?1:0;

                if (currSlice->slice_type == B_SLICE)
                {
                  if (mv_block.biVSPCost[1] > mv_block.biVSPCost[0]) // compare costs for bi-predictions with VSP=1 and VSP=0
                  {
                    memcpy(&(currSlice->bipred_mv[0][0][0][0][0][0]),&(currSlice->bipred_mv_backup[0][0][0][0][0][0]), currSlice->bi_mv_backup_size); // copy backup to bipred_mv
                  }
                }

                for(j=v;j<v+(bsy>>2);++j)
                {
                  for(i=h;i<h+(bsx>>2);++i)
                  {
                    currSlice->isVSPRef[list][ref][blocktype][j][i] = (short)selectVSP;
                  }
                }
                if(0==selectVSP)
                {
                  for(j=v;j<v+(bsy>>2);++j)
                  {
                    for(i=h;i<h+(bsx>>2);++i)
                    {
                      currSlice->all_mv[list][ref][blocktype][j][i] = currSlice->all_mv_backup[list][ref][blocktype][j][i];
                    }
                  }
                 if(currSlice->p_EPZS)
                  {
                    currSlice->p_EPZS->p_motion[list][ref][blocktype - 1][mv_block.block_y][mv_block.pos_x2] = motion_EPZS;
                    currSlice->p_EPZS->distortion[list][blocktype - 1][mv_block.pos_x2] = distortion_EPZS;
                  }
                }

                if(p_Vid->isTextureFirst&&selectVSP)
                {
                  for(j=v;j<v+(bsy>>2);++j)
                  {
                    for(i=h;i<h+(bsx>>2);++i)
                    {
                      currSlice->all_mv[list][ref][blocktype][j][i].mv_x = currMB->iDoNBDV[3][j/(BLOCK_VSP_T/BLOCK_SIZE)][i/(BLOCK_VSP_T/BLOCK_SIZE)];
                      currSlice->all_mv[list][ref][blocktype][j][i].mv_y = 0;
                    }
                  }
                }

                //--- set motion vectors and reference frame (for motion vector prediction) ---             
                set_me_parameters(motion, all_mv, list, (char) ref, step_h, step_v, pic_block_y, currMB->block_x + h, (short)selectVSP);
                mv_block.isVSPRef[list] = 0;
              }
              else
              {
                int j,i;

                int   bsx       = mv_block.blocksize_x;
                int   bsy       = mv_block.blocksize_y;

                //--- set motion vectors and reference frame (for motion vector prediction) ---
                set_me_parameters(motion, all_mv, list, (char) ref, step_h, step_v, pic_block_y, currMB->block_x + h, 0);

                for(j=v;j<v+(bsy>>2);++j)
                {
                  for(i=h;i<h+(bsx>>2);++i)
                  {
                    currSlice->isVSPRef[list][ref][blocktype][j][i] = 0;
                  }
                }
              }
#else
              //--- set motion vectors and reference frame (for motion vector prediction) ---
              set_me_parameters(motion, all_mv, list, (char) ref, step_h, step_v, pic_block_y, currMB->block_x + h);
#endif
            }
          }

          if ((p_Inp->Transform8x8Mode == 1) && p_Inp->RDOQ_CP_MV && (blocktype == 4))
          {
            if (currMB->luma_transform_size_8x8_flag)
            {
              currSlice->tmp_mv8[list][ref][by][bx] = currSlice->all_mv[list][ref][blocktype][by][bx];
              currSlice->motion_cost8[list][ref][block8x8] = *m_cost;
            }
            else
            {
              currSlice->tmp_mv4[list][ref][by][bx] = currSlice->all_mv[list][ref][blocktype][by][bx];
              currSlice->motion_cost4[list][ref][block8x8] = *m_cost;
            }
          }
      }

    }

    free_mv_block(&mv_block);
  }

#if GET_METIME
  gettime(&me_time_end);   // end time ms
  me_tmp_time = timediff (&me_time_start, &me_time_end);
  p_Vid->me_tot_time += me_tmp_time;
  p_Vid->me_time += me_tmp_time;
#endif
}


