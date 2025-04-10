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
 ***************************************************************************
 * \file md_distortion.c
 *
 * \brief
 *    Main macroblock mode decision functions and helpers
 *
 **************************************************************************
 */

#include <math.h>
#include <limits.h>
#include <float.h>

#include "global.h"
#include "rdopt_coding_state.h"
#include "mb_access.h"
#include "intrarefresh.h"
#include "image.h"
#include "transform8x8.h"
#include "ratectl.h"
#include "mode_decision.h"
#include "fmo.h"
#include "me_umhex.h"
#include "me_umhexsmp.h"
#include "macroblock.h"
#include "mv_search.h"
#include "md_distortion.h"

#if EXT3D
#include "configfile.h"

#define max(a,b)                     (((a) > (b)) ? (a) : (b))
#define min(a,b)                     (((a) < (b)) ? (a) : (b))
#endif

void setupDistortion(Slice *currSlice)
{
  currSlice->getDistortion = distortionSSE;
}

/*!
 ***********************************************************************
 * \brief
 *    compute generic SSE
 ***********************************************************************
 */
int64 compute_SSE(imgpel **imgRef, imgpel **imgSrc, int xRef, int xSrc, int ySize, int xSize)
{
  int i, j;
  imgpel *lineRef, *lineSrc;
  int64 distortion = 0;

  for (j = 0; j < ySize; j++)
  {
    lineRef = &imgRef[j][xRef];    
    lineSrc = &imgSrc[j][xSrc];

    for (i = 0; i < xSize; i++)
      distortion += iabs2( *lineRef++ - *lineSrc++ );
  }
  return distortion;
}

distblk compute_SSE_cr(imgpel **imgRef, imgpel **imgSrc, int xRef, int xSrc, int ySize, int xSize)
{
  int i, j;
  imgpel *lineRef, *lineSrc;
  distblk distortion = 0;

  for (j = 0; j < ySize; j++)
  {
    lineRef = &imgRef[j][xRef];    
    lineSrc = &imgSrc[j][xSrc];

    for (i = 0; i < xSize; i++)
      distortion += iabs2( *lineRef++ - *lineSrc++ );
  }

  return dist_scale(distortion);
}

/*!
 ***********************************************************************
 * \brief
 *    compute 16x16 SSE
 ***********************************************************************
 */
distblk compute_SSE16x16(imgpel **imgRef, imgpel **imgSrc, int xRef, int xSrc)
{
  int i, j;
  imgpel *lineRef, *lineSrc;
  distblk distortion = 0;

  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {
    lineRef = &imgRef[j][xRef];    
    lineSrc = &imgSrc[j][xSrc];

    for (i = 0; i < MB_BLOCK_SIZE; i++)
      distortion += iabs2( *lineRef++ - *lineSrc++ );
  }

  return dist_scale(distortion);
}

