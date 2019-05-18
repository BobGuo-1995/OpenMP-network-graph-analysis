// compilation command 
To compile the program, you should execute:
 make

To run the program, you can execute the following command line, 
./network input_file output_file number_of_threads
replacing input_file, ouput_file, and number_of_threads as appropriate.

Example of processing dataSmall.txt with one thread: 
./network dataSmall.txt testOutputSmall.txt 1
Example of processing dataMedium.txt with four threads: 
./network dataMedium.txt testOutputMedium.txt 4
