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


#include <cstdio>
#include <stdlib.h>
#include "H264AVCVideoIoLib.h"
#include "WriteYuvToFile.h"


WriteYuvToFile::WriteYuvToFile()
: m_uiWidth         ( 0 )
, m_uiHeight        ( 0 )
, m_bInitDone       ( false )
, m_bFileInitDone   (false)
{
	m_uiCrop[0]=0;
	m_uiCrop[1]=0;
	m_uiCrop[2]=0;
	m_uiCrop[3]=0;
}


WriteYuvToFile::~WriteYuvToFile()
{
}



ErrVal WriteYuvToFile::create( WriteYuvIf*& rpcWriteYuv, const std::string& rcFileName )
{
  ROT( 0 == rcFileName.size() );

  WriteYuvToFile* pcWriteYuvToFile;

  pcWriteYuvToFile = new WriteYuvToFile;

  rpcWriteYuv = pcWriteYuvToFile;

  ROT( NULL == pcWriteYuvToFile )

  RNOKS( pcWriteYuvToFile->xInit( rcFileName ) );

  return Err::m_nOK;
}


ErrVal WriteYuvToFile::xInit( const std::string& rcFileName )
{
  if( Err::m_nOK != m_cFile.open( rcFileName, LargeFile::OM_WRITEONLY ) )
  { 
    std::cerr << "failed to open YUV output file " << rcFileName.data() << std::endl;
    return Err::m_nERR;
  }

  return Err::m_nOK;
}


ErrVal WriteYuvToFile::destroy()
{
  m_cTempBuffer.deleteData();
  
  if( m_cFile.is_open() )
  {
    RNOK( m_cFile.close() );
  }

  delete this;
  return Err::m_nOK;
}

ErrVal WriteYuvToFile::setCrop(UInt *uiCrop)
{
	int i;
	for(i=0;i<4;i++)
		m_uiCrop[i]=*(uiCrop+i);
	return Err::m_nOK;
}

ErrVal WriteYuvToFile::writeFrame( const UChar *pLum,
                             const UChar *pCb,
                             const UChar *pCr,
                             UInt uiLumHeight,
                             UInt uiLumWidth,
                             UInt uiLumStride )
{
  m_uiWidth  = uiLumWidth;
  m_uiHeight = uiLumHeight;

  //frame crop
  pLum+=m_uiCrop[2]*uiLumStride;
  pLum+=m_uiCrop[0];
  pCb+=m_uiCrop[2]*uiLumStride/4;
  pCb+=m_uiCrop[0]/2;
  pCr+=m_uiCrop[2]*uiLumStride/4;
  pCr+=m_uiCrop[0]/2;
  uiLumHeight-=m_uiCrop[2]+m_uiCrop[3];
  uiLumWidth-=m_uiCrop[0]+m_uiCrop[1];

  RNOKS( xWriteFrame( pLum, pCb, pCr, uiLumHeight, uiLumWidth, uiLumStride ) );

  return Err::m_nOK;
}


ErrVal WriteYuvToFile::xWriteFrame( const UChar *pLum,
                              const UChar *pCb,
                              const UChar *pCr,
                              UInt uiHeight,
                              UInt uiWidth,
                              UInt uiStride )
{

  UInt    y;
  const UChar*  pucSrc;

  pucSrc = pLum;
  for( y = 0; y < uiHeight; y++ )
  {
    RNOK( m_cFile.write( pucSrc, uiWidth ) );
    pucSrc += uiStride;
  }

 
  uiStride >>= 1;
  uiHeight >>= 1;
  uiWidth  >>= 1;

  pucSrc = pCb;
  for( y = 0; y < uiHeight; y++ )
  {
    RNOK( m_cFile.write( pucSrc, uiWidth ) );
    pucSrc += uiStride;
  }


  pucSrc = pCr;
  for( y = 0; y < uiHeight; y++ )
  {
    RNOK( m_cFile.write( pucSrc, uiWidth ) );
    pucSrc += uiStride;
  }
  
  return Err::m_nOK;
}

