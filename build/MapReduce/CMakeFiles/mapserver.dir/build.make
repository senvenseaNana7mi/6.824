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
include MapReduce/CMakeFiles/mapserver.dir/depend.make

# Include the progress variables for this target.
include MapReduce/CMakeFiles/mapserver.dir/progress.make

# Include the compile flags for this target's objects.
include MapReduce/CMakeFiles/mapserver.dir/flags.make

MapReduce/CMakeFiles/mapserver.dir/Map.cpp.o: MapReduce/CMakeFiles/mapserver.dir/flags.make
MapReduce/CMakeFiles/mapserver.dir/Map.cpp.o: ../MapReduce/Map.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hyw/project/6.824/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object MapReduce/CMakeFiles/mapserver.dir/Map.cpp.o"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/mapserver.dir/Map.cpp.o -c /home/hyw/project/6.824/MapReduce/Map.cpp

MapReduce/CMakeFiles/mapserver.dir/Map.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/mapserver.dir/Map.cpp.i"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hyw/project/6.824/MapReduce/Map.cpp > CMakeFiles/mapserver.dir/Map.cpp.i

MapReduce/CMakeFiles/mapserver.dir/Map.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/mapserver.dir/Map.cpp.s"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hyw/project/6.824/MapReduce/Map.cpp -o CMakeFiles/mapserver.dir/Map.cpp.s

MapReduce/CMakeFiles/mapserver.dir/Mapexam.cpp.o: MapReduce/CMakeFiles/mapserver.dir/flags.make
MapReduce/CMakeFiles/mapserver.dir/Mapexam.cpp.o: ../MapReduce/Mapexam.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hyw/project/6.824/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object MapReduce/CMakeFiles/mapserver.dir/Mapexam.cpp.o"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/mapserver.dir/Mapexam.cpp.o -c /home/hyw/project/6.824/MapReduce/Mapexam.cpp

MapReduce/CMakeFiles/mapserver.dir/Mapexam.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/mapserver.dir/Mapexam.cpp.i"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hyw/project/6.824/MapReduce/Mapexam.cpp > CMakeFiles/mapserver.dir/Mapexam.cpp.i

MapReduce/CMakeFiles/mapserver.dir/Mapexam.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/mapserver.dir/Mapexam.cpp.s"
	cd /home/hyw/project/6.824/build/MapReduce && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hyw/project/6.824/MapReduce/Mapexam.cpp -o CMakeFiles/mapserver.dir/Mapexam.cpp.s

# Object files for target mapserver
mapserver_OBJECTS = \
"CMakeFiles/mapserver.dir/Map.cpp.o" \
"CMakeFiles/mapserver.dir/Mapexam.cpp.o"

# External object files for target mapserver
mapserver_EXTERNAL_OBJECTS =

../MapReduce/bin/mapserver: MapReduce/CMakeFiles/mapserver.dir/Map.cpp.o
../MapReduce/bin/mapserver: MapReduce/CMakeFiles/mapserver.dir/Mapexam.cpp.o
../MapReduce/bin/mapserver: MapReduce/CMakeFiles/mapserver.dir/build.make
../MapReduce/bin/mapserver: MapReduce/CMakeFiles/mapserver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hyw/project/6.824/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../MapReduce/bin/mapserver"
	cd /home/hyw/project/6.824/build/MapReduce && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mapserver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
MapReduce/CMakeFiles/mapserver.dir/build: ../MapReduce/bin/mapserver

.PHONY : MapReduce/CMakeFiles/mapserver.dir/build

MapReduce/CMakeFiles/mapserver.dir/clean:
	cd /home/hyw/project/6.824/build/MapReduce && $(CMAKE_COMMAND) -P CMakeFiles/mapserver.dir/cmake_clean.cmake
.PHONY : MapReduce/CMakeFiles/mapserver.dir/clean

MapReduce/CMakeFiles/mapserver.dir/depend:
	cd /home/hyw/project/6.824/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hyw/project/6.824 /home/hyw/project/6.824/MapReduce /home/hyw/project/6.824/build /home/hyw/project/6.824/build/MapReduce /home/hyw/project/6.824/build/MapReduce/CMakeFiles/mapserver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : MapReduce/CMakeFiles/mapserver.dir/depend

