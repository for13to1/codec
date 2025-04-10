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

#if !defined(AFX_MOTIONESTIMATIONQUARTERPEL_H__3CDE908E_D756_4E9A_8B86_C55174766C93__INCLUDED_)
#define AFX_MOTIONESTIMATIONQUARTERPEL_H__3CDE908E_D756_4E9A_8B86_C55174766C93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MotionEstimation.h"
#define X1 24


H264AVC_NAMESPACE_BEGIN

class MotionEstimationQuarterPel :
public MotionEstimation
{
protected:
  MotionEstimationQuarterPel();
  virtual ~MotionEstimationQuarterPel();

public:
  static ErrVal create( MotionEstimation*& rpcMotionEstimation );

  ErrVal compensateBlock( YuvMbBuffer *pcRecPelData, UInt uiBlk, UInt uiMode, YuvMbBuffer *pcRefPelData2 = NULL );

  Void xSubPelSearch( YuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, UInt uiBlk, UInt uiMode );

protected:
  Void xCompensateBlocksHalf( XPel *pPelDes, YuvPicBuffer *pcRefPelData, Mv cMv, UInt uiMode, UInt uiYSize, UInt uiXSize );
  Void xGetSizeFromMode( UInt& ruiXSize, UInt& ruiYSize, UInt uiMode );
  Void xInitBuffer();

protected:
  UInt m_uiBestMode;
  XPel m_aXHPelSearch[17*X1*4];
  XPel m_aXQPelSearch[16*16*9];
  XPel *m_apXHPelSearch[9];
  XPel *m_apXQPelSearch[9];
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MOTIONESTIMATIONQUARTERPEL_H__3CDE908E_D756_4E9A_8B86_C55174766C93__INCLUDED_)
