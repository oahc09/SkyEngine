function(sky_add_exe)
    cmake_parse_arguments(TMP
        "WIN32;MACOSX_BUNDLE"
        "TARGET"
        "SOURCES;INCS;LIBS"
        ${ARGN}
    )

    if (NOT TMP_TARGET)
        message("target not set")
    endif()

    unset(TMP_TYPE)
    if (TMP_WIN32)
        set(TMP_TYPE WIN32)
    elseif(TMP_MACOSX_BUNDLE)
        set(TMP_TYPE MACOSX_BUNDLE)
    endif ()

    add_executable(${TMP_TARGET} ${TMP_TYPE} ${TMP_SOURCES})
    foreach (dep ${TMP_LINK_LIBS})
        if (TARGET ${dep})
            get_target_property(type ${dep} TYPE)
            if (${type} STREQUAL "INTERFACE_LIBRARY")
                get_target_property(tmpInc ${dep} INTERFACE_INCLUDE_DIRECTORIES)
                get_target_property(tmpLib ${dep} INTERFACE_LINK_LIBRARIES)
                list(APPEND TMP_INCS ${tmpInc})
                list(APPEND TMP_LIBS ${tmpLib})
            endif()
        endif()
    endforeach()
    target_include_directories(${TMP_TARGET} PRIVATE ${TMP_INCS})
    target_link_libraries(${TMP_TARGET} ${TMP_LIBS})


endfunction()

function(sky_add_library)
    cmake_parse_arguments(TMP
        "STATIC;SHARED"
        "TARGET"
        "SOURCES;PRIVATE_INC;PUBLIC_INC;LINK_LIBS"
        ${ARGN}
    )

    if (NOT TMP_TARGET)
        message("target not set")
    endif()

    unset(TMP_TYPE)
    if (TMP_STATIC)
        set(TMP_TYPE STATIC)
    elseif(TMP_SHARED)
        set(TMP_TYPE SHARED)
    endif()

    add_library(${TMP_TARGET} ${TMP_TYPE}
        ${TMP_SOURCES}
    )

    set(LINK_LIBRARIES ${TMP_LINK_LIBS})
    foreach (dep ${TMP_LINK_LIBS})
        if (TARGET ${dep})
            get_target_property(type ${dep} TYPE)
            if (${type} STREQUAL "INTERFACE_LIBRARY")
                get_target_property(tmpInc ${dep} INTERFACE_INCLUDE_DIRECTORIES)
                if (tmpInc)
                    list(APPEND TMP_PUBLIC_INC ${tmpInc})
                endif()
                get_target_property(tmpLib ${dep} INTERFACE_LINK_LIBRARIES)
                if (tmpLib)
                    list(APPEND LINK_LIBRARIES ${tmpLib})
                endif()

                get_target_property(tmpDlls ${dep} INTERFACE_DYN_LIBS)
                if (tmpDlls)
                    foreach(dll ${tmpDlls})
                        get_filename_component(dllName ${dll} NAME)
                        add_custom_command(TARGET ${TMP_TARGET} POST_BUILD
                                COMMAND ${CMAKE_COMMAND} -E copy
                                ${dll}
                                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/${dllName}
                                )
                    endforeach()
                endif()
            endif()
        endif()
    endforeach()

    target_include_directories(${TMP_TARGET} PRIVATE ${TMP_PRIVATE_INC})
    target_include_directories(${TMP_TARGET} PUBLIC ${TMP_PUBLIC_INC})
    target_link_libraries(${TMP_TARGET} ${LINK_LIBRARIES})

endfunction()

function(sky_add_test)
    cmake_parse_arguments(TMP
        ""
        "TARGET;WORKING_DIR"
        "SOURCES;INCS;LIBS"
        ${ARGN}
        )

    if (NOT TMP_TARGET)
        message("target not set")
    endif()

    add_executable(${TMP_TARGET} ${TMP_TYPE} ${TMP_SOURCES})
    foreach (dep ${TMP_LINK_LIBS})
        if (TARGET ${dep})
            get_target_property(type ${dep} TYPE)
            if (${type} STREQUAL "INTERFACE_LIBRARY")
                get_target_property(tmpInc ${dep} INTERFACE_INCLUDE_DIRECTORIES)
                get_target_property(tmpLib ${dep} INTERFACE_LINK_LIBRARIES)
                list(APPEND TMP_INCS ${tmpInc})
                list(APPEND TMP_LIBS ${tmpLib})
            endif()
        endif()
    endforeach()
    target_include_directories(${TMP_TARGET} PRIVATE ${TMP_INCS})
    target_link_libraries(${TMP_TARGET} ${TMP_LIBS})
    add_test(
        NAME ${TMP_TARGET}
        COMMAND ${TMP_TARGET}
        WORKING_DIRECTORY ${TMP_WORKING_DIR}
    )
endfunction()

function(sky_add_dependency)
    cmake_parse_arguments(TMP
        ""
        "TARGET"
        "DEPENDENCIES"
        ${ARGN}
        )

    if (NOT TMP_TARGET)
        message("target not set")
    endif()

    foreach (dep ${TMP_DEPENDENCIES})
        get_property(CURR_PROP GLOBAL PROPERTY ${dep}_DEP)
        list(APPEND CURR_PROP ${TMP_TARGET})
        set_property(GLOBAL PROPERTY ${dep}_DEP "${CURR_PROP}")
    endforeach()
endfunction()

function(sky_set_dependency)
    cmake_parse_arguments(TMP
        ""
        "TARGET"
        ""
        ${ARGN}
        )

    if (TMP_TARGET)
        get_property(CURR_PROP GLOBAL PROPERTY ${TMP_TARGET}_DEP)
        add_dependencies(${TMP_TARGET} ${CURR_PROP})
    endif()
endfunction()