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
 *************************************************************************************
 * \file img_io.h
 *
 * \brief
 *    image I/O related functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Alexis Michael Tourapis         <alexismt@ieee.org>
 *************************************************************************************
 */
#include "global.h"

#ifndef _IMG_IO_H_
#define _IMG_IO_H_

#include "io_video.h"
#include "io_raw.h"
#include "io_tiff.h"

extern int ParseSizeFromString           (VideoDataFile *input_file, int *xlen, int *ylen, double *fps);
extern void ParseFrameNoFormatFromString (VideoDataFile *input_file);
extern void OpenFrameFile                (VideoDataFile *input_file, int FrameNumberInFile);
extern void OpenFiles                    (VideoDataFile *input_file);
extern void CloseFiles                   (VideoDataFile *input_file);
extern VideoFileType ParseVideoType      (VideoDataFile *input_file);

#endif

