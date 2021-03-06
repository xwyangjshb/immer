
#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <iostream>
#include <algorithm>

// include:myiota/start
immer::vector<int> myiota(immer::vector<int> v, int first, int last)
{
    auto t = v.transient();
    std::generate_n(std::back_inserter(t),
                    last - first,
                    [&] { return first++; });
    return t.persistent();
}
// include:myiota/end

int main()
{
    auto v = myiota({}, 0, 100);
    std::copy(v.begin(), v.end(),
              std::ostream_iterator<int>{
                  std::cout, "\n"});
}
