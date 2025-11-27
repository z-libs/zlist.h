# zlist.h

`zlist.h` provides generic doubly linked lists for C projects. Unlike typical C list implementations that rely on `void*` casting or intrusive node structures that break type safety, `zlist.h` uses C11 `_Generic` selection and X-Macros to generate fully typed, type-safe implementations for your specific data structures.

## Features

* **Type Safety**: Compiler errors if you try to push a `float` into a `list_int`.
* **O(1) Operations**: Constant time insertions and deletions at both ends and at known node positions.
* **Safe Iteration**: Includes `list_foreach_safe` to allow removing nodes while iterating without crashing.
* **Zero Boilerplate**: Use the **Z-Scanner** tool to automatically generate type registrations.
* **Header Only**: No linking required.
* **Memory Agnostic**: Supports custom allocators (Arenas, Pools, Debuggers).
* **Zero Dependencies**: Only standard C headers used.

## Quick Start (Automated)

The easiest way to use `zlist.h` is with the **Z-Scanner** tool, which scans your code and handles the boilerplate for you.

### 1. Setup

Add `zlist.h` and the `z-core` tools to your project:

```bash
# Copy zlist.h to your root or include folder.
git submodule add https://github.com/z-libs/z-core.git z-core
```

### 2. Write Code

You don't need a separate registry file. Just define the types you need right where you use them (or in your own headers).

```c
#include <stdio.h>
#include "zlist.h"

// Define your struct.
typedef struct { float x, y; } Point;

// Request the list types you need.
// (These are no-ops for the compiler, but markers for the scanner).
DEFINE_LIST_TYPE(int, Int)
DEFINE_LIST_TYPE(Point, Point)

int main(void)
{
    list_Int nums = list_init(Int);
    list_push_back(&nums, 42);

    list_Point path = list_init(Point);
    list_push_back(&path, ((Point){1.0f, 2.0f}));

    printf("Head: %d\n", list_head(&nums)->value);
    
    list_clear(&nums);
    list_clear(&path);
    return 0;
}
```

### 3. Build

Run the scanner before compiling. It will create a header that `zlist.h` automatically detects.

```bash
# Scan your source folder (for example, src/ or .) and output to 'z_registry.h'.
python3 z-core/zscanner.py . z_registry.h

# Compile (Include the folder where z_registry.h lives, or just move it).
gcc main.c -I. -o game
```

## Manual Setup

If you cannot use Python or prefer manual control, you can use the **Registry Header** approach.

* Create a file named `my_lists.h` (or something else).
* Register your types using X-Macros.

```c
#ifndef MY_LISTS_H
#define MY_LISTS_H

typedef struct { float x; } Point;

// **IT HAS TO BE INCLUDED AFTER, NOT BEFORE**.
#define REGISTER_TYPES(X) \
    X(int, Int)           \
    X(Point, Point)

#include "zlist.h"

#endif
```

* Include `"my_lists.h"` instead of `"zlist.h"` in your C files.

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

## Extensions (Experimental)

If you are using a compiler that supports `__attribute__((cleanup))` (like GCC or Clang), you can use the **Auto-Cleanup** extension to automatically free lists when they go out of scope.

| Macro | Description |
| :--- | :--- |
| `list_autofree(Type)` | Declares a list that automatically calls `list_clear` (freeing all nodes) when the variable leaves scope (RAII style). |

**Example:**
```c
void process_queue() {
    // 'queue' nodes will be automatically freed when this function returns.
    list_autofree(int) queue = list_init(int);
    list_push_back(&queue, 100);
}
```

> **Disable Extensions:** To force standard compliance and disable these extensions, define `Z_NO_EXTENSIONS` before including the library.

### Why `list_clear`?

Unlike vectors, which free a single buffer, lists must walk the chain and free every node individually. The `list_autofree` macro automatically calls `list_clear`, ensuring O(N) cleanup happens implicitly so you don't leak nodes.

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
