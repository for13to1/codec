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
 **************************************************************************
 * \file defines.h
 *
 * \brief
 *    Header file containing some useful global definitions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Detlev Marpe
 *     - Karsten S�hring                 <suehring@hhi.de> 
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 *   
 *
 * \date
 *    21. March 2001
 **************************************************************************
 */


#ifndef _DEFINES_H_
#define _DEFINES_H_
#ifdef TRACE
#undef TRACE
#endif
#if defined _DEBUG
# define TRACE           0     //!< 0:Trace off 1:Trace on 2:detailed CABAC context information
#else
# define TRACE           0     //!< 0:Trace off 1:Trace on 2:detailed CABAC context information
#endif

#define JM                  "17 (FRExt)"
#define VERSION             "17.2"
#define EXT_VERSION         "(FRExt)"

#define DUMP_DPB                  0    //!< Dump DPB info for debug purposes
#define PRINTREFLIST              0    //!< Print ref list info for debug purposes
#define PAIR_FIELDS_IN_OUTPUT     0    //!< Pair field pictures for output purposes
#define IMGTYPE                   1    //!< Define imgpel size type. 0 implies byte (cannot handle >8 bit depths) and 1 implies unsigned short
#define ENABLE_FIELD_CTX          1    //!< Enables Field mode related context types for CABAC
#define ENABLE_HIGH444_CTX        1    //!< Enables High 444 profile context types for CABAC. 
#define ZEROSNR                   0    //!< PSNR computation method
#define ENABLE_OUTPUT_TONEMAPPING 1    //!< enable tone map the output if tone mapping SEI present
#define JCOST_CALC_SCALEUP        1    //!< 1: J = (D<<LAMBDA_ACCURACY_BITS)+Lambda*R; 0: J = D + ((Lambda*R+Rounding)>>LAMBDA_ACCURACY_BITS)
#define DISABLE_ERC               1    //!< Disable any error concealment processes
#define JM_PARALLEL_DEBLOCK       0    //!< Enables Parallel Deblocking


#define MVC_EXTENSION_ENABLE      1    //!< enable support for the Multiview High Profile

#define EXT3D                     1

#if EXT3D
//   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   ++++++                     @DT: The macros defined for 3D extensions                                ++++++
//   ++++++  DO NOT MOVE THE POSITION IN THE FILE, UNLESS YOU ARE AWARE OF THE DEPENDENCY OF THE MACROS  ++++++
//   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#undef  EXT_VERSION
#define EXT_VERSION               "(3DV-ATM v14.0)"

#undef  MVC_EXTENSION_ENABLE
#define MVC_EXTENSION_ENABLE      0    //!< Must be set to 0 under EXT3D

#undef  IMGTYPE
#undef  ZEROSNR
#define IMGTYPE                   0    //!< Define imgpel size type. 0 implies byte (cannot handle >8 bit depths). Values tested: 0
#define DISABLE_ERC               1    //!< Disable any error concealment processes
#define FIX_SLICE_HEAD_PRED       1

// @DT: System setup parameters
#define MAX_DISPLAYS              32
#define MAX_CODEVIEW              16
#define MAX_VIEWREFERENCE         2
#define MAX_VALUE                 999999   //!< used for start value for some variables

// @DT: Image resampling algorithm
#define LANCZOS3                  0    //!< @DT: Two image upsampling methods
#define BI_LINEAR                 1

#define IMG_Y                     0
#define IMG_U                     1
#define IMG_V                     2

// @DT: Interlace control
#define ITRI_INTERLACE            1  //!< enable interlace support

// @DT: Normative parameter
#define BLOCK_VSP_T               8    //!< 1 - 1x1, 2 - 2x2, 4 - 4x4, 8 - 8x8
#define SMRC_SKIP_SERIES_MAX_LEN  16

#define IS_INTRA_DEC(MB)          ((MB)->mb_type==SI4MB || (MB)->mb_type==I4MB || (MB)->mb_type==I16MB || (MB)->mb_type==I8MB || (MB)->mb_type==IPCM)

#ifndef _MSC_VER
#define UNREFERENCED_PARAMETER(x)
#endif

#else
#ifndef _MSC_VER
#define UNREFERENCED_PARAMETER(x)
#endif
#endif

#if MVC_EXTENSION_ENABLE||EXT3D
#define MVC_INIT_VIEW_ID          -1
#define MAX_VIEW_NUM              1024   
#define BASE_VIEW_IDX             0
#define FREEPTR(ptr) { if(ptr) {free(ptr); (ptr)=NULL;} }
#endif

#include "typedefs.h"

#define SSE_MEMORY_ALIGNMENT      16

//#define MAX_NUM_SLICES 150
#define MAX_NUM_SLICES     50
#define MAX_REFERENCE_PICTURES 32               //!< H.264 allows 32 fields
#define MAX_CODED_FRAME_SIZE 8000000         //!< bytes for one frame
#define MAX_NUM_DECSLICES  16
#define MAX_DEC_THREADS    16                  //16 core deocoding;
#define MCBUF_LUMA_PAD_X        32
#define MCBUF_LUMA_PAD_Y        12
#define MCBUF_CHROMA_PAD_X      16
#define MCBUF_CHROMA_PAD_Y      8


//AVC Profile IDC definitions
typedef enum {
  FREXT_CAVLC444 = 44,       //!< YUV 4:4:4/14 "CAVLC 4:4:4"
  BASELINE       = 66,       //!< YUV 4:2:0/8  "Baseline"
  MAIN           = 77,       //!< YUV 4:2:0/8  "Main"
  EXTENDED       = 88,       //!< YUV 4:2:0/8  "Extended"
  FREXT_HP       = 100,      //!< YUV 4:2:0/8  "High"
  FREXT_Hi10P    = 110,      //!< YUV 4:2:0/10 "High 10"
  FREXT_Hi422    = 122,      //!< YUV 4:2:2/10 "High 4:2:2"
  FREXT_Hi444    = 244,      //!< YUV 4:4:4/14 "High 4:4:4"
  MVC_HIGH       = 118,      //!< YUV 4:2:0/8  "Multiview High"
  STEREO_HIGH    = 128,       //!< YUV 4:2:0/8  "Stereo High"
#if EXT3D
  ThreeDV_HIGH         = 138,
  ThreeDV_EXTEND_HIGH  = 139
#endif
} ProfileIDC;

#define FILE_NAME_SIZE  255
#define INPUT_TEXT_SIZE 1024

#if (ENABLE_HIGH444_CTX == 1)
# define NUM_BLOCK_TYPES 22  
#else
# define NUM_BLOCK_TYPES 10
#endif


//#define _LEAKYBUCKET_

#define BLOCK_SHIFT            2
#define BLOCK_SIZE             4
#define BLOCK_SIZE_8x8         8
#define SMB_BLOCK_SIZE         8
#define BLOCK_PIXELS          16
#define MB_BLOCK_SIZE         16
#define MB_PIXELS            256 // MB_BLOCK_SIZE * MB_BLOCK_SIZE
#define MB_PIXELS_SHIFT        8 // log2(MB_BLOCK_SIZE * MB_BLOCK_SIZE)
#define MB_BLOCK_SHIFT         4
#define BLOCK_MULTIPLE         4 // (MB_BLOCK_SIZE/BLOCK_SIZE)
#define MB_BLOCK_PARTITIONS   16 // (BLOCK_MULTIPLE * BLOCK_MULTIPLE)
#define BLOCK_CONTEXT         64 // (4 * MB_BLOCK_PARTITIONS)

// These variables relate to the subpel accuracy supported by the software (1/4)
#define BLOCK_SIZE_SP      16  // BLOCK_SIZE << 2
#define BLOCK_SIZE_8x8_SP  32  // BLOCK_SIZE8x8 << 2

//  Available MB modes
typedef enum {
  PSKIP        =  0,
  BSKIP_DIRECT =  0,
  P16x16       =  1,
  P16x8        =  2,
  P8x16        =  3,
  SMB8x8       =  4,
  SMB8x4       =  5,
  SMB4x8       =  6,
  SMB4x4       =  7,
  P8x8         =  8,
  I4MB         =  9,
  I16MB        = 10,
  IBLOCK       = 11,
  SI4MB        = 12,
  I8MB         = 13,
  IPCM         = 14,
  MAXMODE      = 15
} MBModeTypes;

// number of intra prediction modes
#define NO_INTRA_PMODE  9

// Direct Mode types
typedef enum {
  DIR_TEMPORAL = 0, //!< Temporal Direct Mode
  DIR_SPATIAL  = 1 //!< Spatial Direct Mode
} DirectModes;

