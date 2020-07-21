#pragma once
#include <SFML/Graphics.hpp>
#include "KimeBoi.hpp"

void handle_input(Processor::_Memory mem)
{
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            mem.ram[0xFF00 - 0x8000] &= ~(1UL << 4);
            mem.write(0xFF00,mem.read(0xFF00,0) & ~(1UL << 1),0);
            //mem.ram[0xFF00 - 0x8000] &= ~(1UL << 1);
            mem.ram[0xFF0F - 0x8000] |= 0x10;

        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            mem.ram[0xFF00 - 0x8000] &= ~(1UL << 4);
            mem.write(0xFF00,mem.read(0xFF00,0) & ~(1UL << 0),0);
            mem.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            mem.ram[0xFF00 - 0x8000] &= ~(1UL << 4);
            mem.write(0xFF00,mem.read(0xFF00,0) & ~(1UL << 2),0);
            mem.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            mem.ram[0xFF00 - 0x8000] &= ~(1UL << 4);
            mem.write(0xFF00,mem.read(0xFF00,0) & ~(1UL << 3),0);
            mem.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) //start
        {
            mem.ram[0xFF00 - 0x8000] &= ~(1UL << 5);
            mem.write(0xFF00,mem.read(0xFF00,0) & ~(1UL << 3),0);
            mem.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::BackSlash)) //select
        {
            mem.ram[0xFF00 - 0x8000] &= ~(1UL << 5);
            mem.write(0xFF00,mem.read(0xFF00,0) & ~(1UL << 2),0);
            mem.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Slash)) //A
        {
            mem.ram[0xFF00 - 0x8000] &= ~(1UL << 5);
            mem.write(0xFF00,mem.read(0xFF00,0) & ~(1UL << 0),0);

            mem.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Quote)) //B
        {
            mem.ram[0xFF00 - 0x8000] &= ~(1UL << 5);
            mem.write(0xFF00,mem.read(0xFF00,0) & ~(1UL << 1),0);
            mem.ram[0xFF0F - 0x8000] |= 0x10;
        }
}