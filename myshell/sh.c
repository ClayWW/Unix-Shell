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
  char *command, *arg, *commandpath, *p, *pwd, *cwd, *owd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		/* Home directory to start
						  out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  cwd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';

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
    if(len > 0){
      commandline[len-1] = '\0'; //will never forget to do this again lol
      commandlineinput = (char*)malloc(len);
      strcpy(commandlineinput,commandline);
      int input_length = len;
      int index = 0;
      char *token = strtok(commandlineinput, " ");
      while(token != NULL){
        args[index] = token;
        token = strtok(NULL, " ");
      }

      for(int i = 0; i < end_of_list; i++){ //must be struct? (changed to struct but now I have two?)
        if(strcmp(args[0], commands_strings[i]) == 0){
          switch(i){
            case EXIT:
              go = 0;
              break;
            default:
              if(NULL){
                //add execution here
                break;
              } else{
                fprintf(stderr, "%s: Command not found.\n", args[0]);
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

  while(currentpath){
    char *currentpathelement = currentpath->element;
    givenDir = opendir(currentpathelement);
    if(givenDir){
      while((rawDir = readdir(givenDir)) != NULL){
        if(strcmp(rawDir->d_name, command) == 0){
          strcpy(fullpath, currentpathelement);
          strcat(fullpath, "/");
          strcat(fullpath, rawDir->d_name);
          closedir(givenDir);
          return fullpath;                                    //**
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
                                                            //**
  return alllocations;                                      //return all of the locations where command was found

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
      printf("%s\n", rawDir);                       //will print out every entry in the dir
    }
  }
  closedir(givenDir);                               //need to close the directory we opened

  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
} /* list() */

