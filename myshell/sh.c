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

typedef enum commands {
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

int sh( int argc, char **argv, char **envp )
{

  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *commandlineinput = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *cwd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;
  char *prefix;
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
    printf("%s>", cwd);
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
            case CD:  //**
              //args[0] is the cd command
              //args[1] is the directory
              //can't be more than two args
              char* newdirectory = args[1];
              if(num_args > 2){
                perror("too many arguments\n");
              }else{
                if(num_args == 1){
                  newdirectory = homedir;
                }else if(num_args == 2){
                  newdirectory = args[1];
                }
                if(newdirectory[0] == '-'){
                  if(chdir(olddir) < 0){
                    printf("invalid directory\n");
                  }else{
                    free(cwd);
                    cwd = malloc((int)strlen(olddir));
                    strcpy(cwd,olddir);
                    free(olddir);
                    olddir = malloc((int)strlen(commandlineinput));
                    strcpy(olddir, commandlineinput);
                  }
                }else{
                  if(chdir(newdirectory) < 0){
                    printf("invalid directory\n");
                  }else{
                    free(olddir);
                    olddir = malloc((int)strlen(commandlineinput));
                    strcpy(olddir, commandlineinput);
                    free(cwd);
                    cwd = malloc((int)strlen(commandlineinput));
                    strcpy(cwd, commandlineinput);
                  }
                }
              }
              break;
            case PWD: //done
              printf("%s\n", cwd);
              break;
            case LIST:
              if(num_args == 1){
                list(cwd);
                break;
              }else{
                for(int i = 1; i<MAXARGS; i++){
                  if(args[i] != NULL){
                    list(args[i]); //**
                  }
                }
              }
              break;
            case PID:
              int pid = getpid();                                             //**
              printf("%d\n", pid);
              break;
            case KILL: //dude wtf is this
              if(num_args == 2){
                char *stringPid = args[1];

              }
              break;
            case PROMPT:  //**
              if(num_args == 1){
                fgets(commandlineinput, BUFFER_SIZE, stdin);
                len = (int)strlen(commandlineinput);
                //alter string in some way?
                strcpy(prefix, commandlineinput);
              }else if(num_args == 2){
                strcpy(prefix, args[1]);
              }
              break;
            case PRINT_ENV: //done
              printenv(envp, num_args, args);
              break;
            case SET_ENV:
              if(num_args == 1){
                printenv(envp, num_args ,args);
              }else if(num_args == 2){
                setenv(args[1], "", 1);
              }else if(num_args == 3){
                setenv(args[1], args[2], 1); //**
              }
              break;
            default:
              if(pid == fork()){
                waitpid(pid, &status, 0);
              } else{
                execve(args[0], args, envp);
                fprintf(stderr, "%s: Command not found.\n", args[0]);
                exit(1); //**?
              }
          }

        }
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
    printf("Directory %s is invalid", dir);         //if the opened dir is null, then tell the user
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
    }
}