#if EXT3D
distblk compute_VSD16x16(imgpel **imgRef, imgpel **imgSrc, imgpel **texRec, int xRef, int xSrc, double dispCoeff, int texFrameWidth, int depthFrameWidth )
{
  int i, j;
  imgpel *lineRef, *lineSrc, *lineTexRec1, *lineTexRec2;
  distblk distortion = 0, temp, dC;
  int ti;

  if ( texFrameWidth == depthFrameWidth )
  {
    for (j = 0; j < MB_BLOCK_SIZE; j++)
    {
      lineRef = &imgRef[j][xRef];    
      lineSrc = &imgSrc[j][xSrc];
      lineTexRec1 = &texRec[j][0];

      for (i = 0, ti = xSrc ; i < MB_BLOCK_SIZE; i++, ti++ )
      {
        dC = abs( lineTexRec1[max(ti-1,0)] - lineTexRec1[ti] ) + abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] );

        temp = (distblk) ( ( lineRef[i] - lineSrc[i] ) * dispCoeff * (dC>>1) );
        distortion += iabs2((int)temp);
      }
    }
  }

  else if ( texFrameWidth == (depthFrameWidth<<1) )
  {
    int dD[3][3], up_dD[4];
    imgpel *lineRef2, *lineSrc2, *lineRef0, *lineSrc0, *lineRef1, *lineSrc1;

    for ( j = 0; j < MB_BLOCK_SIZE; j++ )
    {
      lineRef1 = &imgRef[j][xRef];    
      lineSrc1 = &imgSrc[j][xSrc];

      lineRef0 = &imgRef[max(j-1,0)][xRef];    
      lineSrc0 = &imgSrc[max(j-1,0)][xSrc];

      lineRef2 = &imgRef[min(j+1,MB_BLOCK_SIZE-1)][xRef];    
      lineSrc2 = &imgSrc[min(j+1,MB_BLOCK_SIZE-1)][xSrc];

      lineTexRec1 = &texRec[j<<1][0];
      lineTexRec2 = &texRec[(j<<1)+1][0];

      for (i = 0, ti = xSrc<<1 ; i < MB_BLOCK_SIZE; i++, ti+=2 )
      {
        dD[0][0] = lineSrc0[max(i-1,0)] - lineRef0[max(i-1,0)];
        dD[0][1] = lineSrc0[i] - lineRef0[i];
        dD[0][2] = lineSrc0[min(i+1,MB_BLOCK_SIZE-1)] - lineRef0[min(i+1,MB_BLOCK_SIZE-1)];

        dD[1][0] = lineSrc1[max(i-1,0)] - lineRef1[max(i-1,0)];
        dD[1][1] = lineSrc1[i] - lineRef1[i];
        dD[1][2] = lineSrc1[min(i+1,MB_BLOCK_SIZE-1)] - lineRef1[min(i+1,MB_BLOCK_SIZE-1)];

        dD[2][0] = lineSrc2[max(i-1,0)] - lineRef2[max(i-1,0)];
        dD[2][1] = lineSrc2[i] - lineRef2[i];
        dD[2][2] = lineSrc2[min(i+1,MB_BLOCK_SIZE-1)] - lineRef2[min(i+1,MB_BLOCK_SIZE-1)];

        up_dD[0] = ( dD[1][1] * 9 + dD[1][0] * 3 + dD[0][1] * 3 + dD[0][0]     ) >> 4;
        up_dD[1] = ( dD[1][1] * 9 + dD[0][1] * 3 + dD[1][2] * 3 + dD[0][2]     ) >> 4;
        up_dD[2] = ( dD[1][1] * 9 + dD[1][0] * 3 + dD[2][1] * 3 + dD[2][0]     ) >> 4;
        up_dD[3] = ( dD[1][1] * 9 + dD[1][2] * 3 + dD[2][1] * 3 + dD[2][2]     ) >> 4;

        dC = ( abs( lineTexRec1[max(ti-1,0)] - lineTexRec1[ti] ) + abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] ) )  * up_dD[0]
        + ( abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] ) + abs( lineTexRec1[min(ti+1,texFrameWidth-1)] - lineTexRec1[min(ti+2,texFrameWidth-1)] ) ) * up_dD[1]
        + ( abs( lineTexRec2[max(ti-1,0)] - lineTexRec2[ti] ) + abs( lineTexRec2[ti] - lineTexRec2[min(ti+1,texFrameWidth-1)] ) ) * up_dD[2]
        + ( abs( lineTexRec2[ti] - lineTexRec2[min(ti+1,texFrameWidth-1)] ) + abs( lineTexRec2[min(ti+1,texFrameWidth-1)] - lineTexRec2[min(ti+2,texFrameWidth-1)] ) ) * up_dD[3];

        temp = (distblk) ( dispCoeff * (dC>>3) );
        distortion += iabs2((int)temp);
      }
    }
  }

  return dist_scale(distortion);
}
#endif

distblk compute_SSE16x16_thres(imgpel **imgRef, imgpel **imgSrc, int xRef, int xSrc, distblk min_cost)
{
  int i, j;
  imgpel *lineRef, *lineSrc;
  distblk distortion = 0;
  int imin_cost = dist_down(min_cost);

  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {
    lineRef = &imgRef[j][xRef];    
    lineSrc = &imgSrc[j][xSrc];

    for (i = 0; i < MB_BLOCK_SIZE; i++)
      distortion += iabs2( *lineRef++ - *lineSrc++ );
    if (distortion > imin_cost)
      return (min_cost);
  }

  return dist_scale(distortion);
}

