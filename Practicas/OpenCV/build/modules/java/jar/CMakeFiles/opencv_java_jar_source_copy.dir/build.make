# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/build

# Utility rule file for opencv_java_jar_source_copy.

# Include the progress variables for this target.
include modules/java/jar/CMakeFiles/opencv_java_jar_source_copy.dir/progress.make

modules/java/jar/CMakeFiles/opencv_java_jar_source_copy: CMakeFiles/dephelper/opencv_java_jar_source_copy


CMakeFiles/dephelper/opencv_java_jar_source_copy: ../cmake/copy_files.cmake
CMakeFiles/dephelper/opencv_java_jar_source_copy: CMakeFiles/dephelper/gen_opencv_java_source
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Copy Java(JAR) source files"
	/usr/bin/cmake -DCONFIG_FILE:PATH=/home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/build/modules/java/jar/copyfiles-JAVA_SRC_COPY.cmake -DCOPYLIST_VAR:STRING=JAVA_SRC_COPY -DDEPHELPER=/home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/build/CMakeFiles/dephelper/opencv_java_jar_source_copy -P /home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/cmake/copy_files.cmake

opencv_java_jar_source_copy: modules/java/jar/CMakeFiles/opencv_java_jar_source_copy
opencv_java_jar_source_copy: CMakeFiles/dephelper/opencv_java_jar_source_copy
opencv_java_jar_source_copy: modules/java/jar/CMakeFiles/opencv_java_jar_source_copy.dir/build.make

.PHONY : opencv_java_jar_source_copy

# Rule to build all files generated by this target.
modules/java/jar/CMakeFiles/opencv_java_jar_source_copy.dir/build: opencv_java_jar_source_copy

.PHONY : modules/java/jar/CMakeFiles/opencv_java_jar_source_copy.dir/build

modules/java/jar/CMakeFiles/opencv_java_jar_source_copy.dir/clean:
	cd /home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/build/modules/java/jar && $(CMAKE_COMMAND) -P CMakeFiles/opencv_java_jar_source_copy.dir/cmake_clean.cmake
.PHONY : modules/java/jar/CMakeFiles/opencv_java_jar_source_copy.dir/clean

modules/java/jar/CMakeFiles/opencv_java_jar_source_copy.dir/depend:
	cd /home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV /home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/modules/java/jar /home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/build /home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/build/modules/java/jar /home/naburoky/Paralela/git_remoto/Paralela/Practicas/OpenCV/build/modules/java/jar/CMakeFiles/opencv_java_jar_source_copy.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : modules/java/jar/CMakeFiles/opencv_java_jar_source_copy.dir/depend

