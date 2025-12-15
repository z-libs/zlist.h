
# zlist.h

**Type-Safe, Generic Doubly Linked List for C/C++.**

`zlist.h` provides a generic, type-safe doubly linked list implementation for C without the overhead of `void*` casting or the complexity of manual intrusive structures. It uses C11 `_Generic` selection to generate fully typed implementations at compile time.

It bridges the gap between C and C++ by offering a native C API and a zero-cost C++ wrapper (`z_list::list`) that shares the same underlying implementation.

## Features

* **Type Safety:** Compile-time errors if you mix types (e.g., pushing `float` into `list_int`).
* **O(1) Operations:** Constant time insertion and removal at both ends and at known positions.
* **Bidirectional:** Full support for forward and backward traversal.
* **Safe Iteration:** Dedicated macros (`zlist_foreach_safe`) allow removing nodes while iterating.
* **C++ Interop:** A `z_list::list<T>` wrapper provides RAII, STL-compatible iterators, and range-based loops.
* **Safe API:** Optional integration with `zerror.h` for robust error handling with stack traces.

## Installation

### Manual

`zlist.h` works best when you use the provided scanner script to manage type registrations, though it can be used manually.

1.  Copy `zlist.h` to your project's include folder.
2.  Add the `z-core` tools (optional but recommended):

```bash
git submodule add https://github.com/z-libs/z-core.git z-core
```

### Clib

If you use the clib package manager, run:

```bash
clib install z-libs/zdk
```

### ZDK (Recommended)

If you use the Zen Development Kit, it is included automatically by including `<zdk/zlist.h>` (or `<zdk/zworld.h>`).

## Usage: C

For C projects, you must register the list types you need. This can be done via the scanner script or manually via the Registry Header.

### Manual Registration

Create a header(for example,`types.h`) to define your lists using X-Macros.

```c
#ifndef TYPES_H
#define TYPES_H

typedef struct { float x, y; } Point;

// Syntax: X(ValueType, ShortName).
#define REGISTER_ZLIST_TYPES(X) \
    X(int, Int)                 \
    X(Point, Point)

#include "zlist.h"

#endif
```

### Basic Operations

```c
#include "types.h"
#include <stdio.h>

int main(void)
{
    // Initialize.
    zlist_Int nums = zlist_init(Int);

    // O(1) Insertions (Returns Z_OK or Z_ENOMEM).
    zlist_push_back(&nums, 10);
    zlist_push_front(&nums, 5);

    // Iteration.
    zlist_node_Int *it;
    zlist_foreach(&nums, it)
    {
        printf("%d ", it->value);
    }
        
    // Cleanup.
    zlist_clear(&nums);
    return 0;
}
```

### Short Names

If you prefer a cleaner API and don't worry about namespace collisions, define `ZLIST_SHORT_NAMES` before including the header.

```c
#define ZLIST_SHORT_NAMES
#include "zlist.h"

// Now you can use:
list(Int) l = list_init(Int);
list_push_back(&l, 10);
list_foreach(&l, it) { ... }
```
## Usage: C++

The C++ wrapper `z_list::list` is a zero-overhead abstraction. It uses the same generated C code under the hood but adds RAII and iterators.

```c
#include <iostream>

// Register types (visible to C++ compiler).
#define REGISTER_ZLIST_TYPES(X) \
    X(int, Int)

#include "zlist.h"

int main()
{
    // RAII handles memory automatically.
    z_list::list<int> numbers = {1, 2, 3};

    numbers.push_front(0);
    numbers.push_back(4);

    // STL-compatible Iterators (Range-based for loop).
    for (int n : numbers)
    {
        std::cout << n << " ";
    }
        
    return 0;
}
```

## Safe API (`zerror` Integration)

If `zerror.h` is present, `zlist` generates "Safe" versions of critical functions. These functions return `zres` (Result) types containing error information and stack traces on failure.

| Operation | Standard API | Safe API | Return Type |
| :--- | :--- | :--- | :--- |
| **Push Back** | `zlist_push_back` | `zlist_push_back_safe` | `zres` (Void Result) |
| **Front** | `zlist_head` (returns ptr) | `zlist_front_safe` | `Result<T>` |

**Example:**

```c
zres res = zlist_push_back_safe(&list, 100);
if (zres_is_err(res))
{
    // Handle OOM with stack trace.
}
```

