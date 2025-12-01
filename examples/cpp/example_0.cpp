
#include <iostream>
#include "zlist.h"

DEFINE_LIST_TYPE(int, Int)

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
