#include "KimeBoi.hpp"
#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <fstream>
#include "interrupts.hpp"
#include "timer.hpp"

bool tmp = 0;
int t = 0;
unsigned char Processor::_Memory::read(unsigned short address, bool cycles)
{
	if(cycles)
		cycle_count += 4;
	
	if(boot_enabled && address < 0x100)
		return boot[address];
	if (address < 0x4000)
		return rom[address];
	if(address < 0x8000)
		return rom[address + rom_offset];	
	return ram[address - 0x8000];
}

void Processor::_Memory::write(unsigned short address, unsigned char value, bool cycles)
{
	if(cycles)
		cycle_count += 4;

	if(address >= 0x2000 && address <= 0x4000) //rom switching
	{
		if(rom_mode)
			rom_offset = (value - 1) * 0x4000;
		return;
	}
	if(address >= 0x6000 && address <= 0x7FFF)
	{
		return;
	}
	if (address < 0x8000)
	{
		std::cout << "Illegal writting at: " << address << std::endl;
		return;
	}
	
	ram[address - 0x8000] = value;
}

void Processor::loadGame(std::string path)
{
	FILE *file = fopen(path.c_str(), "rb");
	int pos = 0;
	
	while (fread(&Memory.rom[pos], 1, 1, file))
	{
		//std::cout << std::hex << (int)Memory.rom[pos] << " ";
		pos++;
	}
	std::cout << std::endl;
	//fclose(file);
}
bool isBitSet(std::uint8_t num, std::uint8_t bit)
{
    return (num & (1 << bit));
}
void setBit (std::uint8_t& num, std::uint8_t bit, bool status)
{
    if (isBitSet(num, bit) != status) 
        num ^= (1 << bit);
}
void Processor::initialise()
{
	//Memory.ram[0xFF00 - 0x8000] = 0xFF;
	Memory.cycle_count = 0;
	pc = 0;
	Registers.A = 0x00;
	Registers.B = 0x00;
	Registers.C = 0x00;
	Registers.D = 0x00;
	Registers.E = 0x00;
	Registers.F = 0x00;
	Registers.H = 0x00;
	Registers.L = 0x00;
	Flags.C = 0;
	Flags.H = 0;
	Flags.HALT = 0;
	Flags.N = 0;
	Flags.Z = 0;
	sp = 0xFFFF;
	Memory.rom.resize(65535);
	for(int i = 0; i < 32768; i++)
		Memory.rom[i] = 0; 
	for(int i = 0; i < 32768; i++)
		Memory.ram[i] = 0; 
	//Memory.write(0xFF44, 144);
	for(uint16_t p = 0xA000; p <= 0xBFFF; p++)
		Memory.write(p,0xFF,0);	
}

