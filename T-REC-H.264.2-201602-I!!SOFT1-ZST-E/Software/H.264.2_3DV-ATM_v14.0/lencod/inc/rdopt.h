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
 * \file
 *    rdopt.h
 *
 * \author
 *    Alexis Michael Tourapis
 *
 * \date
 *    2 January 2008
 *
 * \brief
 *    Headerfile for RDO
 **************************************************************************
 */

#ifndef _RDO_H_
#define _RDO_H_

#include "global.h"
#include "rdopt_coding_state.h"

// Motion Vector structure
typedef struct
{
  double rdcost;
  double dcost;
  double rate;   // why is rate a double? Could we use int64 or even simply int given that this is for a MB? Could a MB require int64 bits (answer is no)? 
  int   cbp;
  distblk  cost;
  short c_imode;
  short i16offset;
  byte  mode;
} BestMode;

struct rdo_structure
{
  RD_8x8DATA *tr4x4;
  RD_8x8DATA *tr8x8;
  imgpel ***rec8x8;
  imgpel ***rec4x4;
  imgpel **pred;
  imgpel ***rec_mb;
  int    **lrec_rec;
  int   ***lrec_rec_uv;

  int   ****cofAC;
  int    ***cofDC; 
  int     **cofAC4x4;
  int   ****cofAC4x4intern;

  MotionVector ****all_mv8x8; 

#if EXT3D
  short ****isVSPRef8x8;
  short ****std_isVSPRef8x8;
  int       std_isVSPRef8x8_size;

  MotionVector  ****std_all_mv8x8; 
  int               all_mv8x8_size;
#endif

  int   *****cofAC4x4CbCrintern;
  int   *****cofAC8x8ts;        // [plane][8x8block][4x4block][level/run][scan_pos]
  int   *****coefAC8x8;
  int   *****coefAC8x8intra;
  int   **cofAC4x4CbCr[2];


  int   cbp;
  char  **l0_refframe;
  char  **l1_refframe;

  Info8x8 best8x8[4];

  CSobj *cs_mb;
  CSobj *cs_b8;
  CSobj *cs_cm;
  CSobj *cs_tmp;

  BestMode mode_best;

// adaptive langrangian parameters
  double lambda_mf_factor;
};


extern int  GetBestTransformP8x8 (Macroblock *currMB);
extern byte field_flag_inference (Macroblock  *currMB);
extern distblk rdcost_for_4x4_intra_blocks     (Macroblock *currMB, int* nonzero, int b8, int b4, int ipmode, int lambda, int mostProbableMode, distblk min_rdcost);
extern distblk rdcost_for_4x4_intra_blocks_444 (Macroblock *currMB, int* nonzero, int b8, int b4, int ipmode, int lambda, int mostProbableMode, distblk min_rdcost);
extern int valid_intra_mode(Slice *currSlice, int ipmode);

extern void init_md_best(BestMode  *best);

//============= rate-distortion optimization ===================
extern void  clear_rdopt (Slice *currSlice);
extern void  init_rdopt  (Slice *currSlice);

extern void UpdatePixelMap(VideoParameters *p_Vid, InputParameters *p_Inp);

extern void   update_qp_cbp     (Macroblock *currMB);
extern void   update_qp_cbp_tmp (Macroblock *currMB, int cbp);

extern void alloc_rd8x8data (RD_8x8DATA *rd_data);
extern void free_rd8x8data  (RD_8x8DATA *rd_data);

extern void restore_nz_coeff(Macroblock *currMB);

extern void end_encode_one_macroblock(Macroblock *currMB);

extern void encode_one_macroblock_low          (Macroblock *currMB);
extern void encode_one_macroblock_high         (Macroblock *currMB);
extern void encode_one_macroblock_highfast     (Macroblock *currMB);
extern void encode_one_macroblock_highloss     (Macroblock *currMB);

extern void store_8x8_motion_vectors_p_slice     (Slice *currSlice, int dir, int block8x8, Info8x8 *B8x8Info);
extern void store_8x8_motion_vectors_b_slice     (Slice *currSlice, int dir, int block8x8, Info8x8 *B8x8Info);

extern void set_modes_and_refs_for_blocks_p_slice(Macroblock *currMB, short mode);
extern void set_modes_and_refs_for_blocks_b_slice(Macroblock *currMB, short mode);
extern void set_modes_and_refs_for_blocks_i_slice(Macroblock *currMB, short mode);

extern void set_coeff_and_recon_8x8_p_slice      (Macroblock* currMB);
extern void set_coeff_and_recon_8x8_b_slice      (Macroblock* currMB);

extern distblk compute_sad4x4_cost (VideoParameters *p_Vid, imgpel **cur_img, imgpel **prd_img, int pic_opix_x, distblk min_cost);

extern Info8x8 init_info_8x8_struct(void);

/*!
 *************************************************************************************
 * \brief
 *    copy data in iblock to oblock
 *************************************************************************************
 */
static inline void copy_4x4block(imgpel **oblock, imgpel **iblock, int o_xoffset, int i_xoffset)
{
  int y;
  for (y = 0; y < BLOCK_SIZE; y++)
  {
    memcpy(&oblock[y][o_xoffset],&iblock[y][i_xoffset], BLOCK_SIZE * sizeof(imgpel));
  }
}
#endif

