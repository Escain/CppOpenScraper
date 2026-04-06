set(CMAKE_AUTOMOC ON)

add_executable(cppscraper_tests
	ContentExtractorTest.cpp
)

target_include_directories(cppscraper_tests PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/external/lexbor/source
)

# UnitTest.hpp from the parent project's external/unittest
# When built standalone, this path must be provided via
# -DUNITTEST_INCLUDE_DIR=<path>
if(NOT DEFINED UNITTEST_INCLUDE_DIR)
	# Default: assume we are inside the trade_advisor tree
	set(UNITTEST_INCLUDE_DIR
		${CMAKE_SOURCE_DIR}/../../external/unittest
		CACHE PATH "Path to UnitTest.hpp directory")
endif()

target_include_directories(cppscraper_tests PRIVATE
	${UNITTEST_INCLUDE_DIR}
)

target_link_libraries(cppscraper_tests PRIVATE
	cppscraper
	lexbor_static
	Qt6::Core
	Qt6::Network
)

# Pass fixture path as compile definition
target_compile_definitions(cppscraper_tests PRIVATE
	FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

add_test(NAME cppscraper_tests COMMAND cppscraper_tests)
