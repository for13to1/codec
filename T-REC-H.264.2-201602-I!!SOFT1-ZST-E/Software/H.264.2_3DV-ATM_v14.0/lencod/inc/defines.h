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

#if defined _DEBUG
# define TRACE           0      //!< 0:Trace off 1:Trace on 2:detailed CABAC context information
#else
# define TRACE           0      //!< 0:Trace off 1:Trace on 2:detailed CABAC context information
#endif

#define JM                  "17 (FRExt)"
#define VERSION             "17.2"
#define EXT_VERSION         "(FRExt)"

#define GET_METIME                1    //!< Enables or disables ME computation time
#define DUMP_DPB                  0    //!< Dump DPB info for debug purposes
#define PRINTREFLIST              0    //!< Print ref list info for debug purposes
#define IMGTYPE                   1    //!< Define imgpel size type. 0 implies byte (cannot handle >8 bit depths) and 1 implies unsigned short
#define ENABLE_FIELD_CTX          1    //!< Enables field context types for CABAC. If disabled, results in speedup for progressive content.
#define ENABLE_HIGH444_CTX        1    //!< Enables High 444 context types for CABAC. If disabled, results in speedup of non High444 profile encodings.
#define DEBUG_BITDEPTH            0    //!< Ensures that > 8 bit content have no values that would result in out of range results
#define ALLOW_GRAYSCALE           1    //!< Allow encoding in grayscale
#define ZEROSNR                   1    //!< PSNR computation method
#define USE_RND_COST              0    //!< Perform ME RD decision using a rounding estimate of the motion cost
#define JM_INT_DIVIDE             1
#define JM_MEM_DISTORTION         0
#define JCOST_CALC_SCALEUP        1    //!< 1: J = (D<<LAMBDA_ACCURACY_BITS)+Lambda*R; 0: J = D + ((Lambda*R+Rounding)>>LAMBDA_ACCURACY_BITS)
#define INTRA_RDCOSTCALC_ET       1    //!< Early termination 
#define INTRA_RDCOSTCALC_NNZ      1    //1: to recover block's nzn after rdcost calculation;
#define JCOST_OVERFLOWCHECK       0    //!<1: to check the J cost if it is overflow>
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
#define MVC_EXTENSION_ENABLE       0     //!< Must be set to 0 under EXT3D

#undef  IMGTYPE
#undef  ZEROSNR
#define IMGTYPE                    0    //!< Define imgpel size type. 0 implies byte (cannot handle >8 bit depths). Values tested: 0
#define ZEROSNR                    0    //!< PSNR computation method

// @DT: System setup parameters
#define MAX_DISPLAYS               32
#define MAX_CODEVIEW               16    //!<the max number of coding view
#define MAX_CODEVIEWPLUSDEPTH      32    //!<the max number of coding view plus depth
#define MAX_SYN_VIEWS              64
#define MAX_VIEWS                  64
#define MAX_VIEWREFERENCE          2

// @DT: Image resampling algorithm
#define LANCZOS3                   0
#define BI_LINEAR                  1

#define IMG_Y                      0
#define IMG_U                      1
#define IMG_V                      2

// @DT: Interlace control
#define ITRI_INTERLACE             1  //!< enable interlace support

// @DT: Normative parameter
#define BLOCK_VSP_T                8    //!< 1 - 1x1, 2 - 2x2, 4 - 4x4, 8 - 8x8
#define SMRC_SKIP_SERIES_MAX_LEN   16
#define FIX_SLICE_HEAD_PRED        1

// @DT: Encoder only macros
#define MR_METRIC_WITH_DC_ADD      1 //!< enables addition of sum of abs differences of DC to MR_SAD4x4
#define USE_MR_METRIC_FOR_ALC      1 //!< enables using Mean-removed metric for ALC inter-view prediction
#define FULL_SEARCH_FOR_ALC        1 //!< enables using Full Search for displacement vector estimation
#define MAX_TEMPORAL_LEVELS        6

#ifndef _MSC_VER
#define UNREFERENCED_PARAMETER(x)
#endif

