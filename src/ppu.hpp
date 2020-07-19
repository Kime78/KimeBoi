#pragma once
#include <cstdint>
#include <array>
#include "KimeBoi.hpp"
#include <SFML/Graphics.hpp>
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
    std::array<std::array<std::uint8_t, 144>, 160> pixel_array {0};
    std::array<Sprite,40> sprites;
    void fetch_pixel_array(Processor::_Memory mem);
    void fetch_sprites(Processor::_Memory mem);
    void draw_sprites(Processor::_Memory mem);
    void draw_window(Processor::_Memory mem);
    PPU()
    {
        tilemap = 0x9800;
        tiledata = 0x8000;
        //bgpalette = 0b11100100;
        bgpalette = 0b00011011;
    }

    void tick_ppu(int cycles, Processor::_Memory mem, sf::RenderWindow &window);
    void oam_search(Processor::_Memory mem);
    void hblank(Processor::_Memory mem);
    void vblank(Processor::_Memory mem, sf::RenderWindow &window);
    void put_line(Processor::_Memory mem);
    void draw_frame(Processor::_Memory mem, sf::RenderWindow &window);
    int ppu_cycles {0}, ppu_mode, ppu_line {0};
    bool in_vblank = 0;

    enum PPU_MODES
    {
        Vblank,
        Hblank,
        OAM_Search,
        Pixel_Transfer,
    }PPU_mode = OAM_Search;
};