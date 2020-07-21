#include "ppu.hpp"
#include <iostream>
#include <SFML/Graphics.hpp>
#include "joypad.hpp"
void PPU::fetch_pixel_array(Processor::_Memory mem)
{
    std::uint8_t tile_index = 0;
    std::uint8_t data1 = 0,data2 = 0;
    int offset = 0;
    uint16_t tile_start;
    bool b1,b2;
    //FF40 lcdc

    tilemap = (((mem.read(0xff40,0) >> 3) & 1) == 1) ? 0x9c00 : 0x9800; //put this elsewere
    tiledata = (((mem.read(0xff40,0) >> 4) & 1) == 1) ? 0x8000 : 0x8800;
    int SX,SY;
    SX = mem.read(0xFF43,0);
    SY = mem.read(0xFF42,0);
    
    for(int x=0;x<20;x++)
    {
        for(int y=0;y<18;y++)
        {
                tile_index = mem.read(tilemap + x + y * 32, 0);
                
                if(tiledata == 0x8800)
                {
                    if(tile_index < 128)
                        tile_start = 0x9000 + tile_index * 16;
                    else
                    {
                        tile_start = 0x8800 + (tile_index - 128) * 16;
                    }
                        
                }
                else
                {
                    tile_start = tiledata + tile_index * 16;
                }
                 
                
    
                for(int j=0;j<8;j++)
                {
                    data1 = mem.read(tile_start + 2 * j,0); 
                    data2 = mem.read(tile_start + 2 * j + 1,0); 
                    for(int i=0;i<8;i++)
                    {     
                        b1 = (data1 >> (7 - i)) & 1;
                        b2 = (data2 >> (7 - i)) & 1;
                        pixel_array[8*x + i - SX][abs(8*y - SY) + j] = b1 | (b2 << 1);
                    }
                }
        
          
        }
    
    }

    draw_sprites(mem);
}

void PPU::fetch_sprites(Processor::_Memory mem)
{
    std::uint16_t address = 0xFE00;

    for(int i = 0; i < 40; i++)
    {
        sprites[i].y = mem.read(address, 0);
        address++;
        sprites[i].x = mem.read(address, 0);
        address++;
        sprites[i].tile_id = mem.read(address, 0);
        address++;
        sprites[i].attributes = mem.read(address, 0);
        address++;
    }
}

void PPU::draw_sprites(Processor::_Memory mem)
{
    fetch_sprites(mem);
    
    for(int index = 0; index < 40; index++)
    {
        if(sprites[index].tile_id != 0)
        {
            
            for(int k=0;k<8;k++)
                {
                    std::uint8_t data1 = mem.read(0x8000 + sprites[index].tile_id * 16 + k, 0); 
                    std::uint8_t data2 = mem.read(0x8000 + sprites[index].tile_id * 16 + k + 1, 0); 
                    for(int m=0;m<8;m++)
                    {     
                        bool b1 = (data1 >> (7 - m)) & 1;
                        bool b2 = (data2 >> (7 - m)) & 1;
                        pixel_array[sprites[index].x + m - 8][sprites[index].y + k - 16] = b1 | (b2 << 1);
                    }
                }
        }
    }

}


