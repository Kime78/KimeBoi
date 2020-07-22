#include "KimeBoi.hpp"
#include "ppu.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
std::ofstream fout;

int main()
{
    Processor game;
    fout.open("debug.out");
    game.initialise();
   // game.Memory.write(0xff44,0x90,0);
    PPU ppu;
    sf::RenderWindow window(sf::VideoMode(320, 288), "KimeBoi");
    window.setFramerateLimit(0);
    FILE *file = fopen("E:\\CodeBlocks Projects (C# is better)\\KimeBoi\\bin\\boot.gb", "rb");
	int pos = 0;
	while (fread(&game.Memory.boot[pos], 1, 1, file))
	{
		//std::cout << std::hex << (int)Memory.rom[pos] << " ";
		pos++;
	}
    game.loadGame("E:\\CodeBlocks Projects (C# is better)\\KimeBoi\\bin\\test.gb");
    
    sf::Texture test_tex;
    sf::Uint8 draw_array [160 * 144 * 4] {0};
    sf::Sprite test_spr;
    test_tex.create(160,144);
    //window.setFramerateLimit(60);
    test_spr.setScale(sf::Vector2f(2.0f,2.0f));
    test_spr.setPosition(sf::Vector2f(0.0f,0.0f));
    while (window.isOpen())
    {   
        game.Memory.write(0xff44,ppu.LY,0);
        //game.Memory.write(0xff00,0xff,0);
        int cycles_taken = 0;
        cycles_taken = game.emulateCycle(); //this has timer and interrupts togheter
        ppu.tick_ppu(cycles_taken, game.Memory, window); 
        //std::cout << std::hex << (int)game->Memory.read(0xFF0F,0) << '\n';
        //fout << std::hex << "PC:\t" << game.to_hex(game.pc) << ' '
        //<< game.to_hex(game.Memory.read(game.pc, 0)) << ' ' 
        //<< game.to_hex(game.Memory.read(game.pc + 1, 0)) << ' '
        //<< game.to_hex(game.Memory.read(game.pc + 2, 0)) << ' '
        //<< '\t' << "IE: " << game.to_hex(game.Memory.read(0xffff,0))
        //<< ' ' << "IF: " << game.to_hex(game.Memory.read(0xff0f, 0))
        //<< ' ' << game.IME 
        //<< " FF0F: " << game.to_hex(game.Memory.read(0xff0f, 0))
        //<< '\n';
        //std::cout << (int)game.Memory.read(0xFF00, 0) << '\n';
        //fout << game.to_hex(game.Memory.read(0xff00, 0)) << '\n';
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                fout.close();
                window.close();
            }
                
        }
        
    }

    return 0;
}
