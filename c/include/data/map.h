#ifndef MAP_H
#define MAP_H
#include "data/list.h"
#include <stdbool.h>

#define MAP_ENTRY(ktype, vtype) ktype##_##vtype##_map_entry
#define MAP(ktype, vtype) ktype##_##vtype##_map
#define LISTMAP_ENTRY(ktype, vtype) ktype##_##vtype##_map_entry_list
#define LIST_NODEMAP_ENTRY(ktype, vtype) ktype##_##vtype##_map_entry_list_node

#define DEFINE_MAP(ktype, vtype)                                                \
        struct MAP_ENTRY(ktype, vtype) {                                        \
                const struct ktype *key;                                        \
                const struct vtype *value;                                      \
        };                                                                      \
        DEFINE_LIST(MAP_ENTRY(ktype, vtype))                                    \
        struct MAP(ktype, vtype) {                                              \
                struct LISTMAP_ENTRY(ktype, vtype) *list;                       \
                bool (*equals)(const struct ktype*, const struct ktype*);       \
                void (*free_func)(const struct MAP_ENTRY(ktype, vtype)*);       \
        };                                                                      \

#define init_map(map, eq, ff, ktype, vtype)                                                                               \
        do {                                                                                                              \
                map->list = malloc(sizeof(struct LISTMAP_ENTRY(ktype, vtype)));     \
                init_list(map->list);                                                                                     \
                map->equals = eq;                                                                                         \
                map->free_func = ff;                                                                                      \
        } while (0);

#define update_map(map, k, v, ktype, vtype)                                                                                                       \
        do {                                                                                                                                      \
                struct MAP_ENTRY(ktype, vtype) *entry = malloc(sizeof(struct MAP_ENTRY(ktype, vtype)));         \
                entry->key = k;                                                                                                                   \
                entry->value = v;                                                                                                                 \
                bool found = false;                                                                                                               \
                for (struct LIST_NODEMAP_ENTRY(ktype, vtype) *node = map->list->head; node != NULL; node = node->next) {                          \
                        if (map->equals(k, node->data->key)) {                                                                                    \
                                map->free_func(node->data);                                                                                       \
                                node->data = entry;                                                                                               \
                                found = true;                                                                                                     \
                                break;                                                                                                            \
                        }                                                                                                                         \
                }                                                                                                                                 \
                if (!found) {                                                                                                                     \
                        append_list(map->list, entry, MAP_ENTRY(ktype, vtype))                                                                    \
                }                                                                                                                                 \
        } while (0);           

#define query_map(map, k, result, ktype, vtype)                                                                            \
        do {                                                                                                               \
                result = NULL;                                                                                             \
                for (struct LIST_NODEMAP_ENTRY(ktype, vtype) *node = map->list->head; node != NULL; node = node->next) {   \
                        if (map->equals(k, node->data->key)){                                                              \
                                result = node->data->value;                                                                \
                                break;                                                                                     \
                        }                                                                                                  \
                }                                                                                                          \
        } while (0);                                                                                                       \

#define free_map(map, ktype, vtype)                                            \
        do {                                                                   \
                free_list(map->list, map->free_func, MAP_ENTRY(ktype, vtype)); \
                free(map->list);                                               \
        } while (0);                                                           \

#endif
