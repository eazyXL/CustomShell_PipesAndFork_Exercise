#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>

int bpid = NULL;
int ctrlc = 0;


void evenSpace(char *line, char* final){
	char new[1024];
	int index = 0;
	for(int i = 0; i < 1024; i++){
		if(line[i] == '|' || line[i] == ';' || line[i] == '>' || line[i] == '<' || line[i] == '&'){
			if(line[i+1] == '\0'){
				final[index] = ' ';
				index++;
				final[index] = line[i];
				index++;
				final[index] = '\0';
				break;
			}
			final[index] = ' ';
			index++;
			final[index] = line[i];
			index++;
			final[index] = ' ';
			index++;
		}else{
			final[index] = line[i];
			index++;
		}	 
	}

}

void zeroSym(char *line){

		
		//printf("zerSym %s\n", line);
		char *split[20][20] = {NULL};
		char *temp;

			temp = strtok(line, " ");
			int i = 0;
			while(temp != NULL){
				strcpy(split[i], temp);
				i++;
				temp = strtok(NULL, " ");
			}

		char *newargs[i];
		newargs[i] = NULL;
		int x =i; int j;
		for(i = 0; i < x; i++){
			
			newargs[i] = malloc(strlen(split[i]) + 1);
			
			strcpy(newargs[i], split[i]);
			
			(newargs[i])[strlen(split[i])] = '\0';
		}

	//for(int n = 0; n <i; n++){
		//printf("%i %s ", n, newargs[n]);
	//} 
	//printf("\n");

	pid_t cpid = 0;

	setbuf(stdout, NULL);

	cpid = fork();
	if(cpid < 0){
		fprintf(stderr, "fork failed: %s\n", strerror(errno));
	}else if(cpid == 0){ //child
		//printf("IN CHILD: pid =%d\n", getpid());
		execvp(newargs[0], newargs);
		exit(0);

	}else{ //parent

		int *status = malloc(sizeof(int));
		pid_t ret1 = waitpid(bpid, status, WNOHANG);

			
		int ret = wait(NULL);
		if(ret1 > 0){
			printf("... child process (PID=%d) is waited/reaped\n", ret1);
		}
		if(ret < 0){
			//fprintf(stderr, "wait failed: %s\n", strerror(errno));
		}else{
			printf("... child process (PID=%d) is waited/reaped\n", cpid);
		}

	}

	
}


