# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /run/media/chipen/数据/Pro_TEST/Pro_TEST_Cpp/Interpreter

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /run/media/chipen/数据/Pro_TEST/Pro_TEST_Cpp/Interpreter/build

# Utility rule file for ContinuousMemCheck.

# Include the progress variables for this target.
include CMakeFiles/ContinuousMemCheck.dir/progress.make

CMakeFiles/ContinuousMemCheck:
	/usr/bin/ctest -D ContinuousMemCheck

ContinuousMemCheck: CMakeFiles/ContinuousMemCheck
ContinuousMemCheck: CMakeFiles/ContinuousMemCheck.dir/build.make

.PHONY : ContinuousMemCheck

# Rule to build all files generated by this target.
CMakeFiles/ContinuousMemCheck.dir/build: ContinuousMemCheck

.PHONY : CMakeFiles/ContinuousMemCheck.dir/build

CMakeFiles/ContinuousMemCheck.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ContinuousMemCheck.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ContinuousMemCheck.dir/clean

CMakeFiles/ContinuousMemCheck.dir/depend:
	cd /run/media/chipen/数据/Pro_TEST/Pro_TEST_Cpp/Interpreter/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /run/media/chipen/数据/Pro_TEST/Pro_TEST_Cpp/Interpreter /run/media/chipen/数据/Pro_TEST/Pro_TEST_Cpp/Interpreter /run/media/chipen/数据/Pro_TEST/Pro_TEST_Cpp/Interpreter/build /run/media/chipen/数据/Pro_TEST/Pro_TEST_Cpp/Interpreter/build /run/media/chipen/数据/Pro_TEST/Pro_TEST_Cpp/Interpreter/build/CMakeFiles/ContinuousMemCheck.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ContinuousMemCheck.dir/depend

