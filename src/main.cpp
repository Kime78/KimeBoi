#include "KimeBoi.hpp"
#include "ppu.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
std::ofstream fout;
int main()
{
    Processor *game;
    game = new Processor;
    fout.open("debug.out");
    game->initialise();
    game->Memory.write(0xff44,0x90,0);
    PPU *ppu;
    ppu = new PPU;
    sf::RenderWindow window(sf::VideoMode(320, 288), "KimeBoy");
    FILE *file = fopen("E:\\CodeBlocks Projects (C# is better)\\KimeBoi\\bin\\boot.gb", "rb");
	int pos = 0;
	while (fread(&game->Memory.boot[pos], 1, 1, file))
	{
		//std::cout << std::hex << (int)Memory.rom[pos] << " ";
		pos++;
	}
    game->loadGame("E:\\CodeBlocks Projects (C# is better)\\KimeBoi\\bin\\test.gb");
    
    sf::Texture test_tex;
    sf::Uint8 draw_array [160 * 144 * 4] {0};
    sf::Sprite test_spr;
    test_tex.create(160,144);
    //window.setFramerateLimit(60);
    test_spr.setScale(sf::Vector2f(2.0f,2.0f));
    test_spr.setPosition(sf::Vector2f(0.0f,0.0f));
    while (window.isOpen())
    {   
        //handle_inputs(game);

        
        //this will be in a function 


        //random guess - STAT/VBlank interrupts being fired while LCD is disabled?
       // game->Memory.write(0xFF0F, game->Memory.read(0xFF0F, 0) | 0x1, 0);

        
        window.clear();
        game->Memory.write(0xFF00,0xFF,0);
        ppu->fetch_pixel_array(game->Memory);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            game->Memory.ram[0xFF00 - 0x8000] &= ~(1UL << 4);
            game->Memory.write(0xFF00,game->Memory.read(0xFF00) & ~(1UL << 1),0);
            //game->Memory.ram[0xFF00 - 0x8000] &= ~(1UL << 1);
            game->Memory.ram[0xFF0F - 0x8000] |= 0x10;

        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            game->Memory.ram[0xFF00 - 0x8000] &= ~(1UL << 4);
            game->Memory.write(0xFF00,game->Memory.read(0xFF00) & ~(1UL << 0),0);
            game->Memory.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            game->Memory.ram[0xFF00 - 0x8000] &= ~(1UL << 4);
            game->Memory.write(0xFF00,game->Memory.read(0xFF00) & ~(1UL << 2),0);
            game->Memory.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            game->Memory.ram[0xFF00 - 0x8000] &= ~(1UL << 4);
            game->Memory.write(0xFF00,game->Memory.read(0xFF00) & ~(1UL << 3),0);
            game->Memory.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) //start
        {
            game->Memory.ram[0xFF00 - 0x8000] &= ~(1UL << 5);
            game->Memory.write(0xFF00,game->Memory.read(0xFF00) & ~(1UL << 3),0);
            game->Memory.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::BackSlash)) //select
        {
            game->Memory.ram[0xFF00 - 0x8000] &= ~(1UL << 5);
            game->Memory.write(0xFF00,game->Memory.read(0xFF00) & ~(1UL << 2),0);
            game->Memory.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Slash)) //A
        {
            game->Memory.ram[0xFF00 - 0x8000] &= ~(1UL << 5);
            game->Memory.write(0xFF00,game->Memory.read(0xFF00) & ~(1UL << 0),0);

            game->Memory.ram[0xFF0F - 0x8000] |= 0x10;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Quote)) //B
        {
            game->Memory.ram[0xFF00 - 0x8000] &= ~(1UL << 5);
            game->Memory.write(0xFF00,game->Memory.read(0xFF00) & ~(1UL << 1),0);
            game->Memory.ram[0xFF0F - 0x8000] |= 0x10;
        }
        for(int i = 0; i < 160; i++)
         {
            game->Memory.write(0xFF44,i,0);
            std::string output = "";
            game->emulateCycle(output);
            
       
            if(output != "" && game->pc != 0x8014)
            {
                output += "\t\tregs:\t\t";
                output += "AF:" + game->to_hex(game->Registers.A << 8 | game->Registers.F) + " ";
                output += "BC:" + game->to_hex(game->Registers.B << 8 | game->Registers.C) + " ";
                output += "DE:" + game->to_hex(game->Registers.D << 8 | game->Registers.E) + " ";
                output += "HL:" + game->to_hex(game->Registers.H << 8 | game->Registers.L) + " ";
                output += "\t\tflags:\t\t";
                output += "N: " + game->to_hex(game->Flags.N) + " ";
                output += "Z: " + game->to_hex(game->Flags.Z) + " ";
                output += "C: " + game->to_hex(game->Flags.C) + " ";
                output += "H: " + game->to_hex(game->Flags.H) + " ";
                //output += "\t\tSP:\t\t" + game->to_hex(game->sp);
               
                output += "cycles: " + std::to_string(game->Memory.cycle_count) + "\t";
                output += "joypad: " + game->to_hex(game->Memory.read(0xFF00,0));

                fout << std::hex << output << std::endl;
                game->Memory.cycle_count += 2;
            }

            //i should do the color pallette soon    
            for(int j = 0; j < 144; j++)
                {
                    int index = 4 * (i + 160 * j);
                    if(ppu->pixel_array[i][j] == 0)
                    {
                        draw_array[index + 0] = 155;
                        draw_array[index + 1] = 188;
                        draw_array[index + 2] = 15;
                        draw_array[index + 3] = 255;
                       
                    }
                    
                    if(ppu->pixel_array[i][j] == 1)
                    {
                        draw_array[index + 0] = 139;
                        draw_array[index + 1] = 172;
                        draw_array[index + 2] = 15;
                        draw_array[index + 3] = 255;
                       
                    }
                    if(ppu->pixel_array[i][j] == 2)
                    {
                        draw_array[index + 0] = 48;
                        draw_array[index + 1] = 98;
                        draw_array[index + 2] = 48;
                        draw_array[index + 3] = 255;
                    }  
                    if(ppu->pixel_array[i][j] == 3)
                    {
                        draw_array[index + 0] = 15;
                        draw_array[index + 1] = 56;
                        draw_array[index + 2] = 15;
                        draw_array[index + 3] = 255;
                    }
                }
    
                //game->Memory.ram[0xFF0F - 0x8000] |= 0x1; //cycleless write for vblank
               
         }
        
        test_tex.update(draw_array);        
        test_spr.setTexture(test_tex);
        window.draw(test_spr); 
        window.display();

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
