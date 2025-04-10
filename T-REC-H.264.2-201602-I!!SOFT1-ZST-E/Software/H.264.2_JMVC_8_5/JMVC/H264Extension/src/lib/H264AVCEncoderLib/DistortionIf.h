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


#if !defined(AFX_DISTORTIONIF_H__DD590742_32BB_4FC6_AB30_A2F60CD21A42__INCLUDED_)
#define AFX_DISTORTIONIF_H__DD590742_32BB_4FC6_AB30_A2F60CD21A42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN


class IntYuvMbBuffer;
class IntYuvPicBuffer;
class XDistortion;
class XDistSearchStruct;

typedef UInt (*XDistortionFunc)( XDistSearchStruct* );


class XDistSearchStruct
{

public:
  XPel*           pYOrg;
  XPel*           pYFix;
  XPel*           pYSearch;
  Int             iYStride;
  XPel*           pUOrg;
  XPel*           pVOrg;
  XPel*           pUFix;
  XPel*           pVFix;
  XPel*           pUSearch;
  XPel*           pVSearch;
  Int             iCStride;
  Int             iRows;
  XDistortionFunc Func;

};

class XDistortionIf
{
protected:
	XDistortionIf         () {}
	virtual ~XDistortionIf() {}

public:
  virtual Void    loadOrgMbPelData( IntYuvPicBuffer* pcOrgYuvBuffer, IntYuvMbBuffer*& rpcOrgMbBuffer ) = 0;

  virtual UInt    get8x8Cb        ( XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD ) = 0;
  virtual UInt    get8x8Cr        ( XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD ) = 0;
  virtual UInt    getLum16x16     ( XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD ) = 0;
  virtual UInt    getLum8x8       ( XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD ) = 0;
  virtual UInt    getLum4x4       ( XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD ) = 0;
//TMM_WP
  ErrVal getLumaWeight( IntYuvPicBuffer* pcOrgPicBuffer, IntYuvPicBuffer* pcRefPicBuffer, Double& rfWeight, UInt uiLumaLog2WeightDenom );
  ErrVal getChromaWeight( IntYuvPicBuffer* pcOrgPicBuffer, IntYuvPicBuffer* pcRefPicBuffer, Double& rfWeight, UInt uiChromaLog2WeightDenom, Bool bCb );

  ErrVal getLumaOffsets( IntYuvPicBuffer* pcOrgPicBuffer, IntYuvPicBuffer* pcRefPicBuffer, Double& rfOffset );
  ErrVal getChromaOffsets( IntYuvPicBuffer* pcOrgPicBuffer, IntYuvPicBuffer* pcRefPicBuffer, Double& rfOffset, Bool bCb );
//TMM_WP
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_DISTORTIONIF_H__DD590742_32BB_4FC6_AB30_A2F60CD21A42__INCLUDED_)
