#ifndef LIST_H
#define LIST_H

#define LIST(type) type##_list
#define LIST_NODE(type) type##_list_node
#define DEFINE_LIST(type) struct LIST_NODE(type) {struct type *data; struct LIST_NODE(type) *next;}; \
        struct LIST(type) {struct LIST_NODE(type) *head; struct LIST_NODE(type) *tail; int len;};    \

#define init_list(list)         \
        do {                    \
                list->head = 0; \
                list->tail = 0; \
                list->len = 0;  \
        }                       \
        while (0);              \

#define append_list(list, item, type)                                                                            \
        do {                                                                                                     \
                struct LIST_NODE(type) *node = malloc(sizeof(struct LIST_NODE(type))); \
                node->data = item;                                                                               \
                if (list->head == 0){                                                                            \
                        list->head = node;                                                                       \
                }                                                                                                \
                else {                                                                                           \
                        list->tail->next = node;                                                                 \
                }                                                                                                \
                list->tail = node;                                                                               \
                list->tail->next = 0;                                                                            \
                ++list->len;                                                                                     \
        }                                                                                                        \
        while (0);                                                                                               \

#define free_list(list, free_method, type)                         \
        do {                                                       \
                struct LIST_NODE(type) *node = list->head;         \
                while (node != 0) {                                \
                        struct LIST_NODE(type) *next = node->next; \
                        free_method(node->data);                   \
                        free(node);                                \
                        node = next;                               \
                }                                                  \
        }                                                          \
        while (0);                                                 \

#endif