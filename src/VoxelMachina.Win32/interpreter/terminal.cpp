#include <iostream>

#include "interpreter.h"

int main()
{
    std::cout << "Voxel Machine Terminal" << '\n';
    while(!std::cin.eof())
    {
        std::cout << ">>> ";
        std::string input;
        std::getline(std::cin, input);

        if (!std::cin.eof())
        {
            Interpreter interpreter = Interpreter(input);
            std::cout << interpreter.expr() << '\n';
        }
    }

    std::cout << '\n';
    return 0;
}
