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
	mem.AddMemory(new Keys(0x04000130, 0x04000133));
	mem.AddMemory(new PhysicalMemory(0x04000200, 0x040003FE)); // Interrupt, waitsait

	/* ### Initialize BIOS memory */
	//mem.Write(0x00000018, 0xe28ffe11);
	mem.Write(0x00000018, 0xea000042);
	mem.Write(0x00000128, 0xe92d500f);
	mem.Write(0x0000012C, 0xe59f000c);
	mem.Write(0x00000130, 0xe08ee00f);
	mem.Write(0x00000134, 0xe510f004);
	mem.Write(0x00000138, 0xe8bd500f);
	mem.Write(0x0000013C, 0xe25ef004);
	mem.Write(0x00000140, 0x03008000);
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
	regSet.SetValue(PC, GBA_BASE_ADDRESS, 0);
	regSet.SetValue(SP, 0, 0); // TODO: change value
	regSet.SetValue(CPSR, 0, 0); // TODO: change value
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

void GBAcpu::runStep(void)
{
	if(interruptsEnabled())
	{
		// TODO: Check all interrupt sources
	
		// LCD
		if(lcd->MustTriggerInterrupt(GBA_IS_LCD_VBLANK))
		{
			std::cout << "VBLANK IRQ HAPPENS NOW!" << std::endl;
			regSet.SetMode(RS_IRQ);
			thumbMode = false;
			regSet.SetValue(PC, 0x18, 0);
		}
		if(lcd->MustTriggerInterrupt(GBA_IS_LCD_HBLANK))
		{
			std::cout << "HBLANK IRQ HAPPENS NOW!" << std::endl;
		}
		if(lcd->MustTriggerInterrupt(GBA_IS_LCD_VCOUNTER_MATCH))
		{
			std::cout << "VCOUNTER IRQ HAPPENS NOW!" << std::endl;
		}
	}

	ARMcpu::runStep();
}

void GBAcpu::onClock(void)
{
	ARMcpu::onClock();
	
	lcd->OnClock();
}

bool GBAcpu::interruptsEnabled(void)
{
	if(ARMcpu::interruptsEnabled())
	{
		u_int32_t ime;
		mem.Read(GBA_IME, ime);
		return (ime & 1) != 0;
	}
	return false;
}

bool GBAcpu::RunTestStack(void)
{
	const u_int32_t INIT_SP = 0x3000020;
	
	bool ret = true;
	u_int32_t spv = INIT_SP;
	u_int32_t pcv;
	u_int32_t values[3];
	DataWrapper wrapper(&pcv);

	regSet.SetValue(SP, spv, 0);
	regSet.SetValue(R0, 0x42, 0);
	regSet.SetValue(R1, 0x28, 0);
	regSet.SetValue(R2, 0x21, 0);
	regSet.SetValue(R3, 0x12, 0);
	regSet.SetValue(R4, 0x13, 0);
	regSet.SetValue(R5, 0x14, 0);

	for(unsigned int i = 12; i; --i)
	{
		mem.Write(spv - 12 + 4 * (12 - i), i);
	}

	std::cout << ">> stmia sp!, {r0, r1, r2}" << std::endl;
	spv = loadstore(SP, 0x7, false, false, false, true, true, wrapper); // stmia sp!, {r0, r1, r2}
	regSet.SetValue(SP, spv, 0);
	if(spv != INIT_SP + 12)
	{
		std::cerr << "[ERROR] Bad sp" << std::endl;
	}
	else
	{
		for(unsigned int i = 0; i < 3; ++i)
			mem.Read(spv - 4 * (i+1), values[2-i]);
		if(values[0] != 0x42 || values[1] != 0x28 || values[2] != 0x21)
			std::cerr << "[ERROR] Values at the wrong place" << std::endl;
		else
			std::cout << "[OK]" << std::endl;
	}
	std::cout << std::endl;

	std::cout << ">> stmdb sp!, {r3, r4, r5}" << std::endl;
	spv = loadstore(SP, 0x38, false, false, false, false, false, wrapper); // stmdb sp!, {r3, r4, r5}
	regSet.SetValue(SP, spv, 0);
	if(spv != INIT_SP)
	{
		std::cerr << "[ERROR] Bad sp" << std::endl;
	}
	else
	{
		for(unsigned int i = 0; i < 3; ++i)
		{
			mem.Read(spv + 4 * i, values[i]);
			printf("%x\n", values[i]);
		}
		if(values[0] != 0x12 || values[1] != 0x13 || values[2] != 0x14)
			std::cerr << "[ERROR] Values at the wrong place" << std::endl;
		else
			std::cout << "[OK]" << std::endl;
	}
	std::cout << std::endl;
	
	std::cout << ">> ldmdb sp!, {r0, r1, r2}" << std::endl;
	spv = loadstore(SP, 0x7, false, false, true, false, false, wrapper); // ldmdb sp!, {r0, r1, r2}
	regSet.SetValue(SP, spv, 0);
	if(spv != INIT_SP - 12)
	{
		std::cerr << "[ERROR] Bad sp" << std::endl;
	}
	else
	{
		if(regSet.GetValue(R0) != 12 || regSet.GetValue(R1) != 11 || regSet.GetValue(R2) != 10)
			std::cerr << "[ERROR] Bad register values" << std::endl;
		else
		{
			for(unsigned int i = 0; i < 3; ++i)
				mem.Read(spv + 4 * i, values[i]);
			if(values[0] != 0xc || values[1] != 0xb || values[2] != 0xa)
				std::cerr << "[ERROR] Bad stack values" << std::endl;
			std::cout << "[Ok]" << std::endl;
		}
	}
	std::cout << std::endl;
	
	std::cout << ">> ldmia sp!, {r3, r4, r5}" << std::endl;
	spv = loadstore(SP, 0x38, false, false, true, true, true, wrapper); // ldmia sp!, {r0, r1, r2}
	regSet.SetValue(SP, spv, 0);
	if(spv != INIT_SP)
	{
		std::cerr << "[ERROR] Bad sp" << std::endl;
	}
	else
	{
		if(regSet.GetValue(R3) != 0xc || regSet.GetValue(R4) != 0xb || regSet.GetValue(R5) != 0xa)
			std::cerr << "[ERROR] Bad values" << std::endl;
		else
		{
			for(unsigned int i = 0; i < 3; ++i)
				mem.Read(spv + 4 * i, values[i]);
			if(values[0] != 0x12 || values[1] != 0x13 || values[2] != 0x14)
				std::cerr << "[Error] Bad stack values" << std::endl;
			else
				std::cout << "[Ok]" << std::endl;
		}
	}
	std::cout << std::endl;
	return ret;
}

