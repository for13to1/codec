/*
********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2009-2012, International Telecommunications Union, Geneva

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

#ifndef __H264AVCVIDEOLIB_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79
#define __H264AVCVIDEOLIB_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
//#define _ATL_APARTMENT_THREADED
#define _ATL_FREE_THREADED


#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>
#include <atlctl.h>


#include "MSysLib.h"
#include "CommonCodecLib.h"
#include "H264AVCEncoderLib.h"
#include "H264AVCDecoderLib.h"

#if defined( MSYS_WIN32 )
  #if defined( H264AVCVIDEOLIB_EXPORTS )
    #define H264AVCVIDEOLIB_API __declspec(dllexport)
  #else
    #if !defined( H264AVCVIDEOLIB_LIB )
      #define H264AVCVIDEOLIB_API __declspec(dllimport)
    #else
      #define H264AVCVIDEOLIB_API
    #endif
  #endif
#elif defined( MSYS_LINUX )
  #define H264AVCVIDEOLIB_API
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

//begin of WTL header file section
//remember to use namespace WTL
#include <atlapp.h>
#include <atlctrls.h>
#include <atlctrlx.h>
//end of WTL header file section


#include "MSysLib.h"
#include "MediaPacketLib.h"
#include "CommonCodecLib.h"

#include <string>
#include <sstream>


#endif //__H264AVCVIDEOLIB_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79