void oneSym(char *line, char *line2, char sym){
		//printf("zerSym %s\n", line);


	if(sym == ';'){
		zeroSym(line);
		zeroSym(line2);
	}else{

		//////split line 1////////////////////////////////
		char *split[20][20] = {NULL};
		char *temp;

			temp = strtok(line, " ");
			int i = 0;
			while(temp != NULL){
				strcpy(split[i], temp);
				i++;
				temp = strtok(NULL, " ");
			}

		char *newargs[i];
		newargs[i] = NULL;
		int x =i;
		for(i = 0; i < x; i++){
			
			newargs[i] = malloc(strlen(split[i]) + 1);
			
			strcpy(newargs[i], split[i]);
			
			(newargs[i])[strlen(split[i])] = '\0';
		}
		//////split line 1////////////////////////////////

		//////split line 2////////////////////////////////
		char *split2[20][20] = {NULL};
		char *temp2;

			temp2 = strtok(line2, " ");
			int j = 0;
			while(temp2 != NULL){
				strcpy(split2[j], temp2);
				j++;
				temp2 = strtok(NULL, " ");
			}

		char *newargs2[j];
		newargs2[j] = NULL;
		 x =j;
		for(j = 0; j < x; j++){
			
			newargs2[j] = malloc(strlen(split2[j]) + 1);
			
			strcpy(newargs2[j], split2[j]);
			
			(newargs2[j])[strlen(split2[j])] = '\0';
		}
		//////split line 2////////////////////////////////

	/*for(int n = 0; n <i; n++){
		printf("%i %s ", n, newargs[n]);
	} 
	printf("\n");
		for(int n = 0; n <j; n++){
		printf("%i %s ", n, newargs2[n]);
	} 
	printf("\n"); */

		if(sym == '&'){
			pid_t cpid = 0;

			setbuf(stdout, NULL);
	
			cpid = fork();
			if(cpid < 0){
				fprintf(stderr, "fork failed: %s\n", strerror(errno));
			}else if(cpid == 0){ //child
				//printf("IN CHILD: pid =%d\n", getpid());
				
				execvp(newargs[0], newargs);
				exit(0);
			}else{
				bpid = cpid;
			}
		}

		if(sym == '|'){
			pid_t cpid[2] = {0};
			int fd[2] = {0};
			int ret = 0;
			
			setbuf(stdout, NULL);

			if(pipe(fd) < 0){
				fprintf(stderr, "pipe() failed: %s\n", strerror(errno));
				return 0;
			}

			cpid[0] = fork();
			if(cpid[0] < 0){
				fprintf(stderr, "fork() 1 failed: %s\n", strerror(errno));
				return 0;
			}
			else if(cpid[0] == 0){//child 1
				printf("");
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
				if(execvp(newargs[0], newargs) < 0){
					fprintf(stderr, "Cannot execute \"%s\": %s\n", newargs[0], strerror(errno));
					exit(-1);
				}
			}
			
			cpid[1] = fork();
			if(cpid[1] < 0){
				fprintf(stderr, "fork() 2 failed: %s\n", strerror(errno));
				return 0;
			}
			else if(cpid[1] == 0){//child 2
				printf("");
				dup2(fd[0], STDIN_FILENO);
				close(fd[0]);
				close(fd[1]);
				if(execvp(newargs2[0], newargs2) < 0){
					fprintf(stderr, "Cannot execute \"%s\": %s\n", newargs2[0], strerror(errno));
					exit(-1);
				}
			}
			
			close(fd[0]);
			close(fd[1]);
			while((ret = wait(NULL)) > 0){
				printf("... child process (PID=%d) is waited/reaped\n", ret);
			}

		}// '|'	

		if(sym == '>'){

			pid_t cpid = 0;

			setbuf(stdout, NULL);
	
			cpid = fork();
			if(cpid < 0){
				fprintf(stderr, "fork failed: %s\n", strerror(errno));
			}else if(cpid == 0){ //child
				//printf("IN CHILD: pid =%d\n", getpid());
				int fd = open(newargs2[0], O_CREAT | O_RDWR | O_TRUNC);
				dup2(fd, 1);
				close(fd);				
				execvp(newargs[0], newargs);
				exit(0);
			}else{//parent
				int ret = wait(NULL);

				if(ret < 0){
					fprintf(stderr, "wait failed: %s\n", strerror(errno));
				}else{
					printf("... child process (PID=%d) is waited/reaped\n", cpid);
				}
			}
			
		}// >

		if(sym == '<'){

			pid_t cpid = 0;

			setbuf(stdout, NULL);
	
			cpid = fork();
			if(cpid < 0){
				fprintf(stderr, "fork failed: %s\n", strerror(errno));
			}else if(cpid == 0){ //child
				//printf("IN CHILD: pid =%d\n", getpid());
				int fd = open(newargs2[0], O_RDONLY, 0);
				dup2(fd, STDIN_FILENO);
				close(fd);				
				execvp(newargs[0], newargs);
				exit(0);
			}else{//parent
				int ret = wait(NULL);

				if(ret < 0){
					fprintf(stderr, "wait failed: %s\n", strerror(errno));
				}else{
					printf("... child process (PID=%d) is waited/reaped\n", cpid);
				}
			}
			
		}// <







	}//else

}

