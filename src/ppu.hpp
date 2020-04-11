#pragma once
#include <cstdint>
#include <array>
#include "KimeBoi.hpp"
class PPU
{
    std::array<std::uint8_t, 160 * 144 * 3> framebuffer;
    std::array<std::uint8_t, 160 * 144 * 4> framebuffer_a;
    std::uint16_t tilemap;
    std::uint16_t tiledata;
    std::uint8_t bgpalette;
    std::uint16_t off;
    public:
    std::array<std::array<std::uint8_t, 160>, 144> pixel_array{0};
    void fetch_pixel_array(Processor::_Memory mem);
    PPU()
    {
       // tilemap = (((mem.read(0xff40) >> 3) & 1) == 1) ? 0x9c00 : 0x9800; //put this elsewere
       // tiledata = (((mem.read(0xff40) >> 4) & 1) == 1) ? 0x8000 : 0x8800;
        tilemap = 0x9800;
        tiledata = 0x8000;
        bgpalette = 0b11100100;
    }


};