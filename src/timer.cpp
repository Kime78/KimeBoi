#include "timer.hpp"

void increment_timer(Processor* cpu)
{
    //FUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUCK
    //Ok this needs cpu cycles which are the the 4 cycles = 1 nop 
    //this means that i have to fully implement timing? NOT NOW
    //this is coming way way sooner than excepted

    //The basic idea is 1 memory access = 4 T-cycles

    //timer
    std::uint8_t TIMA = cpu->Memory.read(0xFF05);
    std::uint8_t TAC = cpu->Memory.read(0xFF07);


    if(TAC == 0b00000100)
        if(cpu->Memory.cycle_count % 1024 == 0)
            cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05) + 1); 

    if(TAC == 0b00000101)
        if(cpu->Memory.cycle_count % 16 == 0)
            cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05) + 1);  

    if(TAC == 0b00000110)
        if(cpu->Memory.cycle_count % 64 == 0)
            cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05) + 1);

    if(TAC == 0b00000111)
        if(cpu->Memory.cycle_count % 256 == 0)
            cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05) + 1);


    if((std::uint8_t)(TIMA + 1) == 0) //this need to check overflow
    {
        cpu->Memory.write(0xFF0F, cpu->Memory.read(0xFF0F) | 0x4); 
        cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF06)); //loads TMA in TIMA
    }


}