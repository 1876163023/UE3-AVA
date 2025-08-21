
#
# Copyright (c) 2005, Epic Games Inc.  All rights reserved.
#
# This is the common rules file for platforms that use makefiles.
# The following is assumed to be set by the including makefile:
#	ROOT_PATH
#	PLATFORM
#	PLATFORM_MODULES
#	PLATFORM_PCHSOURCES
#	PLATFORM_SOURCEFILTER
#	PLATFORM_LIBS
#	PLATFORM_DEFINES
#	PLATFORM_DEBUG_CPPFLAGS
#	PLATFORM_RELEASE_CPPFLAGS
#	PLATFORM_FINAL_RELEASE_CPPFLAGS
#	PLATFORM_LINKERFLAGS
#	PLATFORM_INCLUDES - external include directories, NOT the Inc directories in the compiled modules
#	PLATFORM_DEPENDENCIES - extra dependencies made by the "all" target
#	GAMENAME - [DEMOGAME | UTGAME | WARGAME]
#	EXECUTABLE_PATH
#	EXECUTABLE_NAME
#	BUILDTYPE - [debug | release | finalrelease]
#	CC
#	AR
#	RUN_GAME - a one line command to run the game on your platform
#	DEBUG_GAME - a one line command to debug the game on your platform
#	PLATFORM_POST_BUILD_STEP - optional command to run after compiling
#	NO_DEBUG_INFO - [true | false*]
#
#	@todo - have a PLATFORM_LIBDIRS, BASE_LIBDIRS or something like that

# The directories to compile for all platforms
BASE_MODULES := Core Engine GameFramework Launch UnrealScriptTest

# The files to use to make precompiled headers
BASE_PCHSOURCES := CorePrivate.h EnginePrivate.h GameFramework.h 

# Set Game specific modules
ifeq ($(GAMENAME), EXAMPLEGAME)
CONTENT_DIRECTORY := ExampleGame
BASE_MODULES := $(BASE_MODULES) ExampleGame
BASE_PCHSOURCES := $(BASE_PCHSOURCES) ExampleGame.h
else
ifeq ($(GAMENAME), DEMOGAME)
CONTENT_DIRECTORY := DemoGame
BASE_MODULES := $(BASE_MODULES) DemoGame
BASE_PCHSOURCES := $(BASE_PCHSOURCES) DemoGame.h
else
ifeq ($(GAMENAME), UTGAME)
CONTENT_DIRECTORY := UTGame
BASE_MODULES := $(BASE_MODULES) UTGame
#BASE_PCHSOURCES := $(BASE_PCHSOURCES) UTGame.h
else
ifeq ($(GAMENAME), WARGAME)
CONTENT_DIRECTORY := WarGame
BASE_MODULES := $(BASE_MODULES) WarfareGame
BASE_PCHSOURCES := $(BASE_PCHSOURCES) WarfareGame.h
endif
endif
endif
endif

# Standard libs to link in for all platforms
BASE_LIBS := stdc++

# Standard defines for all platforms
BASE_DEFINES :=

## -Wformat-security -Wformat
# -Wextra
# -Wextra -Wmissing-field-initializers 
# -Wold-style-cast
# -Weffc++
# -Woverloaded-virtual
# -Wsign-promo

# -Weffc++
# -Wshadow

# next up to fix:  -Wswitch-default
# waiting on novodex sdk fixes for this:  

# Standard C++ flags for debug or release builds   -Wall
# look at removing:  -fno-optimize-sibling-calls -mminimal-toc
BASE_CPPFLAGS := -fmessage-length=0 -fshort-wchar -Wcomment -Wmissing-braces -Wparentheses  -Wreorder -Wno-non-virtual-dtor -Wredundant-decls -Werror
#-fno-optimize-sibling-calls 

# Debug info flags
ifeq ($(NO_DEBUG_INFO), true)
DEBUG_INFO_FLAGS :=
else
DEBUG_INFO_FLAGS := -g -ggdb
endif

# Standard debug build C++ flags
BASE_DEBUG_CPPFLAGS := $(DEBUG_INFO_FLAGS) -Winvalid-pch -D_DEBUG -pipe -DFINAL_RELEASE=0

