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


#if !defined(AFX_READYUVFILE_H__F07173C5_E390_46AD_AACE_7A529A0DCE33__INCLUDED_)
#define AFX_READYUVFILE_H__F07173C5_E390_46AD_AACE_7A529A0DCE33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LargeFile.h"

class ReadYuvFile 
{

public:
enum FillMode
{
  FILL_CLEAR = 0,
  FILL_FRAME,    
  FILL_FIELD    
};

protected:
	ReadYuvFile();
	virtual ~ReadYuvFile();

public:
  static ErrVal create( ReadYuvFile*& rpcReadYuvFile );
  ErrVal init( const std::string& rcFileName, UInt uiLumPicHeight, UInt uiLumPicWidth, UInt uiStartLine = 0, UInt uiEndLine = MSYS_UINT_MAX, FillMode eFillMode = FILL_CLEAR );
  ErrVal uninit();
  ErrVal destroy();

  ErrVal readFrame(  UChar *pLum,
                     UChar *pCb,
                     UChar *pCr,
                     UInt uiBufHeight,
                     UInt uiBufWidth,
                     UInt uiBufStride );

  void GoToFrame(const int frameNumber);


protected:
  ErrVal xReadPlane( UChar *pucDest, UInt uiBufHeight, UInt uiBufWidth, UInt uiBufStride, UInt uiPicHeight, UInt uiPicWidth, UInt uiStartLine, UInt uiEndLine );

protected:
  LargeFile m_cFile;
  UInt m_uiLumPicHeight;
  UInt m_uiLumPicWidth;
  UInt m_uiStartLine;
  UInt m_uiEndLine;
  FillMode m_eFillMode;
};

#endif // !defined(AFX_READYUVFILE_H__F07173C5_E390_46AD_AACE_7A529A0DCE33__INCLUDED_)
