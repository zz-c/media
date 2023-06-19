# 设置文件的查找位置
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3party/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)

link_directories("E:/clib/librtmp/x86")

# link_libraries(librtmp_send264 librtmp.lib)

add_executable (librtmp_send264 ${SRC_LIST})

target_link_libraries(librtmp_send264 librtmp)