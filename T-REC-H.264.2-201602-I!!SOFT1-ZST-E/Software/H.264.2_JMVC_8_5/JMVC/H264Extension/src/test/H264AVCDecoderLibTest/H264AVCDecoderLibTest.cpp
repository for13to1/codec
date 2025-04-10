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
#include "H264AVCDecoderLibTest.h"
#include "H264AVCDecoderTest.h"


int
main( int argc, char** argv)
{
  H264AVCDecoderTest* pcH264AVCDecoderTest = NULL;
  DecoderParameter    cParameter;

  printf("JMVC %s Decoder\n\n\n",_JMVC_VERSION_);

  RNOKRS( cParameter.init( argc, argv ), -2 );
//TMM_EC {{
	UInt	nCount	=	1;

  WriteYuvIf*                 pcWriteYuv;

  RNOKS( WriteYuvToFile::createMVC( pcWriteYuv, cParameter.cYuvFile, cParameter.uiNumOfViews ) );

  ReadBitstreamFile *pcReadBitstreamFile;
  RNOKS( ReadBitstreamFile::create( pcReadBitstreamFile ) ); 
  RNOKS( pcReadBitstreamFile->init( cParameter.cBitstreamFile ) );  
//TMM_EC }}
	for( UInt n = 0; n < nCount; n++ )
  {
    RNOKR( H264AVCDecoderTest::create   ( pcH264AVCDecoderTest ), -3 );
    RNOKR( pcH264AVCDecoderTest->init   ( &cParameter, (WriteYuvToFile*)pcWriteYuv, pcReadBitstreamFile ),          -4 );
    RNOKR( pcH264AVCDecoderTest->go     (),                       -5 );
    RNOKR( pcH264AVCDecoderTest->destroy(),                       -6 );
  }
//TMM_EC {{
	if( NULL != pcWriteYuv )              
  {
    RNOK( pcWriteYuv->destroy() );  
  }

  if( NULL != pcReadBitstreamFile )     
  {
    RNOK( pcReadBitstreamFile->uninit() );  
    RNOK( pcReadBitstreamFile->destroy() );  
  }
//TMM_EC }}
  return 0;
}
