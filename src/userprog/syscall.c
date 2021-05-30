#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "userprog/flist.h"
#include "devices/timer.h"

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

static bool verify_fix_length(void* start, unsigned length);
static bool verify_variable_length(char* start);

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

static void sys_exit(int status)
{
  pinfo process = plist_find(&plist, thread_current()->pid);
  process->exited = true;
  process->exit_status = status;
  thread_exit();
  return;
}

static void
syscall_handler (struct intr_frame *f) 
{
  int32_t* esp = (int32_t*)f->esp;

  if(!verify_variable_length((char*)esp))
    sys_exit(-1);

  if(esp[0] > SYS_NUMBER_OF_CALLS || esp[0] < 0)
    sys_exit(-1);
  
  /*
  for(int i = 1; i < argc[esp[0]]; i++)
  {
    if(!verify_variable_length((char*)(esp + i)))
      sys_exit(-1);
  }
  */
  

  if(!verify_fix_length(esp, sizeof(char*) * (argc[esp[0]] + 1)))
    sys_exit(-1); 
  
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

      sys_exit(esp[1]);
      return;
    }

    case SYS_EXEC: // 2
    {
      char* name = (char*)esp[1];
      if(!verify_variable_length(name))
        thread_exit();
      f->eax = process_execute(name);
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

      if(file_name == NULL || !verify_fix_length(file_name, file_size))
      {
        sys_exit(-1);
        return;
      }

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
      char* file_name = (char*)esp[1];

      if(file_name == NULL || !verify_variable_length(file_name))
        sys_exit(-1);;

      struct file* file = filesys_open(file_name);

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

      if(!verify_fix_length(buf, length))
      {
        // Borde inte behöver returna f->eax = -1 eftersom den dör.
        sys_exit(-1);
        return;
      }

      // To Screen or null
      if(fd == STDOUT_FILENO || buf == NULL)
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
        if(file == NULL || buf == NULL)
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

      if(!verify_fix_length(buf, length))
      {
        sys_exit(-1);
        return;
      }

      // From Keyboard or null
      if(fd == STDIN_FILENO || buf == NULL || fd <= 0)
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

      if(file == NULL)
      {
        sys_exit(-1);
        return;
      }

      filesys_close(file);
      return;
    }

    case SYS_PLIST: // 13
    {
      plist_printf(&plist);
      return;
    }

    case SYS_SLEEP: // 13
    {
      timer_msleep(esp[1]);
      return;
    }
  }
}

/* verfy_*_lenght are intended to be used in a system call that accept
 * parameters containing suspisious (user mode) adresses. The
 * operating system (executng the system call in kernel mode) must not
 * be fooled into using (reading or writing) addresses not available
 * to the user mode process performing the system call.
 *
 * In pagedir.h you can find some supporting functions that will help
 * you dermining if a logic address can be translated into a physical
 * addrerss using the process pagetable. A single translation is
 * costly. Work out a way to perform as few translations as
 * possible.
 *
 * Recommended compilation command:
 *
 *  gcc -Wall -Wextra -std=gnu99 -pedantic -m32 -g pagedir.o verify_adr.c
 */

/* Verify all addresses from and including 'start' up to but excluding
 * (start+length). */
bool verify_fix_length(void* start, unsigned length)
{
  if(!is_user_vaddr(start))
    return false;

  int last_page = pg_no((void *)((int)start + length - 1)); 

  for (int cur_page = pg_no(start); cur_page <= last_page; ++cur_page) {
    void* cur = (void *)(cur_page * PGSIZE);
    if(!is_user_vaddr(cur))
      return false;
    if (pagedir_get_page(thread_current()->pagedir, cur) == NULL)
      return false;
  }
  return true;
}

/* Verify all addresses from and including 'start' up to and including
 * the address first containg a null-character ('\0'). (The way
 * C-strings are stored.)
 */
bool verify_variable_length(char* start)
{
  if(!is_user_vaddr(start))
    return false;

  if(pagedir_get_page(thread_current()->pagedir, start) == NULL)
    return false;
 
  char *cur = start;

  while(*cur != '\0')// !is_end_of_string(cur))
  {
    unsigned prev_pg = pg_no(cur++);


    if(pg_no(cur) != prev_pg)
     {
      if(is_user_vaddr(cur))
        return false;

      if(pagedir_get_page(thread_current()->pagedir, cur) == NULL)
        return false;
     }
  }

  return true;
}