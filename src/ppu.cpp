#include "ppu.hpp"
#include <iostream>
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
    std::int8_t SX,SY;
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
        
            offset += 1;
        }
    
    }
    off--;
    draw_sprites(mem);
}

void PPU::fetch_sprites(Processor::_Memory mem)
{
    std::uint16_t address = 0xFE00;

    for(int i = 0; i < 40; i++)
    {
        sprites[i].x = mem.read(address, 0);
        address++;
        sprites[i].y = mem.read(address, 0);
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
                        pixel_array[sprites[index].y + m - 8][sprites[index].x + k - 16] = b1 | (b2 << 1);
                    }
                }
        }
    }

}
/************************************************* NEW PPU UWU
int ppu_cyles, ppu_mode, ppu_line;
void tick_ppu(int cycles, Processor::_Memory mem)
{
    while(cycles >= 4)
    {
        ppu_cyles += 4;
        if(ppu_cyles <= 60)
        {
            oam_search(mem);
        }
        else if(ppu_cyles <= 172)
        {
            continue;
        }
        else if(ppu_cyles <= 204)
        {
            hblank(mem);
        }
    }

    if(ppu_line == 145)
    {
        vblank(mem);
    }
    
}

void oam_search(Processor::_Memory mem)
{
    if(ppu_mode != 2) //if the ppu was in hblank mode
    {
        ppu_mode = 2;
        ppu_line++;
        if((mem.read(0xFF41) & 0x20) >> 5) //is oam interrupt enabled
            mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x1, 0); //fire stat
    }
}

void hblank(Processor::_Memory mem)
{
    if(ppu_mode != 0)
    {
        ppu_mode = 0;
        if((mem.read(0xFF41) & 0x8) >> 3) //is hblank interrupt enabled
            mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x1, 0); //fire stat
        put_line(mem);
    }

}

void vblank(Processor::_Memory mem)
{
    if(ppu_mode != 1)
    {
        ppu_mode = 1;
        if((mem.read(0xFF41) & 0x8) >> 3) //is vblank interrupt enabled
            mem.write(0xFF0F, mem.read(0xFF0F, 0) | 0x1, 0); //fire stat

        draw_frame(mem);
    }
}

void put_line(Processor::_Memory mem)
{
    int x = ppu_line / 20;
    std::uint16_t tilemap = (((mem.read(0xff40,0) >> 3) & 1) == 1) ? 0x9c00 : 0x9800; //put this elsewere
    std::uint16_t tiledata = (((mem.read(0xff40,0) >> 4) & 1) == 1) ? 0x8000 : 0x8800;
    uint16_t tile_index,tile_start;
    std::uint8_t data1 = 0,data2 = 0;
    bool b1,b2;
    std::int8_t SX,SY;
    SX = mem.read(0xFF43,0);
    SY = mem.read(0xFF42,0);
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

sf::Sprite draw_frame(Processor::_Memory mem)
{
    for(int i = 0; i < 160; i++)
        {
            for(int j = 0; j < 160; j++)
            {
                 int index = 4 * (i + 160 * j);
                    if(pixel_array[i][j] == 0)
                    {
                        draw_array[index + 0] = 155;
                        draw_array[index + 1] = 188;
                        draw_array[index + 2] = 15;
                        draw_array[index + 3] = 255;
                       
                    }
                    
                    if(pixel_array[i][j] == 1)
                    {
                        draw_array[index + 0] = 139;
                        draw_array[index + 1] = 172;
                        draw_array[index + 2] = 15;
                        draw_array[index + 3] = 255;
                       
                    }
                    if(pixel_array[i][j] == 2)
                    {
                        draw_array[index + 0] = 48;
                        draw_array[index + 1] = 98;
                        draw_array[index + 2] = 48;
                        draw_array[index + 3] = 255;
                    }  
                    if(pixel_array[i][j] == 3)
                    {
                        draw_array[index + 0] = 15;
                        draw_array[index + 1] = 56;
                        draw_array[index + 2] = 15;
                        draw_array[index + 3] = 255;
                    }
            }
        }
        sf::Texture frame_texture;
        frame_texture.update(draw_array);        
        frame.setTexture(frame_texture);

        window.draw(test_spr); //idk wtf to do about this ffs  
        window.display(); //this to
}
*********************************************************************/







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