void twoSym(char *line, char *line2, char *line3, char sym1, char sym2){
//////split line 1////////////////////////////////
		char *split[20][20] = {NULL};
		char *temp;

			temp = strtok(line, " ");
			int i = 0;
			while(temp != NULL){
				strcpy(split[i], temp);
				i++;
				temp = strtok(NULL, " ");
			}

		char *newargs[i];
		newargs[i] = NULL;
		int x =i;
		for(i = 0; i < x; i++){
			
			newargs[i] = malloc(strlen(split[i]) + 1);
			
			strcpy(newargs[i], split[i]);
			
			(newargs[i])[strlen(split[i])] = '\0';
		}
		//////split line 1////////////////////////////////

		//////split line 2////////////////////////////////
		char *split2[20][20] = {NULL};
		char *temp2;

			temp2 = strtok(line2, " ");
			int j = 0;
			while(temp2 != NULL){
				strcpy(split2[j], temp2);
				j++;
				temp2 = strtok(NULL, " ");
			}

		char *newargs2[j];
		newargs2[j] = NULL;
		 x =j;
		for(j = 0; j < x; j++){
			
			newargs2[j] = malloc(strlen(split2[j]) + 1);
			
			strcpy(newargs2[j], split2[j]);
			
			(newargs2[j])[strlen(split2[j])] = '\0';
		}
		//////split line 2////////////////////////////////

		//////split line 3////////////////////////////////
		char *split3[20][20] = {NULL};
		char *temp3;

			temp3 = strtok(line3, " ");
			int b = 0;
			while(temp3 != NULL){
				strcpy(split3[b], temp3);
				b++;
				temp3 = strtok(NULL, " ");
			}

		char *newargs3[b];
		newargs3[b] = NULL;
		 x =b;
		for(b = 0; b < x; b++){
			
			newargs3[b] = malloc(strlen(split3[b]) + 1);
			
			strcpy(newargs3[b], split3[b]);
			
			(newargs3[b])[strlen(split3[b])] = '\0';
		}
		//////split line 3////////////////////////////////

	if(sym1 == '|' && sym2 == '|'){
			pid_t cpid[3] = {0};
			int fd[2] = {0};
			int ret = 0;
			
			setbuf(stdout, NULL);

			if(pipe(fd) < 0){
				fprintf(stderr, "pipe() failed: %s\n", strerror(errno));
				return 0;
			}

			cpid[0] = fork();
			if(cpid[0] < 0){
				fprintf(stderr, "fork() 1 failed: %s\n", strerror(errno));
				return 0;
			}
			else if(cpid[0] == 0){//child 1
				printf("");
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
				if(execvp(newargs[0], newargs) < 0){
					fprintf(stderr, "Cannot execute \"%s\": %s\n", newargs[0], strerror(errno));
					exit(-1);
				}
			}
			
			cpid[1] = fork();
			if(cpid[1] < 0){
				fprintf(stderr, "fork() 2 failed: %s\n", strerror(errno));
				return 0;
			}
			else if(cpid[1] == 0){//child 2
				printf("");
				dup2(fd[0], STDIN_FILENO);
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
				if(execvp(newargs2[0], newargs2) < 0){
					fprintf(stderr, "Cannot execute \"%s\": %s\n", newargs2[0], strerror(errno));
					exit(-1);
				}
			}

			cpid[2] = fork();
			if(cpid[2] < 0){
				fprintf(stderr, "fork() 3 failed: %s\n", strerror(errno));
				return 0;
			}
			else if(cpid[2] == 0){//child 3
				printf("");
				dup2(fd[0], STDIN_FILENO);
				close(fd[0]);
				close(fd[1]);
				if(execvp(newargs3[0], newargs3) < 0){
					fprintf(stderr, "Cannot execute \"%s\": %s\n", newargs2[0], strerror(errno));
					exit(-1);
				}
			}

			
			close(fd[0]);
			close(fd[1]);
			while((ret = wait(NULL)) > 0){
				printf("... child process (PID=%d) is waited/reaped\n", ret);
			}	

	}// tripple pipe

	if(sym1 == '<' && sym2 == '>'){
		
			pid_t cpid = 0;

			setbuf(stdout, NULL);
	
			cpid = fork();
			if(cpid < 0){
				fprintf(stderr, "fork failed: %s\n", strerror(errno));
			}else if(cpid == 0){ //child
				//printf("IN CHILD: pid =%d\n", getpid());
				int fd = open(newargs2[0], O_RDONLY, 0);
				int fd2 = open(newargs3[0], O_CREAT | O_RDWR | O_TRUNC);
				dup2(fd, STDIN_FILENO);
				dup2(fd2, 1);
				close(fd);
				close(fd2);				
				execvp(newargs[0], newargs);
				exit(0);
			}else{//parent
				int ret = wait(NULL);

				if(ret < 0){
					fprintf(stderr, "wait failed: %s\n", strerror(errno));
				}else{
					printf("... child process (PID=%d) is waited/reaped\n", cpid);
				}
			}

	

	}// double arrow

}

void my_handler(int sig){
	ctrlc = 1;
	int ret = wait(NULL);
	if(ret >= 0){
		fprintf(stderr, "\n... child process (PID=%i) is waited/reaped\n", ret);
	}else{
		fprintf(stderr, "\nCtrl-C catched. But currently there is no foreground process running.\n", sig);
		
	}
	
}

