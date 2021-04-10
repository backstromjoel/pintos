#include "flist.h"

// Fråga om vart mappen ska instansieras

void map_init(struct map* this) 
{
    list_init(&(this->content));

    // First filedescriptor
    this->next_key = 2;
}

key_t map_insert(struct map* this, const value_t value)
{
    struct association* new_association = (struct association*)malloc(sizeof(struct association));
    new_association->value = value;
    new_association->key = this->next_key++;
    
    list_push_back(&this->content, &new_association->elem);

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
    // Kanske fixa kodupprepning
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

            return value;
        }
    }
    return NULL;
}

void map_for_each(struct map* this, void (*exec)(key_t, value_t, int aux), int aux)
{
    struct list_elem* current;
    struct list_elem* end = list_end(&this->content);

    for(current = list_begin(&this->content); 
        current != end; 
        current = list_next(current)) 
    {
        struct association* a = list_entry(current, struct association, elem);
        (*exec)(a->key, a->value, aux);
    }
}

void map_remove_if(struct map* this, bool (*cond)(key_t, value_t, int aux), int aux)
{
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
}