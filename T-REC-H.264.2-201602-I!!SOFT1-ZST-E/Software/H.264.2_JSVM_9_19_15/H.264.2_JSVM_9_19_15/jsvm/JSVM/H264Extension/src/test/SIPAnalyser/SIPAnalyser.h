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

#if !defined(AFX_SIPANALYSER_H__A7CC9210_E601_446B_854A_49856C2A7A5E__INCLUDED_)
#define AFX_SIPANALYSER_H__A7CC9210_E601_446B_854A_49856C2A7A5E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SIPParameters.h"


class SIPAnalyser
{
protected:
  SIPAnalyser();
  ~SIPAnalyser();

public:
  ErrVal          go();
  ErrVal          init(SIPParameters* pcSIPParameters);
  ErrVal          destroy();
  static ErrVal   create( SIPAnalyser*& rpcSIPAnalyser);

protected:
  ErrVal          xPrintLayer(int iLayer);
  ErrVal          xDumpLayer(int iLayer);
  ErrVal          xProcessLayer(int iLayer);
  ErrVal          xProcessKnapsack(int iNumber,int* piWeight,int* piPrice,int iBagCubage,int* piDecision);
  ErrVal          xUninitData();
  ErrVal          xReadData();
  ErrVal          xInitData();
  SIPParameters*  m_pcSIPParameters;
  int***          m_aaaiFrameBits;//m_aaauiFrameBits[layer][POC][0/1 with/without interlayer prediction]
  int**           m_aaiSIPDecision;
  int*            m_aiTotalBits;
};

#endif // !defined(AFX_SIPANALYSER_H__A7CC9210_E601_446B_854A_49856C2A7A5E__INCLUDED_)
