# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
PROJECT(dldt_plugin CXX)


# file (GLOB DLDT_PLUGIN_SRC
        # ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/dldt_plugin/src/*.cpp
        # )

# file (GLOB DLDT_PLUGIN_HEADERS
        # ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/dldt_plugin/include/*.h*
        # )
			 
add_library( # Sets the name of the library.
             dldt_plugin

             # Sets the library as a shared library.
             STATIC
				
			 src/face.cpp
			 src/detectors.cpp
			 src/load_dldt.cpp
			 )
             # Provides a relative path to your source file(s).
             #${DLDT_PLUGIN_SRC}
			 #${DLDT_PLUGIN_HEADERS})
find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
target_link_libraries( dldt_plugin
                       lib_inference
					   lib_mklplugin
					   lib_cpuextension
					   lib_mkl
					   ${log-lib} )