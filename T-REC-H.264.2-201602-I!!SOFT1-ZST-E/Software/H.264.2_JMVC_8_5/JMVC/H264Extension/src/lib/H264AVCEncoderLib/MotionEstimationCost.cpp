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


#include "H264AVCEncoderLib.h"

#include "MotionEstimationCost.h"


H264AVC_NAMESPACE_BEGIN


MotionEstimationCost::MotionEstimationCost():
  m_pcRateDistortionIf      ( NULL ),
  m_puiComponentCostOriginP ( NULL ),
  m_puiComponentCost        ( NULL ),
  m_puiVerCost              ( NULL ),
  m_puiHorCost              ( NULL ),
  m_uiCost                  ( 0 ),
  m_iCostScale              ( 0 ),
  m_iSearchLimit            ( 0xdeaddead )
{
}

MotionEstimationCost::~MotionEstimationCost()
{
}


ErrVal MotionEstimationCost::xUninit()
{

  if( NULL != m_puiComponentCostOriginP )
  {
    delete [] m_puiComponentCostOriginP;
    m_puiComponentCostOriginP = NULL;
  }


  return Err::m_nOK;
}

UInt MotionEstimationCost::getComponentBits( Int iVal )
{
  UInt uiLength = 1;
  UInt uiTemp = ( iVal <= 0) ? (-iVal<<1)+1: (iVal<<1);


  AOF_DBG( uiTemp );

  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  return uiLength;
}


ErrVal MotionEstimationCost::xInitRateDistortionModel( Int iSubPelSearchLimit, RateDistortionIf* pcRateDistortionIf )
{
  ROT( NULL == pcRateDistortionIf )
  m_pcRateDistortionIf = pcRateDistortionIf;

  // make it larger
  iSubPelSearchLimit += 4;
  iSubPelSearchLimit *= 8;

  if( m_iSearchLimit != iSubPelSearchLimit )
  {
    RNOK( xUninit() )

    m_iSearchLimit = iSubPelSearchLimit;

    m_puiComponentCostOriginP = new UInt[ 4 * iSubPelSearchLimit ];
    iSubPelSearchLimit *= 2;

    m_puiComponentCost = m_puiComponentCostOriginP + iSubPelSearchLimit;

    for( Int n = -iSubPelSearchLimit; n < iSubPelSearchLimit; n++)
    {
      m_puiComponentCost[n] = getComponentBits( n );
    }
  }

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
