cmake_minimum_required(VERSION 3.5.1)

# プロジェクト名
set(project_name vlm_analyzer)
project(${project_name} CXX)

# Build Type(Release or Debug)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

# ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

# コンパイルオプション
add_definitions("-Wall -std=c++14")

# インクルードパス
include_directories(../src/include)
include_directories(${CMAKE_SOURCE_DIR}/../realcore/src/include)

# ライブラリパス
link_directories($ENV{BOOST_DIR}/lib)

# ソースファイル
add_executable(
    ${project_name}
    ${CMAKE_SOURCE_DIR}/../realcore/src/SearchManager.cc
    ${CMAKE_SOURCE_DIR}/../realcore/src/Move.cc
    ${CMAKE_SOURCE_DIR}/../realcore/src/MoveList.cc
    ${CMAKE_SOURCE_DIR}/../realcore/src/LineNeighborhood.cc
    ${CMAKE_SOURCE_DIR}/../realcore/src/OpenState.cc
    ${CMAKE_SOURCE_DIR}/../realcore/src/BoardOpenState.cc
    ${CMAKE_SOURCE_DIR}/../realcore/src/BitBoard.cc
    ${CMAKE_SOURCE_DIR}/../realcore/src/Board.cc
    ../../src/VLMAnalyzer.cc
    ../../src/VLMTranspositionTable.cc
    ../vlm_analyzer.cc
)

# ライブラリ
target_link_libraries(${project_name} boost_program_options)

if(APPLE OR WIN32)
  target_link_libraries(${project_name} boost_system-mt)
  target_link_libraries(${project_name} boost_thread-mt)
else()
  target_link_libraries(${project_name} boost_system)
  target_link_libraries(${project_name} boost_thread)
endif()
