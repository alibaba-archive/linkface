# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})

include_directories(src/main/jni/include)


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             facedoor

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             #src/main/jni/linkkit_export.c
             #src/main/jni/linkkit_sample.c
             #src/main/jni/lite_queue.c
             src/main/jni/facedoor.c)

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

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
message("target abi:"${ANDROID_ABI})

include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)

#if (${ANDROID_ABI} STREQUAL "x86_64")
#    include_directories(${ANDROID_SYSROOT}/usr/include/x86_64-linux-android)
#elseif (${ANDROID_ABI} STREQUAL "x86")
#    include_directories(${ANDROID_SYSROOT}/usr/include/i686-linux-android)
#elseif (${ANDROID_ABI} STREQUAL "arm64-v8a")
#    include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
#elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
#    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
#endif()

SET(LIB_TLS
    -Wl,--whole-archive
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libiot_tls.a
    -Wl,--no-whole-archive)

SET(LIB_SDK
    -Wl,--whole-archive
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libiot_sdk.a
    -Wl,--no-whole-archive)

SET(LIB_PLATFORM
    -Wl,--whole-archive
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libiot_platform.a
    -Wl,--no-whole-archive)

SET(LIB_TFS
    -Wl,--whole-archive
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libiot_tfs.a
    -Wl,--no-whole-archive)


target_link_libraries( facedoor
                       #${LIB_TFS}
                       ${LIB_SDK} ${LIB_PLATFORM} ${LIB_TLS}

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )