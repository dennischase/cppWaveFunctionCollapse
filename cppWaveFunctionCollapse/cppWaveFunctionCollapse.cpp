#include <iostream>
#include "map.h"

int main()
{
    int height;
    int width;
    bool animate;
    std::string animateInput;
    int animateSpeed{0};

    // get size from user
    // consider your console window size

    std::cout << "What size of generation would you like?\nHeight: ";
    std::cin >> height; 
    std::cout << "Width: ";
    std::cin >> width; 

    // ask if user wants it to animate
    while (true)
    {

        std::cout << "Animate? (y/n): ";
        std::cin >> animateInput;


        std::transform(animateInput.begin(), animateInput.end(), animateInput.begin(), ::toupper);

        if (animateInput == "Y" || animateInput == "N")
        {
            if (animateInput == "Y") 
            {
                animate = true;
                break;
            }
            if (animateInput == "N") 
            {
                animate = false;
                break;
            }
        }

    }
    
    while (animate && animateSpeed == 0)
    {

        std::cout << "(F)ast, (M)ed, or (S)low? : ";
        std::cin >> animateInput;


        std::transform(animateInput.begin(), animateInput.end(), animateInput.begin(), ::toupper);

        if (animateInput == "F" || animateInput == "M" || animateInput == "S")
        {
            if (animateInput == "F")
            {
                animateSpeed = 0;
                break;
            }
            if (animateInput == "M")
            {
                animateSpeed = 100;
                break;
            }
            if (animateInput == "S")
            {
                animateSpeed = 500;
                break;
            }
        }
    }

    Map map = Map(height, width);
    map.collapse(animate, animateSpeed);
    //map.test();
}