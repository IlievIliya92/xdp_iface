
IF(CPP_BINDINGS)
    add_subdirectory(cpp)
ENDIF()

IF(PYTHON_BINDINGS)
    add_subdirectory(python)
ENDIF()

IF(RUST_BINDINGS)
    add_subdirectory(rust)
ENDIF()
