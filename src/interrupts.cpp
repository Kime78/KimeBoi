#include "interrupts.hpp"

void handle_interrupt(Processor* cpu)
{
    if(cpu->IME) // are the interrupts enabled
    {
        std::uint8_t IE = cpu->Memory.read(0xFFFF); 
        std::uint8_t IF = cpu->Memory.read(0xFF0F);
        if(IF != 0) //If there is any interrupt requested
        {
            if(IF & 0x1) //VBlank request
            {
                if(IE & 0x1) //if VBlank interrupt is enabled
                {
                    cpu->handled = 1;
                    cpu->Memory.write(--cpu->sp,cpu->pc >> 8);
                    cpu->Memory.write(--cpu->sp,cpu->pc & 0xFF);
                    cpu->Memory.write(0xFF0F, IF & ~(1UL << 0));
                    cpu->pc = 0x0040;
                    cpu->IME = 0;
                }
            }
            if((IF & 0x2) >> 1) //LCD STAT request
            {
                if((IE & 0x2) >> 1) //if LCD STAT interrupt is enabled
                {
                    cpu->handled = 1;
                    cpu->Memory.write(--cpu->sp,cpu->pc >> 8);
                    cpu->Memory.write(--cpu->sp,cpu->pc & 0xFF);
                    cpu->Memory.write(0xFF0F, IF & ~(1UL << 1));
                    cpu->pc = 0x0048;
                    cpu->IME = 0;
                }
            }
            if((IF & 0x4) >> 2) //Timer request
            {
                if((IE & 0x4) >> 2) //if timer interrupt is enabled
                {
                    cpu->handled = 1;
                    cpu->Memory.write(--cpu->sp,cpu->pc >> 8);
                    cpu->Memory.write(--cpu->sp,cpu->pc & 0xFF);
                    cpu->Memory.write(0xFF0F, IF & ~(1UL << 2));
                    cpu->pc = 0x0050;
                    cpu->IME = 0;
                }
            }
            if((IF & 0x8) >> 3) //Serial request
            {
                if((IE & 0x8) >> 3) //if serial interrupt is enabled
                {
                    cpu->handled = 1;
                    cpu->Memory.write(--cpu->sp,cpu->pc >> 8);
                    cpu->Memory.write(--cpu->sp,cpu->pc & 0xFF);
                    cpu->Memory.write(0xFF0F, IF & ~(1UL << 3));
                    cpu->pc = 0x0058;
                    cpu->IME = 0;
                }
            }
            if((IF & 0x10) >> 4) //Joypad request
            {
                if((IE & 0x10) >> 4) //if joypad interrupt is enabled
                {
                    cpu->handled = 1;
                    cpu->Memory.write(--cpu->sp,cpu->pc >> 8);
                    cpu->Memory.write(--cpu->sp,cpu->pc & 0xFF);
                    cpu->Memory.write(0xFF0F, IF & ~(1UL << 4));
                    cpu->pc = 0x0060;
                    cpu->IME = 0;
                }
            }
        }

    }
}