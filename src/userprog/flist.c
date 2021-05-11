#include "flist.h"

// inte säker att lås behövs här eftersom endast 1 tråd kommer åt mapen.
// static struct lock flist_lock;


void map_init(struct map* this) 
{
    list_init(&this->content);
    // lock_init(&flist_lock);

    // First filedescriptor
    this->next_key = 2;
}

key_t map_insert(struct map* this, const value_t value)
{
    // lock_acquire(&flist_lock);
    struct association* new_association = (struct association*)malloc(sizeof(struct association));
    new_association->value = value;
    new_association->key = this->next_key++;
    
    list_push_back(&this->content, &new_association->elem);

    // lock_release(&flist_lock);
    return new_association->key;
}

value_t map_find(struct map* this, const key_t key)
{
    struct list_elem* current;
    struct list_elem* end = list_end(&this->content);

    for(current = list_begin(&this->content); 
        current != end; 
        current = list_next(current)) 
    {
        struct association* a = list_entry(current, struct association, elem);

        if(key == a->key) 
            return a->value;
    }

    return NULL;
}

value_t map_remove(struct map* this, const key_t key)
{
    // lock_acquire(&flist_lock);
    struct list_elem* current;
    struct list_elem* end = list_end(&this->content);

    for(current = list_begin(&this->content); 
        current != end; 
        current = list_next(current)) 
    {
        struct association* a = list_entry(current, struct association, elem);

        if(key == a->key) 
        {
            value_t value = a->value;
            list_remove(current);
            free(a);
            // lock_release(&flist_lock);
            return value;
        }
    }
    // lock_release(&flist_lock);
    return NULL;
}

void map_for_each(struct map* this, void (*exec)(key_t, value_t, int aux), int aux)
{
    // lock_acquire(&flist_lock);
    struct list_elem* current;
    struct list_elem* end = list_end(&this->content);

    for(current = list_begin(&this->content); 
        current != end; 
        current = list_next(current)) 
    {
        struct association* a = list_entry(current, struct association, elem);
        (*exec)(a->key, a->value, aux);
    }
    // lock_release(&flist_lock);
}

void map_remove_if(struct map* this, bool (*cond)(key_t, value_t, int aux), int aux)
{
    // lock_acquire(&flist_lock);
    struct list_elem* current = list_begin(&this->content);
    struct list_elem* end = list_end(&this->content);

    while(current != end)
    {
        struct association* a = list_entry(current, struct association, elem);
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
    // lock_release(&flist_lock);
}

/* function passed as parameter to map_remove_if in order to free the
 * memory for all inseted values, and return true to remove them from
 * the map */
static bool do_free(key_t k UNUSED, value_t v, int aux UNUSED)
{
  free(v);     /*! free memory */
  return true; /*  and remove from collection */
}

// Removes everything that remains in map
void map_remove_all(struct map* this)
{
    map_remove_if(this, do_free, 0);
}