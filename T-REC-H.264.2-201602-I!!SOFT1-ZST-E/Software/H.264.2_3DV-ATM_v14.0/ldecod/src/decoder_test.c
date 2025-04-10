/*
 * Disclaimer of Warranty
 *
 * Copyright 2001-2015, International Telecommunication Union, Geneva
 *
 * These software programs are available to the user without any
 * license fee or royalty on an "as is" basis. The ITU disclaims
 * any and all warranties, whether express, implied, or statutory,
 * including any implied warranties of merchantability or of fitness
 * for a particular purpose.  In no event shall the ITU be liable for
 * any incidental, punitive, or consequential damages of any kind
 * whatsoever arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs
 * and the user's customers, employees, agents, transferees,
 * successors, and assigns.
 *
 * The ITU does not represent or warrant that the programs furnished
 * hereunder are free of infringement of any third-party patents.
 * Commercial implementations of ITU-T Recommendations, including
 * shareware, may be subject to royalty fees to patent holders.
 * Information regarding the ITU-T patent policy is available from the
 * ITU web site at http://www.itu.int.
 *
 * THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.
 *
 */



/*!
 ***********************************************************************
 *  \file
 *     decoder_test.c
 *  \brief
 *     H.264/AVC decoder test 
 *  \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Yuwen He       <yhe@dolby.com>
 ***********************************************************************
 */


#include "contributors.h"

#include <sys/stat.h>


//#include "global.h"

#include "h264decoder.h"
#include "configfile.h"

#if EXT3D
#define MAX_LINELENGTH 1024
#define MAX_MVCITEMLENGTH 255

#include "alc.h"
#endif

#define DECOUTPUT_TEST      0

#define PRINT_OUTPUT_POC    0
#define BITSTREAM_FILENAME  "test.264"
#if EXT3D
#define TEXTURE_DECRECON_FILENAME   "test_text_dec.yuv"
#define DEPTH_DECRECON_FILENAME     "test_depth_dec.yuv"
#else
#define DECRECON_FILENAME   "test_dec.yuv"
#endif
#define ENCRECON_FILENAME   "test_rec.yuv"
#define DECOUTPUT_VIEW0_FILENAME  "H264_Decoder_Output_View0.yuv"
#define DECOUTPUT_VIEW1_FILENAME  "H264_Decoder_Output_View1.yuv"



