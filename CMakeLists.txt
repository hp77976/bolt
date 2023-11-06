cmake_minimum_required(VERSION 3.3.0)
project(bolt CXX C)

message(STATUS ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_STANDARD 17)
set(DEFAULT_BUILD_TYPE "Release")
#unset(CMAKE_BUILD_TYPE)
#unset(DEFAULT_BUILD_TYPE)
#unset(CMAKE_CXX_FLAGS)
#unset(CMAKE_C_FLAGS)
#unset(CMAKE_BUILD_TYPE CACHE)
#unset(DEFAULT_BUILD_TYPE CACHE)
#unset(CMAKE_CXX_FLAGS CACHE)
#unset(CMAKE_C_FLAGS CACHE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmax-errors=8 -fPIC -Wno-dangling-reference")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fmax-errors=8 -fPIC")

add_subdirectory(ext)

include(GNUInstallDirs)

set(CMAKE_HAVE_THREADS_LIBRARY 1)

#add_compile_definitions(EMBREE_IGNORE_INVALID_RAYS=1)
#add_compile_definitions(DETERMINISTIC=1)

set(MISC_OPTS "-fPIC -std=c++17")

#treat warnings as errors and include a bunch of stuff
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmax-errors=8 -fPIC")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fmax-errors=8 -fPIC")

if(CMAKE_CXX_COMPILER STREQUAL "icpx")
	set(CMAKE_C_COMPILER "icx")
	message(STATUS "Using Intel compilers\n")
	set(MISC_OPTIONS "${MISC_OPTIONS} -g -O2 -shared-intel")

	#set(NO_WARN_LIST 
	#	"${NO_WARN_LIST} -Wno-tautological-constant-compare \
	#	-Wno-unused-command-line-argument -Wno-unknown-warning-option")

	#if(DEBUG)
		#set(DEBUG_FLAGS "${DEBUG_FLAGS} -debug inline-debug-info -pg")
	#else()
		#set(OPTIMIZATIONS
		#	"-pg -fast -ffast-math -axAVX2 -funroll-loops \
		#	-fassociative-math -fsee -pipe \
		#	-finline-functions -mbranches-within-32B-boundaries \
		#	-ftree-vectorize -fno-math-errno -fno-signed-zeros \
		#	-fno-trapping-math -fp-model fast -debug inline-debug-info \
		#	-Wno-tautological-constant-compare"
		#)
	#endif()
elseif(CMAKE_CXX_COMPILER STREQUAL "g++")
	set(CMAKE_C_COMPILER "gcc")
	message(STATUS "Using G++/GCC compilers\n")

	set(WARN_LIST
		"-Werror -Wall -Wfatal-errors -Wextra -Wuninitialized -pedantic \
		-Wformat-overflow -fdelete-null-pointer-checks -Wnullereference -Wuninitialized \
		-Wmismatchedealloc -Wsequence-point -Wtrivial-auto-var-init -Wmaybe-uninitialized \
		-Walloc-zero -Warith-conversion -Warray-bounds=2 -Warray-compare -Warray-parameter \
		-Wbool-compare -Wbool-operation -Wfree-nonheap-object -Wunsafe-loop-optimizations \
		-Wpointer-arith -Wcast-align -Wdangling-pointer	-Wlogical-op"
	)

	set(NO_WARN_LIST 
		"-Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable \
		-Wno-unused-parameter"
	)

	set(OPTIMIZATIONS2 #optimizations flags, somehow these give an 8x perf increase
		"${OPTIMIZATIONS} -ftree-loop-linear -funroll-loops -funsafe-loop-optimizations \
		-fno-branch-count-reg -fsched-spec-load -ftree-loop-im -ftree-loop-ivcanon \
		-fivopts -ftracer -fvariable-expansion-in-unroller -freorder-blocks-and-partition \
		-fsingle-precision-constant -funswitch-loops -fbranch-target-load-optimize2 \
		-fvisibility=hidden -fvisibility-inlines-hidden	-static-libgcc -ftree-vectorize \
		-fno-math-errno -fno-trapping-math -fassociative-math \
		-fno-rounding-math -fno-signaling-nans -fcx-limited-range -floop-interchange \
		-floop-strip-mine -floop-block -fsee -ftree-loop-linear -ftree-loop-distribution \
		-ftree-loop-im -fivopts -ftracer -fomit-frame-pointer \
		-fno-stack-protector"
	)
endif()

if(GO_FAST)
	#message(STATUS "Going fast!")
	#add_compile_definitions(SIMD_SELECT_USE_BLEND=1) #this should help, but doesn't seem to
	#add_compile_definitions(SIMD_USE_RCP_FOR_DIV=1) # this breaks stuff
	#add_compile_definitions(BDPT_PRE_ALLOC_MI_WEIGHT=1) #this actually helps, 5-10%?
	#add_compile_definitions(RAY_BOX_USE_BLEND=1)
