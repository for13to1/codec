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

#if !defined(AFX_WRITEBITSTREAMTOFILE_H__B008532B_0DD3_488A_85D9_F68395BF2E26__INCLUDED_)
#define AFX_WRITEBITSTREAMTOFILE_H__B008532B_0DD3_488A_85D9_F68395BF2E26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#if defined( MSYS_WIN32 )
#pragma warning( disable: 4275 )
#endif

#if defined( MSYS_WIN32 )
#pragma warning( disable: 4250 )
#endif



#include "LargeFile.h"
#include "WriteBitstreamIf.h"

class H264AVCVIDEOIOLIB_API WriteBitstreamToFile :
public WriteBitstreamIf
{
protected:
  WriteBitstreamToFile() : m_uiNumber(0), m_bNewFileOnNewAu(false) {}
  virtual ~WriteBitstreamToFile() {}

public:
  static ErrVal create( WriteBitstreamToFile*& rpcWriteBitstreamToFile );
  virtual ErrVal destroy();
  ErrVal init( const std::string& rcFileName, Bool bNewFileOnNewAu = false );
  virtual ErrVal uninit();

  virtual ErrVal writePacket( BinDataAccessor* pcBinDataAccessor, Bool bNewAU = false );
  virtual ErrVal writePacket( BinData* pcBinData, Bool bNewAU = false );

  virtual ErrVal writePacket( Void* pBuffer, UInt uiLength );

private:
  UInt m_uiNumber;
  LargeFile m_cFile;
  std::string m_cFileName;
  Bool m_bNewFileOnNewAu;
};

#if defined( MSYS_WIN32 )
#pragma warning( default: 4275 )
#endif

#if defined( MSYS_WIN32 )
#pragma warning( default: 4250 )
#endif


#endif // !defined(AFX_WRITEBITSTREAMTOFILE_H__B008532B_0DD3_488A_85D9_F68395BF2E26__INCLUDED_)
