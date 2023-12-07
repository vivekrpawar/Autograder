all : gradingserver

gradingserver: gradingserver.cpp file_utils.h server_utils.h recv_file_util.cpp
	gcc -o gradingserver gradingserver.cpp file_utils.h server_utils.h recv_file_util.cpp

clean: 
	rm -r compiler_err_dir comp_opt_dir execulable_dir log_dir output_dir results_dir runtime_error_dir
