
if (NOT TARGET TitanEngine)
    add_library(TitanEngine INTERFACE)
    target_include_directories(TitanEngine INTERFACE "${CMAKE_CURRENT_LIST_DIR}/TitanEngine")

    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_libraries(TitanEngine INTERFACE "${CMAKE_CURRENT_LIST_DIR}/TitanEngine/lib/x64/TitanEngine.lib")
        set(TitanEngineDLL "${CMAKE_CURRENT_LIST_DIR}/TitanEngine/lib/x64/TitanEngine.dll")
    else()
        target_link_libraries(TitanEngine INTERFACE "${CMAKE_CURRENT_LIST_DIR}/TitanEngine/lib/x86/TitanEngine.lib")
        set(TitanEngineDLL "${CMAKE_CURRENT_LIST_DIR}/TitanEngine/lib/x86/TitanEngine.dll")
    endif()

    function(CopyTitanEngineDLL target)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${TitanEngineDLL} $<TARGET_FILE_DIR:${target}>
            )
    endfunction()
endif()