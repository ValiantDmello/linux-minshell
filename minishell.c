#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<signal.h>

//simple function to remove space from strat and end of string
char* removeSpace(char *s){	
	if(s[0] == ' '){	//if first char is space
		char *s2 = malloc((strlen(s)-1)*sizeof(char));
		strncpy(s2, &s[1], strlen(s)-1);	//copy in s2 without first char
		int j=strlen(s2)-1;
		while(s2[j]==' '){
			s2[j--] = '\0';	//remove any spaces at the end
		}		
		return s2;
	}
	int j=strlen(s)-1;
	while(s[j]==' '){
			s[j--] = '\0'; //remove any spaces at the end
	}
		
	return s;
}

//function to check if redirection exists and change file descriptors accordingly 
//and returns a string with the command to execute by removing the redirection symbol and file name
char *checkredirection(char cmd_main[]){
	if(strstr(cmd_main, ">>")!=NULL){	//check if >> exists in string
		char *cmd_str = strtok(cmd_main, ">>");		//tokenize to split string
													//cmd_str wll store the command
		char *op = removeSpace(strtok(NULL, ">>"));//tokeize to get the output file name	
		
		int fd_op = open(op, O_CREAT | O_RDWR | O_APPEND, 0777);	//open file with append mode
		
		if(fd_op == -1){	//error opening file
			printf("Error in Redirection to file %s\n", op);
			exit(EXIT_FAILURE);
		}	
		dup2(fd_op, 1);		//change output fd to given file fd
		return cmd_str;		//return command to be executed by removing the file name and redirection symbol
	}else if(strstr(cmd_main, ">")!=NULL){	//check if > exists in string
		char *cmd_str = strtok(cmd_main, ">");	//tokenize to split string
												//cmd_str wll store the command
		char *op = removeSpace(strtok(NULL, ">"));	//tokeize to get the output file name	
		
		int fd_op = open(op, O_CREAT | O_RDWR | O_TRUNC, 0777);//open file 
		
		if(fd_op == -1){	//error opening file
			printf("Error in Redirection to file %s\n", op);	
			exit(EXIT_FAILURE);
		}	
		dup2(fd_op, 1);	//change output fd to given file fd
		return cmd_str; //return command to be executed by removing the file name and redirection symbol
	}else if(strstr(cmd_main, "<")!=NULL){	//check if < exists in string
		char *cmd_str = strtok(cmd_main, "<");	//tokenize to split string
												//cmd_str wll store the command
		char *ip = removeSpace(strtok(NULL, "<"));	//tokeize to get the input file name	
		
		int fd_ip = open(ip, O_RDONLY);	//open file 
		
		if(fd_ip == -1){	//error opening file
			printf("Error in Redirection from file %s\n", ip);
			exit(EXIT_FAILURE);
		}	
		dup2(fd_ip, 0);	//change input fd to given file fd
		return cmd_str;	//return command to be executed by removing the file name and redirection symbol
	}
	return cmd_main;	//return same string if redirection doesnt exist 

}

//function to split the command and command args and execute them
int exec_cmd(char cmd_main[]){
	char *cmd_str = checkredirection(cmd_main);	//ned to check if redirection exists before execution of command
	char *cmd_args[7];	//store arguments for command
	char *token = strtok(cmd_str, " ");	//tokenize to split the string
	int i=0;
	while(token!=NULL){	//loop to split the string
		if(i>6){	//more than 6 arguments give error
			printf("ERROR: argc of individual command is more than 6\n");
			exit(EXIT_FAILURE);
		}
		if(token!="" && token!=" "){	//check that token is not empty or a whitespace
			cmd_args[i++] = token;		//add to array
			token = strtok(NULL, " ");	//tokenize
		}
		
	}
	cmd_args[i] = NULL;	//add last argument as NULL for execv 
	char cmd[sizeof(cmd_args[0])];	//command to be executed
	strcpy(cmd, cmd_args[0]); 	//first element of array consistes the command
								//cmd_args contains the argumets for command
	int e = execvp(cmd, cmd_args);	//execute the command
	if(e==-1){
		printf("ERROR: command %s failed\n", cmd);
		exit(EXIT_FAILURE);
	}
}

//recursion function to execute pipes
void pipeexec(char *cmd_arr[], int n, int i){
	int fd[2];	//fd array for piping
	if(pipe(fd)==-1){	//pipe failed
		printf("Error piping\n");
		exit(EXIT_FAILURE);
	}	
	int f = fork();	//fork to pipe commands
	if(f<0){	//fork failed
		printf("Error forking for pipe\n");
		exit(EXIT_FAILURE);
	}else if(f == 0){	//execute the first command from array in child process
		close(fd[0]);	//close input fd of pipe in child process
		dup2(fd[1], 1);	//assign the std output to pipe output in child	
		exec_cmd(cmd_arr[i]);	//execute the command
	}else{
		close(fd[1]);	//close otput fd of parent in parent process
			dup2(fd[0], 0);	//assign standar input to pipe input in parent
		if(i+1 == n){				//base condition or recurssion when only one command left			
			exec_cmd(cmd_arr[i+1]);	//execute the last command	
		}else{	//more than 1 command still remaining
			pipeexec(cmd_arr, n, i+1);	//recurr on rest of the array
		}
	}
}

