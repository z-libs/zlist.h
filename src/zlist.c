
#ifndef ZLIST_H
#define ZLIST_H

#include "zcommon.h"
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
#include <stdexcept>
#include <initializer_list>
#include <iterator>
#include <utility>

namespace z_list
{
    // Forward declarations.
    template <typename T> struct list;
    template <typename T> class list_iterator;

    /* Traits struct. */
    template <typename T>
    struct traits
    {
        static_assert(sizeof(T) == 0, "No zlist implementation registered for this type (via DEFINE_LIST_TYPE).");
    };

    template <typename T>
    class list_iterator
    {
    public:
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        using CNode = typename list<T>::c_node;

        // Constructor from C node pointer.
        explicit list_iterator(CNode *p) : current(p) {}

        // Accessors.
        reference operator*() const { return current->value; }
        pointer operator->() const { return &current->value; }

        // Comparison.
        bool operator==(const list_iterator& other) const { return current == other.current; }
        bool operator!=(const list_iterator& other) const { return current != other.current; }

        // Increment/decrement.
        list_iterator& operator++() { current = current->next; return *this; }
        list_iterator operator++(int) { list_iterator temp = *this; current = current->next; return temp; }
        list_iterator& operator--() { current = current->prev; return *this; }
        list_iterator operator--(int) { list_iterator temp = *this; current = current->prev; return temp; }

    private:
        CNode *current;
        friend struct list<T>;
    };

    template <typename T>
    struct list
    {
        // Aliases for the internal C types and trait lookup.
        using Traits = traits<T>;
        using c_list = typename Traits::list_type;
        using c_node = typename Traits::node_type;

        // Standard iterator types.
        using iterator = list_iterator<T>;
        using const_iterator = list_iterator<const T>;

        // Internal C structure.
        c_list inner;

        /* Constructors & destructor (RAII). */

        // Default constructor: calls C init function (zeroes struct)
        list() : inner(Traits::init()) {}

        // Initializer list constructor.
        list(std::initializer_list<T> init) : inner(Traits::init())
        {
            for (const auto& item : init) 
            {
                push_back(item);
            }
        }

        // Copy constructor (deep copy).
        list(const list& other) : inner(Traits::init())
        {
            for (const auto& item : other) {
                push_back(item);
            }
        }

        // Move constructor.
        list(list&& other) noexcept : inner(other.inner)
        {
            other.inner = Traits::init();
        }

        // Cleans up memory automatically (calls list_clear_##Name).
        ~list() { Traits::clear(&inner); }

        // Copy assignment.
        list& operator=(const list& other)
        {
            if (this != &other) {
                Traits::clear(&inner);
                inner = Traits::init();
                for (const auto& item : other) { push_back(item); }
            }
            return *this;
        }

        // Move assignment.
        list& operator=(list&& other) noexcept
        {
            if (this != &other) {
                Traits::clear(&inner);
                inner = other.inner;
                other.inner = Traits::init();
            }
            return *this;
        }

        /* Accessors. */

        size_t size() const { return inner.length; }
        bool empty() const { return inner.length == 0; }

        T& front() { if (empty()) throw std::out_of_range("list::front"); return inner.head->value; }
        const T& front() const { if (empty()) throw std::out_of_range("list::front"); return inner.head->value; }
        T& back() { if (empty()) throw std::out_of_range("list::back"); return inner.tail->value; }
        const T& back() const { if (empty()) throw std::out_of_range("list::back"); return inner.tail->value; }

        /* Modifiers. */

        void push_back(const T& val) { Traits::push_back(&inner, val); }
        void push_front(const T& val) { Traits::push_front(&inner, val); }

        void pop_back() { if (empty()) throw std::out_of_range("list::pop_back"); Traits::pop_back(&inner); }
        void pop_front() { if (empty()) throw std::out_of_range("list::pop_front"); Traits::pop_front(&inner); }

        void clear() { Traits::clear(&inner); }

        iterator insert_after(iterator pos, const T& val)
        {
            c_node *prev_node = pos.current;
            Traits::insert_after(&inner, prev_node, val);

            return iterator(prev_node ? prev_node->next : inner.head);
        }

        iterator erase(iterator pos)
        {
            if (pos.current == nullptr) throw std::out_of_range("list::erase on end() iterator");
            c_node *to_remove = pos.current;
            c_node *next_node = to_remove->next;
            Traits::remove_node(&inner, to_remove);
            return iterator(next_node);
        }

        void splice(list&& source)
        {
            Traits::splice(&inner, &source.inner);
        }

