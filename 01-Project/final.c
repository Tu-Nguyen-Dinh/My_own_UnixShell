// Using c/c++ language on linux to build a basic UNIX shell
#include <stdio.h>  // for standard input output
#include <string.h> // string function
#include <stdlib.h>
#include <unistd.h> // for fork
#include <sys/types.h>
#include <sys/wait.h>          // for waitpid()
#include <readline/readline.h> // readline lib uses to control input ouput
#include <readline/history.h>
#include <stdbool.h>
#define clear() printf("\033[H\033[J") 
// Greeting shell during startup 
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
int takeInput(char *input)
{
    char* buf;
    buf = readline("\nTu'shell >> ");
    if(strlen(buf) != 0)
    {
        add_history(buf);
        strcpy(input,buf);
        return 0;
    }else return 1;
}
bool checkDouble(char *input, char **str)
{   int i;

    for ( i = 0; i < 2; i++)
    {   
        str[i] = strsep(&input, "|");
        if (str[i] == NULL)
        {
            break;
        }
    }
    if (str[1] == NULL)
        return false;
    else
        return true;
}
void processSpace(char *command, char **str)
{
    int i = 0;
    char *token = strtok(command, " ");
    while (token != NULL)
    {
        str[i] = token;
        i++;
        token = strtok(NULL, " ");

    }
}
int processInput(char *input, char **command1, char **command2)
{   
    char *str[2];
    
    int tmp = checkDouble(input, str);
    
    
    if (tmp)
    {
        processSpace(str[0], command1);
        processSpace(str[1], command2);      
    }else
    {
        processSpace(str[0], command1);

    }


    if (strcmp(command1[0], "cd")  == 0) 
    {
        chdir(command1[1]);
        return 0;
    }else return tmp+1;
}

void exeSingleComand(char *command[])
{
    /*with the sigle command we just need cread another program to
    solve this command by creating a fork and execvp*/
    pid_t pid; 
    pid = fork();

    if (pid == -1)
    {
        printf("ERROR when creating the fork");

        return;
    }
    else if (pid == 0)
    {
        if (execvp(command[0], command) < 0)  // the end of value of execvp function must be NULL , so this is why i set up all val is null
        {
            printf("ERROR: can not run this command\n");
        }
        exit(0);
    }
    else
    {
        int status;

        waitpid(pid, &status, 0);

        return;
    }
}

/*to solve the double command , spawn two process. the first has its STDout forwarded to the second child's STDout*/
void exeDoubleComand(char **command1,char **command2)
{
    /* need spawn two child process to execute each command.
    the first of which to running the program that is needed to redirect it's STDout
    to the written pipe using dup2(int oldfd, int newfd);
    At the second child process , it is quite similar , execept we will take in the input to the readed fd's pipe
    so we also using dup2() to change the direct STDIN of the second program runned in the second process.
    */
    int fds[2];

    pipe(fds); 
    pid_t child[2];
    child[0] = fork();
    
    if (child[0] == -1)
    {
        printf("can not fork the process!\n");
        return;
    }
    if (child[0] == 0)
    { // the first child 's STDOUT should be written end of the pipe
        close(fds[0]); 
        dup2(fds[1], STDOUT_FILENO);
        execvp(command1[0], command1);
        
    }
    close(fds[1]);
    child[1] = fork();
    if (child[1] == -1)
    {
        printf("can not fork the process!\n");
        return;
    }
    if (child[1] == 0)
    { // the second child's STDIN should be read end of the pipe
        close(fds[1]);
        dup2(fds[0], STDIN_FILENO);
        execvp(command2[0], command2);
       
    }
    close(fds[0]);
    waitpid(child[0], NULL, 0);

    waitpid(child[1], NULL, 0);
}
void printCurDir()
{
    char dir[100];
    getcwd(dir,sizeof(dir));
    printf("Current directory is : %s\n", dir);
}
int main()
{   char input[200]; // should precreate size of input ,because the requiment of strcpy();
    char *sigleArg[100];
    char *doubleArg[100];
    init_shell(); 
    
   while(1)
   {
    int i = 0;
    for(; i < 100 ; i++)
    {
        sigleArg[i] = NULL;
        doubleArg[i] = NULL;
    }
    printCurDir();
    if(takeInput(input))
    {
        continue;
    }
    
    int flag = processInput(input, sigleArg, doubleArg);
    if(strcmp(input, "exit") == 0)
    {
        printf("GOOD BYE MY OWN SHELL!\n");
        break;
    }
    if(flag == 1)
    {  
        exeSingleComand(sigleArg);
    }else if(flag == 2 )
    {
        exeDoubleComand(sigleArg, doubleArg);
    }
   }
}