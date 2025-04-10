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



#ifndef INCLUDE_YUV_FILE_PARAMS
#define INCLUDE_YUV_FILE_PARAMS

#include <string>

struct YUVFileParams {
  YUVFileParams():_height(0), _width(0) {}

  //YUVFileParams(const std::string& fileName, 
  //		const int height, const int width);
  YUVFileParams(const std::string& fileName, const int view_id, 
		const int height, const int vertPadding, 
		const int width, const int horPadding);
  YUVFileParams( const YUVFileParams& other); //copy constructor

  std::string     _fileName;  // name of file to read from
  unsigned int	  _view_id; // view-id used in SPS	
  unsigned int    _height;    // height of an image frame
  unsigned int    _width;     // width of an image frame
  unsigned int    _lumaSize;  // number of pixels in image frame include
                              // padding for margins
  unsigned int    _bufSize;   // total size of buffer for luma and chroma
  unsigned int    _lumaOffset;// point in buffer for first luma pixel
  unsigned int    _cbOffset;  // point in buffer for first Cb pixel
  unsigned int    _crOffset;  // point in buffer for first Cr pixel
  unsigned int    _stride;    // how far to move in buffer to get from luma
                              // pixel at position (x,y) to (x,y+1)
};

#endif