static void Configure(InputParameters *p_Inp, int ac, char *av[])
{
  //char *config_filename=NULL;
  //char errortext[ET_SIZE];
#if EXT3D
  int i;
#endif
  memset(p_Inp, 0, sizeof(InputParameters));
  strcpy(p_Inp->infile, BITSTREAM_FILENAME); //! set default bitstream name
#if EXT3D
  strcpy(p_Inp->outfile[0], TEXTURE_DECRECON_FILENAME); //!< set default texture output file name
  strcpy(p_Inp->outfile[1],DEPTH_DECRECON_FILENAME);    //!< set default depth output file name
  memset(&(p_Inp->ThreeDVRefFile[0][0][0]),0x00,2*MAX_CODEVIEW*FILE_NAME_SIZE*sizeof(char));
#else
  strcpy(p_Inp->outfile, DECRECON_FILENAME); //! set default output file name
  strcpy(p_Inp->reffile, ENCRECON_FILENAME); //! set default reference file name
#endif
  

  p_Inp->FileFormat = PAR_OF_ANNEXB;
  p_Inp->ref_offset=0;
  p_Inp->poc_scale=2;
  p_Inp->silent = FALSE;
  p_Inp->intra_profile_deblocking = 0;

#ifdef _LEAKYBUCKET_
  p_Inp->R_decoder=500000;          //! Decoder rate
  p_Inp->B_decoder=104000;          //! Decoder buffer size
  p_Inp->F_decoder=73000;           //! Decoder initial delay
  strcpy(p_Inp->LeakyBucketParamFile,"leakybucketparam.cfg");    // file where Leaky Bucket parameters (computed by encoder) are stored
#endif
  p_Inp->iDecFrmNum = 0;

#if EXT3D
  p_Inp->write_uv[0]=p_Inp->write_uv[1]=1;
#else
  p_Inp->write_uv=1;
#endif
  // picture error concealment
  p_Inp->conceal_mode = 0;
  p_Inp->ref_poc_gap = 2;
  p_Inp->poc_gap = 2;

  ParseCommand(p_Inp, ac, av);


  fprintf(stdout,"----------------------------- JM %s %s -----------------------------\n", VERSION, EXT_VERSION);
  //fprintf(stdout," Decoder config file                    : %s \n",config_filename);
  if(!p_Inp->bDisplayDecParams)
  {
    fprintf(stdout,"--------------------------------------------------------------------------\n");
    fprintf(stdout," Input H.264 bitstream                  : %s \n",p_Inp->infile);
#if EXT3D
    fprintf(stdout," Output decoded YUV                     : %s \n",p_Inp->outfile[0]); // @DT
#else
    fprintf(stdout," Output decoded YUV                     : %s \n",p_Inp->outfile);
#endif
    //fprintf(stdout," Output status file                     : %s \n",LOGFILE);
#if EXT3D
    i=0;
    while(strlen(p_Inp->ThreeDVRefFile[0][i]))
    {
      fprintf(stdout," Input reference file                   : %s \n",p_Inp->ThreeDVRefFile[0][i++]);
    }
    i=0;
    while(strlen(p_Inp->ThreeDVRefFile[1][i]))
    {
      fprintf(stdout," Input reference file                   : %s \n",p_Inp->ThreeDVRefFile[1][i++]);
    }
#else
    fprintf(stdout," Input reference file                   : %s \n",p_Inp->reffile);
#endif

    fprintf(stdout,"--------------------------------------------------------------------------\n");
  #ifdef _LEAKYBUCKET_
    fprintf(stdout," Rate_decoder        : %8ld \n",p_Inp->R_decoder);
    fprintf(stdout," B_decoder           : %8ld \n",p_Inp->B_decoder);
    fprintf(stdout," F_decoder           : %8ld \n",p_Inp->F_decoder);
    fprintf(stdout," LeakyBucketParamFile: %s \n",p_Inp->LeakyBucketParamFile); // Leaky Bucket Param file
    calc_buffer(p_Inp);
    fprintf(stdout,"--------------------------------------------------------------------------\n");
  #endif
  }

  if (!p_Inp->silent)
  {
    fprintf(stdout,"POC must = frame# or field# for SNRs to be correct\n");
    fprintf(stdout,"--------------------------------------------------------------------------\n");
    fprintf(stdout,"  Frame          POC  Pic#   QP    SnrY     SnrU     SnrV   Y:U:V Time(ms)\n");
    fprintf(stdout,"--------------------------------------------------------------------------\n");
  }

}

/*********************************************************
if bOutputAllFrames is 1, then output all valid frames to file onetime; 
else output the first valid frame and move the buffer to the end of list;
*********************************************************/
static int WriteOneFrame(DecodedPicList *pDecPic, int hFileOutput0, int hFileOutput1, int bOutputAllFrames)
{
  int iOutputFrame=0;
  DecodedPicList *pPic = pDecPic;

  if(pPic && (((pPic->iYUVStorageFormat==2) && pPic->bValid==3) || ((pPic->iYUVStorageFormat!=2) && pPic->bValid==1)) )
  {
    int i, iWidth, iHeight, iStride, iWidthUV, iHeightUV, iStrideUV;
    byte *pbBuf;    
    int hFileOutput;

    iWidth = pPic->iWidth*((pPic->iBitDepth+7)>>3);
    iHeight = pPic->iHeight;
    iStride = pPic->iYBufStride;
    if(pPic->iYUVFormat != YUV444)
      iWidthUV = pPic->iWidth>>1;
    else
      iWidthUV = pPic->iWidth;
    if(pPic->iYUVFormat == YUV420)
      iHeightUV = pPic->iHeight>>1;
    else
      iHeightUV = pPic->iHeight;
    iWidthUV *= ((pPic->iBitDepth+7)>>3);
    iStrideUV = pPic->iUVBufStride;

    do
    {
      if(pPic->iYUVStorageFormat==2)
        hFileOutput = (pPic->iViewId&0xffff)? hFileOutput1 : hFileOutput0;
      else
        hFileOutput = hFileOutput0;
      if(hFileOutput >=0)
      {
        //Y;
        pbBuf = pPic->pY;
        for(i=0; i<iHeight; i++)
          if ( 0 >= write(hFileOutput, pbBuf+i*iStride, iWidth)) return 0;

        if(pPic->iYUVFormat != YUV400)
        {
          //U;
          pbBuf = pPic->pU;
          for(i=0; i<iHeightUV; i++)
            if (0 >= write(hFileOutput, pbBuf+i*iStrideUV, iWidthUV)) return 0;
          //V;
          pbBuf = pPic->pV;
          for(i=0; i<iHeightUV; i++)
            if (0 >= write(hFileOutput, pbBuf+i*iStrideUV, iWidthUV)) return 0;
        }

        iOutputFrame++;
      }

      if((pPic->iYUVStorageFormat==2))
      {
        hFileOutput = ((pPic->iViewId>>16)&0xffff)? hFileOutput1 : hFileOutput0;
        if(hFileOutput>=0)
        {
          int iPicSize =iHeight*iStride;
          //Y;
          pbBuf = pPic->pY+iPicSize;
          for(i=0; i<iHeight; i++)
            if (0 >= write(hFileOutput, pbBuf+i*iStride, iWidth)) return 0;

          if(pPic->iYUVFormat != YUV400)
          {
            iPicSize = iHeightUV*iStrideUV;
            //U;
            pbBuf = pPic->pU+iPicSize;
            for(i=0; i<iHeightUV; i++)
              if (0 >= write(hFileOutput, pbBuf+i*iStrideUV, iWidthUV)) return 0;
            //V;
            pbBuf = pPic->pV+iPicSize;
            for(i=0; i<iHeightUV; i++)
              if (0 >= write(hFileOutput, pbBuf+i*iStrideUV, iWidthUV)) return 0;
          }

          iOutputFrame++;
        }
      }

#if PRINT_OUTPUT_POC
      fprintf(stdout, "\nOutput frame: %d/%d\n", pPic->iPOC, pPic->iViewId);
#endif
      pPic->bValid = 0;
      pPic = pPic->pNext;
    }while(pPic != NULL && pPic->bValid && bOutputAllFrames);
  }
#if PRINT_OUTPUT_POC
  else
    fprintf(stdout, "\nNone frame output\n");
#endif

  return iOutputFrame;
}

