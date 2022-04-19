#include "KimeBoi.hpp"
#include "ppu.hpp"
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <string>

std::ofstream fout;

int main(int argc, char **args) {

  Processor game;
  fout.open("debug.out");
  game.initialise();
  PPU ppu;
  sf::RenderWindow window(sf::VideoMode(320, 288), "KimeBoi");
  if (argc < 2) {
    std::cout << args[1];
    std::cout << "Booting test.gb\n";
    FILE *file = fopen("/home/kime/Documents/Projects/KimeBoi/boot.gb", "rb");
    int pos = 0;
    while (fread(&game.Memory.boot[pos], 1, 1, file)) {
      pos++;
    }
    game.loadGame("/home/kime/Documents/Projects/KimeBoi/dmg-acid2.gb");
  } else {
    std::cout << args[0];
    std::cout << "Booting " << args[1] << "\n";
    FILE *file = fopen("./boot.gb", "rb");
    int pos = 0;
    while (fread(&game.Memory.boot[pos], 1, 1, file)) {
      pos++;
    }
    game.loadGame(args[1]);
  }

  sf::Texture test_tex;
  sf::Uint8 draw_array[160 * 144 * 4]{0};
  sf::Sprite test_spr;
  test_tex.create(160, 144);
  test_spr.setScale(sf::Vector2f(2.0f, 2.0f));
  test_spr.setPosition(sf::Vector2f(0.0f, 0.0f));
  window.setFramerateLimit(60);
  while (window.isOpen()) {
    int cycles_taken = 0;
    cycles_taken = game.emulateCycle(); // this has timer and interrupts
                                        // togheter
    ppu.tick_ppu(cycles_taken, game.Memory, window);
    bool debug = 0;
    if (debug) {
      fout //<< '[' << game.to_hex(game.Memory.rom_offset / 0x4000) << ']'
          << ' ' << "PC: " << game.to_hex(game.pc) << ' '
          << game.to_hex(game.Memory.read(game.pc, 0)) << ' '
          << game.to_hex(game.Memory.read(game.pc + 1, 0)) << ' '
          << game.to_hex(game.Memory.read(game.pc + 2, 0)) << '\t' << "AF: "
          << game.to_hex((int)(game.Registers.A << 8 || game.Registers.F))
          << ' '
          //<< "BC: " << game.to_hex(game.Registers.B << 8 || game.Registers.C)
          //<< ' '
          //<< "DE: " << game.to_hex(game.Registers.D << 8 || game.Registers.E)
          //<< ' '
          //<< "HL: " << game.to_hex(game.Registers.H << 8 || game.Registers.L)
          //<< ' '
          << '\n';
    }
  }

  return 0;
}
