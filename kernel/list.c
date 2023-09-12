#include "list.h"

void list_elem_init(list_elem_t *elem) {
    elem->prev = NULL;
    elem->next = NULL;
}

static void list_insert(list_elem_t *prev, list_elem_t *next, list_elem_t *new) {
    new->prev = prev;
    new->next = next;
    next->prev = new;
    prev->next = new;
}

void list_push_back(list_t *list, list_elem_t *elem) {
    list_insert(list->prev, list, elem);
}

bool list_is_linked(list_elem_t *elem) {
    return elem->next != NULL;
}

void list_remove(list_elem_t *elem) {
    if(!list_is_linked(elem))
        return;
    
    // remove element
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;

    list_elem_init(elem);
}

list_elem_t * list_head(list_t *list) {
    list_t *head = list->next;

    if(head == list)
        return NULL;

    return head;
}

list_elem_t *list_tail(list_t *list) {
    list_t *tail = list->prev;

    if(tail == list)
        return NULL;

    return tail;
}

list_elem_t *list_pop_tail(list_t *list) {
    list_t *tail = list->prev;

    if(tail == list)
        return NULL;
    
    // delete element
    tail->prev->next = tail->next;
    list->prev = tail->prev;

    return tail;
}