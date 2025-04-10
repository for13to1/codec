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
 *    mmco.h
 * \brief
 *    MMCO example operations.
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Alexis Michael Tourapis                     <alexismt@ieee.org>
 *     - Athanasios Leontaris                        <aleon@dolby.com>
 ***************************************************************************************
 */

#ifndef _MMCO_H_
#define _MMCO_H_

extern void mmco_long_term(VideoParameters *p_Vid, int current_pic_num);
extern void poc_based_ref_management_frame_pic(DecodedPictureBuffer *p_Dpb, int current_pic_num);
extern void poc_based_ref_management_field_pic(DecodedPictureBuffer *p_Dpb, int current_pic_num);
extern void tlyr_based_ref_management_frame_pic(VideoParameters *p_Vid, int current_pic_num); 
#if EXT3D
extern void mvc_based_ref_management_frame_pic(DecodedPictureBuffer *p_Dpb,Slice* currSlice, int current_pic_num)  ;

// ADAPTIVE_MMCO_REORDER
extern void layered_sliding_ref_management_frame_pic(DecodedPictureBuffer *p_Dpb, int current_pic_num);
#endif

#endif 