        /* Iterators. */

        iterator begin() { return iterator(inner.head); }
        const_iterator begin() const { return const_iterator(inner.head); }
        const_iterator cbegin() const { return const_iterator(inner.head); }

        iterator end() { return iterator(nullptr); }
        const_iterator end() const { return const_iterator(nullptr); }
        const_iterator cend() const { return const_iterator(nullptr); }
    };

    template <typename T>
    using list_T = list<T>;
} // namespace z_list.

extern "C" {
#endif // __cplusplus

 /* C implementation. */

#ifndef Z_LIST_MALLOC
    #define Z_LIST_MALLOC(sz)      Z_MALLOC(sz)
#endif

#ifndef Z_LIST_CALLOC
    #define Z_LIST_CALLOC(n, sz)   Z_CALLOC(n, sz)
#endif

#ifndef Z_LIST_REALLOC
    #define Z_LIST_REALLOC(p, sz)  Z_REALLOC(p, sz)
#endif

#ifndef Z_LIST_FREE
    #define Z_LIST_FREE(p)         Z_FREE(p)
#endif

/* * The generator macro.
 * Expands to a complete implementation of a doubly linked list for type T.
 * T    : The element type (e.g., int, float, Point).
 * Name : The suffix for the generated functions (for example, Int -> list_push_front_Int).
 *        Name must be one word only.
 */
#define Z_LIST_GENERATE_IMPL(T, Name)                                               \
                                                                                    \
/* Node structure. */                                                               \
typedef struct zlist_node_##Name                                                    \
{                                                                                   \
    struct zlist_node_##Name *prev;                                                 \
    struct zlist_node_##Name *next;                                                 \
    T value;                                                                        \
} zlist_node_##Name;                                                                \
                                                                                    \
/* List structure (container). */                                                   \
typedef struct                                                                      \
{                                                                                   \
    zlist_node_##Name *head;    /* Pointer to the first element. */                 \
    zlist_node_##Name *tail;    /* Pointer to the last element. */                  \
    size_t length;              /* Number of elements in the list. */               \
} list_##Name;                                                                      \
                                                                                    \
/* Initializes the list structure. */                                               \
static inline list_##Name list_init_##Name(void)                                    \
{                                                                                   \
    return (list_##Name){0};                                                        \
}                                                                                   \
                                                                                    \
/* Adds an element to the end of the list (O(1)). */                                \
static inline int list_push_back_##Name(list_##Name *l, T val)                      \
{                                                                                   \
    zlist_node_##Name *n = Z_LIST_MALLOC(sizeof(zlist_node_##Name));                \
    if (!n) return Z_ERR;                                                           \
    n->value = val;                                                                 \
    n->next = NULL;                                                                 \
    n->prev = l->tail;                                                              \
    if (l->tail) l->tail->next = n;                                                 \
    l->tail = n;                                                                    \
    if (!l->head) l->head = n;                                                      \
    l->length++;                                                                    \
    return Z_OK;                                                                    \
}                                                                                   \
                                                                                    \
/* Adds an element to the front of the list (O(1)). */                              \
static inline int list_push_front_##Name(list_##Name *l, T val)                     \
{                                                                                   \
    zlist_node_##Name *n = Z_LIST_MALLOC(sizeof(zlist_node_##Name));                \
    if (!n) return Z_ERR;                                                           \
    n->value = val;                                                                 \
    n->next = l->head;                                                              \
    n->prev = NULL;                                                                 \
    if (l->head) l->head->prev = n;                                                 \
    l->head = n;                                                                    \
    if (!l->tail) l->tail = n;                                                      \
    l->length++;                                                                    \
    return Z_OK;                                                                    \
}                                                                                   \
                                                                                    \
/* Inserts an element after a specific node (O(1)). */                              \
static inline int list_insert_after_##Name(list_##Name *l,                          \
    zlist_node_##Name *prev_node, T val)                                            \
    {                                                                               \
    if (!prev_node) return list_push_front_##Name(l, val);                          \
    zlist_node_##Name *n = Z_LIST_MALLOC(sizeof(zlist_node_##Name));                \
    if (!n) return Z_ERR;                                                           \
    n->value = val;                                                                 \
    n->prev = prev_node;                                                            \
    n->next = prev_node->next;                                                      \
    if (prev_node->next) prev_node->next->prev = n;                                 \
    else l->tail = n;                                                               \
    prev_node->next = n;                                                            \
    l->length++;                                                                    \
    return Z_OK;                                                                    \
}                                                                                   \
                                                                                    \
/* Removes the last element (O(1)). */                                              \
static inline void list_pop_back_##Name(list_##Name *l)                             \
{                                                                                   \
    if (!l->tail) return;                                                           \
    zlist_node_##Name *old_tail = l->tail;                                          \
    l->tail = old_tail->prev;                                                       \
    if (l->tail) l->tail->next = NULL;                                              \
    else l->head = NULL;                                                            \
    Z_LIST_FREE(old_tail);                                                          \
    l->length--;                                                                    \
}                                                                                   \
                                                                                    \
/* Removes the first element (O(1)). */                                             \
static inline void list_pop_front_##Name(list_##Name *l)                            \
{                                                                                   \
    if (!l->head) return;                                                           \
    zlist_node_##Name *old_head = l->head;                                          \
    l->head = old_head->next;                                                       \
    if (l->head) l->head->prev = NULL;                                              \
    else l->tail = NULL;                                                            \
    Z_LIST_FREE(old_head);                                                          \
    l->length--;                                                                    \
}                                                                                   \
                                                                                    \
/* Removes an arbitrary node (O(1)). */                                             \
static inline void list_remove_node_##Name(list_##Name *l, zlist_node_##Name *n)    \
{                                                                                   \
    if (!n) return;                                                                 \
    if (n->prev) n->prev->next = n->next;                                           \
    else l->head = n->next;                                                         \
    if (n->next) n->next->prev = n->prev;                                           \
    else l->tail = n->prev;                                                         \
    Z_LIST_FREE(n);                                                                 \
    l->length--;                                                                    \
}                                                                                   \
                                                                                    \
/* Frees all nodes in the list. */                                                  \
static inline void list_clear_##Name(list_##Name *l)                                \
{                                                                                   \
    zlist_node_##Name *curr = l->head;                                              \
    while (curr)                                                                    \
    {                                                                               \
        zlist_node_##Name *next = curr->next;                                       \
        Z_LIST_FREE(curr);                                                          \
        curr = next;                                                                \
    }                                                                               \
    l->head = l->tail = NULL;                                                       \
    l->length = 0;                                                                  \
}                                                                                   \
                                                                                    \
/* Moves all nodes from src to dest (src becomes empty, O(1)). */                   \
static inline void list_splice_##Name(list_##Name *dest, list_##Name *src)          \
{                                                                                   \
    if (dest == src) return;                                                        \
    if (!src->head) return;                                                         \
    if (!dest->head)                                                                \
    {                                                                               \
        *dest = *src;                                                               \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        dest->tail->next = src->head;                                               \
        src->head->prev = dest->tail;                                               \
        dest->tail = src->tail;                                                     \
        dest->length += src->length;                                                \
    }                                                                               \
    src->head = src->tail = NULL;                                                   \
    src->length = 0;                                                                \
}                                                                                   \
                                                                                    \
/* Returns node at index (O(N) - use only for debugging/small lists). */            \
static inline zlist_node_##Name *list_at_##Name(list_##Name *l, size_t index)       \
{                                                                                   \
    if (index >= l->length) return NULL;                                            \
    zlist_node_##Name *curr = l->head;                                              \
    while (index-- > 0) curr = curr->next;                                          \
    return curr;                                                                    \
}                                                                                   \
                                                                                    \
/* Returns the head node. */                                                        \
static inline zlist_node_##Name *list_head_##Name(list_##Name *l)                   \
{                                                                                   \
    return l->head;                                                                 \
}                                                                                   \
                                                                                    \
/* Returns the tail node. */                                                        \
static inline zlist_node_##Name *list_tail_##Name(list_##Name *l)                   \
{                                                                                   \
    return l->tail;                                                                 \
}

/* C Generic dispatch entries. */
#define L_PUSH_B_ENTRY(T, Name)   list_##Name*: list_push_back_##Name,
#define L_PUSH_F_ENTRY(T, Name)   list_##Name*: list_push_front_##Name,
#define L_INS_A_ENTRY(T, Name)    list_##Name*: list_insert_after_##Name,
#define L_POP_B_ENTRY(T, Name)    list_##Name*: list_pop_back_##Name,
#define L_POP_F_ENTRY(T, Name)    list_##Name*: list_pop_front_##Name,
#define L_REM_N_ENTRY(T, Name)    list_##Name*: list_remove_node_##Name,
#define L_CLEAR_ENTRY(T, Name)    list_##Name*: list_clear_##Name,
#define L_SPLICE_ENTRY(T, Name)   list_##Name*: list_splice_##Name,
#define L_HEAD_ENTRY(T, Name)     list_##Name*: list_head_##Name,
#define L_TAIL_ENTRY(T, Name)     list_##Name*: list_tail_##Name,
#define L_AT_ENTRY(T, Name)       list_##Name*: list_at_##Name,

/* Registry & type generation */
#if defined(__has_include) && __has_include("z_registry.h")
    #include "z_registry.h"
#endif

#ifndef Z_AUTOGEN_LISTS
    #define Z_AUTOGEN_LISTS(X)
#endif

#ifndef REGISTER_TYPES
    #define REGISTER_TYPES(X)
#endif

// Combine all sources of types
#define Z_ALL_LISTS(X) \
    Z_AUTOGEN_LISTS(X) \
    REGISTER_TYPES(X)

// Execute the generator for all registered types.
Z_ALL_LISTS(Z_LIST_GENERATE_IMPL)

/* C API Macros (using _Generic). */
#define list_init(Name)           list_init_##Name()

// Auto-cleanup extension (GCC/Clang).
#if Z_HAS_CLEANUP
    #define list_autofree(Name)  Z_CLEANUP(list_clear_##Name) list_##Name
#endif

#define list_push_back(l, val)     _Generic((l),    Z_ALL_LISTS(L_PUSH_B_ENTRY)  default: 0)         (l, val)
#define list_push_front(l, val)    _Generic((l),    Z_ALL_LISTS(L_PUSH_F_ENTRY)  default: 0)         (l, val)
#define list_insert_after(l, n, v) _Generic((l),    Z_ALL_LISTS(L_INS_A_ENTRY)   default: 0)         (l, n, v)
#define list_pop_back(l)           _Generic((l),    Z_ALL_LISTS(L_POP_B_ENTRY)   default: (void)0)   (l)
#define list_pop_front(l)          _Generic((l),    Z_ALL_LISTS(L_POP_F_ENTRY)   default: (void)0)   (l)
#define list_remove_node(l, n)     _Generic((l),    Z_ALL_LISTS(L_REM_N_ENTRY)   default: (void)0)   (l, n)
#define list_clear(l)              _Generic((l),    Z_ALL_LISTS(L_CLEAR_ENTRY)   default: (void)0)   (l)
#define list_splice(dst, src)      _Generic((dst),  Z_ALL_LISTS(L_SPLICE_ENTRY)  default: (void)0)   (dst, src)
#define list_head(l)               _Generic((l),    Z_ALL_LISTS(L_HEAD_ENTRY)    default: (void*)0)  (l)
#define list_tail(l)               _Generic((l),    Z_ALL_LISTS(L_TAIL_ENTRY)    default: (void*)0)  (l)
#define list_at(l, idx)            _Generic((l),    Z_ALL_LISTS(L_AT_ENTRY)      default: (void*)0)  (l, idx)

// Iteration helpers (C only).
#define list_foreach(l, iter) \
    for ((iter) = (l)->head; (iter) != NULL; (iter) = (iter)->next)

#define list_foreach_safe(l, iter, safe_iter) \
    for ((iter) = (l)->head, (safe_iter) = (iter) ? (iter)->next : NULL; \
         (iter) != NULL; \
         (iter) = (safe_iter), (safe_iter) = (iter) ? (iter)->next : NULL)

/*C++ trait specialization. */
#ifdef __cplusplus
} // extern "C"

namespace z_list
{
    #define ZLIST_CPP_TRAITS(T, Name)                                       \
        template<> struct traits<T>                                         \
        {                                                                   \
            using list_type = list_##Name;                                  \
            using node_type = zlist_node_##Name;                            \
            /* Define static constexpr auto for all C functions. */         \
            static constexpr auto init = list_init_##Name;                  \
            static constexpr auto push_back = list_push_back_##Name;        \
            static constexpr auto push_front = list_push_front_##Name;      \
            static constexpr auto insert_after = list_insert_after_##Name;  \
            static constexpr auto pop_back = list_pop_back_##Name;          \
            static constexpr auto pop_front = list_pop_front_##Name;        \
            static constexpr auto remove_node = list_remove_node_##Name;    \
            static constexpr auto clear = list_clear_##Name;                \
            static constexpr auto splice = list_splice_##Name;              \
            static constexpr auto head = list_head_##Name;                  \
            static constexpr auto tail = list_tail_##Name;                  \
        };

    Z_ALL_LISTS(ZLIST_CPP_TRAITS)
} // namespace z_list
#endif // __cplusplus

#endif // ZLIST_H
