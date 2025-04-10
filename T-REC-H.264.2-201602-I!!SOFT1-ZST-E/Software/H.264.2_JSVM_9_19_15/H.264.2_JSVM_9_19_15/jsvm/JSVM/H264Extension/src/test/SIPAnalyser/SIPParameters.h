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

#if !defined(AFX_SIPPARAMETERS_H__61034ED7_54DC_4213_A281_A965AEA62520__INCLUDED_)
#define AFX_SIPPARAMETERS_H__61034ED7_54DC_4213_A281_A965AEA62520__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib.h"



typedef struct LayerParameters
{
  UInt              m_uiFps;
  float             m_fTolerableRatio;
  std::string       m_strInputFileWithInterPred;
  std::string       m_strInputFileWithoutInterPred;
  std::string       m_strOutputFile;
} * PLAYERPARAMETERS;

class SIPParameters
{
protected:
  SIPParameters();
  ~SIPParameters();

  ErrVal            xReadConfigFile(FILE* pFile);
  void              xPrintUsage();
  ErrVal            xReadLine( FILE* pFile, const char* pcFormat, void* pPar );
  ErrVal            xCheck();
public:

  ErrVal            destroy();
  static ErrVal     create( SIPParameters*& rpcSIPParameters);
  ErrVal            init  ( Int argc, Char** argv );
  PLAYERPARAMETERS  getLayerParameter(UInt uiLayer);
  UInt              getFrameNum(){return m_uiFrameNum;}
  UInt              getLayerNum(){return m_uiLayerNum;}
  UInt              getInFps(){return m_uiInFps;}

protected:
  LayerParameters*  m_pcLayerParameters;
  UInt              m_uiFrameNum;
  UInt              m_uiLayerNum;
  UInt              m_uiInFps;

};

#endif // !defined(AFX_SIPPARAMETERS_H__61034ED7_54DC_4213_A281_A965AEA62520__INCLUDED_)
