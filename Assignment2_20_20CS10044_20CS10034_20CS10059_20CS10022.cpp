#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <glob.h>
#include <time.h>
#include <bits/stdc++.h>

#define HIST "my_shell_history.txt"
#define LIMIT 1000
#define MAX_COMMAND_LENGTH 1000
#define MAX_CHAR 100
#define MAX_LEN 1024
#define clear() printf("\033[H\033[J")
#define MAX_FILENAME_LENGTH 100
#define MAX_PROCESSES 100
#define BSIZE 128

struct proc{
	int pid;
	int ppid;
	double tm;
	int n;
};

using namespace std;

void kill_process(pid_t pid) {
  if (kill(pid, SIGKILL) == -1) {
    perror("kill");
    exit(EXIT_FAILURE);
  }
}

void print_pids_with_file_open(char *filename,int **pid_arr,int * i) {
    *pid_arr = (int *) malloc (sizeof(int)*100);
    DIR *dirp;
    struct dirent *entry;
    char path[1024];
    char exe[1024];

    dirp = opendir("/proc");
    if (dirp == NULL) {
        perror("Failed to open /proc");
        return;
    }
    *i=0;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type != DT_DIR)
            continue;

        int pid = atoi(entry->d_name);
        if (pid <= 0)
            continue;

        sprintf(path, "/proc/%d/fd", pid);
        DIR *fd_dirp = opendir(path);
        if (fd_dirp == NULL)
            continue;

        while ((entry = readdir(fd_dirp)) != NULL) {
            sprintf(path, "/proc/%d/fd/%s", pid, entry->d_name);
            char link[1024];
            ssize_t len = readlink(path, link, sizeof(link));
            if (len == -1)
                continue;
            link[len] = '\0';

            if (strcmp(link, filename) == 0) 
             {
                sprintf(exe, "/proc/%d/exe", pid);
                char exe_link[1024];
                ssize_t exe_len = readlink(exe, exe_link, sizeof(exe_link));
                if (exe_len == -1)
                    continue;
                exe_link[exe_len] = '\0';
                //printf("1 %d\n", pid);
                //printf("%d", pid);
                (*pid_arr)[(*i)++]=pid;
                //printf("2 %d\n", pid);
                //printf("3 %d\n",(*pid_arr)[i-1]);
                break;
            }
        }
        closedir(fd_dirp);
    }

    closedir(dirp);
}

void lock(char *filepath)
{
	int PID_ARR[2];
    int *pid_arr;
    int size;
    char buf[MAX_LEN];
    if(access(filepath,F_OK)==-1)
    {
        printf("file does not exist\n");
        return;
    }

    if(pipe(PID_ARR)==-1)
    {
        perror("pipe creation failed.\n");
        return;
    }
    int x = fork();
    int status;
    if(x==0)
    {
        //child process
        close(PID_ARR[0]);
        printf("PID's of processes using or locked the file :\n");
        print_pids_with_file_open(filepath,&pid_arr,&size);
        for(int i=0;i<size;i++)
        printf("%d\n",pid_arr[i]);
        write(PID_ARR[1],&size,sizeof(size));
        write(PID_ARR[1],pid_arr,sizeof(pid_arr));
        close(PID_ARR[1]);
        exit(0);
    }
    else if(x>0)
    {
        //parent process
        waitpid(x, &status, 0);
        close(PID_ARR[1]);
        int read_count=0;
        read(PID_ARR[0],&read_count,sizeof(read_count));
        int read_pidarr[read_count];
        read(PID_ARR[0],read_pidarr,read_count*sizeof(int));
        char response[10];
        int flag = 1;
        if(read_count==0) printf("file is not locked or being used.\n");
        for(int i=0;i<read_count;i++)
        {
            printf("Kill the process (%d)??[yes/no]",read_pidarr[i]);
            scanf("%s",response);
            fflush(stdin);
            if(strcmp(response,"yes")==0) kill_process(read_pidarr[i]);
            else 
                {
                    flag = 0;
                    break;
                }
        }
        if(flag)
        {
            unlink(filepath);
            printf("file (%s) deleted..\n",filepath);
        }
        else
        {
            printf("can't delete file\n");
        }
        close(PID_ARR[1]);
        
        fflush(stdout);
    }
    else
    {
        perror("fork error :");
    }
    return;
}


