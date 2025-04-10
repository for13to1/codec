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

#include "H264AVCDecoderLibTest.h"
#include "H264AVCDecoderTest.h"


int
main( int argc, char** argv)
{
#ifdef SHARP_AVC_REWRITE_OUTPUT
  printf( "JSVM %s AVC REWRITER\n\n\n", _JSVM_VERSION_ );
#else
  printf( "JSVM %s Decoder\n\n\n",      _JSVM_VERSION_ );
#endif

  H264AVCDecoderTest*   pcH264AVCDecoderTest  = 0;
  ReadBitstreamFile*    pcReadStream          = 0;
#ifdef SHARP_AVC_REWRITE_OUTPUT
  WriteBitstreamToFile* pcWriteStream         = 0;
#else
  WriteYuvToFile*       pcWriteYuv            = 0;
#endif
  DecoderParameter      cParameter;

  //===== create instances =====
  RNOKRS( ReadBitstreamFile   ::create  ( pcReadStream ),                             -1 );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  RNOKRS( WriteBitstreamToFile::create  ( pcWriteStream ),                            -2 );
#else
  RNOKRS( WriteYuvToFile      ::create  ( pcWriteYuv ),                               -2 );
#endif
  RNOKRS( H264AVCDecoderTest  ::create  ( pcH264AVCDecoderTest ),                     -3 );

  //===== initialization =====
  RNOKRS( cParameter           .init    ( argc, argv ),                               -4 );
  RNOKRS( pcReadStream        ->init    ( cParameter.cBitstreamFile ),                -5 );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  RNOKRS( pcWriteStream       ->init    ( cParameter.cYuvFile ),                      -6 );
  RNOKRS( pcH264AVCDecoderTest->init    ( &cParameter, pcReadStream, pcWriteStream ), -7 );
#else
  RNOKRS( pcWriteYuv          ->init    ( cParameter.cYuvFile ),                      -6 );
  RNOKRS( pcH264AVCDecoderTest->init    ( &cParameter, pcReadStream, pcWriteYuv ),    -7 );
#endif

  //===== run =====
  RNOKR ( pcH264AVCDecoderTest->go      (),                                           -8 );

  //===== uninit and destroy instances =====
  RNOKR ( pcH264AVCDecoderTest->uninit  (),                                           -9 );
  RNOKR ( pcH264AVCDecoderTest->destroy (),                                           -10 );
  if( pcReadStream )
  {
    RNOKR ( pcReadStream      ->uninit  (),                                           -11 );
    RNOKR ( pcReadStream      ->destroy (),                                           -12 );
  }
#ifdef SHARP_AVC_REWRITE_OUTPUT
  if( pcWriteStream )
  {
    RNOKR ( pcWriteStream     ->uninit  (),                                           -13 );
    RNOKR ( pcWriteStream     ->destroy (),                                           -14 );
  }
#else
  if( pcWriteYuv )
  {
    RNOKR ( pcWriteYuv        ->uninit  (),                                           -13 );
    RNOKR ( pcWriteYuv        ->destroy (),                                           -14 );
  }
#endif

  return 0;
}
