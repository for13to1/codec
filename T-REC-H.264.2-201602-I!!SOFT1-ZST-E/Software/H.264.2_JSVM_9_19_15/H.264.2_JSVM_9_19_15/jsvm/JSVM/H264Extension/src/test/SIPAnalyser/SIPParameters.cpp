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

#include "SIPParameters.h"

SIPParameters::SIPParameters():
m_uiLayerNum(0),
m_uiFrameNum(0),
m_pcLayerParameters(NULL),
m_uiInFps(0)
{
  //initialization
}

SIPParameters::~SIPParameters()
{
  if(m_pcLayerParameters!=NULL)
  {
    delete[] m_pcLayerParameters;
    m_pcLayerParameters=NULL;
  }
}

ErrVal SIPParameters::create(SIPParameters*& rpcSIPParameters)
{
  rpcSIPParameters = new SIPParameters;
  ROT( NULL == rpcSIPParameters );
  return Err::m_nOK;
}

ErrVal SIPParameters::init  (Int argc,Char** argv )
{
  if(argc<2||argc>3)
  {
    xPrintUsage();
    return Err::m_nInvalidParameter;
  }

  FILE* pFile = fopen( argv[1], "rt" );

  if( ! pFile )
  {
    printf("\n\nCannot open config file \"%s\"\n\n", argv[1] );
    exit(1);
  }
  else
  {
    if(xReadConfigFile(pFile)!=Err::m_nOK)
    {
      printf("\n\nError while reading from config file \"%s\"\n\n", argv[1] );
      exit(1);
    }
    fclose( pFile );
  }

  RNOK(xCheck());

  if(argc==3)//FileLabel
  {
    for(UInt i=0;i<m_uiLayerNum;i++)
    {
      m_pcLayerParameters[i].m_strInputFileWithInterPred.insert(m_pcLayerParameters[i].m_strInputFileWithInterPred.find_last_of('.'),argv[2]);
      m_pcLayerParameters[i].m_strInputFileWithoutInterPred.insert(m_pcLayerParameters[i].m_strInputFileWithoutInterPred.find_last_of('.'),argv[2]);
      m_pcLayerParameters[i].m_strOutputFile.insert(m_pcLayerParameters[i].m_strOutputFile.find_last_of('.'),argv[2]);
    }
  }
  return Err::m_nOK;
}


void SIPParameters::xPrintUsage()
{
  printf("\nusage:\n\n");
  printf("SIPAnalyser <ConfigFile> [FileLabel]\n");
  printf("\n");
  exit(1);
}

ErrVal SIPParameters::xReadConfigFile(FILE *pFile)
{
  RNOKS(xReadLine(pFile,"",NULL));
  RNOKS(xReadLine(pFile,"",NULL));//logo

  RNOKS(xReadLine(pFile,"%d",&m_uiLayerNum));
  RNOKS(xReadLine(pFile,"%d",&m_uiFrameNum));
  RNOKS(xReadLine(pFile,"%d",&m_uiInFps));
  RNOKS(xReadLine(pFile,"",NULL));//separator

  if(m_pcLayerParameters!=NULL)
  {
    delete[] m_pcLayerParameters;
    m_pcLayerParameters=NULL;
  }
  m_pcLayerParameters=new LayerParameters[m_uiLayerNum];
  char acStrTemp[1024];

  for(UInt i=0;i<m_uiLayerNum;i++)
  {
    RNOKS(xReadLine(pFile,"",NULL));//separator
    RNOKS(xReadLine(pFile,"%f",&m_pcLayerParameters[i].m_fTolerableRatio));
    RNOKS(xReadLine(pFile,"%d",&m_pcLayerParameters[i].m_uiFps));
    RNOKS(xReadLine(pFile,"%s",acStrTemp));
    m_pcLayerParameters[i].m_strInputFileWithoutInterPred=acStrTemp;
    RNOKS(xReadLine(pFile,"%s",acStrTemp));
    m_pcLayerParameters[i].m_strInputFileWithInterPred=acStrTemp;
    RNOKS(xReadLine(pFile,"%s",acStrTemp));
    m_pcLayerParameters[i].m_strOutputFile=acStrTemp;
    RNOKS(xReadLine(pFile,"",NULL));//separator
  }

  return Err::m_nOK;
}

ErrVal SIPParameters::xReadLine( FILE* pFile, const char* pcFormat, void* pPar )
{
  if( pPar )
  {
    int  result = fscanf( pFile, pcFormat, pPar );
    ROT( result <= 0 );
  }

  for( int n = 0; n < 1024; n++ )
  {
    if( '\n' == fgetc( pFile ) )
    {
      return Err::m_nOK;
    }
  }
  return Err::m_nERR;
}

ErrVal SIPParameters::destroy()
{
  if(m_pcLayerParameters!=NULL)
  {
    delete[] m_pcLayerParameters;
    m_pcLayerParameters=NULL;
  }
  delete this;

  return Err::m_nOK;
}

PLAYERPARAMETERS  SIPParameters::getLayerParameter(UInt uiLayer)
{
  AOF(m_pcLayerParameters);
  AOT(uiLayer>=m_uiLayerNum);

  return &m_pcLayerParameters[uiLayer];
}

ErrVal SIPParameters::xCheck()
{
  if(m_uiLayerNum<2||m_uiLayerNum>6)
  {
    printf("Invalidated layer number\n");
    return Err::m_nERR;
  }
  if(m_uiFrameNum<1)
  {
    printf("Invalidated frame number\n");
    return Err::m_nERR;
  }

  for(UInt i=0;i<m_uiLayerNum;i++)
  {
    if(m_pcLayerParameters[i].m_fTolerableRatio<1.00)
    {
      printf("Invalidated tolerable ratio\n");
      return Err::m_nERR;
    }

    if(i>0)
      if(m_pcLayerParameters[i].m_uiFps<m_pcLayerParameters[i-1].m_uiFps||
        m_pcLayerParameters[i].m_uiFps%m_pcLayerParameters[i-1].m_uiFps!=0)//not integer times
      {
        printf("Invalidated fps parameters\n");
        return Err::m_nERR;
      }
    if(m_uiInFps%m_pcLayerParameters[i].m_uiFps!=0)
    {
      printf("Invalidated fps parameters\n");
      return Err::m_nERR;
    }
  }
  return Err::m_nOK;
}
