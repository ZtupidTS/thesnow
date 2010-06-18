### Select the build type
# Use Release/Devel/Debug     : -DCMAKE_BUILD_TYPE=Release|Devel|Debug
# Enable/disable the stipping : -DCMAKE_BUILD_STRIP=TRUE|FALSE
### Force the choice of 3rd party library in pcsx2 over system libraries
# Use all         internal lib: -DFORCE_INTERNAL_ALL=TRUE
# Use bzip        internal lib: -DFORCE_INTERNAL_BZIP2=TRUE
# Use soundtouch  internal lib: -DFORCE_INTERNAL_SOUNDTOUCH=TRUE
# Use zlib        internal lib: -DFORCE_INTERNAL_ZLIB=TRUE
#-------------------------------------------------------------------------------


#-------------------------------------------------------------------------------
# if no build type is set, use Devel as default
# Note without the CMAKE_BUILD_TYPE options the value is still defined to ""
# Ensure that the value set by the User is correct to avoid some bad behavior later
#-------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE MATCHES "Debug|Devel|Release")
	set(CMAKE_BUILD_TYPE Devel)
	message(STATUS "BuildType set to ${CMAKE_BUILD_TYPE} by default")
endif(NOT CMAKE_BUILD_TYPE MATCHES "Debug|Devel|Release")
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Set default strip option. Can be set with -DCMAKE_BUILD_STRIP=TRUE/FALSE
#-------------------------------------------------------------------------------
if(NOT DEFINED CMAKE_BUILD_STRIP)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_BUILD_STRIP TRUE)
        message(STATUS "Enable the stripping by default in ${CMAKE_BUILD_TYPE} build !!!")
    else(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_BUILD_STRIP FALSE)
        message(STATUS "Disable the stripping by default in ${CMAKE_BUILD_TYPE} build !!!")
    endif(CMAKE_BUILD_TYPE STREQUAL "Release")
endif(NOT DEFINED CMAKE_BUILD_STRIP)

#-------------------------------------------------------------------------------
# Select library system vs 3rdparty
#-------------------------------------------------------------------------------
if(FORCE_INTERNAL_ALL)
    set(FORCE_INTERNAL_BZIP2 TRUE)
    set(FORCE_INTERNAL_SOUNDTOUCH TRUE)
    set(FORCE_INTERNAL_ZLIB TRUE)
endif(FORCE_INTERNAL_ALL)

if(NOT DEFINED FORCE_INTERNAL_BZIP2)
    set(FORCE_INTERNAL_BZIP2 FALSE)
endif(NOT DEFINED FORCE_INTERNAL_BZIP2)

if(NOT DEFINED FORCE_INTERNAL_SOUNDTOUCH)
    set(FORCE_INTERNAL_SOUNDTOUCH TRUE)
    message(STATUS "Use internal version of Soundtouch by default.
    Note: There have been issues in the past with sound quality depending on the version of Soundtouch
    Use -DFORCE_INTERNAL_SOUNDTOUCH=FALSE at your own risk")
    # set(FORCE_INTERNAL_SOUNDTOUCH FALSE)
endif(NOT DEFINED FORCE_INTERNAL_SOUNDTOUCH)

if(NOT DEFINED FORCE_INTERNAL_ZLIB)
    set(FORCE_INTERNAL_ZLIB FALSE)
endif(NOT DEFINED FORCE_INTERNAL_ZLIB)
