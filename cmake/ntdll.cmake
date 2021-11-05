
if (NOT TARGET ntdll)
    add_library(ntdll INTERFACE)
    target_include_directories(ntdll INTERFACE "${CMAKE_CURRENT_LIST_DIR}/ntdll")

    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_libraries(ntdll INTERFACE "${CMAKE_CURRENT_LIST_DIR}/ntdll/ntdll_x64.lib")
    else()
        target_link_libraries(ntdll INTERFACE "${CMAKE_CURRENT_LIST_DIR}/ntdll/ntdll_x86.lib")
    endif()
endif()