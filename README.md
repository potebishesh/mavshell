# Mavshell

### Description
Mavshell is a shell program with following capabilities:
- It supports any command in /bin, /usr/bin/, /usr/local/bin/ and the current working directory such as ls, cd, mkdir, rmdir, and ps. 
- It supports up to 10 command line parameters in addition to the command. 
- It stores last 15 commands entered by the user which can be listed using the command "*history*".
- It stores the PIDs of the last 15 processes spawned by your shell which can be listed using the command "*showpids*".
- It exits with status zero if the command is “*quit*” or “*exit*”.

### Instructions
- Compilation: gcc -Wall msh.c -o msh —std=c99
- Execution: ./msh
