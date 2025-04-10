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

#include <stdio.h>
#include <stdlib.h>


typedef unsigned char uchar;

void
OutDifference( int frame, char fld, int count, int width, uchar test, uchar ref, int chroma )
{
  int y   = count/width;
  int x   = count%width;
  int s   = ( chroma ? 3 : 4 );
  int mb  = (x>>s) + (y>>s) * (width>>s);

  fprintf( stdout, "FRM=%4d COMP=%c POS=(%4d,%4d) MB=%6d  ==>  test=%3d != ref=%3d\n",
    frame, fld, y, x, mb, test, ref );
}


int
main( int argc, char** argv )
{
  int     width, height, fsize, ysize, csize, xsize, maxcount=0, ignoreframes=0;
  int     frame, i, count;
  FILE    *testfile, *reffile;
  uchar   *buft, *bufr;
  size_t  read_size_t;
  size_t  read_size_r;

  if( argc != 5 && argc != 6 && argc != 7 )
  {
    fprintf( stderr, "\nUsage: %s (width) (height) (test.yuv) (ref.yuv) [(max_count)=0 [(ignore_frames]=0]\n\n", "YUVCompare" );
    return 1;
  }

  width   = atoi( argv[1] );
  height  = atoi( argv[2] );

  if( argc >= 6 )
  {
    maxcount = atoi( argv[5] );
  }
  if( argc >= 7 )
  {
    ignoreframes = atoi( argv[6] );
  }

  if( width < 0 || width % 2 || height < 0 || height % 2 )
  {
    fprintf (stderr, "\nUnvalid image dimensions %dx%d (width=\"%s\", height=\"%s\")!\n\n", width, height, argv[1], argv[2]);
    return 1;
  }

  ysize = width*height;
  csize = ysize>>2;
  xsize = ysize+csize;
  fsize = xsize+csize;
  buft  = (uchar*) malloc (sizeof(uchar)*fsize);
  bufr  = (uchar*) malloc (sizeof(uchar)*fsize);
  if( buft == NULL || bufr == NULL )
  {
    fprintf( stderr, "\nMemory exceeded!\n\n" );
    return 1;
  }

  if( ( testfile = fopen( argv[3], "rb" ) ) == NULL )
  {
    fprintf( stderr, "\nCannot open file \"%s\"!\n\n", argv[3] );
    return 1;
  }
  if ((reffile = fopen (argv[4], "rb")) == NULL)
  {
    fprintf( stderr, "\nCannot open file \"%s\"!\n\n", argv[4] );
    return 1;
  }

#define COUNT_OK ( !maxcount || count < maxcount )

  frame = count = 0;
  while( ( ( ( read_size_t = fread( buft, sizeof(uchar), fsize, testfile ) ) +
             ( read_size_r = fread( bufr, sizeof(uchar), fsize,  reffile ) )   ) == (size_t)2*fsize ) && COUNT_OK )
  {
    if( frame >= ignoreframes )
    {
      for( i=0; i<ysize && COUNT_OK; i++ )
      {
        if (buft[i] != bufr[i])
        {
          OutDifference (frame, 'Y', i,       width,   buft[i], bufr[i], 0);
          count++;
        }
      }
      for (   ; i<xsize && COUNT_OK; i++)
      {
        if (buft[i] != bufr[i])
        {
          OutDifference (frame, 'U', i-ysize, width/2, buft[i], bufr[i], 1);
          count++;
        }
      }
      for (   ; i<fsize && COUNT_OK; i++)
      {
        if (buft[i] != bufr[i])
        {
          OutDifference (frame, 'V', i-xsize, width/2, buft[i], bufr[i], 1);
          count++;
        }
      }
    }
    frame++;
  }

  if( !count && ignoreframes == 0 )
  {
    if( read_size_t == read_size_r )
    {
      fprintf (stderr, "\nFiles \"%s\" and \"%s\" are identical (%d frames)!\n\n", argv[3], argv[4], frame);
    }
    else if ( read_size_t < read_size_r )
    {
      fprintf (stderr, "\nFile \"%s\" is shorter (%d frames) than file \"%s\"!\n\n", argv[3], frame, argv[4]);
    }
    else
    {
      fprintf (stderr, "\nFile \"%s\" is shorter (%d frames) than file \"%s\"!\n\n", argv[4], frame, argv[3]);
    }
  }

  free   (buft);
  free   (bufr);
  fclose (testfile);
  fclose (reffile);
  return 0;
}


