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

#if !defined(AFX_CONTROLMNGIF_H__BB1C0D17_55C1_4629_B010_C22270D71001__INCLUDED_)
#define AFX_CONTROLMNGIF_H__BB1C0D17_55C1_4629_B010_C22270D71001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

H264AVC_NAMESPACE_BEGIN

class MbDataCtrl;


class ControlMngIf
{
protected:
  ControlMngIf() {};
  virtual ~ControlMngIf() {};

public:
  virtual ErrVal initSlice0           (SliceHeader*                 rcSH )    = 0;
  virtual ErrVal initSPS              ( SequenceParameterSet&       rcSPS,
                                        UInt                        uiLayer ) = 0;

  virtual ErrVal initParameterSets    ( const SequenceParameterSet& rcSPS,
                                        const PictureParameterSet&  rcPPS ) = 0;

  virtual ErrVal initSliceForCoding   ( const SliceHeader&  rcSH )        = 0;
  virtual ErrVal initSliceForReading  ( const SliceHeader&  rcSH )        = 0;
  virtual ErrVal initSliceForDecoding ( const SliceHeader&  rcSH )        = 0;
  virtual ErrVal initSliceForFiltering( const SliceHeader&  rcSH )        = 0;
  virtual ErrVal initSliceForWeighting( const SliceHeader&  rcSH )        = 0;
  virtual ErrVal finishSlice          ( const SliceHeader&  rcSH,
                                        Bool&               rbPicDone,
                                        Bool&               rbFrameDone ) = 0;

  virtual ErrVal initMbForParsing     ( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex )                      = 0;
  virtual ErrVal initMbForDecoding    ( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff ) = 0;
  virtual ErrVal initMbForFiltering   ( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff ) = 0;

  virtual ErrVal initMbForCoding      ( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff, Bool bFieldFlag )  = 0;
  virtual ErrVal initMbForDecoding    ( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff )                   = 0;
  virtual ErrVal initMbForFiltering   ( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff )                   = 0;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_CONTROLMNGIF_H__BB1C0D17_55C1_4629_B010_C22270D71001__INCLUDED_)
