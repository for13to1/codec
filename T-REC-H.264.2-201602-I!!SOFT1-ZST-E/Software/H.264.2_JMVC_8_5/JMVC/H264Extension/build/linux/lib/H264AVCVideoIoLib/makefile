# the SOURCE definiton lets move your makefile to another position
CONFIG 			= LIBRARY

# set pathes to the correct directories
SRC_DIR			= ../../../../src/lib/H264AVCVideoIoLib
INC_DIR			= ../../../../include
LIB_DIR			= ../../../../../../lib
BIN_DIR			= ../../../../../../bin

SRC_DIR1		=
SRC_DIR2		=
SRC_DIR3		=
SRC_DIR4		=


USER_INC_DIRS	= -I$(SRC_DIR)

# intermediate directory for object files
OBJ_DIR			= ./objects

# the library name
PRJ_NAME	 	= H264AVCVideoIoLib

# version information
MAJOR_VER		= 0
MINOR_VER		= 28
VER				= $(MAJOR_VER).$(MINOR_VER)

# defines to set
DEFS			= -DMSYS_LINUX -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DMSYS_UNIX_LARGEFILE 

# set objects
OBJS          	= \
			$(OBJ_DIR)/H264AVCVideoIoLib.o \
			$(OBJ_DIR)/LargeFile.o \
			$(OBJ_DIR)/ReadBitstreamFile.o \
			$(OBJ_DIR)/ReadYuvFile.o \
			$(OBJ_DIR)/WriteBitstreamToFile.o \
			$(OBJ_DIR)/WriteYuvToFile.o \


LIBS				= -lpthread 

DEBUG_LIBS			=
RELEASE_LIBS		=

STAT_LIBS			=
DYN_LIBS			= -ldl

# the libraries to link with
STAT_DEBUG_LIBS		= 
STAT_RELEASE_LIBS	= 
DYN_DEBUG_LIBS		=
DYN_RELEASE_LIBS	=

# name of the base makefile
MAKE_FILE_NAME		= ../../common/makefile.base

# include the base makefile
include $(MAKE_FILE_NAME)
