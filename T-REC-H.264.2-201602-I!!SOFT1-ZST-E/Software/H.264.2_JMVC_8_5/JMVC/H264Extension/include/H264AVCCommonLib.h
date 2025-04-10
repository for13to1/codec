/*
********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005-2009, International Telecommunications Union, Geneva

The Fraunhofer HHI hereby donate this source code to the ITU, with the following
understanding:
    1. Fraunhofer HHI retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. Fraunhofer HHI retain full patent rights (if any exist) in the technical
       content of techniques and algorithms herein.
    3. The ITU shall make this code available to anyone, free of license or
       royalty fees.

DISCLAIMER OF WARRANTY

These software programs are available to the user without any license fee or
royalty on an "as is" basis. The ITU disclaims any and all warranties, whether
express, implied, or statutory, including any implied warranties of
merchantability or of fitness for a particular purpose. In no event shall the
contributor or the ITU be liable for any incidental, punitive, or consequential
damages of any kind whatsoever arising from the use of these programs.

This disclaimer of warranty extends to the user of these programs and user's
customers, employees, agents, transferees, successors, and assigns.

The ITU does not represent or warrant that the programs furnished hereunder are
free of infringement of any third-party patents. Commercial implementations of
ITU-T Recommendations, including shareware, may be subject to royalty fees to
patent holders. Information regarding the ITU-T patent policy is available from 
the ITU Web site at http://www.itu.int.

THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.

********************************************************************************
*/


#ifndef __H264AVCCOMMONLIB_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79
#define __H264AVCCOMMONLIB_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79

#include "H264AVCCommonIf.h"


#if defined( MSYS_WIN32 )
  #if defined( H264AVCCOMMONLIB_EXPORTS )
    #define H264AVCCOMMONLIB_API __declspec(dllexport)
  #else
    #if !defined( H264AVCCOMMONLIB_LIB )
      #define H264AVCCOMMONLIB_API __declspec(dllimport)
    #else
      #define H264AVCCOMMONLIB_API
    #endif
  #endif
#elif defined( MSYS_LINUX )
  #define H264AVCCOMMONLIB_API
#endif

H264AVCCOMMONLIB_API extern UInt g_nSymbolCounter[8];
H264AVCCOMMONLIB_API extern UInt g_nLayer;



#define H264AVC_NAMESPACE_BEGIN      namespace h264 {
#define H264AVC_NAMESPACE_END        }



#include "H264AVCCommonLib/Macros.h"
#include "H264AVCCommonLib/GlobalFunctions.h"

#include "H264AVCCommonLib/CommonDefs.h"


#if defined( USE_NAMESPACE_H264AVC )
  using namespace h264;
#endif

#include "H264AVCCommonLib/CommonBuffers.h"
#include "H264AVCCommonLib/CommonTypes.h"
#include "H264AVCCommonLib/MbDataAccess.h"

#if defined( USE_NAMESPACE_H264AVC )
  using namespace h264;
#endif


#define H264AVCCommonLib_FIRST_VER      (0)
#define H264AVCCommonLib_SECOND_VER     (1)
#define H264AVCCommonLib_THIRD_VER      (0)
#define H264AVCCommonLib_FOURTH_VER     (0)


#endif //__H264AVCCOMMONLIB_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79
