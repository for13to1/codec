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



#include "global.h"

#if EXT3D 
#include "3dv_acquisition.h"
#include "3dv_refdisp.h"

void init_ref_display_info_SEI(ReferenceDisplayInfoSEI* reference_display_info)
{
    reference_display_info->ref_baseline.pred_mode=1;
    reference_display_info->ref_baseline.exponent_size = 6;


    reference_display_info->ref_display_width.pred_mode = 1;
    reference_display_info->ref_display_width.exponent_size = 6;


    reference_display_info->ref_viewing_distance.pred_mode = 1;
    reference_display_info->ref_viewing_distance.exponent_size = 6;
}
#endif
