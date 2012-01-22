# Library search paths
DEFAULT_LIB_PATHS := -LC:/MinGW32/lib/gcc/mingw32/4.6.1 -LC:/MinGW32/lib
LIB_PATHS ?= $(DEFAULT_LIB_PATHS)

# Library dependencies
DEFAULT_LIBS :=
LIBS ?= $(DEFAULT_LIBS)

# Default package build directory, src directory, include directory
BUILDDIR   ?= build
SRCDIR     ?= src
INCLUDEDIR ?= include

# Include paths
DEFAULT_INCDIRS := -IC:/MinGW32/lib/gcc/mingw32/4.6.1/include/c++ -IC:/MinGW32/lib/gcc/mingw32/4.6.1/include/c++/mingw32 -IC:/MinGW32/include
INCDIRS ?= $(DEFAULT_INCDIRS)
INCDIRS += -Iinclude

# Source file extensions
SRCEXTS ?= .cpp

# Header file extensions
HDREXTS ?= .h .hpp

# Preprocessor/compiler options
DEFAULT_CPPFLAGS := -Wall -mwindows -std=c++0x -g -O2
CPPFLAGS ?= $(DEFAULT_CPPFLAGS)

# Linker options
DEFAULT_LDFLAGS := 
LDFLAGS ?= $(DEFAULT_LDFLAGS)

# C++ compiler
COMPILER ?= g++

# C++ linker
LINKER ?= g++

# Delete command
RM = rm -f

PACKAGE_DEPENDENCIES ?= 
PACKAGE_INCL_LIST := $(foreach p,$(PACKAGE_DEPENDENCIES),$(PROJECT_ROOT_DIR)/$(p)/depends.dep)
PACKAGE_INCLUDES := $(foreach p,$(PACKAGE_DEPENDENCIES),sinclude $(PROJECT_ROOT_DIR)/$(p)/depends.dep)
sinclude $(PACKAGE_INCLUDES)

## Stable Section: usually no need to be changed. But you can add more.
##==========================================================================
EXEC_SUFFIX ?= .exe
SHARED_SUFFIX ?= .so
STATIC_SUFFIX ?= .a
LIB_PREFIX ?= lib
TARGET ?= default

SOURCES = $(wildcard $(addprefix $(SRCDIR)/*,$(SRCEXTS)))
HEADERS = $(wildcard $(addprefix $(INCLUDEDIR)/*,$(HDREXTS)))
SRCS    = $(filter-out %.c,$(SOURCES))
OBJS    = $(foreach s,$(SOURCES),$(addsuffix .o, $(BUILDDIR)/$(notdir $(basename $(s)))))
DEPS    = $(OBJS:.o=.d)

INCDIRS += $(INCL_DEPENDS)
LIBS += $(LIB_DEPENDS)
LIB_PATHS += $(LIB_PATH_DEPENDS)

## Define some useful variables.
COMPILE = $(COMPILER) $(INCDIRS) $(CPPFLAGS) -c
LINK    = $(LINKER) $(CPPFLAGS) $(LDFLAGS)
SHAREDLINK = $(LINKER) $(CPPFLAGS) $(LDFLAGS) -shared
STATICLINK = ar cr
DEPENDENCY_FILE = depends.dep

.PHONY: all objs clean distclean help show

# Delete the default suffixes
.SUFFIXES:

all: $(TARGET)$(EXEC_SUFFIX)

# Rules for creating dependency files (.d).
#------------------------------------------

deps:$(DEPS)

$(BUILDDIR)/%.d:$(SRCDIR)/%.cpp
	$(COMPILER) $(INCDIRS) -MF$@ -MD -MM -MT$@ -MT$(BUILDDIR)/$(basename $(notdir $<)).o -c $<

# Rules for generating object files (.o).
#----------------------------------------
objs:$(OBJS)

$(BUILDDIR)/%.o:$(SRCDIR)/%.cpp
	$(COMPILE) $< -o $@

# Rules for generating the executable.
#-------------------------------------
$(TARGET)$(EXEC_SUFFIX):$(OBJS)
	$(LINK) $(OBJS) $(LIB_PATHS) $(LIBS) -o $@
	@echo Type ./$@ to execute the program.

clean:
	$(RM) $(OBJS) $(TARGET)$(EXEC_SUFFIX) $(LIB_PREFIX)$(TARGET)$(STATIC_SUFFIX) $(LIB_PREFIX)$(TARGET)$(SHARED_SUFFIX)

distclean: clean
	$(RM) $(DEPS) depends.dep

staticlib: $(OBJS) $(DEPENDENCY_FILE)
	$(STATICLINK) $(LIB_PREFIX)$(TARGET)$(STATIC_SUFFIX) $(OBJS)
	
sharedlib: $(OBJS) $(DEPENDENCY_FILE)
	$(SHAREDLINK) -o $(LIB_PREFIX)$(TARGET)$(SHARED_SUFFIX) $(OBJS)
	
$(DEPENDENCY_FILE):
	@echo $(PACKAGE_INCLUDES) > depends.dep
	@echo INCL_DEPENDS += -I$(CURDIR)/$(INCLUDEDIR) >> depends.dep
	@echo LIB_PATH_DEPENDS += >> depends.dep
	@echo LIB_DEPENDS += $(LIBS) $(CURDIR)/$(LIB_PREFIX)$(TARGET)$(STATIC_SUFFIX) >> depends.dep
	
sinclude $(DEPS)

# Show help.
help:
	@echo
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all       (=make) compile and link.'
	@echo '  objs      compile only (no linking).'
	@echo '  clean     clean objects and the executable file.'
	@echo '  distclean clean objects, the executable and dependencies.'
	@echo '  show      show variables (for debug use only).'
	@echo '  help      print this message.'
	@echo
	@echo 'Report bugs to <whyglinux AT gmail DOT com>.'

# Show variables (for debug use only.)
show:
	@echo 'PROGRAM     :' $(TARGET)$(EXEC_SUFFIX)
	@echo 'SHARED_LIB  :' $(LIB_PREFIX)$(TARGET)$(SHARED_SUFFIX)
	@echo 'STATIC_LIB  :' $(LIB_PREFIX)$(TARGET)$(STATIC_SUFFIX)
	@echo 'SRCDIR      :' $(SRCDIR)
	@echo 'HEADERS     :' $(HEADERS)
	@echo 'SOURCES     :' $(SOURCES)
	@echo 'LIBS        :' $(LIBS)
	@echo 'LIB_PATHS   :' $(LIB_PATHS)
	@echo 'SRCS        :' $(SRCS)
	@echo 'OBJS        :' $(OBJS)
	@echo 'DEPS        :' $(DEPS)
	@echo 'COMPILE     :' $(COMPILE)
	@echo 'LINK        :' $(LINK)
	@echo 'LIB_DEPENDS :' $(LIB_DEPENDS) $(LIB_PATH_DEPENDS)
	@echo 'INCL_DEPENDS:' $(INCL_DEPENDS)

## End of the Makefile