## API Reference (C)

The library uses `_Generic` dispatch, so the same macro names work for all registered list types.

**Management**

| Macro | Description |
| :--- | :--- |
| `zlist_init(Name)` | Initialize an empty list. |
| `zlist_clear(l)` | Free all nodes and reset list. |
| `zlist_splice(dest, src)` | Move all nodes from `src` to end of `dest`. O(1). |
| `zlist_autofree(Name)` | (GCC/Clang) Auto-cleanup variable. |

**Access & State**

| Macro | Description |
| :--- | :--- |
| `zlist_is_empty(l)` | Returns `true` if list contains no nodes. O(1). |
| `zlist_head(l)` | Returns pointer to first node (`zlist_node_Name*`). |
| `zlist_tail(l)` | Returns pointer to last node. |
| `zlist_at(l, idx)` | Returns pointer to node at index (O(N) scan). |

**Modification**

| Macro | Description |
| :--- | :--- |
| `zlist_push_back(l, val)` | Append to tail. Returns `Z_OK`/`Z_ENOMEM`. |
| `zlist_push_front(l, val)` | Prepend to head. Returns `Z_OK`/`Z_ENOMEM`. |
| `zlist_pop_back(l)` | Remove tail node. |
| `zlist_pop_front(l)` | Remove head node. |
| `zlist_insert_after(l, n, v)` | Insert `v` after node `n`. |
| `zlist_remove_node(l, n)` | Unlink and free specific node `n`. |
| `zlist_detach_node(l, n)` | Unlink node `n` **without** freeing. Returns `n`. |
| `zlist_reverse(l)` | Reverses the list in-place. O(N). |

**Iteration**

`zlist.h` provides two sets of iteration macros: "Smart" macros that attempt to auto-declare variables (on supported compilers), and "Explicit" macros that always declare variables portably by taking the type name.

| Macro | Description |
| :--- | :--- |
| `zlist_foreach(l, it)` | Forward iteration. **GCC/Clang**: Auto-declares `it`. **Std C**: `it` must be declared before. |
| `zlist_foreach_safe(l, it, tmp)` | Safe forward iteration (allows removal). **GCC/Clang**: Auto-declares `it`, `tmp`. **Std C**: Pre-declare variables. |
| `zlist_foreach_rev(l, it)` | Reverse iteration (tail to head). Same auto-declaration rules as above. |
| `zlist_foreach_rev_safe(l, it, tmp)` | Safe reverse iteration. Same auto-declaration rules as above. |
| `zlist_foreach_decl(Name, l, it)` | **Portable C99**. Forward iteration. Declares `it` as `zlist_node_Name*` inside the loop. |
| `zlist_foreach_safe_decl(Name, l, it, tmp)` | **Portable C99**. Safe forward iteration. Declares both `it` and `tmp` inside the loop. |
| `zlist_foreach_rev_decl(Name, l, it)` | **Portable C99**. Reverse iteration. Declares `it` inside the loop. |
| `zlist_foreach_rev_safe_decl(Name, l, it, tmp)` | **Portable C99**. Safe reverse iteration. Declares variables inside the loop. |

## API Reference (C++)

The C++ wrapper lives in the `z_list` namespace.

### class z_list::list<T>

**Constructors & Management**

| Method | Description |
| :--- | :--- |
| `list()` | Default constructor. |
| `~list()` | Destructor. Calls `zlist_clear`. |
| `size()` | Returns number of nodes. |
| `empty()` | Returns `true` if empty. |
| `clear()` | Frees all nodes. |
| `splice(other)` | Moves nodes from `other` list to this one. |

**Access & Modification**

| Method | Description |
| :--- | :--- |
| `front()`, `back()` | Access first/last element. Throws `out_of_range`. |
| `push_back(v)` | Append value. Throws `bad_alloc` on failure. |
| `push_front(v)` | Prepend value. |
| `pop_back()`, `pop_front()` | Remove elements. |
| `erase(it)` | Remove element at iterator. Returns next iterator. |
| `reverse()` | Reverses the list in-place. |

## Configuration

### Memory Management
You can override the memory allocators globally or specifically for lists.

```c
// Global Override
#define ZLIST_MALLOC  my_malloc
#define ZLIST_FREE    my_free
#define ZLIST_REALLOC my_realloc
#define ZLIST_CALLOC  my_calloc
```
