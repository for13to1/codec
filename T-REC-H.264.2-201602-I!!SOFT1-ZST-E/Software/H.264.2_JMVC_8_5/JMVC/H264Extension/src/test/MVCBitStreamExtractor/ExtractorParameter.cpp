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
#include "ExtractorParameter.h"


#ifndef MSYS_WIN32
#define stricmp strcasecmp
#endif

#define equal(a,b)  (!stricmp((a),(b)))




ExtractorParameter::ExtractorParameter()
: m_cInFile       ()
, m_cOutFile      ()
, m_iResult       ( -10 )
, m_bAnalysisOnly ( true )
{
}



ExtractorParameter::~ExtractorParameter()
{
}



ErrVal
ExtractorParameter::init( Int     argc,
                          Char**  argv )	
{
  Bool	bOpIdSpecified			  = false;

  
#define EXIT(x,m) {if(x){printf("\n%s\n",m);RNOKS(xPrintUsage(argv))}}

  //===== get file names and set parameter "AnalysisOnly" =====
 EXIT( argc < 2, "No arguments specified" );
  m_iResult       = 0;
  m_bAnalysisOnly = ( argc == 2 ? true : false );
  m_cInFile       = argv[1];
  ROTRS( m_bAnalysisOnly, Err::m_nOK );
  m_cOutFile      = argv[2];

  //===== process arguments =====
  for( Int iArg = 3; iArg < argc; iArg++ )
  {
	if( equal( "-op", argv[iArg] ) )
	{
      EXIT( iArg + 1 == argc,           "Option \"-viewid\" without argument specified" );
      EXIT( bOpIdSpecified,            "Multiple options \"-t\"" );
      m_uiOpId       = atoi( argv[ ++iArg ] );
      bOpIdSpecified = true;
      continue;	  
	}
    EXIT( true, "Unknown option specified" );
  }

  return Err::m_nOK;
#undef EXIT
}


ErrVal
ExtractorParameter::xPrintUsage( Char **argv )
{
  printf("\nUsage: %s InputStream [OutputStream | [-op] ]", argv[0] ); 
  printf("\noptions:\n");
  printf("\t-op Op -> extract the corresponding packets with operation point id = Op\n");
  printf("\n");
  RERRS();
}