#else
#ifndef _MSC_VER
#define UNREFERENCED_PARAMETER(x)
#endif
#endif

#define EOS_OUTPUT                0 

#define EPZSREF                   1

#define MAX_RC_MODE               3
#define RC_MAX_TEMPORAL_LEVELS    5

#define SSE_MEMORY_ALIGNMENT      16

//#define BEST_NZ_COEFF 1   // yuwen 2005.11.03 => for high complexity mode decision (CAVLC, #TotalCoeff)

//AVC Profile IDC definitions
enum {
  FREXT_CAVLC444 = 44,       //!< YUV 4:4:4/14 "CAVLC 4:4:4"
  BASELINE       = 66,       //!< YUV 4:2:0/8  "Baseline"
  MAIN           = 77,       //!< YUV 4:2:0/8  "Main"
  EXTENDED       = 88,       //!< YUV 4:2:0/8  "Extended"
  FREXT_HP       = 100,      //!< YUV 4:2:0/8  "High"
  FREXT_Hi10P    = 110,      //!< YUV 4:2:0/10 "High 10"
  FREXT_Hi422    = 122,      //!< YUV 4:2:2/10 "High 4:2:2"
  FREXT_Hi444    = 244,      //!< YUV 4:4:4/14 "High 4:4:4"
  MULTIVIEW_HIGH = 118,      //!< YUV 4:2:0/8  "Multiview High"
  STEREO_HIGH    = 128,      //!< YUV 4:2:0/8  "Stereo High"
#if EXT3D
  ThreeDV_HIGH   = 138,
  ThreeDV_EXTEND_HIGH = 139
#endif
} ProfileIDC;

// Some typedefs used in the software
#include "types.h"

#define FILE_NAME_SIZE  255
#define INPUT_TEXT_SIZE 1024

#define CAVLC_LEVEL_LIMIT 2063

#if (ENABLE_HIGH444_CTX == 1)
# define NUM_BLOCK_TYPES 22  
#else
# define NUM_BLOCK_TYPES 10
#endif

#if !EXT3D
#define _LEAKYBUCKET_
#endif

// ---------------------------------------------------------------------------------
// FLAGS and DEFINES for new chroma intra prediction, Dzung Hoang
// Threshold values to zero out quantized transform coefficients.
// Recommend that _CHROMA_COEFF_COST_ be low to improve chroma quality
#define _LUMA_COEFF_COST_       4 //!< threshold for luma coeffs
#define _CHROMA_COEFF_COST_     4 //!< threshold for chroma coeffs, used to be 7
#define _LUMA_MB_COEFF_COST_    5 //!< threshold for luma coeffs of inter Macroblocks
#define _LUMA_8x8_COEFF_COST_   5 //!< threshold for luma coeffs of 8x8 Inter Partition

//#define IMG_PAD_SIZE           20 //!< Number of pixels padded around the reference frame (>=4)
//#define IMG_PAD_SIZE_TIMES4    80 //!< Number of pixels padded around the reference frame in subpel units(>=16)
#define IMG_PAD_SIZE_X         32 //!< Number of pixels padded around the reference frame (>=4)
#define IMG_PAD_SIZE_Y         18  //!< Number of pixels padded around the reference frame (>=4)
#define IMG_PAD_SIZE_X_TIMES4  128 //!< Number of pixels padded around the reference frame in subpel units(>=16)
#define IMG_PAD_SIZE_Y_TIMES4  72 //!< Number of pixels padded around the reference frame in subpel units(>=16)

#define MAX_VALUE       999999   //!< used for start value for some variables
#define INVALIDINDEX  (-135792468)

#define DUMMY   14
#define ET_SIZE 300      //!< size of error text buffer

#define  LAMBDA_ACCURACY_BITS         5
#define  LAMBDA_FACTOR(lambda)        ((int)((double)(1 << LAMBDA_ACCURACY_BITS) * lambda + 0.5))

#if (IMGTYPE == 0)
#define DISTBLK_MAX  INT_MAX
#elif (IMGTYPE == 2)
#define DISTBLK_MAX  FLT_MAX
#else
#define DISTBLK_MAX  ((distblk) INT_MAX << LAMBDA_ACCURACY_BITS)
#endif

