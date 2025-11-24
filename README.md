# zlist.h

`zlist.h` provides generic doubly linked lists for C projects. Unlike typical C list implementations that rely on `void*` casting or intrusive node structures that break type safety, `zlist.h` uses C11 `_Generic` selection and X-Macros to generate fully typed, type-safe implementations for your specific data structures.

## Features

* **Type Safety**: Compiler errors if you try to push a `float` into a `list_int`.
* **O(1) Operations**: Constant time insertions and deletions at both ends (push/pop) and at known node positions.
* **Safe Iteration**: Includes `list_foreach_safe` to allow removing nodes while iterating without crashing.
* **Header Only**: No build scripts or linking required.
* **C11 Generics**: One API (`list_push_back`, `list_remove_node`, etc.) works for all registered types.
* **Zero Dependencies**: Only standard C headers used.

## Installation & Setup

Since `zlist.h` generates code for your specific types, you don't just include the library: you create a Registry Header.

> You can include the logic inside the source file, but if you are going to use the library in more than one file, this approach prevents code duplication.

### 1. Add the library

Copy `zlist.h` into your project's include directory.

### 2. Create a Registry Header

Create a file named `my_lists.h` (or similar) to define which types need lists.

```c
// my_lists.h
#ifndef MY_LISTS_H
#define MY_LISTS_H

#include "zlist.h"

typedef struct
{
    float x, y;
} Point;

// Register Types (The X-Macro):
// Syntax: X(ActualType, ShortName).
// - ActualType: The C type (e.g., 'int', 'struct Point').
// - ShortName:  Suffix for the generated functions (e.g., 'int', 'Point').
#define REGISTER_TYPES(X)     \
    X(int, int)               \
    X(Point, Point)

// This generates the implementation for you.
REGISTER_TYPES(DEFINE_LIST_TYPE)

#endif
```

### 3. Use in your code

Include your **registry header** (`my_lists.h`), not `zlist.h`.

```c
#include <stdio.h>
#include "my_lists.h"

int main(void)
{
    // Initialize (struct is on stack, nodes are malloc'd).
    list_int nums = list_init(int);

    // Push values.
    list_push_back(&nums, 10);
    list_push_front(&nums, 5);

    // Iterate safely (allowing removal).
    zlist_node_Int *curr, *safe;
    list_foreach_safe(&nums, curr, safe)
    {
        printf("%d ", curr->value);
        if (curr->value == 10) {
            list_remove_node(&nums, curr);
        }
    }

    // Cleanup (frees all remaining nodes).
    list_clear(&nums);
    return 0;
}
```

## API Reference

`zlist.h` uses C11 `_Generic` to automatically select the correct function implementation based on the list type you pass.

### Initialization & Management

| Macro | Description |
| :--- | :--- |
| `list_init(Name)` | Returns an empty list structure `{0}`. |
| `list_clear(l)` | Frees every node in the list and resets head/tail to `NULL`. |
| `list_splice(dest, src)` | Moves all nodes from `src` to the end of `dest` in O(1) time. `src` becomes empty. |

### Data Access

| Macro | Description |
| :--- | :--- |
| `list_head(l)` | Returns a pointer to the first **node**, or `NULL` if empty. |
| `list_tail(l)` | Returns a pointer to the last **node**, or `NULL` if empty. |
| `list_at(l, index)` | Returns a pointer to the **node** at `index` (O(N) traversal), or `NULL`. |

### Modification

| Macro | Description |
| :--- | :--- |
| `list_push_back(l, val)` | Allocates a new node with `val` and appends it to the tail. Returns `0` on success. |
| `list_push_front(l, val)` | Allocates a new node with `val` and prepends it to the head. Returns `0` on success. |
| `list_pop_back(l)` | Removes and frees the tail node. Does nothing if empty. |
| `list_pop_front(l)` | Removes and frees the head node. Does nothing if empty. |
| `list_insert_after(l, n, val)`| Allocates a new node with `val` and inserts it immediately after node `n`. |
| `list_remove_node(l, n)` | Unlinks and frees the specific node `n`. `n` must belong to list `l`. |

### Iteration

| Macro | Description |
| :--- | :--- |
| `list_foreach(l, iter)` | Standard loop helper. `iter` is a node pointer variable; it traverses from head to tail. |
| `list_foreach_safe(l, iter, safe)` | Safe loop helper. Requires two node pointers (`iter`, `safe`). Allows you to call `list_remove_node(l, iter)` inside the loop body without breaking the iterator. |

## Memory Management

By default, `zlist.h` uses the standard C library functions (`malloc`, `calloc`, `realloc`, `free`).

However, you can override these to use your own memory subsystem (e.g., **Memory Arenas**, **Pools**, or **Debug Allocators**).

### First Option: Global Override (Recommended)

To use a custom allocator, define the `Z_` macros **inside your registry header**, immediately before including `zlist.h`.

**Example: my_lists.h**

```c
#ifndef MY_LISTS_H
#define MY_LISTS_H

// Define your custom memory macros **HERE**.
#include "my_memory_system.h"

// IMPORTANT: Override all four to prevent mixing allocators.
//            This applies to all the z-libs.
#define Z_MALLOC(sz)      my_custom_alloc(sz)
#define Z_CALLOC(n, sz)   my_custom_calloc(n, sz)
#define Z_REALLOC(p, sz)  my_custom_realloc(p, sz)
#define Z_FREE(p)         my_custom_free(p)


// Then include the library.
#include "zlist.h"

// ... Register types ...


#endif
```

> **Note:** You **must** override **all four macros** (`MALLOC`, `CALLOC`, `REALLOC`, `FREE`) if you override one, to ensure consistency.

### Second Option: Library-Specific Override (Advanced)

If you need different allocators for different containers (e.g., an Arena for Lists but the Heap for Vectors), you can use the library-specific macros. These take priority over the global `Z_` macros.

```c
// Example: Lists use an Arena, everything else uses standard malloc.
#define Z_LIST_MALLOC(sz)    arena_alloc(my_arena, sz)
#define Z_LIST_FREE(p)       /* no-op for linear arena */

#include "zlist.h"
#include "zvec.h" // zvec will still use standard malloc!
```

## Notes

### Why do I need to provide a "Short Name"?

In `REGISTER_TYPES(X)`, you must provide two arguments: the **Actual Type** and a **Short Name**.

```c
//     Actual Type    Short Name
X(struct Point,         Point)
```

The reason is that C macros cannot handle spaces when generating names. The library tries to create structs and functions by gluing `list_` + `Name`.

If you used `struct Point` as the name, the macro would try to generate `list_struct Point`, which is a syntax error. By passing `Point`, it correctly generates `list_Point` and `zlist_node_Point`.