void PPU::tick_ppu(int cycles, Processor::_Memory mem, sf::RenderWindow &window)
{
    
    while(cycles >= 4)
    {
        
        cycles -= 4;
        switch (PPU_mode)
        {
            case OAM_Search:
            {
                //std::cout << 'o';
                if(ppu_cycles == 0) //if the ppu was in hblank mode
                {
                   // std::cout << 'l';
                    ppu_line++;
                    mem.write(0xFF44,ppu_line,0);
                    if((mem.read(0xFF41,0) & 0x40) >> 6) 
                        if(mem.read(0xFF44,0) == mem.read(0xFF45,0))
                            mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x2, 0);
                    if((mem.read(0xFF41,0) & 0x20) >> 5) //is oam interrupt enabled
                        mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x2, 0); //fire stat
                }

                if(ppu_cycles == 80)
                {
                    PPU_mode = Pixel_Transfer;
                    ppu_cycles = -4;
                }
                break;
            }
            case Pixel_Transfer:
            {
                //std::cout <<'p';
                if(ppu_cycles == 172)
                {
                    PPU_mode = Hblank;
                    ppu_cycles = -4;
                }
                break;
            }
            case Hblank:
            {
                //std::cout <<'h';
                if(ppu_cycles == 0)
                {
                    if((mem.read(0xFF41,0) & 0x8) >> 3) //is hblank interrupt enabled
                        mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x2, 0); //fire stat
                    put_line(mem);
                }
                if(ppu_cycles == 204)
                {
                    if(ppu_line == 144)
                    {
                        ppu_line = 0;
                        PPU_mode = Vblank;
                        ppu_cycles = -4;
                        
                    }
                    else
                    {
                        
                        PPU_mode = OAM_Search;
                        ppu_cycles = -4;
                    }
                    
                }
            
                
                break;
            }
            case Vblank:
            {
                //std::cout <<'v';
                if(ppu_cycles == 0)
                {
                    //increment LY in vblank
                    
                    mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x1, 0); // vblank interrupt
                    if((mem.read(0xFF41,0) & 0x8) >> 3) //is stat vblank interrupt enabled
                        mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x2, 0); //fire stat vblank
                    //draw_sprites(mem);
                    draw_frame(mem, window);
                    //draw_window(mem);
                    handle_input(mem);
                }
                if(ppu_cycles == 4560)
                {
                    PPU_mode = OAM_Search;
                    ppu_cycles = -4;
                }
                break;
            }
            default: exit(0);
        }
            ppu_cycles += 4;
            
    }
    /*
    while(cycles >= 4)
    {
        
        if(ppu_cyles <= 80) //oam
        {
            //std::cout << 'o';
            oam_search(mem);
        }
        else if(ppu_cyles <= 172 + 80) //pixel transfer
        {
            //std::cout << 'p';
            continue;
        }
        else if(ppu_cyles <= 204 + 172)//hblank
        {
            //std::cout << 'h';
            hblank(mem);
        }
    }
    
    if(ppu_line == 145)
    {
       
        vblank(mem, window);
    }*/

}

void PPU::oam_search(Processor::_Memory mem)
{
    if(ppu_mode != 2) //if the ppu was in hblank mode
    {
        
        ppu_mode = 2;
        ppu_line++;
        
        if((mem.read(0xFF41,0) & 0x20) >> 5) //is oam interrupt enabled
            mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x2, 0); //fire stat
    }
}

void PPU::hblank(Processor::_Memory mem)
{
    if(ppu_mode != 0)
    {
        ppu_mode = 0;
        if((mem.read(0xFF41) & 0x8) >> 3) //is hblank interrupt enabled
            mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x2, 0); //fire stat
        put_line(mem);
    }
    if(ppu_cycles == 204)
        ppu_cycles = 0;
}

void PPU::vblank(Processor::_Memory mem, sf::RenderWindow &window)
{
    //std::cout << "vblank!\n";
    if(ppu_mode != 1)
    {
        mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x1, 0);
        ppu_mode = 1;
        if((mem.read(0xFF41) & 0x8) >> 3) //is vblank interrupt enabled
            mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x2, 0); //fire stat
        //draw_sprites(mem);
        draw_frame(mem, window);
        //draw_window(mem);
        handle_input(mem);
    }
}

void PPU::put_line(Processor::_Memory mem)
{
    tilemap = (((mem.read(0xff40,0) >> 3) & 1) == 1) ? 0x9c00 : 0x9800; //put this elsewere
    tiledata = (((mem.read(0xff40,0) >> 4) & 1) == 1) ? 0x8000 : 0x8800;
    uint16_t tile_start;
    for(int x = 0; x < 160; x++)
    {
        int y = ppu_line;
        int tile_x = x % 8;
        int tile_y = y % 8;
        std::uint8_t tile_index = mem.read(tilemap + x / 8 + (y / 8)* 32, 0);
                
                if(tiledata == 0x8800)
                {
                    if(tile_index < 128)
                        tile_start = 0x9000 + tile_index * 16;
                    else
                    {
                        tile_start = 0x8800 + (tile_index - 128) * 16;
                    }
                        
                }
                else
                {
                    tile_start = tiledata + tile_index * 16;
                }
        std::uint8_t data1 = mem.read(tile_start + 2 * tile_y, 0); 
        std::uint8_t data2 = mem.read(tile_start + 2 * tile_y + 1, 0);         
        bool b1 = (data1 >> (7 - tile_x)) & 1;
        bool b2 = (data2 >> (7 - tile_x)) & 1;
        pixel_array[x][ppu_line] = b1 | (b2 << 1);
       

    }    

}

