# the SOURCE definiton lets you move your makefile to another position
CONFIG 				= CONSOLE

# set directories to your wanted values
SRC_DIR				= ../../../../src/tools/PSNR
INC_DIR				= ../../../../include
LIB_DIR				= ../../../../../../lib
BIN_DIR				= ../../../../../../bin

SRC_DIR1		=
SRC_DIR2		=
SRC_DIR3		=
SRC_DIR4		=

USER_INC_DIRS	= -I$(SRC_DIR)

# intermediate directory for object files
OBJ_DIR				= ./objects

# set executable name
PRJ_NAME			= PSNR

# defines to set
DEFS				= -DMSYS_LINUX -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DMSYS_UNIX_LARGEFILE

# set objects
OBJS          		= 	\
					$(OBJ_DIR)/main.o \

# set libs to link with
LIBS				= -ldl

DEBUG_LIBS			=
RELEASE_LIBS		=

STAT_LIBS			= -lpthread 
DYN_LIBS			=


DYN_DEBUG_LIBS		= 
DYN_DEBUG_PREREQS		= 
STAT_DEBUG_LIBS		= 
STAT_DEBUG_PREREQS		= 

DYN_RELEASE_LIBS	= 
DYN_RELEASE_PREREQS	= 
STAT_RELEASE_LIBS	= 
STAT_RELEASE_PREREQS	= 


# name of the base makefile
MAKE_FILE_NAME		= ../../common/makefile.base

# include the base makefile
include $(MAKE_FILE_NAME)
