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
include test/CMakeFiles/benchstat.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/benchstat.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/benchstat.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/benchstat.dir/flags.make

test/CMakeFiles/benchstat.dir/bench.cpp.o: test/CMakeFiles/benchstat.dir/flags.make
test/CMakeFiles/benchstat.dir/bench.cpp.o: /Users/fake.andrey/CLionProjects/basicviz/test/bench.cpp
test/CMakeFiles/benchstat.dir/bench.cpp.o: test/CMakeFiles/benchstat.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/fake.andrey/CLionProjects/basicviz/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/benchstat.dir/bench.cpp.o"
	cd /Users/fake.andrey/CLionProjects/basicviz/build/test && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/benchstat.dir/bench.cpp.o -MF CMakeFiles/benchstat.dir/bench.cpp.o.d -o CMakeFiles/benchstat.dir/bench.cpp.o -c /Users/fake.andrey/CLionProjects/basicviz/test/bench.cpp

test/CMakeFiles/benchstat.dir/bench.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/benchstat.dir/bench.cpp.i"
	cd /Users/fake.andrey/CLionProjects/basicviz/build/test && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/fake.andrey/CLionProjects/basicviz/test/bench.cpp > CMakeFiles/benchstat.dir/bench.cpp.i

test/CMakeFiles/benchstat.dir/bench.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/benchstat.dir/bench.cpp.s"
	cd /Users/fake.andrey/CLionProjects/basicviz/build/test && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/fake.andrey/CLionProjects/basicviz/test/bench.cpp -o CMakeFiles/benchstat.dir/bench.cpp.s

# Object files for target benchstat
benchstat_OBJECTS = \
"CMakeFiles/benchstat.dir/bench.cpp.o"

# External object files for target benchstat
benchstat_EXTERNAL_OBJECTS =

test/benchstat: test/CMakeFiles/benchstat.dir/bench.cpp.o
test/benchstat: test/CMakeFiles/benchstat.dir/build.make
test/benchstat: test/CMakeFiles/benchstat.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/fake.andrey/CLionProjects/basicviz/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable benchstat"
	cd /Users/fake.andrey/CLionProjects/basicviz/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/benchstat.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/benchstat.dir/build: test/benchstat
.PHONY : test/CMakeFiles/benchstat.dir/build

test/CMakeFiles/benchstat.dir/clean:
	cd /Users/fake.andrey/CLionProjects/basicviz/build/test && $(CMAKE_COMMAND) -P CMakeFiles/benchstat.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/benchstat.dir/clean

test/CMakeFiles/benchstat.dir/depend:
	cd /Users/fake.andrey/CLionProjects/basicviz/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/fake.andrey/CLionProjects/basicviz /Users/fake.andrey/CLionProjects/basicviz/test /Users/fake.andrey/CLionProjects/basicviz/build /Users/fake.andrey/CLionProjects/basicviz/build/test /Users/fake.andrey/CLionProjects/basicviz/build/test/CMakeFiles/benchstat.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/benchstat.dir/depend

