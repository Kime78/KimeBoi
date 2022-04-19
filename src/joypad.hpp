#pragma once
#include "KimeBoi.hpp"
#include <SFML/Graphics.hpp>

void handle_input(Processor::_Memory &mem) {
  // mem.write(0xff00,0xff,0);

  // joypad be like
  // start  select a b up down left right
  //   0       1   2 3 4    5   6     7

  for (int i = 0; i < 8; i++)
    mem.keystates[i] = 0;
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
    mem.keystates[mem.LEFT] = 1;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
    mem.keystates[mem.RIGHT] = 1;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
    mem.keystates[mem.UP] = 1;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
    mem.keystates[mem.DOWN] = 1;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) // start
  {
    mem.keystates[mem.START] = 1;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSlash)) // select
  {
    mem.keystates[mem.SELECT] = 1;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Slash)) // A
  {
    mem.keystates[mem.A] = 1;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Quote)) // B
  {
    mem.keystates[mem.B] = 1;
  }

  // for(int i = 0; i < 8; i++)
  // std::cout << mem.keystates[i];
  // std::cout << '\n';
}