#if EXT3D
distblk compute_VSD16x16_thres(imgpel **imgRef, imgpel **imgSrc, imgpel **texRec, int xRef, int xSrc, distblk min_cost, double dispCoeff, int texFrameWidth, int depthFrameWidth )
{
  int i, j;
  imgpel *lineRef, *lineSrc, *lineTexRec1, *lineTexRec2;
  distblk distortion = 0, temp, dC;
  int ti;
  int imin_cost = dist_down(min_cost);

  if ( texFrameWidth == depthFrameWidth )
  {
    for (j = 0; j < MB_BLOCK_SIZE; j++)
    {
      lineRef = &imgRef[j][xRef];    
      lineSrc = &imgSrc[j][xSrc];
      lineTexRec1 = &texRec[j][0];

      for ( i = 0, ti = xSrc ; i < MB_BLOCK_SIZE; i++, ti++ )
      {
        dC = abs( lineTexRec1[max(ti-1,0)] - lineTexRec1[ti] ) + abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] );

        temp = (distblk) ( ( lineRef[i] - lineSrc[i] ) * dispCoeff * (dC>>1) );
        distortion += iabs2((int)temp);
      }

      if ( distortion > imin_cost )
        return (min_cost);

    }
  }

  else if ( texFrameWidth == (depthFrameWidth<<1) )
  {
    int dD[3][3], up_dD[4];
    imgpel *lineRef2, *lineSrc2, *lineRef0, *lineSrc0, *lineRef1, *lineSrc1;

    for ( j = 0; j < MB_BLOCK_SIZE; j++ )
    {
      lineRef1 = &imgRef[j][xRef];    
      lineSrc1 = &imgSrc[j][xSrc];

      lineRef0 = &imgRef[max(j-1,0)][xRef];    
      lineSrc0 = &imgSrc[max(j-1,0)][xSrc];

      lineRef2 = &imgRef[min(j+1,MB_BLOCK_SIZE-1)][xRef];    
      lineSrc2 = &imgSrc[min(j+1,MB_BLOCK_SIZE-1)][xSrc];

      lineTexRec1 = &texRec[j<<1][0];
      lineTexRec2 = &texRec[(j<<1)+1][0];

      for ( i = 0, ti = xSrc<<1 ; i < MB_BLOCK_SIZE; i++, ti+=2 )
      {
        dD[0][0] = lineSrc0[max(i-1,0)] - lineRef0[max(i-1,0)];
        dD[0][1] = lineSrc0[i] - lineRef0[i];
        dD[0][2] = lineSrc0[min(i+1,MB_BLOCK_SIZE-1)] - lineRef0[min(i+1,MB_BLOCK_SIZE-1)];

        dD[1][0] = lineSrc1[max(i-1,0)] - lineRef1[max(i-1,0)];
        dD[1][1] = lineSrc1[i] - lineRef1[i];
        dD[1][2] = lineSrc1[min(i+1,MB_BLOCK_SIZE-1)] - lineRef1[min(i+1,MB_BLOCK_SIZE-1)];

        dD[2][0] = lineSrc2[max(i-1,0)] - lineRef2[max(i-1,0)];
        dD[2][1] = lineSrc2[i] - lineRef2[i];
        dD[2][2] = lineSrc2[min(i+1,MB_BLOCK_SIZE-1)] - lineRef2[min(i+1,MB_BLOCK_SIZE-1)];

        up_dD[0] = ( dD[1][1] * 9 + dD[1][0] * 3 + dD[0][1] * 3 + dD[0][0]     ) >> 4;
        up_dD[1] = ( dD[1][1] * 9 + dD[0][1] * 3 + dD[1][2] * 3 + dD[0][2]     ) >> 4;
        up_dD[2] = ( dD[1][1] * 9 + dD[1][0] * 3 + dD[2][1] * 3 + dD[2][0]     ) >> 4;
        up_dD[3] = ( dD[1][1] * 9 + dD[1][2] * 3 + dD[2][1] * 3 + dD[2][2]     ) >> 4;

        dC = ( abs( lineTexRec1[max(ti-1,0)] - lineTexRec1[ti] ) + abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] ) )  * up_dD[0]
        + ( abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] ) + abs( lineTexRec1[min(ti+1,texFrameWidth-1)] - lineTexRec1[min(ti+2,texFrameWidth-1)] ) ) * up_dD[1]
        + ( abs( lineTexRec2[max(ti-1,0)] - lineTexRec2[ti] ) + abs( lineTexRec2[ti] - lineTexRec2[min(ti+1,texFrameWidth-1)] ) ) * up_dD[2]
        + ( abs( lineTexRec2[ti] - lineTexRec2[min(ti+1,texFrameWidth-1)] ) + abs( lineTexRec2[min(ti+1,texFrameWidth-1)] - lineTexRec2[min(ti+2,texFrameWidth-1)] ) ) * up_dD[3];

        temp = (distblk) ( dispCoeff * (dC>>3) );
        distortion += iabs2((int)temp);
      }

      if ( distortion > imin_cost )
        return (min_cost);
    }
  }

  return dist_scale(distortion);
}
#endif

