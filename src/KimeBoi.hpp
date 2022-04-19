#ifndef KIMEBOI_HPP_INCLUDED
#define KIMEBOI_HPP_INCLUDED
#include <stdint.h>
#include <string>
#include <vector>

class Processor {

public:
  class _Memory {
  public:
    int rom_offset = 0;
    int ram_offset = 0;
    bool rom_mode = 1;
    bool ram_enabled = 0;
    std::vector<unsigned char> rom;
    bool boot_enabled = 1;
    unsigned char ram[1048576];
    unsigned char boot[0x100];
    std::uint64_t cycle_count{0};
    bool keystates[8]{0};
    // joypad be like
    // start  select a b up down left right
    //   0       1   2 3 4    5   6     7
    enum Keys {
      START = 0,
      SELECT = 1,
      A = 2,
      B = 3,
      UP = 4,
      DOWN = 5,
      LEFT = 6,
      RIGHT = 7,
    };
    void write(unsigned short address, unsigned char value, bool cycles = 1);
    int cycles_taken{0};
    unsigned char read(unsigned short address, bool cycles = 1);

  } Memory;
  std::string to_hex(int x) {
    std::string res = "";
    char hex_string[20];

    sprintf(hex_string, "%X", x);
    res = hex_string;
    return res;
  }
  // private:
  struct _regs {
    unsigned char A;
    unsigned char B;
    unsigned char C;
    unsigned char D;
    unsigned char E;
    unsigned char F;
    unsigned char H;
    unsigned char L;
  } Registers;

  struct _flag {
    unsigned char Z; // ZERO FLAG; set to 1 if current operation results in
                     // Zero, or two values match on a CMP operation
    unsigned char N; // SUBTRACT FLAG; set to 1 if a subtraction was performed
    unsigned char H; // HALF CARRY FLAG; set to 1 if a carry occured from the
                     // lower nibble in the last operation
    unsigned char C; // CARRY FLAG; set to 1 if a carry occured in the last
                     // operation or if A is the smaller value on CP instruction
    bool HALT;
  } Flags;

  // unsigned char memory[8192];
  unsigned short pc;
  unsigned short sp;
  unsigned short stack[128];
  bool IME = 0;
  bool handled = 0;
  int div_cycles = 0;

  // unsigned char gameMemory[0x100];
public:
  void loadGame(std::string path);
  int emulateCycle();
  void initialise();
  void initialiseOpenGL();
};

// void handle_inputs(Processor*);

#endif // KIMEBOI_HPP_INCLUDED
