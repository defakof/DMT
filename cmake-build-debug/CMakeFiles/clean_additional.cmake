# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\DMT_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\DMT_autogen.dir\\ParseCache.txt"
  "DMT_autogen"
  )
endif()
