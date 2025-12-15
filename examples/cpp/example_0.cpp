
#include <iostream>

#define REGISTER_ZLIST_TYPES(X) \
    X(int, Int)

#include "zlist.h"

int main()
{
    z_list::list<int> l = {1, 2, 3};

    l.push_front(0);
    l.push_back(4);

    for (int n : l) 
    {
        std::cout << n << " ";
    }
    std::cout << "\n";

    return 0;
}
