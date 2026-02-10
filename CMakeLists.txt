cmake_minimum_required(VERSION 3.20)
project(CppOpenScraper VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# lexbor submodule (static, linked into our shared lib)
set(LEXBOR_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LEXBOR_INSTALL_HEADERS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/lexbor)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(TARGET "cppscraper")
add_library(${TARGET} SHARED
	src/Scraper.cpp
	src/Fetcher.cpp
	src/LexborDocument.cpp
	src/ContentExtractor.cpp
)

set_property(TARGET ${TARGET} PROPERTY AUTOMOC ON)
target_compile_options(${TARGET} PRIVATE -fPIC)

target_include_directories(${TARGET}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/lexbor/source
)

target_link_libraries(${TARGET}
	PUBLIC Qt6::Core Qt6::Network
	PRIVATE lexbor_static
)

# Optional: example executable
option(CPPSCRAPER_BUILD_EXAMPLES "Build example programs" OFF)
if(CPPSCRAPER_BUILD_EXAMPLES)
	add_executable(scrape_url examples/scrape_url.cpp)
	target_link_libraries(scrape_url PRIVATE ${TARGET})
endif()
