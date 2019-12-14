cmake_minimum_required(VERSION 3.5)

project(Grep LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wconversion -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wunused")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(Grep
  main.cpp
  grep.cpp grep.h
  grep.ui
  searcher.cpp searcher.h
  directorywalker.cpp directorywalker.h
  filesearcher.cpp filesearcher.h
  taskqueue.cpp taskqueue.h
  search.cpp search.h
  pool.cpp pool.h
)

#add_executable(SearchTest
#  searcher.cpp searcher.h
#  search_test.cpp
#)

target_link_libraries(Grep PRIVATE Qt5::Widgets)
#target_link_libraries(SearchTest PRIVATE Qt5::Widgets)
