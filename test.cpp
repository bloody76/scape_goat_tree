#include "spg.hpp"
#include <chrono>
#include <set>
#include <vector>
#include <random>
#include <algorithm>

int main(void)
{
    // Random vector.
    std::size_t l_Size = 1000000;

    std::vector<int> v;
    v.reserve(l_Size);

    for (int i = 0; i < l_Size; i++)
        v.push_back(i);

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(v.begin(), v.end(), g);

    /// SPG.

    auto l_clock1 = std::clock();
    SPG<int> s{0.59f};
    for (int e : v)
        s.insert(e);

    //s.print();

    auto sum = 0;
    for (auto i : s)
        sum += i;
        //std::cout << i << std::endl;

    auto l_clock2 = std::clock();
    std::cout << l_clock2 - l_clock1 << std::endl;

    l_clock1 = std::clock();

    /// SET.
    std::set<int> s2;
    for (int e : v)
        s2.insert(e);

    sum = 0;
    for (auto i = s2.cbegin(); i != s2.cend(); ++i)
        sum += *i;

    l_clock2 = std::clock();

    /// CALLGRIND_DUMP_STATS;

    std::cout << l_clock2 - l_clock1 << std::endl;

    s2.clear();

    return 0;
}
