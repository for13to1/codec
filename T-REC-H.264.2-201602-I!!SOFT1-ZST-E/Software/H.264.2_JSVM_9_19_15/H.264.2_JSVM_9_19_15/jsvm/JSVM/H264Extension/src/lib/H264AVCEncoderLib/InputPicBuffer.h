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

#if !defined _INPUT_PIC_BUFFER_INCLUDED_
#define      _INPUT_PIC_BUFFER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

H264AVC_NAMESPACE_BEGIN


class InputAccessUnit
{
public:
  InputAccessUnit   ( UInt        uiContFrameNumber,
                      PicBuffer*  pcInputPicBuffer );
  ~InputAccessUnit  ();

  UInt        getContFrameNumber  ()  const   { return m_uiContFrameNumber; }
  PicBuffer*  getInputPicBuffer   ()          { return m_pcInputPicBuffer; }
private:
  UInt        m_uiContFrameNumber;
  PicBuffer*  m_pcInputPicBuffer;
};



class InputPicBuffer
{
public:
  InputPicBuffer            ();
  virtual ~InputPicBuffer   ();

  static ErrVal     create  ( InputPicBuffer*&  rpcInputPicBuffer );
  ErrVal            destroy ();

  ErrVal            init    ();
  ErrVal            uninit  ();

  Bool              empty   ();
  ErrVal            add     ( PicBuffer*        pcInputPicBuffer );
  InputAccessUnit*  remove  ( UInt              uiContFrameNumber );

private:
  Bool                        m_bInit;
  std::list<InputAccessUnit*> m_cInputUnitList;
  UInt                        m_uiContFrameNumber;
};

H264AVC_NAMESPACE_END

#endif  // _INPUT_PIC_BUFFER_INCLUDED_

