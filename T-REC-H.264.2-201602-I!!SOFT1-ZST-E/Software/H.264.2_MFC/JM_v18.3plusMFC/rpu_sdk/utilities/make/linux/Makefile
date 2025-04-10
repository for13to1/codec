#
#  Makefile for building RPU kernel library
#

# Project name
RPU_KRNL=c3d_utilities

AR=ar

# c = Suppresses the message ar normally prints when it creates a new archive file.
# r = Replaces or adds file to archive.
# s = Regenerates the symbol table regardless of whether the command modifies the archive.
# v = Gives verbose output.
ARFLAGS=crvs

# Directories
LIBDIR=../../lib/linux
INCDIR=../../inc
INCDIR_RPU_KRNL=../../../rpu_kernel/inc
SRCDIR=../../src
OBJDIR=obj

OBJSUF=.o
PREFIX=lib

# Files
SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%$(OBJSUF))
LIB=$(LIBDIR)/$(PREFIX)$(RPU_KRNL).a

CFLAGS=-Wall -I$(INCDIR) -I$(INCDIR_RPU_KRNL) -O3 

ifeq ($(M32),1)
CFLAGS+=-m32
endif


.PHONY: all objdir_mk libdir_mk clean_all clean

all: $(RPU_KRNL) Makefile

$(RPU_KRNL): objdir_mk libdir_mk $(OBJ)
	@echo
	@echo 'Creating library: $(LIB)'
	@$(AR) $(ARFLAGS) $(LIB) $(OBJ)
	@echo '... done'

# Build an object file for each source file
$(OBJDIR)/%$(OBJSUF): $(SRCDIR)/%.c
	@echo 'Compiling object file: $@'
	@$(CC) -c -o $@ $(CFLAGS) $<

objdir_mk:
	@echo 'Creating directory: $(OBJDIR)'
	@mkdir -p $(OBJDIR)

libdir_mk:
	@echo 'Creating directory: $(LIBDIR)'
	@mkdir -p $(LIBDIR)

clean_all: clean
	@echo 'Removing library files'	
	@rm -rf $(LIBDIR)

clean:
	@echo 'Removing object files'
	@rm -rf $(OBJDIR)

