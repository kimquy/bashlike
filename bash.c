// Long Nguyen
// Spring 2012
// Build a bash-liked shell
// New features added: aliasing, history, and pipe
// list of new commands: alias, unalias, history, and |

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

struct node{
	char key[10];
	char command[100];
	struct node *next;
	struct node *parent;
	struct node *last;
	int count;
};

typedef struct node* nodeptr;


void add_node(nodeptr n, char k[], char cmd[]){
	n->count = n->count + 1;
	if(n==NULL){
		strcpy(n->key,k);
		strcpy(n->command, cmd);
		n->next = NULL;
		n->parent = NULL;
	}
	else{
		nodeptr temp;
		temp = malloc(sizeof(struct node));

		strcpy(temp->key,k);
		strcpy(temp->command, cmd);

		if(n->count == 1 ){
			n->next = temp;
			temp->next = NULL;
			temp->parent = n;
			n->last = temp;
		}
		else{
			n->next->parent = temp;
			temp->next = n->next;
			temp->parent = n;
			n->next = temp;
		}

	}
}

void delete_node(nodeptr n, char k[]){
	nodeptr temp;
	temp = n;
	
	while(temp != NULL){

		if(strcmp(temp->key, k) == 0){
			if(n->count == 1){
				n = NULL;
				return;
			}
			nodeptr temp1;
			temp1 = temp->parent; 
			temp1->next = temp->next;
			free(temp);
		}
		temp = temp->next;
	}
}



// break the command into arguments then storing it 
voids store_arg(char* arg[], char command[]){
   int i=0;
   char *temp;
   int count = 0;
   temp = strtok(command," :1234567890'=");
   while(temp != NULL){
     arg[i] = temp;
     temp = strtok(NULL," :1234567890'=");
     i++;
   }
   arg[i] = NULL;	
}

// read command from standard input
void read_cmd_from_file(char cmd[], FILE *f){
	char c = 's';
	int i=0;
	while(c != '\n' && !feof(f)){
		c = fgetc(f);
		cmd[i] = c;
		i++;
	}
	cmd[i-1] = '\0';
	
}

// execute the command
int exec_cmd(char* arg[], char command[]){
	int status;	
  	pid_t child;
 	child = fork();


 	if(child != 0){
  		wait(&status); 
	}
 	else{
 		if (execvp(arg[0],arg) == -1){
 			
 			//cd command
 			if(strcmp(arg[0], "cd") ==0){
 				if(arg[1] == NULL){
 					printf("There is no directory.\n");
 					return;	
 				}
 				else{
 					chdir(arg[1]);
 					return;
 				}
 			}
 			
 		 	printf("Unknown command.\n");
 		 	exit(1);
 		}
 		else{

			execvp(arg[0], arg);
			EXIT_SUCCESS;
		}
	}
}

// empty argument list
void empty(char *arg[]){
	int i=0;
	while(i<30){
		arg[i] = NULL;
		i++;
	}
}

void cmd_in_alias(char str[], char cmd[]){
	
	int i;
	int first;
	char c;
	for(i=0; i<100; i++){
		if(str[i] == '\''){
			first = i+1;
			break;
		}
	}

	c = str[first];
	i=0;
	while(c!= '\''){
		cmd[i] = str[first];
		i++;
		first++;
		c = str[first];
	}

}

int find_cmd_in_alias_list(nodeptr n, char key[]){
	nodeptr temp;
	temp = n;
	while(temp != NULL){
		if(strcmp(temp->key, key) == 0){
			char *arg_list[30];
			store_arg(arg_list, temp->command);
			exec_cmd(arg_list, key);
			return 1;
		}
		temp = temp->next;
	}
	return 0;
}

void unalias(nodeptr n, char key[]){
	delete_node(n, key);
}

