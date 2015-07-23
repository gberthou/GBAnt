#include <iostream>
#include <fstream>

#include "GBAcpu.h"

#define GBA_ROM_LOADER_BUF_SIZE 1024

const u_int32_t GBA_BASE_ADDRESS = 0x08000000;

GBAcpu::GBAcpu():
	ARMcpu(GBA_BASE_ADDRESS)
{
	mem.AddMemory(new PhysicalMemory(0x02000000, 0x0203FFFF)); // WRAM - 256 kB
	mem.AddMemory(new PhysicalMemory(0x03000000, 0x03007FFF)); // WRAM - 32 kB
	mem.AddMemory(new PhysicalMemory(0x05000000, 0x050003FF)); // BG - 1 kB
	mem.AddMemory(new PhysicalMemory(0x06000000, 0x06017FFF)); // VRAM - 96 kB
	mem.AddMemory(new PhysicalMemory(0x07000000, 0x070003FF)); //OAM - 1 kB
	mem.AddMemory(cartridge = new PhysicalMemory(0x08000000, 0x09FFFFFF)); // Cartridge - 32 MB

	/* ### IO ### */

	mem.AddMemory(new PhysicalMemory(0x040000B0, 0x040000E3)); // DMA
	mem.AddMemory(new PhysicalMemory(0x04000200, 0x040003FE)); // Interrupt, waitsait
}

GBAcpu::~GBAcpu()
{
}

bool GBAcpu::LoadGBA(const char *filename)
{
	if(cartridge)
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		if(file.is_open())
		{
			u_int32_t address = GBA_BASE_ADDRESS;
			char buf[GBA_ROM_LOADER_BUF_SIZE];
			bool ok = true;
			while(!file.eof() && ok)
			{
				file.read(buf, GBA_ROM_LOADER_BUF_SIZE);
				ok = cartridge->CopyToMemory(address, buf, GBA_ROM_LOADER_BUF_SIZE);
				if(!ok)
				{
					std::cerr << "GBA ROM is too large and exceeds allocated GamePak memory" << std::endl;
				}
				address += GBA_ROM_LOADER_BUF_SIZE;
			}
			file.close();
			return ok;
		}

		std::cerr << "Cannot read GBA ROM file: " << filename << std::endl;
		std::cerr << "Please check that the file exist or that user has correct access rights" << std::endl;
		return false;
	}

	std::cerr << "Cannot load GBA ROM file because cartridge memory is not allocated..." << std::endl;
	return false;
}

void GBAcpu::Run(void)
{
	regSet.SetMode(RS_USER);
	regSet.SetValue(PC, GBA_BASE_ADDRESS);
	regSet.SetValue(SP, 0); // TODO: change value
	regSet.SetValue(CPSR, 0); // TODO: change value
	ARMcpu::Run();
}

