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
 *    mode_decision_p8x8.h
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Alexis Michael Tourapis         <alexismt@ieee.org>
 *
 * \date
 *    1 September 2009
 *
 * \brief
 *    Headerfile for 8x8 mode decision
 **************************************************************************
 */

#ifndef _MODE_DECISION_P8x8_H_
#define _MODE_DECISION_P8x8_H_

//==== MODULE PARAMETERS ====
//static const int  b8_mode_table[6]   = {0, 4, 5, 6, 7};                             // DO NOT CHANGE ORDER !!!
//static const char mb_mode_table[10]  = {0, 1, 2, 3, P8x8, I16MB, I4MB, I8MB, IPCM, SI4MB}; // DO NOT CHANGE ORDER !!!

extern void submacroblock_mode_decision_p_slice (Macroblock *currMB, RD_PARAMS *, RD_8x8DATA *, int ****, int, distblk *);
extern void submacroblock_mode_decision_b_slice (Macroblock *currMB, RD_PARAMS *, RD_8x8DATA *, int ****, int, distblk *);
extern void submacroblock_mode_decision_low(Macroblock *currMB, RD_PARAMS *, RD_8x8DATA *, int ****, int *, int, distblk *, distblk *, distblk *, int);
extern void copy_part_info(Info8x8 *b8x8, Info8x8 *part);

#endif

