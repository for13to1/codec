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

#if !defined(AFX_LARGEFILE_H__7E94650C_CCB2_4248_AEF5_74966C842261__INCLUDED_)
#define AFX_LARGEFILE_H__7E94650C_CCB2_4248_AEF5_74966C842261__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined( MSYS_LINUX )
# if (!defined( MSYS_UNIX_LARGEFILE )) || (!defined(_LARGEFILE64_SOURCE) )
#  error Large file support requires MSYS_UNIX_LARGEFILE and _LARGEFILE64_SOURCE defined
# endif
#endif

class H264AVCVIDEOIOLIB_API LargeFile
{
public:
  enum OpenMode
  {
    OM_READONLY,
    OM_WRITEONLY,
    OM_APPEND,
    OM_READWRITE
  };

public:
  LargeFile();
  ~LargeFile();

  ErrVal open( const std::string& rcFilename, enum OpenMode eOpenMode, int iPermMode=0777 );

  bool is_open() const { return -1 != m_iFileHandle; }

  ErrVal close();
  ErrVal seek( Int64 iOffset, int iOrigin );
  Int64 tell();

  ErrVal read( Void *pvBuffer, UInt32 uiCount, UInt32& ruiBytesRead );
  ErrVal write( const Void *pvBuffer, UInt32 uiCount );

  Int getFileHandle() { return m_iFileHandle; }

private:
  Int m_iFileHandle;
};



#endif // !defined(AFX_LARGEFILE_H__7E94650C_CCB2_4248_AEF5_74966C842261__INCLUDED_)
