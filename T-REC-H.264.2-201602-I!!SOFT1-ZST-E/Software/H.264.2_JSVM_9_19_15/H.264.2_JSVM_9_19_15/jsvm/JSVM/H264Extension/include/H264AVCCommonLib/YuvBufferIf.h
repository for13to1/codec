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

#if !defined(AFX_YUVBUFFERRWIF_H__E94AE8DA_A40D_4411_8226_A471358FDCDD__INCLUDED_)
#define AFX_YUVBUFFERRWIF_H__E94AE8DA_A40D_4411_8226_A471358FDCDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

H264AVC_NAMESPACE_BEGIN

class H264AVCCOMMONLIB_API YuvBufferIf
{
protected:
  YuvBufferIf() {}
  virtual ~YuvBufferIf() {}
public:
  virtual const Int getCStride()   const  = 0;
  virtual Pel* getLumBlk()   = 0;
  virtual Pel* getYBlk( LumaIdx cIdx ) = 0;
  virtual Pel* getUBlk( LumaIdx cIdx ) = 0;
  virtual Pel* getVBlk( LumaIdx cIdx ) = 0;
  virtual const Int getLStride()    const = 0;

  virtual Pel* getMbLumAddr() = 0;
  virtual Pel* getMbCbAddr()  = 0;
  virtual Pel* getMbCrAddr()  = 0;

  virtual Void set4x4Block( LumaIdx cIdx ) = 0;

  virtual const Int getLWidth()     const = 0;
  virtual const Int getLHeight()    const = 0;
  virtual const Int getLXMargin()   const = 0;
  virtual const Int getLYMargin()   const = 0;
  virtual const Int getCWidth()     const = 0;
  virtual const Int getCHeight()    const = 0;
};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_YUVBUFFERRWIF_H__E94AE8DA_A40D_4411_8226_A471358FDCDD__INCLUDED_)