void exec_by_num_history(char cmd[], char *arg_list[], FILE *f, FILE *h){
	int counter = history_counter();
	char str[100];
	int i=0;
	int j=1;
	int num;
	while(j<strlen(cmd)){
		str[i] = cmd[j];
		j++;
		i++;
	}
	num = atoi(str);

	i=1;
	char c;
	char command[200];
	fseek(h,0,SEEK_SET);
	if(num > counter){
		printf("bash: !%d: event not found\n", num);
		return;
	}
		
	while(i<=counter){
		if(i == num){
			read_cmd_from_file(command, h);
			store_arg(arg_list, command);
			exec_cmd(arg_list,command);
			break;
		}
		while(1){
			c = getc(h);
			if(c=='\n'){
				i++;
				break;
			}
		}
	}
	
	
}

void exec_path_cmd(FILE *f, char *arg_list[]){
	if(strlen(arg_list[2]) == 0){
		return;
	}else{
		fseek(f,0,SEEK_END);
		fputc(':',f);
		fprintf(f, "%s", arg_list[2]);
	}
	
}


int history_counter(){
	FILE *f= fopen(".nsh_history", "r");
	char c = getc(f);
	int x=1;
	while(!feof(f)){
		if(c=='\n'){
			x++;
		}
		c =getc(f);
	}
	fclose(f);
	return x;
}

int temp_counter(){
	FILE *f= fopen("temporary.txt", "r");
	char c = getc(f);
	int x=1;
	while(!feof(f)){
		if(c=='\n'){
			x++;
		}
		c =getc(f);
	}
	fclose(f);
	return x;
}

//store the successfull command to history and write to .nsh_history file
void store_cmd_to_history(char cmd[], FILE *ff){
	int counter = history_counter();
	int history_limit;

	FILE *nhistory = fopen(".login","r");
	fscanf(nhistory, "%d", &history_limit);
	fclose(nhistory);

	if(counter < history_limit){
		FILE *f = fopen(".nsh_history", "a");
		fprintf(f, "%s\n", cmd);
		fclose(f);
	}
	else{
		FILE *f = fopen(".nsh_history","r");
		FILE *temp = fopen("temporary.txt","w+");

		char str[200];

		fgets(str,200,f);
		int i=1;
		int counter = history_counter();
		while(i<=counter){
			fgets(str,200,f);
			fprintf(temp, "%s", str);
			i++;
		}
		fclose(temp);

		FILE *t = fopen("temporary.txt","r");
		f = fopen(".nsh_history","w+");
		
		i = 1;
		counter = temp_counter();
		while(i<=counter){
			fgets(str,200,t);
			fprintf(f, "%s", str);
			i++;
		}
		fprintf(f, "\n");
		fprintf(f, "%s\n", cmd);

		remove("temporary.txt");
		fclose(f);
	}
}

void set_counter_history(FILE *f, int x){
	fprintf(f, "%d\n", x);
}


void print_history(FILE *ff){
	FILE *f = fopen(".nsh_history", "r");
	char str[200];
	
	int counter = history_counter();
	int x = 1;

	while(x < counter){
		fscanf(f,"%[^\n]%*c", str);
		printf(" %d %s\n",x,str);
		x++;
	}
	fclose(f);
}

void exec_last_cmd(FILE *l, FILE *h, char *arg_list[]){

	int counter = history_counter();
	char c;
	int i=1;
	fseek(h,0,SEEK_SET);
	char cmd[200];
	while(i<=counter){
		if(i==(counter-1)){
			read_cmd_from_file(cmd, h);
			store_arg(arg_list, cmd);
			exec_cmd(arg_list,cmd);
		}
		read_cmd_from_file(cmd,h);
		i++;
	}
}

