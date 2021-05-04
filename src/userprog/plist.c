#include <stddef.h>

#include "plist.h"

void plist_init(struct plist* this) 
{
    list_init(&(this->content));

    // First process id
    this->next_key = 1;
}

pid_t plist_insert(struct plist* this, const pinfo value)
{
    struct p_association* new_p_association = (struct p_association*)malloc(sizeof(struct p_association));
    sema_init(&value->wait_sema, 0);
    value->parent_exited = false;
    value->exited = false;
    new_p_association->value = value;
    new_p_association->key = this->next_key++;

    
    list_push_back(&this->content, &new_p_association->elem);

    return new_p_association->key;
}

pinfo plist_find(struct plist* this, const pid_t key)
{
    struct list_elem* current;
    struct list_elem* end = list_end(&this->content);

    for(current = list_begin(&this->content); 
        current != end; 
        current = list_next(current)) 
    {
        struct p_association* a = list_entry(current, struct p_association, elem);

        if(key == a->key) 
            return a->value;
    }
    return NULL;
}

pinfo plist_remove(struct plist* this, const pid_t key)
{
    struct list_elem* current;
    struct list_elem* end = list_end(&this->content);

    for(current = list_begin(&this->content); 
        current != end; 
        current = list_next(current)) 
    {
        struct p_association* a = list_entry(current, struct p_association, elem);

        if(key == a->key) 
        {
            pinfo value = a->value;
            list_remove(current);
            free(a);

            return value;
        }
    }
    return NULL;
}

void plist_for_each(struct plist* this, void (*exec)(pid_t, pinfo, int aux), int aux)
{
    struct list_elem* current;
    struct list_elem* end = list_end(&this->content);

    for(current = list_begin(&this->content); 
        current != end; 
        current = list_next(current)) 
    {
        struct p_association* a = list_entry(current, struct p_association, elem);
        (*exec)(a->key, a->value, aux);
    }
}


static void print_value_struct(pid_t key, pinfo value, int aux UNUSED)
{
    printf("PROCESS ID: %d\nName: %s\nParent ID: %d\nExit_status: %d\n", 
            key, value->name, value->parent, value->exit_status);
    printf("--------------------------------------\n");
}

void plist_printf(struct plist* this)
{
    plist_for_each(this, print_value_struct, 0);
}

void plist_remove_if(struct plist* this, bool (*cond)(pid_t, pinfo, int aux), int aux)
{
    struct list_elem* current = list_begin(&this->content);
    struct list_elem* end = list_end(&this->content);

    while(current != end)
    {
        struct p_association* a = list_entry(current, struct p_association, elem);
        bool to_remove = (*cond)(a->key, a->value, aux);

        if(to_remove)
        {
            current = list_remove(current);
            free(a);
        }
        else
        {
            list_next(current);
        }
    }
}


/* function passed as parameter to plist_remove_if in order to free the
 * memory for all inseted values, and return true to remove them from
 * the plist */
static bool do_free(pid_t k UNUSED, pinfo v, int aux UNUSED)
{
  free(v);     /*! free memory */
  return true; /*  and remove from collection */
}

// Removes everything that remains in plist
void plist_remove_all(struct plist* this)
{
    plist_remove_if(this, do_free, 0);
}