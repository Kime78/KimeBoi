#pragma once
#include <cstdint>
#include <array>
#include "KimeBoi.hpp"
class PPU
{
    struct Sprite
    {
        std::uint8_t x;
        std::uint8_t y;
        std::uint8_t tile_id;
        std::uint8_t attributes;
    };

    std::uint16_t tilemap;
    std::uint16_t tiledata;
    std::uint8_t bgpalette;
    std::uint16_t off;

    public:
    std::array<std::array<std::uint8_t, 160>, 144> pixel_array {0};
    std::array<Sprite,40> sprites;
    void fetch_pixel_array(Processor::_Memory mem);
    void fetch_sprites(Processor::_Memory mem);
    void draw_sprites(Processor::_Memory mem);
    PPU()
    {
        tilemap = 0x9800;
        tiledata = 0x8000;
        //bgpalette = 0b11100100;
        bgpalette = 0b00011011;
    }


};