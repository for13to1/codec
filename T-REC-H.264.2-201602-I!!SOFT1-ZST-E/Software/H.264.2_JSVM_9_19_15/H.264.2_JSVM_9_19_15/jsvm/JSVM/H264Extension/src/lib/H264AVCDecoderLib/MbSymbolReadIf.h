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

#if !defined(AFX_MBSYMBOLREADIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_)
#define AFX_MBSYMBOLREADIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

H264AVC_NAMESPACE_BEGIN


class MbSymbolReadIf
{
protected:
  MbSymbolReadIf() {}
	virtual ~MbSymbolReadIf() {}

public:
  virtual Bool    isMbSkipped ( MbDataAccess& rcMbDataAccess, UInt& uiNextSkippedVLC ) = 0;
  virtual Bool    isBLSkipped ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual Bool    isEndOfSlice()                               = 0;
  virtual ErrVal  blockModes  ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  mbMode      ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  resPredFlag ( MbDataAccess& rcMbDataAccess ) = 0;

  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx ) = 0;

  virtual ErrVal  cbp( MbDataAccess& rcMbDataAccess, UInt uiStart, UInt uiStop ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;

  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;

  virtual ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, LumaIdx   cIdx, ResidualMode eResidualMode, UInt& ruiMbExtCbp, UInt uiStart = 0, UInt uiStop = 16 ) = 0;
  virtual ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode, UInt uiStart = 0, UInt uiStop = 16 ) = 0;

  virtual ErrVal  deltaQp             ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  intraPredModeLuma   ( MbDataAccess& rcMbDataAccess, LumaIdx cIdx ) = 0;
  virtual ErrVal  intraPredModeChroma ( MbDataAccess& rcMbDataAccess ) = 0;
	virtual ErrVal  fieldFlag           ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  samplesPCM          ( MbDataAccess& rcMbDataAccess ) = 0;

  virtual ErrVal  startSlice          ( const SliceHeader& rcSliceHeader ) = 0;
  virtual ErrVal  finishSlice         ( ) = 0;

  virtual ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess) = 0;
  virtual ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx, UInt uiStart = 0, UInt uiStop = 16 ) = 0;
  virtual ErrVal  intraPredModeLuma8x8( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx ) = 0;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBSYMBOLREADIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_)
