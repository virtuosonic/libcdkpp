
#include "../cdk.hpp"
#include <iostream>
int main()
{
    cdk::screen screen(initscr());
    cdk::label label(screen,{10,5},{"libcdkpp","demo","1.0"},{});
    std::cin.get();

    return 0;
}