endif()

#set(FINAL_FLAGS "${WARN_LIST} ${NO_WARN_LIST} ${DEBUG_FLAGS} ${MISC_OPTS} ${OPTIMIZATIONS}")
set(FINAL_FLAGS
	"-Wall -Wextra -Wuninitialized ${NO_WARN_LIST} -Wno-class-memaccess \
	-Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable \
	-Wno-missing-field-initializers -Wno-narrowing \
	-Wno-dangling-reference"
)
set(FINAL_FLAGS "${FINAL_FLAGS} ${MISC_OPTS}")
#set(FINAL_FLAGS "${FINAL_FLAGS} ${OPTIMIZATIONS}")

#enables debugging
#set(FINAL_FLAGS "${FINAL_FLAGS} -O0 -ggdb3 -pg -fno-omit-frame-pointer")

#enables asan
#set(FINAL_FLAGS "${FINAL_FLAGS} \
#	-O2 -ggdb3 -pg -fsanitize=address,undefined \
#	-fno-omit-frame-pointer -fvar-tracking-assignments \
#	--param=max-vartrack-size=147483647"
#)

#enables profiling
#set(FINAL_FLAGS "${FINAL_FLAGS} -O0 -ggdb3 -g -fno-omit-frame-pointer")
#enables optimizations #-ffast-math BREAKS the packet tracer
set(OPTIMS "-fno-stack-protector -fomit-frame-pointer -fsee -fassociative-math -funroll-loops\
			-ftree-vectorize -fno-math-errno -fno-trapping-math\
			-fno-rounding-math -fno-signaling-nans")
set(FINAL_FLAGS "${FINAL_FLAGS} -O2")

#do not use -pg or -ggdb3 outside of debug mode. they cause HUGE slowdowns!
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FINAL_FLAGS} -march=native -fPIC")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${FINAL_FLAGS} -march=native -fPIC")

message(STATUS "Base directory: ${BASE_DIRECTORY}")

#list(APPEND CMAKE_MODULE_PATH "/home/batman/Development/Optix/SDK/CMake/")
#find_package(OptiX REQUIRED)
#find_package(CUDA REQUIRED)
#include_directories("${CUDA_INCLUDE_DIRS}")
#include_directories("/home/batman/Development/Optix/include/")

#this has to go after ext for some reason
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(embree 4 REQUIRED)

set(SOURCES
	src/base/accelerator.cpp	
	src/base/integrator.cpp

	src/core/spectrum.cpp
	src/core/film.cpp
	src/core/math/lambda.cpp
	src/core/scene.cpp

	src/units/importers/imp_accelerator.cpp
	src/units/importers/imp_bin.cpp
	src/units/importers/imp_camera.cpp
	src/units/importers/imp_filter.cpp
	src/units/importers/imp_image.cpp
	src/units/importers/imp_integrator.cpp
	src/units/importers/imp_light.cpp
	src/units/importers/imp_materials.cpp
	src/units/importers/imp_mesh.cpp
	src/units/importers/imp_sampler.cpp

	src/units/accelerators/vk2.cpp
	src/units/cameras/perspective.cpp
	src/units/integrators/path/pt.cpp
	src/units/integrators/depth/depth.cpp
)

#main render executable
add_executable(bolt src/bolt.cpp ${SOURCES})

target_link_libraries(bolt PRIVATE
	Imath::Imath
	OpenEXR::OpenEXR
	OpenEXR::OpenEXRUtil
	OpenEXR::IlmThread
#	${PNG_LIBRARY}
#	${png_static}
	embree
	vk-bootstrap
	vulkan
#	kompute::kompute
)

pybind11_add_module(bolt_module src/bolt_module.cpp ${SOURCES})

target_link_libraries(bolt_module PRIVATE
	Imath::Imath
	OpenEXR::OpenEXR
	OpenEXR::OpenEXRUtil
	OpenEXR::IlmThread
#	${PNG_LIBRARY}
#	${png_static}
	embree
	vk-bootstrap
	vulkan
#	kompute::kompute
)

target_compile_definitions(bolt_module PRIVATE VERSION_INFO="WAAAAAAA")

#add_executable(rng_test src/tests/rng_test.cpp)
#add_executable(embree_test src/tests/embree_test.cpp)
#add_executable(kompute_test src/tests/kompute.cpp)
#target_link_libraries(embree_test PRIVATE embree OpenEXR::IlmThread)
#target_link_libraries(kompute_test PRIVATE embree OpenEXR::IlmThread vk-bootstrap vulkan kompute::kompute)