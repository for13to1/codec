/*
***********************************************************************
* COPYRIGHT AND WARRANTY INFORMATION
*
* Copyright 2001-2014, International Telecommunications Union, Geneva
*
* DISCLAIMER OF WARRANTY
*
* These software programs are available to the user without any
* license fee or royalty on an "as is" basis. The ITU disclaims
* any and all warranties, whether express, implied, or
* statutory, including any implied warranties of merchantability
* or of fitness for a particular purpose.  In no event shall the
* contributor or the ITU be liable for any incidental, punitive, or
* consequential damages of any kind whatsoever arising from the
* use of these programs.
*
* This disclaimer of warranty extends to the user of these programs
* and user's customers, employees, agents, transferees, successors,
* and assigns.
*
* The ITU does not represent or warrant that the programs furnished
* hereunder are free of infringement of any third-party patents.
* Commercial implementations of ITU-T Recommendations, including
* shareware, may be subject to royalty fees to patent holders.
* Information regarding the ITU-T patent policy is available from
* the ITU Web site at http://www.itu.int.
*
* THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.
************************************************************************
*/

// rtp_loss.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#ifdef WIN32
#include <Winsock2.h>
#else
#include <netinet/in.h>
#endif

void print_usage(char *argv [])
{
  printf ("Usage: %s input_file output_file loss_percent <keep_leading_packets>\n", argv[0]);
  exit (-1);
}

int keep_packet(int loss_percent)
{
  int rnd;
  if (loss_percent>100)
    return 1;
  if (loss_percent<=0)
    return 0;
  
  rnd = int (100 * (((float) rand()) / RAND_MAX));

  return (rnd >= loss_percent );
}

int main(int argc, char* argv[])
{
  unsigned int bufsize, pacno=0;
  unsigned char buf[65000];
  int i, intime;
  FILE *fr;                                // file for reading
  FILE *fw;                                // file for writing


  if ((argc != 4) && (argc != 5))
  {
    print_usage (argv);
  }

  if (NULL == (fr = fopen (argv[1], "rb")))
  {
    printf ("%s: cannot open H.264 packet file %s for reading\n", argv[0], argv[1]);
    return -2;
  }

  if (NULL == (fw = fopen (argv[2], "wb")))
  {
    printf ("%s: cannot open H.264 packet file %s for reading\n", argv[0], argv[1]);
    fclose (fr);
    return -2;
  }


  if (argc==5)
  {
    for (i=0; i< atoi (argv[4]); i++)
    {
      if (4 != fread (&bufsize, 1, 4, fr))
        return 0;
      if (4 != fread (&intime, 1, 4, fr))
      {
        printf ("Panic, cannot read timestamp, old software version file?\n");
        return -1;
      }
      if (bufsize != fread (buf, 1, bufsize, fr))
      {
        printf ("Problems while reading buffer, exit\n");
        return -3;
      }

      if (4 != fwrite (&bufsize, 1, 4, fw))
      {
        printf ("Problems while writing buffer size, exit\n");
        return -1;
      }
      if (4 != fwrite (&intime, 1, 4, fw))
      {
        printf ("Problems while writing timestamp, exit\n");
        return -1;
      }
      if (bufsize != fwrite (buf, 1, bufsize, fw))
      {
        printf ("Problems while writing buffer, exit\n");
        return -3;
      }
      pacno++;
    }

  }

  for (;;) 
  {
    if (4 != fread (&bufsize, 1, 4, fr))
      return 0;
    if (4 != fread (&intime, 1, 4, fr))
    {
      printf ("Panic, cannot read timestamp, old software version file?\n");
      return -1;
    }
    if (bufsize != fread (buf, 1, bufsize, fr))
    {
      printf ("Problems while reading buffer, exit\n");
      return -3;
    }
    if (keep_packet(atoi (argv[3])))
    {
      if (4 != fwrite (&bufsize, 1, 4, fw))
      {
        printf ("Problems while writing buffer size, exit\n");
        return -1;
      }
      if (4 != fwrite (&intime, 1, 4, fw))
      {
        printf ("Problems while writing timestamp, exit\n");
        return -1;
      }
      if (bufsize != fwrite (buf, 1, bufsize, fw))
      {
        printf ("Problems while writing buffer, exit\n");
        return -3;
      }
    }
    else
    {
      printf ("lost packet #%d\n", pacno);
    }
    pacno++;

  }
}
