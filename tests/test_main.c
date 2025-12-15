
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct 
{ 
    float x, y; 
} Vec2;

#define REGISTER_ZLIST_TYPES(X) \
    X(int, Int)                 \
    X(Vec2, Vec2)

#include "zlist.h"

#define TEST(name) printf("[TEST] %-35s", name);
#define PASS() printf(" \033[0;32mPASS\033[0m\n")

void test_init_management(void) 
{
    TEST("Init, Is_Empty, Clear");

    // zlist_init.
    zlist_Int list = zlist_init(Int);
    assert(zlist_is_empty(&list));
    assert(list.length == 0);
    assert(list.head == NULL);
    assert(list.tail == NULL);

    // Basic push to verify clear works later.
    zlist_push_back(&list, 10);
    assert(!zlist_is_empty(&list));
    assert(list.length == 1);

    // zlist_clear.
    zlist_clear(&list);
    assert(zlist_is_empty(&list));
    assert(list.length == 0);
    assert(list.head == NULL);
    assert(list.tail == NULL);

    PASS();
}

void test_modification(void) 
{
    TEST("Push/Pop (Front/Back), Insert");

    zlist_Int list = zlist_init(Int);

    // Push back: [10, 20].
    zlist_push_back(&list, 10);
    zlist_push_back(&list, 20);
    assert(zlist_tail(&list)->value == 20);

    // Push front: [0, 10, 20].
    zlist_push_front(&list, 0);
    assert(zlist_head(&list)->value == 0);
    assert(list.length == 3);

    // Insert after: [0, 10, 15, 20].
    // Find node 10 (index 1)
    zlist_node_Int* node_10 = zlist_head(&list)->next;
    assert(node_10->value == 10);
    zlist_insert_after(&list, node_10, 15);
    
    assert(node_10->next->value == 15);
    assert(list.length == 4);

    // Pop front: [10, 15, 20].
    zlist_pop_front(&list);
    assert(zlist_head(&list)->value == 10);

    // Pop back: [10, 15].
    zlist_pop_back(&list);
    assert(zlist_tail(&list)->value == 15);
    assert(list.length == 2);

    zlist_clear(&list);
    PASS();
}

void test_data_access(void) 
{
    TEST("Head, Tail, At, Remove");

    zlist_Int list = zlist_init(Int);
    zlist_push_back(&list, 100);
    zlist_push_back(&list, 200);
    zlist_push_back(&list, 300);

    // Head/tail.
    assert(zlist_head(&list)->value == 100);
    assert(zlist_tail(&list)->value == 300);

    // At (O(N) linear scan).
    assert(zlist_at(&list, 1)->value == 200);
    assert(zlist_at(&list, 0)->value == 100);
    assert(zlist_at(&list, 2)->value == 300);
    assert(zlist_at(&list, 99) == NULL);

    // Remove node (middle).
    zlist_node_Int* n = zlist_at(&list, 1); // Node 200
    zlist_remove_node(&list, n); // [100, 300]
    
    assert(list.length == 2);
    assert(zlist_head(&list)->next->value == 300);
    assert(zlist_tail(&list)->prev->value == 100);

    zlist_clear(&list);
    PASS();
}

void test_algorithms(void) 
{
    TEST("Foreach, Reverse, Splice, Detach");

    zlist_Int list = zlist_init(Int);
    zlist_push_back(&list, 1);
    zlist_push_back(&list, 2);
    zlist_push_back(&list, 3);
    zlist_push_back(&list, 4);

    // zlist_foreach.
    int sum = 0;
    zlist_node_Int* it;
    zlist_foreach(&list, it) 
    {
        sum += it->value;
    }
    assert(sum == 10);

    // zlist_foreach_rev (Reverse iteration).
    sum = 0;
    zlist_foreach_rev(&list, it)
    {
        sum += it->value;
    }
    assert(sum == 10);

    // zlist_reverse: [4, 3, 2, 1].
    zlist_reverse(&list);
    assert(zlist_head(&list)->value == 4);
    assert(zlist_tail(&list)->value == 1);
    assert(zlist_at(&list, 1)->value == 3);

    // zlist_detach_node.
    zlist_node_Int* node_3 = zlist_at(&list, 1); // Value 3
    zlist_node_Int* detached = zlist_detach_node(&list, node_3);
    
    // Validate list state: [4, 2, 1].
    assert(list.length == 3);
    assert(zlist_head(&list)->next->value == 2);
    assert(detached->value == 3);
    assert(detached->next == NULL); // Should be isolated
    assert(detached->prev == NULL);
    
    ZLIST_FREE(detached); // We own it now.

    // zlist_splice (Move [4, 2, 1] into new list).
    zlist_Int dest = zlist_init(Int);
    zlist_push_back(&dest, 99); // Dest: [99]
    
    zlist_splice(&dest, &list);
    
    // Dest: [99, 4, 2, 1], List: [].
    assert(zlist_is_empty(&list));
    assert(dest.length == 4);
    assert(zlist_tail(&dest)->value == 1);
    assert(zlist_head(&dest)->value == 99);

    zlist_clear(&dest);
    PASS();
}

// Extension test (GCC/Clang only).
#if defined(__GNUC__) || defined(__clang__)
void test_autofree(void) 
{
    TEST("Auto-Cleanup Extension");
    
    {
        zlist_autofree(Int) auto_l = zlist_init(Int);
        zlist_push_back(&auto_l, 123);
        assert(zlist_head(&auto_l)->value == 123);
    } // auto_l cleared here.
    
    PASS();
}
#endif

int main(void) 
{
    printf("=> Running tests (zlist.h, main).\n");
    
    test_init_management();
    test_modification();
    test_data_access();
    test_algorithms();

#   if defined(__GNUC__) || defined(__clang__)
    test_autofree();
#   endif

    printf("=> All tests passed successfully.\n");
    return 0;
}

