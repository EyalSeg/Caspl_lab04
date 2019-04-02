#include "util.h"
#include <fcntl.h>

#define SYS_OPEN 0x05
#define SYS_CLOSE 0x06

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define true 1
#define false 0

int isDebug = false;

void printDebug(char*str, int len)
{
  system_call(SYS_WRITE, STDERR, str, len);
}

void exit(int code)
{
  system_call(SYS_EXIT, code);
}

int systemcall_wrapper(int callname, int argb, int argc, int argd)
{
  int ret = system_call(callname, argb, argc, argd);

  if (isDebug == true)
  {
    char *buffer = "SYSTEM CALL: 0x";
    printDebug(buffer, strlen(buffer));

    buffer = itoa(callname);
    printDebug(buffer, strlen(buffer));

    buffer = " return call: 0x";
    printDebug(buffer, strlen(buffer));

    buffer = itoa(ret);
    printDebug(buffer, strlen(buffer));

    buffer = "\n";
    printDebug(buffer, strlen(buffer));
  }

  return ret;
}

void printn(int file_desc, char* str, int n)
{
  systemcall_wrapper(SYS_WRITE, file_desc, str, n);
}


int readn(int file_desc, char *buffer, int n)
{
  return systemcall_wrapper(SYS_READ, file_desc, buffer, n);
}

int fopen(char *filename, char *mode)
{
    int flags;

    flags = O_RDONLY;

    if (strcmp(mode, "r") == 0)
      flags = O_RDONLY;
    else if (strcmp(mode, "w") == 0)
      flags = O_WRONLY | O_CREAT;
    else if (strcmp(mode, "rw") == 0)
      flags = O_RDWR | O_CREAT;
    else
    {
      char *msg = "Cannot understand mode.\n\0";
      printDebug(msg, strlen(msg));
      exit(1);
    } 

    int desc;
    desc = systemcall_wrapper(SYS_OPEN, filename, flags, 0777);

    if (desc == -1)
    {
      char *msg = "Error reading file.\n\0";
      printDebug(msg, strlen(msg));
      exit(1);
    }

    return desc;    
}

void fclose(int fileDesc)
{
  systemcall_wrapper(SYS_CLOSE, fileDesc, 0, 0);
}


void str_tolower(char *c, int len)
{
  int i;
  for (i = 0; i < len; i++)
  {
    if (c[i] >= 'A' && c[i] <= 'Z')
    {
      c[i] -= 'A';
      c[i] += 'a';
    }
      
  }
}

void printlower(int file_desc, char* str, int len)
{
  str_tolower(str, len);
  printn(file_desc, str, len);
}

void read_loop(input_desc, output_desc)
{
  
    char c[100];

    int bytesRead = readn(input_desc, c, 100);
    while(bytesRead == 100)
    {
      printlower(output_desc, c, bytesRead);

      bytesRead = readn(input_desc, c, 100);
    }

    if (bytesRead != 0)
    {
      printlower(output_desc, c, bytesRead);
    }
}



int main (int argc , char* argv[], char* envp[])
{
    int i;
    int input = STDIN;
    int output = STDOUT;
    for (i = 1; i < argc; i++)
    {
        if(strcmp(argv[i],"-D")==0)
	        isDebug = true;
        else if(strncmp(argv[i], "-i", 2) == 0)
        {
            char *fileName;
            fileName = argv[i] + 2;

            input = fopen(fileName, "r\0");
        }
        else if(strncmp(argv[i], "-o", 2) == 0)
        {
            char *fileName;
            fileName = argv[i] + 2;

            output = fopen(fileName, "w");
        } 
   
    }
  read_loop(input, output);

  if (input != STDIN)
    fclose(input);

  if (output != STDOUT)
    fclose(output);

  return 0;
}
