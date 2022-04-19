#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "list.h"

struct Node
{
    int num;
    struct list_head l_ptr;
};

int main(void)
{
    LIST_HEAD(root);

    /* Add */
    for (size_t i = 0; i < 100; ++i)
    {
        struct Node *new_node = malloc(sizeof(struct Node));
        new_node->num = i + 1;
        // list_add(&new_node->l_ptr, &root);
        list_add_tail(&new_node->l_ptr, &root);
    }

    /* print list */
    struct list_head *element = NULL;
    list_for_each(element, &root)
    {
        printf("%d\n", list_entry(element, struct Node, l_ptr)->num);
    }

    /* Delete */
    struct Node *a_node = NULL;
    struct Node *b_node = NULL;
    list_for_each_entry_safe(a_node, b_node, &root, l_ptr)
    {
        list_del(&a_node->l_ptr);
        free(a_node);
    }

    if (list_empty(&root))
        printf("Free test_list successfully\n");
}
