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

#include "H264AVCVideoIoLib.h"
#include "WriteYuvToFile.h"


WriteYuvToFile::WriteYuvToFile()
{
}

WriteYuvToFile::~WriteYuvToFile()
{
}

ErrVal
WriteYuvToFile::create( WriteYuvToFile*& rpcWriteYuv )
{
  rpcWriteYuv = new WriteYuvToFile;
  ROF( rpcWriteYuv )
  return Err::m_nOK;
}

ErrVal
WriteYuvToFile::destroy()
{
  ROT( m_cFile.is_open() );
  delete this;
  return Err::m_nOK;
}

ErrVal
WriteYuvToFile::init( const std::string& rcFileName )
{
  ROF( rcFileName.size() );
#if 1  // DOLBY_ENCMUX_ENABLE
  if( rcFileName.compare("none") && rcFileName.compare("\"\"") )
#endif // DOLBY_ENCMUX_ENABLE
  {
    if( Err::m_nOK != m_cFile.open( rcFileName, LargeFile::OM_WRITEONLY ) )
    {
      std::cerr << "failed to open YUV output file " << rcFileName.data() << std::endl;
      return Err::m_nERR;
    }
  }
  return Err::m_nOK;
}

ErrVal
WriteYuvToFile::uninit()
{
  if( m_cFile.is_open() )
  {
    RNOK( m_cFile.close() );
  }
  return Err::m_nOK;
}

ErrVal
WriteYuvToFile::writeFrame( const UChar* pLum,
                            const UChar* pCb,
                            const UChar* pCr,
                            UInt         uiHeight,
                            UInt         uiWidth,
                            UInt         uiStride,
                            const UInt   rauiCropping[] )
{
  ROFRS( m_cFile.is_open(), Err::m_nOK );

  UInt          y;
  const UChar*  pucSrc;

  uiWidth   -= rauiCropping[0] + rauiCropping[1];
  uiHeight  -= rauiCropping[2] + rauiCropping[3];

  pucSrc = pLum + ( rauiCropping[0] + rauiCropping[2] * uiStride );
  for( y = 0; y < uiHeight; y++ )
  {
    RNOK( m_cFile.write( pucSrc, uiWidth ) );
    pucSrc += uiStride;
  }

  uiStride >>= 1;
  uiHeight >>= 1;
  uiWidth  >>= 1;

  pucSrc = pCb + ( ( rauiCropping[0] + rauiCropping[2] * uiStride ) >> 1 );
  for( y = 0; y < uiHeight; y++ )
  {
    RNOK( m_cFile.write( pucSrc, uiWidth ) );
    pucSrc += uiStride;
  }

  pucSrc = pCr + ( ( rauiCropping[0] + rauiCropping[2] * uiStride ) >> 1 );
  for( y = 0; y < uiHeight; y++ )
  {
    RNOK( m_cFile.write( pucSrc, uiWidth ) );
    pucSrc += uiStride;
  }

  return Err::m_nOK;
}

