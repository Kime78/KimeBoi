#include "timer.hpp"

void increment_timer(Processor* cpu)
{
    //FUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUCK
    //Ok this needs cpu cycles which are the the 4 cycles = 1 nop 
    //this means that i have to fully implement timing? NOT NOW
    //this is coming way way sooner than excepted

    //The basic idea is 1 memory access = 4 T-cycles

    //timer
    std::uint8_t TIMA = cpu->Memory.read(0xFF05, 0);
    std::uint8_t TAC = cpu->Memory.read(0xFF07, 0);

   /* if(cpu->Memory.cycle_count % 256)
    {
        cpu->Memory.write(0xFF04, cpu->Memory.read(0xFF04) + 1, 0);
    }*/

    if(TAC == 0b00000100)
    {
        if(cpu->Memory.cycle_count % 1024 == 0)
        {
            if(TIMA == 0xff)
            {
                cpu->Memory.write(0xFF0F, cpu->Memory.read(0xFF0F) | 0x4, 0); //is this how u request a timer interrupt
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF06), 0); //loads TMA in TIMA
            }
            else
            {
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05) + 1, 0);
            }
        }
          
    }

    if(TAC == 0b00000101)
    {
        if(cpu->Memory.cycle_count % 16 == 0)
        {
            if(TIMA == 0xff)
            {
                cpu->Memory.write(0xFF0F, cpu->Memory.read(0xFF0F) | 0x4, 0); //is this how u request a timer interrupt
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF06), 0); //loads TMA in TIMA
            }
            else
            {
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05) + 1, 0);
            }
        }
          
    }

    if(TAC == 0b00000110)
    {
        if(cpu->Memory.cycle_count % 64 == 0)
        {
            if(TIMA == 0xff)
            {
                cpu->Memory.write(0xFF0F, cpu->Memory.read(0xFF0F) | 0x4, 0); //is this how u request a timer interrupt
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF06), 0); //loads TMA in TIMA
            }
            else
            {
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05) + 1, 0);
            }
        }
          
    }
    if(TAC == 0b00000111)
    {
        if(cpu->Memory.cycle_count % 256 == 0)
        {
            if(TIMA == 0xff)
            {
                cpu->Memory.write(0xFF0F, cpu->Memory.read(0xFF0F) | 0x4, 0); //is this how u request a timer interrupt
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF06)); //loads TMA in TIMA
            }
            else
            {
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05) + 1, 0);
            }
        }
            
    }
        
/*
 if((std::uint8_t)(TIMA + 1) == 0) 
    {
        cpu->Memory.write(0xFF0F, cpu->Memory.read(0xFF0F) | 0x4); //is this how u request a timer interrupt
        cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF06)); //loads TMA in TIMA
    }
    */
   


}