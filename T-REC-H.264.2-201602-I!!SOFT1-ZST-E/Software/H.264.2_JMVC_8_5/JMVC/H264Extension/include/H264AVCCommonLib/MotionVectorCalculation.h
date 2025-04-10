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


#if !defined(AFX_MOTIONVECTORCALCULATION_H__9F13FBF7_8AD0_49DA_985B_08EE7CE0F231__INCLUDED_)
#define AFX_MOTIONVECTORCALCULATION_H__9F13FBF7_8AD0_49DA_985B_08EE7CE0F231__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN


class FrameMng;

class H264AVCCOMMONLIB_API MotionVectorCalculation
{
protected:
	MotionVectorCalculation();
	virtual ~MotionVectorCalculation();

public:
  ErrVal initSlice( const SliceHeader& rcSH );
  ErrVal uninit();

protected:
  Void xCalcSDirect( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase );
  Void xCalc16x16( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase );
  Void xCalc16x8( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase );
  Void xCalc8x16( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase );
  Void xCalc8x8( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase, Bool bFaultTolerant );
  Void xCalc8x8( B8x8Idx c8x8Idx, MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase, Bool bFaultTolerant );

protected:
  UInt m_uiMaxBw;
  Bool m_bSpatialDirectMode;
  Mv3D m_cMvA;
  Mv3D m_cMvB;
  Mv3D m_cMvC;
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_MOTIONVECTORCALCULATION_H__9F13FBF7_8AD0_49DA_985B_08EE7CE0F231__INCLUDED_)