void PPU::draw_frame(Processor::_Memory mem, sf::RenderWindow &window)
{
    sf::Uint8 draw_array [160 * 160 * 4] {0};
    for(int i = 0; i < 160; i++)
        {
            for(int j = 0; j < 144; j++)
            {   
                    int index = 4 * (i + 160 * j);
                    if(pixel_array[i][j] == 0)
                    {
                        draw_array[index + 0] = 155;
                        draw_array[index + 1] = 188;
                        draw_array[index + 2] = 15;
                        draw_array[index + 3] = 255;
                       
                    }
                    else if(pixel_array[i][j] == 1)
                    {
                        draw_array[index + 0] = 139;
                        draw_array[index + 1] = 172;
                        draw_array[index + 2] = 15;
                        draw_array[index + 3] = 255;
                       
                    }
                    else if(pixel_array[i][j] == 2)
                    {
                        draw_array[index + 0] = 48;
                        draw_array[index + 1] = 98;
                        draw_array[index + 2] = 48;
                        draw_array[index + 3] = 255;
                    }  
                    else if(pixel_array[i][j] == 3)
                    {
                        draw_array[index + 0] = 15;
                        draw_array[index + 1] = 56;
                        draw_array[index + 2] = 15;
                        draw_array[index + 3] = 255;
                    }
                    else
                    {
                        //std::cout << (int)pixel_array[i][j] << " ";
                    }
                    
            }
        }
        sf::Texture frame_texture;
        frame_texture.create(160,144);
        frame_texture.update(draw_array); 
        sf::Sprite frame;       
        frame.setScale(sf::Vector2f(2.0f,2.0f));
        frame.setPosition(sf::Vector2f(0.0f,0.0f));
        frame.setTexture(frame_texture);
        window.clear();
        window.draw(frame); //idk wtf to do about this ffs  
        window.display(); //this to
}








//i know this isnt the place but whatever
/********************************************** NEW MAIN LOOP
int main()
{
    //init...
    while(true)
    {
        gb_tick();
        handle_input();
    }
}
void gb_tick()
{
    emulate_cycle(); //this has timer and interrupts togheter
    tick_ppu();
}
****************************************************/



void PPU::draw_window(Processor::_Memory mem)
{
    if(((mem.read(0xff40,0) >> 7) & 1) == 1)
    {

        std::uint8_t tile_index = 0;
        std::uint8_t data1 = 0,data2 = 0;
        int offset = 0;
        uint16_t tile_start;
        bool b1,b2;
        //FF40 lcdc

        uint16_t w_tilemap = (((mem.read(0xff40,0) >> 6) & 1) == 1) ? 0x9c00 : 0x9800; //put this elsewere
        std::uint8_t wx = mem.read(0xff4b,0) - 7, wy = mem.read(0xff4a);
        tiledata = 0x8800;
        for(int x=0;x<20;x++)
        {
            for(int y=0;y<22;y++)
            {
                    tile_index = mem.read(w_tilemap + x + y * 32, 0);
                    
                    if(tiledata == 0x8800)
                    {
                        if(tile_index < 128)
                            tile_start = 0x9000 + tile_index * 16;
                        else
                        {
                            tile_start = 0x8800 + (tile_index - 128) * 16;
                        }
                            
                    }
                    else
                    {
                        tile_start = tiledata + tile_index * 16;
                    }
                    
                    
        
                    for(int j=0;j<8;j++)
                    {
                        data1 = mem.read(tile_start + 2 * j,0); 
                        data2 = mem.read(tile_start + 2 * j + 1,0); 
                        for(int i=0;i<8;i++)
                        {     
                            b1 = (data1 >> (7 - i)) & 1;
                            b2 = (data2 >> (7 - i)) & 1;
                            pixel_array[8*wx + i][abs(8*wy) + j] = b1 | (b2 << 1);
                        }
                    }
            
            
            }
        
        }
    }

}