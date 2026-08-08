include_guard(GLOBAL)

function(vyra_apply_compiler_flags TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            /W4
            /permissive-
            /utf-8
            /Zc:preprocessor
            /EHsc
            /MP
        )
        target_compile_definitions(${TARGET_NAME} PRIVATE
            _CRT_SECURE_NO_WARNINGS
            UNICODE
            _UNICODE
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wno-unused-parameter
        )
    endif()
endfunction()