void pipe_cmd(char str[]){
	int fd[2];
	pipe(fd);
	char cmd1[100];
	char cmd2[100];
	char *arg_list1[10];
	char *arg_list2[10];

	int i=0;
	while(str[i] != '|'){
		cmd1[i] = str[i];
		i++;
	}
	i++;
	store_arg(arg_list1,cmd1);

	int j=0;
	while(i < strlen(str)){
		cmd2[j] = str[i];
		j++;
		i++;
	}
	store_arg(arg_list2,cmd2);
	int status;

	pid_t pid = fork();

	if(pid!=0){
		wait(&status);
	}else{
		if (!fork()) {
		close(1);      
		dup(fd[1]); 
		close(fd[0]); 
		execvp(arg_list1[0], arg_list1);
		exit(EXIT_SUCCESS);
	} else {
		close(0);      
		dup(fd[0]);   
		close(fd[1]); 
		execvp(arg_list2[0], arg_list2);
		exit(EXIT_SUCCESS);
	}
	}

}

int find_pipe(char str[]){
	int i=0;
	while(i<strlen(str)){
		if(str[i] == '|'){
			return 1;
		}
		i++;
	}
	return 0;
}

int main(){
	char command[1000];
	char *arg_list[30];
	char alias_cmd[1000];
	char temp[1000];

	nodeptr alias_list;
	alias_list = malloc(sizeof(struct node));
	
	FILE *input;
	input = fopen(".nshsrc", "r");
	if(!input){
		printf("Failed to open input file\n");
		return;
	}

	FILE* bash;
	bash = fopen(".bash_profile","w+");
	if(!bash){
		printf("Failed to open path file\n");
		return;
	}

	FILE* history_output = fopen(".nsh_history", "r");
	if(!history_output){
		history_output = fopen(".nsh_history", "w+");
	}
	fclose(history_output);

	FILE *login = fopen(".login","w+");
	if(!input){
		printf("Failed to open .login\n");
		return;
	}
	set_counter_history(login,20);
	fclose(login);
	
	while(!feof(input)){
		read_cmd_from_file(command, input);
		printf("?: ");

		strcpy(temp,command);
		
		if(strcmp(command, "exit") == 0){
			store_cmd_to_history(temp, history_output);
			break;
		}
			
		printf("%s",command);
		printf("\n");

		store_arg(arg_list, command);

		if(strcmp(arg_list[0], "alias") == 0){
			cmd_in_alias(temp, alias_cmd);
			add_node(alias_list, arg_list[1], alias_cmd);
			store_cmd_to_history(temp, history_output);
		}
		else if(strcmp(arg_list[0], "unalias") == 0){
			unalias(alias_list, arg_list[1]);
			store_cmd_to_history(temp, history_output);

		}
		else if(strcmp(arg_list[0], "!!") == 0){
			empty(arg_list);
			exec_last_cmd(login,history_output,arg_list);
			store_cmd_to_history(temp, history_output);
		}
		else if(strcmp(arg_list[0], "!") == 0){
			empty(arg_list);
			exec_by_num_history(temp, arg_list, login, history_output);
			store_cmd_to_history(temp, history_output);
		}
		else if(strcmp(arg_list[0], "history") == 0){
			store_cmd_to_history(temp, history_output);
			print_history(history_output);

		}
		else if(strcmp(arg_list[0], "$PATH") == 0){
			printf("bash: ");
			char str[1000]; 
			fseek(bash, 1, SEEK_SET);
			fgets(str,1000,bash);
			printf("%s",str);
			printf("\n");
			store_cmd_to_history(temp, history_output);
		}
		else if(strcmp(arg_list[0], "PATH") == 0 && strcmp(arg_list[1], "$PATH") == 0){
			exec_path_cmd(bash,arg_list);
			store_cmd_to_history(temp, history_output);
			
		}
		else if(find_pipe(temp) == 1 ){
			pipe_cmd(temp);
			store_cmd_to_history(temp, history_output);
			
		}
		else{
			if(find_cmd_in_alias_list(alias_list, temp) ==1){
				continue;
			}
			store_cmd_to_history(temp, history_output);
			exec_cmd(arg_list, temp);
			empty(arg_list);
		}

	}

	return 0;
}
