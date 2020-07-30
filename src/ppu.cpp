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


void PPU::tick_ppu(int cycles, Processor::_Memory& mem, sf::RenderWindow &window)
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
                    LY++;
                    mem.write(0xFF44,LY,0);
                  
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
                        //LY = 0; //this should be at the end of vblank
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
                if(ppu_cycles % 456)
                {
                    LY++;
                    mem.write(0xFF44,LY,0);
                }
                //std::cout <<'v';
                if(ppu_cycles == 0)
                {
                    
                    mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x1, 0); // vblank interrupt
                    
                    if((mem.read(0xFF41,0) & 0x10) >> 4) //is stat vblank interrupt enabled
                        mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x2, 0); //fire stat vblank
                
                    draw_frame(mem, window);
                    handle_input(mem);
                }
                if(ppu_cycles == 4560)
                {
                    LY = 0;
                    PPU_mode = OAM_Search;
                    ppu_cycles = -4;
                }
                break;
            }
            default: exit(0);
        }
            ppu_cycles += 4;
            
    }

}

void PPU::put_line(Processor::_Memory& mem)
{
    int SX,SY;
    SX = mem.read(0xFF43,0);
    SY = mem.read(0xFF42,0);
    int WX,WY;
    WX = mem.read(0xFF4B, 0) - 7;
    WY = mem.read(0xFF4A, 0);
    std::uint16_t bg_tilemap = (((mem.read(0xff40,0) >> 3) & 1) == 1) ? 0x9c00 : 0x9800;
    std::uint16_t w_tilemap = (((mem.read(0xff40,0) >> 6) & 1) == 1) ? 0x9c00 : 0x9800;
    tiledata = (((mem.read(0xff40,0) >> 4) & 1) == 1) ? 0x8000 : 0x8800;
    bool window_enabled = 0;
    if((((mem.read(0xff40,0) & 0x20) >> 5)))
        window_enabled = 1;

    if(window_enabled)
        tilemap = w_tilemap;
    else
        tilemap = bg_tilemap;
    
        
    uint16_t tile_start;

    for(int x = 0; x < 160; x++)
    {
        int y = ppu_line;
        int tile_x;
        int tile_y;
        std::uint8_t tile_index;
        if(window_enabled)
        {
            tile_x = ((x + WX) % 256) % 8;
            tile_y = ((y + WY) % 256) % 8;
            tile_index = mem.read(tilemap + (x + WX) / 8 + ((y + WY) / 8) * 32, 0);
        }
        else
        {
            tile_x = ((x + SX) % 256) % 8;
            tile_y = ((y + SY) % 256) % 8;
            tile_index = mem.read(tilemap + ((x + SX) % 256) / 8 + (((y + SY) % 256) / 8) * 32, 0);
        }
        
        
                
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

void PPU::draw_frame(Processor::_Memory& mem, sf::RenderWindow &window)
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