#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"

#define BUFFER_SIZE 100 //100 seems like a nice medium number for this

typedef enum commands { //A blessing from God himself
        EXIT,
        WHICH,
        WHERE,
        CD,
        PWD,
        LIST,
        PID,
        KILL,
        PROMPT,
        PRINT_ENV, 
        SET_ENV,
        end_of_list
    } commands;

void ctrlhanlder(int sig){};

int sh( int argc, char **argv, char **envp )
{
  extern char **environ; //new addition because Google said so
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *commandlineinput = calloc(MAX_CANON, sizeof(char));
  char pBuffer[BUFFER_SIZE];
  char *command, *arg, *commandpath, *p, *pwd, *cwd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;
  char *olddir;

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		/* Home directory to start out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  cwd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(cwd, pwd, strlen(pwd));
  prompt[0] = ' '; 
  prompt[1] = '\0';

  olddir = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(olddir, pwd, strlen(pwd));
  prompt[0] = ' '; 
  prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();
  
  char *commands_strings[] = {
      "exit",
      "which",
      "where",
      "cd",
      "pwd",
      "list",
      "pid",
      "kill",
      "prompt",
      "printenv",
      "setenv"
  };

  while ( go )
  {
    printf("\n%s [%s]>", prompt, cwd);
    signal(SIGINT, ctrlhanlder); //ctrl c
    signal(SIGTSTP, ctrlhanlder); //ctrl z
    fgets(commandline, BUFFER_SIZE, stdin);
    int len = (int)strlen(commandline);

    if(len >= 2){                      
      int num_args = 0;               //**
      commandline[len-1] = '\0'; //will never forget to do this again lol
      commandlineinput = (char*)malloc(len);
      strcpy(commandlineinput,commandline);
      int input_length = len;
      int index = 0;
      char *token = strtok(commandlineinput, " ");
      while(token){ //NEED TO ADD SPECIAL CHARACTERS
        //breaks up the input into tokens that are stored in the array and then counts how many total arguments we have
        len = (int) strlen(token);
        args[num_args] = (char *) malloc(len);
        strcpy(args[num_args], token);
        token = strtok(NULL, " ");
        num_args++;
      }
      for(int i = 0; i < end_of_list; i++){ //must be struct? (changed to struct but now I have two?)
        if(strcmp(args[0], commands_strings[i]) == 0){
          switch(i){
            case EXIT: //done
              go = 0;
              break;
            case WHICH: //done
              if(args[1] == NULL){
                printf("%s", "not enough arguments.\n");
              }else{
                for(int i = 1; i < MAXARGS; i++){
                  if(args[i]){
                    char *rval = which(args[i],pathlist);
                    if(rval){
                      printf("%s\n", rval);
                      free(rval);
                    }else{
                      printf("%s is invalid", args[i]);
                    }
                  }else{
                    break;
                  }
                }
              }
              break;
            case WHERE: //done
              if(args[1] == NULL){
                printf("%s", "not enough arguments.\n");
              }else{
                for(int i = 1; i < MAXARGS; i++){
                  if(args[i]){
                    char *rval = where(args[i],pathlist);
                    if(rval){
                      printf("%s\n", rval);
                      free(rval);
                    }else{
                      printf("%s is invalid", args[i]);
                    }
                  }else{
                    break;
                  }
                }
              }
              break;
            case CD:  //done
              //args[0] is the cd command
              //args[1] is the directory
              //can't be more than two args
              //JESUS H CHRIST IT FINALLY WORKS
              if(args[1] == NULL){
                strcpy(olddir, cwd);
                strcpy(cwd, homedir);
                chdir(cwd);
              }else if(num_args > 2){
                printf("Too many arguments\n");
              }else if(strcmp(args[1],"-") == 0){
                char *temp = cwd;
                cwd = olddir;
                olddir = temp;
                chdir(cwd);
              }else if(args[1] != NULL && args[2] == NULL){
                if(chdir(args[1]) < 0){
                  perror("Invalid Directory");
                }else{
                  free(olddir);
                  olddir = malloc((int)strlen(commandlineinput));
                  strcpy(olddir, cwd);
                  free(cwd);
                  cwd = malloc((int)strlen(commandlineinput));
                  strcpy(cwd, commandlineinput);
                }
              }
            case PWD: //done
              printf("%s\n", cwd);
              break;
            case LIST: //done
              if(num_args == 1){
                list(cwd);
                break;
              }else{
                for(int i = 1; i<MAXARGS; i++){
                  if(args[i] != NULL){
                    list(args[i]); 
                  }
                }
                printf("\n");
              }
              break;
            case PID: //done i believe
              int pid = getpid();                                             
              printf("%d\n", pid);
              break;
            case KILL: //done
              if(args[1] == NULL){ //works
                printf("No argument for target\n");
              }else if(args[2] == NULL){ //works
                int temp = -1;
                sscanf(args[1], "%d", &temp);
                if(temp != -1){
                  if(kill(temp, 15) == -1){
                    perror("Error\n");
                  }else{
                    kill(temp,15);
                  }
                }else{
                    printf("Invalid PID\n");
                }
              }else if(args[3] == NULL){ //doesn't work :( (works when you type the pid right smh I'm dumb)
                int temp = -1;
                int signal = 0; //I'm bouta do what's called a pro gamer move
                sscanf(args[2], "%d", &temp); //convert both to ints, args[2] is target
                sscanf(args[1], "%d", &signal); //args[1] is the flag/signal
                if(temp != -1 && signal < 0){ //the signal starts with -(signal) so it's technically a negative int lol
                  if(temp == getpid() && signal == -1){ //killing the whole shell
                    //gotta free everything
                    free(cwd);
                    free(pwd);
                    free(prompt);
                    free(olddir);
                    free(commandline);
                    pathlist = get_path();
                  }
                  if(kill(temp, abs(signal)) == -1){
                    perror("Error\n");
                  }else{
                    kill(temp, abs(signal));
                  }
                }else{
                  printf("Invalid arguments\n");
                }
              }
              break;
            case PROMPT:  //done
              if(num_args == 1){
                printf("Enter Prompt:");
                if(fgets(pBuffer, BUFFER_SIZE, stdin) != NULL){
                  len = (int)strlen(pBuffer);
                  if(pBuffer[len-1] == '\n'){
                    pBuffer[len-1] = 0;
                  }
                  strtok(pBuffer, " "); //in case of multiple words
                  strcpy(prompt, pBuffer);
                }
              }else if(num_args == 2){
                strcpy(prompt, args[1]);
              }
              break;
            case PRINT_ENV: //done I think??
              printenv(envp, num_args, args);
              break;
            case SET_ENV: //also done I think?
              if(num_args == 1){
                printenv(envp, num_args ,args);
              }else if(num_args == 2 && (strcmp(args[1], "PATH") == 0 || strcmp(args[1], "HOME") == 0)){
                printf("DO NOT SET TO EMPTY\n");
              }else if(num_args == 2){
                if(setenv(args[1], "", 1) == -1){
                  perror("Error\n");
                }else{
                setenv(args[1], "", 1);
                }
              }else if(num_args == 3){
                if(setenv(args[1], args[2], 1) == -1){
                  perror("Error\n");
                }else{
                  if(strcmp(args[1], "PATH") == 0){
                    pathlist = get_path();
                  }
                  if(strcmp(args[1], "HOME") == 0){
                    homedir = args[2];
                  }
                }
              }else{
                printf("Too many arguments\n");
              }
              break;
            default:
              status = 0;
              pid_t pid1;
              if((pid1 = fork()) < 0){
                perror("Error.\n");
              }else if(pid1 == 0){
                char *execPath = which(commandlineinput, pathlist);
                if(execPath != NULL){
                  execPath = calloc(BUFFER_SIZE, sizeof(char));
                  strcpy(execPath, commandlineinput);
                }else{
                  execve(execPath, args, environ);
                  free(execPath);
                  printf("Command not found.\n");
                  break;
                }
              }else{
                status = 0;
                waitpid(pid1, &status, 0);
              }
              break;

          }

        }
          /*
          if((commandlineinput[0] == '/') | ((commandlineinput[0] == '.') & (commandlineinput[1] == '/')) | ((commandlineinput[1] == '.') & (commandlineinput[2] == '/'))){
            //absolute path

          }
          if(which(commandlineinput, pathlist) == NULL){
                printf("Command not found.\n");
                break;
          }else{
                char* executePath = which(commandlineinput, pathlist);
                pid_t pid;
                if((pid = fork()) < 0){
                  printf("Error.\n");
                }else if(pid == 0){

                }else{
                  waitpid(pid, NULL, 0);
                }
          }
        */
      }
    }

    /* print your prompt */

    /* get command line and process */

    /* check for each built in command and implement */

     /*  else  program to exec */
    {
       /* find it */
       /* do fork(), execve() and waitpid() */

      /* else */
        /* fprintf(stderr, "%s: Command not found.\n", args[0]); */
    }
    clearerr(stdin); //ctrl d
  }
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
  struct pathelement *currentpath = pathlist;                //the current pathlist
  DIR *givenDir;                                             //the directory we were given
  struct dirent *rawDir;                                     //the variable that holds the pointer resulting from readdir
  char fullpath[BUFFER_SIZE];                                //a large char variable to hold the full path name
  int len;
  strcpy(fullpath, "");                                                  

  while(currentpath){                                         //while the current pathlist still exists (isn't NULL)
    char *currentpathelement = currentpath->element;          //the element associated with the node at the current path
    givenDir = opendir(currentpathelement);                   //open the directory that is associated in the element at the current point in the pathlist
    if(givenDir){                                             //if the given directory at thre current path element is not null
      while((rawDir = readdir(givenDir)) != NULL){            //while loop that executes so long as the pointer returned in readdir is not returned as NULL
        if(strcmp(rawDir->d_name, command) == 0){             //if the entry in the directory matches the command we're looking for,
          strcpy(fullpath, currentpathelement);               //add it to the full path 
          strcat(fullpath, "/");
          strcat(fullpath, rawDir->d_name);
          len = (int) strlen(fullpath);                       //needs to return a char pointer
          char *returnpath = (char*)malloc(len);
          strcpy(returnpath, fullpath);
          closedir(givenDir);                                 //close the directory we're in and returnthe fullpath associated with that command
          return returnpath;                                  
        }
      }
    }
    closedir(givenDir);
    currentpath = currentpath->next;
  }
  return(NULL);
   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */

} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  struct pathelement *currentpath = pathlist;                //the current pathlist
  DIR *givenDir;                                             //the directory we were given
  struct dirent *rawDir;                                     //the variable that holds the pointer resulting from readdir
  char alllocations[BUFFER_SIZE];                            //a large char variable to hold all of the locations of the command
  int len;
  strcpy(alllocations, "");

  while(currentpath){                                        //while the current pathlist still exists (isn't NULL)
      char *currentpathelement = currentpath->element;       //the element associated with the node at the current path
      givenDir = opendir(currentpathelement);                //open the directory that is associated in the element at the current point in the pathlist
      if(givenDir){                                          //if the given directory at the current path element is not null
          while((rawDir = readdir(givenDir)) != NULL){       //while loop that executes so long as the pointer returned in readdir is not returned as NULL
              if(strcmp(rawDir->d_name, command) == 0){      //if the entry in the directory matches the command we're looking for,
                strcat(alllocations, currentpathelement);    // then add it to the alllocations char variable (with formatting)
                strcat(alllocations, "/");
                strcat(alllocations, rawDir->d_name);
                strcat(alllocations, "\n");
              }
          }
      }
      closedir(givenDir);                                    //close the given directory after use
      currentpath = currentpath->next;                       //move to the next node in the pathlist
  }
  len = (int)strlen(alllocations);
  char *returnlocations = (char*)malloc(len);
  strcpy(returnlocations, alllocations);
  return returnlocations;                                      //return all of the locations where command was found

  /* similarly loop through finding all locations of command */
} /* where() */

void list ( char *dir )
{
  DIR *givenDir;                                    //using the built in DIR struct
  givenDir = opendir(dir);                          //givenDir is now the actual dir
  
  struct dirent *rawDir;                            //a pointer that will eventually point to every entry in the directory
  if(givenDir == NULL){
                                                    //do nothing if invalid
  }else{                                            //otherwise directory is viable
    while((rawDir = readdir(givenDir)) != NULL){    //I think this will iterate? might be missing the iterative component here
      printf("%s\n", rawDir->d_name);                       //will print out every entry in the dir
    }
  }
  closedir(givenDir);                               //need to close the directory we opened

  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
} /* list() */

void printenv(char **envp, int num_args, char **args){
    if(num_args == 1){ //in the case of printing all environment varibales
      int index = 0;
      while(envp[index]){
        printf("%s\n", envp[index]);
        index++;
      }
    }else if(num_args == 2){ //in the case of printing a specific variable
      char *env_var = getenv(args[1]);
      if(env_var){
        printf("%s\n", env_var);
      }
    }else{
      printf("Too many arguments");
    }
}

