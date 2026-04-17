#include <iostream>
#include "map.h"

int main()
{
    bool debug = false;

    int height = 25;
    int width = 60;
    bool animate = true;
    int animateSpeed = 25;
    bool showEntropy = true;

 

    if (!debug)
    {
        // get size from user
        // consider your console window size
        std::cout << "What size of generation would you like?\nHeight: ";
        std::cin >> height; 
        std::cout << "Width: ";
        std::cin >> width;

        // ask if user wants it to animate
        while (true)
        {

            std::string animateInput;
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

        // ask if user wants it to show the entropy
        while (animate)
        {
            std::string entropyInput;

            std::cout << "Show entropy? (y/n): ";
            std::cin >> entropyInput;


            std::transform(entropyInput.begin(), entropyInput.end(), entropyInput.begin(), ::toupper);

            if (entropyInput == "Y" || entropyInput == "N")
            {
                if (entropyInput == "Y")
                {
                    showEntropy = true;
                    break;
                }
                if (entropyInput == "N")
                {
                    showEntropy = false;
                    break;
                }
            }

        }

        if (animate)
        {
            while (true)
            {
                std::string animateInput;

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
                        animateSpeed = 50;
                        break;
                    }
                    if (animateInput == "S")
                    {
                        animateSpeed = 500;
                        break;
                    }
                }
            }
        }
    }

    Map map = Map(height, width);
    map.collapse(animate, animateSpeed, showEntropy);
}