/*!
 ***********************************************************************
 * \brief
 *    compute 8x8 SSE
 ***********************************************************************
 */
distblk compute_SSE8x8(imgpel **imgRef, imgpel **imgSrc, int xRef, int xSrc)
{
  int i, j;
  imgpel *lineRef, *lineSrc;
  distblk distortion = 0;

  for (j = 0; j < BLOCK_SIZE_8x8; j++)
  {
    lineRef = &imgRef[j][xRef];    
    lineSrc = &imgSrc[j][xSrc];

    for (i = 0; i < BLOCK_SIZE_8x8; i++)
      distortion += iabs2( *lineRef++ - *lineSrc++ );
  }

  return dist_scale(distortion);
}


#if EXT3D
distblk compute_VSD8x8(imgpel **imgRef, imgpel **imgSrc, imgpel **texRec, int xRef, int xSrc, double dispCoeff, int texFrameWidth, int depthFrameWidth )
{
  int i, j, ti;
  imgpel *lineRef, *lineSrc, *lineTexRec1, *lineTexRec2;
  distblk distortion = 0, temp, dC;

  if ( texFrameWidth == depthFrameWidth )
  {
    for ( j = 0; j < BLOCK_SIZE; j++ )
    {
      lineRef = &imgRef[j][xRef];    
      lineSrc = &imgSrc[j][xSrc];
      lineTexRec1 = &texRec[j][0];

      for ( i = 0, ti = xSrc ; i < BLOCK_SIZE; i++, ti++ )
      {
        dC = abs( lineTexRec1[max(ti-1,0)] - lineTexRec1[ti] ) + abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] );

        temp = (distblk) ( ( lineRef[i] - lineSrc[i] ) * dispCoeff * (dC>>1) );
        distortion += iabs2((int)temp);
      }
    }

  }

  else if ( texFrameWidth == (depthFrameWidth<<1) )
  {
    int dD[3][3], up_dD[4];
    imgpel *lineRef2, *lineSrc2, *lineRef0, *lineSrc0, *lineRef1, *lineSrc1;

    for ( j = 0; j < BLOCK_SIZE_8x8; j++ )
    {
      lineRef1 = &imgRef[j][xRef];    
      lineSrc1 = &imgSrc[j][xSrc];

      lineRef0 = &imgRef[max(j-1,0)][xRef];    
      lineSrc0 = &imgSrc[max(j-1,0)][xSrc];

      lineRef2 = &imgRef[min(j+1,BLOCK_SIZE_8x8-1)][xRef];    
      lineSrc2 = &imgSrc[min(j+1,BLOCK_SIZE_8x8-1)][xSrc];

      lineTexRec1 = &texRec[j<<1][0];
      lineTexRec2 = &texRec[(j<<1)+1][0];

      for ( i = 0, ti = xSrc<<1 ; i < BLOCK_SIZE_8x8; i++, ti+=2 )
      {
        dD[0][0] = lineSrc0[max(i-1,0)] - lineRef0[max(i-1,0)];
        dD[0][1] = lineSrc0[i] - lineRef0[i];
        dD[0][2] = lineSrc0[min(i+1,BLOCK_SIZE_8x8-1)] - lineRef0[min(i+1,BLOCK_SIZE_8x8-1)];

        dD[1][0] = lineSrc1[max(i-1,0)] - lineRef1[max(i-1,0)];
        dD[1][1] = lineSrc1[i] - lineRef1[i];
        dD[1][2] = lineSrc1[min(i+1,BLOCK_SIZE_8x8-1)] - lineRef1[min(i+1,BLOCK_SIZE_8x8-1)];

        dD[2][0] = lineSrc2[max(i-1,0)] - lineRef2[max(i-1,0)];
        dD[2][1] = lineSrc2[i] - lineRef2[i];
        dD[2][2] = lineSrc2[min(i+1,BLOCK_SIZE_8x8-1)] - lineRef2[min(i+1,BLOCK_SIZE_8x8-1)];

        up_dD[0] = ( dD[1][1] * 9 + dD[1][0] * 3 + dD[0][1] * 3 + dD[0][0]     ) >> 4;
        up_dD[1] = ( dD[1][1] * 9 + dD[0][1] * 3 + dD[1][2] * 3 + dD[0][2]     ) >> 4;
        up_dD[2] = ( dD[1][1] * 9 + dD[1][0] * 3 + dD[2][1] * 3 + dD[2][0]     ) >> 4;
        up_dD[3] = ( dD[1][1] * 9 + dD[1][2] * 3 + dD[2][1] * 3 + dD[2][2]     ) >> 4;

        dC = ( abs( lineTexRec1[max(ti-1,0)] - lineTexRec1[ti] ) + abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] ) )  * up_dD[0]
        + ( abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] ) + abs( lineTexRec1[min(ti+1,texFrameWidth-1)] - lineTexRec1[min(ti+2,texFrameWidth-1)] ) ) * up_dD[1]
        + ( abs( lineTexRec2[max(ti-1,0)] - lineTexRec2[ti] ) + abs( lineTexRec2[ti] - lineTexRec2[min(ti+1,texFrameWidth-1)] ) ) * up_dD[2]
        + ( abs( lineTexRec2[ti] - lineTexRec2[min(ti+1,texFrameWidth-1)] ) + abs( lineTexRec2[min(ti+1,texFrameWidth-1)] - lineTexRec2[min(ti+2,texFrameWidth-1)] ) ) * up_dD[3];

        temp = (distblk) ( dispCoeff * (dC>>3) );
        distortion += iabs2((int)temp);
      }
    }
  }

  return dist_scale(distortion);
}
#endif