# Standard release build C++ flags
BASE_RELEASE_CPPFLAGS := $(DEBUG_INFO_FLAGS) -O3 -DNDEBUG -pipe -fforce-addr -ffast-math -funroll-loops -fno-rtti -DFINAL_RELEASE=0 -Wuninitialized -Winit-self

# Standard final release build C++ flags
BASE_FINAL_RELEASE_CPPFLAGS := -O3 -DNDEBUG -pipe -fforce-addr -ffast-math -funroll-loops -fno-rtti -DFINAL_RELEASE=1 -DNO_LOGGING=1

# Standard include directories, independent of the compiled modules
BASE_INCLUDES := $(ROOT_PATH)/Development/External/zlib/Inc $(ROOT_PATH)/Development/External/lzopro/include

# Standard debug linker flags for all platforms
BASE_DEBUG_LINKERFLAGS := $(DEBUG_INFO_FLAGS)

# Standard release linker flags for all platforms
BASE_RELEASE_LINKERFLAGS := $(DEBUG_INFO_FLAGS)

# Standard final release linker flags for all platforms
BASE_FINAL_RELEASE_LINKERFLAGS :=

# Global filter to strip out all windows-specific files (yes, the % is necessary)
BASE_SOURCEFILTER := %UnVcWin32.cpp %UnThreadingWindows.cpp %UnConsoleSupportContainer.cpp \
	%UnEdCoordSystem.cpp %UnPNG.cpp %UIEditor.cpp %Launch/Src/Launch.cpp %Launch/Src/LaunchMisc.cpp \
	%UnStatsNotifyProviders.cpp %UnSocketWin.cpp %UnMaterial.cpp %UnScene.cpp %FFileManagerWindows.cpp

#--------------------------------------------------------------------------------------------------

# Path to all executables
EXECUTABLE_PATH := $(ROOT_PATH)/Binaries

# All directories that will be built 
MODULES := $(PLATFORM_MODULES) $(BASE_MODULES)

# All precompiled header sources to use
PCHSOURCES := $(BASE_PCHSOURCES) $(PLATFORM_PCHSOURCES)

# All libs to link against
LIBS := $(BASE_LIBS) $(PLATFORM_LIBS)

# All defines
DEFINES := $(BASE_DEFINES) $(PLATFORM_DEFINES) GAMENAME=$(GAMENAME)

# All include directories
INCLUDES := $(BASE_INCLUDES) $(PLATFORM_INCLUDES)

# All C++ and linker flags
DEBUG_CPPFLAGS := $(BASE_CPPFLAGS) $(BASE_DEBUG_CPPFLAGS) $(PLATFORM_DEBUG_CPPFLAGS)
RELEASE_CPPFLAGS := $(BASE_CPPFLAGS) $(BASE_RELEASE_CPPFLAGS) $(PLATFORM_RELEASE_CPPFLAGS)
FINAL_RELEASE_CPPFLAGS := $(BASE_CPPFLAGS) $(BASE_FINAL_RELEASE_CPPFLAGS) $(PLATFORM_FINAL_RELEASE_CPPFLAGS)

ifeq ($(BUILDTYPE),debug)
    CPPFLAGS := $(DEBUG_CPPFLAGS)
	LINKERFLAGS := $(BASE_RELEASE_LINKERFLAGS) $(PLATFORM_LINKERFLAGS)
else
ifeq ($(BUILDTYPE),finalrelease)
    CPPFLAGS := $(FINAL_RELEASE_CPPFLAGS)
	LINKERFLAGS := $(BASE_FINAL_RELEASE_LINKERFLAGS) $(PLATFORM_LINKERFLAGS)
else
    CPPFLAGS := $(RELEASE_CPPFLAGS)
	LINKERFLAGS := $(BASE_DEBUG_LINKERFLAGS) $(PLATFORM_LINKERFLAGS)
endif
endif

ifeq ($(NODEBUG),true)
	CPPFLAGS := $(filter-out -g -ggdb, $(CPPFLAGS))
	LINKERFLAGS := $(filter-out -g -ggdb, $(LINKERFLAGS))
endif

# All filters to apply
SOURCEFILTER := $(BASE_SOURCEFILTER) $(PLATFORM_SOURCEFILTER)