/*!
 ***********************************************************************
 * \brief
 *    main function for JM decoder
 ***********************************************************************
 */
int main(int argc, char **argv)
{
  int iRet;
  DecodedPicList *pDecPicList;
  int hFileDecOutput0=-1, hFileDecOutput1=-1;
  int iFramesOutput=0, iFramesDecoded=0;
  InputParameters InputParams;

#if DECOUTPUT_TEST
  hFileDecOutput0 = open(DECOUTPUT_VIEW0_FILENAME, OPENFLAGS_WRITE, OPEN_PERMISSIONS);
  fprintf(stdout, "Decoder output view0: %s\n", DECOUTPUT_VIEW0_FILENAME);
  hFileDecOutput1 = open(DECOUTPUT_VIEW1_FILENAME, OPENFLAGS_WRITE, OPEN_PERMISSIONS);
  fprintf(stdout, "Decoder output view1: %s\n", DECOUTPUT_VIEW1_FILENAME);
#endif

  //get input parameters;
  Configure(&InputParams, argc, argv);
  //open decoder;
  iRet = OpenDecoder(&InputParams);
  if(iRet != DEC_OPEN_NOERR)
  {
    fprintf(stderr, "Open encoder failed: 0x%x!\n", iRet);
    return -1; //failed;
  }

#if EXT3D 
  ALC_Create();
#endif

  //decoding;
  do
  {
    iRet = DecodeOneFrame(&pDecPicList);
    if(iRet==DEC_EOS || iRet==DEC_SUCCEED)
    {
      //process the decoded picture, output or display;
      iFramesOutput += WriteOneFrame(pDecPicList, hFileDecOutput0, hFileDecOutput1, 0);
      iFramesDecoded++;
    }
    else
    {
      //error handling;
      fprintf(stderr, "Error in decoding process: 0x%x\n", iRet);
    }
  }while((iRet == DEC_SUCCEED) && ((p_Dec->p_Inp->iDecFrmNum==0) || (iFramesDecoded<p_Dec->p_Inp->iDecFrmNum)));
#if EXT3D
  ALC_Destroy();
#endif
  iRet = FinitDecoder(&pDecPicList);
  iFramesOutput += WriteOneFrame(pDecPicList, hFileDecOutput0, hFileDecOutput1 , 1);
  iRet = CloseDecoder();

  //quit;
  if(hFileDecOutput0>=0)
  {
    close(hFileDecOutput0);
  }
  if(hFileDecOutput1>=0)
  {
    close(hFileDecOutput1);
  }
  //printf("%d frames are decoded.\n", iFramesDecoded);
  //printf("%d frames are decoded, %d frames output.\n", iFramesDecoded, iFramesOutput);
  return 0;
}

