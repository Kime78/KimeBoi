#ifndef KIMEBOI_HPP_INCLUDED
#define KIMEBOI_HPP_INCLUDED
#include <string>
#include <vector>
#include <stdint.h>

class Processor
{

    public: class _Memory
    {
    public:
        std::vector<unsigned char> rom;
        bool boot_enabled = 1;
        unsigned char ram[32768];
        unsigned char boot[0x100];
        void write(unsigned short address, unsigned char value);
        unsigned char read(unsigned short address);
    }Memory;
    std::string to_hex(int x)
    {
        std::string res = "";
        char hex_string[20];
        
        sprintf(hex_string,"%X",x);
        res = hex_string;
        return res;

    }
    //private:
    struct _regs
    {
        unsigned char A;
        unsigned char B;
        unsigned char C;
        unsigned char D;
        unsigned char E;
        unsigned char F;
        unsigned char H;
        unsigned char L;
    }Registers;

    struct _flag
    {
        unsigned char Z; // ZERO FLAG; set to 1 if current operation results in Zero, or two values match on a CMP operation
        unsigned char N; // SUBTRACT FLAG; set to 1 if a subtraction was performed
        unsigned char H; // HALF CARRY FLAG; set to 1 if a carry occured from the lower nibble in the last operation
        unsigned char C; // CARRY FLAG; set to 1 if a carry occured in the last operation or if A is the smaller value on CP instruction
        bool HALT;
    }Flags;

    //unsigned char memory[8192];
    unsigned short pc;
    unsigned short sp;
    unsigned short stack[128];
    bool IME = 0;
    bool handled = 0;
  
    //unsigned char gameMemory[0x100];
public:

    void loadGame(std::string path);
    void emulateCycle(std::string& output);
    void initialise();
    void initialiseOpenGL();


};



#endif // KIMEBOI_HPP_INCLUDED