# Build the full executable name
EXECUTABLE := $(EXECUTABLE_PATH)/$(EXECUTABLE_NAME)

#--------------------------------------------------------------------------------------------------

# The base intermediate directory
INTERMEDIATE := $(ROOT_PATH)/Development/Intermediate

# Make the relative module directories, and paths
MODULES := $(addprefix $(ROOT_PATH)/Development/Src/, $(MODULES))

# Look in each module's /Src directory for .cpp files
SOURCEPATH := $(addsuffix /Src, $(MODULES))

# Create a filtered list of all the source files for this platform
SOURCES := $(foreach dir, $(MODULES), $(filter-out $(SOURCEFILTER), $(wildcard $(dir)/Src/*.cpp)))
SOURCESSTRIPPED := $(notdir $(SOURCES))

# Destination directory for objects
OBJPATH := $(INTERMEDIATE)/$(PLATFORM)/$(GAMENAME)/$(BUILDTYPE)

# Make a list of precompiled headers to create
PCHS := $(patsubst %.h, $(OBJPATH)/%.h.gch, $(PCHSOURCES))

# Include every modules /Inc directory
INCLUDES := $(INCLUDES) $(addsuffix /Inc, $(MODULES))

# Set the search paths in which to look for targets and prerequisites for the different file types
vpath %.cpp $(SOURCEPATH)
vpath %.h $(INCLUDES)
vpath %.o $(OBJPATH)
vpath %.h.gch $(OBJPATH)

# The list of destination .o files to be created
OBJECTS := $(patsubst %.cpp, $(OBJPATH)/%.o, $(SOURCESSTRIPPED))

# Tack on the OBJPATH for searching for the precompiled header
INCLUDES := $(OBJPATH) $(INCLUDES)

# Directory containing all dependency files
DEPSPATH := $(INTERMEDIATE)/$(PLATFORM)/$(GAMENAME)/depends

# Add a -l for each library
LIBS := $(addprefix -l, $(LIBS))

# Add a -D for each define
DEFINES := $(addprefix -D, $(DEFINES))

# Add a -I for each include directory
INCLUDES := $(addprefix -I, $(INCLUDES))

# assemble compile flags (CPPFLAGS is a hardcoded name understood by make)
CPPFLAGS := $(CPPFLAGS) $(DEFINES) $(INCLUDES)


#--------------------------------------------------------------------------------------------------

.PHONY: all makereqdirs clean cleantemps

all:  makereqdirs $(PLATFORM_PREDEPENDENCIES) pchsync $(PLATFORM_DEPENDENCIES)

run : all
	$(RUN_GAME)

debug : all
	$(DEBUG_GAME)

makereqdirs:
	@mkdir -p $(OBJPATH)
	@mkdir -p $(DEPSPATH)

#pchsync calls make on the executable (which compiles source and executable) to make sure that all pchs are generated before it compiles any source
pchsync: $(PCHS)
	@$(MAKE) $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	@echo Linking $@
	@$(CC) $(OBJECTS) $(LINKERFLAGS) $(LIBS) -o $(EXECUTABLE)
	@$(PLATFORM_POST_BUILD_STEP)
	@echo Finished!

$(OBJPATH)/%.h.gch : %.h
	@echo Precompiling $(*F).h.gch from $(*F).h
	@$(COMPILE.c) -x c++-header -MMD -MF $(DEPSPATH)/$(*F).h.P -MT $@ -o $@ $<

$(OBJPATH)/%.o : %.cpp
	@echo Compiling $(*F).cpp
	@$(COMPILE.c) -MMD -MF $(DEPSPATH)/$(*F).P -MT $@ -o $@ $<

clean:
	rm -Rf $(OBJPATH)
	rm -Rf $(EXECUTABLE)
	rm -Rf $(DEPSPATH)

cleantemps:
	rm -fR $(foreach dir, $(MODULES), $(wildcard $(dir)/*~))
  
# include the dependency information for all .o files as well as for the SDK files
-include $(patsubst %.h, $(DEPSPATH)/%.h.P, $(PCHSOURCES))
-include $(patsubst %.cpp, $(DEPSPATH)/%.P, $(SOURCESSTRIPPED))) 
