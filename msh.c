// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017, 2020 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11    // The maximum num of arguments is 11, 1 command and 10 params

#define MAX_HISTORY 15          // The maximum number of history saved  

#define MAX_PIDS 15             // The maximum number of pids saved



// This function prints history of commands entered upto MAX_HISTORY.
// It has parameters "history" array of string that holds individual
// commands entered. Another parameter "count" holds the number of 
// commands entered.
void print_history(char** history, int count)
{
  // This variable "lower_num" stores whether count of
  // entered commands "count" or maximum history array 
  // capacity is lower. The one that is lower is needed
  // to print history. If count is lower than max size,
  // only "count" number of commands are saved. Else,
  // max number of commands are saved.
  int lower_num;
  int i;
  
  if(count > MAX_HISTORY)
  {   
    lower_num = MAX_HISTORY;
  }
  else
  {
    lower_num = count;
  }
   
  for(i = 0; i < lower_num; i++)
  {
    printf("%d: %s", (i+1), history[i]); 
  } 
}

// This function saves history of commands entered upto MAX_HISTORY.
// It has parameters "history" array of string to save individual
// commands entered. "new" is the command entered most recently to
// save. "count" is the total command entered.
void add_history(char** history, char* new, int count)
{ 
  if(count < MAX_HISTORY)
  {
    strcpy(history[count], new);
  }
  else
  { 
    int i;
    for(i = 0; i < (MAX_HISTORY - 1); i++)
    {
      strcpy(history[i], history[i+1]);
    }
    strcpy(history[MAX_HISTORY - 1], new);
  }
}


// This function prints saved pids in the array "pids".
// "pid_count" is total number of process created.
void print_pids(pid_t *pids, int pid_count)
{
  // This variable "lower_num" stores whether count of
  // child process generated "pid count" or max pid array 
  // capacity is lower. The one that is lower is needed
  // to print pids. If count is lower than max size,
  // only "pid_count" number of commands are saved. Else,
  // max number of pids are saved.
  int lower_num;
  int i;
  
  if(pid_count > MAX_PIDS)
  {   
    lower_num = MAX_PIDS;
  }
  else
  { 
    lower_num = pid_count;
  }
   
  for(i = 0; i < lower_num; i++)
  {  
    printf("%d: %d\n", (i+1), pids[i]); 
  } 
}

// This function adds pid of most recently generated child
// process "new_pid" in the array "pids". "pid_count" is 
// the count of total child process created.
void add_pids(pid_t new_pid, pid_t *pids, int pid_count)
{
  if (pid_count < MAX_PIDS)
  {
    pids[pid_count] = new_pid;
  }
  else
  {
    int i;
    for(i = 0; i < (MAX_PIDS - 1); i++)
    {
      pids[i] = pids[i+1];
    }
    pids[MAX_PIDS - 1] = new_pid;
  }
}