//function to check for pipes and execute if found
void mspipe(char cmd_list[]){
	char *cmd_arr[6];	//array to store individual commands
	char *token = strtok(cmd_list, "|");	//tokenize string to splitby "|"
	int n=0;
	while(token!=NULL){	//loop split string into individual commands
		if(n>6){	//more that 6 individual command give error
			printf("ERROR: individual commands are more than 6\n");
			exit(EXIT_FAILURE);
		}
		cmd_arr[n++] = token;	//add the individual command to array
		token = strtok(NULL, "|");	//tokenise
		
	}
	if(n == 1){				//no pipes exist
		exec_cmd(cmd_arr[0]);	//execute the command
	}else{
		pipeexec(cmd_arr, n-1, 0);	//execute pipes
	}
}

//recursive function for implemnting the conditonal execution of command array
void condexe(char *delimiter[], char *cmds[], int delcount, int cmdcount, int i, int j){
	int status;
	int pid = fork();	//fork to implement conditional execution
	
	if(pid<0){
		printf("Error Forking at Conditional execution\n");
		exit(EXIT_FAILURE);
	}
	if(pid==0){				//child process
		mspipe(cmds[i]);	//take this command to next stage to check for pipes
	}else{
		int w = waitpid(pid, &status, 0);	//wait for child	
		if(WIFEXITED(status)){	//when child exits
			int exit_status = WEXITSTATUS(status);	//get the exit status of child
			if(j == delcount-1){	//base condition for recurssion when on last command in array
				if(delimiter[j] == "&&"){	//"&&" conditional execution
					if(exit_status == 0){	//check if exit success
						mspipe(cmds[i+1]);	//take this command to next stage to check for pipes
						
					}
				}else{						//"||" conditional execution
					if(exit_status != 0){	//check if exit failure
						mspipe(cmds[i+1]);	//take this command to next stage to check for pipes
						
					}
				}
			}else{		//recursion condition more than 1 commands in array remaining	
				if(delimiter[j] == "&&"){	//"&&" conditional execution
					if(exit_status == 0){	//check if exit success
						condexe(delimiter, cmds, delcount, cmdcount, i+1, j+1);	//recurr for further array
						
					}else{	//if exit failure
						while(delimiter[j]=="&&" && j < delcount){	//check for next "||" condition 
							j++;
							i++;
						}
						i++;
						j++;
						if(i == cmdcount-1 && delimiter[j-1]=="||"){	//if conditonal || found and on last command of array
							mspipe(cmds[i]);							//take this command to next stage to check for pipes
						}else if(i < cmdcount-1 && delimiter[j-1]=="||"){	//if conditonal || found and more than 1 commands in array remaining
							condexe(delimiter, cmds, delcount, cmdcount, i, j);	//recurr for further array
						}
					}
				}else{		//"||" conditional execution same logic as above for "&&" just check for exit failure
					if(exit_status != 0){	//check if exit failure
						condexe(delimiter, cmds, delcount, cmdcount, i+1, j+1);	//recurr for further array
					}else{	//if exit success
						while(delimiter[j]=="||" && j < delcount){	//check for next "&&" condition 
							j++;
							i++;
						}
						i++;
						j++;
						if(i == cmdcount-1 && delimiter[j-1]=="&&"){	//if conditonal && found and more than 1 commands in array remaining
							mspipe(cmds[i]);							//take this command to next stage to check for pipes
						}else if(i < cmdcount-1 && delimiter[j-1]=="&&"){		//if conditonal && found and more than 1 commands in array remaining
							condexe(delimiter, cmds, delcount, cmdcount, i, j);	//recurr for further array
						}
					}
				}
			}
		}
	}
}

//this function checks for conditional execution and implement it if exist
void conditional(char cmd_list[]){
	if(strstr(cmd_list, "&&")!=NULL || strstr(cmd_list, "||")!=NULL){	//check if string consists of "&&" or "||"	
		char *delimiter[5];	//array to store && and || in the order which they are encountered
		char *cmds[6];	//aray to store all individual commands
		int i, j;
		int delcount = 0, cmdcount = 0, st = 0;	//delcount = count of conditional symbloys(&&, ||) cmdcount = count of commands
		for(i=0; i<strlen(cmd_list); i++){		//loop to travel the string and
												//extract the "&&" and "||" symbols in order and get array of commands seperated by them
			if(delcount>5 || cmdcount>6){	//more than 6 individual commands so give errors
				printf("Error: More than 6 individual commands\n");
				exit(EXIT_FAILURE);
			}
			if(cmd_list[i] == '&' && cmd_list[i+1] == '&'){	//"&&" encountered
				delimiter[delcount++] = "&&";	//add && to delmiter
				char *temp = malloc((i-st)*sizeof(char));	//string to store individual command
				strncpy(temp, &cmd_list[st], i-st);		//copy the command to temp string => i is the current index; st is the index where the command starts from 
				cmds[cmdcount++] = temp;	//add that command to array
				i++;	//increament i because next character will be & again
				st = i+1;	//reassign start index to i+1
			}
			if(cmd_list[i] == '|' && cmd_list[i+1] == '|'){	//same logic as above but for "||" command
				delimiter[delcount++] = "||";
				char *temp = malloc((i-st)*sizeof(char));
				strncpy(temp, &cmd_list[st], i-st);
				cmds[cmdcount++] = temp;
				i++;
				st = i+1;
			}
			if(cmd_list[i+1] == '\0'){	//reached the end of string	
				char *temp = malloc((i-st)*sizeof(char));
				strncpy(temp, &cmd_list[st], i-st+1);	//get the last command
				cmds[cmdcount++] = temp;				
				i++;
				st = i+1;
			}
		}
		condexe(delimiter, cmds, delcount, cmdcount, 0, 0);	//conditional execution recursive function
	}else{	//no conditional execution symbols exist
		mspipe(cmd_list);	//take the command to next stage to check for pipes
	}	
}

