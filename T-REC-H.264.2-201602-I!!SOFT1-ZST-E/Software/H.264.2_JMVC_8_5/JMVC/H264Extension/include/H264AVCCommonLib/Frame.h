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


#if !defined(AFX_FRAME_H__F0945458_9AA5_4D1A_9A9E_BFAAA9C416EF__INCLUDED_)
#define AFX_FRAME_H__F0945458_9AA5_4D1A_9A9E_BFAAA9C416EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/YuvPicBuffer.h"


H264AVC_NAMESPACE_BEGIN


class FrameUnit;
class QuarterPelFilter;


class H264AVCCOMMONLIB_API Frame
{
public:
  class PocOrder
  {
  public:
      __inline Int operator() ( const Frame* pcFrame1, const Frame* pcFrame2 )
      {
          return pcFrame1->getPOC() < pcFrame2->getPOC();
      }
  };

  Frame( YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& rcYuvHalfPelBufferCtrl, PicType ePicType );
  virtual ~Frame();
  ErrVal init( Pel* pucYuvBuffer, FrameUnit* pcFrameUnit );
  ErrVal uninit();
  ErrVal extendFrame( QuarterPelFilter* pcQuarterPelFilter, Bool bFrameMbsOnly, Bool bFGS );

  FrameUnit*       getFrameUnit()         { return m_pcFrameUnit; }
  const FrameUnit* getFrameUnit()   const { return m_pcFrameUnit; }
  YuvPicBuffer* getFullPelYuvBuffer()     { return &m_cFullPelYuvBuffer; }
  YuvPicBuffer* getHalfPelYuvBuffer()     { return &m_cHalfPelYuvBuffer; }

  Bool  isPOCAvailable()            const { return m_bPOCisSet; }
  Int   getPOC        ()            const { return m_iPOC; }
  Void  setPOC        ( Int iPOC )        { m_iPOC = iPOC; m_bPOCisSet = true; }

  PicType getPicType()              const { return m_ePicType; }
  Bool isShortTerm()                const;
  //Int   getPOC        (PicType ePicType)  const {return ePicType==FRAME?m_iPOC:(ePicType==TOP_FIELD?m_cTopField.setPOC( iPoc ):m_cBotField.setPOC( iPoc ));}

  const Int stamp()                 const { return m_iStamp; }
  Int& stamp()                            { return m_iStamp; }

  const Bool isUsed()               const;

  UInt  getViewId     ()            const { return m_uiViewId; }
  Void  setViewId     ( UInt uiViewId )   { m_uiViewId = uiViewId; }
	
	Bool	getInterViewFlag ()					const { return m_uiInterviewflag;}  //JVT-W056
	Void	setInterViewFlag (Bool InterViewFlag) {m_uiInterviewflag = InterViewFlag;}//JVT-W056

protected:
  YuvPicBuffer  m_cFullPelYuvBuffer;
  YuvPicBuffer  m_cHalfPelYuvBuffer;
  FrameUnit*    m_pcFrameUnit;
  Bool          m_bPOCisSet;
  Int           m_iPOC;
  Int           m_iStamp;
  UInt          m_uiViewId;
	Bool					m_uiInterviewflag; //JVT-W056
    const PicType m_ePicType;
};



class RefPic
{
public:
  RefPic( const Frame* pcFrame = NULL, Int iStamp = 0 ):m_pcFrame(pcFrame), m_iStamp(iStamp){}
  const Bool operator==( const RefPic& rcRefPic ) const { return ((m_pcFrame == rcRefPic.m_pcFrame) && (m_iStamp == rcRefPic.m_iStamp)); }
  const Bool operator!=( const RefPic& rcRefPic ) const { return ((m_pcFrame != rcRefPic.m_pcFrame) || (m_iStamp != rcRefPic.m_iStamp)); }
  const Int  getStamp()                           const { return m_iStamp; }
  const Frame* getFrame()                         const { /*AOF_DBG(isAvailable());*/ return m_pcFrame; }
  const Frame& getPic()                           const { AOF_DBG(isAvailable()); return *m_pcFrame; }
  const Bool isAvailable()                        const { return ((m_pcFrame != NULL) && (m_iStamp == m_pcFrame->stamp())); }
  const Bool isAssigned()                         const { return (m_pcFrame != NULL); }
  Void setFrame( const Frame* pcFrame)                  { m_pcFrame = pcFrame;   m_iStamp  = ( pcFrame == 0 ? 0 : m_pcFrame->stamp() ); }

private:
  const Frame* m_pcFrame;
  Int          m_iStamp;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_FRAME_H__F0945458_9AA5_4D1A_9A9E_BFAAA9C416EF__INCLUDED_)
