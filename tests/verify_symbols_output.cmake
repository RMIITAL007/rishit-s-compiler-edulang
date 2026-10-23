execute_process(
    COMMAND "${COMPILER}" symbols "${SOURCE_FILE}"
    RESULT_VARIABLE symbols_exit_code
    OUTPUT_VARIABLE symbols_stdout
    ERROR_VARIABLE symbols_stderr)

if(NOT symbols_exit_code EQUAL 0)
    message(FATAL_ERROR "Symbol-table mode failed.\nstdout:\n${symbols_stdout}\nstderr:\n${symbols_stderr}")
endif()

foreach(expected_line
    "Source Name  Storage Name  Scope  Status        Type  Declared At"
    "x  x  global  active  int  1:5"
    "inner  inner  block 1  out of scope  int  3:6"
    "x  x$1  block 1  out of scope  int  4:6"
    "x  x$2  block 1  out of scope  int  7:6")
    string(FIND "${symbols_stdout}" "${expected_line}" found_index)
    if(found_index EQUAL -1)
        message(FATAL_ERROR "Symbol-table output is missing '${expected_line}'.\nstdout:\n${symbols_stdout}\nstderr:\n${symbols_stderr}")
    endif()
endforeach()
