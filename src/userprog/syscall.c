#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "userprog/flist.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
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

    case SYS_HALT: // 0
    {
      power_off();
      return;
    }

    case SYS_EXIT: // 1
    {
      f->eax = esp[1];
      thread_exit();
      return;
    }

    case SYS_EXEC: // 2
    {
      f->eax = process_execute((char*)esp[1]);
      return;
    }

    case SYS_WAIT: // 3
    {
      f->eax = process_wait(esp[1]);
      return;
    }

    case SYS_CREATE: // 4
    {
      char* file_name = (char*)esp[1];
      int file_size = esp[2];

      printf("Creating file: %s, with size: %d\n", file_name, file_size);

      f->eax = filesys_create(file_name, file_size);

      return;
    }

    case SYS_REMOVE: // 5
    {
      char* file_name = (char*)esp[1];
      f->eax = filesys_remove(file_name);
      return;
    }

    case SYS_OPEN: // 6
    {
      struct file* file = filesys_open((char*)esp[1]);

      if(file == NULL)
      {
        f->eax = -1;
        return;
      }

      int fd = map_insert(&(thread_current()->fmap), file);
      f->eax = fd;

      return;
    }

    case SYS_FILESIZE: // 7
    {
      int fd = esp[1];
      struct file* file = map_find(&(thread_current()->fmap), fd);
      if(file == NULL)
      {
        f->eax = -1;
        return;
      }

      int length = file_length(file);

      f->eax = length;

      return;
    }

    case SYS_READ: // 8
    {

      int fd = esp[1];
      char* buf = (char*)esp[2];
      int length = esp[3];
      int read = 0;

      // To Screen
      if(fd == STDOUT_FILENO)
      {
        f->eax = -1;
        return;
      }

      // From Keyboard
      else if(fd == STDIN_FILENO)
      {
        // Kanske ska vänta på enter? och hantera backspace?
        for(int i = 0; i < length; i++)
        {
          buf[i] = input_getc();

          if(buf[i] == '\r')
            buf[i] = '\n';
          putbuf(&buf[i], 1);
          read++;
        }
      }

      // From File
      else
      {
        struct file* file = map_find(&(thread_current()->fmap), fd);
        if(file == NULL)
        {
          f->eax = -1;
          return;
        }
        read = file_read(file, buf, length);
      }

      f->eax = read;

      return;
    }

    case SYS_WRITE: // 9
    {
      int fd = esp[1];
      char* buf = (char*)esp[2];
      int length = esp[3];
      int written = 0;

      // From Keyboard
      if(fd == STDIN_FILENO)
      {
        f->eax = -1;
        return;
      }

      // To Screen
      else if(fd == STDOUT_FILENO)
      {
        putbuf(buf, length);
        written = length;
      }

      // To File
      else
      {
        struct file* file = map_find(&(thread_current()->fmap), fd);
        if(file == NULL)
        {
          f->eax = -1;
          return;
        }
        written = file_write(file, buf, length);
      }

      f->eax = written;
      return;
    }

    case SYS_SEEK: // 10
    {
      int fd = esp[1];
      unsigned pos = esp[2];

      struct file* file = map_find(&(thread_current()->fmap), fd);
      if(file == NULL)
      {
        f->eax = -1;
        return;
      }
      file_seek(file, pos);
      return;
    }
    case SYS_TELL:
    {
      int fd = esp[1];

      struct file* file = map_find(&(thread_current()->fmap), fd);
      if(file == NULL)
      {
        f->eax = -1;
        return;
      }

      f->eax = file_tell(file);
      return;
    }

    case SYS_CLOSE: // 12
    {
      int fd = esp[1];
      struct file* file = map_remove(&(thread_current()->fmap), fd);
      filesys_close(file);
      return;
    }

    case SYS_PLIST: // 13
    {
      plist_printf(&plist);
      return;
    }

    // case SYS_SLEEP: // 13
    // {
    //   return;
    // }
  }
}

