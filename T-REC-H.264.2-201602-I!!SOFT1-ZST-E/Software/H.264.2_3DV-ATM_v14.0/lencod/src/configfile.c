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
 * \file
 *    configfile.c
 * \brief
 *    Configuration handling.
 * \author
 *  Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Stephan Wenger           <stewe@cs.tu-berlin.de>
 * \note
 *    In the future this module should hide the Parameters and offer only
 *    Functions for their access.  Modules which make frequent use of some parameters
 *    (e.g. picture size in macroblocks) are free to buffer them on local variables.
 *    This will not only avoid global variable and make the code more readable, but also
 *    speed it up.  It will also greatly facilitate future enhancements such as the
 *    handling of different picture sizes in the same sequence.                         \n
 *                                                                                      \n
 *    For now, everything is just copied to the inp_par structure (gulp)
 *
 **************************************************************************************
 * \par Configuration File Format
 **************************************************************************************
 * Format is line oriented, maximum of one parameter per line                           \n
 *                                                                                      \n
 * Lines have the following format:                                                     \n
 * \<ParameterName\> = \<ParameterValue\> # Comments \\n                                    \n
 * Whitespace is space and \\t
 * \par
 * \<ParameterName\> are the predefined names for Parameters and are case sensitive.
 *   See configfile.h for the definition of those names and their mapping to
 *   cfgparams->values.
 * \par
 * \<ParameterValue\> are either integers [0..9]* or strings.
 *   Integers must fit into the wordlengths, signed values are generally assumed.
 *   Strings containing no whitespace characters can be used directly.  Strings containing
 *   whitespace characters are to be inclosed in double quotes ("string with whitespace")
 *   The double quote character is forbidden (may want to implement something smarter here).
 * \par
 * Any Parameters whose ParameterName is undefined lead to the termination of the program
 * with an error message.
 *
 * \par Known bug/Shortcoming:
 *    zero-length strings (i.e. to signal an non-existing file
 *    have to be coded as "".
 *
 * \par Rules for using command files
 *                                                                                      \n
 * All Parameters are initially taken from DEFAULTCONFIGFILENAME, defined in configfile.h.
 * If an -f \<config\> parameter is present in the command line then this file is used to
 * update the defaults of DEFAULTCONFIGFILENAME.  There can be more than one -f parameters
 * present.  If -p <ParameterName = ParameterValue> parameters are present then these
 * override the default and the additional config file's settings, and are themselves
 * overridden by future -p parameters.  There must be whitespace between -f and -p commands
 * and their respective parameters
 ***********************************************************************
 */

#define INCLUDED_BY_CONFIGFILE_C

#include <sys/stat.h>

#include "global.h"
#include "config_common.h"
#include "configfile.h"
#include "fmo.h"
#include "conformance.h"
#include "mc_prediction.h"
#include "mv_search.h"
#include "img_io.h"
#include "ratectl.h"

#if EXT3D
#include "nonlinear_depth.h"
#include <ctype.h>

#define MAX_LINELENGTH 1024
#define MAX_MVCITEMLENGTH 255

static void CopyConfigFromTexture(VideoParameters* p_Vid, InputParameters* p_Inp);
static void ParseMVCConfigFile(InputParameters* p_Inp, int is_depth);
static void ParseViewsConfig(InputParameters* p_Inp,FILE* pfMVCFile,FILE* pfCamFile, int is_depth);
static void PatchInp                (VideoParameters *p_Vid, InputParameters *p_Inp,int is_depth);
#else
static void PatchInp                (VideoParameters *p_Vid, InputParameters *p_Inp);
#endif
static int  TestEncoderParams       (Mapping *Map, int bitdepth_qp_scale[3]);
static int  DisplayEncoderParams    (Mapping *Map);



static const int mb_width_cr[4] = {0,8, 8,16};
static const int mb_height_cr[4]= {0,8,16,16};

#define MAX_ITEMS_TO_PARSE  10000

/*!
 ***********************************************************************
 * \brief
 *   print help message and exit
 ***********************************************************************
 */
void JMHelpExit (void)
{
  fprintf( stderr, "\n   lencod [-h] [-d defenc.cfg] {[-f curenc1.cfg]...[-f curencN.cfg]}"
    " {[-p EncParam1=EncValue1]..[-p EncParamM=EncValueM]}\n\n"
    "## Parameters\n\n"

    "## Options\n"
    "   -h :  prints function usage\n"
    "   -d :  use <defenc.cfg> as default file for parameter initializations.\n"
    "         If not used then file defaults to encoder.cfg in local directory.\n"
    "   -f :  read <curencM.cfg> for reseting selected encoder parameters.\n"
    "         Multiple files could be used that set different parameters\n"
    "   -p :  Set parameter <EncParamM> to <EncValueM>.\n"
    "         See default encoder.cfg file for description of all parameters.\n\n"

    "## Supported video file formats\n"
    "   RAW:  .yuv.,rgb ->   P444 - Planar, 4:4:4 \n"
    "                        P422 - Planar, 4:2:2 \n"
    "                        P420 - Planar, 4:2:0  \n"
    "                        P400 - Planar, 4:0:0 \n"
    "                        I444 - Packed, 4:4:4 \n"
    "                        I422 - Packed, 4:2:2 \n"
    "                        I420 - Packed, 4:2:0 \n"
    "                        IYUV/YV12 - Planar, 4:2:0 \n"
    "                        IYU1 - Packed, 4:2:0 (UYYVYY) \n"
    "                        IYU2 - Packed, 4:4:4 (UYV) \n"
    "                        YUY2 - Packed, 4:2:2 (YUYV) \n"
    "                        YUV  - Packed, 4:4:4 (YUV) \n\n"

    "## Examples of usage:\n"
    "   lencod\n"
    "   lencod  -h\n"
    "   lencod  -d default.cfg\n"
    "   lencod  -f curenc1.cfg\n"
    "   lencod  -f curenc1.cfg -p InputFile=\"e:\\data\\container_qcif_30.yuv\" -p SourceWidth=176 -p SourceHeight=144\n"
    "   lencod  -f curenc1.cfg -p FramesToBeEncoded=30 -p QPISlice=28 -p QPPSlice=28 -p QPBSlice=30\n");

  exit(-1);
}

/*!
 ************************************************************************
 * \brief
 *    Reads Input File Size 
 *
 ************************************************************************
 */
int64 getVideoFileSize(int video_file)
{
   int64 fsize;   

   lseek(video_file, 0, SEEK_END); 
   fsize = tell((int) video_file); 
   lseek(video_file, 0, SEEK_SET); 

   return fsize;
}

/*!
 ************************************************************************
 * \brief
 *    Updates the number of frames to encode based on the file size
 *
 ************************************************************************
 */
void get_number_of_frames (InputParameters *p_Inp, VideoDataFile *input_file)
{
  int64 fsize = getVideoFileSize(input_file->f_num);
  int64 isize = (int64) p_Inp->source.size;
  int maxBitDepth = imax(p_Inp->source.bit_depth[0], p_Inp->source.bit_depth[1]);

  isize <<= (maxBitDepth > 8)? 1: 0;
  p_Inp->no_frames   = (int) (((fsize - p_Inp->infile_header)/ isize) - p_Inp->start_frame);
}

#if EXT3D
/*!
************************************************************************
* \brief
*    get the index of ViewId in coding order
************************************************************************
*/
int  GetVOIdx(InputParameters* p_Inp, int ViewId)
{
  int  i=0;

  for(i=0;i<p_Inp->NumOfViews;++i)
  {
    if(p_Inp->ViewCodingOrder[i]==ViewId)
      return i;
  }
  return  -1;
}

#endif

/*!
 ************************************************************************
 * \brief
 *    Updates images max values
 *
 ************************************************************************
 */
static void updateMaxValue(FrameFormat *format)
{
  format->max_value[0] = (1 << format->bit_depth[0]) - 1;
  format->max_value_sq[0] = format->max_value[0] * format->max_value[0];
  format->max_value[1] = (1 << format->bit_depth[1]) - 1;
  format->max_value_sq[1] = format->max_value[1] * format->max_value[1];
  format->max_value[2] = (1 << format->bit_depth[2]) - 1;
  format->max_value_sq[2] = format->max_value[2] * format->max_value[2];
}

/*!
 ************************************************************************
 * \brief
 *    Update output format parameters (resolution & bit-depth) given input
 *
 ************************************************************************
 */
static void updateOutFormat(InputParameters *p_Inp)
{
  FrameFormat *output = &p_Inp->output;
  FrameFormat *source = &p_Inp->source;
  output->yuv_format  = (ColorFormat) p_Inp->yuv_format;
  source->yuv_format  = (ColorFormat) p_Inp->yuv_format;

#if EXT3D
  source->force_yuv400  =  p_Inp->force_yuv400;
#endif

  if (p_Inp->src_resize == 0)
  {
    output->width[0]  = source->width[0];
    output->height[0] = source->height[0];
  }

  if (p_Inp->yuv_format == YUV400) // reset bitdepth of chroma for 400 content
  {
    source->bit_depth[1] = 8;
    output->bit_depth[1] = 8;
    source->width[1]  = 0;
    source->width[2]  = 0;
    source->height[1] = 0;
    source->height[2] = 0;
    output->width[1]  = 0;
    output->width[2]  = 0;
    output->height[1] = 0;
    output->height[2] = 0;
  }
  else
  {
    source->width[1]  = (source->width[0]  * mb_width_cr [output->yuv_format]) >> 4;
    source->width[2]  = source->width[1];
    source->height[1] = (source->height[0] * mb_height_cr[output->yuv_format]) >> 4;
    source->height[2] = source->height[1];
    output->width[1]  = (output->width[0]  * mb_width_cr [output->yuv_format]) >> 4;
    output->width[2]  = output->width[1];
    output->height[1] = (output->height[0] * mb_height_cr[output->yuv_format]) >> 4;
    output->height[2] = output->height[1];
  }

#if EXT3D
  if (p_Inp->force_yuv400 == 1)
  {
    source->width[1]  = (source->width[0]  * mb_width_cr [output->yuv_format+1]) >> 4;
    source->width[2]  = source->width[1];
    source->height[1] = (source->height[0] * mb_height_cr[output->yuv_format+1]) >> 4;
    source->height[2] = source->height[1];
  }
#endif

  // source size
  source->size_cmp[0] = source->width[0] * source->height[0];
  source->size_cmp[1] = source->width[1] * source->height[1];
  source->size_cmp[2] = source->size_cmp[1];
  source->size        = source->size_cmp[0] + source->size_cmp[1] + source->size_cmp[2];
  source->mb_width    = source->width[0]  / MB_BLOCK_SIZE;
  source->mb_height   = source->height[0] / MB_BLOCK_SIZE;
  source->pic_unit_size_on_disk = (imax(source->bit_depth[0], source->bit_depth[1]) > 8) ? 16 : 8;
  source->pic_unit_size_shift3 = source->pic_unit_size_on_disk >> 3;


  // output size (excluding padding)
  output->size_cmp[0] = output->width[0] * output->height[0];
  output->size_cmp[1] = output->width[1] * output->height[1];
  output->size_cmp[2] = output->size_cmp[1];
  output->size        = output->size_cmp[0] + output->size_cmp[1] + output->size_cmp[2];
  output->mb_width    = output->width[0]  / MB_BLOCK_SIZE;
  output->mb_height   = output->height[0] / MB_BLOCK_SIZE;


  // both chroma components have the same bitdepth
  source->bit_depth[2] = source->bit_depth[1];
  output->bit_depth[2] = output->bit_depth[1];

  // if no bitdepth rescale ensure bitdepth is same
  if (p_Inp->src_BitDepthRescale == 0) 
  {    
    output->bit_depth[0] = source->bit_depth[0];
    output->bit_depth[1] = source->bit_depth[1];
    output->bit_depth[2] = source->bit_depth[2];
  }
  output->pic_unit_size_on_disk = (imax(output->bit_depth[0], output->bit_depth[1]) > 8) ? 16 : 8;
  output->pic_unit_size_shift3 = output->pic_unit_size_on_disk >> 3;

  if (p_Inp->enable_32_pulldown)
  {
    source->frame_rate  = source->frame_rate  * 5 / 4;
    p_Inp->idr_period   = p_Inp->idr_period   * 5 / 4;
    p_Inp->intra_period = p_Inp->intra_period * 5 / 4;
    p_Inp->no_frames    = p_Inp->no_frames    * 5 / 4;
  }

  output->frame_rate = source->frame_rate / (p_Inp->frame_skip + 1);
  output->color_model = source->color_model;

  updateMaxValue(source);
  updateMaxValue(output);
}



#if EXT3D
/*!
 *************************************************************************
 *\brief
 *Parse the config for each view
 *The structure should be like this!
 *      ViewId  XX
 *      InputFile     filename 
 *      ReconFile   filename  
 *      QPOffset XX
 *      FwdAnchorRefs  XX  XX[-XX-XX-...XX]
 *      BwdAnchorRefs  XX  XX[-XX-XX-...XX]
 *      FwdNonAnchorRefs   XX  XX[-XX-XX-...XX]
 *      BwdNonAnchorRefs   XX  XX[-XX-XX-...XX]

 *\param p_Inp
 *InputParameters structure as input configuration
 *\param pfMVCFile
 *Config file for MVC

 *************************************************************************
 */
void ParseViewsConfig(InputParameters* p_Inp,FILE* pfMVCFile,FILE* pfCamFile, int is_depth)
{
  int iNumOfViews=p_Inp->NumOfViews;
  char szBuffer[MAX_LINELENGTH]={'\0'};
  char szItem[MAX_MVCITEMLENGTH]={'\0'};
  char szNextItem[MAX_MVCITEMLENGTH]  ={'\0'}   ;
  char szItemContent[MAX_MVCITEMLENGTH]={'\0'};
  char szId[MAX_LINELENGTH]={'\0'};

  char szError[MAX_LINELENGTH]={'\0'};

  char*psz=NULL;
  char*comment=NULL;
  int j=0,iEnd=0,iCurrViewId=0,iNumOfRef=0,iVOIdx=0,iViewBeRead=0;
  int iCamParaFound=0;
  int iRead=0;

  memset(szBuffer,0x00,MAX_LINELENGTH);

  strcpy(szNextItem,"ViewId")   ;

  for(iViewBeRead=0;(iViewBeRead<iNumOfViews)&&(fgets(szBuffer,MAX_LINELENGTH,pfMVCFile));)
  {
    psz=szBuffer;
    while((*psz=='\t')||(*psz==' ')||(*psz=='\r'))
      psz++;
    if((*psz=='#')||(*psz=='\n'))
    {
      memset(szBuffer,0x00,MAX_LINELENGTH);
      continue;
    }

    iEnd=0;
    while(isalpha(psz[iEnd]))
      iEnd++;
    memset(szItem,0x00,MAX_MVCITEMLENGTH);
    strncpy(szItem,psz,iEnd);
    psz+=iEnd;

    while((*psz=='\t')||(*psz==' '))
      psz++;
    if((*psz=='#')||(*psz=='\n'))
    {
      snprintf(szError,MAX_LINELENGTH,"The content of %s should be presented clearly.\n", szItem);
      error(szError,100);
    }
    comment=strchr(psz,'#');
    if(comment)
      *comment='\0'  ;
    if(0==strcmp(szItem,szNextItem))
    {
      if(0==strcmp("ViewId",szNextItem))
      {
        //  !<ViewId     
        iRead=sscanf(psz,"%d",&iCurrViewId);
        assert(iRead);
        iVOIdx=GetVOIdx(p_Inp,iCurrViewId);
        if(-1==iVOIdx)
          strcpy(szNextItem,"ViewId");
        else
          strcpy(szNextItem,"InputFile");
      }
      else if(0==strcmp("InputFile",szNextItem))
      {
        //!<InputFile
        iRead=sscanf(psz,"%s",p_Inp->InputFile[iVOIdx].fname);
        assert(iRead);
        if(0==iVOIdx)
        {
          if(is_depth)
            strcpy(szNextItem,"ZNear");
          else
          {
            strcpy(szNextItem,"ViewId");
            iViewBeRead++;
            continue;
          }
        }
        else
          strcpy(szNextItem,"QPOffset")  ;
      }
      else if(0==strcmp("QPOffset",szNextItem))
      {
        //!<QPOffset
        iRead=sscanf(psz,"%d",&(p_Inp->ViewQPOffset[iVOIdx]))   ;
        assert(iRead);
        strcpy(szNextItem,"ReorderAtAnchor");
      }
      else if(0==strcmp("ReorderAtAnchor",szNextItem))
      {
        //!<InterviewReorder
        //iRead=sscanf(psz,"%4s",&(p_Inp->ReorderAtAnchor[iVOIdx]));
        iRead=sscanf(psz,"%4s",&(p_Inp->ReorderAtAnchor[iVOIdx][0]));  // Dong2Dmytro: Is this right?
        assert(iRead);
        strcpy(szNextItem,"ReorderAtNonAnchor")  ;
      }
      else if(0==strcmp("ReorderAtNonAnchor",szNextItem))
      {
        //iRead=sscanf(psz,"%4s",&(p_Inp->ReorderAtNonAnchor[iVOIdx]));
        iRead=sscanf(psz,"%4s",&(p_Inp->ReorderAtNonAnchor[iVOIdx][0]));  // Dong2Dmytro: Is this right?
        assert(iRead);
        strcpy(szNextItem,"FwdAnchorRefs")  ;
      }
      else if(0==strcmp("FwdAnchorRefs",szNextItem))
      {
        //!<FwdAnchorRefs
        iEnd=0;
        while(isdigit(psz[iEnd]))
          iEnd++;
        assert(iEnd);

        memset(szItemContent,0x00,MAX_MVCITEMLENGTH);
        strncpy(szItemContent,psz,iEnd);

        iNumOfRef=p_Inp->NumOfFwdAnchorRefs[iVOIdx]=atoi(szItemContent);
        if(iNumOfRef>0)
        {
          psz+=iEnd;
          while((*psz=='\t')||(*psz==' ')||(*psz=='\r'))
            psz++;
          for(j=0;;)
          {
            iEnd=0;
            while(isdigit(psz[iEnd]))
              iEnd++;
            assert(iEnd);

            memset(szItemContent,0x00,MAX_MVCITEMLENGTH);
            strncpy(szItemContent,psz,iEnd);
            p_Inp->FwdAnchorRefs[iVOIdx][j]=atoi(szItemContent);
            psz+=iEnd;
            j++;
            if(j==iNumOfRef)
              break;
            while((*psz!='#')&&(*psz!='\n')&&(!isdigit(*psz)))
              psz++;
            if(!isdigit(*psz))
            {
              snprintf(szError,MAX_LINELENGTH,"The format of anchor reference frame for view %d is invalid\n",iCurrViewId);
              error(szError,100);
            }
          }
        }
        strcpy(szNextItem,"BwdAnchorRefs")  ;
      }
      else if(0==strcmp("BwdAnchorRefs",szNextItem))
      {
        //!<BwdAnchorRefs
        iEnd=0;
        while(isdigit(psz[iEnd]))
          iEnd++;
        assert(iEnd);

        memset(szItemContent,0x00,MAX_MVCITEMLENGTH);
        strncpy(szItemContent,psz,iEnd);

        iNumOfRef=p_Inp->NumOfBwdAnchorRefs[iVOIdx]=atoi(szItemContent);
        if(iNumOfRef>0)
        {
          psz+=iEnd;
          while((*psz=='\t')||(*psz==' ')||(*psz=='\r'))
            psz++;
          for(j=0;;)
          {
            iEnd=0;
            while(isdigit(psz[iEnd]))
              iEnd++;
            assert(iEnd);

            memset(szItemContent,0x00,MAX_MVCITEMLENGTH);
            strncpy(szItemContent,psz,iEnd);
            p_Inp->BwdAnchorRefs[iVOIdx][j]=atoi(szItemContent);
            psz+=iEnd;
            j++;
            if(j==iNumOfRef)
              break;
            while((*psz!='#')&&(*psz!='\n')&&(!isdigit(*psz)))
              psz++;
            if(!isdigit(*psz))
            {
              snprintf(szError,MAX_LINELENGTH,"The format of anchor reference frame for view %d is invalid\n",iCurrViewId);
              error(szError,100);
            }
          }
        }
        strcpy(szNextItem,"FwdNonAnchorRefs")  ;
      }
      else if(0==strcmp("FwdNonAnchorRefs",szNextItem))
      {
        //!<FwdNonAnchorRefs
        iEnd=0;
        while(isdigit(psz[iEnd]))
          iEnd++;
        memset(szItemContent,0x00,MAX_MVCITEMLENGTH);
        strncpy(szItemContent,psz,iEnd);

        iNumOfRef=p_Inp->NumOfFwdNoAnchorRefs[iVOIdx]=atoi(szItemContent);
        if(iNumOfRef>0)
        {
          psz+=iEnd;
          while((*psz=='\t')||(*psz==' ')||(*psz=='\r'))
            psz++;
          for(j=0;;)
          {
            iEnd=0;
            while(isdigit(psz[iEnd]))
              iEnd++;
            assert(iEnd);

            memset(szItemContent,0x00,MAX_MVCITEMLENGTH);
            strncpy(szItemContent,psz,iEnd);
            p_Inp->FwdNoAnchorRefs[iVOIdx][j]=atoi(szItemContent);
            psz+=iEnd;
            j++;
            if(j==iNumOfRef)
              break;
            while((*psz!='#')&&(*psz!='\n')&&(!isdigit(*psz)))
              psz++;
            if(!isdigit(*psz))
            {
              snprintf(szError,MAX_LINELENGTH,"The format of forward non-anchor reference frame for view %d is invalid\n",iCurrViewId);
              error(szError,100);
            }
          }
        }
        strcpy(szNextItem,"BwdNonAnchorRefs")  ;
      }
      else if(0==strcmp("BwdNonAnchorRefs",szNextItem))
      {
        //!<BwdNonAnchorRefs
        iEnd=0;
        while(isdigit(psz[iEnd]))
          iEnd++;
        memset(szItemContent,0x00,MAX_MVCITEMLENGTH);
        strncpy(szItemContent,psz,iEnd);

        iNumOfRef=p_Inp->NumOfBwdNoAnchorRefs[iVOIdx]=atoi(szItemContent);
        if(iNumOfRef>0)
        {
          psz+=iEnd;
          while((*psz=='\t')||(*psz==' ')||(*psz=='\r'))
            psz++;
          for(j=0;;)
          {
            iEnd=0;
            while(isdigit(psz[iEnd]))
              iEnd++;
            assert(iEnd);

            memset(szItemContent,0x00,MAX_MVCITEMLENGTH);
            strncpy(szItemContent,psz,iEnd);
            p_Inp->BwdNoAnchorRefs[iVOIdx][j]=atoi(szItemContent);
            psz+=iEnd;
            j++;
            if(j==iNumOfRef)
              break;
            while((*psz!='#')&&(*psz!='\n')&&(!isdigit(*psz)))
              psz++;
            if(!isdigit(*psz))
            {
              snprintf(szError,MAX_LINELENGTH,"The format of back non-anchor reference frame for view %d is invalid\n",iCurrViewId);
              error(szError,100);
            }
          }
        }
        if(is_depth)
          strcpy(szNextItem,"ZNear");
        else
        {
          strcpy(szNextItem,"ViewId");
          iViewBeRead++;
          continue;;
        }
      }
      else if(0==strcmp("ZNear",szNextItem))
      {
        if(p_Inp->AcquisitionIdx==1)
        {
          iRead=sscanf(psz,"%lf",&( p_Inp->ZNears[iVOIdx][0]))  ;
          assert(iRead);
        }
        strcpy(szNextItem,"ZFar");
      }
      else if(0==strcmp("ZFar",szNextItem))
      {
        //!<ZFar
        if(p_Inp->AcquisitionIdx==1)
        {
          iRead=sscanf(psz,"%lf",&( p_Inp->ZFars[iVOIdx][0]))  ;
          assert(iRead);
        }
        strcpy(szNextItem,"CameraName");
      }
      else if(0==strcmp("CameraName",szNextItem))
      {
        //!<CameraName
        int iRead=0;
        iEnd=0;

        iRead=sscanf(psz,"%s",szItemContent)   ;
        assert(iRead);

        fseek(pfCamFile,0,SEEK_SET)  ;
        iCamParaFound=0;

        iRead=0;
        while(0<fscanf(pfCamFile,"%s",szId)) // O.STANKIEWICZ: BUG-FIX
        {
          if(0==strcmp(szId,szItemContent))
          {
  
            double Gomi[2]={0.0,0.0};
            iRead += fscanf(pfCamFile, "%lf %lf %lf", &(p_Inp->IntrinsicMatrix[iVOIdx][0][0]), 
              &(p_Inp->IntrinsicMatrix[iVOIdx][0][1]), &(p_Inp->IntrinsicMatrix[iVOIdx][0][2]));
            iRead += fscanf(pfCamFile, "%lf %lf %lf", &(p_Inp->IntrinsicMatrix[iVOIdx][1][0]), 
              &(p_Inp->IntrinsicMatrix[iVOIdx][1][1]), &(p_Inp->IntrinsicMatrix[iVOIdx][1][2]));
            iRead += fscanf(pfCamFile, "%lf %lf %lf", &(p_Inp->IntrinsicMatrix[iVOIdx][2][0]), 
              &(p_Inp->IntrinsicMatrix[iVOIdx][2][1]), &(p_Inp->IntrinsicMatrix[iVOIdx][2][2]));
            iRead += fscanf(pfCamFile, "%lf %lf", &Gomi[0], &Gomi[1]);
            if(p_Inp->AcquisitionIdx==2)
            {
              iRead += fscanf(pfCamFile, "%lf %lf %lf %*s", &(p_Inp->ExtrinsicMatrix[iVOIdx][0][0]), 
                &(p_Inp->ExtrinsicMatrix[iVOIdx][0][1]), &(p_Inp->ExtrinsicMatrix[iVOIdx][0][2]) );
            }
            else
            {
              iRead += fscanf(pfCamFile, "%lf %lf %lf %lf", &(p_Inp->ExtrinsicMatrix[iVOIdx][0][0]), 
                &(p_Inp->ExtrinsicMatrix[iVOIdx][0][1]), &(p_Inp->ExtrinsicMatrix[iVOIdx][0][2]), &(p_Inp->TranslationVector[iVOIdx][0][0]) );
            }
            iRead += fscanf(pfCamFile, "%lf %lf %lf %lf", &(p_Inp->ExtrinsicMatrix[iVOIdx][1][0]), 
              &(p_Inp->ExtrinsicMatrix[iVOIdx][1][1]), &(p_Inp->ExtrinsicMatrix[iVOIdx][1][2]), &(p_Inp->TranslationVector[iVOIdx][1][0]) );
            iRead += fscanf(pfCamFile, "%lf %lf %lf %lf", &(p_Inp->ExtrinsicMatrix[iVOIdx][2][0]), 
              &(p_Inp->ExtrinsicMatrix[iVOIdx][2][1]), &(p_Inp->ExtrinsicMatrix[iVOIdx][2][2]), &(p_Inp->TranslationVector[iVOIdx][2][0]) );
            if((iRead!=23)&&(iRead!=22))
            {
              printf("The format of the camera file is invalid\n");
              iCamParaFound=0;
            }
            else
              iCamParaFound=1;
            break;
          }
        }
        if(!iCamParaFound)
        {
          snprintf(szError,MAX_LINELENGTH,"The camera parameter of %s for view %d can not be found.\n",szItemContent,iCurrViewId);
          error(szError,100);
        }
        strcpy(szNextItem,"TanslationFile");
      }
      else if(0==strcmp("TanslationFile",szNextItem))
      {
        //!<TanslationFile
        int iFrameBeRead=0;
        FILE* pfTransFile=NULL;
        if(p_Inp->AcquisitionIdx!=2)
        {
          strcpy(szNextItem,"ZNearFile");
          continue;
        }

        iRead=sscanf(psz,"%s",szItemContent)   ;
        assert(iRead);
        if((pfTransFile=fopen(szItemContent,"r"))==NULL)
        {
          snprintf(szError,MAX_LINELENGTH,"The translation file for view %d can not be open",iCurrViewId);
          error(szError,100);
        }
        iRead=0;
        while((iFrameBeRead<p_Inp->no_frames+p_Inp->start_frame)&&!feof(pfTransFile))
        {
          double Translation=0.0;
          iRead+=fscanf(pfTransFile,"%lf",&(Translation))  ;
          if(iFrameBeRead>=p_Inp->start_frame)
          {
            p_Inp->TranslationVector[iVOIdx][0][iFrameBeRead-p_Inp->start_frame]=Translation;
            p_Inp->TranslationVector[iVOIdx][1][iFrameBeRead-p_Inp->start_frame]=p_Inp->TranslationVector[iVOIdx][1][0];
            p_Inp->TranslationVector[iVOIdx][2][iFrameBeRead-p_Inp->start_frame]=p_Inp->TranslationVector[iVOIdx][2][0];
          }
          iFrameBeRead++;
        }
        if(iRead!=p_Inp->no_frames+p_Inp->start_frame)
        {
          snprintf(szError,MAX_LINELENGTH,"The translation file may be invalid, because the number of the translation is smaller than the number of frame to be encoded.\n") ;
          error(szError,100);
        }
        if(pfTransFile)
          fclose(pfTransFile);
        strcpy(szNextItem,"ZNearFile");
      }
      else if(0==strcmp("ZNearFile",szNextItem))
      {
        //!<ZNear file
        int iFrameBeRead=0;
        FILE* pfZNearFile=NULL;
        if(p_Inp->AcquisitionIdx!=2)
        {
          strcpy(szNextItem,"ZFarFile");
          continue;
        }
        
        iRead=sscanf(psz,"%s",szItemContent)   ;
        assert(iRead);

        if((pfZNearFile=fopen(szItemContent,"r"))==NULL)
        {
          snprintf(szError,MAX_LINELENGTH,"The ZNear file for view %d can not be open",iCurrViewId);
          error(szError,100);
        }
        iRead=0;
        while((iFrameBeRead<p_Inp->no_frames+p_Inp->start_frame)&&!feof(pfZNearFile))
        {
          double ZNear=0.0;
          iRead+=fscanf(pfZNearFile,"%lf",&(ZNear))  ;
          if(iFrameBeRead>=p_Inp->start_frame)
            p_Inp->ZNears[iVOIdx][iFrameBeRead-p_Inp->start_frame]=ZNear;
          iFrameBeRead++;
        }
        if(iRead!=p_Inp->no_frames+p_Inp->start_frame)
        {
          snprintf(szError,MAX_LINELENGTH,"The ZNear file is invalid, because the number of the translation is smaller than the number of frame to be encoded.\n")  ;
          error(szError,100);
        }
        if(pfZNearFile)
          fclose(pfZNearFile);
        strcpy(szNextItem,"ZFarFile");
      }
      else if(0==strcmp("ZFarFile",szNextItem))
      {
        //!<ZFar file
        int iFrameBeRead=0;
        FILE* pfZFarFile=NULL;
        if(p_Inp->AcquisitionIdx!=2)
        {
          strcpy(szNextItem,"GridPosX");
          continue;
        }
        iEnd=0;

        iRead=sscanf(psz,"%s",szItemContent)   ;
        assert(iRead);
        if((pfZFarFile=fopen(szItemContent,"r"))==NULL)
        {
          snprintf(szError,MAX_LINELENGTH,"The ZNear file for view %d can not be open",iCurrViewId);
          error(szError,100);
        }
        iRead=0;
        while((iFrameBeRead<p_Inp->no_frames+p_Inp->start_frame)&&!feof(pfZFarFile))
        {
          double ZFar=0.0;
          iRead+=fscanf(pfZFarFile,"%lf",&(ZFar))  ;
          if(iFrameBeRead>=p_Inp->start_frame)
            p_Inp->ZFars[iVOIdx][iFrameBeRead-p_Inp->start_frame]=ZFar;
          iFrameBeRead++;
        }
        if(iRead!=p_Inp->no_frames+p_Inp->start_frame)
        {
          snprintf(szError,MAX_LINELENGTH,"The ZNear file is invalid, because the number of the translation is smaller than the number of frame to be encoded.\n")  ;
          error(szError,100);
        }
        if(pfZFarFile)
          fclose(pfZFarFile);
        strcpy(szNextItem,"GridPosX");
      }

      else if(0==strcmp("GridPosX",szNextItem))
      {
        iRead=sscanf(psz,"%d",&(p_Inp->grid_pos_x[iCurrViewId]))  ;
        assert(iRead);
        strcpy(szNextItem,"GridPosY");
      }
      else if(0==strcmp("GridPosY",szNextItem))
      {
        iRead=sscanf(psz,"%d",&(p_Inp->grid_pos_y[iCurrViewId]))  ;
        assert(iRead);

        strcpy(szNextItem,"ViewId");
        iViewBeRead++;
      }

    }
    else
    {
      if(strcmp(szNextItem,"ViewId"))
      {
        snprintf(szError,MAX_LINELENGTH,"%s for each view should be presented clearly.\n",szNextItem);
        error(szError,100);
      }

    }
    memset(szBuffer,0x00,MAX_LINELENGTH);
  }

  if(iViewBeRead!=iNumOfViews)
  {
    snprintf(szError,MAX_LINELENGTH,"The config for each view should be presented correctly\n");
    error(szError,100);
  }
}

/*!
 ************************************************************************
 *\brief
 *Parse  the multi-view config file.
 *\param p_Inp
 *InputParameters structure as input configuration
 ************************************************************************
 */
void ParseMVCConfigFile(InputParameters* p_Inp, int is_depth)
{
  FILE* pfMVCFile=NULL;
  int i,iEnd=0,iNumOfViews=0,iRead=0; // iStart=0

  char szBuffer[MAX_LINELENGTH]={'\0'};
  char szItem[MAX_MVCITEMLENGTH]={'\0'};
  char szNextItem[MAX_MVCITEMLENGTH]={'\0'};
  char szItemContent[MAX_MVCITEMLENGTH]={'\0'};
  char CamParaFileName[FILE_NAME_SIZE]={'\0'};

  char szError[MAX_LINELENGTH]={'\0'};
  char* psz=NULL;
  long int liFilePos=0;

  pfMVCFile=fopen(p_Inp->ThreeDVConfigFileName,"rb");
  if(pfMVCFile==NULL)
  {
    printf("Open MVC config file failed. Non-base view is out of consideration.\n");
    p_Inp->NumOfViews=1;
    return ;
  }

  iNumOfViews=p_Inp->NumOfViews;



  strcpy(szNextItem,"ViewCodingOrder");

  while(!feof(pfMVCFile))
  {
    memset(szBuffer,0x00,MAX_LINELENGTH);
    memset(szItem,0x00,MAX_MVCITEMLENGTH);

    liFilePos=ftell(pfMVCFile)  ;

    if(!fgets(szBuffer,MAX_LINELENGTH,pfMVCFile))
    {
      snprintf(szError,MAX_LINELENGTH,"Read MVC config file failed!");
      error(szError,100);
    }

    psz=szBuffer;
    while((*psz=='\t')||(*psz==' ')||(*psz=='\r'))
      psz++;

    if((*psz=='#')||(*psz=='\n'))
      continue;//!<skip comment or empty line
    else
    {
      char*comment=NULL;
      // iStart=0;
      iEnd=0   ;
      while(isalpha(psz[iEnd]))
        iEnd++;
      strncpy(szItem,psz,iEnd);
      psz+=iEnd;
      while((*psz=='\t')||(*psz==' ')||(*psz=='\r'))
        psz++;
      if((*psz=='#')||(*psz=='\n'))
      {
        snprintf(szError,MAX_LINELENGTH,"The content of %s should be presented clearly",szItem);
        error(szError,100);
      }

      comment=strchr(psz,'#');
      if(comment)
        *comment='\0';
       if(0==strcmp(szItem,szNextItem))
       {
         if(0==strcmp("ViewCodingOrder",szNextItem))
         {
           //!<view coding order
          assert(isdigit(*psz));   //!<only work during debug 
           for(i=0;i<iNumOfViews;i++)
           {
             memset(szItemContent,0x00,MAX_MVCITEMLENGTH);
             while(*psz=='-')
               psz++;

             iEnd=0;
             while(isdigit(psz[iEnd]))
               iEnd++;
             strncpy(szItemContent,psz,iEnd);
             psz+=iEnd;
             p_Inp->ViewCodingOrder[i]=atoi(szItemContent);
           }
           if(is_depth)
             strcpy(szNextItem,"AcquisitionIdc")  ;
           else
             strcpy(szNextItem,"ViewId");
         }
         else if(0==strcmp("AcquisitionIdc",szNextItem))
         {
           iRead=sscanf(psz,"%d",&(p_Inp->AcquisitionIdx));
           if (iRead <= 0)
           {
              snprintf(szError,MAX_LINELENGTH,"Acquisition index");
              error(szError,100);
           }
           if(p_Inp->AcquisitionIdx)
           {
             for(i=0;i<p_Inp->NumOfViews;++i)
             {
               p_Inp->TranslationVector[i][0]=calloc((p_Inp->AcquisitionIdx==2)?p_Inp->no_frames:1,sizeof(double));
               p_Inp->TranslationVector[i][1]=calloc((p_Inp->AcquisitionIdx==2)?p_Inp->no_frames:1,sizeof(double));
               p_Inp->TranslationVector[i][2]=calloc((p_Inp->AcquisitionIdx==2)?p_Inp->no_frames:1,sizeof(double));
               p_Inp->ZNears[i]=calloc((p_Inp->AcquisitionIdx==2)?p_Inp->no_frames:1,sizeof(double));
               p_Inp->ZFars[i]=calloc((p_Inp->AcquisitionIdx==2)?p_Inp->no_frames:1,sizeof(double));
             }
             for(;i<MAX_CODEVIEW;++i)
             {
               p_Inp->TranslationVector[i][0]=NULL;
               p_Inp->TranslationVector[i][1]=NULL;
               p_Inp->TranslationVector[i][2]=NULL;
               p_Inp->ZNears[i]=NULL;
               p_Inp->ZFars[i]=NULL;
             }
           }
           strcpy(szNextItem,"CameraOrder");
         }
         else if(0==strcmp("CameraOrder",szNextItem))
         {
           //!<camera order
           assert(isdigit(*psz));     
           for(i=0;i<iNumOfViews;i++)
           {
             memset(szItemContent,0x00,MAX_MVCITEMLENGTH);
             while(*psz=='-')
               psz++;

             iEnd=0;
             while(isdigit(psz[iEnd]))
               iEnd++;
             strncpy(szItemContent,psz,iEnd);
             psz+=iEnd;
             p_Inp->CamOrder[i]=atoi(szItemContent);
           }
           strcpy(szNextItem,"CameraParameterFile")  ;
         }
         else if(0==strcmp("CameraParameterFile",szNextItem))
         {
           //!<CameraParameterFile
           iRead=sscanf(psz,"%s",CamParaFileName);
           if (iRead <= 0)
           {
              snprintf(szError,MAX_LINELENGTH,"CamParaFileName");
              error(szError,100);
           }
           strcpy(szNextItem,"ViewId");
         }
         else if(0==strcmp("ViewId",szNextItem))
         {
           //!<ViewId
           FILE* pfCamFile=NULL;
           if(is_depth)
           {
             if(NULL==(pfCamFile=fopen(CamParaFileName,"rb")))
             {
               snprintf(szError,MAX_LINELENGTH,"Open the camera file failed.\n");
               error(szError,100);
             }
           }
           fseek(pfMVCFile,liFilePos,SEEK_SET);
           ParseViewsConfig(p_Inp,pfMVCFile,pfCamFile,is_depth);
           if(pfCamFile)
             fclose(pfCamFile);
           break;
         }
      }
      else
      {
        snprintf(szError,MAX_LINELENGTH,"The content of %s should be presented clearly\n",szItem);
        error(szError,100);
      }
    }
  }
  fclose(pfMVCFile);
}

/*!
 ************************************************************************
 *\brief
 *Parse  the reference display config file.
 *\param p_Inp
 *InputParameters structure as input configuration
 ************************************************************************
 */
void ParseRefDisplayConfigFile(InputParameters* p_Inp)
{
  FILE* pfMVCFile=NULL;
  int iEnd=0,iRead=0;
  char szBuffer[MAX_LINELENGTH]={'\0'};
  char szItem[MAX_MVCITEMLENGTH]={'\0'};
  char szNextItem[MAX_MVCITEMLENGTH]={'\0'};

  char szError[MAX_LINELENGTH]={'\0'};
  char* psz=NULL;
  int numberOfDisplays = 0;
  int count = 0;
  Boolean refViewDistanceFlag = 0;
  pfMVCFile=fopen(p_Inp->ReferenceDisplayFile,"rb");
  if(pfMVCFile==NULL)
  {
    printf("\nError opening reference display parameter file. Reference display info SEI will not be sent.\n");
    p_Inp->ReferenceDisplayInfoFlag = 0;
    return ;
  }

  strcpy(szNextItem,"BaselinePrecision");

  while(!feof(pfMVCFile))
  {
    memset(szBuffer,0x00,MAX_LINELENGTH);
    memset(szItem,0x00,MAX_MVCITEMLENGTH);

    if(!fgets(szBuffer,MAX_LINELENGTH,pfMVCFile))
    {
      snprintf(szError,MAX_LINELENGTH,"Reading reference config file failed!");
      error(szError,100);
    }

    psz=szBuffer;
    while((*psz=='\t')||(*psz==' ')||(*psz=='\r'))
      psz++;

    if((*psz=='#')||(*psz=='\n'))
      continue;//!<skip comment or empty line
    else
    {
      char*comment=NULL;
      // iStart=0;
      iEnd=0   ;
      while(isalpha(psz[iEnd]))
        iEnd++;
      strncpy(szItem,psz,iEnd);
      psz+=iEnd;
      while((*psz=='\t')||(*psz==' ')||(*psz=='\r'))
        psz++;
      if((*psz=='#')||(*psz=='\n'))
      {
        snprintf(szError,MAX_LINELENGTH,"The content of %s should be presented clearly",szItem);
        error(szError,100);
      }

      comment=strchr(psz,'#');
      if(comment)
        *comment='\0';
      if(0==strcmp(szItem,szNextItem))
      {
        if(0==strcmp("BaselinePrecision",szNextItem))
        {
          iRead=sscanf(psz,"%d",&(p_Inp->BaselinePrecision));
          if (iRead <= 0)
          {
            snprintf(szError,MAX_LINELENGTH,"Baseline Precision");
            error(szError,100);
          }
          strcpy(szNextItem,"WidthPrecision");
        }
        else if(0==strcmp("WidthPrecision",szNextItem))
        {
          iRead=sscanf(psz,"%d",&(p_Inp->WidthPrecsion));
          if (iRead <= 0)
          {
            snprintf(szError,MAX_LINELENGTH,"Width precision");
            error(szError,100);
          }
          if (refViewDistanceFlag == 0)
            strcpy(szNextItem,"ViewingDistancePrecision");
          else
            strcpy(szNextItem,"ReferenceBaseline");
        }
        else if(0==strcmp("ViewingDistancePrecision",szNextItem))
        {
          p_Inp->ReferenceViewingDistanceFlag = 1;
          iRead=sscanf(psz,"%d",&(p_Inp->ViewingDistancePrecision));
          if (iRead <= 0)
          {
            snprintf(szError,MAX_LINELENGTH,"Viewing distance precision");
            error(szError,100);
          }
          strcpy(szNextItem,"ReferenceBaseline");
        }
        else if(0==strcmp("ReferenceBaseline",szNextItem))
        {
          if(count != 0)
            numberOfDisplays += 1;
          iRead=sscanf(psz,"%lf",&(p_Inp->ReferenceBaseline[numberOfDisplays]));
          if (iRead <= 0)
          {
            snprintf(szError,MAX_LINELENGTH,"Reference baseline");
            error(szError,100);
          }
          strcpy(szNextItem,"ReferenceWidth");
          count += 1;
        }
        else if(0==strcmp("ReferenceWidth",szNextItem))
        {
          iRead=sscanf(psz,"%lf",&(p_Inp->ReferenceWidth[numberOfDisplays]));
          if (iRead <= 0)
          {
            snprintf(szError,MAX_LINELENGTH,"Reference width");
            error(szError,100);
          }
          if(refViewDistanceFlag == 1)
            strcpy(szNextItem,"SampleShift");
          else
            strcpy(szNextItem,"ReferenceViewingDistance");
        }
        else if(0==strcmp("ReferenceViewingDistance",szNextItem))
        {
          iRead=sscanf(psz,"%lf",&(p_Inp->ReferenceViewingDistance[numberOfDisplays]));
          if (iRead <= 0)
          {
            snprintf(szError,MAX_LINELENGTH,"Reference viewing distance");
            error(szError,100);
          }
          strcpy(szNextItem,"SampleShift");
        }
        else if(0==strcmp("SampleShift",szNextItem))
        {
          iRead=sscanf(psz,"%d",&(p_Inp->NumSampleShift[numberOfDisplays]));
          p_Inp->NumSampleShiftFlag[numberOfDisplays] = 1;
          if (iRead <= 0)
          {
            snprintf(szError,MAX_LINELENGTH,"Number sample shift");
            error(szError,100);
          }
          strcpy(szNextItem,"ReferenceBaseline");
        }
      }
      else
      {
        if (0==strcmp("ViewingDistancePrecision",szNextItem))
        {
          p_Inp->ReferenceViewingDistanceFlag = 0;
          refViewDistanceFlag = 1;
          if(count != 0)
            numberOfDisplays += 1;
          iRead=sscanf(psz,"%lf",&(p_Inp->ReferenceBaseline[numberOfDisplays]));
          if (iRead <= 0)
          {
            snprintf(szError,MAX_LINELENGTH,"Reference baseline");
            error(szError,100);
          }
          strcpy(szNextItem,"ReferenceWidth");
          count += 1;
          continue;
        }
        if (0==strcmp("SampleShift",szNextItem))
        {
          p_Inp->NumSampleShiftFlag[numberOfDisplays] = 0;
          if (0==strcmp("ReferenceBaseline",szItem))
          {
            if(count != 0)
              numberOfDisplays += 1;
            iRead=sscanf(psz,"%lf",&(p_Inp->ReferenceBaseline[numberOfDisplays]));
            if (iRead <= 0)
            {
              snprintf(szError,MAX_LINELENGTH,"Reference baseline");
              error(szError,100);
            }
            strcpy(szNextItem,"ReferenceWidth");
            count += 1;
          }
          else if (refViewDistanceFlag == 1 && 0==strcmp("ReferenceViewingDistance",szItem))
            strcpy(szNextItem,"SampleShift");
          else
            strcpy(szNextItem,"ReferenceBaseline");
          continue;
        }
        if (0==strcmp("ReferenceViewingDistance",szNextItem) && refViewDistanceFlag == 1)
        {
          continue;
        }
        snprintf(szError,MAX_LINELENGTH,"The content of %s should be presented clearly\n",szItem);
        error(szError,100);
      }
    }
  }
  p_Inp->NumberOfDiplays = numberOfDisplays + 1;
  fclose(pfMVCFile);
}
#endif

/*!
 ***********************************************************************
 * \brief
 *    Parse the command line parameters and read the config files.
 * \param p_Vid
 *    VideoParameters structure for encoding
 * \param p_Inp
 *    InputParameters structure as input configuration
 * \param ac
 *    number of command line parameters
 * \param av
 *    command line parameters
 ***********************************************************************
 */
#if EXT3D
void Configure (VideoParameters *p_Vid, InputParameters *p_Inp, int ac, char *av[],int is_depth)
#else
void Configure (VideoParameters *p_Vid, InputParameters *p_Inp, int ac, char *av[])
#endif
{
  char *content = NULL;
  int CLcount, ContentLen, NumberParams;
  char *filename=DEFAULTCONFIGFILENAME;

#if EXT3D
  if(is_depth)
    filename=DEFAULTDEPTHCONFIGFILENAME;
#endif

  if (ac==2)
  {
    if (0 == strncmp (av[1], "-v", 2))
    {
      printf("JM-" VERSION "\n");
      exit(0);
    }
    if (0 == strncmp (av[1], "-V", 2))
    {
      printf("JM " JM ": compiled " __DATE__ " " __TIME__ "\n");
#if ( IMGTYPE == 0 )
      printf("support for more than 8 bits/pel disabled\n");
#endif
#if ( ENABLE_FIELD_CTX == 0 )
      printf("CABAC field coding disabled\n");
#endif
#if ( ENABLE_HIGH444_CTX == 0 )
      printf("CABAC High 4:4:4 profile coding disabled\n");
#endif
      exit(0);
    }

    if (0 == strncmp (av[1], "-h", 2))
    {
      JMHelpExit();
    }
  }


#if EXT3D
  if(!is_depth)
  {
    //for texture
    memset (&cfgparams, 0, sizeof (InputParameters));
    //Set default parameters.
    printf ("Setting Default Parameters...\n");
    InitParams(Map);
  }
  else
  {
    //for depth
    memset (&cfgparamsDepth, 0, sizeof (InputParameters));
    //Set default parameters.
    printf ("Setting Default Parameters...\n");
    InitParams(MapDepth);
  }
#else
  memset (&cfgparams, 0, sizeof (InputParameters));
  //Set default parameters.
  printf ("Setting Default Parameters...\n");
  InitParams(Map);
#endif

  // Process default config file
  CLcount = 1;
#if EXT3D
  if(!is_depth)
  {
    if(ac>=3)
    {
      if (0 == strncmp (av[1], "-d", 2))
      {
        filename=av[2];
        CLcount = 3;
        if((ac>=5)&&(0 == strncmp (av[3], "-depd", 5)))
          CLcount=5;
      }
      if (0 == strncmp (av[1], "-h", 2))
      {
        JMHelpExit();
      }
    }
  }
  else
  {
    //!<depth
    if(ac>=5)
    {
      if(0 == strncmp (av[3], "-depd", 5))
      {
        filename=av[4];
        CLcount=5;
      }
    }
    else if(ac>=3)
    {
      if(0 == strncmp (av[1], "-d", 2))
      {
        printf("The config file for depth coding is not presented.\n")  ;
        printf("Default config file for depth(encoder_depth.cfg) is enabled");
        CLcount=3;
      }
    }
  }
#else
  if (ac>=3)
  {
    if (0 == strncmp (av[1], "-d", 2))
    {
      filename=av[2];
      CLcount = 3;
    }
    if (0 == strncmp (av[1], "-h", 2))
    {
      JMHelpExit();
    }
  }
#endif
  printf ("Parsing Configfile %s", filename);
  content = GetConfigFileContent (filename);
  if (NULL==content)
    error (errortext, 300);
#if EXT3D
  ParseContent (p_Inp,is_depth?MapDepth: Map, content, (int) strlen(content),is_depth);
#else
  ParseContent (p_Inp, Map, content, (int) strlen(content));
#endif
  printf ("\n");
  free (content);

  // Parse the command line

  while (CLcount < ac)
  {
    if (0 == strncmp (av[CLcount], "-h", 2))
    {
      JMHelpExit();
    }

    if (0 == strncmp (av[CLcount], "-f", 2) || 0 == strncmp (av[CLcount], "-F", 2))  // A file parameter?
    {
#if EXT3D
      if(is_depth)
      {
        CLcount+=2;
        continue;
      }
#endif
      content = GetConfigFileContent (av[CLcount+1]);
      if (NULL==content)
        error (errortext, 300);
      printf ("Parsing Configfile %s", av[CLcount+1]);
#if EXT3D
      ParseContent (p_Inp, Map, content, (int) strlen (content),is_depth);
#else
      ParseContent (p_Inp, Map, content, (int) strlen (content));
#endif
      printf ("\n");
      free (content);
      CLcount += 2;
    } 
#if EXT3D
    else if(0 == strncmp (av[CLcount], "-depf", 5) || 0 == strncmp (av[CLcount], "-DEPF", 5))
    {
      if(!is_depth)
      {
        CLcount+=2;
        continue;
      }
      content = GetConfigFileContent (av[CLcount+1]);
      if (NULL==content)
        error (errortext, 300);
      printf ("Parsing Configfile %s", av[CLcount+1]);

      ParseContent (p_Inp, MapDepth, content, (int) strlen (content),is_depth);
      printf ("\n");
      free (content);
      CLcount += 2;
    } 
#endif
    else
    {
      if (0 == strncmp (av[CLcount], "-p", 2) || 0 == strncmp (av[CLcount], "-P", 2))  // A config change?
      {
        // Collect all data until next parameter (starting with -<x> (x is any character)),
        // put it into content, and parse content.
        ++CLcount;
        ContentLen = 0;
        NumberParams = CLcount;

        // determine the necessary size for content
        while (NumberParams < ac && av[NumberParams][0] != '-')
          ContentLen += (int) strlen (av[NumberParams++]);        // Space for all the strings
        ContentLen += 1000;                     // Additional 1000 bytes for spaces and \0s
#if EXT3D
        if(is_depth)
        {
          CLcount=NumberParams;
          continue;
        }
#endif

        if ((content = malloc (ContentLen))==NULL) no_mem_exit("Configure: content");;
        content[0] = '\0';

        // concatenate all parameters identified before

        while (CLcount < NumberParams)
        {
          char *source = &av[CLcount][0];
          char *destin = &content[(int) strlen (content)];

          while (*source != '\0')
          {
            if (*source == '=')  // The Parser expects whitespace before and after '='
            {
              *destin++=' '; *destin++='='; *destin++=' ';  // Hence make sure we add it
            } 
            else
              *destin++=*source;
            source++;
          }
          *destin = '\0';
          CLcount++;
        }
        printf ("Parsing command line string '%s'", content);
#if EXT3D
        ParseContent (p_Inp, Map, content, (int) strlen(content),is_depth);
#else
        ParseContent (p_Inp, Map, content, (int) strlen(content));
#endif
        free (content);
        printf ("\n");
      }
#if EXT3D
      else if(0 == strncmp (av[CLcount], "-depp", 5) || 0 == strncmp (av[CLcount], "-DEPP", 5))
      {
        // Collect all data until next parameter (starting with -<x> (x is any character)),
        // put it into content, and parse content.
        ++CLcount;
        ContentLen = 0;
        NumberParams = CLcount;

        // determine the necessary size for content
        while (NumberParams < ac && av[NumberParams][0] != '-')
          ContentLen += (int) strlen (av[NumberParams++]);        // Space for all the strings
        ContentLen += 1000;                     // Additional 1000 bytes for spaces and \0s
        if(!is_depth)
        {
          CLcount=NumberParams;
          continue;
        }

        if ((content = malloc (ContentLen))==NULL) 
          no_mem_exit("Configure: content");;
        content[0] = '\0';

        // concatenate all parameters identified before

        while (CLcount < NumberParams)
        {
          char *source = &av[CLcount][0];
          char *destin = &content[(int) strlen (content)];

          while (*source != '\0')
          {
            if (*source == '=')  // The Parser expects whitespace before and after '='
            {
              *destin++=' '; *destin++='='; *destin++=' ';  // Hence make sure we add it
            } 
            else
              *destin++=*source;
            source++;
          }
          *destin = '\0';
          CLcount++;
        }
        printf ("Parsing command line string '%s'", content);
        ParseContent (p_Inp, MapDepth, content, (int) strlen(content),is_depth);
        free (content);
        printf ("\n");
      }
#endif
      else
      {
        snprintf (errortext, ET_SIZE, "Error in command line, ac %d, around string '%s', missing -f or -p parameters?", CLcount, av[CLcount]);
        error (errortext, 300);
      }
    }
  }
  printf ("\n");

#if EXT3D
  if(p_Inp->yuv_format == 0)
  {
    p_Inp->force_yuv400 = 0;
  }
  if (p_Inp->force_yuv400 == 1)
  {
    p_Inp->yuv_format = 0;
  }  

  if(is_depth)
    CopyConfigFromTexture(p_Vid,p_Inp);

  if((strlen(p_Inp->ThreeDVConfigFileName)!=0)&&(p_Inp->NumOfViews>1))
  {
    ParseMVCConfigFile(p_Inp, is_depth);
  }

  if(p_Inp->ReferenceDisplayInfoFlag)
  {
    if(strlen(p_Inp->ReferenceDisplayFile)!=0)
      ParseRefDisplayConfigFile(p_Inp);
    else
      p_Inp->ReferenceDisplayInfoFlag = 0;
  }

  if (is_depth)
  {
    int SubWidthC  [4]= { 1, 2, 2, 1};
    int SubHeightC [4]= { 1, 2, 1, 1};
    int depth_w,depth_h;
    int text_w,text_h;
    int mb_only_flag;
    if (p_Inp->PicInterlace || p_Inp->MbInterlace)
    {
      mb_only_flag=0;
    }else
    {
      mb_only_flag=1;
    }
    
    
    text_w=p_Vid->p_DualInp->output.width[0];
    text_h=p_Vid->p_DualInp->output.height[0];
    depth_w=p_Inp->output.width[0];
    depth_h=p_Inp->output.height[0];

    if ((text_w & 0x0F) !=0)
      p_Vid->text_width_padded=text_w+16-(text_w & 0x0F);
    else
      p_Vid->text_width_padded=text_w;

    if ((text_h & 0x0F) !=0)
       p_Vid->text_height_padded=text_h+16-(text_h & 0x0F);
    else
      p_Vid->text_height_padded=text_h;

    if ((depth_w & 0x0F) !=0)
      p_Vid->depth_width_padded=depth_w+16-(depth_w & 0x0F);
    else
      p_Vid->depth_width_padded=depth_w;

    if ((depth_h & 0x0F) !=0)
      p_Vid->depth_height_padded=depth_h+16-(depth_h & 0x0F);
    else
     p_Vid->depth_height_padded=depth_h;

    p_Vid->depth_width=depth_w;
    p_Vid->depth_height=depth_h;
    p_Vid->text_height=text_h;
    p_Vid->text_width=text_w;


    p_Vid->depth_hor_mult=p_Vid->p_DualVid->depth_hor_mult=p_Inp->depth_hor_mult_minus1+1;
    p_Vid->depth_ver_mult=p_Vid->p_DualVid->depth_ver_mult=p_Inp->depth_ver_mult_minus1+1;
    p_Vid->depth_hor_rsh=p_Vid->p_DualVid->depth_hor_rsh=p_Inp->depth_hor_rsh;
    p_Vid->depth_ver_rsh=p_Vid->p_DualVid->depth_ver_rsh=p_Inp->depth_ver_rsh;
    
    if (p_Inp->depth_frame_cropping_flag)
    {
      p_Vid->DepthCropLeftCoord = p_Vid->p_DualVid->DepthCropLeftCoord = p_Inp->depth_frame_crop_left_offset * SubWidthC[p_Inp->yuv_format];
      p_Vid->DepthCropRightCoord = p_Vid->p_DualVid->DepthCropRightCoord = depth_w - 1 - p_Inp->depth_frame_crop_right_offset * SubWidthC[p_Inp->yuv_format];
      p_Vid->DepthCropTopCoord = p_Vid->p_DualVid->DepthCropTopCoord = p_Inp->depth_frame_crop_top_offset * SubHeightC[p_Inp->yuv_format] * (2 - mb_only_flag);
      p_Vid->DepthCropBottomCoord = p_Vid->p_DualVid->DepthCropBottomCoord = depth_h - 1- p_Inp->depth_frame_crop_bottom_offset * SubHeightC[p_Inp->yuv_format] * (2 - mb_only_flag);
    }else
    {
      p_Vid->DepthCropLeftCoord = p_Vid->p_DualVid->DepthCropLeftCoord  = 0;
      p_Vid->DepthCropTopCoord = p_Vid->p_DualVid->DepthCropTopCoord = 0;
      p_Vid->DepthCropRightCoord = p_Vid->p_DualVid->DepthCropRightCoord =depth_w - 1 ;
      p_Vid->DepthCropBottomCoord = p_Vid->p_DualVid->DepthCropBottomCoord = depth_h - 1;

    }

    memcpy(p_Vid->grid_pos_x,p_Inp->grid_pos_x,sizeof(p_Inp->grid_pos_x));
    memcpy(p_Vid->p_DualVid->grid_pos_x,p_Inp->grid_pos_x,sizeof(p_Inp->grid_pos_x));

    memcpy(p_Vid->grid_pos_y,p_Inp->grid_pos_y,sizeof(p_Inp->grid_pos_y));
    memcpy(p_Vid->p_DualVid->grid_pos_y,p_Inp->grid_pos_y,sizeof(p_Inp->grid_pos_y));

  }

  PatchInp(p_Vid,p_Inp,is_depth);
  if(is_depth)
    cfgparamsDepth=*p_Inp;
  else
    cfgparams=*p_Inp;
#else
  PatchInp(p_Vid, p_Inp);
  cfgparams = *p_Inp;
#endif

#if EXT3D
  if ( p_Inp->VSD )
  {
    int n = 0;
    for ( n = 0 ; n < MAX_VIEWS ; n++ )  p_Inp->posCam[n] = 0;

    if ( strlen(p_Inp->VSDCfg) > 0 )
    {
      char *pos = p_Inp->VSDCfg;
      int numView = 0, numSynView = 0;
      double posView[MAX_SYN_VIEWS];
      char c;
      float fp;

      for ( ; *pos == '[' ; pos++ ) ;

      for ( ; *pos != ';' ; pos++ )
      {
        if ( *pos == ' ' )  continue;

        if ( sscanf( pos, "%c", &c ) )
        {
          if ( c == 'C' || c == 'c' ) 
          {
            p_Inp->posCam[numView++] = 0;
          }
          else if ( c == 'L' || c == 'l' ) 
          {
            p_Inp->posCam[numView++] = 1;
          }
          else if ( c == 'R' || c == 'r' ) 
          {
            p_Inp->posCam[numView++] = 2;
          }
        }
      }

      for ( pos++ ; *pos != ']' ; pos++ )
      {
        if ( *pos == ' ' )  continue;

        if ( sscanf( pos, "%f", &fp ) )
        {
          if ( fp > 0  &&  fp < 1 ) 
            posView[numSynView++] = fp;

          for ( ; *pos != ' ' ; pos++ ) ;
        }
      }

      if ( numSynView == 0 )
      {
        printf( "ViewSynCfg is incorrect \n" );
        exit(1);
      }

      p_Inp->dispCoeffSynPos = 0.0;
      for ( n = 0 ; n < numSynView ; n++ )
      {
        p_Inp->dispCoeffSynPos += ( 1.0 - posView[n] ) * posView[n] * posView[n] * 4.0;
      }
      p_Inp->dispCoeffSynPos = sqrt( (p_Inp->dispCoeffSynPos*2.0) / numSynView );

    }
    else
    {
      p_Inp->dispCoeffSynPos = 1.0;
    }

    //printf (" dispCoeffSynPos : %f \n", p_Inp->dispCoeffSynPos );
  }

  if(is_depth && p_Inp->ProfileIDC ==ThreeDV_EXTEND_HIGH)
  {
    p_Inp->NonlinearDepthNum = 0;
    p_Inp->NonlinearDepthPoints[0] = 0; // beginning
    if(is_depth)
    {
      int i;
      if (p_Inp->NonlinearDepthCfg)
      {
        FILE *base_depth_file;
        unsigned char *depth_buf;
        int histogram[256];
        size_t size;
        float weighted_avg;
        base_depth_file = fopen(p_Inp->InputFile[0].fname, "rb");
        if (base_depth_file)
        {
          size = p_Inp->source.width[0]*p_Inp->source.height[0];
          depth_buf = (unsigned char *)malloc(size);
          if (size != fread(depth_buf, 1, size, base_depth_file))
          {
            fclose(base_depth_file);
            return;
          }
          fclose(base_depth_file);
          memset(histogram, 0, sizeof(histogram));
          for (i=0; i<(int)size;++i) histogram[depth_buf[i]]++;
          weighted_avg = 0;
          for (i=0; i<256; ++i) weighted_avg += (float)i*(float)histogram[i];
          weighted_avg /= size;
        } else 
          printf ("\nCannot compute weighted average of depth : leaving NonlinearDepthRepresentation ON\n");
      }

      if (p_Inp->NonlinearDepthCfg)
      {
        if (strlen(p_Inp->NonlinearDepthModelCfg)>0)
        {

          char *start = p_Inp->NonlinearDepthModelCfg;
          char *pos = p_Inp->NonlinearDepthModelCfg;
          for(;; pos++)
          {
            if ((*pos==';') || (*pos==' ') || (*pos==0))
            {
              int v = 0;
              int r = sscanf(start, " %d", &v);
              if (r==1) 
              {
                if (v<0) v=0;
                if (v>255) v=255;
                ++p_Inp->NonlinearDepthNum;
                p_Inp->NonlinearDepthPoints[p_Inp->NonlinearDepthNum] = (char)v;
              }
              start = pos+1;
            }
            if (*pos==0) break;
          }
        }
      }
    }
    p_Inp->NonlinearDepthPoints[p_Inp->NonlinearDepthNum+1] = 0; // end
    p_Vid->NonlinearDepthNum = p_Inp->NonlinearDepthNum;
    memcpy(p_Vid->NonlinearDepthPoints, p_Inp->NonlinearDepthPoints, sizeof(p_Inp->NonlinearDepthPoints));
  }

  if(is_depth && p_Inp->ProfileIDC !=ThreeDV_EXTEND_HIGH)
  {
    p_Inp->NonlinearDepthNum = 0;
    p_Inp->NonlinearDepthPoints[0] = 0; // beginning
    if(is_depth)
    {
      int i;
      if (p_Inp->NonlinearDepthCfg && (p_Inp->NonlinearDepthThresholdCfg>0))
      {
        FILE *base_depth_file;
        unsigned char *depth_buf;
        int histogram[256];
        size_t size;
        float weighted_avg;
        base_depth_file = fopen(p_Inp->InputFile[0].fname, "rb");
        if (base_depth_file)
        {
          size = p_Inp->source.width[0]*p_Inp->source.height[0];
          depth_buf = (unsigned char *)malloc(size);
          if (size != fread(depth_buf, 1, size, base_depth_file))
          {
            fclose(base_depth_file);
            return;
          }
          fclose(base_depth_file);
          memset(histogram, 0, sizeof(histogram));
          for (i=0; i<(int)size;++i) histogram[depth_buf[i]]++;
          weighted_avg = 0;
          for (i=0; i<256; ++i) weighted_avg += (float)i*(float)histogram[i];
          weighted_avg /= size;

          if (weighted_avg<p_Inp->NonlinearDepthThresholdCfg)
          {
            p_Inp->NonlinearDepthCfg = 0;
            printf ("\nWeighted average of depth histogram:%f < %d, turning NonlinearDepthRepresentation OFF\n", weighted_avg, p_Inp->NonlinearDepthThresholdCfg);
          } else printf ("\nWeighted average of depth histogram:%f >= %d, leaving NonlinearDepthRepresentation ON\n", weighted_avg, p_Inp->NonlinearDepthThresholdCfg);
        } else printf ("\nCannot compute weighted average of depth : leaving NonlinearDepthRepresentation ON\n");
      }

      if (p_Inp->NonlinearDepthCfg)
      {
        if (strlen(p_Inp->NonlinearDepthModelCfg)>0)
        {

          char *start = p_Inp->NonlinearDepthModelCfg;
          char *pos = p_Inp->NonlinearDepthModelCfg;
          for(;; pos++)
          {
            if ((*pos==';') || (*pos==' ') || (*pos==0))
            {
              int v = 0;
              int r = sscanf(start, " %d", &v);
              if (r==1) 
              {
                if (v<0) v=0;
                if (v>255) v=255;
                ++p_Inp->NonlinearDepthNum;
                p_Inp->NonlinearDepthPoints[p_Inp->NonlinearDepthNum] = (char)v;
              }
              start = pos+1;
            }
            if (*pos==0) break;
          }
        }
        printf ("\nNonlinear depth model (num:%d): ", (int)p_Inp->NonlinearDepthNum);
        for (i=1; i<=p_Inp->NonlinearDepthNum; ++i)
          printf ("%d ", (int)p_Inp->NonlinearDepthPoints[i]);
        printf ("\n");

        if (p_Inp->ProfileIDC==ThreeDV_HIGH)
        {
          p_Inp->DepthRepresentationInfoFlag = 1;      
          printf ("\nAuto-enabling DepthRepresentationInfoFlag\n");
        };
      }
    }
    p_Inp->NonlinearDepthPoints[p_Inp->NonlinearDepthNum+1] = 0; // end
    p_Vid->NonlinearDepthNum = p_Inp->NonlinearDepthNum;
    memcpy(p_Vid->NonlinearDepthPoints, p_Inp->NonlinearDepthPoints, sizeof(p_Inp->NonlinearDepthPoints));
  }

  if(p_Inp->DisplayEncParams)
  {
    if(!is_depth)
      DisplayEncoderParams(Map);
    else
      DisplayEncoderParams(MapDepth);
  }
  p_Vid->is_depth=is_depth;
#else
  if (p_Inp->DisplayEncParams)
    DisplayEncoderParams(Map);
#endif
}




/*!
 ***********************************************************************
 * \brief
 *    Validates encoding parameters.
 * \return
 *    -1 for error
 ***********************************************************************
 */
static int TestEncoderParams(Mapping *Map, int bitdepth_qp_scale[3])
{
  int i = 0;

  while (Map[i].TokenName != NULL)
  {
    if (Map[i].param_limits == 1)
    {
      if (Map[i].Type == 0)
      {
        if ( * (int *) (Map[i].Place) < (int) Map[i].min_limit || * (int *) (Map[i].Place) > (int) Map[i].max_limit )
        {
          snprintf(errortext, ET_SIZE, "Error in input parameter %s. Check configuration file. Value should be in [%d, %d] range.", Map[i].TokenName, (int) Map[i].min_limit,(int)Map[i].max_limit );
          error (errortext, 400);
        }

      }
      else if (Map[i].Type == 2)
      {
        if ( * (double *) (Map[i].Place) < Map[i].min_limit || * (double *) (Map[i].Place) > Map[i].max_limit )
        {
          snprintf(errortext, ET_SIZE, "Error in input parameter %s. Check configuration file. Value should be in [%.2f, %.2f] range.", Map[i].TokenName,Map[i].min_limit ,Map[i].max_limit );
          error (errortext, 400);
        }
      }
    }
    else if (Map[i].param_limits == 2)
    {
      if (Map[i].Type == 0)
      {
        if ( * (int *) (Map[i].Place) < (int) Map[i].min_limit )
        {
          snprintf(errortext, ET_SIZE, "Error in input parameter %s. Check configuration file. Value should not be smaller than %d.", Map[i].TokenName, (int) Map[i].min_limit);
          error (errortext, 400);
        }
      }
      else if (Map[i].Type == 2)
      {
        if ( * (double *) (Map[i].Place) < Map[i].min_limit )
        {
          snprintf(errortext, ET_SIZE, "Error in input parameter %s. Check configuration file. Value should not be smaller than %2.f.", Map[i].TokenName,Map[i].min_limit);
          error (errortext, 400);
        }
      }
    }
    else if (Map[i].param_limits == 3) // Only used for QPs
    {

      if (Map[i].Type == 0)
      {
        int cur_qp = * (int *) (Map[i].Place);
        int min_qp = (int) (Map[i].min_limit - bitdepth_qp_scale[0]);
        int max_qp = (int) Map[i].max_limit;

        if (( cur_qp < min_qp ) || ( cur_qp > max_qp ))
        {
          snprintf(errortext, ET_SIZE, "Error in input parameter %s. Check configuration file. Value should be in [%d, %d] range.", Map[i].TokenName, min_qp, max_qp );
          error (errortext, 400);
        }
      }
    }

    i++;
  }
  return -1;
}



/*!
 ***********************************************************************
 * \brief
 *    Outputs encoding parameters.
 * \return
 *    -1 for error
 ***********************************************************************
 */
static int DisplayEncoderParams(Mapping *Map)
{
  int i = 0;

  printf("******************************************************\n");
  printf("*               Encoder Parameters                   *\n");
  printf("******************************************************\n");
  while (Map[i].TokenName != NULL)
  {
    if (Map[i].Type == 0)
      printf("Parameter %s = %d\n",Map[i].TokenName,* (int *) (Map[i].Place));
    else if (Map[i].Type == 1)
      printf("Parameter %s = ""%s""\n",Map[i].TokenName,(char *)  (Map[i].Place));
    else if (Map[i].Type == 2)
      printf("Parameter %s = %.2f\n",Map[i].TokenName,* (double *) (Map[i].Place));
      i++;
  }
  printf("******************************************************\n");
  return -1;
}

/*!
 ************************************************************************
 * \brief
 *    read the slice group configuration file. Returns without action
 *    if type is not 0, 2 or 6
 ************************************************************************
 */
void read_slice_group_info(VideoParameters *p_Vid, InputParameters *p_Inp)
{
  FILE * sgfile=NULL;
  int i;
  int ret;
  unsigned int PicSizeInMapUnits;

  if ((p_Inp->slice_group_map_type != 0) && (p_Inp->slice_group_map_type != 2) && (p_Inp->slice_group_map_type != 6))
  {
    // nothing to do
    return;
  }

  // do we have a file name (not only NULL character)
  if ((int) strlen (p_Inp->SliceGroupConfigFileName) <= 1)
    error ("No slice group config file name specified", 500);

  // open file
  sgfile = fopen(p_Inp->SliceGroupConfigFileName,"r");

  if ( NULL==sgfile )
  {
    snprintf(errortext, ET_SIZE, "Error opening slice group file %s", p_Inp->SliceGroupConfigFileName);
    error (errortext, 500);
  }

  switch (p_Inp->slice_group_map_type)
  {
  case 0:
    p_Inp->run_length_minus1=(int *)malloc(sizeof(int)*(p_Inp->num_slice_groups_minus1+1));
    if ( NULL==p_Inp->run_length_minus1 )
    {
      fclose(sgfile);
      no_mem_exit("read_slice_group_info: p_Inp->run_length_minus1");
    }

    // each line contains one 'run_length_minus1' value
    for(i=0; i <= p_Inp->num_slice_groups_minus1;i++)
    {
      ret  = fscanf(sgfile,"%d",(p_Inp->run_length_minus1+i));
      if ( 1!=ret )
      {
        fclose(sgfile);
        snprintf(errortext, ET_SIZE, "Error while reading slice group config file (line %d)", i+1);
        error (errortext, 500);
      }
      // scan remaining line
      ret = fscanf(sgfile,"%*[^\n]");
    }
    break;

  case 2:
    // determine expected frame size in map units
    PicSizeInMapUnits = (p_Inp->output.width[0] >> 4) * (p_Inp->output.height[0] >> 4);
    if (p_Inp->MbInterlace||p_Inp->PicInterlace) 
      PicSizeInMapUnits >>= 1;

    p_Inp->top_left     = (unsigned *)malloc(sizeof(unsigned)*p_Inp->num_slice_groups_minus1);
    p_Inp->bottom_right = (unsigned *)malloc(sizeof(unsigned)*p_Inp->num_slice_groups_minus1);

    if (NULL==p_Inp->top_left)
    {
      fclose(sgfile);
      no_mem_exit("PatchInp: p_Inp->top_left");
    }

    if (NULL==p_Inp->bottom_right)
    {
      fclose(sgfile);
      no_mem_exit("PatchInp: p_Inp->bottom_right");
    }

    // every two lines contain 'top_left' and 'bottom_right' value
    for(i=0;i<p_Inp->num_slice_groups_minus1;i++)
    {
      ret = fscanf(sgfile,"%ud",(p_Inp->top_left+i));
      if ( 1!=ret )
      {
        fclose(sgfile);
        snprintf(errortext, ET_SIZE, "Error while reading slice group config file (line %d)", 2*i +1);
        error (errortext, 500);
      }
      if (p_Inp->top_left[i] > PicSizeInMapUnits)
      {
        fprintf(stderr, "Warning: slice group # %d top_left exceeds picture size (will be clipped)\n", i);
      }
      // scan remaining line
      ret = fscanf(sgfile,"%*[^\n]");
      ret = fscanf(sgfile,"%ud",(p_Inp->bottom_right+i));
      if ( 1!=ret )
      {
        fclose(sgfile);
        snprintf(errortext, ET_SIZE, "Error while reading slice group config file (line %d)", 2*i + 2);
        error (errortext, 500);
      }
      if (p_Inp->bottom_right[i] > PicSizeInMapUnits)
      {
        fprintf(stderr, "Warning: slice group # %d bottom_right exceeds picture size (will be clipped)\n", i);
      }
      // scan remaining line
      ret = fscanf(sgfile,"%*[^\n]");
    }
    break;
  case 6:
    {
      int tmp;
      int frame_mb_only;
      int mb_width, mb_height, mapunit_height;

      frame_mb_only = !(p_Inp->PicInterlace || p_Inp->MbInterlace);
      mb_width  = (p_Inp->output.width[0] + p_Vid->auto_crop_right)>>4;
      mb_height = (p_Inp->output.height[0] + p_Vid->auto_crop_bottom)>>4;
      mapunit_height = mb_height / (2-frame_mb_only);

      p_Inp->slice_group_id=(byte * ) malloc(sizeof(byte)*mapunit_height*mb_width);
      if (NULL==p_Inp->slice_group_id)
      {
        fclose(sgfile);
        no_mem_exit("PatchInp: p_Inp->slice_group_id");
      }

      // each line contains slice_group_id for one Macroblock
      for (i=0;i<mapunit_height*mb_width;i++)
      {
        ret = fscanf(sgfile,"%d", &tmp);
        p_Inp->slice_group_id[i]= (byte) tmp;
        if ( 1!=ret )
        {
          fclose(sgfile);
          snprintf(errortext, ET_SIZE, "Error while reading slice group config file (line %d)", i + 1);
          error (errortext, 500);
        }
        if ( *(p_Inp->slice_group_id+i) > p_Inp->num_slice_groups_minus1 )
        {
          fclose(sgfile);
          snprintf(errortext, ET_SIZE, "Error while reading slice group config file: slice_group_id not allowed (line %d)", i + 1);
          error (errortext, 500);
        }
        // scan remaining line
        ret = fscanf(sgfile,"%*[^\n]");
      }
    }
    break;
  default:
    // we should not get here
    error ("Wrong slice group type while reading config file", 500);
    break;
  }

  // close file again
  fclose(sgfile);
}


#if EXT3D
static void CopyConfigFromTexture(VideoParameters* p_Vid, InputParameters* p_Inp)
{
  InputParameters* p_TextInp=p_Vid->p_DualInp;

  p_Inp->source.frame_rate=p_TextInp->source.frame_rate;
#if ITRI_INTERLACE
  if (p_Inp->ProfileIDC==ThreeDV_EXTEND_HIGH)
#endif
    p_Inp->LevelIDC=p_TextInp->LevelIDC;
  p_Inp->start_frame=p_TextInp->start_frame;
  p_Inp->intra_period=p_TextInp->intra_period;
  p_Inp->idr_period=p_TextInp->idr_period;
  p_Inp->adaptive_idr_period=p_TextInp->adaptive_idr_period;
  p_Inp->adaptive_intra_period=p_TextInp->adaptive_intra_period;
  p_Inp->EnableIDRGOP=p_TextInp->EnableIDRGOP;
  p_Inp->EnableOpenGOP=p_TextInp->EnableOpenGOP;
  p_Inp->no_frames=p_TextInp->no_frames;
  p_Inp->frame_skip=p_TextInp->frame_skip;

  p_Inp->OriginalHeight=p_TextInp->OriginalHeight;
  p_Inp->OriginalWidth=p_TextInp->OriginalWidth;

  p_Inp->Log2MaxFNumMinus4=p_TextInp->Log2MaxFNumMinus4;
  p_Inp->Log2MaxPOCLsbMinus4=p_TextInp->Log2MaxPOCLsbMinus4;
#if ITRI_INTERLACE
  if (p_Inp->ProfileIDC==ThreeDV_EXTEND_HIGH)
#endif
    p_Inp->num_ref_frames=p_TextInp->num_ref_frames;
  p_Inp->NumberBFrames=p_TextInp->NumberBFrames;
  p_Inp->HierarchicalCoding=p_TextInp->HierarchicalCoding;
  strcpy(p_Inp->ExplicitHierarchyFormat,p_TextInp->ExplicitHierarchyFormat);
}

static void PatchInp (VideoParameters *p_Vid, InputParameters *p_Inp,int is_depth)
{
  int i;
  //int storedBplus1;
  int bitdepth_qp_scale[3];

  if (p_Inp->src_BitDepthRescale)
  {
    bitdepth_qp_scale [0] = 6*(p_Inp->output.bit_depth[0] - 8);
    bitdepth_qp_scale [1] = 6*(p_Inp->output.bit_depth[1] - 8);
    bitdepth_qp_scale [2] = 6*(p_Inp->output.bit_depth[2] - 8);
  }
  else
  {
    bitdepth_qp_scale [0] = 6*(p_Inp->source.bit_depth[0] - 8);
    bitdepth_qp_scale [1] = 6*(p_Inp->source.bit_depth[1] - 8);
    bitdepth_qp_scale [2] = 6*(p_Inp->source.bit_depth[2] - 8);
  }
  if(!is_depth)
    TestEncoderParams(Map, bitdepth_qp_scale);
  else
    TestEncoderParams(MapDepth,bitdepth_qp_scale)  ;

  if (p_Inp->source.frame_rate == 0.0)
    p_Inp->source.frame_rate = (double) INIT_FRAME_RATE;



  for(i=0;i<p_Inp->NumOfViews;++i)
  {
    //  strncpy(p_Inp->InputFile[i].fname,p_Inp->InputFileMVC[i],FILE_NAME_SIZE);
    ParseVideoType(&(p_Inp->InputFile[i]))   ;
    ParseFrameNoFormatFromString(&(p_Inp->InputFile[i]));
  }


  // Read resolution from file name
  if (p_Inp->source.width[0] == 0 || p_Inp->source.height[0] == 0)
  {
    if (ParseSizeFromString (&(p_Inp->InputFile[0]), &(p_Inp->source.width[0]), &(p_Inp->source.height[0]), &(p_Inp->source.frame_rate)) == 0)
    {
      snprintf(errortext, ET_SIZE, "File name does not contain resolution information.");    
      error (errortext, 500);
    }
  }

#if (!ENABLE_FIELD_CTX)
  if ( (p_Inp->PicInterlace || p_Inp->MbInterlace) && p_Inp->symbol_mode )
  {
    snprintf(errortext, ET_SIZE, "Recompile with ENABLE_FIELD_CTX set to one to enable interlaced coding with CABAC.");    
    error (errortext, 500);
  }
#endif
#if (!ENABLE_HIGH444_CTX)
  if ( p_Inp->ProfileIDC == 244 && p_Inp->symbol_mode )
  {
    snprintf(errortext, ET_SIZE, "Recompile with ENABLE_HIGH444_CTX set to one to enable the High 4:4:4 Profile with CABAC.");    
    error (errortext, 500);
  }
#endif

  if(is_depth)
  {
    if(p_Inp->AcquisitionIdx==0)
    {
      if((p_Vid->p_DualInp->DepthBasedMVP))
      {
        snprintf(errortext,ET_SIZE,"Without camera and depth range information,Depth based MVP cannot be opened");
        error(errortext,500);
      }
    }

    if (is_depth)
    {
      int top,bot,left,right;

      if (p_Vid->depth_hor_rsh<0 || p_Vid->depth_ver_rsh<0 || p_Vid->depth_hor_mult<=0 || p_Vid->depth_hor_mult<=0)
      {
        error("invalid depth RatioX or RatioY",500);
      }

      left=(p_Vid->DepthCropLeftCoord<<p_Vid->depth_hor_rsh)/p_Vid->depth_hor_mult;
      right=( (1+p_Vid->DepthCropRightCoord)<<p_Vid->depth_hor_rsh)/p_Vid->depth_hor_mult-p_Vid->text_width;
      top=(p_Vid->DepthCropTopCoord<<p_Vid->depth_ver_rsh)/p_Vid->depth_ver_mult;
      bot=( (1+p_Vid->DepthCropBottomCoord)<<p_Vid->depth_ver_rsh)/p_Vid->depth_ver_mult-p_Vid->text_height;

      for(i=0;i<p_Inp->NumOfViews;i++)
      {
        int viewid;
        viewid=p_Inp->ViewCodingOrder[i];
        if (left<p_Vid->grid_pos_x[viewid] || right>p_Vid->grid_pos_x[viewid])
        {
          snprintf(errortext,ET_SIZE,"invalid cropping  rectangle or invalid GridPosX of view %d",viewid);
          error(errortext,500);
        }
        if(top<p_Vid->grid_pos_y[viewid] || bot>p_Vid->grid_pos_y[viewid])
        {
          snprintf(errortext,ET_SIZE,"invalid cropping  rectangle or invalid GridPosY of view %d",viewid);
          error(errortext,500);
        }
      }
      //if (p_Vid->depth_hor_rsh==-1 || p_Vid->depth_ver_rsh==-1)
      //{
      //  error("the depth resolution is not supported\n",500);
      //}
      if ( (p_Vid->DepthCropRightCoord+1-p_Vid->DepthCropLeftCoord) & 1)
      {
        snprintf(errortext,ET_SIZE,"the width of depth picture (after applying the cropping) is required to be a multiple of 2.Current width:%d",p_Vid->DepthCropRightCoord+1-p_Vid->DepthCropLeftCoord);
        error(errortext,500);
      }
    }
  }
  else
  {
    if(p_Inp->SliceHeaderPred&& !p_Inp->PredSliceHeaderSrc)
    {
      snprintf(errortext,ET_SIZE,"PredSliceHeaderSrc cannot be 0 when slice header prediction is enabled\n");
//      error(errortext,500);
    }
    if(p_Inp->ThreeDVCoding==0)
    {
      if(p_Inp->CompatibilityCategory)
      {
        snprintf(errortext,ET_SIZE,"3DV is disabled, please enable MVC-compatible\n");
        error(errortext,500);
      }
      if(p_Inp->DepthBasedMVP)
      {
        snprintf(errortext,ET_SIZE,"DepthBasedMVP cannot be applied when 3DV is disabled\n");
        error(errortext,500);
      }
      if(p_Inp->SliceHeaderPred&&(p_Inp->PredSliceHeaderSrc==2 || p_Inp->PredRefListsSrc==2 || p_Inp->PredDecRefPicMarkingSrc==2 || p_Inp->PredWeightTableSrc==2))
      {
        snprintf(errortext,ET_SIZE,"slice header prediction from same view cannot be applied\n");
        error(errortext,500);
      }
    }
  }

  // Currently to simplify things, lets copy everything (overwrites yuv_format)
  p_Inp->InputFile[0].format=p_Inp->source;

  if (p_Inp->idr_period && p_Inp->intra_delay && p_Inp->idr_period <= p_Inp->intra_delay)
  {
    snprintf(errortext, ET_SIZE, " IntraDelay cannot be larger than or equal to IDRPeriod.");
    error (errortext, 500);
  }

  // Let us set up p_Inp->jumpd from frame_skip and NumberBFrames
  p_Inp->jumpd = (p_Inp->NumberBFrames + 1) * (p_Inp->frame_skip + 1) - 1;

  updateOutFormat(p_Inp);

  if (p_Inp->no_frames == -1)
  {
    OpenFiles(&(p_Inp->InputFile[0]));
    get_number_of_frames (p_Inp, &(p_Inp->InputFile[0]));
    CloseFiles(&(p_Inp->InputFile[0]));
  }

  //storedBplus1 = (p_Inp->BRefPictures ) ? p_Inp->NumberBFrames + 1: 1;

  if (p_Inp->no_frames < 1)
  {      
    snprintf(errortext, ET_SIZE, "Not enough frames to encode (%d)", p_Inp->no_frames);
    error (errortext, 500);
  }

  // Direct Mode consistency check
  if(p_Inp->NumberBFrames && p_Inp->direct_spatial_mv_pred_flag != DIR_SPATIAL && p_Inp->direct_spatial_mv_pred_flag != DIR_TEMPORAL)
  {
    snprintf(errortext, ET_SIZE, "Unsupported direct mode=%d, use TEMPORAL=0 or SPATIAL=1", p_Inp->direct_spatial_mv_pred_flag);
    error (errortext, 400);
  }

  if (p_Inp->PicInterlace>0 || p_Inp->MbInterlace>0)
  {
    if (p_Inp->directInferenceFlag==0)
      printf("\nWarning: DirectInferenceFlag set to 1 due to interlace coding.");
    p_Inp->directInferenceFlag = 1;
  }

#if TRACE
  if(!is_depth)
  {
    if ((int) strlen (p_Inp->TraceFile) > 0 && (p_Enc->p_trace = fopen(p_Inp->TraceFile,"w"))==NULL)
    {
      snprintf(errortext, ET_SIZE, "Error open file %s", p_Inp->TraceFile);
      error (errortext, 500);
    }
  }
#endif

#if ITRI_INTERLACE
  if((p_Inp->NumOfViews>2)&&(p_Inp->PicInterlace!=0))
  {
    snprintf(errortext,ET_SIZE,"Unsupported more than two field or field/frame adaptive pictures in multi-view coding.");
    error(errortext,500);
  }
#else
  if((p_Inp->NumOfViews>1)&&(p_Inp->PicInterlace!=0))
  {
    snprintf(errortext,ET_SIZE,"Unsupported field or field/frame adaptive picture in multi-view coding.");
    error(errortext,500);
  }
#endif

  if((p_Inp->NumOfViews>1)&&(p_Inp->MbInterlace!=0))
  {
    snprintf(errortext,ET_SIZE,"Unsupported field or field/frame adaptive MB in multi-view coding." );
    error(errortext,500);
  }

  if ((p_Inp->slice_mode == 1)&&(p_Inp->MbInterlace != 0))
  {
    if ((p_Inp->slice_argument & 0x01)!=0)
    {
      fprintf ( stderr, "Warning: slice border within macroblock pair. ");
      if (p_Inp->slice_argument > 1)
      {
        p_Inp->slice_argument--;
      }
      else
      {
        p_Inp->slice_argument++;
      }
      fprintf ( stderr, "Using %d MBs per slice.\n", p_Inp->slice_argument);
    }
  }  

  if (p_Inp->WPMCPrecision && (p_Inp->RDPictureDecision != 1 || p_Inp->GenerateMultiplePPS != 1) )
  {
    snprintf(errortext, ET_SIZE, "WPMCPrecision requires both RDPictureDecision=1 and GenerateMultiplePPS=1.\n");
    error (errortext, 400);
  }
  if (p_Inp->WPMCPrecision && p_Inp->WPMCPrecFullRef && p_Inp->num_ref_frames < 16 )
  {
    p_Inp->num_ref_frames++;
    if ( p_Inp->P_List0_refs )
      p_Inp->P_List0_refs++;
    else
      p_Inp->P_List0_refs = p_Inp->num_ref_frames;
    if ( p_Inp->B_List0_refs )
      p_Inp->B_List0_refs++;
    else
      p_Inp->B_List0_refs = p_Inp->num_ref_frames;
    if ( p_Inp->B_List1_refs )
      p_Inp->B_List1_refs++;
    else
      p_Inp->B_List1_refs = p_Inp->num_ref_frames;
  }
  else if ( p_Inp->WPMCPrecision && p_Inp->WPMCPrecFullRef )
  {
    snprintf(errortext, ET_SIZE, "WPMCPrecFullRef requires NumberReferenceFrames < 16.\n");
    error (errortext, 400);
  }

  if ( p_Inp->WPMCPrecision && ( p_Inp->PicInterlace || p_Inp->MbInterlace ) )
  {
    snprintf(errortext, ET_SIZE, "WPMCPrecision does not work for PicInterlace > 0 or MbInterlace > 0.\n");
    error (errortext, 400);
  }

  if (p_Inp->PicInterlace) 
  {
    if (p_Inp->ReferenceReorder == 2)
    {
      snprintf(errortext, ET_SIZE, "ReferenceReorder = 2 is not supported with field encoding\n");
      error (errortext, 400);
    }
    if (p_Inp->PocMemoryManagement >= 2)
    {
      snprintf(errortext, ET_SIZE, "PocMemoryManagement >= 2 is not supported with field encoding\n");
      error (errortext, 400);
    }
#if ITRI_INTERLACE
    if ( (p_Inp->NumOfViews > 1) && (p_Inp->ReferenceReorder > 0) )
    {
      snprintf(errortext, ET_SIZE, "ReferenceReorder is not supported with more than one view for interlace coding mode.\n");
      error (errortext, 400);
    }

    if ( (p_Inp->NumOfViews > 1) && (p_Inp->PocMemoryManagement > 0) )
    {
      snprintf(errortext, ET_SIZE, "PocMemoryManagement is not supported with more than one view for interlace coding mode.\n");
      error (errortext, 400);
    }
#endif
  }

  if (p_Inp->ReferenceReorder && p_Inp->MbInterlace )
  {
    snprintf(errortext, ET_SIZE, "ReferenceReorder is not supported with MBAFF\n");
    error (errortext, 400);
  }

  if (p_Inp->SetFirstAsLongTerm && (p_Inp->ReferenceReorder == 1 || p_Inp->ReferenceReorder == 2))
  {
    printf("SetFirstAsLongTerm is set. ReferenceReorder is not supported and therefore disabled. \n");
    p_Inp->ReferenceReorder = 0;
  }

  if (p_Inp->PocMemoryManagement && p_Inp->MbInterlace )
  {
    snprintf(errortext, ET_SIZE, "PocMemoryManagement is not supported with MBAFF\n");
    error (errortext, 400);
  }

  if(p_Inp->MbInterlace && p_Inp->RDPictureDecision && p_Inp->GenerateMultiplePPS)
  {
    snprintf(errortext, ET_SIZE, "RDPictureDecision+GenerateMultiplePPS not supported with MBAFF. RDPictureDecision therefore disabled\n");
    p_Inp->RDPictureDecision = 0;
  }

  if ((!p_Inp->rdopt)&&(p_Inp->MbInterlace==2))
  {
    snprintf(errortext, ET_SIZE, "MB AFF is not compatible with non-rd-optimized coding.");
    error (errortext, 500);
  }

  // check RDoptimization mode and profile. FMD does not support Frex Profiles.
  if (p_Inp->rdopt==2 && ( p_Inp->ProfileIDC>=FREXT_HP || p_Inp->ProfileIDC==FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "Fast Mode Decision methods not supported in FREX Profiles");
    error (errortext, 500);
  }

  // Tian Dong: May 31, 2002
  // The number of frames in one sub-seq in enhanced layer should not exceed
  // the number of reference frame number.
  if ( p_Inp->NumFramesInELSubSeq > p_Inp->num_ref_frames || p_Inp->NumFramesInELSubSeq < 0 )
  {
    snprintf(errortext, ET_SIZE, "NumFramesInELSubSeq (%d) is out of range [0,%d).", p_Inp->NumFramesInELSubSeq, p_Inp->num_ref_frames);
    error (errortext, 500);
  }
  // Tian Dong: Enhanced GOP is not supported in bitstream mode. September, 2002
  if ( p_Inp->NumFramesInELSubSeq > 0 )
  {
    snprintf(errortext, ET_SIZE, "Enhanced GOP is not properly supported yet.");
    error (errortext, 500);
  }
  // Tian Dong (Sept 2002)
  // The AFF is not compatible with spare picture for the time being.
  if ((p_Inp->PicInterlace || p_Inp->MbInterlace) && p_Inp->SparePictureOption == TRUE)
  {
    snprintf(errortext, ET_SIZE, "AFF is not compatible with spare picture.");
    error (errortext, 500);
  }

  // Only the RTP mode is compatible with spare picture for the time being.
  if (p_Inp->of_mode != PAR_OF_RTP && p_Inp->SparePictureOption == TRUE)
  {
    snprintf(errortext, ET_SIZE, "Only RTP output mode is compatible with spare picture features.");
    error (errortext, 500);
  }

  if( (p_Inp->DepthRangeBasedWP||p_Inp->WeightedPrediction > 0 || p_Inp->WeightedBiprediction > 0) && (p_Inp->MbInterlace))
  {
    snprintf(errortext, ET_SIZE, "Weighted prediction coding is not supported for MB AFF currently.");
    error (errortext, 500);
  }
  if ( p_Inp->NumFramesInELSubSeq > 0 && p_Inp->WeightedPrediction > 0 && p_Inp->DepthRangeBasedWP)
  {
    snprintf(errortext, ET_SIZE, "Enhanced GOP is not supported in weighted prediction coding mode yet.");
    error (errortext, 500);
  }
  if( (p_Inp->ProfileIDC==ThreeDV_HIGH)&&(p_Inp->DepthRangeBasedWP) )
  {
    snprintf(errortext, ET_SIZE, "DepthRangeBasedWP is not supported for MVCD.");
    error (errortext, 500);
  }

  if(is_depth)
  {
    if((p_Inp->source.width[0]>p_Inp->OriginalWidth)||(p_Inp->source.height[0]>p_Inp->OriginalHeight))
    {
      snprintf(errortext,ET_SIZE,"The original resolution for depth shall be not smaller than encoding resolution.");
      error(errortext,500);
    }
    if((p_Inp->source.width[0]>p_Vid->p_DualInp->source.width[0])||(p_Inp->source.height[0]>p_Vid->p_DualInp->source.height[0]))
    {
      snprintf(errortext,ET_SIZE,"The encoding resolution for depth shall be not larger than texture's coding resolution.");
      error(errortext,500);
    }

  }
  else
  {
    if((p_Inp->source.width[0]>p_Inp->OriginalWidth)||(p_Inp->source.height[0]>p_Inp->OriginalHeight))
    {
      snprintf(errortext,ET_SIZE,"The original resolution for texture shall be not smaller than encoding resolution.");
      error(errortext,500);
    }
  }

  if ((!is_depth)&&(p_Inp->ThreeDVCoding)&&(p_Inp->rdopt!=1))
  {
    snprintf(errortext,ET_SIZE,"Current 3DV coding is built on high complexity RDO mode.");
    error(errortext,500);
  }
  if(p_Inp->ThreeDVCoding&&(p_Inp->SearchMode!=3))
  {
    snprintf(errortext,ET_SIZE,"Current 3DV coding is built on fast motion estimation with EPZS.");
    error(errortext,500);
  }

  // Rate control
  if(p_Inp->RCEnable)
  {

    if ( p_Inp->RCUpdateMode == RC_MODE_1 && 
      !( (p_Inp->intra_period == 1 || p_Inp->idr_period == 1 || p_Inp->BRefPictures == 2 ) && !p_Inp->NumberBFrames ) )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 1 only for all intra or all B-slice coding.");
      error (errortext, 500);
    }
    if ( (p_Inp->RCUpdateMode == RC_MODE_0 || p_Inp->RCUpdateMode == RC_MODE_2 || p_Inp->RCUpdateMode == RC_MODE_3) && 
      (p_Inp->PReplaceBSlice && p_Inp->NumberBFrames ) )
    {
      snprintf(errortext, ET_SIZE, "PReplaceBSlice is not supported with RCUpdateMode=0,2,3.");
      error (errortext, 500);
    }

    if ( p_Inp->BRefPictures == 2 && p_Inp->intra_period == 0 && p_Inp->RCUpdateMode != RC_MODE_1 )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 1 for all B-slice coding.");
      error (errortext, 500);
    }

    if ( p_Inp->HierarchicalCoding && p_Inp->RCUpdateMode != RC_MODE_2 && p_Inp->RCUpdateMode != RC_MODE_3 )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 2 or 3 for hierarchical B-picture coding.");
      error (errortext, 500);
    }
    if ( (p_Inp->RCUpdateMode != RC_MODE_1) && (p_Inp->intra_period == 1) )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 1 for all intra coding.");
      error (errortext, 500);
    }
  }

  if ((p_Inp->NumberBFrames)&&(p_Inp->BRefPictures)&&(p_Inp->idr_period)&&(p_Inp->pic_order_cnt_type!=0))
  {
    error("Stored B pictures combined with IDR pictures only supported in Picture Order Count type 0\n",-1000);
  }

  if( !p_Inp->direct_spatial_mv_pred_flag && p_Inp->num_ref_frames<2 && p_Inp->NumberBFrames >0)
    error("temporal direct needs at least 2 ref frames\n",-1000);

  if (p_Inp->SearchMode == FAST_FULL_SEARCH && p_Inp->MEErrorMetric[F_PEL] > ERROR_SSE)
  {
    snprintf(errortext, ET_SIZE, "\nOnly SAD and SSE distortion computation supported with Fast Full Search.");
    error (errortext, 500);
  }

  if (p_Inp->rdopt == 0)
  {
    if (p_Inp->DisableSubpelME)
    {
      if (p_Inp->MEErrorMetric[F_PEL] != p_Inp->ModeDecisionMetric)
      {
        snprintf(errortext, ET_SIZE, "\nLast refinement level (FPel) distortion not the same as Mode decision distortion.\nPlease update MEDistortionFPel (%d) and/or  MDDistortion(%d).", p_Inp->MEErrorMetric[F_PEL], p_Inp->ModeDecisionMetric);
        error (errortext, 500);
      }
    }
    else if (p_Inp->MEErrorMetric[Q_PEL] != p_Inp->ModeDecisionMetric)
    {
      snprintf(errortext, ET_SIZE, "\nLast refinement level (QPel) distortion not the same as Mode decision distortion.\nPlease update MEDistortionQPel (%d) and/or  MDDistortion(%d).", p_Inp->MEErrorMetric[Q_PEL], p_Inp->ModeDecisionMetric);
      error (errortext, 500);
    }
  }
  // frext
  if(p_Inp->Transform8x8Mode && p_Inp->sp_periodicity /*SP-frames*/)
  {
    snprintf(errortext, ET_SIZE, "\nThe new 8x8 mode is not implemented for sp-frames.");
    error (errortext, 500);
  }

  if(p_Inp->Transform8x8Mode && ( p_Inp->ProfileIDC<FREXT_HP && p_Inp->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nTransform8x8Mode may be used only with ProfileIDC %d to %d.", FREXT_HP, FREXT_Hi444);
    error (errortext, 500);
  }

  if (p_Inp->DisableIntra4x4 == 1 && p_Inp->DisableIntra16x16 == 1 && p_Inp->EnableIPCM == 0 && p_Inp->Transform8x8Mode == 0)
  {
    snprintf(errortext, ET_SIZE, "\nAt least one intra prediction mode needs to be enabled.");
    error (errortext, 500);
  }

  if(p_Inp->ScalingMatrixPresentFlag && ( p_Inp->ProfileIDC<FREXT_HP && p_Inp->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nScalingMatrixPresentFlag may be used only with ProfileIDC %d to %d.", FREXT_HP, FREXT_Hi444);
    error (errortext, 500);
  }

  if(p_Inp->yuv_format==YUV422 && ( p_Inp->ProfileIDC < FREXT_Hi422 && p_Inp->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nFRExt Profile(YUV Format) Error!\nYUV422 can be used only with ProfileIDC %d or %d\n",FREXT_Hi422, FREXT_Hi444);
    error (errortext, 500);
  }
  if(p_Inp->yuv_format==YUV444 && ( p_Inp->ProfileIDC < FREXT_Hi444 && p_Inp->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nFRExt Profile(YUV Format) Error!\nYUV444 can be used only with ProfileIDC %d.\n",FREXT_Hi444);
    error (errortext, 500);
  }

  if (p_Inp->NumberBFrames && ((p_Inp->BiPredMotionEstimation) && (p_Inp->search_range < p_Inp->BiPredMESearchRange)))
  {
    snprintf(errortext, ET_SIZE, "\nBiPredMESearchRange must be smaller or equal SearchRange.");
    error (errortext, 500);
  }

  if (p_Inp->BiPredMotionEstimation)
  {
    p_Inp->BiPredMotionEstimation = 0;
    for (i = 0 ; i < 4; i++)
      p_Inp->BiPredMotionEstimation |= p_Inp->BiPredSearch[i];
  }
  else
  {
    for (i = 0 ; i < 4; i++)
      p_Inp->BiPredSearch[i] = 0;
  }

  // check consistency
  if ( p_Inp->ChromaMEEnable && !(p_Inp->ChromaMCBuffer) ) 
  {
    snprintf(errortext, ET_SIZE, "\nChromaMCBuffer must be set to 1 if ChromaMEEnable is set.");
    error (errortext, 500);
  }

  if ( p_Inp->ChromaMEEnable && p_Inp->yuv_format ==  YUV400) 
  {
    fprintf(stderr, "Warning: ChromaMEEnable cannot be used with monochrome color format, disabling ChromaMEEnable.\n");
    p_Inp->ChromaMEEnable = FALSE;
  }

  if ( (p_Inp->ChromaMCBuffer == 0) && (( p_Inp->yuv_format ==  YUV444) && (!p_Inp->separate_colour_plane_flag)) )
  {
    fprintf(stderr, "Warning: Enabling ChromaMCBuffer for 4:4:4 combined color coding.\n");
    p_Inp->ChromaMCBuffer = 1;
  }

  if (p_Inp->EnableOpenGOP && p_Inp->ReferenceReorder == 2)
  {
    printf("If OpenGOP is enabled than ReferenceReorder is set to 1. \n");
  }

  if (p_Inp->EnableOpenGOP)
    p_Inp->ReferenceReorder = 1;

  if((p_Inp->ProfileIDC==MULTIVIEW_HIGH || p_Inp->ProfileIDC==STEREO_HIGH) && p_Inp->NumOfViews ==0)
  {
    snprintf(errortext, ET_SIZE, "NumberOfViews must be more than one if ProfileIDC is set to 118 (Multiview High Profile). Otherwise (for a single view) please select a non-multiview profile such as 100.");
    error (errortext, 500);
  }

  if (p_Inp->SearchMode != EPZS)
    p_Inp->EPZSSubPelGrid = 0;

  if (p_Inp->redundant_pic_flag)
  {
    if (p_Inp->PicInterlace || p_Inp->MbInterlace)
    {
      snprintf(errortext, ET_SIZE, "Redundant pictures cannot be used with interlaced tools.");
      error (errortext, 500);
    }
    if (p_Inp->RDPictureDecision)
    {
      snprintf(errortext, ET_SIZE, "Redundant pictures cannot be used with RDPictureDecision.");
      error (errortext, 500);
    }
    if (p_Inp->NumberBFrames)
    {
      snprintf(errortext, ET_SIZE, "Redundant pictures cannot be used with B frames.");
      error (errortext, 500);
    }
    if (p_Inp->PrimaryGOPLength < (1 << p_Inp->NumRedundantHierarchy))
    {
      snprintf(errortext, ET_SIZE, "PrimaryGOPLength must be equal or greater than 2^NumRedundantHierarchy.");
      error (errortext, 500);
    }
    if (p_Inp->num_ref_frames < p_Inp->PrimaryGOPLength)
    {
      snprintf(errortext, ET_SIZE, "NumberReferenceFrames must be greater than or equal to PrimaryGOPLength.");
      error (errortext, 500);
    }
  }
  if(p_Vid->is_depth)
  {
    if (p_Inp->num_ref_frames == 1 && p_Inp->NumberBFrames)
    {
      fprintf( stderr, "\nWarning: B slices used but only one reference allocated within reference buffer.\n");
      fprintf( stderr, "         Performance may be considerably compromised! \n");
      fprintf( stderr, "         2 or more references recommended for use with B slices.\n");
    }
    if ((p_Inp->HierarchicalCoding || p_Inp->BRefPictures) && p_Inp->NumberBFrames)
    {
      fprintf( stderr, "\nWarning: Hierarchical coding or Referenced B slices used.\n");
      fprintf( stderr, "         Make sure that you have allocated enough references\n");
      fprintf( stderr, "         in reference buffer to achieve best performance.\n");
    }
  }
  if (p_Inp->FastMDEnable == 0)
  {
    p_Inp->FastIntraMD = 0;
    p_Inp->FastIntra16x16 = 0;
    p_Inp->FastIntra4x4 = 0;
    p_Inp->FastIntra8x8 = 0;
    p_Inp->FastIntraChroma = 0;
  }
  if (p_Inp->UseRDOQuant == 1)
  {
    if (p_Inp->rdopt == 0)
    {
      snprintf(errortext, ET_SIZE, "RDO Quantization not supported with low complexity RDO.");
      error (errortext, 500);
    }

    if (p_Inp->MbInterlace != 0)
    {
      printf("RDO Quantization currently not supported with MBAFF. Option disabled.\n");
      p_Inp->UseRDOQuant = 0;
      p_Inp->RDOQ_QP_Num = 1;
      p_Inp->RDOQ_CP_MV = 0;
      p_Inp->RDOQ_CP_Mode = 0;
    }
    else
    {
      p_Inp->AdaptiveRounding = 0;
      printf("AdaptiveRounding is disabled when RDO Quantization is used\n");
      if (p_Inp->RDOQ_QP_Num < 2)
      {
        p_Inp->RDOQ_CP_MV = 0;
        p_Inp->RDOQ_CP_Mode = 0;
      }
    }
  }
  else
  {
    p_Inp->RDOQ_QP_Num = 1;
    p_Inp->RDOQ_CP_MV = 0;
    p_Inp->RDOQ_CP_Mode = 0;
  }

  if(p_Inp->num_slice_groups_minus1 > 0 && (p_Inp->GenerateMultiplePPS ==1 && p_Inp->RDPictureDecision == 1))
  {
    printf("Warning: Weighted Prediction may not function correctly for multiple slices\n"); 
  }
  ProfileCheck(p_Inp);
  if(!p_Inp->RDPictureDecision)
  {
    p_Inp->RDPictureMaxPassISlice = 1;
    p_Inp->RDPictureMaxPassPSlice = 1;
    p_Inp->RDPictureMaxPassBSlice = 1;
    p_Inp->RDPictureDeblocking    = 0;
    p_Inp->RDPictureDirectMode    = 0;
    p_Inp->RDPictureFrameQPPSlice = 0;
    p_Inp->RDPictureFrameQPBSlice = 0;
  }

#if ITRI_INTERLACE
  if(is_depth)
  {
    if( (p_Vid->p_DualVid->p_Inp->PicInterlace==0)&&(p_Inp->PicInterlace!=0) )
    {
      snprintf(errortext, ET_SIZE, "Unsupported Progressive Texture plus Interlace depth coding.");
      error (errortext, 500);
    }
    if( (p_Inp->ProfileIDC!=ThreeDV_HIGH)&&(p_Vid->p_DualVid->p_Inp->PicInterlace!=0)&&(p_Inp->PicInterlace!=0) )
    {
      snprintf(errortext, ET_SIZE, "Unsupported interlace coding modes.");
      error (errortext, 500);
    }
    if( (p_Inp->WeightedPrediction > 0 || p_Inp->WeightedBiprediction > 0) && 
      (p_Inp->NumOfViews > 1) && (p_Inp->ProfileIDC==ThreeDV_HIGH) && (p_Inp->PicInterlace!=0) )
    {
      snprintf(errortext, ET_SIZE, "Weighted prediction coding is not supported currently.");
      error (errortext, 500);
    }
  }
#endif
}

#else

/*!
 ***********************************************************************
 * \brief
 *    Checks the input parameters for consistency.
 ***********************************************************************
 */
static void PatchInp (VideoParameters *p_Vid, InputParameters *p_Inp)
{
  int i;
  int storedBplus1;
  int bitdepth_qp_scale[3];

  if (p_Inp->src_BitDepthRescale)
  {
    bitdepth_qp_scale [0] = 6*(p_Inp->output.bit_depth[0] - 8);
    bitdepth_qp_scale [1] = 6*(p_Inp->output.bit_depth[1] - 8);
    bitdepth_qp_scale [2] = 6*(p_Inp->output.bit_depth[2] - 8);
  }
  else
  {
    bitdepth_qp_scale [0] = 6*(p_Inp->source.bit_depth[0] - 8);
    bitdepth_qp_scale [1] = 6*(p_Inp->source.bit_depth[1] - 8);
    bitdepth_qp_scale [2] = 6*(p_Inp->source.bit_depth[2] - 8);
  }

  TestEncoderParams(Map, bitdepth_qp_scale);

  if (p_Inp->source.frame_rate == 0.0)
    p_Inp->source.frame_rate = (double) INIT_FRAME_RATE;



  ParseVideoType(&p_Inp->input_file1);
  ParseFrameNoFormatFromString (&p_Inp->input_file1);
#if (MVC_EXTENSION_ENABLE)
  if ( p_Inp->num_of_views == 2 )
  {
    ParseVideoType(&p_Inp->input_file2);
    ParseFrameNoFormatFromString (&p_Inp->input_file2);
  }
#endif

  // Read resolution from file name
  if (p_Inp->source.width[0] == 0 || p_Inp->source.height[0] == 0)
  {
    if (ParseSizeFromString (&p_Inp->input_file1, &(p_Inp->source.width[0]), &(p_Inp->source.height[0]), &(p_Inp->source.frame_rate)) == 0)
    {
      snprintf(errortext, ET_SIZE, "File name does not contain resolution information.");    
      error (errortext, 500);
    }
  }

#if (!ENABLE_FIELD_CTX)
  if ( (p_Inp->PicInterlace || p_Inp->MbInterlace) && p_Inp->symbol_mode )
  {
    snprintf(errortext, ET_SIZE, "Recompile with ENABLE_FIELD_CTX set to one to enable interlaced coding with CABAC.");    
    error (errortext, 500);
  }
#endif
#if (!ENABLE_HIGH444_CTX)
  if ( p_Inp->ProfileIDC == 244 && p_Inp->symbol_mode )
  {
    snprintf(errortext, ET_SIZE, "Recompile with ENABLE_HIGH444_CTX set to one to enable the High 4:4:4 Profile with CABAC.");    
    error (errortext, 500);
  }
#endif

  // Currently to simplify things, lets copy everything (overwrites yuv_format)
  p_Inp->input_file1.format = p_Inp->source;

  if (p_Inp->idr_period && p_Inp->intra_delay && p_Inp->idr_period <= p_Inp->intra_delay)
  {
    snprintf(errortext, ET_SIZE, " IntraDelay cannot be larger than or equal to IDRPeriod.");
    error (errortext, 500);
  }

  // Let us set up p_Inp->jumpd from frame_skip and NumberBFrames
  p_Inp->jumpd = (p_Inp->NumberBFrames + 1) * (p_Inp->frame_skip + 1) - 1;


  updateOutFormat(p_Inp);

  if (p_Inp->no_frames == -1)
  {
    OpenFiles(&p_Inp->input_file1);
    get_number_of_frames (p_Inp, &p_Inp->input_file1);
    CloseFiles(&p_Inp->input_file1);
  }

  storedBplus1 = (p_Inp->BRefPictures ) ? p_Inp->NumberBFrames + 1: 1;

  
  if (p_Inp->no_frames < 1)
  {      
    snprintf(errortext, ET_SIZE, "Not enough frames to encode (%d)", p_Inp->no_frames);
    error (errortext, 500);
  }

  // Direct Mode consistency check
  if(p_Inp->NumberBFrames && p_Inp->direct_spatial_mv_pred_flag != DIR_SPATIAL && p_Inp->direct_spatial_mv_pred_flag != DIR_TEMPORAL)
  {
    snprintf(errortext, ET_SIZE, "Unsupported direct mode=%d, use TEMPORAL=0 or SPATIAL=1", p_Inp->direct_spatial_mv_pred_flag);
    error (errortext, 400);
  }

  if (p_Inp->PicInterlace>0 || p_Inp->MbInterlace>0)
  {
    if (p_Inp->directInferenceFlag==0)
      printf("\nWarning: DirectInferenceFlag set to 1 due to interlace coding.");
    p_Inp->directInferenceFlag = 1;
  }

#if TRACE
  if ((int) strlen (p_Inp->TraceFile) > 0 && (p_Enc->p_trace = fopen(p_Inp->TraceFile,"w"))==NULL)
  {
    snprintf(errortext, ET_SIZE, "Error open file %s", p_Inp->TraceFile);
    error (errortext, 500);
  }
#endif

  
  
  if ((p_Inp->slice_mode == 1)&&(p_Inp->MbInterlace != 0))
  {
    if ((p_Inp->slice_argument & 0x01)!=0)
    {
      fprintf ( stderr, "Warning: slice border within macroblock pair. ");
      if (p_Inp->slice_argument > 1)
      {
        p_Inp->slice_argument--;
      }
      else
      {
        p_Inp->slice_argument++;
      }
      fprintf ( stderr, "Using %d MBs per slice.\n", p_Inp->slice_argument);
    }
  }  

  if (p_Inp->WPMCPrecision && (p_Inp->RDPictureDecision != 1 || p_Inp->GenerateMultiplePPS != 1) )
  {
    snprintf(errortext, ET_SIZE, "WPMCPrecision requires both RDPictureDecision=1 and GenerateMultiplePPS=1.\n");
    error (errortext, 400);
  }
  if (p_Inp->WPMCPrecision && p_Inp->WPMCPrecFullRef && p_Inp->num_ref_frames < 16 )
  {
    p_Inp->num_ref_frames++;
    if ( p_Inp->P_List0_refs )
      p_Inp->P_List0_refs++;
    else
      p_Inp->P_List0_refs = p_Inp->num_ref_frames;
    if ( p_Inp->B_List0_refs )
      p_Inp->B_List0_refs++;
    else
      p_Inp->B_List0_refs = p_Inp->num_ref_frames;
    if ( p_Inp->B_List1_refs )
      p_Inp->B_List1_refs++;
    else
      p_Inp->B_List1_refs = p_Inp->num_ref_frames;
  }
  else if ( p_Inp->WPMCPrecision && p_Inp->WPMCPrecFullRef )
  {
    snprintf(errortext, ET_SIZE, "WPMCPrecFullRef requires NumberReferenceFrames < 16.\n");
    error (errortext, 400);
  }

  if ( p_Inp->WPMCPrecision && ( p_Inp->PicInterlace || p_Inp->MbInterlace ) )
  {
    snprintf(errortext, ET_SIZE, "WPMCPrecision does not work for PicInterlace > 0 or MbInterlace > 0.\n");
    error (errortext, 400);
  }

  if (p_Inp->PicInterlace) 
  {
    if (p_Inp->ReferenceReorder == 2)
    {
      snprintf(errortext, ET_SIZE, "ReferenceReorder = 2 is not supported with field encoding\n");
      error (errortext, 400);
    }
    if (p_Inp->ReferenceReorder == 2)
    {
      snprintf(errortext, ET_SIZE, "PocMemoryManagement = 2 is not supported with field encoding\n");
      error (errortext, 400);
    }
  }

  if (p_Inp->ReferenceReorder && p_Inp->MbInterlace )
  {
    snprintf(errortext, ET_SIZE, "ReferenceReorder is not supported with MBAFF\n");
    error (errortext, 400);
  }

  if (p_Inp->SetFirstAsLongTerm && (p_Inp->ReferenceReorder == 1 || p_Inp->ReferenceReorder == 2))
  {
    printf("SetFirstAsLongTerm is set. ReferenceReorder is not supported and therefore disabled. \n");
    p_Inp->ReferenceReorder = 0;
  }

#if (MVC_EXTENSION_ENABLE)
  if ( (p_Inp->num_of_views > 1) && (p_Inp->ReferenceReorder > 0) )
  {
    snprintf(errortext, ET_SIZE, "ReferenceReorder is not supported with more than one view.\n");
    error (errortext, 400);
  }

  if ( (p_Inp->num_of_views > 1) && (p_Inp->PocMemoryManagement > 0) )
  {
    snprintf(errortext, ET_SIZE, "PocMemoryManagement is not supported with more than one view.\n");
    error (errortext, 400);
  }
#endif


  if (p_Inp->PocMemoryManagement && p_Inp->MbInterlace )
  {
    snprintf(errortext, ET_SIZE, "PocMemoryManagement is not supported with MBAFF\n");
    error (errortext, 400);
  }

  if(p_Inp->MbInterlace && p_Inp->RDPictureDecision && p_Inp->GenerateMultiplePPS)
  {
    snprintf(errortext, ET_SIZE, "RDPictureDecision+GenerateMultiplePPS not supported with MBAFF. RDPictureDecision therefore disabled\n");
    p_Inp->RDPictureDecision = 0;
  }

  if ((!p_Inp->rdopt)&&(p_Inp->MbInterlace==2))
  {
    snprintf(errortext, ET_SIZE, "MB AFF is not compatible with non-rd-optimized coding.");
    error (errortext, 500);
  }

  // check RDoptimization mode and profile. FMD does not support Frex Profiles.
  if (p_Inp->rdopt==2 && ( p_Inp->ProfileIDC>=FREXT_HP || p_Inp->ProfileIDC==FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "Fast Mode Decision methods not supported in FREX Profiles");
    error (errortext, 500);
  }

  // Tian Dong: May 31, 2002
  // The number of frames in one sub-seq in enhanced layer should not exceed
  // the number of reference frame number.
  if ( p_Inp->NumFramesInELSubSeq > p_Inp->num_ref_frames || p_Inp->NumFramesInELSubSeq < 0 )
  {
    snprintf(errortext, ET_SIZE, "NumFramesInELSubSeq (%d) is out of range [0,%d).", p_Inp->NumFramesInELSubSeq, p_Inp->num_ref_frames);
    error (errortext, 500);
  }
  // Tian Dong: Enhanced GOP is not supported in bitstream mode. September, 2002
  if ( p_Inp->NumFramesInELSubSeq > 0 )
  {
    snprintf(errortext, ET_SIZE, "Enhanced GOP is not properly supported yet.");
    error (errortext, 500);
  }
  // Tian Dong (Sept 2002)
  // The AFF is not compatible with spare picture for the time being.
  if ((p_Inp->PicInterlace || p_Inp->MbInterlace) && p_Inp->SparePictureOption == TRUE)
  {
    snprintf(errortext, ET_SIZE, "AFF is not compatible with spare picture.");
    error (errortext, 500);
  }

  // Only the RTP mode is compatible with spare picture for the time being.
  if (p_Inp->of_mode != PAR_OF_RTP && p_Inp->SparePictureOption == TRUE)
  {
    snprintf(errortext, ET_SIZE, "Only RTP output mode is compatible with spare picture features.");
    error (errortext, 500);
  }

  if( (p_Inp->WeightedPrediction > 0 || p_Inp->WeightedBiprediction > 0) && (p_Inp->MbInterlace))
  {
    snprintf(errortext, ET_SIZE, "Weighted prediction coding is not supported for MB AFF currently.");
    error (errortext, 500);
  }
  if ( p_Inp->NumFramesInELSubSeq > 0 && p_Inp->WeightedPrediction > 0)
  {
    snprintf(errortext, ET_SIZE, "Enhanced GOP is not supported in weighted prediction coding mode yet.");
    error (errortext, 500);
  }

  // Rate control
  if(p_Inp->RCEnable)
  {

    if ( p_Inp->RCUpdateMode == RC_MODE_1 && 
      !( (p_Inp->intra_period == 1 || p_Inp->idr_period == 1 || p_Inp->BRefPictures == 2 ) && !p_Inp->NumberBFrames ) )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 1 only for all intra or all B-slice coding.");
      error (errortext, 500);
    }
    if ( (p_Inp->RCUpdateMode == RC_MODE_0 || p_Inp->RCUpdateMode == RC_MODE_2 || p_Inp->RCUpdateMode == RC_MODE_3) && 
      (p_Inp->PReplaceBSlice && p_Inp->NumberBFrames ) )
    {
      snprintf(errortext, ET_SIZE, "PReplaceBSlice is not supported with RCUpdateMode=0,2,3.");
      error (errortext, 500);
    }

    if ( p_Inp->BRefPictures == 2 && p_Inp->intra_period == 0 && p_Inp->RCUpdateMode != RC_MODE_1 )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 1 for all B-slice coding.");
      error (errortext, 500);
    }

    if ( p_Inp->HierarchicalCoding && p_Inp->RCUpdateMode != RC_MODE_2 && p_Inp->RCUpdateMode != RC_MODE_3 )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 2 or 3 for hierarchical B-picture coding.");
      error (errortext, 500);
    }
    if ( (p_Inp->RCUpdateMode != RC_MODE_1) && (p_Inp->intra_period == 1) )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 1 for all intra coding.");
      error (errortext, 500);
    }
  }

  if ((p_Inp->NumberBFrames)&&(p_Inp->BRefPictures)&&(p_Inp->idr_period)&&(p_Inp->pic_order_cnt_type!=0))
  {
    error("Stored B pictures combined with IDR pictures only supported in Picture Order Count type 0\n",-1000);
  }

  if( !p_Inp->direct_spatial_mv_pred_flag && p_Inp->num_ref_frames<2 && p_Inp->NumberBFrames >0)
    error("temporal direct needs at least 2 ref frames\n",-1000);

  if (p_Inp->SearchMode == FAST_FULL_SEARCH && p_Inp->MEErrorMetric[F_PEL] > ERROR_SSE)
  {
    snprintf(errortext, ET_SIZE, "\nOnly SAD and SSE distortion computation supported with Fast Full Search.");
    error (errortext, 500);
  }

  if (p_Inp->rdopt == 0)
  {
    if (p_Inp->DisableSubpelME)
    {
      if (p_Inp->MEErrorMetric[F_PEL] != p_Inp->ModeDecisionMetric)
      {
        snprintf(errortext, ET_SIZE, "\nLast refinement level (FPel) distortion not the same as Mode decision distortion.\nPlease update MEDistortionFPel (%d) and/or  MDDistortion(%d).", p_Inp->MEErrorMetric[F_PEL], p_Inp->ModeDecisionMetric);
        error (errortext, 500);
      }
    }
    else if (p_Inp->MEErrorMetric[Q_PEL] != p_Inp->ModeDecisionMetric)
    {
      snprintf(errortext, ET_SIZE, "\nLast refinement level (QPel) distortion not the same as Mode decision distortion.\nPlease update MEDistortionQPel (%d) and/or  MDDistortion(%d).", p_Inp->MEErrorMetric[Q_PEL], p_Inp->ModeDecisionMetric);
      error (errortext, 500);
    }
  }
  // frext
  if(p_Inp->Transform8x8Mode && p_Inp->sp_periodicity /*SP-frames*/)
  {
    snprintf(errortext, ET_SIZE, "\nThe new 8x8 mode is not implemented for sp-frames.");
    error (errortext, 500);
  }

  if(p_Inp->Transform8x8Mode && ( p_Inp->ProfileIDC<FREXT_HP && p_Inp->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nTransform8x8Mode may be used only with ProfileIDC %d to %d.", FREXT_HP, FREXT_Hi444);
    error (errortext, 500);
  }

  if (p_Inp->DisableIntra4x4 == 1 && p_Inp->DisableIntra16x16 == 1 && p_Inp->EnableIPCM == 0 && p_Inp->Transform8x8Mode == 0)
  {
    snprintf(errortext, ET_SIZE, "\nAt least one intra prediction mode needs to be enabled.");
    error (errortext, 500);
  }

  if(p_Inp->ScalingMatrixPresentFlag && ( p_Inp->ProfileIDC<FREXT_HP && p_Inp->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nScalingMatrixPresentFlag may be used only with ProfileIDC %d to %d.", FREXT_HP, FREXT_Hi444);
    error (errortext, 500);
  }

  if(p_Inp->yuv_format==YUV422 && ( p_Inp->ProfileIDC < FREXT_Hi422 && p_Inp->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nFRExt Profile(YUV Format) Error!\nYUV422 can be used only with ProfileIDC %d or %d\n",FREXT_Hi422, FREXT_Hi444);
    error (errortext, 500);
  }
  if(p_Inp->yuv_format==YUV444 && ( p_Inp->ProfileIDC < FREXT_Hi444 && p_Inp->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nFRExt Profile(YUV Format) Error!\nYUV444 can be used only with ProfileIDC %d.\n",FREXT_Hi444);
    error (errortext, 500);
  }

  if (p_Inp->NumberBFrames && ((p_Inp->BiPredMotionEstimation) && (p_Inp->search_range < p_Inp->BiPredMESearchRange)))
  {
    snprintf(errortext, ET_SIZE, "\nBiPredMESearchRange must be smaller or equal SearchRange.");
    error (errortext, 500);
  }

  if (p_Inp->BiPredMotionEstimation)
  {
    p_Inp->BiPredMotionEstimation = 0;
    for (i = 0 ; i < 4; i++)
      p_Inp->BiPredMotionEstimation |= p_Inp->BiPredSearch[i];
  }
  else
  {
    for (i = 0 ; i < 4; i++)
      p_Inp->BiPredSearch[i] = 0;
  }

  // check consistency
  if ( p_Inp->ChromaMEEnable && !(p_Inp->ChromaMCBuffer) ) 
  {
    snprintf(errortext, ET_SIZE, "\nChromaMCBuffer must be set to 1 if ChromaMEEnable is set.");
    error (errortext, 500);
  }

  if ( p_Inp->ChromaMEEnable && p_Inp->yuv_format ==  YUV400) 
  {
    fprintf(stderr, "Warning: ChromaMEEnable cannot be used with monochrome color format, disabling ChromaMEEnable.\n");
    p_Inp->ChromaMEEnable = FALSE;
  }

  if ( (p_Inp->ChromaMCBuffer == 0) && (( p_Inp->yuv_format ==  YUV444) && (!p_Inp->separate_colour_plane_flag)) )
  {
    fprintf(stderr, "Warning: Enabling ChromaMCBuffer for 4:4:4 combined color coding.\n");
    p_Inp->ChromaMCBuffer = 1;
  }

  if (p_Inp->EnableOpenGOP && p_Inp->ReferenceReorder == 2)
  {
    printf("If OpenGOP is enabled than ReferenceReorder is set to 1. \n");
  }

  if (p_Inp->EnableOpenGOP)
    p_Inp->ReferenceReorder = 1;
#if (MVC_EXTENSION_ENABLE)
  if((p_Inp->ProfileIDC==MULTIVIEW_HIGH || p_Inp->ProfileIDC==STEREO_HIGH) && p_Inp->num_of_views != 2)
  {
    snprintf(errortext, ET_SIZE, "NumberOfViews must be two if ProfileIDC is set to 118 (Multiview High Profile). Otherwise (for a single view) please select a non-multiview profile such as 100.");
    error (errortext, 500);
  }

//  if (p_Inp->PicInterlace == 2 && p_Inp->MVCInterViewReorder != 0)
//  {
//    snprintf(errortext, ET_SIZE, "MVCInterViewReorder not supported with Adaptive Frame Field Coding");
//    error (errortext, 500);
//  }

  if(p_Inp->MVCInterViewReorder)
  {
    if(p_Inp->ProfileIDC!=MULTIVIEW_HIGH && p_Inp->ProfileIDC!=STEREO_HIGH)
    {
      snprintf(errortext, ET_SIZE, "ProfileIDC must be 118 or 128 to use MVCInterViewReorder=1.");
      error (errortext, 500);
    }
    if(p_Inp->ReferenceReorder!=0)
    {
      snprintf(errortext, ET_SIZE, "ReferenceReorder=1 is not supported with MVCInterViewReorder=1.");
      error (errortext, 500);
    }
  }
#endif

  if (p_Inp->SearchMode != EPZS)
    p_Inp->EPZSSubPelGrid = 0;

  if (p_Inp->redundant_pic_flag)
  {
    if (p_Inp->PicInterlace || p_Inp->MbInterlace)
    {
      snprintf(errortext, ET_SIZE, "Redundant pictures cannot be used with interlaced tools.");
      error (errortext, 500);
    }
    if (p_Inp->RDPictureDecision)
    {
      snprintf(errortext, ET_SIZE, "Redundant pictures cannot be used with RDPictureDecision.");
      error (errortext, 500);
    }
    if (p_Inp->NumberBFrames)
    {
      snprintf(errortext, ET_SIZE, "Redundant pictures cannot be used with B frames.");
      error (errortext, 500);
    }
    if (p_Inp->PrimaryGOPLength < (1 << p_Inp->NumRedundantHierarchy))
    {
      snprintf(errortext, ET_SIZE, "PrimaryGOPLength must be equal or greater than 2^NumRedundantHierarchy.");
      error (errortext, 500);
    }
    if (p_Inp->num_ref_frames < p_Inp->PrimaryGOPLength)
    {
      snprintf(errortext, ET_SIZE, "NumberReferenceFrames must be greater than or equal to PrimaryGOPLength.");
      error (errortext, 500);
    }
  }

  if (p_Inp->num_ref_frames == 1 && p_Inp->NumberBFrames)
  {
    fprintf( stderr, "\nWarning: B slices used but only one reference allocated within reference buffer.\n");
    fprintf( stderr, "         Performance may be considerably compromised! \n");
    fprintf( stderr, "         2 or more references recommended for use with B slices.\n");
  }
  if ((p_Inp->HierarchicalCoding || p_Inp->BRefPictures) && p_Inp->NumberBFrames)
  {
    fprintf( stderr, "\nWarning: Hierarchical coding or Referenced B slices used.\n");
    fprintf( stderr, "         Make sure that you have allocated enough references\n");
    fprintf( stderr, "         in reference buffer to achieve best performance.\n");
  }

  if (p_Inp->FastMDEnable == 0)
  {
    p_Inp->FastIntraMD = 0;
    p_Inp->FastIntra16x16 = 0;
    p_Inp->FastIntra4x4 = 0;
    p_Inp->FastIntra8x8 = 0;
    p_Inp->FastIntraChroma = 0;
  }



  if (p_Inp->UseRDOQuant == 1)
  {
    if (p_Inp->rdopt == 0)
    {
      snprintf(errortext, ET_SIZE, "RDO Quantization not supported with low complexity RDO.");
      error (errortext, 500);
    }

    if (p_Inp->MbInterlace != 0)
    {
      printf("RDO Quantization currently not supported with MBAFF. Option disabled.\n");
      p_Inp->UseRDOQuant = 0;
      p_Inp->RDOQ_QP_Num = 1;
      p_Inp->RDOQ_CP_MV = 0;
      p_Inp->RDOQ_CP_Mode = 0;
    }
    else
    {
      p_Inp->AdaptiveRounding = 0;
      printf("AdaptiveRounding is disabled when RDO Quantization is used\n");
      if (p_Inp->RDOQ_QP_Num < 2)
      {
        p_Inp->RDOQ_CP_MV = 0;
        p_Inp->RDOQ_CP_Mode = 0;
      }
    }
  }
  else
  {
    p_Inp->RDOQ_QP_Num = 1;
    p_Inp->RDOQ_CP_MV = 0;
    p_Inp->RDOQ_CP_Mode = 0;
  }

  if(p_Inp->num_slice_groups_minus1 > 0 && (p_Inp->GenerateMultiplePPS ==1 && p_Inp->RDPictureDecision == 1))
  {
    printf("Warning: Weighted Prediction may not function correctly for multiple slices\n"); 
  }



  ProfileCheck(p_Inp);


  if(!p_Inp->RDPictureDecision)
  {
    p_Inp->RDPictureMaxPassISlice = 1;
    p_Inp->RDPictureMaxPassPSlice = 1;
    p_Inp->RDPictureMaxPassBSlice = 1;
    p_Inp->RDPictureDeblocking    = 0;
    p_Inp->RDPictureDirectMode    = 0;
    p_Inp->RDPictureFrameQPPSlice = 0;
    p_Inp->RDPictureFrameQPBSlice = 0;
  }
}


#endif