int curr;
pid_t pid;
static pid_t fgpid = 0;

class History{
   public:
         int pos=0;
         deque<string>commands;
    History(){
       ifstream file(HIST);
       if(file.is_open()){
          string s;
          while(getline(file,s))
          commands.push_back(s);
          file.close();
          commands.push_front("");
       }
       else{
         ofstream file(HIST);
         file.close();
         commands.push_front("");
       }
    }
    string getCommand(int key_up){
        if(key_up){
          if(pos<(int)commands.size()-1&&pos<=LIMIT)pos++;
          return commands[pos];
        }
        else{
          if(pos>0)pos--;
          return commands[pos];
        }
       
    }
    void addToHis(string cmd){
          commands.insert(commands.begin()+1,cmd);
           while(commands.size()>LIMIT)
                commands.pop_back();
    }

    ~History(){
          ofstream ofs;
          ofs.open(HIST,ofstream::trunc|ofstream::out);

          for(int i=1;i<(int)commands.size()-1;i++)ofs<<commands[i]<<'\n';
          if(commands.size())ofs<<commands.back();
              ofs.close();
    }
    
};

History H;

int UP(int count,int key){
  if(curr){
    if(strlen(rl_line_buffer)>0&&H.pos==0){H.commands[0]=string(rl_line_buffer);curr=0;}
  }
  rl_replace_line(H.getCommand(1).c_str(),0);
  rl_end_of_line(0,0);
  rl_redisplay();
  return 0;
}
int DOWN(int count,int key){
  if(curr){
    if(strlen(rl_line_buffer)>0&&H.pos==0){H.commands[0]=string(rl_line_buffer);curr=0;}
  }
  rl_replace_line(H.getCommand(0).c_str(),0);
  rl_end_of_line(0,0);
  rl_redisplay();
  return 0;
}
int CTRL_A(int x,int y){
  
  rl_beg_of_line(0, 0);
  rl_redisplay();
  return 0;
}
int CTRL_E(int x,int y){
  rl_end_of_line(0,0);
  rl_redisplay();
  return 0;
}


int get_ppid(int pid){

	char stat[64];
	sprintf(stat, "/proc/%d/stat", pid);

	int fd = open(stat, O_RDONLY);
	if (fd == -1) {
		printf("Error: Failed to open %s\n", stat);
		return 0;
	}

	char buf[BSIZE];
	memset(buf, 0, BSIZE);
	int read_size = read(fd, buf, BSIZE);
	close(fd);

	if (read_size <= 0) {
		printf("Error: Failed to read from %s\n", stat);
		return 0;
	}

	int ppid=0;
	sscanf(buf, "%*d %*s %*c %d", &ppid);

	return ppid;

}


int find_malware(int pid, int suggest, struct proc *processes, int num_processes) {
  
  	int malware = -1;
  
  
  	for (int i = num_processes-1; i>=0; i--){
      
      	    if (suggest){
      
		char stat[64];
		sprintf(stat, "/proc/%d/stat", processes[i].pid);

		int fd = open(stat, O_RDONLY);
		if (fd == -1) {
		printf("Error: Failed to open %s\n", stat);
		continue;
		}

		char buf[BSIZE];
		memset(buf, 0, BSIZE);
		int read_size = read(fd, buf, BSIZE);
		close(fd);

		if (read_size <= 0) {
		printf("Error: Failed to read from %s\n", stat);
		continue;
		}

		unsigned long utime, stime;
		sscanf(buf, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime);
      
        
		clock_t ticks = sysconf(_SC_CLK_TCK);
		processes[i].tm = (utime + stime) / (double)ticks ;
		
		
		
		processes[i].n = 0;
		
		sprintf(stat, "/proc/%d/task", processes[i].pid);
		DIR *dr = opendir(stat);
		if(dr==NULL){ perror("Error opening the directory"); return 0;}
		
		struct dirent *entry;
		while((entry = readdir(dr)) != NULL){
			if(entry->d_type == DT_DIR && entry->d_name[0]>='0' && entry->d_name[0]<='9') processes[i].n++;
		}
		
		closedir(dr);
		
		 
		if(processes[i].tm>60 || processes[i].n>10){
			malware = processes[i].pid;
			break;
		}
	
		
	    }
  
  	}
  	
  	return malware;
}

