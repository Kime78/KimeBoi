#include "KimeBoi.hpp"
#include "ppu.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
std::ofstream fout;

int main(int argc, char** args)
{
    Processor game;
    fout.open("debug.out");
    game.initialise();
    PPU ppu;
    sf::RenderWindow window(sf::VideoMode(320, 288), "KimeBoi");
    window.setFramerateLimit(0);

    if(argc < 2)
    {
        std::cout << "Booting test.gb\n";
        FILE *file = fopen("E:\\CodeBlocks Projects (C# is better)\\KimeBoi\\bin\\boot.gb", "rb");
        int pos = 0;
        while (fread(&game.Memory.boot[pos], 1, 1, file))
        {
            pos++;
        }
        game.loadGame("E:\\CodeBlocks Projects (C# is better)\\KimeBoi\\bin\\test.gb");
    }
    else
    {
        std::cout << "Booting " << args[1] << "\n";
        FILE *file = fopen("E:\\CodeBlocks Projects (C# is better)\\KimeBoi\\bin\\boot.gb", "rb");
        int pos = 0;
        while (fread(&game.Memory.boot[pos], 1, 1, file))
        {
            pos++;
        }
        game.loadGame(args[1]);
    }
    
    
    sf::Texture test_tex;
    sf::Uint8 draw_array [160 * 144 * 4] {0};
    sf::Sprite test_spr;
    test_tex.create(160,144);
    test_spr.setScale(sf::Vector2f(2.0f,2.0f));
    test_spr.setPosition(sf::Vector2f(0.0f,0.0f));
    while (window.isOpen())
    {   
   
        int cycles_taken = 0;
        cycles_taken = game.emulateCycle(); //this has timer and interrupts togheter
        ppu.tick_ppu(cycles_taken, game.Memory, window); 
        //std::cout << std::hex << (int)game->Memory.read(0xFF0F,0) << '\n';
        //fout << "PC:\t" << game.to_hex(game.pc) << ' '
        //<< game.to_hex(game.Memory.read(game.pc, 0)) << ' ' 
        //<< game.to_hex(game.Memory.read(game.pc + 1, 0)) << ' '
        //<< game.to_hex(game.Memory.read(game.pc + 2, 0)) << ' '
        //<< '\t' << "IE: " << game.to_hex(game.Memory.read(0xffff,0))
        //<< ' ' << "IF: " << game.to_hex(game.Memory.read(0xff0f, 0))
        //<< ' ' << game.IME 
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
