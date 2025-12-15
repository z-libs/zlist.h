
#include <iostream>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <iterator>

struct Vec2 
{ 
    float x, y; 
    
    bool operator==(const Vec2& other) const 
    {
        return x == other.x && y == other.y;
    }
};

#define REGISTER_ZLIST_TYPES(X) \
    X(int, Int)                 \
    X(Vec2, Vec2)

#include "zlist.h"

#define TEST(name) printf("[TEST] %-40s", name);
#define PASS() std::cout << "\033[0;32mPASS\033[0m\n";

void test_constructors() 
{
    TEST("Constructors (Default, InitList)");

    // Default.
    z_list::list<int> l1;
    assert(l1.empty());
    assert(l1.size() == 0);

    // Initializer list.
    z_list::list<int> l2 = {1, 2, 3, 4, 5};
    assert(l2.size() == 5);
    assert(l2.front() == 1);
    assert(l2.back() == 5);

    PASS();
}

void test_rule_of_five() 
{
    TEST("Rule of 5 (Copy/Move Semantics)");

    // Copy constructor (deep copy).
    z_list::list<int> original = {10, 20, 30};
    z_list::list<int> copy = original;
    
    assert(copy.size() == 3);
    assert(copy.front() == 10);
    // Ensure deep copy (modifying copy doesn't affect original).
    copy.pop_front();
    assert(copy.size() == 2);
    assert(original.size() == 3);

    // Move Constructor (pointer stealing).
    z_list::list<int> moved = std::move(original);
    
    assert(moved.size() == 3);
    assert(moved.front() == 10);
    assert(original.empty()); // Source must be empty.
    assert(original.size() == 0);

    // Move assignment.
    z_list::list<int> assigned;
    assigned = std::move(moved);
    assert(assigned.size() == 3);
    assert(assigned.front() == 10);
    assert(moved.empty());

    PASS();
}

void test_stl_interop() 
{
    TEST("STL Compatibility (Iterators, Algo)");

    z_list::list<int> l = {10, 20, 30, 40, 50};

    // Range-based for loop.
    int sum = 0;
    for (int x : l) 
    {
        sum += x;
    }
    assert(sum == 150);

    // std::find (bidirectional iterators).
    auto it = std::find(l.begin(), l.end(), 30);
    assert(it != l.end());
    assert(*it == 30);

    // std::distance.
    assert(std::distance(l.begin(), it) == 2);

    // Reverse iteration.
    // (rbegin/rend usually requires extra adapter, but we can test decrement).
    auto bit = l.end();
    bit--;
    assert(*bit == 50);
    bit--;
    assert(*bit == 40);

    PASS();
}

void test_access_modifiers() 
{
    TEST("Access & Modifiers (Push, Pop, Rev)");

    z_list::list<Vec2> points;
    
    // Push back / front.
    points.push_back({10, 10});
    points.push_front({5, 5});  // [{5,5}, {10,10}].
    
    assert(points.size() == 2);
    assert(points.front().x == 5);
    assert(points.back().x == 10);

    // Pop.
    points.pop_front(); // [{10,10}].
    assert(points.front().x == 10);
    assert(points.size() == 1);

    // Reverse.
    points.push_back({20, 20}); // [{10,10}, {20,20}].
    points.reverse();           // [{20,20}, {10,10}].
    
    assert(points.front().x == 20);
    assert(points.back().x == 10);

    // Exceptions.
    try 
    {
        points.clear();
        points.pop_back(); // Should throw.
        assert(false && "Should have thrown out_of_range");
    } 
    catch (const std::out_of_range&) 
    {
        // Success.
    }

    PASS();
}

void test_splice()
{
    TEST("Splice (List Move)");
    
    z_list::list<int> l1 = {1, 2};
    z_list::list<int> l2 = {3, 4};

    l1.splice(std::move(l2));

    assert(l1.size() == 4);
    assert(l1.back() == 4);
    assert(l2.empty()); // Source must be empty.

    PASS();
}

void test_const_correctness() 
{
    TEST("Const Correctness");
    
    const z_list::list<int> l = {100, 200};
    
    assert(!l.empty());
    assert(l.size() == 2);
    assert(l.front() == 100);
    
    // Const iterator check.
    int sum = 0;
    for (const int& x : l) 
    {
        sum += x;
    }
    assert(sum == 300);
    
    PASS();
}

int main() 
{
    std::cout << "=> Running tests (zlist.h, cpp).\n";
    
    test_constructors();
    test_rule_of_five();
    test_stl_interop();
    test_access_modifiers();
    test_splice();
    test_const_correctness();

    std::cout << "=> All tests passed successfully.\n";
    return 0;
}