int squashbug(int argc, char* argv[])
{
     if (argc < 2) {
	    printf("Correct Usage: %s pid [-suggest]\n", argv[0]);
	    return 1;
	  }
	  
      cout << argv[1]<< endl;
	  int pid = atoi(argv[1]);
	  int sgst = 0;
	  
	  if (argc>=3 && strcmp(argv[2], "-suggest") == 0) sgst=1;
	  
	  struct proc processes[MAX_PROCESSES];
	  int num = 0;
	  

	  while (1){
	    cout<< num<< endl;
	    processes[num].pid = pid;
	    processes[num].ppid = get_ppid(pid);
	    
	    if (processes[num].ppid == 0) break;
	    
	    pid = processes[num].ppid;
	    num++;
	  }
	  
	  int malware_pid = find_malware(pid, sgst, processes, num);
	  
	  for (int i = num; i >= 0; i--) {
	    
	    printf("Pid:- %d,    PPid:- %d, Time:- %lf, No. of children:- %d", processes[i].pid, processes[i].ppid, processes[i].tm, processes[i].n);
	    
	    if(processes[i].pid == malware_pid) printf("   -------   The Root of all the Trouble");
	    
	    printf("\n");
	  }
	  return 0;
}

// Function to print Current Directory.
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\n%s", cwd);
}

void interrupt(int sig)
{
    if(pid!=0)
    {
    	printf("\nInterrupted.\n");
    	fflush(stdout);
    }
}

void interrupt1(int sig)
{
    printf("\nInterrupted.\n");
    printDir();
   	printf("$ \n>>> ");
    fflush(stdout);
}

void background(int sig)
{
	if(pid ==0 ) {
       
        printf("\nChild process in bg.....\n");
        signal(SIGTSTP, SIG_DFL);
        raise(SIGTSTP);
       
    }
}


void init_shell()
{
    clear();
    printf("\n\n\n\n******************"
        "************************");
    printf("\n\n\n\t****MY SHELL****");
    printf("\n\n\t-USE AT YOUR OWN RISK-");
    printf("\n\n\n\n*******************"
        "***********************");
    char* username = getenv("USER");
    printf("\n\n\nUSER is: @%s", username);
    printf("\n");
    sleep(1);
    clear();
}
  
// Function to take input
int takeInput(char* str)
{
    signal(SIGINT, interrupt1);
    string prompt(getcwd(NULL,1024));
   	prompt.append("$ \n>>> ");
    curr=1;
    H.commands[0]="";
    char *buf=readline(prompt.c_str());
    if(strcmp(buf,"") != 0) H.addToHis(string(buf));
    else return 1;
    strcpy(str, buf);
    
    free(buf);
    return 0;
}
  


void openHelp()
{
    puts("\n***WELCOME TO MY SHELL HELP***"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
        "\n>all other general commands available in UNIX shell"
        "\n>pipe handling"
        "\n>improper space handling");
  
    return;
}
  
// Function to execute builtin commands
int ownCmdHandler(int argc, char** parsed)
{
	signal(SIGINT, interrupt);
    int NoCmds = 5, OwnArg = 0;
    string List[NoCmds];
  
    List[0] = "exit";
    List[1] = "cd";
    List[2] = "delep";
    List[3] = "help";
    List[4] = "sb";
  
    for (int i = 0; i < NoCmds; i++) {
        if (strcmp(parsed[0], (char *)List[i].c_str()) == 0) {
            OwnArg = i + 1;
            break;
        }
    }
  
    switch (OwnArg) {
    case 1:
        printf("\nGoodbye\n");
        exit(0);
    case 2:
        chdir(parsed[1]);
        return 1;
    case 3:
        lock(parsed[1]);
        return 1;
    case 4:
        openHelp();
        return 1;
    case 5:
        squashbug(argc, parsed);
        return 1;
    default:
        break;
    }
  
    return 0;
}

