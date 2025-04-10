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



#include "global.h"
#include "memalloc.h"

#if EXT3D // @DT: Enclose all code under this file

#include "3dv_acquisition.h"

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))


int32 getVarLength(int32 exponent, int32 precision) 
{
  if (exponent==0)
    return max(0, -30+precision);
  else 
    return max(0, exponent-31+precision);
}

void get_exponent_mantissa(ThreeDVAE* threeDV_ae,int voidx)
{
  double arg=fabs(threeDV_ae->original[voidx]);
  double log_val;
  int32 expo,v=0,i;
  double mant;
  int32 exponnent=0;
  int32 mantissa=0;
  int32 bits=0;

  if(threeDV_ae->pred_mode)
  {
    int32 bit;
    int mant_prec=threeDV_ae->precision;
    /* Find expo & mant when arg=(-1)^sign*2^expo*(0.mant),*/
    if (arg==0.0)
    {
      expo=0;
      mant=0;
    } 
    else 
    {
      log_val= log(arg)/log(2.0);
      expo = (int)floor(log_val);
      mant = pow(2.0, log_val - expo);
      if (fabs(mant) > 1.0) 
      {
        mant = mant / (double)2.0;
        expo= expo + 1;
      }    
    }

    /* Convert expo & mant so that X=(-1)^s*2^expo*(1.mant) */
    if (expo == 0 && mant ==0.0 ) 
    {
      v=0;
    } 
    else if (expo> -30)
    { 
      while ((double)fabs(mant)<(double)1.0) 
      {
        mant=mant*2.0;
        expo=expo-1;
      }
      if (mant>=0.0)
        mant = mant-1.0;
      v= expo + mant_prec; // number of necessary mantissa bits in the case of truncation
      if (v<0)
        v=0;
      expo += 31;
    } 
    else if (expo == -30) 
    {
      v= expo + mant_prec; // number of necessary mantissa bits in the case of truncation
      if (v<0) 
        v=0;
      expo=0;
    }

    exponnent = expo;
    if(v>30)
      v=30;

    /* Convert a float number mant (0<=mant<1) into a binary representation N which is a mantissa with v bits */
    mantissa=0;


    for (i=1; i<=v; i++ )
    {
      bit = (2.0*mant >= 1.0);
      mant = 2.0*mant - (double)bit;
      mantissa = (mantissa << 1 ) | bit ; // MSB of M corresponds to 1/2 and MSB-1 to 1/4 and so on.
    }
    bits=v;
  }
  else
  {
    int32 mantissa_len=threeDV_ae->mantissa_length;
    log_val= log(arg)/log(2.0);
    expo = (int)floor(log_val);
    exponnent=expo+31;

    mantissa=(int32)floor((arg/pow(2,expo)-1)*pow(2,mantissa_len)+0.5);
    bits=mantissa_len;
  }
  threeDV_ae->element[voidx].exponent=exponnent;
  threeDV_ae->element[voidx].mantissa=mantissa;
  threeDV_ae->element[voidx].mantissa_bits=bits;
}
void get_rec_double_type(ThreeDVAE* threeDV_ae, int voidx)
{
  int32 i;
  int32 sgn= (threeDV_ae->element[voidx].sign==0) ? 1 : -1;
  int32 exponent=threeDV_ae->element[voidx].exponent;
  int32 mantissa=threeDV_ae->element[voidx].mantissa;
  int32 exponent_size=(1<<threeDV_ae->exponent_size)-1;
  double *recon=&(threeDV_ae->rec[voidx]);

  assert(exponent!=exponent_size);

  if(threeDV_ae->pred_mode)
  {
    int32 length=getVarLength(exponent,threeDV_ae->precision);


    double factor=1.0;

    if(length>30)
      length=30;
    for(i=0;i<length;i++)
      factor=factor/2;


    *recon=0.0;



    for (i=0;i<(int)length;i++) 
    {
      *recon += factor*((mantissa>>i)&0x01);
      factor *= (double)2.0;
    }
    if (exponent>0 && exponent<exponent_size)
      *recon = (double)sgn*(double)pow((double)2,(double)exponent-31)*(1.0+*recon);
    if (exponent==0)
      *recon = (double)sgn*(double)pow((double)2,(double)-30)*(*recon);  
  }
  else
  {
    int32 mantissa_len=threeDV_ae->mantissa_length;
    *recon=sgn*pow(2,exponent-31)*(1+mantissa/pow(2,mantissa_len));
  }
}
int get_mem_acquisition_info(ThreeDVAcquisitionInfo** threeDV_acquisition_info)
{
  *threeDV_acquisition_info=calloc(1,sizeof(ThreeDVAcquisitionInfo));
  if(*threeDV_acquisition_info==NULL)
    no_mem_exit("get_mem_acquisition_info:*threeDV_acquisition_info");
  (*threeDV_acquisition_info)->focal_length_x_ae=calloc(1,sizeof(ThreeDVAE));
  if((*threeDV_acquisition_info)->focal_length_x_ae==NULL)
    no_mem_exit("get_mem_acquisition_info:(*threeDV_acquisition_info)->focal_length_x_ae");
  (*threeDV_acquisition_info)->focal_length_y_ae=calloc(1,sizeof(ThreeDVAE));
  if((*threeDV_acquisition_info)->focal_length_y_ae==NULL)
    no_mem_exit("get_mem_acquisition_info:(*threeDV_acquisition_info)->focal_length_y_ae");

  (*threeDV_acquisition_info)->principal_point_x_ae=calloc(1,sizeof(ThreeDVAE));
  if((*threeDV_acquisition_info)->principal_point_x_ae==NULL)
    no_mem_exit("get_mem_acquisition_info:(*threeDV_acquisition_info)->principal_point_x_ae");
  (*threeDV_acquisition_info)->principal_point_y_ae=calloc(1,sizeof(ThreeDVAE));
  if((*threeDV_acquisition_info)->principal_point_y_ae==NULL)
    no_mem_exit("get_mem_acquisition_info:(*threeDV_acquisition_info)->principal_point_y_ae");

  (*threeDV_acquisition_info)->translation_ae=calloc(1,sizeof(ThreeDVAE));
  if((*threeDV_acquisition_info)->translation_ae==NULL)
    no_mem_exit("get_mem_acquisition_infO:(*threeDV_acquisition_info)->translation_ae");

  (*threeDV_acquisition_info)->depth_near_ae=calloc(1,sizeof(ThreeDVAE));
  if((*threeDV_acquisition_info)->depth_near_ae==NULL)
    no_mem_exit("get_mem_acquisition_infO:(*threeDV_acquisition_info)->depth_near_ae");
  (*threeDV_acquisition_info)->depth_far_ae=calloc(1,sizeof(ThreeDVAE));
  if((*threeDV_acquisition_info)->depth_far_ae==NULL)
    no_mem_exit("get_mem_acquisition_infO:(*threeDV_acquisition_info)->depth_far_ae");
  return sizeof(ThreeDVAcquisitionInfo)+7*sizeof(ThreeDVAE);
}

