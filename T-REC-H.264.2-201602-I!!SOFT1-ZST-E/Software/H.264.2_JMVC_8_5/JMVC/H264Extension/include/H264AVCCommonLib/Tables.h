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


#if !defined(AFX_TABLES_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_)
#define AFX_TABLES_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN


H264AVCCOMMONLIB_API extern const UChar g_aucFrameScan        [16];
H264AVCCOMMONLIB_API extern const UChar g_aucIndexChromaDCScan[4];
H264AVCCOMMONLIB_API extern const UChar g_aucLumaFrameDCScan  [16];
H264AVCCOMMONLIB_API extern const UChar g_aucFieldScan        [16];
H264AVCCOMMONLIB_API extern const UChar g_aucLumaFieldDCScan  [16];
H264AVCCOMMONLIB_API extern const Int   g_aaiQuantCoef     [6][16];
H264AVCCOMMONLIB_API extern const Int   g_aaiDequantCoef   [6][16];
H264AVCCOMMONLIB_API extern const UChar g_aucChromaScale      [52];


H264AVCCOMMONLIB_API extern const UChar g_aucFrameScan64      [64];
H264AVCCOMMONLIB_API extern const UChar g_aucFieldScan64      [64];
H264AVCCOMMONLIB_API extern const Int   g_aaiDequantCoef64 [6][64];
H264AVCCOMMONLIB_API extern const Int   g_aaiQuantCoef64   [6][64];

H264AVCCOMMONLIB_API extern const UChar g_aucScalingMatrixDefault4x4Intra[16];
H264AVCCOMMONLIB_API extern const UChar g_aucScalingMatrixDefault4x4Inter[16];
H264AVCCOMMONLIB_API extern const UChar g_aucScalingMatrixDefault8x8Intra[64];
H264AVCCOMMONLIB_API extern const UChar g_aucScalingMatrixDefault8x8Inter[64];


H264AVC_NAMESPACE_END


#endif //AFX_TABLES_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_

