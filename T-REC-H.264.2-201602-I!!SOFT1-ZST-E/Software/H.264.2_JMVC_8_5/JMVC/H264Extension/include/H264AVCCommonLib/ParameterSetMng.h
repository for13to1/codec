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


#if !defined(AFX_PARAMETERSETMNG_H__5F7FDE27_D5CF_4594_A15B_EE376A1650D7__INCLUDED_)
#define AFX_PARAMETERSETMNG_H__5F7FDE27_D5CF_4594_A15B_EE376A1650D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/SequenceParameterSet.h"

#include "list"

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


H264AVC_NAMESPACE_BEGIN

class H264AVCCOMMONLIB_API ParameterSetMng
{
protected:
  ParameterSetMng();
  virtual ~ParameterSetMng() {}

public:
  static ErrVal create( ParameterSetMng*& rpcParameterSetMng );
  ErrVal destroy();
  ErrVal init()                                        { return Err::m_nOK; }
  ErrVal uninit();

  ErrVal setParamterSetList( std::list<SequenceParameterSet*>& rcSPSList, std::list<PictureParameterSet*>& rcPPSList) const;
  Bool   isValidSPS( UInt uiSPSId )                    { return m_cSPSBuf.isValidOffset(uiSPSId) && NULL != m_cSPSBuf.get( uiSPSId); }
  ErrVal get( SequenceParameterSet *& rpcSPS, UInt uiSPSId);
  ErrVal store( SequenceParameterSet* pcSPS );
  ErrVal get( PictureParameterSet *& rpcPPS, UInt uiPPSId );
  Bool   isValidPPS( UInt uiPPSId )                    { return m_cPPSBuf.isValidOffset(uiPPSId) && NULL != m_cPPSBuf.get( uiPPSId); }
  ErrVal store( PictureParameterSet* pcPPS );

private:
  ErrVal xDestroyPPS(UInt uiPPSId);
  ErrVal xDestroySPS(UInt uiSPSId);

private:
  StatBuf<SequenceParameterSet*,32>  m_cSPSBuf;
  StatBuf<PictureParameterSet*,256> m_cPPSBuf;
  UInt m_uiActiveSPSId;
  UInt m_uiActivePPSId;
  std::list<SequenceParameterSet*>  m_cSPSList;
  std::list<PictureParameterSet*>   m_cPPSList;
};



H264AVC_NAMESPACE_END


#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

#endif // !defined(AFX_PARAMETERSETMNG_H__5F7FDE27_D5CF_4594_A15B_EE376A1650D7__INCLUDED_)
