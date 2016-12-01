
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS OFF)

include(DetectCPUFeatures)
include(FindCxaDemangle)

CheckAndAppendCompilerFlags(${CMAKE_BUILD_TYPE} "-Wall")
CheckAndAppendCompilerFlags(${CMAKE_BUILD_TYPE} "-Wextra")
CheckAndAppendCompilerFlags("Release" "-ffast-math")
CheckAndAppendCompilerFlags("Release" "-funroll-loops")
CheckAndAppendCompilerFlags("Release" "-D__extern_always_inline=\"extern __always_inline\" ")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	CheckAndAppendCompilerFlags("Release" "-no-inline-min-size -no-inline-max-size")
endif ()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	CheckAndAppendCompilerFlags("Release" "/EHsc")
    CheckAndAppendCompilerFlags("Debug" "/Od")
    CheckAndAppendCompilerFlags("Release" "/Ox")
    set(vars CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    foreach(var ${vars})
        string(REPLACE "/MD" "-MT" ${var} "${${var}}")
    endforeach(var)
    add_definitions("/DMSVC_COMPILER")
endif ()

if ( CODE_COVERAGE )
    CheckAndAppendCompilerFlags("Debug" "-g -fprofile-arcs -ftest-coverage -lgcov")
endif()

if(APPLE)
    message("found APPLE")
    if(USE_LIBCPP)
	message("use LIBC++")
        CheckAndAppendCompilerFlags(${CMAKE_BUILD_TYPE} "-stdlib=libc++")
    endif()
endif()