int main(void){

	signal(SIGINT, my_handler);
	char line[1024];
	
		
	while(1) {

		
		printf("eazyXL_Shell>");
		
		
		if(!fgets(line, 1023, stdin)) {
			break;
		}

		if(strcmp(line, "exit\n") == 0) break;

///////////////////////////////////////gets standard string with normal spaces///////////////////////////////
		int length;
		for(int i = 0; i< strlen(line); i++) {
			if(line[i] == '\n'){
				length = i+1;
				line[i] = '\0';
			}
		}
		char newLine[length];
		for(int i = 0; i < length; i++) {
			newLine[i] = line[i];
		}
		//front space
		int count = 0;
		for(int i = 0; i < length; i++) {
			if(newLine[i] == ' '){
				count++;
			}else{
				break;
			}
		}
		//printf("length %d\ncount %d\n", length, count);
		char newerLine[length - count];
		for(int i = count; i < length; i++) {
			newerLine[i-count] = newLine[i];
		}

		//end spaces
		int count2 = 0;
		for(int i = length - 1; i >= 0; i--) {
			if(newLine[i-1] == ' '){
				count2++;
			}else{
				break;
			}
		}
		//printf("length %d\ncount %d\n", length-count, count2);
		char newernewerLine[length - count - count2];
		for(int i = 0; i < length -1 -count2 - count; i++) {
			newernewerLine[i] = newerLine[i];
		}
		newernewerLine[length -1 -count2 - count] = '\0';
		//does not have trailing or leading white space



		char final[1024];
		evenSpace(newernewerLine, final);
		//printf("%s\n", final); 
///////////////////////////////////////gets standard string with normal spaces///////////////////////////////

///////////////////////////////////////Counts num symbols in string///////////////////////////////
		int symbolCount = 0;
		for(int i = 0; i < 1024; i++){
			if(final[i] == '\0'){
				break;
			}
			if(final[i] == '|' || final[i] == ';' || final[i] == '>' || final[i] == '<' || final[i] == '&'){
				symbolCount++;
			}
		}
		//printf("num symbols %i\n", symbolCount);
///////////////////////////////////////Counts num symbols in string///////////////////////////////

///////////////////////////////////////breaks up string into lines of commands and string of the symbols///////////////////////////////
	char *cmdArray[symbolCount+1][50];
	memset( cmdArray, NULL, symbolCount+1*50*sizeof(char) );
	char symbolArray[symbolCount];
	memset( symbolArray, NULL, symbolCount*sizeof(char) );
	
	int cIndex = 0;
	int cIndexSub = 0;
	int sIndex = 0;
	for(int i = 0; i < 1024; i++){
		if(final[i] == '\0'){
			cmdArray[cIndex][cIndexSub] = '\0';
			if(symbolCount != 0){
			symbolArray[sIndex] = '\0';
			}
			//printf("setting [%i][%i] to NULL ", cIndex, cIndexSub);
			break;
		}
		if(final[i] == '|' || final[i] == ';' || final[i] == '>' || final[i] == '<' || final[i] == '&'){
			cmdArray[cIndex][cIndexSub] = '\0';
			symbolArray[sIndex] = final[i];
			sIndex++;
			cIndex++;
			cIndexSub = 0;
		}else{
			cmdArray[cIndex][cIndexSub] = final[i];
			//printf("setting [%i][%i] to %c ", cIndex, cIndexSub, final[i]);
			cIndexSub++;
		}
		
	}

	for(int i = 0; i < symbolCount+1; i++){
		//printf("cmd line: ");
		for(int k = 0; k < 50; k++){
			if(cmdArray[i][k] == '\0'){
				//printf("printing NULL\n");
				break;
			}
			//printf("%c", cmdArray[i][k]);
		}
	}
	//printf("\n");
	//if(symbolCount != 0){
	//printf("symbbols %s\n", symbolArray);
	//}
///////////////////////////////////////breaks up string into lines of commands and string of the symbols///////////////////////////////
	
	//have int symbolCount
	int numCMDs = symbolCount+1;
	int curCMD = 0;
	int curSym = 0;

////////if no symbol///////////////
	if(symbolCount == 0){
		char temp[50] = {NULL};
		for(int i = 0; i < 50; i++){
			temp[i] = cmdArray[0][i];
		}
		if(ctrlc == 0 && temp[0] != NULL){
		zeroSym(temp); 
		}
	}
	ctrlc = 0;
////////if no symbol///////////////

	if(symbolCount - curSym == 1){
		char temp[50] = {NULL};
		char temp2[50] = {NULL};
		for(int i = 0; i < 50; i++){
			temp[i] = cmdArray[0][i];
			temp2[i] = cmdArray[1][i];
		}
		oneSym(temp, temp2, symbolArray[curSym]);
	}

	if(symbolCount - curSym == 2){
		char temp[50] = {NULL};
		char temp2[50] = {NULL};
		char temp3[50] = {NULL};
		for(int i = 0; i < 50; i++){
			temp[i] = cmdArray[0][i];
			temp2[i] = cmdArray[1][i];
			temp3[i] = cmdArray[2][i];
		}
		twoSym(temp, temp2, temp3, symbolArray[curSym], symbolArray[curSym + 1]);
	}


	}//main
    
    return 0;
}

