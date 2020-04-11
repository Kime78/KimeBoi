#include "KimeBoi.hpp"
#include "ppu.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
std::ofstream fout;
int main()
{
    
    
    Processor *game = new Processor;
    fout.open("debug.out");
    game->initialise();
    game->Memory.write(0xff44,0x90);
    PPU *ppu = new PPU;
    sf::RenderWindow window(sf::VideoMode(320, 288), "KimeBoy");
    FILE *file = fopen("E:\\CodeBlocks Projects (C# is better)\\KimeBoi\\bin\\boot.gb", "rb");
	int pos = 0;
	while (fread(&game->Memory.boot[pos], 1, 1, file))
	{
		//std::cout << std::hex << (int)Memory.rom[pos] << " ";
		pos++;
	}
    game->loadGame("E:\\CodeBlocks Projects (C# is better)\\KimeBoi\\bin\\test.gb");
    sf::Texture texture;
    texture.loadFromFile("E:\\CodeBlocks Projects (C# is better)\\KimeBoi\\bin\\text.png");
    sf::Sprite test2;
    test2.setScale(sf::Vector2f(2.0f,2.0f));
    test2.setTexture(texture);


    sf::Texture test_tex;
    sf::Uint8 draw_array [160 * 144 * 4] {0};
    sf::Sprite test_spr;
    test_tex.create(160,144);
    //window.setFramerateLimit(60);
    test_spr.setScale(sf::Vector2f(2.0f,2.0f));
    test_spr.setPosition(sf::Vector2f(0.0f,0.0f));
    while (window.isOpen())
    {   
        
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                fout.open("debug.out");
                
             
                /*
                
                for(int i=0x8000;i<0xA000;i++)
                {
                    fout << std::hex <<(int)game->Memory.read(i) << " ";
                    if(i % 16 == 15)
                        fout << '\n';
                    if(i == 0x9800)
                        fout << "n\n\n\ngood part \n\n\n\n";
                    if(i >= 0x9800 && i<= 0x9BFF && game->Memory.read(i) != 0)
                        fout << "<- " << i << " ";    
                    if(i == 0x9BFF)
                        fout <<  "\n\n\n\nend\n\n\n\n";   
                }
               
                */
                fout.close();
                window.close();
            }
                
        }
        
        window.clear();
        
        
        ppu->fetch_pixel_array(game->Memory);
         for(int i=0;i<160;i++)
         {
            game->Memory.write(0xFF44,i);
            std::string output = "";
            game->emulateCycle(output);
            if(output != "")
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
                output += "\t\tSP:\t\t" + game->to_hex(game->sp);
                output += "\t\t(SP)\t\t" + game->to_hex(game->Memory.read(game->sp-1) << 8 | game->Memory.read(game->sp-2));

                fout << std::hex << output << std::endl;
            }
                
            for(int j=0;j<144;j++)
                {
                    int index = 4*(i + 160*j);
                    if(ppu->pixel_array[i][j] == 0)
                    {
                        draw_array[index + 0] = 255;
                        draw_array[index + 1] = 255;
                        draw_array[index + 2] = 255;
                        draw_array[index + 3] = 255;
                    }
                    
                    if(ppu->pixel_array[i][j] == 1)
                    {
                        draw_array[index + 0] = 144;
                        draw_array[index + 1] = 144;
                        draw_array[index + 2] = 144;
                        draw_array[index + 3] = 255;
                    }
                    if(ppu->pixel_array[i][j] == 2)
                    {
                        draw_array[index + 0] = 55;
                        draw_array[index + 1] = 55;
                        draw_array[index + 2] = 55;
                        draw_array[index+ 3] = 255;
                    }  
                    if(ppu->pixel_array[i][j] == 3)
                    {
                        draw_array[index + 0] = 0;
                        draw_array[index + 1] = 0;
                        draw_array[index + 2] = 0;
                        draw_array[index + 3] = 255;
                    }
                }
         }
        test_tex.update(draw_array);        
        test_spr.setTexture(test_tex);
        window.draw(test_spr); 
        window.display();
    }

    return 0;
}