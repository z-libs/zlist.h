
#include <iostream>
#include <string>

struct Point 
{
    float x, y;
};

#include "zlist.h"

DEFINE_LIST_TYPE(int, Int)
DEFINE_LIST_TYPE(Point, Point)

int main() 
{
    std::cout << "=> Integer List\n";

    z_list::list<int> numbers = {10, 20, 30};

    numbers.push_front(5);
    numbers.push_back(40);

    for (int n : numbers)
    {
        std::cout << n << " ";
    }
    std::cout << "\n\n";

    std::cout << "=> Struct List\n";

    z_list::list<Point> path;
    path.push_back({1.0f, 2.0f});
    path.push_back({3.5f, 4.5f});

    for (const auto& p : path) 
    {
        std::cout << "Point: {" << p.x << ", " << p.y << "}\n";
    }

    return 0; // Destructors automatically free memory here.
}