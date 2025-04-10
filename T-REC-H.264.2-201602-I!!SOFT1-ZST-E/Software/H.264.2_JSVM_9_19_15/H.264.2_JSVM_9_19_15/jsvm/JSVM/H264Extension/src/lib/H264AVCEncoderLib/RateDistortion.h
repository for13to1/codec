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

#if !defined(AFX_RATEDISTORTION_H__C367E6C2_6E98_4DCB_9E6D_C4F84B9EC0D6__INCLUDED_)
#define AFX_RATEDISTORTION_H__C367E6C2_6E98_4DCB_9E6D_C4F84B9EC0D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RateDistortionIf.h"

H264AVC_NAMESPACE_BEGIN

class CodingParameter;

class RateDistortion :
public RateDistortionIf
{
protected:
  RateDistortion();
  virtual ~RateDistortion();

public:
  virtual ErrVal  setMbQpLambda( MbDataAccess& rcMbDataAccess, UInt uiQp, Double dLambda );

  static  ErrVal create( RateDistortion *&rpcRateDistortion );
  virtual ErrVal destroy();

  Double  getCost( UInt uiBits, UInt uiDistortion );
  Double  getFCost( UInt uiBits, UInt uiDistortion );
  UInt    getMotionCostShift( Bool bSad) { return (bSad) ? m_uiCostFactorMotionSAD : m_uiCostFactorMotionSSE; }

  ErrVal  fixMacroblockQP( MbDataAccess& rcMbDataAccess );

protected:
  Double m_dCost;
  Double m_dSqrtCost;
  UInt m_uiCostFactorMotionSAD;
  UInt m_uiCostFactorMotionSSE;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_RATEDISTORTION_H__C367E6C2_6E98_4DCB_9E6D_C4F84B9EC0D6__INCLUDED_)
