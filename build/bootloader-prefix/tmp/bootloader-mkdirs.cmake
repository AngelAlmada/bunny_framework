# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/angelalmada/esp/esp-idf/components/bootloader/subproject"
  "/home/angelalmada/FrameworkBunny/bunny_framework/build/bootloader"
  "/home/angelalmada/FrameworkBunny/bunny_framework/build/bootloader-prefix"
  "/home/angelalmada/FrameworkBunny/bunny_framework/build/bootloader-prefix/tmp"
  "/home/angelalmada/FrameworkBunny/bunny_framework/build/bootloader-prefix/src/bootloader-stamp"
  "/home/angelalmada/FrameworkBunny/bunny_framework/build/bootloader-prefix/src"
  "/home/angelalmada/FrameworkBunny/bunny_framework/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/angelalmada/FrameworkBunny/bunny_framework/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/angelalmada/FrameworkBunny/bunny_framework/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