#define  MAXSLICEPERPICTURE           100
#define  MAX_REFERENCE_PICTURES        32

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

// wavelet based weighted PSNR wavelet levels
#define NUM_WAVELET_LEVEL 4

// RDOQ
#define MAX_PREC_COEFF    25



//  Available MB modes
enum {
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

#if EXT3D
  PVSP_SKIP    = 15,
  BVSP_DIRECT  = 15,
  MAXMODE      = 16
#else
  MAXMODE      = 15
#endif

} MBModeTypes;

// number of intra prediction modes
#define NO_INTRA_PMODE  9

// Direct Mode types
enum {
  DIR_TEMPORAL = 0, //!< Temporal Direct Mode
  DIR_SPATIAL  = 1 //!< Spatial Direct Mode
} DirectModes;

// CAVLC block types
enum {
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
enum {
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

#define IS_INTRA(MB)    ((MB)->mb_type==SI4MB || (MB)->mb_type==I4MB || (MB)->mb_type==I16MB || (MB)->mb_type==I8MB || (MB)->mb_type==IPCM)

#define LEVEL_NUM         6
#define TOTRUN_NUM       15
#define RUNBEFORE_NUM     7
#define RUNBEFORE_NUM_M1  6

// Quantization parameter range
#define MIN_QP          0
#define MAX_QP          51
#define SHIFT_QP        12

// 4x4 intra prediction modes 
enum {
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
enum {
  VERT_PRED_16   = 0,
  HOR_PRED_16    = 1,
  DC_PRED_16     = 2,
  PLANE_16       = 3
} I16x16PredModes;

// 8x8 chroma intra prediction modes
enum {
  DC_PRED_8     =  0,
  HOR_PRED_8    =  1,
  VERT_PRED_8   =  2,
  PLANE_8       =  3
} I8x8PredModes;

#define INIT_FRAME_RATE 30
enum {
  EOS = 1,    //!< End Of Sequence
  SOP = 2,    //!< Start Of Picture
  SOS = 3     //!< Start Of Slice
};

// MV Prediction types
enum {
  MVPRED_MEDIAN   = 0,
  MVPRED_L        = 1,
  MVPRED_U        = 2,
  MVPRED_UR       = 3
} MVPredTypes;

#define MAX_SYMBOLS_PER_MB  1200  //!< Maximum number of different syntax elements for one MB
                                  // CAVLC needs more symbols per MB


#define MAX_PART_NR     3 /*!< Maximum number of different data partitions.
                               Some reasonable number which should reflect
                               what is currently defined in the SE2Partition map (elements.h) */

//Start code and Emulation Prevention need this to be defined in identical manner at encoder and decoder
#define ZEROBYTES_SHORTSTARTCODE 2 //indicates the number of zero bytes in the short start-code prefix

#define Q_BITS          15
#define DQ_BITS         6

#define Q_BITS_8        16
#define DQ_BITS_8       6

// Context Adaptive Lagrange Multiplier (CALM)
#define CALM_MF_FACTOR_THRESHOLD 512.0

#define MAX_PLANE       3
#define IS_FREXT_PROFILE(profile_idc) ( profile_idc>=FREXT_HP || profile_idc == FREXT_CAVLC444 )

#define MAXSLICEGROUPIDS 8


#define NUM_MB_TYPE_CTX  11
#define NUM_B8_TYPE_CTX  9
#define NUM_MV_RES_CTX   10
#define NUM_REF_NO_CTX   6
#define NUM_DELTA_QP_CTX 4
#define NUM_MB_AFF_CTX   4

#define NUM_TRANSFORM_SIZE_CTX 3

#define NUM_IPR_CTX    2
#define NUM_CIPR_CTX   4
#define NUM_CBP_CTX    4
#define NUM_BCBP_CTX   4
#define NUM_MAP_CTX   15
#define NUM_LAST_CTX  15
#define NUM_ONE_CTX    5
#define NUM_ABS_CTX    5

#endif
