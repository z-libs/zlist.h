
#include <stdio.h>
#include "my_lists.h"

int main(void) 
{
    list_Int nums = list_init(Int);

    list_push_back(&nums, 10);
    list_push_back(&nums, 20);
    list_push_back(&nums, 30);

    printf("Integers: ");
    
    zlist_node_Int *iter; 
    list_foreach(&nums, iter)
    {
        printf("%d ", iter->value);
    }
    printf("\n");

    list_Point points = list_init(Point);

    list_push_back(&points, ((Point){1.5f, 2.5f}));
    list_push_back(&points, ((Point){3.0f, 4.0f}));

    zlist_node_Point *n0 = list_at(&points, 0);
    if (n0) 
    {
        printf("Point 0: {x: %.1f, y: %.1f}\n", n0->value.x, n0->value.y);
    }

    zlist_node_Point *n1 = list_at(&points, 1);
    if (n1) 
    {
        printf("Point 1: {x: %.1f, y: %.1f}\n", n1->value.x, n1->value.y);
    }

    printf("Clearing points...\n");
    zlist_node_Point *curr, *safe;
    list_foreach_safe(&points, curr, safe) 
    {
        printf("Removing {%.1f, %.1f}\n", curr->value.x, curr->value.y);
        list_remove_node(&points, curr);
    }

    list_clear(&nums);
    list_clear(&points); 

    return 0;
}