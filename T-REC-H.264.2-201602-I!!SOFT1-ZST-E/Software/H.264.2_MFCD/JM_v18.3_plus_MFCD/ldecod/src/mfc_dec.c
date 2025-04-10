/*
***********************************************************************
* COPYRIGHT AND WARRANTY INFORMATION
*
* Copyright 2001-2016, International Telecommunications Union, Geneva
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


/*!
*************************************************************************************
* \file  mfc_dec.c
*
* \brief 
*        MFC SDK  decoder layer interface to JM decoder
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


#include "c3d_decoder_layer_api.h"
#include "c3d_utilities_api.h"
#include "mfc_dec.h"


void* allocate_mfc_decoder_wrapper(){          
    
   
    return(allocate_mfc_decoder_layer());

}

int initialize_mfc_decoder_wrapper(void *pv_dec_lyr_ctxt,void  *init_params){            

    return(initialize_mfc_decoder_layer(pv_dec_lyr_ctxt,(RPUData *)init_params));

}

  int processDecRpuWrapper(
                        void    *pv_dec_layer_context,
                const    void    *pv_recon_bl,
                        void    *pv_pred_el                        
                        )
{
    int status = 0;


    status = process_dec_rpu(pv_dec_layer_context,
                        (ImageData *)pv_recon_bl,                                             
                        (ImageData *)pv_pred_el                                                
                        );



                    
     
    return status;

}
    
int   process3DDeMuxWrapper(void    *pv_dec_layer_context,
                            void    *pv_view0_image,
                            void    *pv_view1_image,
                    const    void   *pv_bl_image,
                    const    void   *pv_el_image,                        
                    const    void   *pv_predel_image,
                              int     deMuxMode,
                              int     rpu_field_processing
                        )
{

	ImageData p_v0 ;
	ImageData p_v1 ;
	ImageData p_bl;
	ImageData p_el;
	ImageData p_pel;
    int i32_loop;
    int status = 0;

	for(i32_loop=rpu_field_processing;i32_loop<2*rpu_field_processing+ 1;i32_loop++)
	{


			/* Setup Image Format for Input/Output Images */
			change_image_format(&p_v0		,(ImageData *)pv_view0_image		,(PICTURE_TYPE)i32_loop);
            change_image_format(&p_v1		,(ImageData *)pv_view1_image		,(PICTURE_TYPE)i32_loop);
            change_image_format(&p_bl		,(ImageData *)pv_bl_image		    ,(PICTURE_TYPE)i32_loop);
            change_image_format(&p_el		,(ImageData *)pv_el_image		    ,(PICTURE_TYPE)i32_loop);
            change_image_format(&p_pel		,(ImageData *)pv_predel_image		,(PICTURE_TYPE)i32_loop);



            if(deMuxMode==1)
            {
                status = process3DDeMux_OM(pv_dec_layer_context,
                                    &p_v0,
                                    &p_v1,
                                    &p_bl,
                                    &p_el,
                                    &p_pel
                                    );
            }
            else
            {
                    status = process3DDeMux_FC(pv_dec_layer_context,
                                    &p_v0,
                                    &p_v1,
                                    &p_bl
                                    );                
            }

    }

     
    return status;

}


int   process3DCopyWrapper(void    *pv_dec_layer_context,
                            void    *pv_view0_image,
                            void    *pv_view1_image,
                    const    void   *pv_bl_image,
                    const    void   *pv_el_image,                        
                    const    void   *pv_predel_image,
                              int     deMuxMode,
                              int     rpu_field_processing
                        )
{

	ImageData p_v0 ;
	ImageData p_v1 ;
	ImageData p_bl;
	ImageData p_el;
	ImageData p_pel;
    int i32_loop;
    int status = 0;


	for(i32_loop=rpu_field_processing;i32_loop<2*rpu_field_processing+ 1;i32_loop++)
	{
		change_image_format(&p_v0		,(ImageData *)pv_view0_image		,(PICTURE_TYPE)i32_loop);
        change_image_format(&p_v1		,(ImageData *)pv_view1_image		,(PICTURE_TYPE)i32_loop);
        change_image_format(&p_bl		,(ImageData *)pv_bl_image		    ,(PICTURE_TYPE)i32_loop);
        change_image_format(&p_el		,(ImageData *)pv_el_image		    ,(PICTURE_TYPE)i32_loop);
            
		copy_complete_ImageData(&p_v0,&p_bl);
		copy_complete_ImageData(&p_v1,&p_el);
    }

     
    return status;

}


int32_t free_mfc_decoder_wrapper(void  *pv_dec_layer_ctxt){        
    
    free_mfc_decoder_layer(pv_dec_layer_ctxt);       


    return 0;

}





