//Operating Systems
//Project 1
//Ryan Moran

// manual implementation of an operating system's ability to copy a file

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

void display_message(){
	printf("copyit: still copying...\n");
	alarm(1);
}

int main (int argc, char *argv[] ){
	signal(SIGALRM,display_message);
	char* sourceFile = argv[1]; //take first command line argument as source file
	char* targetFile = argv[2]; //second command line argument is target file
	char buffer[1000]; 		    //buffer used for read and write

	int read_result;	    // will keep track of read return value for error checking
	int write_result;           //keeps track of write return value for error checking
	int chmod_return;           //keeps track of chmod return value for error checking

	int bytesCopied=0;         //used to return total bytes transferred at end of copy
	
	int file_descriptor_source = open(sourceFile,O_RDONLY); //open the source file in read only mode
	if(file_descriptor_source<0) { //check for errors opening source file
		printf("Unable to open %s: %s\n",sourceFile,strerror(errno)); //report error
		exit(1);
	}

	int file_descriptor_target = creat(targetFile, O_RDWR); //create the target file to copy to
	if(file_descriptor_target<0) { //check for errors creating target file
		printf("Unable to create %s: %s\n",targetFile,strerror(errno)); //report error
		exit(1);
	}

	alarm(1); //set alarm so that every second from now on, message will be displayed

	int endloop = 0;
	while (endloop!=1){

		read_result = read(file_descriptor_source, &buffer, 1000); //read from sourceFile into buffer 1000 bytes at a time
		
		if (read_result == 0){ //done reading file, so exit loop
			endloop=1;
		}
		else if (read_result<0 && errno == EINTR){ //if read was interrupted, redo the read
			//printf("Did this");
			read_result = read(file_descriptor_source, &buffer, 1000);
		}
		else if (read_result < 0){ //if there was some other problem reading
			printf("Problem reading from %s: %s\n",sourceFile,strerror(errno)); //report error
			exit(1);
		}
		
		if (read_result>0){ //only want to write to target if additional byte(s) were actually read
			bytesCopied = bytesCopied + read_result; //update bytesCopied with each read
			write_result = write(file_descriptor_target, &buffer, read_result); //write from buffer to targetFile
			if (write_result<0 && errno == EINTR){ //if write was interrupted, redo the write
				//printf("Did this");
				write_result = write(file_descriptor_target, &buffer, read_result);
			}
			else if (write_result<0){ //if some other error, report it and exit
				printf("Problem writing to %s: %s\n",targetFile,strerror(errno)); //report error
				exit(1);
			}	
		}		
	}

	chmod_return = syscall(SYS_chmod, targetFile, 0666); //change target permissions to read and write; creat not setting for some reason
	if (chmod_return<0){ //report if some error occurred with chmod
  		printf("Chmod failed to change permissions on %s: %s\n",targetFile,strerror(errno));
	}

	close(file_descriptor_source);
	close(file_descriptor_target); //close the read and write files

	printf("copyit: Copied %d bytes from file '%s' to file '%s'.\n", bytesCopied, sourceFile, targetFile); //report bytes copied
	exit(0);
}
