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

#if !defined(AFX_CABACCONTEXTMODEL_H__2D76E021_2277_44AD_95CC_EE831C7AC09F__INCLUDED_)
#define AFX_CABACCONTEXTMODEL_H__2D76E021_2277_44AD_95CC_EE831C7AC09F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN


class H264AVCCOMMONLIB_API CabacContextModel
{
public:
  CabacContextModel();
  ~CabacContextModel();

  const UChar getState()           { return m_ucState>>1; }
  const UChar getMps()             { return m_ucState&1;  }
  Void toggleMps()                 { m_ucState ^= 1;      }
  Void setState( UChar ucState )   { m_ucState = (ucState<<1)+getMps(); }

  Void init( Short asCtxInit[], Int iQp )
  {
    Int iState = ( ( asCtxInit[0] * iQp ) >> 4 ) + asCtxInit[1];
    iState = gMin (gMax ( 1, iState), 126 );

    if (iState>=64)
    {
      m_ucState = iState - 64;
      m_ucState += m_ucState + 1;
    }
    else
    {
      m_ucState = 63 - iState;
      m_ucState += m_ucState;
    }
    m_uiCount = 0;
  }

  Void initEqualProbability()
  {
    m_ucState = 0;
    m_uiCount = 0;
  }

  Void  incrementCount()  { m_uiCount++; }
	//JVT-X046 {
	UChar getucState(void)
	{
		return m_ucState;
	}
	UInt getuiCount(void)
	{
		return m_uiCount;
	}
	void set(CabacContextModel* pcCContextModel)
  {
		m_ucState = pcCContextModel->getucState();
		m_uiCount = pcCContextModel->getuiCount();
  }
  //JVT-X046 }
private:
  UChar m_ucState;
  UInt  m_uiCount;

  static  const Double m_afProbability[128];
  static  const Double m_afEntropy    [128];
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_CABACCONTEXTMODEL_H__2D76E021_2277_44AD_95CC_EE831C7AC09F__INCLUDED_)