//This function checks if process is background or not 
void runcmd(char cmd_list[]){
	if(cmd_list[strlen(cmd_list)-1] == '&'){	//check if bg process
		cmd_list[strlen(cmd_list)-1] = '\0';	//remope & from cmd string
		int f = fork();							//fork needed if bg process
		if(f==0){							//child process				
			setpgid(getpid(), getpid());	//change pgid of the bg process
			conditional(cmd_list);			//take the command to next stage to check if conditional execution exists
		}else{			//parent process
			//exit(0);	//continue minishell
		}
	}else{						//if not bg process
		conditional(cmd_list);	//take the command to next stage to check if conditional execution exists
	}
}

//this function recursively executes the commands from sequential exeution
void seqrecur(char *cmd_arr[], int n, int i){
	if(n == i+1){			//base condition recurssion or condition if there is no sequential execution
		runcmd(cmd_arr[i]);		//continue to run the individual command to next stage
	}else{
		int f = fork();		//fork to run sequential commands from cmd_arr array
		if(f<0){
			printf("ERROR forking sequential execution\n");
			exit(EXIT_FAILURE);
		}else if(f == 0){		//child proceaa
			runcmd(cmd_arr[i]);	//continue to run the first command in array to next stage
		}else{							//parent process
			waitpid(f, NULL, 0);		//wait for child execution
			seqrecur(cmd_arr, n, i+1);	//recurr for rest of the array
		}
	}
}

//this function checkes if there are any sequential execution and implements them if they are present
void sequence(char cmd_list[]){		
	char *cmd_arr[6];	
	char *token = strtok(cmd_list, ";");	////tokenize string to split string by ;
	int n=0;
	while(token!=NULL){		//if string contains ;, the string will be delimited by ; and stored in cmd_arr
		if(n>6){
			printf("ERROR: individual commands are more than 6\n");
			exit(EXIT_FAILURE);
		}
		cmd_arr[n++] = token;
		token = strtok(NULL, ";");	//tokenize string
		
	}

	seqrecur(cmd_arr, n, 0);	//execute sequential execution recursively
}

/*void mySigIntHandler(int signo){
	printf("\nms$");
}*/

int main(){
	//signal(SIGINT, msSigIntHandler); //Not sure if minishell need to be sensitive to ^c or not

	for(;;){	//infinite loop for mini shell
		printf("ms$ ");	
		char cmd_list[512];
		fgets(cmd_list, 512, stdin);	//get command from input
		if(cmd_list[0] != '\n'){
			if(cmd_list[strlen(cmd_list)-1] == '\n') 	//fgets scans last char as \n 
				cmd_list[strlen(cmd_list)-1] = '\0';	//so replace it with null
			
			int f = fork();	//fork to run the input command
			
			if(f<0){
				printf("Error forking\n");
			}else if(f == 0){	//child process executes the input command
				sequence(cmd_list);	//checks if sequential execution exists
			}else{	//parent process continues that main minishell
				waitpid(f, NULL, 0);	//wait for childs completion
			}
		}
	}

	//char cmd_list[] = "ls -1 /home/dmellov/Desktop";
	//char cmd_list[] = "cat lab6.c|grep std|wc|wc -w";
	//char cmd_list[] = "ls -1 /home/dmellov/Desktop | wc";
	//char cmd_list[] = "ls -1 | wc";
	//char cmd_list[] = "ls -1";
	//char cmd_list[] = "ls -1>dislist.txt";
	//char cmd_list[] = "ls -1 | wc>>dislist.txt";
	//char cmd_list[]= "cat<dislist.txt";
	//char cmd_list[] = "./ex0 && echo here";
	//char cmd_list[] = "./ex1 || ./ex0 || ./ex1  && ls -1 | wc -w";
	//char cmd_list[] = "sleep 100 &";
	//char cmd_list[] = "echo sequence; ls -1 | wc; cat test.c | wc -w";
	
	
	return 0;
}