void init_acquisition_info(ThreeDVAcquisitionInfo* ThreeDV_acquisition_info)
{
  memset(ThreeDV_acquisition_info->focal_length_x_ae,0x00,sizeof(ThreeDVAE));
  memset(ThreeDV_acquisition_info->focal_length_y_ae,0x00,sizeof(ThreeDVAE));
  memset(ThreeDV_acquisition_info->principal_point_x_ae,0x00,sizeof(ThreeDVAE));
  memset(ThreeDV_acquisition_info->principal_point_y_ae,0x00,sizeof(ThreeDVAE));
  memset(ThreeDV_acquisition_info->translation_ae,0x00,sizeof(ThreeDVAE));
  memset(ThreeDV_acquisition_info->depth_near_ae,0x00,sizeof(ThreeDVAE));
  memset(ThreeDV_acquisition_info->depth_far_ae,0x00,sizeof(ThreeDVAE));
  ThreeDV_acquisition_info->focal_length_x_ae->pred_mode=1;
  ThreeDV_acquisition_info->focal_length_x_ae->delta_flag=0;
  ThreeDV_acquisition_info->focal_length_x_ae->exponent_size=6;

  ThreeDV_acquisition_info->focal_length_y_ae->pred_mode=1;
  ThreeDV_acquisition_info->focal_length_y_ae->delta_flag=0;
  ThreeDV_acquisition_info->focal_length_y_ae->exponent_size=6;

  ThreeDV_acquisition_info->principal_point_x_ae->pred_mode=1;
  ThreeDV_acquisition_info->principal_point_x_ae->delta_flag=0;
  ThreeDV_acquisition_info->principal_point_x_ae->exponent_size=6;

  ThreeDV_acquisition_info->principal_point_y_ae->pred_mode=1;
  ThreeDV_acquisition_info->principal_point_y_ae->delta_flag=0;
  ThreeDV_acquisition_info->principal_point_y_ae->exponent_size=6;

  ThreeDV_acquisition_info->translation_ae->pred_mode=1;
  ThreeDV_acquisition_info->translation_ae->delta_flag=1;
  ThreeDV_acquisition_info->translation_ae->exponent_size=6;

  ThreeDV_acquisition_info->depth_near_ae->pred_mode=0;
  ThreeDV_acquisition_info->depth_near_ae->delta_flag=0;
  ThreeDV_acquisition_info->depth_near_ae->exponent_size=7;

  ThreeDV_acquisition_info->depth_far_ae->pred_mode=0;
  ThreeDV_acquisition_info->depth_far_ae->delta_flag=0;
  ThreeDV_acquisition_info->depth_far_ae->exponent_size=7;

#if EXT3D
  ThreeDV_acquisition_info->disp_param_flag=0;
  memset(ThreeDV_acquisition_info->d_disparity_scale[0],0x00,MAX_CODEVIEW*MAX_CODEVIEW*sizeof(double));
  memset(ThreeDV_acquisition_info->d_disparity_offset[0],0x00,MAX_CODEVIEW*MAX_CODEVIEW*sizeof(double));
  memset(ThreeDV_acquisition_info->i_disparity_scale[0],0x00,MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memset(ThreeDV_acquisition_info->i_disparity_offset[0],0x00,MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memset(ThreeDV_acquisition_info->i_disparity_scale_diff[0],0x00,MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memset(ThreeDV_acquisition_info->i_disparity_offset_diff[0],0x00,MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
#endif

}

void copy_acquisition_info(ThreeDVAcquisitionInfo* dest_acquisition_infor, ThreeDVAcquisitionInfo* src_acquisition_info)
{
  memcpy(dest_acquisition_infor->focal_length_x_ae,src_acquisition_info->focal_length_x_ae,sizeof(ThreeDVAE));
  memcpy(dest_acquisition_infor->focal_length_y_ae,src_acquisition_info->focal_length_y_ae,sizeof(ThreeDVAE));
  memcpy(dest_acquisition_infor->principal_point_x_ae,src_acquisition_info->principal_point_x_ae,sizeof(ThreeDVAE));
  memcpy(dest_acquisition_infor->principal_point_y_ae,src_acquisition_info->principal_point_y_ae,sizeof(ThreeDVAE));
  memcpy(dest_acquisition_infor->translation_ae,src_acquisition_info->translation_ae,sizeof(ThreeDVAE));
  memcpy(dest_acquisition_infor->depth_near_ae,src_acquisition_info->depth_near_ae,sizeof(ThreeDVAE));
  memcpy(dest_acquisition_infor->depth_far_ae,src_acquisition_info->depth_far_ae,sizeof(ThreeDVAE));
#if EXT3D
  dest_acquisition_infor->disp_param_flag=src_acquisition_info->disp_param_flag;
  memcpy(dest_acquisition_infor->d_disparity_scale[0],src_acquisition_info->d_disparity_scale[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(double));
  memcpy(dest_acquisition_infor->d_disparity_offset[0],src_acquisition_info->d_disparity_offset[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(double));
  memcpy(dest_acquisition_infor->i_disparity_scale[0],src_acquisition_info->i_disparity_scale[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memcpy(dest_acquisition_infor->i_disparity_offset[0],src_acquisition_info->i_disparity_offset[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memcpy(dest_acquisition_infor->i_disparity_scale_diff[0],src_acquisition_info->i_disparity_scale_diff[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memcpy(dest_acquisition_infor->i_disparity_offset_diff[0],src_acquisition_info->i_disparity_offset_diff[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
#endif
}

/*!
************************************************************************
* \brief
*    free the memory for 3DV acquisition information
************************************************************************
*/
void free_mem_acquisition_info(ThreeDVAcquisitionInfo* threeDV_acqusition_info)
{
  if(threeDV_acqusition_info)
  {
    free(threeDV_acqusition_info->focal_length_x_ae);
    free(threeDV_acqusition_info->focal_length_y_ae);
    free(threeDV_acqusition_info->principal_point_x_ae);
    free(threeDV_acqusition_info->principal_point_y_ae);
    free(threeDV_acqusition_info->translation_ae);
    free(threeDV_acqusition_info->depth_near_ae);
    free(threeDV_acqusition_info->depth_far_ae);
    free(threeDV_acqusition_info);
  }
}
#if EXT3D
void get_exponent_mantissa_SEI(ThreeDVAESEI* threeDV_ae)
{
  double arg=fabs(threeDV_ae->original);
  double log_val;
  int32 expo,v=0,i;
  double mant;
  int32 exponnent=0;
  int32 mantissa=0;
  int32 bits=0;

  if(threeDV_ae->pred_mode)
  {
    int32 bit;
    int mant_prec=threeDV_ae->precision;
    /* Find expo & mant when arg=(-1)^sign*2^expo*(0.mant),*/
    if (arg==0.0)
    {
      expo=0;
      mant=0;
    } 
    else 
    {
      log_val= log(arg)/log(2.0);
      expo = (int)floor(log_val);
      mant = pow(2.0, log_val - expo);
      if (fabs(mant) > 1.0) 
      {
        mant = mant / (double)2.0;
        expo= expo + 1;
      }    
    }

    /* Convert expo & mant so that X=(-1)^s*2^expo*(1.mant) */
    if (expo == 0 && mant ==0.0 ) 
    {
      v=0;
    } 
    else if (expo> -30)
    { 
      while ((double)fabs(mant)<(double)1.0) 
      {
        mant=mant*2.0;
        expo=expo-1;
      }
      if (mant>=0.0)
        mant = mant-1.0;
      v= expo + mant_prec; // number of necessary mantissa bits in the case of truncation
      if (v<0)
        v=0;
      expo += 31;
    } 
    else if (expo == -30) 
    {
      v= expo + mant_prec; // number of necessary mantissa bits in the case of truncation
      if (v<0) 
        v=0;
      expo=0;
    }

    exponnent = expo;
    if(v>30)
      v=30;

    /* Convert a float number mant (0<=mant<1) into a binary representation N which is a mantissa with v bits */
    mantissa=0;


    for (i=1; i<=v; i++ )
    {
      bit = (2.0*mant >= 1.0);
      mant = 2.0*mant - (double)bit;
      mantissa = (mantissa << 1 ) | bit ; // MSB of M corresponds to 1/2 and MSB-1 to 1/4 and so on.
    }
    bits=v;
  }
  else
  {
    int32 mantissa_len=threeDV_ae->mantissa_length;
    log_val= log(arg)/log(2.0);
    expo = (int)floor(log_val);
    exponnent=expo+31;

    mantissa=(int32)floor((arg/pow(2,expo)-1)*pow(2,mantissa_len)+0.5);
    bits=mantissa_len;
  }
  threeDV_ae->element.exponent=exponnent;
  threeDV_ae->element.mantissa=mantissa;
  threeDV_ae->element.mantissa_bits=bits;
}
void get_rec_double_type_SEI(ThreeDVAESEI* threeDV_ae)
{
  int32 i;
  int32 sgn= (threeDV_ae->element.sign==0) ? 1 : -1;
  int32 exponent=threeDV_ae->element.exponent;
  int32 mantissa=threeDV_ae->element.mantissa;
  int32 exponent_size=(1<<threeDV_ae->exponent_size)-1;
  double *recon=&(threeDV_ae->rec);

  assert(exponent!=exponent_size);

  if(threeDV_ae->pred_mode)
  {
    int32 length=getVarLength(exponent,threeDV_ae->precision);


    double factor=1.0;

    if(length>30)
      length=30;
    for(i=0;i<length;i++)
      factor=factor/2;


    *recon=0.0;



    for (i=0;i<(int)length;i++) 
    {
      *recon += factor*((mantissa>>i)&0x01);
      factor *= (double)2.0;
    }
    if (exponent>0 && exponent<exponent_size)
      *recon = (double)sgn*(double)pow((double)2,(double)exponent-31)*(1.0+*recon);
    if (exponent==0)
      *recon = (double)sgn*(double)pow((double)2,(double)-30)*(*recon);  
  }
  else
  {
    int32 mantissa_len=threeDV_ae->mantissa_length;
    *recon=sgn*pow(2,exponent-31)*(1+mantissa/pow(2,mantissa_len));
  }
}

#if EXT3D
void init_depth_acquisition_info_SEI(DepthAcquisitionInfoSEI* depth_acquisition_info)
{
  depth_acquisition_info->depth_near_ae.pred_mode=0;
  depth_acquisition_info->depth_near_ae.exponent_size=7;

  depth_acquisition_info->depth_far_ae.pred_mode=0;
  depth_acquisition_info->depth_far_ae.exponent_size=7;

  depth_acquisition_info->d_max_ae.pred_mode=0;
  depth_acquisition_info->d_max_ae.exponent_size=7;

  depth_acquisition_info->d_min_ae.pred_mode=0;
  depth_acquisition_info->d_min_ae.exponent_size=7;
}

void init_multiview_acquisition_info_SEI(MultiviewAcquisitionInfoSEI* multiview_acquisition_info)
{
  int i,j;

  multiview_acquisition_info->focal_length_x_ae.pred_mode=1;
  multiview_acquisition_info->focal_length_x_ae.exponent_size=6;

  multiview_acquisition_info->focal_length_y_ae.pred_mode=1;
  multiview_acquisition_info->focal_length_y_ae.exponent_size=6;

  multiview_acquisition_info->principal_point_x_ae.pred_mode=1;
  multiview_acquisition_info->principal_point_x_ae.exponent_size=6;

  multiview_acquisition_info->principal_point_y_ae.pred_mode=1;
  multiview_acquisition_info->principal_point_y_ae.exponent_size=6;

  multiview_acquisition_info->skew_factor_ae.pred_mode=1;
  multiview_acquisition_info->skew_factor_ae.exponent_size=6;

  for (i=0; i<3; i++)
  {
    multiview_acquisition_info->translation_ae[i].pred_mode=1;
    multiview_acquisition_info->translation_ae[i].exponent_size=6;
  }

  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
    {
      multiview_acquisition_info->rotation_ae[i][j].pred_mode=1;
      multiview_acquisition_info->rotation_ae[i][j].exponent_size=6;
    }
}
#endif
#endif

#endif
