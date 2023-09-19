#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct list {
    struct list *prev;
    struct list *next;
};

typedef struct list list_t;
typedef struct list list_elem_t;

#define LIST_INIT(list)                                                                     \
({  (list)->next = (list);                                                                  \
    (list)->prev = (list);                                                                  \
})

#define LIST_CONTAINER(elem, container, field)                                              \
    (elem == NULL ? NULL :(container *) ((uintptr_t) (elem) - offsetof(container, field)))

#define LIST_FOR_EACH(elem, list, container, field)                                         \
    for (container *elem = LIST_CONTAINER((list)->next, container, field),                  \
                   *__next = NULL;                                                          \
         (&elem->field != (list)                                                            \
          && (__next = LIST_CONTAINER(elem->field.next, container, field)));                \
         elem = __next)

#define LIST_POP_TAIL(list, container, field)                                               \
    ({                                                                                      \
        list_elem_t *__elem = list_pop_tail(list);                                          \
        (__elem) ? LIST_CONTAINER(__elem, container, field) : NULL;                         \
    })

void list_elem_init(list_elem_t *elem);
void list_push_back(list_t *list, list_elem_t *elem);
bool list_is_linked(list_elem_t *elem);
void list_remove(list_elem_t *elem);
list_elem_t * list_head(list_t *list);
list_elem_t * list_tail(list_t *list);
list_elem_t * list_pop_tail(list_t *list);