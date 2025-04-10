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
#include "MVCBStreamExtractor.h"
#include "Extractor.h"


int main( int argc, char** argv)
{
  Extractor*          pcExtractor = NULL;
  ExtractorParameter  cParameter;

  printf( "JMVC %s BitStream Extractor \n\n", _JMVC_VERSION_ );

  RNOKRS( cParameter.init       ( argc, argv ),   -2 );

  for( Int n = 0; n < 1; n++ )
  {
    RNOKR( Extractor::create    ( pcExtractor ),  -3 );

    RNOKR( pcExtractor->init    ( &cParameter ),  -4 );

    RNOKR( pcExtractor->go      (),               -5 );

    RNOKR( pcExtractor->destroy (),               -6 );
  }

  return 0;
}
