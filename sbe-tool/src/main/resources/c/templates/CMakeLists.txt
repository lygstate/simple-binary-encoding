#
# Copyright 2013-2020 Real Logic Limited.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

function(sbe_test name source)
    add_executable("C${name}" "${source}")
    target_include_directories("C${name}"
        PRIVATE ${GTEST_SOURCE_DIR}/googletest/include
        PRIVATE ${C_CODEC_TARGET_DIR}
    )
    target_compile_options("C${name}" PRIVATE $<$<CXX_COMPILER_ID:AppleClang>:-Werror>)
    target_compile_options("C${name}" PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Werror>)
    target_compile_options("C${name}" PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Werror -Wno-maybe-uninitialized>)
    target_link_libraries("C${name}" ${GTEST_LIBS} ${CMAKE_THREAD_LIBS_INIT})
    add_test(NAME C${name} COMMAND C${name} WORKING_DIRECTORY ${C_CODEC_TARGET_DIR})
    add_dependencies(C${name} gtest)

endfunction()

# codec tests
sbe_test(SbecCheck SbecCheck.cpp)

sbe_test(SbecCheckEmpty SbecCheckEmpty.c)

