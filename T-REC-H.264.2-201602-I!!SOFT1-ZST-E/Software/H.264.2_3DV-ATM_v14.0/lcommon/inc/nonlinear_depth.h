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


#ifndef DB_NONLINEAR_DEPTH_H
#define DB_NONLINEAR_DEPTH_H

#include "typedefs.h"

#define NONLINEAR_DEPTH_LUT_SIZE 256

extern double nonlinear_depth_lerp(double value, imgpel lut[NONLINEAR_DEPTH_LUT_SIZE]);

extern void nonlinear_depth_init_lut ( imgpel lut[NONLINEAR_DEPTH_LUT_SIZE], int nonlinear_depth_num, char *nonlinear_depth_points, int forward );
extern void nonlinear_depth_process_buf2d_uchar   ( unsigned char **out, unsigned char **in, int size_x, int size_y, int nonlinear_depth_num, char *nonlinear_depth_points, int forward );
extern void nonlinear_depth_process_buf2d_imgpel  ( imgpel **out, imgpel **in, int size_x, int size_y, int nonlinear_depth_num, char *nonlinear_depth_points, int forward );
extern void nonlinear_depth_process_buf2d_with_pad_imgpel ( imgpel **out, imgpel **in, int size_x, int size_y, int nonlinear_depth_num, char *nonlinear_depth_points, int pad_x, int pad_y, int forward );

#endif

