# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/2.8.9/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/2.8.9/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/local/Cellar/cmake/2.8.9/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/mai4/work/rach/src/tools/silk

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/mai4/work/rach/src/tools/silk

# Include any dependencies generated for this target.
include tools/silkc/CMakeFiles/silkc.dir/depend.make

# Include the progress variables for this target.
include tools/silkc/CMakeFiles/silkc.dir/progress.make

# Include the compile flags for this target's objects.
include tools/silkc/CMakeFiles/silkc.dir/flags.make

tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o: tools/silkc/CMakeFiles/silkc.dir/flags.make
tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o: tools/silkc/silkc.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/mai4/work/rach/src/tools/silk/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o"
	cd /Users/mai4/work/rach/src/tools/silk/tools/silkc && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/silkc.dir/silkc.cpp.o -c /Users/mai4/work/rach/src/tools/silk/tools/silkc/silkc.cpp

tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/silkc.dir/silkc.cpp.i"
	cd /Users/mai4/work/rach/src/tools/silk/tools/silkc && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/mai4/work/rach/src/tools/silk/tools/silkc/silkc.cpp > CMakeFiles/silkc.dir/silkc.cpp.i

tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/silkc.dir/silkc.cpp.s"
	cd /Users/mai4/work/rach/src/tools/silk/tools/silkc && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/mai4/work/rach/src/tools/silk/tools/silkc/silkc.cpp -o CMakeFiles/silkc.dir/silkc.cpp.s

tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o.requires:
.PHONY : tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o.requires

tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o.provides: tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o.requires
	$(MAKE) -f tools/silkc/CMakeFiles/silkc.dir/build.make tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o.provides.build
.PHONY : tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o.provides

tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o.provides.build: tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o

# Object files for target silkc
silkc_OBJECTS = \
"CMakeFiles/silkc.dir/silkc.cpp.o"

# External object files for target silkc
silkc_EXTERNAL_OBJECTS =

tools/silkc/silkc: tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o
tools/silkc/silkc: tools/silkc/CMakeFiles/silkc.dir/build.make
tools/silkc/silkc: lib/VMCore/libSilkVMCore.a
tools/silkc/silkc: lib/decil/libSilkDecil.a
tools/silkc/silkc: lib/Support/libSilkSupport.a
tools/silkc/silkc: tools/silkc/CMakeFiles/silkc.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable silkc"
	cd /Users/mai4/work/rach/src/tools/silk/tools/silkc && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/silkc.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/silkc/CMakeFiles/silkc.dir/build: tools/silkc/silkc
.PHONY : tools/silkc/CMakeFiles/silkc.dir/build

tools/silkc/CMakeFiles/silkc.dir/requires: tools/silkc/CMakeFiles/silkc.dir/silkc.cpp.o.requires
.PHONY : tools/silkc/CMakeFiles/silkc.dir/requires

tools/silkc/CMakeFiles/silkc.dir/clean:
	cd /Users/mai4/work/rach/src/tools/silk/tools/silkc && $(CMAKE_COMMAND) -P CMakeFiles/silkc.dir/cmake_clean.cmake
.PHONY : tools/silkc/CMakeFiles/silkc.dir/clean

tools/silkc/CMakeFiles/silkc.dir/depend:
	cd /Users/mai4/work/rach/src/tools/silk && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mai4/work/rach/src/tools/silk /Users/mai4/work/rach/src/tools/silk/tools/silkc /Users/mai4/work/rach/src/tools/silk /Users/mai4/work/rach/src/tools/silk/tools/silkc /Users/mai4/work/rach/src/tools/silk/tools/silkc/CMakeFiles/silkc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/silkc/CMakeFiles/silkc.dir/depend
