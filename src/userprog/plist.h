#ifndef _PLIST_H_
#define _PLIST_H_


/* Place functions to handle a running process here (process list).
   
   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
     inserts this in a list of running processes and return an integer
     that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
     FIND the process information in the list. Should return some
     failure code if no process matching the integer is in the list.
     Or, optionally, several functions to access any information of a
     particular process that you currently need.

   - A function that given an integer REMOVE the process information
     from the list. Should only remove the information when no process
     or thread need it anymore, but must guarantee it is always
     removed EVENTUALLY.
     
   - A function that print the entire content of the list in a nice,
     clean, readable format.
     
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "threads/malloc.h"
#include "../lib/kernel/list.h"
#include "threads/synch.h"

struct process_information;

typedef unsigned pid_t;
typedef struct process_information* pinfo;

struct process_information 
{
  char* name;
  pid_t parent;
  int exit_status;
  struct semaphore wait_sema;
};

struct p_association
{
    pid_t key;
    pinfo value;
    struct list_elem elem;
};

struct plist
{
    struct list content;
    pid_t next_key;
};

void plist_init(struct plist*);
pid_t plist_insert(struct plist*, const pinfo);
pinfo plist_find(struct plist*, const pid_t);
pinfo plist_remove(struct plist*, const pid_t);
void plist_printf(struct plist*);

void plist_for_each(struct plist*, void (*exec)(pid_t, pinfo, int aux), int aux);
void plist_remove_if(struct plist* this, bool (*cond)(pid_t, pinfo, int aux), int aux);
void plist_remove_all(struct plist* this);

#endif

