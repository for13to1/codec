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
#include "AssemblerParameter.h"

#include <string> 

#ifndef MSYS_WIN32
#define stricmp strcasecmp
#endif

#define equal(a,b)  (!stricmp((a),(b)))


using namespace std;

AssemblerParameter::AssemblerParameter()
: m_uiNumViews          (  0    )
, m_pcInFile            ( NULL  )
, m_cOutFile            (       )
, m_uiViewId            (  0    )
, m_iResult             (  -10  )
, m_bTraceFile          ( false )
, m_bTraceAssembler     ( false )
, m_cTraceFile          (       )
, m_cTraceAssmblerFile  (       )
, m_uiSuffix            (  1    )
{
}



AssemblerParameter::~AssemblerParameter()
{
  if( m_pcInFile!=NULL )
      delete [] m_pcInFile;
  m_pcInFile = NULL;
}




ErrVal
AssemblerParameter::init( Int     argc,
                          Char**  argv )	
{
  
  Char* pcCom;


  ROTS( argc < 2 )

  for( Int n = 1; n < argc; n++ )
  {
    pcCom = argv[n++];
// only support configuration files
    if( equals( pcCom, "-vf", 3) )
    {
      ROTS( NULL == argv[n] );
      std::string cFilename = argv[n];
      RNOKS( xReadFromFile( cFilename) );  
      continue;
    }
    return Err::m_nERR;
  }

//  RNOKS( check() );
  return Err::m_nOK;
}


ErrVal
AssemblerParameter::xPrintUsage( Char **argv )
{
  printf("\n supported options:\n\n");
  printf("  -vf Parameter File Name\n\n");
  printf("\n");
  RERRS();
}


ErrVal
AssemblerParameter::xReadFromFile( std::string& rcFilename )
{
  std::string acTags[4];
  UInt        uiParLnCount = 0;
  UInt        uiLayerCnt   = 0;

  FILE *f = fopen( rcFilename.c_str(), "r");
  if( NULL == f )
  { 
    printf( "failed to open %s parameter file\n", rcFilename.c_str() );
    return Err::m_nERR;
  } 

  m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("OutputFile"   ,       &m_cOutFile ,                                      "test_mvc.264");
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("NumberOfViews",       &m_uiNumViews,                                      1 );
//  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("SuffixUnit",          &m_uiSuffix,                                        0 );

  m_pCfgLines[uiParLnCount] = NULL;

  while (!feof(f))
  {
    RNOK( xReadLine( f, acTags ) );
    if ( acTags[0].empty() )
    {
      continue;
    }
    for (UInt ui=0; m_pCfgLines[ui] != NULL; ui++)
    {
//      printf("%s %s \n", acTags[0].c_str(), m_pCfgLines[ui]->getTag().c_str());
      if( acTags[0] == m_pCfgLines[ui]->getTag() )
      {
        m_pCfgLines[ui]->setVar( acTags[1] );
        if( acTags[0] == "NumberOfViews" )
        {
          m_pcInFile= new std::string [m_uiNumViews];
          for(UInt i=0 ; i<m_uiNumViews ; i++, uiLayerCnt++ ) 
          {
            char sview[256] = "";
            sprintf(sview, "InputFile%d",i);
            m_pCfgLines[uiParLnCount++] = new ConfigLineStr (sview       ,       &m_pcInFile[i],                                     "test0.264" );
          }
          m_pCfgLines[uiParLnCount] = NULL;
        }
         break;
      }
    }
  }

  uiParLnCount = 0;
  while (m_pCfgLines[uiParLnCount] != NULL)
  {
    delete m_pCfgLines[uiParLnCount];
    m_pCfgLines[uiParLnCount] = NULL;
    uiParLnCount++;
  }

/*
  if ( uiLayerCnt != m_uiNumViews )
  {
    fprintf(stderr, "Number of the items of input bit-streams do not match NuberOfViews: check config file syntax\n");
    AF();
  }
*/
  fclose( f );
  for( UInt uiV=0; uiV < m_uiNumViews; uiV++ )
  {
    printf("%s \n", m_pcInFile[uiV].c_str());
  }
  printf("Output to: %s\n", m_cOutFile.c_str());
  return Err::m_nOK;
}




ErrVal 
AssemblerParameter::xReadLine( FILE* hFile, std::string* pacTag )
{
  ROF( pacTag );

  Int  n;
  UInt uiTagNum = 0;
  Bool          bComment  = false;
  std::string*  pcTag     = &pacTag[0];

  for( n = 0; n < 4; n++ )
  {
    pacTag[n] = "";
  }

  for( n = 0; ; n++ )
  {
    Char cChar = (Char) fgetc( hFile );
    ROTRS( cChar == '\n' || feof( hFile ), Err::m_nOK );  // end of line
    if   ( cChar == '#' )
    {
      bComment = true;
    }
    if( ! bComment )
    {
      if ( cChar == '\t' || cChar == ' ' ) // white space
      {
        ROTR( uiTagNum == 3, Err::m_nERR );
        if( ! pcTag->empty() )
        {
          uiTagNum++;
          pcTag = &pacTag[uiTagNum]; 
        }
    }
    else
    {
      *pcTag += cChar;
     }
    }
  }
}
