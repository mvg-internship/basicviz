# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/CMake.app/Contents/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/fake.andrey/CLionProjects/basicviz

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/fake.andrey/CLionProjects/basicviz/build

# Include any dependencies generated for this target.
include test/CMakeFiles/ttf-sample.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/ttf-sample.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/ttf-sample.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/ttf-sample.dir/flags.make

test/CMakeFiles/ttf-sample.dir/ttf_sample.cpp.o: test/CMakeFiles/ttf-sample.dir/flags.make
test/CMakeFiles/ttf-sample.dir/ttf_sample.cpp.o: /Users/fake.andrey/CLionProjects/basicviz/test/ttf_sample.cpp
test/CMakeFiles/ttf-sample.dir/ttf_sample.cpp.o: test/CMakeFiles/ttf-sample.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/fake.andrey/CLionProjects/basicviz/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/ttf-sample.dir/ttf_sample.cpp.o"
	cd /Users/fake.andrey/CLionProjects/basicviz/build/test && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/ttf-sample.dir/ttf_sample.cpp.o -MF CMakeFiles/ttf-sample.dir/ttf_sample.cpp.o.d -o CMakeFiles/ttf-sample.dir/ttf_sample.cpp.o -c /Users/fake.andrey/CLionProjects/basicviz/test/ttf_sample.cpp

test/CMakeFiles/ttf-sample.dir/ttf_sample.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ttf-sample.dir/ttf_sample.cpp.i"
	cd /Users/fake.andrey/CLionProjects/basicviz/build/test && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/fake.andrey/CLionProjects/basicviz/test/ttf_sample.cpp > CMakeFiles/ttf-sample.dir/ttf_sample.cpp.i

test/CMakeFiles/ttf-sample.dir/ttf_sample.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ttf-sample.dir/ttf_sample.cpp.s"
	cd /Users/fake.andrey/CLionProjects/basicviz/build/test && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/fake.andrey/CLionProjects/basicviz/test/ttf_sample.cpp -o CMakeFiles/ttf-sample.dir/ttf_sample.cpp.s

# Object files for target ttf-sample
ttf__sample_OBJECTS = \
"CMakeFiles/ttf-sample.dir/ttf_sample.cpp.o"

# External object files for target ttf-sample
ttf__sample_EXTERNAL_OBJECTS =

test/ttf-sample: test/CMakeFiles/ttf-sample.dir/ttf_sample.cpp.o
test/ttf-sample: test/CMakeFiles/ttf-sample.dir/build.make
test/ttf-sample: /opt/homebrew/lib/libSDL2.dylib
test/ttf-sample: /opt/homebrew/Cellar/sdl2_ttf/2.20.1/lib/libSDL2_ttf.dylib
test/ttf-sample: test/CMakeFiles/ttf-sample.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/fake.andrey/CLionProjects/basicviz/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ttf-sample"
	cd /Users/fake.andrey/CLionProjects/basicviz/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ttf-sample.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/ttf-sample.dir/build: test/ttf-sample
.PHONY : test/CMakeFiles/ttf-sample.dir/build

test/CMakeFiles/ttf-sample.dir/clean:
	cd /Users/fake.andrey/CLionProjects/basicviz/build/test && $(CMAKE_COMMAND) -P CMakeFiles/ttf-sample.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/ttf-sample.dir/clean

test/CMakeFiles/ttf-sample.dir/depend:
	cd /Users/fake.andrey/CLionProjects/basicviz/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/fake.andrey/CLionProjects/basicviz /Users/fake.andrey/CLionProjects/basicviz/test /Users/fake.andrey/CLionProjects/basicviz/build /Users/fake.andrey/CLionProjects/basicviz/build/test /Users/fake.andrey/CLionProjects/basicviz/build/test/CMakeFiles/ttf-sample.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/ttf-sample.dir/depend

