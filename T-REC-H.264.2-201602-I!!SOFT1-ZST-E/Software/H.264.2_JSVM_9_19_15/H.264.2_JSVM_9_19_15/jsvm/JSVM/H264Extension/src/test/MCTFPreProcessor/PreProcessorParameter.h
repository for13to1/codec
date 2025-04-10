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

#ifndef _PRE_PROCESSOR_PARAMETER_H_
#define _PRE_PROCESSOR_PARAMETER_H_

#include "Typedefs.h"
#include "string.h"
#include "H264AVCCommonLib.h"



class PreProcessorParameter
{
public:
  PreProcessorParameter();
  ~PreProcessorParameter();

  static ErrVal create( PreProcessorParameter*& rpcPreProcessorParameter );
  ErrVal        destroy();
  ErrVal        init  ( Int argc, Char** argv );

  const std::string&  getInputFileName   () const { return m_cInputFileName;  }
  const std::string&  getOutputFileName  () const { return m_cOutputFileName;  }
  UInt                getFrameWidth      () const { return m_uiFrameWidth; }
  UInt                getFrameHeight     () const { return m_uiFrameHeight; }
  UInt                getNumFrames       () const { return m_uiNumFrames; }
  UInt                getGOPSize         () const { return m_uiGOPSize; }
  Double              getQP              () const { return m_dQP; }

protected:
  ErrVal              xPrintUsage        ( Char** argv );

private:
  std::string   m_cInputFileName;
  std::string   m_cOutputFileName;
  UInt          m_uiFrameWidth;
  UInt          m_uiFrameHeight;
  UInt          m_uiNumFrames;
  UInt          m_uiGOPSize;
  Double        m_dQP;
};


#endif //_PRE_PROCESSOR_PARAMETER_H_