// CAVLC block types
typedef enum {
  LUMA              =  0,
  LUMA_INTRA16x16DC =  1,
  LUMA_INTRA16x16AC =  2,
  CB                =  3,
  CB_INTRA16x16DC   =  4,
  CB_INTRA16x16AC   =  5,
  CR                =  8,
  CR_INTRA16x16DC   =  9,
  CR_INTRA16x16AC   = 10
} CAVLCBlockTypes;

// CABAC block types
typedef enum {
  LUMA_16DC     =   0,
  LUMA_16AC     =   1,
  LUMA_8x8      =   2,
  LUMA_8x4      =   3,
  LUMA_4x8      =   4,
  LUMA_4x4      =   5,
  CHROMA_DC     =   6,
  CHROMA_AC     =   7,
  CHROMA_DC_2x4 =   8,
  CHROMA_DC_4x4 =   9,
  CB_16DC       =  10,
  CB_16AC       =  11,
  CB_8x8        =  12,
  CB_8x4        =  13,
  CB_4x8        =  14,
  CB_4x4        =  15,
  CR_16DC       =  16,
  CR_16AC       =  17,
  CR_8x8        =  18,
  CR_8x4        =  19,
  CR_4x8        =  20,
  CR_4x4        =  21
} CABACBlockTypes;

// Macro defines
#define Q_BITS          15
#define DQ_BITS          6
#define Q_BITS_8        16
#define DQ_BITS_8        6 


#define IS_I16MB(MB)    ((MB)->mb_type==I16MB  || (MB)->mb_type==IPCM)
#define IS_DIRECT(MB)   ((MB)->mb_type==0     && (currSlice->slice_type == B_SLICE ))

#define TOTRUN_NUM       15
#define RUNBEFORE_NUM     7
#define RUNBEFORE_NUM_M1  6

// Quantization parameter range
#define MIN_QP          0
#define MAX_QP          51
// 4x4 intra prediction modes 
typedef enum {
  VERT_PRED            = 0,
  HOR_PRED             = 1,
  DC_PRED              = 2,
  DIAG_DOWN_LEFT_PRED  = 3,
  DIAG_DOWN_RIGHT_PRED = 4,
  VERT_RIGHT_PRED      = 5,
  HOR_DOWN_PRED        = 6,
  VERT_LEFT_PRED       = 7,
  HOR_UP_PRED          = 8
} I4x4PredModes;

// 16x16 intra prediction modes
typedef enum {
  VERT_PRED_16   = 0,
  HOR_PRED_16    = 1,
  DC_PRED_16     = 2,
  PLANE_16       = 3
} I16x16PredModes;

// 8x8 chroma intra prediction modes
typedef enum {
  DC_PRED_8     =  0,
  HOR_PRED_8    =  1,
  VERT_PRED_8   =  2,
  PLANE_8       =  3
} I8x8PredModes;

enum {
  EOS = 1,    //!< End Of Sequence
  SOP = 2,    //!< Start Of Picture
  SOS = 3,     //!< Start Of Slice
  SOS_CONT = 4
};

// MV Prediction types
typedef enum {
  MVPRED_MEDIAN   = 0,
  MVPRED_L        = 1,
  MVPRED_U        = 2,
  MVPRED_UR       = 3
} MVPredTypes;

enum {
  DECODING_OK     = 0,
  SEARCH_SYNC     = 1,
  PICTURE_DECODED = 2
};

#define  LAMBDA_ACCURACY_BITS         16
#define INVALIDINDEX  (-135792468)

#define RC_MAX_TEMPORAL_LEVELS   5

//Start code and Emulation Prevention need this to be defined in identical manner at encoder and decoder
#define ZEROBYTES_SHORTSTARTCODE 2 //indicates the number of zero bytes in the short start-code prefix

#define MAX_PLANE       3
#define IS_FREXT_PROFILE(profile_idc) ( profile_idc>=FREXT_HP || profile_idc == FREXT_CAVLC444 )
#define HI_INTRA_ONLY_PROFILE         (((p_Vid->active_sps->profile_idc>=FREXT_Hi10P)&&(p_Vid->active_sps->constrained_set3_flag))||(p_Vid->active_sps->profile_idc==FREXT_CAVLC444)) 

#endif
