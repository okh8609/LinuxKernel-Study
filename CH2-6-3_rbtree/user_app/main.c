#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "rbtree_augmented.h"

struct RBNode
{
    struct rb_node rbn;
    int num;
};

struct RBNode *search(struct rb_root *root, int key)
{
    struct rb_node *cur_rbn = root->rb_node;
    while (cur_rbn)
    {
        struct RBNode *data = container_of(cur_rbn, struct RBNode, rbn);

        if (key < data->num)
            cur_rbn = cur_rbn->rb_left;
        else if (key > data->num)
            cur_rbn = cur_rbn->rb_right;
        else
            return data;
    }
    return NULL;
}

int insert(struct rb_root *root, struct RBNode *data)
{
    struct rb_node *parents_rbn = NULL;
    struct rb_node **current_rbn = &(root->rb_node);

    while (*current_rbn)
    {
        parents_rbn = *current_rbn;

        struct RBNode *this = container_of(*current_rbn, struct RBNode, rbn);

        if (data->num < this->num)
            current_rbn = &((*current_rbn)->rb_left);
        else if (data->num > this->num)
            current_rbn = &((*current_rbn)->rb_right);
        else
            return -1;
    }
    rb_link_node(&(data->rbn), parents_rbn, current_rbn);
    rb_insert_color(&(data->rbn), root);
    return 0;
}

static int black_path_count(struct rb_node *rb) {
	int count;
	for (count = 0; rb; rb = rb_parent(rb))
		count += !rb_is_red(rb);

	return count;
}

static void check(struct rb_root *root)
{
	struct rb_node *rb;
	int count = 0, blacks = 0;
	int prev_key = 0; //

	for (rb = rb_first(root); rb; rb=rb_next(rb)) {
		struct RBNode *node = rb_entry(rb, struct RBNode, rbn);
		if (node->num < prev_key)
			printf("[WARN] node->key(%d) < prev_key(%d)\n", node->num, prev_key);
		if (rb_is_red(rb) && (!rb_parent(rb) || rb_is_red(rb_parent(rb))))
			printf("[WARN] two red nodes\n");
		if (!count)
			blacks = black_path_count(rb);
		else if ((!rb->rb_left || !rb->rb_right) && (blacks != black_path_count(rb)))
			printf("[WARN] black count wrongs\n");

		prev_key = node->num;
		count++;
	}
}

int main(void)
{
    srand(time(NULL)); // Initialization, should only be called once.

    // 開始
    struct rb_root myTree = RB_ROOT;
    for (size_t i = 0; i < 1000; ++i)
    {
        struct RBNode *data = (struct RBNode *)malloc(sizeof(struct RBNode));
        data->num = rand() % 65535; // Returns a pseudo-random integer between 0 and RAND_MAX.
        printf("%d\t", data->num);
        insert(&myTree, data);
    }

    printf("\n\n#####################################\n\n");

	/* check */
	check(&myTree);
    // 印出
    // for (struct rb_node *node = rb_first(&myTree); node; node = rb_next(node))
    // {
    //     struct RBNode *this = rb_entry(node, struct RBNode, rbn);
    //     printf("%d\t", this->num);
    // }

    printf("\n\n#####################################\n\n");

    // 結束(釋放資源)
    for (struct rb_node *node = rb_first(&myTree); node; node = rb_first(&myTree))
    {
        struct RBNode *this = rb_entry(node, struct RBNode, rbn);
        if (this != NULL)
        {
            rb_erase(&(this->rbn), &myTree);
            free(this);
        }
    }

    return 0;
}