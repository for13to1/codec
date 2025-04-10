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
 *    list_reorder.h
 *
 * \date
 *    25 Feb 2009
 *
 * \brief
 *    Headerfile for slice-related functions
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Athanasios Leontaris            <aleon@dolby.com>
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 **************************************************************************
 */

#ifndef _LIST_REORDER_H_
#define _LIST_REORDER_H_

#include "global.h"
#include "mbuffer.h"

extern void init_ref_pic_list_reordering( Slice *currSlice, int refReorderMethod );
extern void reorder_lists               ( Slice *currSlice );
extern void wp_mcprec_reorder_lists     ( Slice *currSlice );


extern void poc_ref_pic_reorder_frame_default( Slice *currSlice, StorablePicture **list, unsigned num_ref_idx_lX_active, 
                                       int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no );
extern void poc_ref_pic_reorder_field( Slice *currSlice, StorablePicture **list, unsigned num_ref_idx_lX_active, 
                               int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no );

extern void tlyr_ref_pic_reorder_frame_default( Slice *currSlice, StorablePicture **list, unsigned num_ref_idx_lX_active, 
                                       int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no );

#if EXT3D
extern void poc_ref_pic_reorder_frame_mvc_default( Slice *currSlice, StorablePicture **list, unsigned num_ref_idx_lX_active, 
                                                   int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no );
extern void interview_ref_pic_reorder_frame_mvc  ( Slice *currSlice, FrameStore ** fs, unsigned num_ref_idx_lX_active, 
                                                   int *reordering_of_interview_nums_idc, int *abs_diff_view_idx_minus1, int list_no );
extern void inter_ref_pic_reorder_frame_mvc      (Slice*currSlice,int* reordering_of_pic_nums_idc,
                                                   int* abs_diff_pic_num_minus1,int list_no);
extern void get_mvc_lists                        (Slice* currSlice);
#endif

#endif
