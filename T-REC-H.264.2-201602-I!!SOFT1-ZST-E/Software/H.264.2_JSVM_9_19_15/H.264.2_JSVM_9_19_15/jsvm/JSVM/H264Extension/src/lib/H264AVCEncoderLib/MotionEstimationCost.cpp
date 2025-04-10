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
#include "MotionEstimationCost.h"


H264AVC_NAMESPACE_BEGIN


MotionEstimationCost::MotionEstimationCost()
: m_pcRateDistortionIf    (  0 )
, m_puiComponentCostAlloc (  0 )
, m_puiComponentCost      (  0 )
, m_puiHorCost            (  0 )
, m_puiVerCost            (  0 )
, m_uiMvScaleShift        (  0 )
, m_uiCostFactor          (  0 )
, m_iSubPelSearchLimit    ( -1 )
{
}

MotionEstimationCost::~MotionEstimationCost()
{
  ANOK( xUninit() );
}

ErrVal
MotionEstimationCost::xInit( const Int iSubPelSearchLimit, RateDistortionIf* pcRateDistortionIf )
{
  ROF( iSubPelSearchLimit >= 0 );
  ROF( pcRateDistortionIf );
  m_pcRateDistortionIf = pcRateDistortionIf;
  if( m_iSubPelSearchLimit != iSubPelSearchLimit )
  {
    RNOK( xUninit() );
    m_iSubPelSearchLimit    = iSubPelSearchLimit;
    Int iNumPositionsDiv2   = ( iSubPelSearchLimit + 4 )  << 4;
    m_puiComponentCostAlloc = new UInt[ iNumPositionsDiv2 << 1 ];
    m_puiComponentCost      = m_puiComponentCostAlloc + iNumPositionsDiv2;
    for( Int iPos = -iNumPositionsDiv2; iPos < iNumPositionsDiv2; iPos++ )
    {
      m_puiComponentCost[ iPos ] = xGetComponentBits( iPos );
    }
  }
  return Err::m_nOK;
}

ErrVal 
MotionEstimationCost::xUninit()
{
  if( m_puiComponentCostAlloc )
  {
    delete [] m_puiComponentCostAlloc;
    m_puiComponentCostAlloc = 0;
  }
  return Err::m_nOK;
}

ErrVal
MotionEstimationCost::xSetMEPars( const UInt uiMvScaleShift, const Bool bSad )
{
  ROF( m_pcRateDistortionIf );
  m_uiMvScaleShift  = uiMvScaleShift;
  m_uiCostFactor    = m_pcRateDistortionIf->getMotionCostShift( bSad );
  return Err::m_nOK;
}

ErrVal
MotionEstimationCost::xSetPredictor( const Mv& rcMv )
{
  m_puiHorCost = m_puiComponentCost - rcMv.getHor();
  m_puiVerCost = m_puiComponentCost - rcMv.getVer();
  return Err::m_nOK;
}

UInt 
MotionEstimationCost::xGetComponentBits( Int iPos ) const
{
  UInt   uiCodeLength = 1;
  UInt   uiAbs        = UInt( iPos < 0 ? -iPos : iPos );
  UInt   uiTempVal    = ( uiAbs << 1 ) + 1;
  while( uiTempVal != 1 )
  {
    uiTempVal   >>= 1;
    uiCodeLength += 2;
  }
  return uiCodeLength;
}

H264AVC_NAMESPACE_END
