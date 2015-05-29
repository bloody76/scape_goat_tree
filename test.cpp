#include "spg.hpp"
#include <chrono>
#include <set>

int main(void)
{
    std::size_t l_Size = 80000;
    /// SPG.
    std::clock_t l_clock1 = std::clock();
    //std::set<int> s2;
    //for (int i = 1; i < l_Size; i++)
    //    s2.insert(i);
    std::clock_t l_clock2 = std::clock();
    std::cout << l_clock2 - l_clock1 << std::endl;

    /// SET.
    l_clock1 = std::clock();
    SPG<> s{0.6f};
    for (int i = 1; i < l_Size; i++)
        s.insert(i);
    l_clock2 = std::clock();
    std::cout << l_clock2 - l_clock1 << std::endl;

    return 0;
}
