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
 *  \file
 *      mbuffer_mvc.h
 *
 *  \brief
 *      MVC Frame buffer functions
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Athanasios Leontaris     <aleon@dolby.com>
 *      - Karsten Sühring          <suehring@hhi.de>
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 ***********************************************************************
 */
#ifndef _MBUFFER_MVC_H_
#define _MBUFFER_MVC_H_

#include "global.h"

#if MVC_EXTENSION_ENABLE||EXT3D
extern void reorder_lists_mvc     (Slice * currSlice, int currPOC);
extern void init_lists_mvc        (Slice *currSlice);
extern void init_lists_p_slice_mvc(Slice *currSlice);
extern void init_lists_b_slice_mvc(Slice *currSlice);
extern void init_lists_i_slice_mvc(Slice *currSlice);

extern void reorder_ref_pic_list_mvc(Slice *currSlice, int cur_list, int **anchor_ref, int **non_anchor_ref,
                                                 int view_id, int anchor_pic_flag, int currPOC, int listidx);

extern void reorder_short_term(Slice *currSlice, int cur_list, int num_ref_idx_lX_active_minus1, int picNumLX, int *refIdxLX, int currViewID);
extern void reorder_long_term(Slice *currSlice, StorablePicture **RefPicListX, int num_ref_idx_lX_active_minus1, int LongTermPicNum, int *refIdxLX, int currViewID);
#endif

#endif