/*!
 ***********************************************************************
 * \brief
 *    compute 4x4 SSE
 ***********************************************************************
 */
distblk compute_SSE4x4(imgpel **imgRef, imgpel **imgSrc, int xRef, int xSrc)
{
  int i, j;
  imgpel *lineRef, *lineSrc;
  distblk distortion = 0;

  for (j = 0; j < BLOCK_SIZE; j++)
  {
    lineRef = &imgRef[j][xRef];    
    lineSrc = &imgSrc[j][xSrc];

    for (i = 0; i < BLOCK_SIZE; i++)
      distortion += iabs2( *lineRef++ - *lineSrc++ );
  }

  return dist_scale(distortion);
}


#if EXT3D
distblk compute_VSD_FlexDepth_UN(VideoParameters *p_Vid,imgpel **imgRef, imgpel **imgSrc, imgpel **texRec, int xRef, int xSrc, double dispCoeff, int texFrameWidth, int blk_size, int threshold, distblk min_cost, int grid_posx)
{
  int i, j,k,p,ti;
  imgpel *lineRef, *lineSrc, *lineTexRec[32],*lineTexRec1,*lineTexRec2;
  distblk distortion = 0, temp, dC;
  int imin_cost = dist_down(min_cost);
  int xRatio=( (1<<p_Vid->depth_hor_rsh)+ (p_Vid->depth_hor_mult>>1) )/p_Vid->depth_hor_mult;
  int yRatio=( (1<<p_Vid->depth_ver_rsh)+ (p_Vid->depth_ver_mult>>1) )/p_Vid->depth_ver_mult;

  int dD[3][3], up_dD[4];
  imgpel *lineRef2, *lineSrc2, *lineRef0, *lineSrc0, *lineRef1, *lineSrc1;

  if ((1<<p_Vid->depth_ver_rsh)==(p_Vid->depth_ver_mult<<1) && (1<<p_Vid->depth_hor_rsh)==(p_Vid->depth_hor_mult<<1))
  {
    for ( j = 0; j < blk_size; j++ )
    {
      lineRef1 = &imgRef[j][xRef];    
      lineSrc1 = &imgSrc[j][xSrc];

      lineRef0 = &imgRef[max(j-1,0)][xRef];    
      lineSrc0 = &imgSrc[max(j-1,0)][xSrc];

      lineRef2 = &imgRef[min(j+1,blk_size-1)][xRef];    
      lineSrc2 = &imgSrc[min(j+1,blk_size-1)][xSrc];

      lineTexRec1 = &texRec[j<<1][0];
      lineTexRec2 = &texRec[(j<<1)+1][0];

      for ( i = 0, ti = (xSrc<<1)-grid_posx; i < blk_size; i++, ti+=2 )
      {
        dD[0][0] = lineSrc0[max(i-1,0)] - lineRef0[max(i-1,0)];
        dD[0][1] = lineSrc0[i] - lineRef0[i];
        dD[0][2] = lineSrc0[min(i+1,blk_size-1)] - lineRef0[min(i+1,blk_size-1)];

        dD[1][0] = lineSrc1[max(i-1,0)] - lineRef1[max(i-1,0)];
        dD[1][1] = lineSrc1[i] - lineRef1[i];
        dD[1][2] = lineSrc1[min(i+1,blk_size-1)] - lineRef1[min(i+1,blk_size-1)];

        dD[2][0] = lineSrc2[max(i-1,0)] - lineRef2[max(i-1,0)];
        dD[2][1] = lineSrc2[i] - lineRef2[i];
        dD[2][2] = lineSrc2[min(i+1,blk_size-1)] - lineRef2[min(i+1,blk_size-1)];

        up_dD[0] = ( dD[1][1] * 9 + dD[1][0] * 3 + dD[0][1] * 3 + dD[0][0]     ) >> 4;
        up_dD[1] = ( dD[1][1] * 9 + dD[0][1] * 3 + dD[1][2] * 3 + dD[0][2]     ) >> 4;
        up_dD[2] = ( dD[1][1] * 9 + dD[1][0] * 3 + dD[2][1] * 3 + dD[2][0]     ) >> 4;
        up_dD[3] = ( dD[1][1] * 9 + dD[1][2] * 3 + dD[2][1] * 3 + dD[2][2]     ) >> 4;


        dC = ( abs( lineTexRec1[max(ti-1,0)] - lineTexRec1[ti] ) + abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] ) )  * up_dD[0]
        + ( abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] ) + abs( lineTexRec1[min(ti+1,texFrameWidth-1)] - lineTexRec1[min(ti+2,texFrameWidth-1)] ) ) * up_dD[1]
        + ( abs( lineTexRec2[max(ti-1,0)] - lineTexRec2[ti] ) + abs( lineTexRec2[ti] - lineTexRec2[min(ti+1,texFrameWidth-1)] ) ) * up_dD[2]
        + ( abs( lineTexRec2[ti] - lineTexRec2[min(ti+1,texFrameWidth-1)] ) + abs( lineTexRec2[min(ti+1,texFrameWidth-1)] - lineTexRec2[min(ti+2,texFrameWidth-1)] ) ) * up_dD[3];


        temp = (distblk) ( dispCoeff * (dC>>3) );
        distortion += iabs2((int)temp);
      }

      if ( threshold && distortion > imin_cost )
        return (min_cost);

    }
  }
  else
  {
    for (j = 0; j < blk_size; j++)
    {
      int jj0=( j*(1<<p_Vid->depth_ver_rsh)+(p_Vid->depth_ver_mult>>1) )/p_Vid->depth_ver_mult;
      lineRef = &imgRef[j][xRef];    
      lineSrc = &imgSrc[j][xSrc];
      for(k=0;k<yRatio;k++)
      {
        //lineTexRec[k]=&texRec[j*yRatio+k][0];
        lineTexRec[k]=&texRec[jj0+k][0];
      }

      // for (i = 0, ti = xSrc*xRatio-grid_posx ; i < blk_size; i++, ti+=xRatio )
      for (i = 0 ; i < blk_size; i++ )
      {
        dC=0;
        ti = ( (xSrc+i)*(1<<p_Vid->depth_hor_rsh)+(p_Vid->depth_hor_mult>>1) )/p_Vid->depth_hor_mult-grid_posx;
        for(k=0;k<yRatio;k++)
        {
          dC += abs( lineTexRec[k][max(ti-1,0)] - lineTexRec[k][ti] ) +  abs(lineTexRec[k][ti+xRatio-1]-lineTexRec[k][min(ti+xRatio,texFrameWidth-1)] );
          for(p=ti;p<ti+xRatio-1;p++)
            dC+= abs( lineTexRec[k][p] - lineTexRec[k][min(p+1,texFrameWidth-1)] ) << 1 ;
        }

        temp = (distblk) ( ( lineRef[i] - lineSrc[i] ) * dispCoeff * (dC/(2*xRatio*yRatio)) );
        distortion += (distblk)iabs2( (int) temp);
      }

      if ( threshold && distortion > imin_cost )
        return (min_cost);
    }
  }
  return dist_scale(distortion);
}

