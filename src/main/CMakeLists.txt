aux_source_directory(./ SRC)

add_executable(main ${SRC})

target_include_directories(main PRIVATE ${SRCPATH}/util)

link_directories(${LIBPATH})
target_link_libraries(main util)
