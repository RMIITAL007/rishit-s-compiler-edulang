execute_process(
    COMMAND "${COMPILER}" bench "${SOURCE_FILE}"
    RESULT_VARIABLE benchmark_exit_code
    OUTPUT_VARIABLE benchmark_stdout
    ERROR_VARIABLE benchmark_stderr)

if(benchmark_exit_code EQUAL 0)
    message(FATAL_ERROR "Benchmark mode succeeded after a failure.\nstdout:\n${benchmark_stdout}\nstderr:\n${benchmark_stderr}")
endif()

if(NOT benchmark_stderr MATCHES "${EXPECTED_DIAGNOSTIC}")
    message(FATAL_ERROR "Benchmark mode did not report the expected diagnostic.\nstdout:\n${benchmark_stdout}\nstderr:\n${benchmark_stderr}")
endif()