void Processor::emulateCycle(std::string &output)
{
	//Memory.ram[0xFF00 - 0x8000] = 0xFF; //faking joypad
	increment_timer(this);
	handle_interrupt(this);
	Registers.F = Registers.F & 0xF0;
	setBit(Registers.F,7,Flags.Z);
	setBit(Registers.F,6,Flags.N);
	setBit(Registers.F,5,Flags.H);
	setBit(Registers.F,4,Flags.C);

	if(!Memory.boot_enabled)
	{
		
	//	std::cout <<(int)Memory.read(0xFF44) << '\n';
	//	std::cout << pc << " ";
	//	std::cout <<(int)Memory.read(pc) << " " <<(int)Memory.read(pc+1) << " " <<(int)Memory.read(pc+2) <<   std::hex;
	//	std::cout << '\n';
		output = to_hex(pc) + " " + to_hex((int)Memory.read(pc)) + " " + to_hex((int)Memory.read(pc+1)) + " " + to_hex((int)Memory.read(pc+2));
	}
	//    blarggs test - serial output
	/*if (Memory.read(0xff02) == 0x81) {
		char c = Memory.read(0xff01);
		printf("%c", c);
		Memory.write(0xff02, 0x0);
	}*/

 	if(tmp != IME && handled == 0) // this is triggered by the handling
	{
		
		t++;
		if(t == 2)
		{
			t = 0;
			IME = tmp;
		}

	}
	switch (Memory.read(pc)) //+4
	{
		///example of combined registers
		///BC = B << 8 | C

		///upper = pc + 2
		///lower = pc + 1

		//(((a&0xf) + (b&0xf)) & 0x10) == 0x10 //3-4 carry
		//(((a&0xff) + (b&0xff)) & 0x100) == 0x10 //7-8 carry
		//(((a&0xfff) + (b&0xfff)) & 0x1000) == 0x10 //11-12 carry
		//(((a&0xffff) + (b&0xffff)) & 0x10000) == 0x10 //15-16 carry

		///TO-DO	
		
		case 0x00: //good
		{
			pc++;
			break;
		}
		case 0x01: //fixed
		{
			Registers.C = Memory.read(pc + 1);
			Registers.B = Memory.read(pc + 2);
			pc += 3;
			break;
		}
		case 0x02: //good
		{
			Memory.cycle_count += 4;
			Memory.write(Registers.B << 8 | Registers.C, Registers.A);
			pc++;
			break;
		}
		case 0x03: //good
		{
			unsigned short res = (Registers.B << 8 | Registers.C) + 1;
			Registers.B = res >> 8;
			Registers.C = res & 0xFF;
			pc++;
			break;
		}
		case 0x04: //good
		{
			Flags.N = 0;
			
			if((((Registers.B & 0xf) + (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			Registers.B++;
			if(Registers.B == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			
			pc++;
			break;	
		}
		case 0x05: //good
		{
			Flags.N = 1;
			//other flags
			if(Registers.B - 1 == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.B & 0xf) - (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
				
			Registers.B--;
			pc++;
			break;	
		}
		case 0x06: //good
		{
			Registers.B = Memory.read(pc + 1);
			pc += 2;
			break;
		}
		case 0x07: //maybe 
		{
			bool msb = Registers.A >> 7;
			Flags.C = msb;
			Registers.A <<= 1;
			Registers.A |= msb;
			Flags.Z = 0;
			Flags.N = 0;
			Flags.H = 0;
			pc++;
			break;		
		}
		case 0x08:
		{
			std::uint16_t add,u16;
			u16 = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
			//add = Memory.read(u16);
			Memory.write(u16,sp & 0xFF);
			Memory.write(u16 + 1,sp >> 8);
			pc += 3;
			break;
		}
		case 0x09: //done?
		{
			Memory.cycle_count += 4;
			std::uint16_t bc = Registers.B << 8 | Registers.C;
			std::uint16_t hl = Registers.H << 8 | Registers.L;
			std::uint16_t res = hl + bc;
			Flags.N = 0;
			//(((a&0xfff) + (b&0xfff)) & 0x1000) == 0x10 //11-12 carry
			//(((a&0xffff) + (b&0xffff)) & 0x10000) == 0x10 //15-16 carry

			if((((hl&0xfff) + (bc&0xfff)) & 0x1000) == 0x1000)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((hl&0xffff) + (bc&0xffff)) & 0x10000) == 0x10000)
				Flags.C = 1;
			else
				Flags.C = 0;	
			//implement flags ....
			hl = res;
			Registers.H = res >> 8;
			Registers.L = res & 0xFF;
			pc++;
			break;
		}
		case 0x0A: //good
		{
			Registers.A = Memory.read(Registers.B << 8 | Registers.C);
			pc++;
			break; 
		}
		case 0x0B: //good
		{
			Memory.cycle_count += 4;
			std::uint16_t res = Registers.B << 8 | Registers.C;
			res--;
			Registers.B = res >> 8;
			Registers.C = res & 0xFF;
			pc++;
			break;
		}
		case 0x0C: //good
		{
			Flags.N = 0;
			
			if((((Registers.C & 0xf) + (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			Registers.C++;
			if(Registers.C == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			
			pc++;
			break;	
		}
		case 0x0D: //good
		{
			
			Flags.N = 1;
			//other flags
			if(Registers.C - 1 == 0)
				Flags.Z = 1;
			else
			{
				Flags.Z = 0;
			}
				
			if((((Registers.C & 0xf) - (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
			{
				Flags.H = 0;
			}
				
			Registers.C--;
			pc++;
			break;	
		}
		
		case 0x0E: //good
		{
			Registers.C = Memory.read(pc + 1);
			pc += 2;
			break;
		}
		case 0x0F: 
		{
			Flags.H = 0;
			Flags.N = 0;
			bool lsb = Registers.A & 0x1;
			Flags.C = lsb;
			Registers.A >>= 1;
			Registers.A = lsb << 7 | Registers.A;
			Flags.Z = 0;
			pc++;
			break;
		}
		case 0x10: //not now
		{
			//input shit
			pc += 1;
			break;
		}
		case 0x11: //good
		{
			//Registers.D = memory[pc + 2];
			//Registers.E = memory[pc + 1];
			Registers.D = Memory.read(pc + 2);
			Registers.E = Memory.read(pc + 1);
			pc += 3;
			break;
		}

		case 0x12: //good
		{

			//std::cout << (int)(Registers.D << 8 | Registers.E) << ": " <<(int) Registers.A << std::endl;
			Memory.write(Registers.D << 8 | Registers.E, Registers.A);
			//memory[(std::uint8_t)Registers.D << 8 | Registers.E] = Registers.A;
			pc++;
			break;
		}

		case 0x13: //good
		{
			Memory.cycle_count += 4;
			std::uint16_t tmp = Registers.D << 8 | Registers.E;
			tmp++;
			Registers.D = tmp >> 8;
			Registers.E = tmp & 0xFF; 
			
			pc++;
			break;
		}
		case 0x14: //good
		{
			Flags.N = 0;
			
			if((((Registers.D & 0xf) + (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			Registers.D++;
			if(Registers.D == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			
			pc++;
			break;	
		}
		case 0x15: //good
		{
			Flags.N = 1;
			//other flags
			if(Registers.D - 1 == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.D & 0xf) - (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
				
			Registers.D--;
			pc++;
			break;
		}
		case 0x16: //good
		{
			Registers.D = Memory.read(pc + 1);
			pc += 2;
			break;
		}
		case 0x17: //maybe
		{
			bool msb = (Registers.A >> 7) & 1;
			bool nlsb = Flags.C;
			Flags.C = msb;
			Registers.A = (Registers.A << 1) | nlsb;
			Flags.Z = 0;
			Flags.N = 0;
			Flags.H = 0;
			pc++;
			break;	
		}
		case 0x18: //good
		{
			Memory.cycle_count += 4;
			std::int8_t add = Memory.read(pc + 1);
			pc += 2;
			pc = pc + add;
			break;
			
		} 
		case 0x19:
		{
			Memory.cycle_count += 4;
			Flags.N = 0;
			std::uint16_t hl,de,res;
			hl = Registers.H << 8 | Registers.L;
			de = Registers.D << 8 | Registers.E;
			res = de + hl;

			if((((hl & 0xfff) + (de &0xfff)) & 0x1000) == 0x1000)
				Flags.H = 1;
			else
				Flags.H = 0;

			if((((hl&0xffff) + (de & 0xffff)) & 0x10000) == 0x10000)
				Flags.C = 1;
			else
				Flags.C = 0;	
			//implement flags ....
			//hl = res;
			Registers.H = res >> 8;
			Registers.L = res & 0xFF;
			pc++;
			break;	

		}
		case 0x1A: //good
		{
			Registers.A = Memory.read(Registers.D << 8 | Registers.E);
			pc++;
			break;
		}
		case 0x1B:
		{
			Memory.cycle_count += 4;
			std::uint16_t res = Registers.D << 8 | Registers.E;
			res--;
			Registers.D = res >> 8;
			Registers.E = res & 0xFF;
			pc++;
			break;
		}
        case 0x1C: //good
		{
			Flags.N = 0;
			
			if((((Registers.E & 0xf) + (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			Registers.E++;
			if(Registers.E == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			pc++;
			break;	
		}
		case 0x1D: //good
		{
			Flags.N = 1;
			//other flags
			if(Registers.E - 1 == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.E & 0xf) - (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
				
			Registers.E--;
			pc++;
			break;
		}
		case 0x1E: //good
		{
			Registers.E = Memory.read(pc + 1);
			pc += 2;
			break;
		}
		case 0x1F: //maybe
		{
			Flags.H = 0;
			Flags.N = 0;
			Flags.Z = 0;

			bool msb = Flags.C;
			Flags.C = Registers.A & 0x1;
			Registers.A >>= 1;
			Registers.A = msb << 7 | Registers.A;
			pc++;
			break;
		}
		case 0x20: //good
		{
			Memory.cycle_count += 4;
			if (Flags.Z == 0)
			{
				Memory.cycle_count += 4;
				std::int8_t tmp = Memory.read(pc + 1);
				pc += 2;
				pc = pc + tmp;
				//pc = (signed short)memory[pc + 1] + pc;
				break;
			}
			
			pc += 2;
			break;
		}

		case 0x21: //not sure
		{
			Registers.H = Memory.read(pc + 2);
			Registers.L = Memory.read(pc + 1);
			pc += 3;
			break;
		}
		case 0x22: //good
		{

			std::uint16_t res = Registers.H << 8 | Registers.L;
			Memory.write(Registers.H << 8 | Registers.L,Registers.A);
			res++;
			Registers.H = res >> 8;
			Registers.L = res & 0xFF;
			pc++;
			break;
		}
		case 0x23: //good
		{
			Memory.cycle_count += 4;
			std::uint16_t tmp = Registers.H << 8 | Registers.L;
			tmp++;
			Registers.H = tmp >> 8;
			Registers.L = tmp & 0xFF;
			pc++;
			break;
		}
		case 0x24: //good
		{
			Flags.N = 0;
			
			if((((Registers.H & 0xf) + (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			Registers.H++;
			if(Registers.H == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			
			pc++;
			break;	
		}
		case 0x25:
		{
			Flags.N = 1;
			//other flags
			if(Registers.H - 1 == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.H & 0xf) - (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
				
			Registers.H--;
			pc++;
			break;
		}
		case 0x26: //good
		{
			Registers.H = Memory.read(pc + 1);
			pc += 2;
			break;
		}
		case 0x27:
		{
			std::uint8_t reg = Registers.A;

			std::uint16_t correction = Flags.C
				? 0x60
				: 0x00;

			if (Flags.H || (!Flags.N && ((reg & 0x0F) > 9))) {
				correction |= 0x06;
			}

			if (Flags.C || (!Flags.N && (reg > 0x99))) {
				correction |= 0x60;
			}

			if (Flags.N) {
				reg = reg - correction;
			} else {
				reg = reg + correction;
			}

			if (((correction << 2) & 0x100) != 0) {
				Flags.C = 1;
			}

			Flags.H = 0;
			if(reg == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;	
			
			Registers.A = reg;
			pc++;
			break;
			
		}
		case 0x28: //good
		{
			Memory.cycle_count += 4;
			if(Flags.Z == 1)
			{
				Memory.cycle_count += 4;
				std::int8_t tmp = Memory.read(pc + 1);
				pc += 2;
				pc = pc + tmp;
				break;
			}
			pc += 2;
			break;
		}
		case 0x29: //good
		{
			Memory.cycle_count += 4;
			std::uint16_t hl = Registers.H << 8 | Registers.L;
			Flags.N = 0;
			Flags.H = (((((Registers.H << 8 | Registers.L) & 0xfff) + ((Registers.H << 8 | Registers.L) & 0xfff)) & 0x1000) == 0x1000);
			Flags.C = (((((Registers.H << 8 | Registers.L) & 0xffff) + ((Registers.H << 8 | Registers.L) & 0xffff)) & 0x10000) == 0x10000);
			hl = hl + hl;
			Registers.H = hl >> 8;
			Registers.L = hl & 0xFF;
			pc++;
			break;			
		}
		case 0x2A: //good
		{
			unsigned short add = Registers.H << 8 | Registers.L;
			Registers.A = Memory.read(add);
			add++;
			Registers.H = add >> 8;
			Registers.L = add & 0xFF;
			//output += " HL:  " +  to_hex((int)add);
			pc++;
			break;
		}
		case 0x2B:
		{
			std::uint16_t hl = Registers.H << 8 | Registers.L;
			hl--;
			Registers.H = hl >> 8;
			Registers.L = hl & 0xff;
			pc++;
			break;
		}
		case 0x2C: //good
		{
			Flags.N = 0;
			
			if((((Registers.L & 0xf) + (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			Registers.L++;
			if(Registers.L == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			
			pc++;
			break;	
		}
		case 0x2D: //dec good
		{
			Flags.N = 1;
			//other flags
			if(Registers.L - 1 == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.L & 0xf) - (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
				
			Registers.L--;
			pc++;
			break;
		}
		case 0x2E: //good
		{
			Registers.L = Memory.read(pc + 1);
			pc += 2;
			break;
		}	
		case 0x2F:

		{
			Flags.N = 1;
			Flags.H = 1;
			Registers.A ^= 0xFF;
			pc++;
			break;
		}
        case 0x30: //fixed
		{
			Memory.cycle_count += 4;
			if(Flags.C == 0)
			{
				std::int8_t tmp = Memory.read(pc + 1);
				pc += 2;
				pc = pc + tmp;
				break;
			}
			pc += 2;
			break;
		}
		case 0x31: // needs further tests
		{
			//std::cout << "check";
			sp = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
			//sp = memory[pc + 2] << 8 | memory[pc + 1];
			pc += 3;
			break;
		}
		case 0x32: //good
		{
			std::uint16_t res = Registers.H << 8 | Registers.L ;
			Memory.write(res,Registers.A);
			res--;
			Registers.H = res >> 8;
			Registers.L = res & 0xFF;
			pc++;
			break;
		}
		case 0x33:
		{
			sp++;
			pc++;
			break;
		}
		
		case 0x34: //fixed
		{
			Flags.N = 0;
			std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
			if((((hl & 0xf) + (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			Memory.write(Registers.H << 8 | Registers.L,hl + 1);
			if(Memory.read(Registers.H << 8 | Registers.L) == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			
			pc++;
			break;
		}
		case 0x35:
		{
			std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
			Flags.N = 1;
			//other flags
			if(hl - 1 == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((hl & 0xf) - (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
				
			Memory.write(Registers.H << 8 | Registers.L, hl - 1);
			pc++;
			break;
		}
		case 0x36: //good
		{
			Memory.write(Registers.H << 8 | Registers.L,Memory.read(pc + 1));
			pc += 2;
			break;
		}
		case 0x37:
		{
			Flags.C = 1;
			Flags.N = 0;
			Flags.H = 0;
			pc++;
			break;
		}
		case 0x38:  //good
		{
			if(Flags.C == 1)
			{
				std::int8_t tmp = Memory.read(pc + 1);
				pc += 2;
				pc += tmp;
				break;
			}
			pc += 2;
			break;
		}
		case 0x39: //fixed
		{
			std::uint16_t result = (Registers.H << 8 | Registers.L) + sp;
			Flags.N = 0;
			if(((((Registers.H << 8 | Registers.L) & 0xfff) + ( sp & 0xfff)) & 0x1000) == 0x1000) 
				Flags.H = 1;
			else
			{
				Flags.H = 0;
			}
				
			if(((((Registers.H << 8 | Registers.L) & 0xffff) + ( sp & 0xffff)) & 0x10000) == 0x10000)
				Flags.C = 1;
			else
			{
				Flags.C = 0;
			}
			
			Registers.H = result >> 8;
			Registers.L = result & 0xFF;
			pc++;
			break;	 	
		}
		case 0x3A:
		{
			std::uint16_t hl = Registers.H << 8 | Registers.L;
			Registers.A = Memory.read(hl);
			hl--;
			Registers.H = hl >> 8;
			Registers.L = hl & 0xFF;
			pc++;
			break;
		}
		case 0x3B:
		{
			sp--;
			pc++;
			break;
		}
		case 0x3C:
		{
			Flags.N = 0;
			
			if((((Registers.A & 0xf) + (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			Registers.A++;
			if(Registers.A == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			
			pc++;
			break;	
		}
		case 0x3D: //good
		{
			Flags.N = 1;
			if(Registers.A - 1 == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.A & 0xf) - (1 & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
				
			Registers.A--;
			pc++;
			break;
		}
		case 0x3E: //good
		{
			Registers.A = Memory.read(pc + 1);
			pc += 2;
			break;
		}
		case 0x3F:
		{
			Flags.H = 0;
			Flags.N = 0;
			if(Flags.C == 1)
				Flags.C = 0;
			else 
				Flags.C = 1;
			pc++;
			break;		
		}
		case 0x40:
		{
			pc++;
			break;
		}
		case 0x41:
		{
			Registers.B = Registers.C;
			pc++;
			break;
		}
		case 0x42:
		{
			Registers.B = Registers.D;
			pc++;
			break;
		}
		case 0x43:
		{
			Registers.B = Registers.E;
			pc++;
			break;
		}
		case 0x44: //good
		{
			Registers.B = Registers.H;
			pc++;
			break;
		}
		case 0x45: //good
		{
			Registers.B = Registers.L;
			pc++;
			break;
		}
		case 0x46: //good
		{
			Registers.B = Memory.read(Registers.H << 8 | Registers.L);
			pc++;
			break;
		} 
		case 0x47: //good
		{
			Registers.B = Registers.A;
			pc++;
			break;
		}
		case 0x48:
		{
			Registers.C = Registers.B;
			pc++;
			break;
		}
		case 0x49:
		{
			Registers.C = Registers.C;
			pc++;
			break;
		}
		case 0x4A:
		{
			Registers.C = Registers.D;
			pc++;
			break;
		}
		case 0x4B: //good
		{
			Registers.C = Registers.E;
			pc++;
			break;
		}
		case 0x4C:
		{
			Registers.C = Registers.H;
			pc++;
			break;
		}
		case 0x4D: //good
		{
			Registers.C = Registers.L;
			pc++;
			break;
		}
		case 0x4E:
		{
			Registers.C = Memory.read(Registers.H << 8 | Registers.L);
			pc++;
			break;
		}
		case 0x4F:
		{
			Registers.C = Registers.A;
			pc++;
			break;
		}
		case 0x50:
		{
			Registers.D = Registers.B;
			pc++;
			break;
		}
		case 0x51:
		{
			Registers.D = Registers.C;
			pc++;
			break;
		}
		case 0x52:
		{
			Registers.D = Registers.D;
			pc++;
			break;
		}
		case 0x53:
		{
			Registers.D = Registers.E;
			pc++;
			break;
		}
		case 0x54:
		{
			Registers.D = Registers.H;
			pc++;
			break;
		}
		case 0x55:
		{
			Registers.D = Registers.L;
			pc++;
			break;
		}
		case 0x56:
		{
			Registers.D = Memory.read(Registers.H << 8 | Registers.L);
			pc++;
			break;
		}
		case 0x57:
		{
			Registers.D = Registers.A;
			pc++;
			break;
		}
		case 0x58:
		{
			Registers.E = Registers.B;
			pc++;
			break;
		}
		case 0x59:
		{
			Registers.E = Registers.C;
			pc++;
			break;
		}
		case 0x5A:
		{
			Registers.E = Registers.D;
			pc++;
			break;
		}
		case 0x5B:
		{
			Registers.E = Registers.E;
			pc++;
			break;
		}
		case 0x5C:
		{
			Registers.E = Registers.H;
			pc++;
			break;
		}
		case 0x5D:
		{
			Registers.E = Registers.L;
			pc++;
			break;
		}
		case 0x5E:
		{
			Registers.E = Memory.read(Registers.H << 8 | Registers.L);
			pc++;
			break;
		}
		case 0x5F: 
		{ 
			Registers.E = Registers.A; 
			pc++; 
			break; 
		} 
		case 0x60: 
		{ 
			Registers.H = Registers.B; 
			pc++; 
			break; 
		} 
		case 0x61: 
		{ 
			Registers.H = Registers.C; 
			pc++; 
			break; 
		} 
		case 0x62:
		{
			Registers.H = Registers.D;
			pc++;
			break;
		}
		case 0x63:
		{
			Registers.H = Registers.E;
			pc++;
			break;
		}
		case 0x64: 
		{
			Registers.H = Registers.H;
			pc++;
			break;
		}
		case 0x65:
		{
			Registers.H = Registers.L;
			pc++;
			break;
		}
		case 0x66:
		{
			Registers.H = Memory.read(Registers.H << 8 | Registers.L);
			pc++;
			break;
		}
		case 0x67:
		{
			Registers.H = Registers.A;
			pc++;
			break;
		}
		case 0x68:
		{
			Registers.L = Registers.B;
			pc++;
			break;
		}
		case 0x69:
		{
			Registers.L = Registers.C;
			pc++;
			break;
		}
		case 0x6A:
		{
			Registers.L = Registers.D;
			pc++;
			break;
		}
		case 0x6B:
		{
			Registers.L = Registers.E;
			pc++;
			break;
		}
		case 0x6C:
		{
			Registers.L = Registers.H;
			pc++;
			break;
		}
		case 0x6D:
		{
			Registers.L = Registers.L;
			pc++;
			break;
		}
		case 0x6E:
		{
			Registers.L = Memory.read(Registers.H << 8 | Registers.L);
			pc++;
			break;
		}
		case 0x6F:
		{
			Registers.L = Registers.A;
			pc++;
			break;
		}
		case 0x70:
		{
			
			Memory.write(Registers.H << 8 | Registers.L, Registers.B);
			pc++;
			break;
		}
		case 0x71:
		{
			Memory.write(Registers.H << 8 | Registers.L, Registers.C);
			pc++;
			break;
		}
		case 0x72:
		{
			Memory.write(Registers.H << 8 | Registers.L, Registers.D);
			pc++;
			break;
		}
		case 0x73:
		{
			Memory.write(Registers.H << 8 | Registers.L, Registers.E);
			pc++;
			break;
		}
		case 0x74:
		{
			Memory.write(Registers.H << 8 | Registers.L, Registers.H);
			pc++;
			break;
		}
		case 0x75:
		{
			Memory.write(Registers.H << 8 | Registers.L, Registers.L);
			pc++;
			break;
		}
		case 0x76: //HALT
		{
			
			if(Memory.ram[0xff0f - 0x8000])
				pc++;
			
			break;
		}
		case 0x77:
		{
			Memory.write(Registers.H << 8 | Registers.L, Registers.A);
			pc++;
			break;
		}
		case 0x78:
		{
			Registers.A = Registers.B;
			pc++;
			break;
		}
		case 0x79:
		{
			Registers.A = Registers.C;
			pc++;
			break;
		}
		case 0x7A:
		{
			Registers.A = Registers.D;
			pc++;
			break;
		}
		case 0x7B:
		{
			Registers.A = Registers.E;
			pc++;
			break;
		}
		case 0x7C:
		{
			Registers.A = Registers.H;
			pc++;
			break;
		}
		case 0x7D:
		{
			Registers.A = Registers.L;
			pc++;
			break;
		}
		case 0x7E:
		{
			Registers.A = Memory.read(Registers.H << 8 | Registers.L);
			pc++;
			break;
		}
		case 0x7F:
		{
			pc++;
			break;
		}
		case 0x80:
		{
			if((((Registers.A & 0xF) + (Registers.B & 0xF)) & 0x10) == 0x10)
				Flags.H = 1;
			else
			{
				Flags.H = 0;
			}
				
			if((((Registers.A & 0xFF) + (Registers.B & 0xFF)) & 0x100) == 0x100)
				Flags.C = 1;
			else
			{
				Flags.C = 0;
			}
				
			Flags.N = 0;
			unsigned char result = Registers.A + Registers.B;
			if(result == 0)
				Flags.Z = 1;
			else
			{
				Flags.Z = 0;
			}
				
			Registers.A += Registers.B;
			pc++;
			break;
		}
		case 0x81:
		{
			if((((Registers.A & 0xF) + (Registers.C & 0xF)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xFF) + (Registers.C & 0xFF)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;
			Flags.N = 0;
			unsigned char result = Registers.A + Registers.C;
			if(result == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			Registers.A += Registers.C;
			pc++;
			break;
		}
		case 0x82:
		{
			if((((Registers.A & 0xF) + (Registers.D & 0xF)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xFF) + (Registers.D & 0xFF)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;
			Flags.N = 0;
			unsigned char result = Registers.A + Registers.D;
			if(result == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			Registers.A += Registers.D;
			pc++;
			break;
		}
		case 0x83:
		{
			if((((Registers.A & 0xF) + (Registers.E & 0xF)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xFF) + (Registers.E & 0xFF)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;
			Flags.N = 0;
			unsigned char result = Registers.A + Registers.E;
			if(result == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			Registers.A += Registers.E;
			pc++;
			break;
		}
		case 0x84:
		{
			if((((Registers.A & 0xF) + (Registers.H & 0xF)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xFF) + (Registers.H & 0xFF)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;
			Flags.N = 0;
			unsigned char result = Registers.A + Registers.H;
			if(result == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			Registers.A += Registers.H;
			pc++;
			break;
		}
		case 0x85:
		{
			//(((a&0xf) + (b&0xf)) & 0x10) == 0x10
			if((((Registers.A & 0xF) + (Registers.L & 0xF)) & 0x10) == 0x10)
				Flags.H = 1;
			else
			{
				Flags.H = 0;
			}
				
			if((((Registers.A & 0xFF) + (Registers.L & 0xFF)) & 0x100) == 0x100)
				Flags.C = 1;
			else
			{
				Flags.C = 0;
			}
				
			Flags.N = 0;
			unsigned char result = Registers.A + Registers.L;
			if(result == 0)
				Flags.Z = 1;
			else
			{
				Flags.Z = 0;
			}
				
			Registers.A += Registers.L;
			pc++;
			break;

		}
		case 0x86:
		{
			//(((a&0xf) + (b&0xf)) & 0x10) == 0x10
			std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
			if((((Registers.A & 0xF) + (hl & 0xF)) & 0x10) == 0x10)
				Flags.H = 1;
			else
			{
				Flags.H = 0;
			}
				
			if((((Registers.A & 0xFF) + (hl & 0xFF)) & 0x100) == 0x100)
				Flags.C = 1;
			else
			{
				Flags.C = 0;
			}
				
			Flags.N = 0;
			unsigned char result = Registers.A + hl;
			if(result == 0)
				Flags.Z = 1;
			else
			{
				Flags.Z = 0;
			}
				
			Registers.A += hl;
			pc++;
			break;

		}
		case 0x87:
		{
			if((((Registers.A & 0xF) + (Registers.A & 0xF)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xFF) + (Registers.A & 0xFF)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;
			Flags.N = 0;
			unsigned char result = Registers.A + Registers.A;
			if(result == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			Registers.A += Registers.A;

			pc++;
			break;
		}
		case 0x88:
		{
			Flags.N = 0;
			std::uint8_t res = Registers.A + Registers.B + Flags.C;
			Flags.H = ((((Registers.A & 0xf) + (Registers.B & 0xf) + Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) + (Registers.B & 0xff) + Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc += 2;
			break;
		}
		case 0x89:
		{
			Flags.N = 0;
			std::uint8_t res = Registers.A + Registers.C + Flags.C;
			Flags.H = ((((Registers.A & 0xf) + (Registers.C & 0xf) + Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) + (Registers.C & 0xff) + Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc += 2;
			break;
		}
		case 0x8A:
		{
			Flags.N = 0;
			std::uint8_t res = Registers.A + Registers.D + Flags.C;
			Flags.H = ((((Registers.A & 0xf) + (Registers.D & 0xf) + Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) + (Registers.D & 0xff) + Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc += 2;
			break;
		}
		case 0x8B:
		{
			Flags.N = 0;
			std::uint8_t res = Registers.A + Registers.E + Flags.C;
			Flags.H = ((((Registers.A & 0xf) + (Registers.E & 0xf) + Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) + (Registers.E & 0xff) + Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc += 2;
			break;
		}
		case 0x8C:
		{
			Flags.N = 0;
			std::uint8_t res = Registers.A + Registers.H + Flags.C;
			Flags.H = ((((Registers.A & 0xf) + (Registers.H & 0xf) + Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) + (Registers.H & 0xff) + Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc += 2;
			break;
		}
		case 0x8D:
		{
			Flags.N = 0;
			std::uint8_t res = Registers.A + Registers.L + Flags.C;
			Flags.H = ((((Registers.A & 0xf) + (Registers.L & 0xf) + Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) + (Registers.L & 0xff) + Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc += 2;
			break;
		}
		case 0x8E:
		{
			std::uint8_t res = Registers.A + Memory.read(Registers.H << 8 | Registers.L) + Flags.C;

			Flags.N = 0;
			Flags.H = ((((Registers.A & 0xf) + (Memory.read(Registers.H << 8 | Registers.L) & 0xf) + Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) + (Memory.read(Registers.H << 8 | Registers.L) & 0xff) + Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			
			Registers.A = res;	
			pc += 2;
			break;
		}
		case 0x8F:
		{
			Flags.N = 0;
			std::uint8_t res = Registers.A + Registers.A + Flags.C;
			Flags.H = ((((Registers.A & 0xf) + (Registers.A & 0xf) + Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) + (Registers.A & 0xff) + Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc += 2;
			break;
		}
		case 0x90:
		{
			Flags.N = 1;
			//other flags
			Flags.Z = (Registers.A - Registers.B == 0);
			Flags.H = ((((Registers.A & 0xf) - (Registers.B & 0xf)) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.B & 0xff)) & 0x100) == 0x100);	
			Registers.A -= Registers.B;
			pc++;
			break;
		}
		case 0x91:
		{
			Flags.N = 1;
			//other flags
			Flags.Z = (Registers.A - Registers.C == 0);
			Flags.H = ((((Registers.A & 0xf) - (Registers.C & 0xf)) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.C & 0xff)) & 0x100) == 0x100);	
			Registers.A -= Registers.C;
			pc++;
			break;
		}
		case 0x92:
		{
			Flags.N = 1;
			//other flags
			Flags.Z = (Registers.A - Registers.D == 0);
			Flags.H = ((((Registers.A & 0xf) - (Registers.D & 0xf)) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.D & 0xff)) & 0x100) == 0x100);	
			Registers.A -= Registers.D;
			pc++;
			break;
		}
		case 0x93:
		{
			Flags.N = 1;
			//other flags
			Flags.Z = (Registers.A - Registers.E == 0);
			Flags.H = ((((Registers.A & 0xf) - (Registers.E & 0xf)) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.E & 0xff)) & 0x100) == 0x100);	
			Registers.A -= Registers.E;
			pc++;
			break;
		}
		case 0x94:
		{
			Flags.N = 1;
			//other flags
			Flags.Z = (Registers.A - Registers.H == 0);
			Flags.H = ((((Registers.A & 0xf) - (Registers.H & 0xf)) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.H & 0xff)) & 0x100) == 0x100);	
			Registers.A -= Registers.H;
			pc++;
			break;
		}
		case 0x95:
		{
			Flags.N = 1;
			//other flags
			Flags.Z = (Registers.A - Registers.L == 0);
			Flags.H = ((((Registers.A & 0xf) - (Registers.L & 0xf)) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.L & 0xff)) & 0x100) == 0x100);	
			Registers.A -= Registers.L;
			pc++;
			break;
		}
		case 0x96:
		{
			Flags.N = 1;
			//other flags
			Flags.Z = (Registers.A - Memory.read(Registers.H << 8 | Registers.L) == 0);	
			if((((Registers.A & 0xf) - (Memory.read(Registers.H << 8 | Registers.L) & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xff) - (Memory.read(Registers.H << 8 | Registers.L) & 0xff)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;	
			Registers.A -= Memory.read(Registers.H << 8 | Registers.L);
			pc++;
			break;
		}
		case 0x97:
		{
			Flags.N = 1;
			//other flags
			Flags.Z = (Registers.A - Registers.A == 0);
			Flags.H = ((((Registers.A & 0xf) - (Registers.A & 0xf)) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.A & 0xff)) & 0x100) == 0x100);	
			Registers.A -= Registers.A;
			pc++;
			break;
		}

		case 0x98:
		{
			std::uint8_t res = Registers.A - Registers.B - Flags.C;
			
			Flags.N = 1;
			Flags.H = ((((Registers.A & 0xf) - (Registers.B & 0xf) - Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.B & 0xff) - Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;

			pc++;
			break;
		}
		case 0x99:
		{
			std::uint8_t res = Registers.A - Registers.C - Flags.C;
			
			Flags.N = 1;
			Flags.H = ((((Registers.A & 0xf) - (Registers.C & 0xf) - Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.C & 0xff) - Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;
		}
		case 0x9A:
		{
			std::uint8_t res = Registers.A - Registers.D - Flags.C;
			
			Flags.N = 1;
			Flags.H = ((((Registers.A & 0xf) - (Registers.D & 0xf) - Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.D & 0xff) - Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;
		}
		case 0x9B:
		{
			std::uint8_t res = Registers.A - Registers.E - Flags.C;
			
			Flags.N = 1;
			Flags.H = ((((Registers.A & 0xf) - (Registers.E & 0xf) - Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.E & 0xff) - Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;

			pc++;
			break;
		}
		case 0x9C:
		{
			std::uint8_t res = Registers.A - Registers.H - Flags.C;
			
			Flags.N = 1;
			Flags.H = ((((Registers.A & 0xf) - (Registers.H & 0xf) - Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.H & 0xff) - Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;
		}
		case 0x9D:
		{
			std::uint8_t res = Registers.A - Registers.L - Flags.C;
			
			Flags.N = 1;
			Flags.H = ((((Registers.A & 0xf) - (Registers.L & 0xf) - Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.L & 0xff) - Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;
		}
		case 0x9E:
		{
			std::uint8_t res = Registers.A - Memory.read(Registers.H << 8 | Registers.L) - Flags.C;
			
			Flags.N = 1;
			Flags.H = ((((Registers.A & 0xf) - (Memory.read(Registers.H << 8 | Registers.L) & 0xf) - Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Memory.read(Registers.H << 8 | Registers.L) & 0xff) - Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;
		}
		case 0x9F:
		{
			std::uint8_t res = Registers.A - Registers.A - Flags.C;
			
			Flags.N = 1;
			Flags.H = ((((Registers.A & 0xf) - (Registers.A & 0xf) - Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Registers.A & 0xff) - Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;

			pc++;
			break;
		}
		case 0xA0: 
		{
			std::uint8_t res = Registers.A & Registers.B;
			Flags.H = 1;
			Flags.N = 0;
			Flags.C = 0;
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;		
		}
		case 0xA1: 
		{
			std::uint8_t res = Registers.A & Registers.C;
			Flags.H = 1;
			Flags.N = 0;
			Flags.C = 0;
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;		
		}
		case 0xA2: 
		{
			std::uint8_t res = Registers.A & Registers.D;
			Flags.H = 1;
			Flags.N = 0;
			Flags.C = 0;
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;		
		}
		case 0xA3: 
		{
			std::uint8_t res = Registers.A & Registers.E;
			Flags.H = 1;
			Flags.N = 0;
			Flags.C = 0;
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;		
		}
		case 0xA4: 
		{
			std::uint8_t res = Registers.A & Registers.H;
			Flags.H = 1;
			Flags.N = 0;
			Flags.C = 0;
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;		
		}
		case 0xA5: 
		{
			std::uint8_t res = Registers.A & Registers.L;
			Flags.H = 1;
			Flags.N = 0;
			Flags.C = 0;
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;		
		}
		case 0xA6:
		{
			std::uint8_t res = Registers.A & Memory.read(Registers.H << 8 | Registers.L);
			Flags.H = 1;
			Flags.N = 0;
			Flags.C = 0;
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Registers.A = res;
			pc++;
			break;	
		}
		case 0xA7: 
		{
			std::uint8_t res = Registers.A & Registers.A;
			Flags.H = 1;
			Flags.N = 0;
			Flags.C = 0;
			Flags.Z = (res == 0);
			Registers.A = res;
			pc++;
			break;		
		}
		case 0xA8:
		{
			Flags.Z = ((Registers.A ^ Registers.B) == 0);
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A ^= Registers.B;
			pc++;
			break;		
		}
		case 0xA9:
		{
			Flags.Z = ((Registers.A ^ Registers.C) == 0);
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A ^= Registers.C;
			pc++;
			break;		
		}
		case 0xAA:
		{
			Flags.Z = ((Registers.A ^ Registers.D) == 0);
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A ^= Registers.D;
			pc++;
			break;		
		}
		case 0xAB:
		{
			if((Registers.A ^ Registers.E) == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A ^= Registers.E;
			pc++;
			break;	
		}
		case 0xAC:
		{
			Flags.Z = ((Registers.A ^ Registers.H) == 0);
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A ^= Registers.H;
			pc++;
			break;		
		}
		case 0xAD:
		{
			if((Registers.A ^ Registers.L) == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A ^= Registers.L;
			pc++;
			break;
		}
		case 0xAE: //fixed
		{
			if((Registers.A ^ Memory.read(Registers.H << 8 | Registers.L)) == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A ^= Memory.read(Registers.H << 8 | Registers.L);
			pc++;
			break;	
		}
		case 0xAF:
		{
			if((Registers.A ^ Registers.A) == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A ^= Registers.A;
			pc++;
			break;	
		}
		case 0xB0:
		{
			std::uint8_t res = Registers.A | Registers.B;
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A = res;
			pc++;
			break;	
		}
		case 0xB1:
		{
			std::uint8_t res = Registers.A | Registers.C;
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A = res;
			pc++;
			break;		

		}
		case 0xB2:
		{
			std::uint8_t res = Registers.A | Registers.D;
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A = res;
			pc++;
			break;
		}
		case 0xB3:
		{
			std::uint8_t res = Registers.A | Registers.E;
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A = res;
			pc++;
			break;
		}
		case 0xB4:
		{
			std::uint8_t res = Registers.A | Registers.H;
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A = res;
			pc++;
			break;
		}
		case 0xB5:
		{
			std::uint8_t res = Registers.A | Registers.L;
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A = res;
			pc++;
			break;
		}
		case 0xB6: //fixed
		{
			std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
			if((Registers.A | hl) == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.C = 0;
			Flags.N = 0;
			Flags.H = 0;
			Registers.A |= hl;
			pc++;
			break;
		}
		
		case 0xB7:
		{
			if((Registers.A | Registers.A) == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.C = 0;
			Flags.N = 0;
			Flags.H = 0;
			Registers.A |= Registers.A;
			pc++;
			break;		
		}
		case 0xB8:
		{
			Flags.N = 1;
			if(Registers.A - Registers.B == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			Flags.N = 1;
			if(Registers.A < Registers.B)
				Flags.C = 1;
			else
				Flags.C = 0;
			if((((Registers.A & 0xf) - (Registers.B & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			pc++;
			break;		
		}
		case 0xB9:
		{
			Flags.N = 1;
			if(Registers.A - Registers.C == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			Flags.N = 1;
			if(Registers.A < Registers.C)
				Flags.C = 1;
			else
				Flags.C = 0;
			if((((Registers.A & 0xf) - (Registers.C & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			pc++;
			break;		
		}
		case 0xBA: //needs confirmation
		{
			Flags.N = 1;
			if(Registers.A - Registers.D == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			Flags.N = 1;
			if(Registers.A < Registers.D)
				Flags.C = 1;
			else
				Flags.C = 0;
			if((((Registers.A & 0xf) - (Registers.D & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			pc++;
			break;		
		}
		case 0xBB:
		{
			Flags.N = 1;
			if(Registers.A - Registers.E == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			Flags.N = 1;
			if(Registers.A < Registers.E)
				Flags.C = 1;
			else
				Flags.C = 0;
			if((((Registers.A & 0xf) - (Registers.E & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			pc++;
			break;		
		}
		case 0xBC:
		{
			Flags.N = 1;
			if(Registers.A - Registers.H == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			Flags.N = 1;
			if(Registers.A < Registers.H)
				Flags.C = 1;
			else
				Flags.C = 0;
			if((((Registers.A & 0xf) - (Registers.H & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			pc++;
			break;		
		}
		case 0xBD:
		{
			Flags.N = 1;
			if(Registers.A - Registers.L == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			Flags.N = 1;
			if(Registers.A < Registers.L)
				Flags.C = 1;
			else
				Flags.C = 0;
			if((((Registers.A & 0xf) - (Registers.L & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			pc++;
			break;		
		}
		case 0xBE: //needs confirmation
		{
			if(pc == 0xFF86)
				pc = pc;
			Flags.N = 1;
			if(Registers.A < Memory.read(Registers.H << 8 | Registers.L))
				Flags.C = 1;
			else 
				Flags.C = 0;
			if(Registers.A == Memory.read(Registers.H << 8 | Registers.L))
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 1;
			if((((Registers.A & 0xf) - (Memory.read(Registers.H << 8 | Registers.L) & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else 
				Flags.H = 0;
			pc += 1;
			break;	
		}
		case 0xBF:
		{
			Flags.N = 1;
			if(Registers.A - Registers.A == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			Flags.N = 1;
			if(Registers.A < Registers.A)
				Flags.C = 1;
			else
				Flags.C = 0;
			if((((Registers.A & 0xf) - (Registers.A & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			pc++;
			break;		
		}
		case 0xC0:
		{
			if(Flags.Z == 0)
			{
				std::uint16_t low;
				std::uint16_t hi;
				low = Memory.read(sp++);
				hi = Memory.read(sp++);
				pc = hi << 8 | low;
				break; 
			}
			pc++;
			break;
		}
		case 0xC1: 
		{
			Registers.C = Memory.read(sp++);
			Registers.B = Memory.read(sp++);
			pc++;
			break;

		}
		case 0xC2: //fixed
		{
			if(Flags.Z == 0)
			{
				std::uint16_t add = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
				pc = add;
				break;
			}
			pc += 3;
			break;
		}
		case 0xC3:
		{
			pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
			//pc += memory[pc + 2] << 8 | memory[pc + 1];
			break;
		}
		case 0xC4:
		{
			Memory.cycle_count += 8;
			if(Flags.Z == 0)
			{
				Memory.write(--sp, (pc + 3) >> 8);
				Memory.write(--sp, (pc + 3) & 0xFF);
				pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
				break;
			}
			pc += 3;
			break;
		}
		case 0xC5:
		{
			Memory.write(--sp,Registers.B);
			Memory.write(--sp,Registers.C);
			pc++;
			break;
		}
		case 0xC6: //fixed
		{
			std::uint8_t result = Registers.A + Memory.read(pc + 1);
			Flags.N = 0;
			if(result == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
			Flags.N = 0;
			if((((Registers.A & 0xf) + (Memory.read(pc + 1) & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else 
				Flags.H = 0;
			if((((Registers.A & 0xff) + (Memory.read(pc + 1) & 0xff)) & 0x100) == 0x100)
				Flags.C = 1;
			else 
				Flags.C = 0;
			Registers.A = result;	
			pc += 2;
			break;							
		}
		case 0xC7:
		{
			pc++;
			Memory.write(--sp,pc >> 8);
			Memory.write(--sp,pc & 0xFF);
			pc = 0x0000;
			break;
		}
		case 0xC8: //idk
		{
			if(Flags.Z == 1)
			{
				std::uint16_t low;
				std::uint16_t hi;
				low = Memory.read(sp++);
				hi = Memory.read(sp++);
				pc = hi << 8 | low;
				break; 
			}
			pc++;
			break;
		}

		case 0xC9: //fixed
		{
			std::uint8_t low;
			std::uint8_t hi;
			low = Memory.read(sp++);
			hi = Memory.read(sp++);

			pc = hi << 8 | low;
			break; 
		}
		case 0xCA:
		{
			if(Flags.Z == 1)
			{
				pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
				break;
			}
			pc += 3;
			break;
		}
		case 0xCB:
		{
			switch (Memory.read(pc + 1))
			{
				case 0x00: 
				{
					bool msb = Registers.B >> 7;
					Flags.C = msb;
					Registers.B <<= 1;
					Registers.B |= msb;
					Flags.Z = (Registers.B == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x01: 
				{
					bool msb = Registers.C >> 7;
					Flags.C = msb;
					Registers.C <<= 1;
					Registers.C |= msb;
					Flags.Z = (Registers.C == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x02: 
				{
					bool msb = Registers.D >> 7;
					Flags.C = msb;
					Registers.D <<= 1;
					Registers.D |= msb;
					Flags.Z = (Registers.D == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x03: 
				{
					bool msb = Registers.E >> 7;
					Flags.C = msb;
					Registers.E <<= 1;
					Registers.E |= msb;
					Flags.Z = (Registers.E == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x04: 
				{
					bool msb = Registers.H >> 7;
					Flags.C = msb;
					Registers.H <<= 1;
					Registers.H |= msb;
					Flags.Z = (Registers.H == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x05: 
				{
					bool msb = Registers.L >> 7;
					Flags.C = msb;
					Registers.L <<= 1;
					Registers.L |= msb;
					Flags.Z = (Registers.L == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x06: 
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					bool msb = hl >> 7;
					Flags.C = msb;
					hl <<= 1;
					hl |= msb;
					Flags.Z = (hl == 0);
					Flags.N = 0;
					Flags.H = 0;
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;	
				}
				case 0x07: 
				{
					bool msb = Registers.A >> 7;
					Flags.C = msb;
					Registers.A <<= 1;
					Registers.A |= msb;
					Flags.Z = (Registers.A == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x08:
				{
					Flags.H = 0;
					Flags.N = 0;
					bool lsb = Registers.B & 0x1;
					Flags.C = lsb;
					Registers.B >>= 1;
					Registers.B = lsb << 7 | Registers.B;
					Flags.Z = (Registers.B == 0);
					pc += 2;
					break;
				}
				case 0x09:
				{
					Flags.H = 0;
					Flags.N = 0;
					bool lsb = Registers.C & 0x1;
					Flags.C = lsb;
					Registers.C >>= 1;
					Registers.C = lsb << 7 | Registers.C;
					Flags.Z = (Registers.C == 0);
					pc += 2;
					break;
				}
				case 0x0A:
				{
					Flags.H = 0;
					Flags.N = 0;
					bool lsb = Registers.D & 0x1;
					Flags.C = lsb;
					Registers.D >>= 1;
					Registers.D = lsb << 7 | Registers.D;
					Flags.Z = (Registers.D == 0);
					pc += 2;
					break;
				}
				case 0x0B:
				{
					Flags.H = 0;
					Flags.N = 0;
					bool lsb = Registers.E & 0x1;
					Flags.C = lsb;
					Registers.E >>= 1;
					Registers.E = lsb << 7 | Registers.E;
					Flags.Z = (Registers.E == 0);
					pc += 2;
					break;
				}
				case 0x0C:
				{
					Flags.H = 0;
					Flags.N = 0;
					bool lsb = Registers.H & 0x1;
					Flags.C = lsb;
					Registers.H >>= 1;
					Registers.H = lsb << 7 | Registers.H;
					Flags.Z = (Registers.H == 0);
					pc += 2;
					break;
				}
				case 0x0D:
				{
					Flags.H = 0;
					Flags.N = 0;
					bool lsb = Registers.L & 0x1;
					Flags.C = lsb;
					Registers.L >>= 1;
					Registers.L = lsb << 7 | Registers.L;
					Flags.Z = (Registers.L == 0);
					pc += 2;
					break;
				}
				case 0x0E: //shame
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					Flags.H = 0;
					Flags.N = 0;
					bool lsb = hl & 0x1;
					Flags.C = lsb;
					hl >>= 1;
					hl = lsb << 7 | hl;
					Flags.Z = (hl == 0);
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				}
				case 0x0F:
				{
					Flags.H = 0;
					Flags.N = 0;
					bool lsb = Registers.A & 0x1;
					Flags.C = lsb;
					Registers.A >>= 1;
					Registers.A = lsb << 7 | Registers.A;
					Flags.Z = (Registers.A == 0);
					pc += 2;
					break;
				}
				case 0x10: 
				{
					bool msb = Flags.C;
					Flags.C = (Registers.B >> 7) & 1;
					Registers.B = (Registers.B << 1) | msb;
					Flags.Z = (Registers.B == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x11: 
				{
					bool msb = Flags.C;
					Flags.C = (Registers.C >> 7) & 1;
					Registers.C = (Registers.C << 1) | msb;
					Flags.Z = (Registers.C == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x12: 
				{
					bool msb = Flags.C;
					Flags.C = (Registers.D >> 7) & 1;
					Registers.D = (Registers.D << 1) | msb;
					Flags.Z = (Registers.D == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x13: 
				{
					bool msb = Flags.C;
					Flags.C = (Registers.E >> 7) & 1;
					Registers.E = (Registers.E << 1) | msb;
					Flags.Z = (Registers.E == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x14: 
				{
					bool msb = Flags.C;
					Flags.C = (Registers.H >> 7) & 1;
					Registers.H = (Registers.H << 1) | msb;
					Flags.Z = (Registers.H == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x15: 
				{
					bool msb = Flags.C;
					Flags.C = (Registers.L >> 7) & 1;
					Registers.L = (Registers.L << 1) | msb;
					Flags.Z = (Registers.L == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}
				case 0x16: 
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					bool msb = Flags.C;
					Flags.C = (hl >> 7) & 1;
					hl = (hl << 1) | msb;
					Flags.Z = (hl == 0);
					Flags.N = 0;
					Flags.H = 0;
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;	
				}
				case 0x17: 
				{
					bool msb = Flags.C;
					Flags.C = (Registers.A >> 7) & 1;
					Registers.A = (Registers.A << 1) | msb;
					Flags.Z = (Registers.A == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;	
				}

				case 0x18:
				{
					Flags.H = 0;
					Flags.N = 0;

					bool msb = Flags.C;
					Flags.C = Registers.B & 0x1;
					Registers.B >>= 1;
					Registers.B = msb << 7 | Registers.B;
					if(Registers.B == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;	
					pc += 2;
					break;
				}
				case 0x19:
				{
					Flags.H = 0;
					Flags.N = 0;

					bool msb = Flags.C;
					Flags.C = Registers.C & 0x1;
					Registers.C >>= 1;
					Registers.C = msb << 7 | Registers.C;
					if(Registers.C == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;	
					pc += 2;
					break;
				}
				case 0x1A:
				{
					Flags.H = 0;
					Flags.N = 0;

					bool msb = Flags.C;
					Flags.C = Registers.D & 0x1;
					Registers.D >>= 1;
					Registers.D = msb << 7 | Registers.D;
					if(Registers.D == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;	
					pc += 2;
					break;
				}
				case 0x1B:
				{
					Flags.H = 0;
					Flags.N = 0;

					bool msb = Flags.C;
					Flags.C = Registers.E & 0x1;
					Registers.E >>= 1;
					Registers.E = msb << 7 | Registers.E;
					if(Registers.E == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;	
					pc += 2;
					break;
				}
				case 0x1C:
				{
					Flags.H = 0;
					Flags.N = 0;

					bool msb = Flags.C;
					Flags.C = Registers.H & 0x1;
					Registers.H >>= 1;
					Registers.H = msb << 7 | Registers.H;
					if(Registers.H == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;	
					pc += 2;
					break;
				}
				case 0x1D:
				{
					Flags.H = 0;
					Flags.N = 0;

					bool msb = Flags.C;
					Flags.C = Registers.L & 0x1;
					Registers.L >>= 1;
					Registers.L = msb << 7 | Registers.L;
					if(Registers.L == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;	
					pc += 2;
					break;
				}
				case 0x1E:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					bool msb = Flags.C;

					Flags.H = 0;
					Flags.N = 0;

					Flags.C = hl & 0x1;
					hl >>= 1;
					hl = msb << 7 | hl;
					Flags.Z = (hl == 0);
						
					Memory.write(Registers.H << 8 | Registers.L, hl);

					pc += 2;
					break;
				}
				case 0x1F:
				{
					Flags.H = 0;
					Flags.N = 0;

					bool msb = Flags.C;
					Flags.C = Registers.A & 0x1;
					Registers.A >>= 1;
					Registers.A = msb << 7 | Registers.A;
					if(Registers.A == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;	
					pc += 2;
					break;
				}
				case 0x20:
				{
					Flags.C = Registers.B & 0x80;
					Registers.B <<= 1;

					Flags.Z = (Registers.B == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x21:
				{
					Flags.C = Registers.C & 0x80;
					Registers.C <<= 1;
					
					Flags.Z = (Registers.C == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x22:
				{
					Flags.C = Registers.D & 0x80;
					Registers.D <<= 1;
					
					Flags.Z = (Registers.D == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x23:
				{
					Flags.C = Registers.E & 0x80;
					Registers.E <<= 1;
					
					Flags.Z = (Registers.E == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x24:
				{
					Flags.C = Registers.H & 0x80;
					Registers.H <<= 1;
					
					Flags.Z = (Registers.H == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x25:
				{
					Flags.C = Registers.L & 0x80;
					Registers.L <<= 1;
					
					Flags.Z = (Registers.L == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x26:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					Flags.C = hl & 0x80;
					hl <<= 1;
					Flags.Z = (hl == 0);
					Flags.N = 0;
					Flags.H = 0;
					Memory.write(Registers.H << 8 | Registers.L,hl);
					pc += 2;
					break;
				}
				case 0x27:
				{
					Flags.C = Registers.A & 0x80;
					Registers.A <<= 1;
					
					Flags.Z = (Registers.A == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x28:
				{
					Flags.C = Registers.B & 0x1;
					Registers.B = (Registers.B >> 1) + (Registers.B & 0x80);

					Flags.Z = (Registers.B == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x29:
				{
					Flags.C = Registers.C & 0x1;
					Registers.C = (Registers.C >> 1) + (Registers.C & 0x80);

					Flags.Z = (Registers.C == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x2A:
				{
					Flags.C = Registers.D & 0x1;
					Registers.D = (Registers.D >> 1) + (Registers.D & 0x80);

					Flags.Z = (Registers.D == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x2B:
				{
					Flags.C = Registers.E & 0x1;
					Registers.E = (Registers.E >> 1) + (Registers.E & 0x80);

					Flags.Z = (Registers.E == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x2C:
				{
					Flags.C = Registers.H & 0x1;
					Registers.H = (Registers.H >> 1) + (Registers.H & 0x80);

					Flags.Z = (Registers.H == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x2D:
				{
					Flags.C = Registers.L & 0x1;
					Registers.L = (Registers.L >> 1) + (Registers.L & 0x80);

					Flags.Z = (Registers.L == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x2E:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					Flags.C = hl & 0x1;
					hl = (hl >> 1) + (hl & 0x80);

					Flags.Z = (hl == 0);
					Flags.N = 0;
					Flags.H = 0;

					Memory.write(Registers.H << 8 | Registers.L, hl);

					pc += 2;
					break;
				}
				case 0x2F:
				{
					Flags.C = Registers.A & 0x1;
					Registers.A = (Registers.A >> 1) + (Registers.A & 0x80);

					Flags.Z = (Registers.A == 0);
					Flags.N = 0;
					Flags.H = 0;
					pc += 2;
					break;
				}
				case 0x30: //swap
				{
					std::uint8_t tmp = Registers.B >> 4;
					Registers.B <<= 4;
					Registers.B = Registers.B | tmp;
					if(Registers.B == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					Flags.C = 0;
					Flags.H = 0;
					Flags.N = 0;		
					pc += 2;
					break;
				}
				case 0x31: //swap
				{
					std::uint8_t tmp = Registers.C >> 4;
					Registers.C <<= 4;
					Registers.C = Registers.C | tmp;
					if(Registers.C == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					Flags.C = 0;
					Flags.H = 0;
					Flags.N = 0;		
					pc += 2;
					break;
				}
				case 0x32: //swap
				{
					std::uint8_t tmp = Registers.D >> 4;
					Registers.D <<= 4;
					Registers.D = Registers.D | tmp;
					if(Registers.D == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					Flags.C = 0;
					Flags.H = 0;
					Flags.N = 0;		
					pc += 2;
					break;
				}
				case 0x33: //swap
				{
					std::uint8_t tmp = Registers.E >> 4;
					Registers.E <<= 4;
					Registers.E = Registers.E | tmp;
					if(Registers.E == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					Flags.C = 0;
					Flags.H = 0;
					Flags.N = 0;		
					pc += 2;
					break;
				}
				case 0x34: //swap
				{
					std::uint8_t tmp = Registers.H >> 4;
					Registers.H <<= 4;
					Registers.H = Registers.H | tmp;
					if(Registers.H == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					Flags.C = 0;
					Flags.H = 0;
					Flags.N = 0;		
					pc += 2;
					break;
				}
				case 0x35: //swap
				{
					std::uint8_t tmp = Registers.L >> 4;
					Registers.L <<= 4;
					Registers.L = Registers.L | tmp;
					if(Registers.L == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					Flags.C = 0;
					Flags.H = 0;
					Flags.N = 0;		
					pc += 2;
					break;
				}
				case 0x36: //swap
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					std::uint8_t tmp = hl >> 4;

					hl <<= 4;
					hl = hl | tmp;

					Flags.Z = (hl == 0);
					Flags.C = 0;
					Flags.H = 0;
					Flags.N = 0;		

					Memory.write(Registers.H << 8 | Registers.L, hl);

					pc += 2;
					break;
				}
				case 0x37: //swap
				{
					std::uint8_t tmp = Registers.A >> 4;
					Registers.A <<= 4;
					Registers.A = Registers.A | tmp;
					if(Registers.A == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					Flags.C = 0;
					Flags.H = 0;
					Flags.N = 0;		
					pc += 2;
					break;
				}
				case 0x38: //broken wohooo
				{
					Flags.C = Registers.B & 0x1;
					Flags.N = 0;
					Flags.H = 0;
					Registers.B >>= 1;
					if (Registers.B == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					pc += 2;
					break;
				}
				case 0x39: //broken wohooo
				{
					Flags.C = Registers.C & 0x1;
					Flags.N = 0;
					Flags.H = 0;
					Registers.C >>= 1;
					if (Registers.C == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					pc += 2;
					break;
				}
				case 0x3A: //broken wohooo
				{
					Flags.C = Registers.D & 0x1;
					Flags.N = 0;
					Flags.H = 0;
					Registers.D >>= 1;
					if (Registers.D == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					pc += 2;
					break;
				}
				case 0x3B: //broken wohooo
				{
					Flags.C = Registers.E & 0x1;
					Flags.N = 0;
					Flags.H = 0;
					Registers.E >>= 1;
					if (Registers.E == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					pc += 2;
					break;
				}
				case 0x3C: //broken wohooo
				{
					Flags.C = Registers.H & 0x1;
					Flags.N = 0;
					Flags.H = 0;
					Registers.H >>= 1;
					if (Registers.H == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					pc += 2;
					break;
				}
				case 0x3D: 
				{
					Flags.C = Registers.L & 0x1;
					Flags.N = 0;
					Flags.H = 0;
					Registers.L >>= 1;
					if (Registers.L == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					pc += 2;
					break;
				}
				case 0x3E: 
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);

					Flags.C = hl & 0x1;
					Flags.N = 0;
					Flags.H = 0;
					hl >>= 1;
					Flags.Z = (hl == 0);
					
					Memory.write(Registers.H << 8 | Registers.L, hl);

					pc += 2;
					break;
				}
				case 0x3f:
				{
					Flags.C = Registers.A & 0x1;
					Flags.N = 0;
					Flags.H = 0;
					Registers.A >>= 1;
					if (Registers.A == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					pc += 2;
					break;
				}
				case 0x40:
				{
					Flags.Z = ((Registers.B & 0x1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x41:
				{
					Flags.Z = ((Registers.C & 0x1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x42:
				{
					Flags.Z = ((Registers.D & 0x1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x43:
				{
					Flags.Z = ((Registers.E & 0x1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x44:
				{
					Flags.Z = ((Registers.H & 0x1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x45:
				{
					Flags.Z = ((Registers.L & 0x1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x46:
				{
					Flags.Z = ((Memory.read(Registers.H << 8 | Registers.L) & 0x1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x47:
				{
					Flags.Z = ((Registers.A & 0x1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x48:
				{
					Flags.Z = (((Registers.B & 0x2) >> 1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x49:
				{
					Flags.Z = (((Registers.C & 0x2) >> 1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x4A:
				{
					Flags.Z = (((Registers.D & 0x2) >> 1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x4B:
				{
					Flags.Z = (((Registers.E & 0x2) >> 1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x4C:
				{
					Flags.Z = (((Registers.H & 0x2) >> 1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x4D:
				{
					Flags.Z = (((Registers.L & 0x2) >> 1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x4E:
				{
					Flags.Z = (((Memory.read(Registers.H << 8 | Registers.L) & 0x2) >> 1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x4F:
				{
					Flags.Z = (((Registers.A & 0x2) >> 1) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x50:
				{
					Flags.Z = (((Registers.B & 0x4) >> 2) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x51:
				{
					Flags.Z = (((Registers.C & 0x4) >> 2) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x52:
				{
					Flags.Z = (((Registers.D & 0x4) >> 2) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x53:
				{
					Flags.Z = (((Registers.E & 0x4) >> 2) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x54:
				{
					Flags.Z = (((Registers.H & 0x4) >> 2) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x55:
				{
					Flags.Z = (((Registers.L & 0x4) >> 2) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x56:
				{
					Flags.Z = (((Memory.read(Registers.H << 8 | Registers.L) & 0x4) >> 2) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x57:
				{
					Flags.Z = (((Registers.A & 0x4) >> 2) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x58:
				{
					Flags.Z = (((Registers.B & 0x8) >> 3) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x59:
				{
					Flags.Z = (((Registers.C & 0x8) >> 3) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x5A:
				{
					Flags.Z = (((Registers.D & 0x8) >> 3) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x5B:
				{
					Flags.Z = (((Registers.E & 0x8) >> 3) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x5C:
				{
					Flags.Z = (((Registers.H & 0x8) >> 3) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x5D:
				{
					Flags.Z = (((Registers.L & 0x8) >> 3) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x5E:
				{
					Flags.Z = (((Memory.read(Registers.H << 8 | Registers.L) & 0x8) >> 3) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x5F:
				{
					Flags.Z = (((Registers.A & 0x8) >> 3) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x60:
				{
					Flags.Z = (((Registers.B & 0x10) >> 4) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x61:
				{
					Flags.Z = (((Registers.C & 0x10) >> 4) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x62:
				{
					Flags.Z = (((Registers.D & 0x10) >> 4) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x63:
				{
					Flags.Z = (((Registers.E & 0x10) >> 4) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x64:
				{
					Flags.Z = (((Registers.H & 0x10) >> 4) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x65:
				{
					Flags.Z = (((Registers.L & 0x10) >> 4) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x66:
				{
					Flags.Z = (((Memory.read(Registers.H << 8 | Registers.L) & 0x10) >> 4) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x67:
				{
					Flags.Z = (((Registers.A & 0x10) >> 4) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x68:
				{
					Flags.Z = (((Registers.B & 0x20) >> 5) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x69:
				{
					Flags.Z = (((Registers.C & 0x20) >> 5) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x6A:
				{
					Flags.Z = (((Registers.D & 0x20) >> 5) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x6B:
				{
					Flags.Z = (((Registers.E & 0x20) >> 5) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x6C:
				{
					Flags.Z = (((Registers.H & 0x20) >> 5) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x6D:
				{
					Flags.Z = (((Registers.L & 0x20) >> 5) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x6E:
				{
					Flags.Z = (((Memory.read(Registers.H << 8 | Registers.L) & 0x20) >> 5) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x6F:
				{
					Flags.Z = (((Registers.A & 0x20) >> 5) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x70:
				{
					Flags.Z = (((Registers.B & 0x40) >> 6) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x71:
				{
					Flags.Z = (((Registers.C & 0x40) >> 6) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x72:
				{
					Flags.Z = (((Registers.D & 0x40) >> 6) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x73:
				{
					Flags.Z = (((Registers.E & 0x40) >> 6) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x74:
				{
					Flags.Z = (((Registers.H & 0x40) >> 6) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x75:
				{
					Flags.Z = (((Registers.L & 0x40) >> 6) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x76:
				{
					Flags.Z = (((Memory.read(Registers.H << 8 | Registers.L) & 0x40) >> 6) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x77:
				{
					Flags.Z = (((Registers.A & 0x40) >> 6) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x78:
				{
					Flags.Z = (((Registers.B & 0x80) >> 7) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x79:
				{
					Flags.Z = (((Registers.C & 0x80) >> 7) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x7A:
				{
					Flags.Z = (((Registers.D & 0x80) >> 7) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x7B:
				{
					Flags.Z = (((Registers.E & 0x80) >> 7) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x7C:
				{
					Flags.Z = (((Registers.H & 0x80) >> 7) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x7D:
				{
					Flags.Z = (((Registers.L & 0x80) >> 7) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x7E:
				{
					Flags.Z = (((Memory.read(Registers.H << 8 | Registers.L) & 0x80) >> 7) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x7F:
				{
					Flags.Z = (((Registers.A & 0x80) >> 7) == 0);
					Flags.N = 0;
					Flags.H = 1;

					pc += 2;
					break;
				}
				case 0x80:
				{
					Registers.B &= ~(1UL << 0);
					pc += 2;
					break;
				} 
				case 0x81:
				{
					Registers.C &= ~(1UL << 0);
					pc += 2;
					break;
				} 
				case 0x82:
				{
					Registers.D &= ~(1UL << 0);
					pc += 2;
					break;
				} 
				case 0x83:
				{
					Registers.E &= ~(1UL << 0);
					pc += 2;
					break;
				} 
				case 0x84:
				{
					Registers.H &= ~(1UL << 0);
					pc += 2;
					break;
				} 
				case 0x85:
				{
					Registers.L &= ~(1UL << 0);
					pc += 2;
					break;
				} 
				case 0x86:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl &= ~(1UL << 0);
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				} 
				case 0x87:
				{
					Registers.A &= ~(1UL << 0);
					pc += 2;
					break;
				} 
				case 0x88:
				{
					Registers.B &= ~(1UL << 1);
					pc += 2;
					break;
				} 
				case 0x89:
				{
					Registers.C &= ~(1UL << 1);
					pc += 2;
					break;
				} 
				case 0x8A:
				{
					Registers.D &= ~(1UL << 1);
					pc += 2;
					break;
				} 
				case 0x8B:
				{
					Registers.E &= ~(1UL << 1);
					pc += 2;
					break;
				} 
				case 0x8C:
				{
					Registers.H &= ~(1UL << 1);
					pc += 2;
					break;
				} 
				case 0x8D:
				{
					Registers.L &= ~(1UL << 1);
					pc += 2;
					break;
				} 
				case 0x8E:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl &= ~(1UL << 1);
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				} 
				case 0x8F:
				{
					Registers.A &= ~(1UL << 1);
					pc += 2;
					break;
				} 
				case 0x90:
				{
					Registers.B &= ~(1UL << 2);
					pc += 2;
					break;
				} 
				case 0x91:
				{
					Registers.C &= ~(1UL << 2);
					pc += 2;
					break;
				} 
				case 0x92:
				{
					Registers.D &= ~(1UL << 2);
					pc += 2;
					break;
				} 
				case 0x93:
				{
					Registers.E &= ~(1UL << 2);
					pc += 2;
					break;
				} 
				case 0x94:
				{
					Registers.H &= ~(1UL << 2);
					pc += 2;
					break;
				} 
				case 0x95:
				{
					Registers.L &= ~(1UL << 2);
					pc += 2;
					break;
				} 
				case 0x96:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl &= ~(1UL << 2);
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				} 
				case 0x97:
				{
					Registers.A &= ~(1UL << 2);
					pc += 2;
					break;
				} 
				case 0x98:
				{
					Registers.B &= ~(1UL << 3);
					pc += 2;
					break;
				} 
				case 0x99:
				{
					Registers.C &= ~(1UL << 3);
					pc += 2;
					break;
				} 
				case 0x9A:
				{
					Registers.D &= ~(1UL << 3);
					pc += 2;
					break;
				} 
				case 0x9B:
				{
					Registers.E &= ~(1UL << 3);
					pc += 2;
					break;
				} 
				case 0x9C:
				{
					Registers.H &= ~(1UL << 3);
					pc += 2;
					break;
				} 
				case 0x9D:
				{
					Registers.L &= ~(1UL << 3);
					pc += 2;
					break;
				} 
				case 0x9E:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl &= ~(1UL << 3);
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				} 
				case 0x9F:
				{
					Registers.A &= ~(1UL << 3);
					pc += 2;
					break;
				} 
				case 0xA0:
				{
					Registers.B &= ~(1UL << 4);
					pc += 2;
					break;
				} 
				case 0xA1:
				{
					Registers.C &= ~(1UL << 4);
					pc += 2;
					break;
				} 
				case 0xA2:
				{
					Registers.D &= ~(1UL << 4);
					pc += 2;
					break;
				} 
				case 0xA3:
				{
					Registers.E &= ~(1UL << 4);
					pc += 2;
					break;
				} 
				case 0xA4:
				{
					Registers.H &= ~(1UL << 4);
					pc += 2;
					break;
				} 
				case 0xA5:
				{
					Registers.L &= ~(1UL << 4);
					pc += 2;
					break;
				} 
				case 0xA6:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl &= ~(1UL << 4);
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				} 
				case 0xA7:
				{
					Registers.A &= ~(1UL << 4);
					pc += 2;
					break;
				} 
				case 0xA8:
				{
					Registers.B &= ~(1UL << 5);
					pc += 2;
					break;
				} 
				case 0xA9:
				{
					Registers.C &= ~(1UL << 5);
					pc += 2;
					break;
				} 
				case 0xAA:
				{
					Registers.D &= ~(1UL << 5);
					pc += 2;
					break;
				} 
				case 0xAB:
				{
					Registers.E &= ~(1UL << 5);
					pc += 2;
					break;
				} 
				case 0xAC:
				{
					Registers.H &= ~(1UL << 5);
					pc += 2;
					break;
				} 
				case 0xAD:
				{
					Registers.L &= ~(1UL << 5);
					pc += 2;
					break;
				} 
				case 0xAE:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl &= ~(1UL << 5);
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				} 
				case 0xAF:
				{
					Registers.A &= ~(1UL << 5);
					pc += 2;
					break;
				} 
				case 0xB0:
				{
					Registers.B &= ~(1UL << 6);
					pc += 2;
					break;
				} 
				case 0xB1:
				{
					Registers.C &= ~(1UL << 6);
					pc += 2;
					break;
				} 
				case 0xB2:
				{
					Registers.D &= ~(1UL << 6);
					pc += 2;
					break;
				} 
				case 0xB3:
				{
					Registers.E &= ~(1UL << 6);
					pc += 2;
					break;
				} 
				case 0xB4:
				{
					Registers.H &= ~(1UL << 6);
					pc += 2;
					break;
				} 
				case 0xB5:
				{
					Registers.L &= ~(1UL << 6);
					pc += 2;
					break;
				} 
				case 0xB6:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl &= ~(1UL << 6);
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				} 
				case 0xB7:
				{
					Registers.A &= ~(1UL << 6);
					pc += 2;
					break;
				} 
				case 0xB8:
				{
					Registers.B &= ~(1UL << 7);
					pc += 2;
					break;
				} 
				case 0xB9:
				{
					Registers.C &= ~(1UL << 7);
					pc += 2;
					break;
				} 
				case 0xBA:
				{
					Registers.D &= ~(1UL << 7);
					pc += 2;
					break;
				} 
				case 0xBB:
				{
					Registers.E &= ~(1UL << 7);
					pc += 2;
					break;
				} 
				case 0xBC:
				{
					Registers.H &= ~(1UL << 7);
					pc += 2;
					break;
				} 
				case 0xBD:
				{
					Registers.L &= ~(1UL << 7);
					pc += 2;
					break;
				} 
				case 0xBE:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl &= ~(1UL << 7);
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				} 
				case 0xBF:
				{
					Registers.A &= ~(1UL << 7);
					pc += 2;
					break;
				} 
				case 0xC0:
				{
					Registers.B |= 0x1;
					pc += 2;
					break;
				}
				case 0xC1:
				{
					Registers.C |= 0x1;
					pc += 2;
					break;
				}
				case 0xC2:
				{
					Registers.D |= 0x1;
					pc += 2;
					break;
				}
				case 0xC3:
				{
					Registers.E |= 0x1;
					pc += 2;
					break;
				}
				case 0xC4:
				{
					Registers.H |= 0x1;
					pc += 2;
					break;
				}
				case 0xC5:
				{
					Registers.L |= 0x1;
					pc += 2;
					break;
				}
				case 0xC6:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl |= 0x1;
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				}
				case 0xC7:
				{
					Registers.A |= 0x1;
					pc += 2;
					break;
				}
				case 0xC8:
				{
					Registers.B |= 0x2;
					pc += 2;
					break;
				}
				case 0xC9:
				{
					Registers.C |= 0x2;
					pc += 2;
					break;
				}
				case 0xCA:
				{
					Registers.D |= 0x2;
					pc += 2;
					break;
				}
				case 0xCB:
				{
					Registers.E |= 0x2;
					pc += 2;
					break;
				}
				case 0xCC:
				{
					Registers.H |= 0x2;
					pc += 2;
					break;
				}
				case 0xCD:
				{
					Registers.L |= 0x2;
					pc += 2;
					break;
				}
				case 0xCE:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl |= 0x2;
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				}
				case 0xCF:
				{
					Registers.A |= 0x2;
					pc += 2;
					break;
				}
				case 0xD0:
				{
					Registers.B |= 0x4;
					pc += 2;
					break;
				}
				case 0xD1:
				{
					Registers.C |= 0x4;
					pc += 2;
					break;
				}
				case 0xD2:
				{
					Registers.D |= 0x4;
					pc += 2;
					break;
				}
				case 0xD3:
				{
					Registers.E |= 0x4;
					pc += 2;
					break;
				}
				case 0xD4:
				{
					Registers.H |= 0x4;
					pc += 2;
					break;
				}
				case 0xD5:
				{
					Registers.L |= 0x4;
					pc += 2;
					break;
				}
				case 0xD6:
				{
					Memory.cycle_count -= 4;
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl |= 0x4;
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				}
				case 0xD7:
				{
					Registers.A |= 0x4;
					pc += 2;
					break;
				}
				case 0xD8:
				{
					Registers.B |= 0x8;
					pc += 2;
					break;
				}
				case 0xD9:
				{
					Registers.C |= 0x8;
					pc += 2;
					break;
				}
				case 0xDA:
				{
					Registers.D |= 0x8;
					pc += 2;
					break;
				}
				case 0xDB:
				{
					Registers.E |= 0x8;
					pc += 2;
					break;
				}
				case 0xDC:
				{
					Registers.H |= 0x8;
					pc += 2;
					break;
				}
				case 0xDD:
				{
					Registers.L |= 0x8;
					pc += 2;
					break;
				}
				case 0xDE:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl |= 0x8;
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				}
				case 0xDF:
				{
					Registers.A |= 0x8;
					pc += 2;
					break;
				}
				case 0xE0:
				{
					Registers.B |= 0x10;
					pc += 2;
					break;
				}
				case 0xE1:
				{
					Registers.C |= 0x10;
					pc += 2;
					break;
				}
				case 0xE2:
				{
					Registers.D |= 0x10;
					pc += 2;
					break;
				}
				case 0xE3:
				{
					Registers.E |= 0x10;
					pc += 2;
					break;
				}
				case 0xE4:
				{
					Registers.H |= 0x10;
					pc += 2;
					break;
				}
				case 0xE5:
				{
					Registers.L |= 0x10;
					pc += 2;
					break;
				}
				case 0xE6:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl |= 0x10;
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				}
				case 0xE7:
				{
					Registers.A |= 0x10;
					pc += 2;
					break;
				}
				case 0xE8:
				{
					Registers.B |= 0x20;
					pc += 2;
					break;
				}
				case 0xE9:
				{
					Registers.C |= 0x20;
					pc += 2;
					break;
				}
				case 0xEA:
				{
					Registers.D |= 0x20;
					pc += 2;
					break;
				}
				case 0xEB:
				{
					Registers.E |= 0x20;
					pc += 2;
					break;
				}
				case 0xEC:
				{
					Registers.H |= 0x20;
					pc += 2;
					break;
				}
				case 0xED:
				{
					Registers.L |= 0x20;
					pc += 2;
					break;
				}
				case 0xEE:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl |= 0x20;
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				}
				case 0xEF:
				{
					Registers.A |= 0x20;
					pc += 2;
					break;
				}
				case 0xF0:
				{
					Registers.B |= 0x40;
					pc += 2;
					break;
				}
				case 0xF1:
				{
					Registers.C |= 0x40;
					pc += 2;
					break;
				}
				case 0xF2:
				{
					Registers.D |= 0x40;
					pc += 2;
					break;
				}
				case 0xF3:
				{
					Registers.E |= 0x40;
					pc += 2;
					break;
				}
				case 0xF4:
				{
					Registers.H |= 0x40;
					pc += 2;
					break;
				}
				case 0xF5:
				{
					Registers.L |= 0x40;
					pc += 2;
					break;
				}
				case 0xF6:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl |= 0x40;
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				}
				case 0xF7:
				{
					Registers.A |= 0x40;
					pc += 2;
					break;
				}
				case 0xF8:
				{
					Registers.B |= 0x80;
					pc += 2;
					break;
				}
				case 0xF9:
				{
					Registers.C |= 0x80;
					pc += 2;
					break;
				}
				case 0xFA:
				{
					Registers.D |= 0x80;
					pc += 2;
					break;
				}
				case 0xFB:
				{
					Registers.E |= 0x80;
					pc += 2;
					break;
				}
				case 0xFC:
				{
					Registers.H |= 0x80;
					pc += 2;
					break;
				}
				case 0xFD:
				{
					Registers.L |= 0x80;
					pc += 2;
					break;
				}
				case 0xFE:
				{
					std::uint8_t hl = Memory.read(Registers.H << 8 | Registers.L);
					hl |= 0x80;
					Memory.write(Registers.H << 8 | Registers.L, hl);
					pc += 2;
					break;
				}
				case 0xFF:
				{
					Registers.A |= 0x80; 
					pc += 2;
					break;
				}
				default:
				{
					std::cout << std::hex <<(int)Memory.read(pc)  << " " << (int)Memory.read(pc + 1) << '\n';
					return;
				}
			}
			break;
		}
		case 0xCC: // sus
		{
			Memory.cycle_count += 8;
			if(Flags.Z == 1)
			{
				Memory.write(--sp, (pc + 3) >> 8);
				Memory.write(--sp, (pc + 3) & 0xFF);
				pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
				break;
			}
			pc += 3;
			break;
		}
		case 0xCD: //sus
		{
			Memory.cycle_count += 4;
			Memory.write(--sp, (pc + 3) >> 8);
			Memory.write(--sp, (pc + 3) & 0xFF);
			pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
			break;
		}
		case 0xCE:
		{
			std::uint8_t res = Registers.A + Memory.read(pc + 1) + Flags.C;
			//std::uint8_t tmp = Memory.read(pc + 1);

			Flags.H = ((((Registers.A & 0xf) + (Memory.read(pc + 1) & 0xf) + Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) + (Memory.read(pc + 1) & 0xff) + Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Flags.N = 0;

			Registers.A = res;    
			pc += 2;
			break;					
		}
		case 0xCF:
		{
			pc++;
			Memory.write(--sp,pc >> 8);
			Memory.write(--sp,pc & 0xFF);
			pc = 0x0000 + 0x08;
			break;
		}
		case 0xD0:
		{
			if(Flags.C == 0)
			{
				std::uint16_t low;
				std::uint16_t hi;
				low = Memory.read(sp++);
				hi = Memory.read(sp++);
				pc = hi << 8 | low;
				break; 
			}
			pc++;
			break;
		}
		case 0xD1:
		{
			Registers.E = Memory.read(sp++);
			Registers.D = Memory.read(sp++);
			pc++;
			break;
		}
		case 0xD2:
		{
			if(Flags.C == 0)
			{
				pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
				break;
			}
			pc += 3;
			break;
		}
		case 0xD4:
		{
			Memory.cycle_count += 8;
			if(Flags.C == 0)
			{
				Memory.write(--sp, (pc + 3) >> 8);
				Memory.write(--sp, (pc + 3) & 0xFF);
				pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
				break;
			}
			pc += 3;
			break;
		
		}
		case 0xD5:
		{
			Memory.write(--sp,Registers.D);
			Memory.write(--sp,Registers.E);
			pc++;
			break;
		}
		case 0xD6: //have i solved the mistery
		{
			std::uint8_t res = Registers.A - Memory.read(pc + 1);
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			
			if(Registers.A < Memory.read(pc + 1))	
				Flags.C = 1;
			else
			{
				Flags.C = 0;
			}
			if((((Registers.A & 0xf) - (Memory.read(pc + 1) & 0xf)) & 0x10) == 0x10)	
				Flags.H = 1;
			else
			{
				Flags.H = 0;
			}
			Flags.N = 1;
			Registers.A = res;
			pc += 2;
			break;	
					
		}	
		case 0xD7:
		{
			pc++;
			Memory.write(--sp,pc >> 8);
			Memory.write(--sp,pc & 0xFF);
			pc = 0x0000 + 0x10;
			break;
		}
		case 0xD8:
		{
			if(Flags.C == 1)
			{
				std::uint16_t low;
				std::uint16_t hi;
				low = Memory.read(sp++);
				hi = Memory.read(sp++);
				pc = hi << 8 | low;
				break; 
			}
			pc++;
			break;
		}
		case 0xD9:
		{
			std::uint8_t low;
			std::uint8_t hi;
			low = Memory.read(sp++);
			hi = Memory.read(sp++);
			IME = true;
			pc = hi << 8 | low;
			break; 
		}
		case 0xDA:
		{
			if(Flags.C == 1)
			{
				pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
				break;
			}
			pc += 3;
			break;
		}
		case 0xDC: // sus
		{
			Memory.cycle_count += 8;
			if(Flags.C == 1)
			{
				Memory.write(--sp, (pc + 3) >> 8);
				Memory.write(--sp, (pc + 3) & 0xFF);
				pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
				break;
			}
			pc += 3;
			break;
		}
		case 0xDE:
		{
			std::uint8_t res = Registers.A - Memory.read(pc + 1) - Flags.C;
			
			Flags.N = 1;
			Flags.H = ((((Registers.A & 0xf) - (Memory.read(pc + 1) & 0xf) - Flags.C) & 0x10) == 0x10);
			Flags.C = ((((Registers.A & 0xff) - (Memory.read(pc + 1) & 0xff) - Flags.C) & 0x100) == 0x100);
			Flags.Z = (res == 0);
			Registers.A = res;
			
			pc += 2;
			break;
		}
		case 0xDF:
		{
			pc++;
			Memory.write(--sp,pc >> 8);
			Memory.write(--sp,pc & 0xFF);
			pc = 0x0000 + 0x18;
			break;
		}
		case 0xE0:
		{
			if(0xFF00 + Memory.read(pc + 1) == 0xFF50)
			{
				Memory.boot_enabled = 0;
			}
				
			Memory.write(0xFF00 + Memory.read(pc + 1), Registers.A);
			pc += 2;
			break;
		}
		case 0xE1:
		{
			Registers.L = Memory.read(sp++);
			Registers.H = Memory.read(sp++);
			pc += 1;
			break;
		}
		case 0xE2:
		{
			Memory.write(0xFF00 + Registers.C,Registers.A);
			pc++;
			break;
		}
		case 0xE5:
		{
			Memory.write(--sp,Registers.H);
			Memory.write(--sp,Registers.L);
			pc++;
			break;
		}
		case 0xE6:
		{
			Registers.A &= Memory.read(pc + 1);
			Flags.N = 0;
			Flags.H = 1;
			Flags.C = 0;
			if(Registers.A == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			pc += 2;
			break;		
		}
		case 0xE7:
		{
			pc++;
			Memory.write(--sp,pc >> 8);
			Memory.write(--sp,pc & 0xFF);
			pc = 0x0000 + 0x20;
			break;
		}
		case 0xE8:
		{
			//(((a&0xfff) + (b&0xfff)) & 0x1000) == 0x10 //11-12 carry
			//(((a&0xffff) + (b&0xffff)) & 0x10000) == 0x10 //15-16 carry
			std::int8_t i8 = Memory.read(pc + 1);
			Flags.N = 0;
			Flags.Z = 0;
			Flags.H = ((((sp & 0xf) + (i8 & 0xf)) & 0x10) == 0x10);
			Flags.C = ((((sp & 0xff) + (i8 & 0xff)) & 0x100) == 0x100);
			sp += i8;
			pc += 2;
			break;
		}
		case 0xE9:
		{
			pc = Registers.H << 8 | Registers.L;
			//pc++;
			break;
		}
		case 0xEA:
		{
			
			std::uint16_t u16 = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
			Memory.write(u16, Registers.A);
			//memory[(pc+1) << 8 | pc+2] = Registers.A;
			pc += 3;
			break;
		}
		case 0xEE:
		{
			if((Registers.A ^ Memory.read(pc + 1)) == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A ^= Memory.read(pc + 1);
			pc += 2;
			break;	
		}
		case 0xEF:
		{
			pc++;
			Memory.write(--sp,pc >> 8);
			Memory.write(--sp,pc & 0xFF);
			pc = 0x0028;
			break;
		}
		case 0xF0:
		{
			Registers.A = Memory.read(0xFF00 + Memory.read(pc + 1));
			pc += 2;
			break;
		}
		case 0xF1:
		{
			Registers.F = Memory.read(sp++);
			Registers.A = Memory.read(sp++);
			Flags.Z = (Registers.F & 0x80) >> 7;
			Flags.N = (Registers.F & 0x40) >> 6;
			Flags.H = (Registers.F & 0x20) >> 5;
			Flags.C = (Registers.F & 0x10) >> 4;

			pc++;
			break;
		}
		case 0xF2:
		{
			std::uint16_t test = 0xFF00 + Registers.C;
			Registers.A = Memory.read(test);
			pc++;
			break;
		}
		case 0xF3: //badish
		{
			tmp = false;
			handled = 0;
			pc++;
			break;
		}
		case 0xF5: 
		{
			Memory.write(--sp,Registers.A);
			Memory.write(--sp,Registers.F);
			pc++;
			break;
		}
		case 0xF6:
		{
			if((Registers.A | Memory.read(pc + 1)) == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.C = 0;
			Flags.N = 0;
			Flags.H = 0;
			Registers.A |= Memory.read(pc + 1);
			pc += 2;
			break;	
		}
		case 0xF7: //maybe
		{
			pc++;
			Memory.write(--sp,pc >> 8);
			Memory.write(--sp,pc & 0xFF);
			pc = 0x0000 + 0x30;
			break;
		}
		case 0xF8:
		{
			std::uint16_t hl;
			std::int8_t i8 = Memory.read(pc + 1);
			Flags.N = 0;
			Flags.Z = 0;
			Flags.H = ((((sp & 0xf) + (i8 & 0xf)) & 0x10) == 0x10);
			Flags.C = ((((sp & 0xff) + (i8 & 0xff)) & 0x100) == 0x100);
			hl = Registers.H << 8 | Registers.L;
			hl = sp + i8;
			Registers.H = hl >> 8;
			Registers.L = hl & 0xFF;
			pc += 2;
			break;

		}
		case 0xF9:
		{
			sp = Registers.H << 8 | Registers.L;
			pc++;
			break;
		}
		case 0xFA:
		{
			Registers.A = Memory.read(Memory.read(pc + 2) << 8 | Memory.read(pc + 1));
			pc += 3;
			break;
		}
		
		case 0xFB: //badish
		{
			tmp = true;
			handled = 0;
			pc++;
			break;
		}
		
		case 0xFE:
		{
			std::uint8_t val = Memory.read(pc + 1);
			Flags.N = 1;
			Flags.C = (Registers.A < val);
			Flags.Z = (Registers.A == val);
			Flags.H = (((Registers.A & 0xf) < (val & 0xf)));
			pc += 2;
			break;					
		}
		case 0xFF: //bad
		{
			pc++;
			Memory.write(--sp,pc >> 8);
			Memory.write(--sp,pc & 0xFF);
			pc = 0x0000 + 0x38;
			break;
		}
		default:
		{
			std::cout << std::hex <<(int)pc<<": " << (int)Memory.read(pc) <<"! " << std::endl;
			return;
		}	
	}
}