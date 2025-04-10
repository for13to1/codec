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
 *    rdopt_coding_state.h
 *
 * \author
 *    Heiko Schwarz
 *
 * \date
 *    17. April 2001
 *
 * \brief
 *    Headerfile for storing/restoring coding state
 *    (for rd-optimized mode decision)
 **************************************************************************
 */

#ifndef _RD_OPT_CS_H_
#define _RD_OPT_CS_H_

struct coding_state {

  // important variables of data partition array
  int                  no_part;
  Bitstream            *bitstream;
  EncodingEnvironment  *encenv;

  // contexts for binary arithmetic coding
  MotionInfoContexts   *mot_ctx;
  TextureInfoContexts  *tex_ctx;

  // bit counter
  BitCounter            bits;

  // elements of current macroblock
  short                 mvd[2][BLOCK_MULTIPLE][BLOCK_MULTIPLE][2];
  int64                 cbp_bits[3];
  int64                 *cbp_bits_8x8;
};

typedef struct coding_state CSobj;

extern void  delete_coding_state  (CSobj *);  //!< delete structure
extern CSobj *create_coding_state  (InputParameters *p_Inp);       //!< create structure

extern void init_coding_state_methods(Slice *currSlice);  //!< Init methods given entropy coding


#endif

