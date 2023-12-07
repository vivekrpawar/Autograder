#ifndef GRADERUTILS_H
#define GRADERUTILS_H 

// size of required bufferes and queues
const int MAX_FILE_SIZE_BYTES = 4;
const int MAX_QUEUE_SIZE = 100;
const int MAX_REQID_SIZE = 1024;
const int BUFFER_SIZE = 1024;
const int MAX_QUEUE = 50;

// All required Directories
const char SUBMISSION_DIR[] = "./submission_dir/";
const char EXECUTABLE_DIR[] = "./executable_dir/";
const char OUTPUT_DIR[] = "./output_dir/";
const char COMPILER_ERR_DIR[] = "./compiler_err_dir/";
const char RUNTIME_ERR_DIR[] = "./runtime_err_dir/";
const char EXPECTED_OUTPUT[] = "./expected_output_dir/expected_output.txt";
const char COMPARE_OPT_DIR[] = "./comp_opt_dir/";
const char RESULT_DIR[] = "./result_dir/";
const char LOG_DIR[] = "./log_dir/";
const char LOG_FILE[] = "./log_dir/log.txt";
const char PASS_MSG[] = "PROGRAM RAN\n";
const char COMPILER_ERROR_MSG[] = "COMPILER ERROR\n";
const char RUNTIME_ERROR_MSG[] = "RUNTIME ERROR\n";
const char OUTPUT_ERROR_MSG[] = "OUTPUT ERROR\n";

#endif