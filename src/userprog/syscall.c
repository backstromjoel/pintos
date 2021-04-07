#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "threads/init.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:
   
   int sys_read_arg_count = argc[ SYS_READ ];
   
   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1, 
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};

static void
syscall_handler (struct intr_frame *f) 
{
  int32_t* esp = (int32_t*)f->esp;
  
  switch ( esp[0] /* retrive syscall number */ )
  {
    default:
    {
      printf ("Executed an unknown system call!\n");
      
      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);
      printf ("Stack top + 2: %d\n", esp[2]);
      printf ("Stack top + 3: %d\n", esp[3]);
      
      thread_exit ();
    }

    // /* The basic systemcalls. The ones you will implement. */
    // SYS_HALT,                   /* Halt the operating system. */
    // SYS_EXIT,                   /* Terminate this process. */
    // SYS_EXEC,                   /* Start another process. */
    // SYS_WAIT,                   /* Wait for a child process to die. */
    // SYS_CREATE,                 /* Create a file. */
    // SYS_REMOVE,                 /* Delete a file. */
    // SYS_OPEN,                   /* Open a file. */
    // SYS_FILESIZE,               /* Obtain a file's size. */
    // SYS_READ,                   /* Read from a file. */
    // SYS_WRITE,                  /* Write to a file. */
    // SYS_SEEK,                   /* Change position in a file. */
    // SYS_TELL,                   /* Report current position in a file. */
    // SYS_CLOSE,                  /* Close a file. */

    case SYS_HALT: // 0
    {
      printf("Running SYS_HALT\n");
      power_off();
      return;
    }
    case SYS_EXIT: // 1
    {
      printf("Running SYS_EXIT with STATUS: %d\n", esp[1]);
      thread_exit();
      return;
    }
    case SYS_READ: // 8
    {

      int fd = esp[1];
      char* buf = (char*)esp[2];
      int length = esp[3];
      int read = 0;

      if(fd == STDIN_FILENO)
      {
        for(int i = 0; i < length; i++)
        {
          buf[i] = input_getc();

          if(buf[i] == '\r')
            buf[i] = '\n';
            
          putbuf(&buf[i], 1);
          read++;
        }
      }

      f->eax = read;

      return;
    }
    case SYS_WRITE: // 9
    {
      int fd = esp[1];
      char* buf = (char*)esp[2];
      int length = esp[3];

      if(fd == STDOUT_FILENO)
      {
        putbuf(buf, length);
      }
      f->eax = length;

      return;
    }
  }
}
