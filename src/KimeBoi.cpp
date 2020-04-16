#include "KimeBoi.hpp"
#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <fstream>
bool tmp = 0;
int t = 0;
unsigned char Processor::_Memory::read(unsigned short address)
{
	if(boot_enabled && address < 0x100)
		return boot[address];
	if (address < 0x8000)
		return rom[address];
	return ram[address - 0x8000];
}

void Processor::_Memory::write(unsigned short address, unsigned char value)
{
	if (address <= 0x8000)
	{
		std::cout << "Illegal writting at: " << address << std::endl;
		return;
	}
	if(address == 0xFF01)
		std::cout << "Blarg's Output: " << (char)value << '\n';
	
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
	Memory.rom.resize(32768);
	for(int i = 0; i < 32768; i++)
		Memory.rom[i] = 0; 
	for(int i = 0; i < 32768; i++)
		Memory.ram[i] = 0; 
	//Memory.write(0xFF44, 144);	
}

void Processor::emulateCycle(std::string &output)
{
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
	
 	if(tmp != interupts)
	{
		t++;
		if(t == 2)
		{
			t = 0;
			interupts = tmp;
		}

	}
	if(Memory.read(pc) != 0)
	 pc = pc;
	switch (Memory.read(pc))
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
			bool msb = (Registers.A >> 8) & 1;
			Flags.C = (Registers.A >> 7) & 1;
			Registers.A = (Registers.A << 1) | msb;
			if(Registers.A == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;
				
			Flags.N = 0;
			Flags.H = 0;
			Flags.C = 0;
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
			Registers.H = res << 8;
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
			std:uint16_t res = Registers.B << 8 | Registers.C;
			res--;
			Registers.B = res << 8;
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
			bool msb = Registers.A & 0x1;
			Registers.A >>=1;
			Registers.A = msb << 7 | Registers.A;
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
			std::int8_t add = Memory.read(pc + 1);
			pc += 2;
			pc = pc + add;
			break;
			
		} 
		case 0x19:
		{
			Flags.N = 0;
			std::uint16_t hl,de,res;
			hl = Registers.H << 8 | Registers.L;
			de = Registers.D << 8 | Registers.E;
			res = de + hl;

			if((((hl & 0xfff) + (de &0xfff)) & 0x1000) == 0x1000)
				Flags.H = 1;
			else
				Flags.H = 0;

			if((((hl&0xffff) + (de & 0xffff)) & 0x1000) == 0x10000 )
				Flags.C = 1;
			else
				Flags.C = 0;	
			//implement flags ....
			hl = res;
			Registers.H = res << 8;
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
			
			if (Flags.Z == 0)
			{
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
		case 0x28: //good
		{
			if(Flags.Z == 1)
			{
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
			Flags.N = 1;
			std::uint16_t hl = Registers.H << 8 | Registers.L;
			if((((hl & 0xfff) + (hl & 0xfff)) & 0x1000) == 0x1000)
				Flags.H = 1;
			else 
				Flags.H = 0;
			if((((hl & 0xffff) + (hl & 0xffff)) & 0x10000) == 0x10000)	
				Flags.C = 1;
			else 
				Flags.C = 0;
			hl += hl;
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
			unsigned short result = (Registers.H << 8 | Registers.L) + sp;
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
			Registers.C = Registers.H; 
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
		
		case 0x8D:
		{
			Flags.N = 0;
			std::uint8_t res = Registers.A + Registers.L + Flags.C;
			std::uint8_t tmp = Registers.L + Flags.C;
			if((((Registers.A & 0xf) + (tmp & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else 
				Flags.H = 0;
			if((((Registers.A & 0xff) + (tmp & 0xff)) & 0x100) == 0x100)
				Flags.C = 1;
			else 
				Flags.C = 0;
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Registers.A = res;	
			pc += 2;
			break;
		}
		case 0x8E:
		{
			Flags.N = 0;
			std::uint8_t res = Registers.A + Memory.read(Registers.H << 8 | Registers.L) + Flags.C;
			std::uint8_t tmp = Memory.read(Registers.H << 8 | Registers.L) + Flags.C;
			if((((Registers.A & 0xf) + (tmp & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else 
				Flags.H = 0;
			if((((Registers.A & 0xff) + (tmp & 0xff)) & 0x100) == 0x100)
				Flags.C = 1;
			else 
				Flags.C = 0;
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Registers.A = res;	
			pc += 2;
			break;
		}
		case 0x90:
		{
			Flags.N = 1;
			//other flags
			if(Registers.A - Registers.B == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.A & 0xf) - (Registers.B & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xff) - (Registers.B & 0xff)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;	
			Registers.A -= Registers.B;
			pc++;
			break;
		}
		case 0x96:
		{
			Flags.N = 1;
			//other flags
			if(Registers.A - Memory.read(Registers.H << 8 | Registers.L) == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
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

		case 0x9B:
		{
			std::uint8_t sub = Registers.E - Flags.C;
			Flags.N = 1;
			//other flags
			if(Registers.A - sub == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.A & 0xf) - (sub & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xff) - (sub & 0xff)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;	
			Registers.A -= sub;
			pc++;
			break;
		}
		case 0x9C:
		{
			std::uint8_t sub = Registers.H - Flags.C;
			Flags.N = 1;
			//other flags
			if(Registers.A - sub == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.A & 0xf) - (sub & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xff) - (sub & 0xff)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;	
			Registers.A -= sub;
			pc++;
			break;
		}
		case 0x9E:
		{
			std::uint8_t sub = Memory.read(Registers.H << 8 | Registers.L) - Flags.C;
			Flags.N = 1;
			//other flags
			if(Registers.A - sub == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.A & 0xf) - (sub & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xff) - (sub & 0xff)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;	
			Registers.A -= sub;
			pc++;
			break;
		}
		case 0xA1: //fixed
		{
			std::uint8_t res = Registers.A & Registers.C;
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
		case 0xA9:
		{
			if((Registers.A ^ Registers.C) == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Flags.N = 0;
			Flags.C = 0;
			Flags.H = 0;
			Registers.A ^= Registers.C;
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
			if(Flags.Z == 1)
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
			std::uint16_t result = Registers.A + Memory.read(pc + 1);
			Flags.N = 0;
			if(result == 0)
				Flags.Z = 1;
			else
			{
				Flags.Z = 0;
			}
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
		case 0xCB:
		{
			switch (Memory.read(pc + 1))
			{
				
				case 0x11:
				{
					bool msb = (Registers.C >> 7) & 1;
					bool nlsb = Flags.C;
					Flags.C = msb;
					Registers.C = (Registers.C << 1) | nlsb;
					if(Registers.C == 0)
						Flags.Z = 1;
					else
						Flags.Z = 0;
						
					Flags.N = 0;
					Flags.H = 0;
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
				case 0x7C:
				{
	
					std::uint8_t tmp;
					tmp = Registers.H >> 7;
					tmp <<= 1;
					tmp >>= 8;

					if(tmp == 0)
						Flags.Z = 1;
					else 
						Flags.Z = 0;
					Flags.N = 0;
					Flags.H = 1;
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
			if(Flags.Z == 1)
			{
				Memory.write(--sp, (pc + 3) >> 8);
				Memory.write(--sp, (pc + 3) & 0xFF);
				pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
				break;
			}
			pc += 2;
			break;
		}
		case 0xCD: //sus
		{
			Memory.write(--sp, (pc + 3) >> 8);
			Memory.write(--sp, (pc + 3) & 0xFF);
			pc = Memory.read(pc + 2) << 8 | Memory.read(pc + 1);
			break;
		}
		case 0xCE:
		{
			Flags.N = 0;
			std::uint8_t res = Registers.A + Memory.read(pc + 1) + Flags.C;
			std::uint8_t tmp = Memory.read(pc + 1) + Flags.C;
			if((((Registers.A & 0xf) + (tmp & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else 
				Flags.H = 0;
			if((((Registers.A & 0xff) + (tmp & 0xff)) & 0x100) == 0x100)
				Flags.C = 1;
			else 
				Flags.C = 0;
			if(res == 0)
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			Registers.A = res;	
			pc += 2;
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
		case 0xDE:
		{
			std::uint8_t sub = Memory.read(pc + 1) - Flags.C;
			Flags.N = 1;
			//other flags
			if(Registers.A - sub == 0)
				Flags.Z = 1;
			else
				Flags.Z = 0;	
			if((((Registers.A & 0xf) - (sub & 0xf)) & 0x10) == 0x10)
				Flags.H = 1;
			else
				Flags.H = 0;
			if((((Registers.A & 0xff) - (sub & 0xff)) & 0x100) == 0x100)
				Flags.C = 1;
			else
				Flags.C = 0;	
			Registers.A -= sub;
			pc++;
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
			pc++;
			break;		
		}
		case 0xE8:
		{
			std::int8_t i8 = Memory.read(pc + 1);
			Flags.N = 0;
			Flags.Z = 0;
			sp += i8;
			pc++;
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

			if(pc == 0xc7f2)
				pc = pc;
			Flags.Z = (Registers.F & 0x80) >> 7;
			Flags.N = (Registers.F & 0x40) >> 6;
			Flags.H = (Registers.F & 0x20) >> 5;
			Flags.C = (Registers.F & 0x10) >> 4;

			pc++;
			break;
		}
		case 0xF2:
		{
			Registers.A = Memory.read(0xFF00 + Flags.C);
			pc++;
			break;
		}
		case 0xF3: //badish
		{
			tmp = false;
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
			Memory.write(--sp,pc >> 8);
			Memory.write(--sp,pc & 0xff);
			pc = 0x0000 + 0x30;
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
			pc++;
			break;
		}
		
		case 0xFE:
		{
			Flags.N = 1;
			if(Registers.A < Memory.read(pc + 1))
				Flags.C = 1;
			else 
				Flags.C = 0;
			if(Registers.A == Memory.read(pc + 1))
				Flags.Z = 1;
			else 
				Flags.Z = 0;
			
			if((((Registers.A & 0xf) - (Memory.read(pc + 1) & 0xf)) == 0x10))
				Flags.H = 1;
			else 
				Flags.H = 0;
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