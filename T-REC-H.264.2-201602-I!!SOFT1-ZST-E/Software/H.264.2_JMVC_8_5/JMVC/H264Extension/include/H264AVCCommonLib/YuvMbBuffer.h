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


#if !defined(AFX_YUVMBBUFFER_H__F4C18313_4C6D_4412_96E3_BADC7F1AE13F__INCLUDED_)
#define AFX_YUVMBBUFFER_H__F4C18313_4C6D_4412_96E3_BADC7F1AE13F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN


class IntYuvMbBuffer;

class H264AVCCOMMONLIB_API YuvMbBuffer
{
public:
	YuvMbBuffer();
	virtual ~YuvMbBuffer();

  Pel* getYBlk( LumaIdx cIdx )
  {
    return &m_aucYuvBuffer[   MB_BUFFER_WIDTH + 4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<2)];
  }
  Pel* getLumBlk()       { return m_pPelCurr; }
  const Int getLStride()      const { return MB_BUFFER_WIDTH;}
  const Int getCStride()      const { return MB_BUFFER_WIDTH;}

  Pel* getUBlk( LumaIdx cIdx )
  {
    return &m_aucYuvBuffer[18*MB_BUFFER_WIDTH + 4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)];
  }
  Pel* getVBlk( LumaIdx cIdx )
  {
    return &m_aucYuvBuffer[18*MB_BUFFER_WIDTH + 16 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)];
  }
  Void set4x4Block( LumaIdx cIdx )
  {
    m_pPelCurr = &m_aucYuvBuffer[   MB_BUFFER_WIDTH + 4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<2)];
  }

  Pel* getMbLumAddr()   { return &m_aucYuvBuffer[   MB_BUFFER_WIDTH +  4]; }
  Pel* getMbCbAddr()    { return &m_aucYuvBuffer[18*MB_BUFFER_WIDTH +  4]; }
  Pel* getMbCrAddr()    { return &m_aucYuvBuffer[18*MB_BUFFER_WIDTH + 16];  }

  const Int getLWidth()       const { return 16; }
  const Int getLHeight()      const { return 16; }
  const Int getCWidth()       const { return 8; }
  const Int getCHeight()      const { return 8; }

  Void loadIntraPredictors( YuvPicBuffer* pcSrcBuffer );
  Void loadBuffer( IntYuvMbBuffer* pcSrcBuffer );
  Void loadBufferClip( IntYuvMbBuffer* pcSrcBuffer );
  Void  setZero();

protected:
  Pel* m_pPelCurr;
  Pel m_aucYuvBuffer[MB_BUFFER_WIDTH*26];
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_YUVMBBUFFER_H__F4C18313_4C6D_4412_96E3_BADC7F1AE13F__INCLUDED_)