distblk compute_VSD4x4(imgpel **imgRef, imgpel **imgSrc, imgpel **texRec, int xRef, int xSrc, double dispCoeff, int texFrameWidth, int depthFrameWidth )
{
  int i, j, ti;
  imgpel *lineRef, *lineSrc, *lineTexRec1, *lineTexRec2;
  distblk distortion = 0, temp, dC;

  if ( texFrameWidth == depthFrameWidth )
  {
    for (j = 0; j < BLOCK_SIZE; j++)
    {
      lineRef = &imgRef[j][xRef];    
      lineSrc = &imgSrc[j][xSrc];
      lineTexRec1 = &texRec[j][0];

      for ( i = 0, ti = xSrc ; i < BLOCK_SIZE; i++, ti++ )
      {
        dC = abs( lineTexRec1[max(ti-1,0)] - lineTexRec1[ti] ) + abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] );

        temp = (distblk) ( ( lineRef[i] - lineSrc[i] ) * dispCoeff * (dC>>1) );
        distortion += iabs2((int)temp);
      }
    }

  }

  else if ( texFrameWidth == (depthFrameWidth<<1) )
  {
    int dD[3][3], up_dD[4];
    imgpel *lineRef2, *lineSrc2, *lineRef0, *lineSrc0, *lineRef1, *lineSrc1;

    for ( j = 0; j < BLOCK_SIZE; j++ )
    {
      lineRef1 = &imgRef[j][xRef];    
      lineSrc1 = &imgSrc[j][xSrc];

      lineRef0 = &imgRef[max(j-1,0)][xRef];    
      lineSrc0 = &imgSrc[max(j-1,0)][xSrc];

      lineRef2 = &imgRef[min(j+1,BLOCK_SIZE-1)][xRef];    
      lineSrc2 = &imgSrc[min(j+1,BLOCK_SIZE-1)][xSrc];

      lineTexRec1 = &texRec[j<<1][0];
      lineTexRec2 = &texRec[(j<<1)+1][0];

      for ( i = 0, ti = xSrc<<1 ; i < BLOCK_SIZE; i++, ti+=2 )
      {
        dD[0][0] = lineSrc0[max(i-1,0)] - lineRef0[max(i-1,0)];
        dD[0][1] = lineSrc0[i] - lineRef0[i];
        dD[0][2] = lineSrc0[min(i+1,BLOCK_SIZE-1)] - lineRef0[min(i+1,BLOCK_SIZE-1)];

        dD[1][0] = lineSrc1[max(i-1,0)] - lineRef1[max(i-1,0)];
        dD[1][1] = lineSrc1[i] - lineRef1[i];
        dD[1][2] = lineSrc1[min(i+1,BLOCK_SIZE-1)] - lineRef1[min(i+1,BLOCK_SIZE-1)];

        dD[2][0] = lineSrc2[max(i-1,0)] - lineRef2[max(i-1,0)];
        dD[2][1] = lineSrc2[i] - lineRef2[i];
        dD[2][2] = lineSrc2[min(i+1,BLOCK_SIZE-1)] - lineRef2[min(i+1,BLOCK_SIZE-1)];

        up_dD[0] = ( dD[1][1] * 9 + dD[1][0] * 3 + dD[0][1] * 3 + dD[0][0]     ) >> 4;
        up_dD[1] = ( dD[1][1] * 9 + dD[0][1] * 3 + dD[1][2] * 3 + dD[0][2]     ) >> 4;
        up_dD[2] = ( dD[1][1] * 9 + dD[1][0] * 3 + dD[2][1] * 3 + dD[2][0]     ) >> 4;
        up_dD[3] = ( dD[1][1] * 9 + dD[1][2] * 3 + dD[2][1] * 3 + dD[2][2]     ) >> 4;

        dC = ( abs( lineTexRec1[max(ti-1,0)] - lineTexRec1[ti] ) + abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] ) )  * up_dD[0]
        + ( abs( lineTexRec1[ti] - lineTexRec1[min(ti+1,texFrameWidth-1)] ) + abs( lineTexRec1[min(ti+1,texFrameWidth-1)] - lineTexRec1[min(ti+2,texFrameWidth-1)] ) ) * up_dD[1]
        + ( abs( lineTexRec2[max(ti-1,0)] - lineTexRec2[ti] ) + abs( lineTexRec2[ti] - lineTexRec2[min(ti+1,texFrameWidth-1)] ) ) * up_dD[2]
        + ( abs( lineTexRec2[ti] - lineTexRec2[min(ti+1,texFrameWidth-1)] ) + abs( lineTexRec2[min(ti+1,texFrameWidth-1)] - lineTexRec2[min(ti+2,texFrameWidth-1)] ) ) * up_dD[3];

        temp = (distblk) ( dispCoeff * (dC>>3) );
        distortion += iabs2((int)temp);
      }
    }
  }

  return dist_scale(distortion);
}
#endif

