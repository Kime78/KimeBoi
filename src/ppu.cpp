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

void PPU::fetch_sprites(Processor::_Memory& mem)
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
bool PPU::isBitSet(std::uint8_t num, std::uint8_t bit)
{
    return (num & (1 << bit));
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
                //fetch_sprites(mem); // makes sense duh
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
    fetch_sprites(mem);
    int SX,SY;
    SX = mem.read(0xFF43, 0);
    SY = mem.read(0xFF42, 0);
    int WX,WY;
    WX = mem.read(0xFF4B, 0) - 7;
    WY = mem.read(0xFF4A, 0);
    std::uint16_t bg_tilemap = isBitSet(mem.read(0xff40, 0), 3) ? 0x9c00 : 0x9800;
    std::uint16_t w_tilemap = isBitSet(mem.read(0xff40, 0), 6) ? 0x9c00 : 0x9800;
    tiledata = isBitSet(mem.read(0xff40, 0), 4) ? 0x8000 : 0x8800;
    bool window_enabled = 0;
    int sprite_height = isBitSet(mem.read(0xff40, 0), 2) ? 16 : 8; 
    if(isBitSet(mem.read(0xff40, 0), 5))
        window_enabled = 1;

    if(window_enabled)
        tilemap = w_tilemap;
    else
        tilemap = bg_tilemap;
    
        
    uint16_t tile_start;
    int spritenr = 0;
    for(int x = 0; x < 160; x++)
    {
        if(isBitSet(mem.read(0xff40, 0), 7))
        {
            int y = ppu_line;
            int tile_x;
            int tile_y;
            std::uint8_t tile_index;
            if(window_enabled && x >= WX && y >= WY)
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
            if(isBitSet(mem.read(0xff40, 0), 0))
                pixel_array[x][ppu_line] = b1 | (b2 << 1);
            else
                pixel_array[x][ppu_line] = 0;
            
            for(int i = 0; i < 40 && spritenr <= 10; i++)
            {
                if(sprites[i].x == x && sprites[i].y == y)
                    spritenr++;
                if(sprites[i].x - 8 <= x && x < sprites[i].x)
                    if(sprites[i].y - 16 <= y && y < sprites[i].y + sprite_height - 16)
                        if(spritenr < 10)
                        {
                            if(isBitSet(mem.read(0xff40, 0), 1))
                            {
                                bool palette_id = isBitSet(sprites[i].attributes,4);
                                std::uint8_t palette = palette_id ? mem.read(0xFF49, 0) : mem.read(0xFF48, 0);
                                std::uint8_t color0 = pixel_array[x][y]; //this is "transparent"
                                std::uint8_t color1 = isBitSet(palette, 3) << 1 | isBitSet(palette, 2);
                                std::uint8_t color2 = isBitSet(palette, 5) << 1 | isBitSet(palette, 4);
                                std::uint8_t color3 = isBitSet(palette, 7) << 1 | isBitSet(palette, 6);

                                std::uint8_t sprite_tile_x = x % 8;
                                std::uint8_t sprite_tile_y = y % 8;
                                std::uint16_t sprite_tile_index = 0x8000 + sprites[i].tile_id * 16;
                                std::uint8_t sprite_data1;
                                std::uint8_t sprite_data2;
                                if(isBitSet(sprites[i].attributes, 6))
                                {
                                    sprite_data1 = mem.read(sprite_tile_index + 2 * (7 - sprite_tile_y) , 0);
                                    sprite_data2 = mem.read(sprite_tile_index + 2 * (7 - sprite_tile_y) + 1, 0);
                                }
                                else
                                {
                                    sprite_data1 = mem.read(sprite_tile_index + 2 * (sprite_tile_y), 0);
                                    sprite_data2 = mem.read(sprite_tile_index + 2 * (sprite_tile_y) + 1, 0);
                                }
                                
                                
                                bool sprite_b1, sprite_b2;
                                if(isBitSet(sprites[i].attributes, 5))
                                {
                                    sprite_b1 = (sprite_data1 >> (sprite_tile_x)) & 1;
                                    sprite_b2 = (sprite_data2 >> (sprite_tile_x)) & 1;
                                }
                                else
                                {
                                    sprite_b1 = (sprite_data1 >> (7 - sprite_tile_x)) & 1;
                                    sprite_b2 = (sprite_data2 >> (7 - sprite_tile_x)) & 1;
                                }
                                
                              
                                std::uint8_t drawn_color = sprite_b1 | (sprite_b2 << 1);
                                if(drawn_color == 0)
                                {
                                    pixel_array[x][y] = color0;
                                }
                                else if(drawn_color == 1)
                                {
                                    pixel_array[x][y] = color1;     
                                }
                                else if(drawn_color == 2)
                                {
                                    pixel_array[x][y] = color2;
                                }
                                else if(drawn_color =- 3)
                                {
                                    pixel_array[x][y] = color3;
                                }
                                else 
                                {
                                    pixel_array[x][y] = 04; //error
                                }
                                
                                
                            }        
                        }
                        
            }
        }
        else
        {
            pixel_array[x][ppu_line] = 0;
        }
        
        
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
                    else if(pixel_array[i][j] == 4)
                    {
                        draw_array[index + 0] = 255;
                        draw_array[index + 1] = 0;
                        draw_array[index + 2] = 0;
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










