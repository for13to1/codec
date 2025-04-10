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
#include "DecoderParameter.h"

#ifdef MSYS_WIN32
#define  strcasecmp _stricmp
#endif

DecoderParameter::DecoderParameter()
{
}

DecoderParameter::~DecoderParameter()
{
}

ErrVal DecoderParameter::init(int argc, char** argv)
{
#ifdef SHARP_AVC_REWRITE_OUTPUT
  if( argc != 3)
#else
  if( argc < 3 || argc > 6 ) // HS: decoder robustness
#endif
  {
    RNOKS( xPrintUsage( argv ) );
  }

  cBitstreamFile = argv[1];
  cYuvFile       = argv[2];

  if( argc > 3 )
  {
   if( ! strcasecmp( argv[3], "-ec" ) )
	  {
      if( argc != 5 )
      {
        RNOKS( xPrintUsage( argv ) );
      }
      else
      {
        uiErrorConceal  =  atoi( argv[4] );
        if( uiErrorConceal < 0 || uiErrorConceal > 3 )
        {
          RNOKS( xPrintUsage( argv ) );
        }
        uiMaxPocDiff = 1000; // should be large enough
      }
    }
    else
    {
      uiMaxPocDiff = atoi( argv[3] );
      ROF( uiMaxPocDiff );
      if( argc > 4 )
      {
        if( ! strcasecmp( argv[4], "-ec" ) )
	 	    {
          if( argc != 6 )
          {
            RNOKS( xPrintUsage( argv ) );
          }
          else
          {
            uiErrorConceal  =  atoi( argv[5]);
            if( uiErrorConceal < 1 || uiErrorConceal > 3 )
            {
              RNOKS( xPrintUsage( argv ) );
            }
            uiMaxPocDiff = 1000; // should be large enough
          }
        }
      }
      else
      {
        uiErrorConceal  =  0;
      }
    }
  }
  else
  {
    uiMaxPocDiff = 1000; // should be large enough
    uiErrorConceal  =  0;
  }
  return Err::m_nOK;
}



ErrVal DecoderParameter::xPrintUsage(char **argv)
{
#ifdef SHARP_AVC_REWRITE_OUTPUT
  printf("usage: %s BitstreamFile RewrittenAvcFile\n\n", argv[0] );
#else
  printf("usage: %s BitstreamFile YuvOutputFile [MaxPocDiff] [-ec <1..3>]\n\n", argv[0] );
#endif
  RERRS();
}
