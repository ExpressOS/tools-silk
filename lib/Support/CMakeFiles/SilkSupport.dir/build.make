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
include lib/Support/CMakeFiles/SilkSupport.dir/depend.make

# Include the progress variables for this target.
include lib/Support/CMakeFiles/SilkSupport.dir/progress.make

# Include the compile flags for this target's objects.
include lib/Support/CMakeFiles/SilkSupport.dir/flags.make

lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o: lib/Support/CMakeFiles/SilkSupport.dir/flags.make
lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o: lib/Support/raw_istream.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/mai4/work/rach/src/tools/silk/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o"
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/SilkSupport.dir/raw_istream.cpp.o -c /Users/mai4/work/rach/src/tools/silk/lib/Support/raw_istream.cpp

lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SilkSupport.dir/raw_istream.cpp.i"
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/mai4/work/rach/src/tools/silk/lib/Support/raw_istream.cpp > CMakeFiles/SilkSupport.dir/raw_istream.cpp.i

lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SilkSupport.dir/raw_istream.cpp.s"
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/mai4/work/rach/src/tools/silk/lib/Support/raw_istream.cpp -o CMakeFiles/SilkSupport.dir/raw_istream.cpp.s

lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o.requires:
.PHONY : lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o.requires

lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o.provides: lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o.requires
	$(MAKE) -f lib/Support/CMakeFiles/SilkSupport.dir/build.make lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o.provides.build
.PHONY : lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o.provides

lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o.provides.build: lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o

lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o: lib/Support/CMakeFiles/SilkSupport.dir/flags.make
lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o: lib/Support/ErrorHandler.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/mai4/work/rach/src/tools/silk/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o"
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o -c /Users/mai4/work/rach/src/tools/silk/lib/Support/ErrorHandler.cpp

lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.i"
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/mai4/work/rach/src/tools/silk/lib/Support/ErrorHandler.cpp > CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.i

lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.s"
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/mai4/work/rach/src/tools/silk/lib/Support/ErrorHandler.cpp -o CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.s

lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o.requires:
.PHONY : lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o.requires

lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o.provides: lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o.requires
	$(MAKE) -f lib/Support/CMakeFiles/SilkSupport.dir/build.make lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o.provides.build
.PHONY : lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o.provides

lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o.provides.build: lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o

lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o: lib/Support/CMakeFiles/SilkSupport.dir/flags.make
lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o: lib/Support/Util.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/mai4/work/rach/src/tools/silk/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o"
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/SilkSupport.dir/Util.cpp.o -c /Users/mai4/work/rach/src/tools/silk/lib/Support/Util.cpp

lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SilkSupport.dir/Util.cpp.i"
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/mai4/work/rach/src/tools/silk/lib/Support/Util.cpp > CMakeFiles/SilkSupport.dir/Util.cpp.i

lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SilkSupport.dir/Util.cpp.s"
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/mai4/work/rach/src/tools/silk/lib/Support/Util.cpp -o CMakeFiles/SilkSupport.dir/Util.cpp.s

lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o.requires:
.PHONY : lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o.requires

lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o.provides: lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o.requires
	$(MAKE) -f lib/Support/CMakeFiles/SilkSupport.dir/build.make lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o.provides.build
.PHONY : lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o.provides

lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o.provides.build: lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o

# Object files for target SilkSupport
SilkSupport_OBJECTS = \
"CMakeFiles/SilkSupport.dir/raw_istream.cpp.o" \
"CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o" \
"CMakeFiles/SilkSupport.dir/Util.cpp.o"

# External object files for target SilkSupport
SilkSupport_EXTERNAL_OBJECTS =

lib/Support/libSilkSupport.a: lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o
lib/Support/libSilkSupport.a: lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o
lib/Support/libSilkSupport.a: lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o
lib/Support/libSilkSupport.a: lib/Support/CMakeFiles/SilkSupport.dir/build.make
lib/Support/libSilkSupport.a: lib/Support/CMakeFiles/SilkSupport.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library libSilkSupport.a"
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && $(CMAKE_COMMAND) -P CMakeFiles/SilkSupport.dir/cmake_clean_target.cmake
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/SilkSupport.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/Support/CMakeFiles/SilkSupport.dir/build: lib/Support/libSilkSupport.a
.PHONY : lib/Support/CMakeFiles/SilkSupport.dir/build

lib/Support/CMakeFiles/SilkSupport.dir/requires: lib/Support/CMakeFiles/SilkSupport.dir/raw_istream.cpp.o.requires
lib/Support/CMakeFiles/SilkSupport.dir/requires: lib/Support/CMakeFiles/SilkSupport.dir/ErrorHandler.cpp.o.requires
lib/Support/CMakeFiles/SilkSupport.dir/requires: lib/Support/CMakeFiles/SilkSupport.dir/Util.cpp.o.requires
.PHONY : lib/Support/CMakeFiles/SilkSupport.dir/requires

lib/Support/CMakeFiles/SilkSupport.dir/clean:
	cd /Users/mai4/work/rach/src/tools/silk/lib/Support && $(CMAKE_COMMAND) -P CMakeFiles/SilkSupport.dir/cmake_clean.cmake
.PHONY : lib/Support/CMakeFiles/SilkSupport.dir/clean

lib/Support/CMakeFiles/SilkSupport.dir/depend:
	cd /Users/mai4/work/rach/src/tools/silk && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mai4/work/rach/src/tools/silk /Users/mai4/work/rach/src/tools/silk/lib/Support /Users/mai4/work/rach/src/tools/silk /Users/mai4/work/rach/src/tools/silk/lib/Support /Users/mai4/work/rach/src/tools/silk/lib/Support/CMakeFiles/SilkSupport.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/Support/CMakeFiles/SilkSupport.dir/depend