int main()
{

  // This boolean will hold whether or not user command indicate to repeat a
  // command in history. Eg. "!5"
  bool repeat = false;

  // This boolean will hold whether or not user command's argument exceeds
  // maximum argument 
  bool max_exceed = false;

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  // This variable will count non-empty commands entered by the user
  int count = 0;
  
  // This variable will count number of child process generated
  int pid_count = 0;  

  // This array will be used to save address of previous commands entered by the user.
  char **history = (char**) malloc ( MAX_HISTORY * sizeof(char*));
  
  // To save each history
  for (int i = 0; i < MAX_HISTORY; i++)
  {
    history[i] = (char*) malloc (MAX_COMMAND_SIZE);
  }
  
  // To save each children pids
  pid_t *pids = (pid_t*) malloc (MAX_PIDS * sizeof(pid_t));  
 
  while( 1 )
  {
    // If the command is to repeat a command in history, there
    // is no need to generate prompt for user input and no need
    // to get command. If it's not a repeat command, input is
    // required to be collected.
    if(!repeat)
    {
      // Print out the msh prompt
      printf ("msh> ");
    
      // Read the command from the commandline.  The
      // maximum command that will be read is MAX_COMMAND_SIZE
      // This while command will wait here until the user
      // inputs something since fgets returns NULL when there
      // is no input

      while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
    }

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS + 1];

    int token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;
    
    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS + 1))
    {

      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        
      if((token_count == (MAX_NUM_ARGUMENTS)) && 
         (argument_ptr = strsep(&working_str, WHITESPACE)) != NULL)
      {
        printf("Command exceeded maximum argument length.\n"); 
        max_exceed = true;
      } 
      token_count++;
    }
    

    // Reset the repeat bool to false
    repeat = false;
   

    // Just reset the maximum exceed boolean if argument limit exceeds
    // or if the command is empty.
    if(token[0] == NULL)
    {

    }
    else if(max_exceed == true)
    {       
      add_history(history, cmd_str, count);
      count++;
      max_exceed = false;
    }
    else if(strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0)
    {
      exit(0);
    } 
    else if(strcmp(token[0], "cd") == 0)
    {
      add_history(history, cmd_str, count);
      count++;
      // if it is just cd, direct home
      if(token_count == 2)
      {
        chdir(getenv("HOME"));
      }
      else
      {
        int ret;
        ret = chdir(token[1]);
        if(ret == -1)
        {
          perror("");
        }
      }
    }
    else if(strcmp(token[0], "history") == 0)
    {  
      add_history(history, cmd_str, count);
      count++;
      print_history(history, count);    
    }

    // for repeating command in history, the command start with
    // "!" and rest of the characters are numbers. The length of
    // the command determines the number of digits the number has.
    // The "cmd_str" variable is assigned the respective command
    // in history and no input is taken for next repitition.
    else if((token[0][0] == '!') && (strlen(token[0]) == 2) && 
             (isdigit(token[0][1])))
    { 
      int rep_num = token[0][1] - '0';
     
      if(rep_num == 0 || rep_num >= count )
      {
        printf("Command not in history.\n"); 
      }
      else
      {
        repeat = true;
        strcpy(cmd_str, history[rep_num - 1]);
        
      }
    } 
    else if((token[0][0] == '!') && (strlen(token[0]) == 3) && 
             (isdigit(token[0][1])) && (isdigit(token[0][2]))) 
    {
      int rep_num = ((token[0][1] - '0') * 10) + (token[0][2] - '0');
      
      if(rep_num > MAX_HISTORY || rep_num > count)
      {
        printf("Command not in history.\n");
      }
      else   
      {
        repeat = true;
        strcpy(cmd_str, history[rep_num - 1]); 
      }
    }
    else if(strcmp(token[0], "showpids") == 0)
    { 
      add_history(history, cmd_str, count);
      count++;
      print_pids(pids, pid_count);
    }
    else
    { 
      add_history(history, cmd_str, count);
      count++;
      pid_t pid = fork();
      if(pid == -1)
      {
        perror("Execution failed.\n");
        exit(EXIT_FAILURE);
      }
      else if(pid == 0)
      {
        int ret;
        ret = execvp(token[0], &token[0]);
        if(ret == -1)
        {
          // This "temp" variable is used to store "cmd_str"
          // without '\n' at the end of the string for 
          // printing purpose.
          char temp[MAX_COMMAND_SIZE];
          
          strcpy(temp, cmd_str);
          temp[strlen(temp) - 1] = '\0'; 

          printf("%s: Command not found.\n", temp);
        }
        fflush(NULL);
        exit(EXIT_SUCCESS);
      }
      else
      {
        int status;
         
        add_pids(pid, pids, pid_count);            
        pid_count++;
  
        waitpid(pid, &status, 0);
        fflush(NULL);
      }
    } 


    free(working_root);
  }

  for(int j = 0; j < MAX_HISTORY; j++)
  {
    free(history[j]);
  } 
  free(cmd_str); 
  free(history);
  free(pids);
  return 0;
}
