# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/hyw/project/6.824

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hyw/project/6.824/build

# Include any dependencies generated for this target.
include MapReduce/CMakeFiles/reduceserver.dir/depend.make

# Include the progress variables for this target.
include MapReduce/CMakeFiles/reduceserver.dir/progress.make

# Include the compile flags for this target's objects.
include MapReduce/CMakeFiles/reduceserver.dir/flags.make

MapReduce/CMakeFiles/reduceserver.dir/Reduce.cpp.o: MapReduce/CMakeFiles/reduceserver.dir/flags.make
MapReduce/CMakeFiles/reduceserver.dir/Reduce.cpp.o: ../MapReduce/Reduce.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hyw/project/6.824/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object MapReduce/CMakeFiles/reduceserver.dir/Reduce.cpp.o"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/reduceserver.dir/Reduce.cpp.o -c /home/hyw/project/6.824/MapReduce/Reduce.cpp

MapReduce/CMakeFiles/reduceserver.dir/Reduce.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/reduceserver.dir/Reduce.cpp.i"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hyw/project/6.824/MapReduce/Reduce.cpp > CMakeFiles/reduceserver.dir/Reduce.cpp.i

MapReduce/CMakeFiles/reduceserver.dir/Reduce.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/reduceserver.dir/Reduce.cpp.s"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hyw/project/6.824/MapReduce/Reduce.cpp -o CMakeFiles/reduceserver.dir/Reduce.cpp.s

MapReduce/CMakeFiles/reduceserver.dir/Reduceexam.cpp.o: MapReduce/CMakeFiles/reduceserver.dir/flags.make
MapReduce/CMakeFiles/reduceserver.dir/Reduceexam.cpp.o: ../MapReduce/Reduceexam.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hyw/project/6.824/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object MapReduce/CMakeFiles/reduceserver.dir/Reduceexam.cpp.o"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/reduceserver.dir/Reduceexam.cpp.o -c /home/hyw/project/6.824/MapReduce/Reduceexam.cpp

MapReduce/CMakeFiles/reduceserver.dir/Reduceexam.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/reduceserver.dir/Reduceexam.cpp.i"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hyw/project/6.824/MapReduce/Reduceexam.cpp > CMakeFiles/reduceserver.dir/Reduceexam.cpp.i

MapReduce/CMakeFiles/reduceserver.dir/Reduceexam.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/reduceserver.dir/Reduceexam.cpp.s"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hyw/project/6.824/MapReduce/Reduceexam.cpp -o CMakeFiles/reduceserver.dir/Reduceexam.cpp.s

# Object files for target reduceserver
reduceserver_OBJECTS = \
"CMakeFiles/reduceserver.dir/Reduce.cpp.o" \
"CMakeFiles/reduceserver.dir/Reduceexam.cpp.o"

# External object files for target reduceserver
reduceserver_EXTERNAL_OBJECTS =

../MapReduce/bin/reduceserver: MapReduce/CMakeFiles/reduceserver.dir/Reduce.cpp.o
../MapReduce/bin/reduceserver: MapReduce/CMakeFiles/reduceserver.dir/Reduceexam.cpp.o
../MapReduce/bin/reduceserver: MapReduce/CMakeFiles/reduceserver.dir/build.make
../MapReduce/bin/reduceserver: MapReduce/CMakeFiles/reduceserver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hyw/project/6.824/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../MapReduce/bin/reduceserver"
	cd /home/hyw/project/6.824/build/MapReduce && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/reduceserver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
MapReduce/CMakeFiles/reduceserver.dir/build: ../MapReduce/bin/reduceserver

.PHONY : MapReduce/CMakeFiles/reduceserver.dir/build

MapReduce/CMakeFiles/reduceserver.dir/clean:
	cd /home/hyw/project/6.824/build/MapReduce && $(CMAKE_COMMAND) -P CMakeFiles/reduceserver.dir/cmake_clean.cmake
.PHONY : MapReduce/CMakeFiles/reduceserver.dir/clean

MapReduce/CMakeFiles/reduceserver.dir/depend:
	cd /home/hyw/project/6.824/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hyw/project/6.824 /home/hyw/project/6.824/MapReduce /home/hyw/project/6.824/build /home/hyw/project/6.824/build/MapReduce /home/hyw/project/6.824/build/MapReduce/CMakeFiles/reduceserver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : MapReduce/CMakeFiles/reduceserver.dir/depend

