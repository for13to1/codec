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


#if !defined(AFX_YUVBUFFER_H__9B273B48_A740_4E7E_8076_4FCCE69FBA98__INCLUDED_)
#define AFX_YUVBUFFER_H__9B273B48_A740_4E7E_8076_4FCCE69FBA98__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/YuvBufferCtrl.h"

H264AVC_NAMESPACE_BEGIN

class YuvMbBuffer;
class IntYuvMbBuffer;

class H264AVCCOMMONLIB_API YuvPicBuffer
{
public:
	YuvPicBuffer( YuvBufferCtrl& rcYuvBufferCtrl, PicType ePicType );
	virtual ~YuvPicBuffer();

  Pel* getBuffer()      { return m_pucYuvBuffer; }

  Pel* getLumBlk()      { return m_pPelCurr; }
  Pel* getYBlk( LumaIdx cIdx )
  { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getYBlk( cIdx ); }
  const Int getLStride()      const { return m_iStride; }
  Void set4x4Block( LumaIdx cIdx )
  {
    m_pPelCurr = m_pucYuvBuffer + m_rcBufferParam.getYBlk( cIdx );
  }
  Pel* getMbLumAddr()
  { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbLum(); }
  Pel* getMbLumAddrConst() const
  { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbLum(); }

  Pel* getUBlk( LumaIdx cIdx )
  { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getUBlk( cIdx ); }
  Pel* getVBlk( LumaIdx cIdx )
  { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getVBlk( cIdx ); }
  Pel* getMbCbAddr()
  { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbCb(); }
  Pel* getMbCrAddr()
  { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbCr(); }
  const Int getCStride()    const { return m_iStride>>1;}
  Pel* getMbCbAddrConst() const
  { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbCb(); }
  Pel* getMbCrAddrConst() const
  { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbCr(); }


  const Int getLWidth()     const { return m_rcBufferParam.getWidth(); }
  const Int getLHeight()    const { return m_rcBufferParam.getHeight(); }

  const Int getCWidth()     const { return m_rcBufferParam.getWidth()>>1; }
  const Int getCHeight()    const { return m_rcBufferParam.getHeight()>>1; }

  const Int getLXMargin()   const { return m_rcYuvBufferCtrl.getXMargin(); }
  const Int getLYMargin()   const { return m_rcYuvBufferCtrl.getYMargin(); }
  const Int getCXMargin()   const { return m_rcYuvBufferCtrl.getXMargin()>>1; }
  const Int getCYMargin()   const { return m_rcYuvBufferCtrl.getYMargin()>>1; }

  PicType getPicType() const { return m_ePicType; }

  ErrVal loadBuffer( YuvPicBuffer *pcSrcYuvPicBuffer ); //TMM_EC
  ErrVal loadBuffer( YuvMbBuffer *pcYuvMbBuffer );
  ErrVal loadBuffer( IntYuvMbBuffer *pcYuvMbBuffer );
  ErrVal fillMargin();
  ErrVal loadBufferAndFillMargin( YuvPicBuffer *pcSrcYuvPicBuffer );

  ErrVal init( Pel*& rpucYuvBuffer );
  ErrVal uninit();

  Bool isValid()        { return NULL != m_pucYuvBuffer; }

  Pel* getLumOrigin()      const { return m_pucYuvBuffer + m_rcYuvBufferCtrl.getLumOrigin( m_ePicType ); }
  Pel* getCbOrigin()       const { return m_pucYuvBuffer + m_rcYuvBufferCtrl.getCbOrigin ( m_ePicType ); }
  Pel* getCrOrigin()       const { return m_pucYuvBuffer + m_rcYuvBufferCtrl.getCrOrigin ( m_ePicType ); }

  ErrVal copy( YuvPicBuffer* pcPicBuffer ); // HS: decoder robustness
protected:
    Void xFillPlaneMargin( Pel *pucDest, Int iHeight, Int iWidth, Int iStride, Int iXMargin, Int iYMargin );
    Void xCopyFillPlaneMargin( Pel *pucSrc, Pel *pucDest, Int iHeight, Int iWidth, Int iStride, Int iXMargin, Int iYMargin );
    Void xDump( FILE* hFile, Pel* pPel, Int iHeight, Int iWidth, Int iStride );

protected:
    const YuvBufferCtrl::YuvBufferParameter& m_rcBufferParam;
    //TMM_EC {{
public:
    YuvBufferCtrl& m_rcYuvBufferCtrl;
protected:	
    //TMM_EC }}
    Int  m_iStride;
    Pel* m_pPelCurr;
    Pel* m_pucYuvBuffer;
    Pel* m_pucOwnYuvBuffer;
    PicType m_ePicType;
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_YUVBUFFER_H__9B273B48_A740_4E7E_8076_4FCCE69FBA98__INCLUDED_)
