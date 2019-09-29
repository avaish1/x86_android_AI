# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
PROJECT(nn_hal_ext CXX)
PROJECT(dldt_plugin CXX)
#add_subdirectory(src/librealsense)
#include_directories(src/librealsense/include)

add_library( lib_opencvcore STATIC IMPORTED)	
add_library( lib_opencvcalib3d STATIC IMPORTED )
add_library( lib_opencvdnn STATIC IMPORTED )
add_library( lib_opencvfeatures2d STATIC IMPORTED )
add_library( lib_opencvflann STATIC IMPORTED )
add_library( lib_opencvhighgui STATIC IMPORTED )
add_library( lib_opencvimgcodecs STATIC IMPORTED )
add_library( lib_opencvimgproc STATIC IMPORTED )
add_library( lib_opencvml STATIC IMPORTED )
add_library( lib_opencvobjdetect STATIC IMPORTED )
add_library( lib_opencvphoto STATIC IMPORTED )

add_library( lib_opencvstitching STATIC IMPORTED )

add_library( lib_opencvvideo STATIC IMPORTED )
add_library( lib_opencvvideoio STATIC IMPORTED )

add_library( lib_opencv SHARED IMPORTED )

set_target_properties(lib_opencvcore PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_core.a)
set_target_properties(lib_opencvcalib3d PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_calib3d.a)
set_target_properties(lib_opencvdnn PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_dnn.a)
set_target_properties(lib_opencvfeatures2d PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_features2d.a)
set_target_properties(lib_opencvflann PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_flann.a)
set_target_properties(lib_opencvhighgui PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_highgui.a)
set_target_properties(lib_opencvimgcodecs PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_imgcodecs.a)
set_target_properties(lib_opencvimgproc PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_imgproc.a)
set_target_properties(lib_opencvml PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_ml.a)
set_target_properties(lib_opencvobjdetect PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_objdetect.a)
set_target_properties(lib_opencvphoto PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_photo.a)

set_target_properties(lib_opencvstitching PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_stitching.a)

set_target_properties(lib_opencvvideo PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_video.a)
set_target_properties(lib_opencvvideoio PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/staticlibs/${ANDROID_ABI}/libopencv_videoio.a)


add_library( lib_inference SHARED IMPORTED )
set_target_properties(lib_inference PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libinference_engine.so)

add_library( lib_mklplugin SHARED IMPORTED )
set_target_properties(lib_mklplugin PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libMKLDNNPlugin.so)

add_library( lib_mkl SHARED IMPORTED )
set_target_properties(lib_mkl PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libmkldnn.so)

add_library( lib_cpuextension SHARED IMPORTED )
set_target_properties(lib_cpuextension PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libcpu_extension.so)

add_library( lib_pug SHARED IMPORTED )
set_target_properties(lib_pug PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libpugixml.a)

set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../opencv/native/libs/${ANDROID_ABI}/libopencv_java4.so)

#add_library( # Sets the name of the library.
#             real_sense-jni

             # Sets the library as a shared library.
#             SHARED

             # Provides a relative path to your source file(s).
#             src/main/cpp/jni/real_sense-jni.cpp )

		

		
add_library( # Sets the name of the library.
             nnhal_ext_jni

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/jni/nnhal_ext.cpp )

include_directories(
             src/main/cpp/include 
			 src/main/cpp/dldt_plugin/include
			 src/main/cpp/include/inference_engine
			 src/main/cpp/include/inference_engine/cpp
			 src/main/cpp/jni
			 		 
             )
# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )




add_subdirectory (src/main/cpp/dldt_plugin)
		 
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.		 
target_link_libraries( nnhal_ext_jni
					   dldt_plugin
					   lib_inference
					   lib_pug
					   lib_mklplugin
					   lib_cpuextension
					   lib_mkl
					   lib_opencv
                       ${log-lib} )


