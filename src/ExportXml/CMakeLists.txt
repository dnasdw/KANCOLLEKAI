AUTO_FILES("." "src" "\\.(cpp|h)$")
if(MSVC_VERSION LESS 1600)
  AUTO_FILES("${ROOT_SOURCE_DIR}/msvc" "src" "\\.h$")
endif()
AUTO_FILES("${ROOT_SOURCE_DIR}/dep/libsundaowen" "src" "\\.(cpp|h)$")
AUTO_FILES("${ROOT_SOURCE_DIR}/dep/tinyxml2" "src" "tinyxml2\\.(cpp|h)$")
include_directories(${DEP_INCLUDE_DIR})
if(MSVC_VERSION LESS 1600)
  include_directories("${ROOT_SOURCE_DIR}/msvc")
endif()
link_directories(${DEP_LIBRARY_DIR})
add_definitions(-DSDW_MAIN)
if(MSVC)
  string(REPLACE "/MDd" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  string(REPLACE "/MD" "" CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
  string(REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  string(REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
endif()
ADD_EXE(ExportXml "${src}")
if(CYGWIN)
  target_link_libraries(ExportXml iconv)
endif()
install(TARGETS ExportXml DESTINATION bin)
