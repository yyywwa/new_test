aux_source_directory(./ SRC)

add_library(util SHARED ${SRC})

target_include_directories(util PUBLIC ./)
