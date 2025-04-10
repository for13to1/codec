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


#if !defined(AFX_CFMO_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_)
#define AFX_CFMO_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_



H264AVC_NAMESPACE_BEGIN

const unsigned Max_Num_Slice_Groups =8;
const int null= 0;


class ImageParameter
{
public:
	unsigned PicSizeInMbs;
	unsigned int field_pic_flag;
	unsigned PicWidthInMbs;  
	unsigned PicHeightInMapUnits;
	int slice_group_change_cycle;
};


class FMO_PPS
{

public:
	
	unsigned num_slice_group_map_units_minus1;
	unsigned num_slice_groups_minus1;
	unsigned slice_group_map_type;
	unsigned run_length_minus1[Max_Num_Slice_Groups];
	unsigned top_left[Max_Num_Slice_Groups];		
	unsigned bottom_right[Max_Num_Slice_Groups];
	bool slice_group_change_direction_flag;
	unsigned slice_group_change_rate_minus1;
	unsigned int slice_group_id[Max_Num_Slice_Groups];

public:
	FMO_PPS():num_slice_group_map_units_minus1(0)
		,num_slice_groups_minus1(0)
		,slice_group_map_type(0)
		,slice_group_change_direction_flag(0)
		,slice_group_change_rate_minus1(0){};


	void copy_run_length_minus1(unsigned* Run_length_minus1)
	{
		assert(Max_Num_Slice_Groups>0);
		for(int i=0;i<Max_Num_Slice_Groups;i++)
		{
			run_length_minus1[i] = Run_length_minus1[i];
		}
	};

	void copy_top_left(unsigned* Top_left)
	{
		assert(Max_Num_Slice_Groups>0);
		for(UInt i=0;i<num_slice_groups_minus1;i++)
		{
			top_left[i] = Top_left[i];
		}
	}

	void copy_bottom_right(unsigned* Bottom_right)
	{
		assert(Max_Num_Slice_Groups>0);
		for(UInt i=0;i<num_slice_groups_minus1;i++)
		{
			bottom_right[i] = Bottom_right[i];
		}
	}

	void copy_slice_group_id(unsigned int* Slice_group_id)
	{
		assert(Max_Num_Slice_Groups>0);
		for(UInt i=0;i<num_slice_groups_minus1;i++)
			slice_group_id[i] = Slice_group_id[i];
	}

};


class FMO_SPS
{
public:
	FMO_SPS():pic_height_in_map_units_minus1(0),pic_width_in_mbs_minus1(0),frame_mbs_only_flag(1),mb_adaptive_frame_field_flag(0){};
	unsigned pic_height_in_map_units_minus1;
	unsigned pic_width_in_mbs_minus1;
	bool frame_mbs_only_flag;
	bool mb_adaptive_frame_field_flag;
};

class FMO
{

	int NumberOfSliceGroups_;    // the number of slice groups  (0 == scan order, 7 == maximum)

	int *MbToSliceGroupMap_;
	int *MapUnitToSliceGroupMap_; 
	unsigned PicSizeInMapUnits_ ;
	int *numMbInSliceGroup_;
	
	//enc
	int FirstMBInSlice[Max_Num_Slice_Groups];

	static Bool m_siSGId[Max_Num_Slice_Groups];
	static int  m_iPOC, m_iFrame;

public :
	ImageParameter img_;         //!< image parameters
	FMO_PPS pps_;
	FMO_SPS sps_;

	Void setCodedSG(int sgID, int POC)//, int frame) 
	{ 		
		if (POC != m_iPOC)// || frame != m_iFrame)
		{
			m_iPOC = POC;
		//	m_iFrame = frame;
			initCodedSG();
		}
		m_siSGId[sgID] = true;	
	}
	Void setCodedSG(int sgId) { m_siSGId[sgId] = true; }
	Bool isCodedSG(int sgId) { return m_siSGId[sgId]; }
	Void initCodedSG() { for (int i = 0; i < 8; i++) m_siSGId[i] = false; }


public:
	
	FMO():MbToSliceGroupMap_(0),MapUnitToSliceGroupMap_(0),PicSizeInMapUnits_(0),numMbInSliceGroup_(0)	{	};	
	~FMO(){	finit();};		

	int init (FMO_PPS* Pps, FMO_SPS* Sps);
	int finit (); //destroy
	int getNumberOfSliceGroup();
	int getLastMBOfPicture();
	int getLastMBInSliceGroup(int SliceGroup);
	int getSliceGroupId (int mb);
	int getNextMBNr (int CurrentMbNr);
	int getNumMbInSliceGroup(int sliceGroupID);

	//--Enc
	int StartPicture ();

	int EndPicture ();
	
	int getPreviousMBNr (int CurrentMbNr);
	int getFirstMBOfSliceGroup (int SliceGroupID);
	int getLastCodedMBOfSliceGroup (int SliceGroupID);
	int getFirstMacroblockInSlice ( int SliceGroup);
	int SliceGroupCompletelyCoded( int SliceGroupID);

	void setLastMacroblockInSlice ( int mb);


  //--ICU/ETRI FMO Implementation
	unsigned int GetYTopLeft(int iGroup);
	unsigned int GetXTopLeft(int iGroup);
	unsigned int GetYBottomRight(int iGroup);
	unsigned int GetXBottomRight(int iGroup);

  // JVT-S054 (2) (ADD) ->
  UInt getNumMbsInSlice(UInt uiFirstMbInSlice, UInt uiLastMbInSlice);
  int getLastMbInSlice(UInt uiFirstMbInSlice, UInt uiNumMbsInSlice);
  // JVT-S054 (2) (ADD) <-

private:
	void GenerateType0MapUnitMap ();
	void GenerateType1MapUnitMap ();
	void GenerateType2MapUnitMap ();
	void GenerateType3MapUnitMap ();
	void GenerateType4MapUnitMap ();
	void GenerateType5MapUnitMap ();
	void GenerateType6MapUnitMap ();
	
	int GenerateMapUnitToSliceGroupMap();
	int GenerateMbToSliceGroupMap();
	
	void printFmoMaps();
	void mallocMbToSliceGroupMap();
	int initMapUnitToSliceGroupMap();

	unsigned GetNumSliceGroupMapUnits();


	void InitFirstMBsInSlices();

	void calcMbNumInSliceGroup();
};


H264AVC_NAMESPACE_END


#endif //!defined(AFX_CFMO_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_)