ErrVal WriteYuvToFile::createMVC( WriteYuvIf*& rpcWriteYuv, const std::string& rcFileName, UInt numOfViews )
{
  ROT( 0 == rcFileName.size() );

  WriteYuvToFile* pcWriteYuvToFile;

  pcWriteYuvToFile = new WriteYuvToFile;

  rpcWriteYuv = pcWriteYuvToFile;

  ROT( NULL == pcWriteYuvToFile )

  return Err::m_nOK;
}


ErrVal WriteYuvToFile::xInitMVC( const std::string& rcFileName, UInt *vcOrder, 
                                 UInt uiNumOfViews )  // remove active view SEI
{

  std::string cFileName = rcFileName;
  std::string cTemp;
  char t[10];
  UInt view_id;

  m_cFileMVC = new LargeFile[uiNumOfViews];

  UInt pos = cFileName.rfind(".");
  cFileName.erase(pos);
  cTemp = cFileName.append("_");
  if (vcOrder == NULL) uiNumOfViews=0;
  for(UInt i = 0; i < uiNumOfViews; i++)
  {
    view_id = vcOrder ? vcOrder[i] : 0 ; // Dec. 1 fix 
	
    sprintf(t, "%d", view_id);
    cFileName = cTemp;
    cFileName.append(t);
    cFileName.append(".yuv");
    
    if( Err::m_nOK != m_cFileMVC[i].open( cFileName, LargeFile::OM_WRITEONLY ) )
    { 
      std::cerr << "failed to open YUV output file " << cFileName.data() << std::endl;
      return Err::m_nERR;
    }
  }
  
  m_bFileInitDone = true;

  return Err::m_nOK;
}

ErrVal WriteYuvToFile::destroyMVC(UInt uiNumOfViews)
{
  m_cTempBuffer.deleteData();
 
  for(UInt i = 0; i < uiNumOfViews; i++)
  { 
    if( m_cFileMVC[i].is_open() )
    {
      RNOK( m_cFileMVC[i].close() );
    }
  }
  delete m_cFileMVC;

  delete this;
  return Err::m_nOK;
}

ErrVal WriteYuvToFile::writeFrame( const UChar *pLum,
                                   const UChar *pCb,
                                   const UChar *pCr,
                                   UInt uiLumHeight,
                                   UInt uiLumWidth,
                                   UInt uiLumStride,
                                   UInt ViewCnt)
{
  m_uiWidth  = uiLumWidth;
  m_uiHeight = uiLumHeight;

  //frame crop
  pLum+=m_uiCrop[2]*uiLumStride;
  pLum+=m_uiCrop[0];
  pCb+=m_uiCrop[2]*uiLumStride/4;
  pCb+=m_uiCrop[0]/2;
  pCr+=m_uiCrop[2]*uiLumStride/4;
  pCr+=m_uiCrop[0]/2;
  uiLumHeight-=m_uiCrop[2]+m_uiCrop[3];
  uiLumWidth-=m_uiCrop[0]+m_uiCrop[1];

  RNOKS( xWriteFrame( pLum, pCb, pCr, uiLumHeight, uiLumWidth, uiLumStride, ViewCnt ) );

  return Err::m_nOK;
}


ErrVal WriteYuvToFile::xWriteFrame( const UChar *pLum,
                                    const UChar *pCb,
                                    const UChar *pCr,
                                    UInt uiHeight,
                                    UInt uiWidth,
                                    UInt uiStride, 
                                    UInt ViewCnt)
{

  UInt    y;
  const UChar*  pucSrc;

  pucSrc = pLum;
  for( y = 0; y < uiHeight; y++ )
  {
    RNOK( m_cFileMVC[ViewCnt].write( pucSrc, uiWidth ) );
    pucSrc += uiStride;
  }

 
  uiStride >>= 1;
  uiHeight >>= 1;
  uiWidth  >>= 1;

  pucSrc = pCb;
  for( y = 0; y < uiHeight; y++ )
  {
    RNOK( m_cFileMVC[ViewCnt].write( pucSrc, uiWidth ) );
    pucSrc += uiStride;
  }


  pucSrc = pCr;
  for( y = 0; y < uiHeight; y++ )
  {
    RNOK( m_cFileMVC[ViewCnt].write( pucSrc, uiWidth ) );
    pucSrc += uiStride;
  }
  
  return Err::m_nOK;
}

