if(GCL_BUILD_STATIC)
    add_library(gcl STATIC parser.cc tokenizer.cc)
else()
    add_library(gcl SHARED parser.cc tokenizer.cc)
endif()

target_include_directories(gcl PUBLIC ${PROJECT_SOURCE_DIR}/include)
