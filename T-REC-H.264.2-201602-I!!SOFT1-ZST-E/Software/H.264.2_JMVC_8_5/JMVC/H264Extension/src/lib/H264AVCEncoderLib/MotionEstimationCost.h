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


#if !defined(AFX_MOTIONESTIMATIONCOST_H__46CFF00D_F656_4EA9_B8F8_81CC5610D443__INCLUDED_)
#define AFX_MOTIONESTIMATIONCOST_H__46CFF00D_F656_4EA9_B8F8_81CC5610D443__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RateDistortionIf.h"


H264AVC_NAMESPACE_BEGIN


class MotionEstimationCost
{
protected:
	MotionEstimationCost();
	virtual ~MotionEstimationCost();

  ErrVal xUninit();

  Void xGetMotionCost( Bool bSad, Int iAdd )   { m_uiCost = m_pcRateDistortionIf->getMotionCostShift( bSad ) + iAdd;  }
  ErrVal xInitRateDistortionModel( Int iSubPelSearchLimit, RateDistortionIf* pcRateDistortionIf );

  UInt xGetVerCost( UInt y ) {  return (m_uiCost * m_puiVerCost[ y << m_iCostScale ] ) >> 16;  }
  UInt xGetHorCost( UInt x ) {  return (m_uiCost * m_puiHorCost[ x << m_iCostScale] ) >> 16;  }

  UInt xGetCost( UInt b )  { return ( m_uiCost * b ) >> 16; }

  UInt xGetCost( Mv& rcMv )
  {
    return ( m_uiCost * xGetBits( rcMv.getHor(), rcMv.getVer() ) ) >> 16;
  }
  UInt xGetCost( Int x, Int y )  {  return ( m_uiCost * xGetBits( x, y ) ) >> 16;  }
  UInt xGetBits( Int x, Int y )  {  return m_puiHorCost[ x << m_iCostScale] + m_puiVerCost[ y << m_iCostScale ];  }

  Void xSetPredictor( const Mv& rcMv )
  {
    m_puiHorCost = m_puiComponentCost - rcMv.getHor();
    m_puiVerCost = m_puiComponentCost - rcMv.getVer();
  }

  Void xSetCostScale( Int iCostScale ) { m_iCostScale = iCostScale; }
  UInt getComponentBits( Int iVal );

protected:
  RateDistortionIf* m_pcRateDistortionIf;
  UInt* m_puiComponentCostOriginP;
  UInt* m_puiComponentCost;
  UInt* m_puiVerCost;
  UInt* m_puiHorCost;
  UInt  m_uiCost;
  Int   m_iCostScale;
  Int m_iSearchLimit;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MOTIONESTIMATIONCOST_H__46CFF00D_F656_4EA9_B8F8_81CC5610D443__INCLUDED_)