void implement(string command, int fdi, int fdo)
{
	vector<string> tokens;
	
    char *token = strtok((char *)command.c_str(), " ");
    while (token != NULL) {
    	glob_t result;
  		int ret = glob(token, GLOB_NOCHECK, NULL, &result);
  		if (ret != 0) {
    		printf("Error: Failed to handle wildcard\n");
    		//exit(1);
    		
  		}
  		else
  		{
  			for (int i = 0; i < result.gl_pathc; i++) {
    			tokens.push_back(result.gl_pathv[i]);
  			}
  		}
  		globfree(&result);
    	token = strtok(NULL, " ");
    }
    int in=0, out=0, wc=0, flag=1, status=0, bg = 0;
    string in_file, out_file;
    
    
    
    for(int i=0; i<tokens.size(); i++)
    {
    	if(tokens[i] == "<") in=1, in_file=tokens[i+1];
    	if(tokens[i] == ">") out=1, out_file=tokens[i+1];
    	if(tokens[i] == "&") bg=1;
    	
    	if(!in && !out && !bg) wc++;
    }
    
    const char **argv = new const char *[wc+1];
    
    for (int i = 0; i < wc; i++)
    {
        argv[i] = tokens[i].c_str();
        //cout<< tokens[i].c_str()<< endl;
    }
    argv[wc] = NULL;
    if(ownCmdHandler(tokens.size(), (char **)argv)) return;
    signal(SIGINT, interrupt);
    flag = !bg;
    pid = fork();
    if(pid == 0)
    {
    	
    	if(out) fdo = open(out_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
    	if(fdo!=1) {
            dup2(fdo, 1);
        }
            
        if(in) fdi = open(in_file.c_str(), O_RDONLY);           
        if(fdi!=0) {
            dup2(fdi, 0);
        }
        
        
        ///printf("ch pid : %d\n", getpid());
        if(execvp(tokens[0].c_str(), (char **) argv)<0)
        {
        	printf("\nCould not execute command..\n");
        }
        
        exit(0);
        
    }
    else
    {
    	//printf("p pid : %d, flag=%d\n", pid, flag);
    	if(flag)
    	{
    		waitpid(pid,&status,WUNTRACED);
    		if(fdi!=0) close(fdi);
    		if(fdo!=1) close(fdo);
    	}
    	else
    	{
    		waitpid(-1, NULL, WNOHANG);
    		cout<< "Running in background\n";
    	}
    }
    
    return;
}

int main(int argc, char* argv[])
{
    rl_bind_keyseq("\\e[A",UP);
   	rl_bind_keyseq("\033[B",DOWN);
   	rl_bind_key(1,CTRL_A);
   	rl_bind_key(5,CTRL_E);
    
    signal(SIGINT, interrupt);
    signal(SIGTSTP, background);
    
    
    while (1)
    {
    	pid = -1;
    	int i = 0;
    	vector<string> commands;
    	/*printDir();
    	printf("$ ");*/
        // take input
        char buf[MAX_LEN];
        if (takeInput(buf))
        {
            continue;
        }
        
        
        char *token = strtok(buf, "|");
    	while (token != NULL) {
        	commands.push_back(token);
        	token = strtok(NULL, "|");
    	}
        
        
        int no_pipes = commands.size()-1;
        int **pipes = new int *[no_pipes];
        for(int j=0;j<no_pipes;j++){
                pipes[j]=new int[2];
                pipe(pipes[j]);
        }
        
        
        for(int j=0; j<commands.size(); j++)
        {
        	
        	int fdi = 0, fdo = 1;
        	if(j>0) fdi = pipes[j-1][0];
        	if(j<commands.size()-1) fdo = pipes[j][1];
       
        	implement(commands[j], fdi, fdo);
        	//cout<< j<< " "<< commands[j]<< endl;
        }
        
        fflush(stdin);
       	fflush(stdout);
        
    }

    return 0;
}
 
