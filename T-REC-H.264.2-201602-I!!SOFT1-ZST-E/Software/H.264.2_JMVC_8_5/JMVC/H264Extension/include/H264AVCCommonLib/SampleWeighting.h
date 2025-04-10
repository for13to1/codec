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


#if !defined(AFX_SAMPLEWEIGHTING_H__6B0A73D2_EAF3_497F_B114_913D68E1C1C0__INCLUDED_)
#define AFX_SAMPLEWEIGHTING_H__6B0A73D2_EAF3_497F_B114_913D68E1C1C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include  "H264AVCCommonLib/YuvMbBuffer.h"
#include  "H264AVCCommonLib/IntYuvMbBuffer.h"

H264AVC_NAMESPACE_BEGIN

class H264AVCCOMMONLIB_API SampleWeighting 
{
  typedef Void (*MixSampleFunc) ( Pel* pucDest,  Int iDestStride, Pel*  pucSrc, Int iSrcStride, Int iSizeY );
  typedef Void (*XMixSampleFunc)( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY );

protected:
  SampleWeighting();
  virtual ~SampleWeighting()  {}

public:
  static ErrVal create( SampleWeighting*& rpcSampleWeighting );
  ErrVal destroy();
  virtual ErrVal init();
  ErrVal uninit();

  ErrVal initSlice( const SliceHeader& rcSliceHeader );

  Void getTargetBuffers( YuvMbBuffer*    apcTarBuffer[2], YuvMbBuffer*    pcRecBuffer, const PW* pcPW0, const PW* pcPW1 );
  Void getTargetBuffers( IntYuvMbBuffer* apcTarBuffer[2], IntYuvMbBuffer* pcRecBuffer, const PW* pcPW0, const PW* pcPW1 );

  Void weightLumaSamples(   YuvMbBuffer*    pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 );
  Void weightChromaSamples( YuvMbBuffer*    pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 );

  Void weightLumaSamples(   IntYuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 );
  Void weightChromaSamples( IntYuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 );

  //===== for motion estimation of bi-predicted blocks with standard weights =====
  Void inverseLumaSamples  ( IntYuvMbBuffer* pcDesBuffer, IntYuvMbBuffer* pcOrgBuffer, IntYuvMbBuffer* pcFixBuffer, Int iYSize, Int iXSize );

  //===== for motion estimation of bi-predicted blocks with non-standard weights =====
  Void weightInverseLumaSamples  ( IntYuvMbBuffer* pcDesBuffer, IntYuvMbBuffer* pcOrgBuffer, IntYuvMbBuffer* pcFixBuffer, const PW* pcSearchPW, const PW* pcFixPW, Double&  rdWeight, Int iYSize, Int iXSize );

  //===== for motion estimation of unidirectional predicted blocks with non-standard weights =====
  Void weightInverseLumaSamples  ( IntYuvMbBuffer* pcDesBuffer, IntYuvMbBuffer* pcOrgBuffer, const PW* pcPW, Double&  rdWeight, Int iYSize, Int iXSize );
  Void weightInverseChromaSamples( IntYuvMbBuffer* pcDesBuffer, IntYuvMbBuffer* pcOrgBuffer, const PW* pcPW, Double* padWeight, Int iYSize, Int iXSize );

//TMM_WP
  ErrVal initSliceForWeighting( const SliceHeader& rcSliceHeader);

//TMM_WP
  
protected:
  Void xMixB      ( Pel*  pucDest, Int iDestStride, Pel*  pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX );
  Void xMixB      ( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX );
  Void xMixBWeight( Pel*  pucDest, Int iDestStride, Pel*  pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX, Int iWD, Int iWS, Int iOffset, UInt uiDenom );
  Void xMixBWeight( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX, Int iWD, Int iWS, Int iOffset, UInt uiDenom );
  Void xWeight    ( Pel*  pucDest, Int iDestStride,                               Int iSizeY, Int iSizeX, Int iWeight,      Int iOffset, UInt uiDenom );
  Void xWeight    ( XPel* pucDest, Int iDestStride,                               Int iSizeY, Int iSizeX, Int iWeight,      Int iOffset, UInt uiDenom );

private:
  static Void xMixB16x( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY );
  static Void xMixB8x ( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY );
  static Void xMixB4x ( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY );
  static Void xXMixB16x( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY );
  static Void xXMixB8x ( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY );
  static Void xXMixB4x ( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY );

protected:
  MixSampleFunc m_afpMixSampleFunc[5];
  XMixSampleFunc m_afpXMixSampleFunc[5];

private:
  YuvMbBuffer     m_cYuvBiBuffer;
  IntYuvMbBuffer  m_cIntYuvBiBuffer;
  UInt            m_uiLumaLogWeightDenom;
  UInt            m_uiChromaLogWeightDenom;
  Bool            m_bExplicit;
  Bool            m_bWeightedPredDisableP;
  Bool            m_bWeightedPredDisableB;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_SAMPLEWEIGHTING_H__6B0A73D2_EAF3_497F_B114_913D68E1C1C0__INCLUDED_)
