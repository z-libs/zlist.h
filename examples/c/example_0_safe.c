
#include <stdio.h>
#define ZERROR_IMPLEMENTATION
#define ZERROR_SHORT_NAMES
#include "zerror.h"

#define REGISTER_ZLIST_TYPES(X) \
    X(int, Int)

#define ZLIST_SHORT_NAMES
#include "zlist.h"

zres process_numbers(void)
{
    zlist_autofree(Int) nums = list_init(Int);

    check_ctx(list_push_back_safe(&nums, 100), "Failed to push 100");
    check_ctx(list_push_back_safe(&nums, 200), "Failed to push 200");
    check_ctx(list_push_back_safe(&nums, 300), "Failed to push 300");

    printf("List size: %zu\n", nums.length);

    int first = try_into(zres, list_front_safe(&nums));
    int last  = try_into(zres, list_back_safe(&nums));

    printf("First: %d, Last: %d\n", first, last);

    int val = try_into(zres, list_back_safe(&nums));
    
    check(list_pop_back_safe(&nums));
    
    printf("Popped last element: %d\n", val);

    printf("Clearing list and attempting invalid pop...\n");
    list_clear(&nums);

    // This will fail and return zres_err(...) to main.
    check_ctx(list_pop_back_safe(&nums), "Invalid Pop on Empty List");

    printf("We shouldn't be here!\n");

    return zres_ok();
}

int main(void)
{
    return run(process_numbers());
}