/*!
*************************************************************************************
* \brief
*    SSE distortion calculation for a macroblock
*************************************************************************************
*/
distblk distortionSSE(Macroblock *currMB) 
{
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;
  distblk distortionY = 0;
  distblk distortionCr[2] = {0, 0};

  // LUMA
#if EXT3D
  if( p_Inp->VSD && p_Vid->is_depth )
  {
    distblk distortionVSD, distortionD;
    imgpel** img_Texture;
    int cur_view_id=currMB->p_Slice->view_id;
    int grid_posy=p_Vid->grid_pos_y[cur_view_id];
    int grid_posx=p_Vid->grid_pos_x[cur_view_id];
    int tex_pic_y = (currMB->opix_y<<p_Vid->depth_ver_rsh)/p_Vid->depth_ver_mult-grid_posy;
    if ( p_Vid->p_dual_picture )
      img_Texture = p_Vid->p_dual_picture->imgY;  // available reconstructed texture
    else
      img_Texture = p_Vid->p_DualVid->imgData0.frm_data[0];  // original texture

    distortionD = compute_SSE16x16(&p_Vid->pCurImg[currMB->opix_y], &p_Vid->enc_picture->p_curr_img[currMB->pix_y], currMB->pix_x, currMB->pix_x);

    if (currMB->pix_x>=p_Vid->DepthCropLeftCoord && (currMB->pix_x+15)<=p_Vid->DepthCropRightCoord && currMB->opix_y>=p_Vid->DepthCropTopCoord && (currMB->opix_y +15)<=p_Vid->DepthCropBottomCoord )
    {
      distortionVSD = compute_VSD_FlexDepth_UN(p_Vid,&p_Vid->pCurImg[currMB->opix_y], &p_Vid->enc_picture->p_curr_img[currMB->pix_y], &img_Texture[tex_pic_y], currMB->pix_x, currMB->pix_x, p_Inp->dispCoeff, p_Inp->OriginalWidth,16,0,0,grid_posx);
    }else
    {
      distortionVSD=distortionD;
    }

    distortionY = ( p_Inp->vsdWeight * distortionVSD + p_Inp->dWeight * distortionD ) / ( p_Inp->vsdWeight + p_Inp->dWeight );
  }
  else
#endif
  distortionY = compute_SSE16x16(&p_Vid->pCurImg[currMB->opix_y], &p_Vid->enc_picture->p_curr_img[currMB->pix_y], currMB->pix_x, currMB->pix_x);

  // CHROMA
  if ((p_Vid->yuv_format != YUV400) && (p_Inp->separate_colour_plane_flag == 0))
  {
    distortionCr[0] = compute_SSE_cr(&p_Vid->pImgOrg[1][currMB->opix_c_y], &p_Vid->enc_picture->imgUV[0][currMB->pix_c_y], currMB->pix_c_x, currMB->pix_c_x, p_Vid->mb_cr_size_y, p_Vid->mb_cr_size_x);
    distortionCr[1] = compute_SSE_cr(&p_Vid->pImgOrg[2][currMB->opix_c_y], &p_Vid->enc_picture->imgUV[1][currMB->pix_c_y], currMB->pix_c_x, currMB->pix_c_x, p_Vid->mb_cr_size_y, p_Vid->mb_cr_size_x);
  }
#if JCOST_OVERFLOWCHECK //overflow checking;
  if(distortionY * p_Inp->WeightY + distortionCr[0] * p_Inp->WeightCb + distortionCr[1] * p_Inp->WeightCr > DISTBLK_MAX)
  {
    printf("Overflow: %s : %d \n MB: %d, Value: %lf\n", __FILE__, __LINE__, currMB->mbAddrX, (distortionY * p_Inp->WeightY + distortionCr[0] * p_Inp->WeightCb + distortionCr[1] * p_Inp->WeightCr));
    exit(-1);
  }
#endif  //end;
  return (distblk)( distortionY * p_Inp->WeightY + distortionCr[0] * p_Inp->WeightCb + distortionCr[1] * p_Inp->WeightCr );
}

