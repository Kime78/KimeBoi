#include "timer.hpp"

void increment_timer(Processor* cpu, int cycles)
{
    //FUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUCK

    //timer
    std::uint8_t TIMA = cpu->Memory.read(0xFF05, 0);
    std::uint8_t TAC = cpu->Memory.read(0xFF07, 0);
    
    if(TAC == 0b00000100)
    {
        cpu->Memory.cycle_count += cycles;
        while(cpu->Memory.cycle_count >= 1024)
        {
            if(TIMA == 0xff)
            {
                cpu->Memory.write(0xFF0F, cpu->Memory.read(0xFF0F,0) | 0x4, 0); 
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF06,0), 0);
            }
            else
            {
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05,0) + 1, 0);
            }
            cpu->Memory.cycle_count -= 1024;
        }
          
    }

    if(TAC == 0b00000101)
    {
        cpu->Memory.cycle_count += cycles;
        while(cpu->Memory.cycle_count >= 16)
        {
            if(TIMA == 0xff)
            {
                cpu->Memory.write(0xFF0F, cpu->Memory.read(0xFF0F,0) | 0x4, 0); 
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF06,0), 0);
            }
            else
            {
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05,0) + 1, 0);
            }
            cpu->Memory.cycle_count -= 16;
        }
        
          
    }

    if(TAC == 0b00000110)
    {
        cpu->Memory.cycle_count += cycles;
        while(cpu->Memory.cycle_count >= 64)
        {
            if(TIMA == 0xff)
            {
                cpu->Memory.write(0xFF0F, cpu->Memory.read(0xFF0F,0) | 0x4, 0); 
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF06,0), 0);
            }
            else
            {
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05,0) + 1, 0);
            }
            cpu->Memory.cycle_count -= 64;
        }
          
    }
    if(TAC == 0b00000111)
    {
        cpu->Memory.cycle_count += cycles;
        while(cpu->Memory.cycle_count >= 256)
        {
            if(TIMA == 0xff)
            {
                cpu->Memory.write(0xFF0F, cpu->Memory.read(0xFF0F,0) | 0x4, 0); 
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF06,0), 0);
            }
            else
            {
                cpu->Memory.write(0xFF05, cpu->Memory.read(0xFF05,0) + 1, 0);
            }
            cpu->Memory.cycle_count -= 256;
        }
            
    }

   
}