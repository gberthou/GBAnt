#include <iostream>
#include <fstream>

#include "GBAcpu.h"
#include "../peripherals/peripherals.h"

#define GBA_ROM_LOADER_BUF_SIZE 1024

const u_int32_t GBA_BASE_ADDRESS = 0x08000000;
const u_int32_t GBA_IME          = 0x04000208;
const u_int32_t GBA_IE           = 0x04000200;
const u_int32_t GBA_IF           = 0x04000202;

GBAcpu::GBAcpu():
	ARMcpu(GBA_BASE_ADDRESS)
{
	mem.AddMemory(new PhysicalMemory(0x00000018, 0x00003FFF)); // BIOS - 16 kB
	mem.AddMemory(new PhysicalMemory(0x02000000, 0x0203FFFF)); // WRAM - 256 kB
	mem.AddMemory(new PhysicalMemory(0x03000000, 0x03007FFF)); // WRAM - 32 kB
	mem.AddMemory(new PhysicalMemory(0x05000000, 0x050003FF)); // BG - 1 kB
	mem.AddMemory(new PhysicalMemory(0x06000000, 0x06017FFF)); // VRAM - 96 kB
	mem.AddMemory(new PhysicalMemory(0x07000000, 0x070003FF)); //OAM - 1 kB
	mem.AddMemory(cartridge = new PhysicalMemory(0x08000000, 0x09FFFFFF)); // Cartridge - 32 MB
	mem.AddMemory(new PhysicalMemory(0x0E000000, 0x0E00FFFF)); // SRAM 64 kB

	/* ### IO ### */

	mem.AddMemory(lcd = new Lcd(0x04000000, 0x04000057));
	mem.AddMemory(new Sound(0x04000060, 0x040000AB));
	mem.AddMemory(new DMA  (0x040000B0, 0x040000E3, this));
	mem.AddMemory(new Timer(0x04000100, 0x0400010F));
	mem.AddMemory(new PhysicalMemory(0x04000200, 0x040003FE)); // Interrupt, waitsait

	/* ### Initialize BIOS memory */
	mem.Write(0x00000018, 0xeafffffe);
	mem.Write(0x00000128, 0xe92d500f);
	mem.Write(0x0000012C, 0xe3a00301);
	mem.Write(0x00000130, 0xe08ee00f);
	mem.Write(0x00000134, 0xe510f004);
	mem.Write(0x00000138, 0xe8bd500f);
	mem.Write(0x0000013C, 0xe25ef004);
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

void GBAcpu::TriggerInterrupt(GBA_InterruptSource source)
{
	u_int32_t ie;
	mem.Read(GBA_IE, ie);
	if((ie >> source) & 1) // Interrupts are enabled for this particular source
	{
		u_int32_t ifvalue;
		mem.Read(GBA_IF, ifvalue);
		ifvalue |= (1 << source);
		mem.Write(GBA_IF, ifvalue); // Update interrupt flags
	}
}

void GBAcpu::onClock(void)
{
	ARMcpu::onClock();
	
	lcd->OnClock();
}

bool GBAcpu::interruptsEnabled(void)
{
	u_int32_t ime;
	mem.Read(GBA_IME, ime);
	return (ime & 1) != 0;
}

