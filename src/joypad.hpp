#pragma once
#include <SFML/Graphics.hpp>
#include "KimeBoi.hpp"

void handle_input(Processor::_Memory& mem)
{
    //mem.write(0xff00,0xff,0);


    //joypad be like 
    //start  select a b up down left right
    //  0       1   2 3 4    5   6     7 

    for(int i=0; i < 8; i++)
        mem.keystates[i] = 0;
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        mem.keystates[6] = 1;
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        mem.keystates[7] = 1;
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        mem.keystates[4] = 1;
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        mem.keystates[5] = 1;
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) //start
    {
        mem.keystates[0] = 1;
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::BackSlash)) //select
    {
        mem.keystates[1] = 1;
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Slash)) //A
    {
        mem.keystates[2] = 1;
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Quote)) //B
    {
        mem.keystates[3] = 1;
    }

    //for(int i = 0; i < 8; i++)
        //std::cout << mem.keystates[i];
    //std::cout << '\n';    
}