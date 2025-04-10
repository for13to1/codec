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

#include "H264AVCEncoderLib.h"
#include "CodingParameter.h"
#include "math.h"
#include "RateDistortion.h"

H264AVC_NAMESPACE_BEGIN


RateDistortion::RateDistortion():
  m_uiCostFactorMotionSAD( 0 ),
  m_uiCostFactorMotionSSE( 0 )
{
}

RateDistortion::~RateDistortion()
{
}

ErrVal RateDistortion::create( RateDistortion *&rpcRateDistortion )
{
  rpcRateDistortion = new RateDistortion;

  ROT( NULL == rpcRateDistortion );

  return Err::m_nOK;
}

Double RateDistortion::getCost( UInt uiBits, UInt uiDistortion )
{
  Double d = ((Double)uiDistortion + (Double)(Int)uiBits * m_dCost+.5);
  return (Double)(UInt)floor(d);
}

Double RateDistortion::getFCost( UInt uiBits, UInt uiDistortion )
{
  Double d = (((Double)uiDistortion) + ((Double)uiBits * m_dCost));
  return d;
}


ErrVal RateDistortion::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal RateDistortion::fixMacroblockQP( MbDataAccess& rcMbDataAccess )
{
  if( !( rcMbDataAccess.getMbData().getMbCbp() || rcMbDataAccess.getMbData().isIntra16x16() ) ) // has no coded texture
  {
    rcMbDataAccess.getMbData().setQp( rcMbDataAccess.getLastQp() );
  }
  return Err::m_nOK;
}


ErrVal
RateDistortion::setMbQpLambda( MbDataAccess& rcMbDataAccess, UInt uiQp, Double dLambda )
{
  rcMbDataAccess.getMbData().setQp( uiQp );

  m_dCost                 = dLambda;
  m_dSqrtCost             = sqrt( dLambda );
  m_uiCostFactorMotionSAD = (UInt)floor(65536.0 * sqrt( m_dCost ));
  m_uiCostFactorMotionSSE = (UInt)floor(65536.0 *       m_dCost  );

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
