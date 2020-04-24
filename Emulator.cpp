/*--------------------------------------------------------------------------------------
			Author name: Josiah Murray
			Date started: 03/02/20
			Revision date:
			Functional description: A chimera Microproccessor Emulator
			User Advice: N/A
----------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER   "19018590"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500
#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer[MAX_BUFFER_SIZE];

char hex_file[MAX_BUFFER_SIZE];
char trc_file[MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x80
#define FLAG_N  0x20
#define FLAG_V  0x10
#define FLAG_Z  0x08
#define FLAG_C  0x01


#define REGISTER_M  7
#define REGISTER_A	6
#define REGISTER_H	5
#define REGISTER_L	4
#define REGISTER_E	3
#define REGISTER_D	2
#define REGISTER_C	1
#define REGISTER_B	0
WORD IndexRegister = 0;

BYTE Registers[8];
BYTE Flags;
WORD ProgramCounter;
WORD StackPointer;


////////////
// Memory //
////////////

#define MEMORY_SIZE	65536

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x01FA
#define TEST_ADDRESS_2  0x01FB
#define TEST_ADDRESS_3  0x01FC
#define TEST_ADDRESS_4  0x01FD
#define TEST_ADDRESS_5  0x01FE
#define TEST_ADDRESS_6  0x01FF
#define TEST_ADDRESS_7  0x0200

#define TEST_ADDRESS_8  0x0201
#define TEST_ADDRESS_9  0x0202
#define TEST_ADDRESS_10  0x0203
#define TEST_ADDRESS_11  0x0204
#define TEST_ADDRESS_12  0x0205


///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;


///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][14] =
{
"BRA rel      ",
"BCC rel      ",
"BCS rel      ",
"BNE rel      ",
"BEQ rel      ",
"BVC rel      ",
"BVS rel      ",
"BMI rel      ",
"BPL rel      ",
"BGE rel      ",
"BLE rel      ",
"BLS rel      ",
"BHI rel      ",
"ILLEGAL     ",
"RTN impl     ",
"ILLEGAL     ",

"ST abs       ",
"PSH  ,A      ",
"POP A,       ",
"ILLEGAL     ",
"ILLEGAL     ",
"CLC impl     ",
"SEC impl     ",
"CLI impl     ",
"STI impl     ",
"SEV impl     ",
"CLV impl     ",
"DEX impl     ",
"INX impl     ",
"NOP impl     ",
"WAI impl     ",
"ILLEGAL     ",

"ST abs,X     ",
"PSH  ,s      ",
"POP s,       ",
"ILLEGAL     ",
"ILLEGAL     ",
"ADI  #       ",
"SBI  #       ",
"CPI  #       ",
"ANI  #       ",
"XRI  #       ",
"MVI  #,B     ",
"MVI  #,C     ",
"MVI  #,D     ",
"MVI  #,E     ",
"MVI  #,L     ",
"MVI  #,H     ",

"ILLEGAL     ",
"PSH  ,B      ",
"POP B,       ",
"JPR abs      ",
"CCC abs      ",
"CCS abs      ",
"CNE abs      ",
"CEQ abs      ",
"CVC abs      ",
"CVS abs      ",
"CMI abs      ",
"CPL abs      ",
"CHI abs      ",
"CLE abs      ",
"ILLEGAL     ",
"ILLEGAL     ",

"ILLEGAL     ",
"PSH  ,C      ",
"POP C,       ",
"TST abs      ",
"INC abs      ",
"DEC abs      ",
"RCR abs      ",
"RCL abs      ",
"SAL abs      ",
"ASR abs      ",
"NOT abs      ",
"ROL abs      ",
"ROR abs      ",
"ILLEGAL     ",
"LDX  #       ",
"LODS  #      ",

"STOX abs     ",
"PSH  ,D      ",
"POP D,       ",
"TST abs,X    ",
"INC abs,X    ",
"DEC abs,X    ",
"RCR abs,X    ",
"RCL abs,X    ",
"SAL abs,X    ",
"ASR abs,X    ",
"NOT abs,X    ",
"ROL abs,X    ",
"ROR abs,X    ",
"ILLEGAL     ",
"LDX abs      ",
"LODS abs     ",

"STOX abs,X   ",
"PSH  ,E      ",
"POP E,       ",
"TSTA A,A     ",
"INCA A,A     ",
"DECA A,A     ",
"RCRA A,A     ",
"RCLA A,A     ",
"SALA A,A     ",
"ASRA A,A     ",
"NOTA A,A     ",
"ROLA A,A     ",
"RORA A,A     ",
"ILLEGAL     ",
"LDX abs,X    ",
"LODS abs,X   ",

"ILLEGAL     ",
"PSH  ,L      ",
"POP L,       ",
"ILLEGAL     ",
"TAS impl     ",
"TSA impl     ",
"ILLEGAL     ",
"ILLEGAL     ",
"MOVE A,A     ",
"MOVE B,A     ",
"MOVE C,A     ",
"MOVE D,A     ",
"MOVE E,A     ",
"MOVE L,A     ",
"MOVE H,A     ",
"MOVE M,A     ",

"ILLEGAL     ",
"PSH  ,H      ",
"POP H,       ",
"ILLEGAL     ",
"ILLEGAL     ",
"SWI impl     ",
"RTI impl     ",
"ILLEGAL     ",
"MOVE A,B     ",
"MOVE B,B     ",
"MOVE C,B     ",
"MOVE D,B     ",
"MOVE E,B     ",
"MOVE L,B     ",
"MOVE H,B     ",
"MOVE M,B     ",

"ADC A,B      ",
"SBC A,B      ",
"CMP A,B      ",
"IOR A,B      ",
"AND A,B      ",
"XOR A,B      ",
"BT A,B       ",
"ILLEGAL     ",
"MOVE A,C     ",
"MOVE B,C     ",
"MOVE C,C     ",
"MOVE D,C     ",
"MOVE E,C     ",
"MOVE L,C     ",
"MOVE H,C     ",
"MOVE M,C     ",

"ADC A,C      ",
"SBC A,C      ",
"CMP A,C      ",
"IOR A,C      ",
"AND A,C      ",
"XOR A,C      ",
"BT A,C       ",
"ILLEGAL     ",
"MOVE A,D     ",
"MOVE B,D     ",
"MOVE C,D     ",
"MOVE D,D     ",
"MOVE E,D     ",
"MOVE L,D     ",
"MOVE H,D     ",
"MOVE M,D     ",

"ADC A,D      ",
"SBC A,D      ",
"CMP A,D      ",
"IOR A,D      ",
"AND A,D      ",
"XOR A,D      ",
"BT A,D       ",
"LD  #        ",
"MOVE A,E     ",
"MOVE B,E     ",
"MOVE C,E     ",
"MOVE D,E     ",
"MOVE E,E     ",
"MOVE L,E     ",
"MOVE H,E     ",
"MOVE M,E     ",

"ADC A,E      ",
"SBC A,E      ",
"CMP A,E      ",
"IOR A,E      ",
"AND A,E      ",
"XOR A,E      ",
"BT A,E       ",
"LD abs       ",
"MOVE A,L     ",
"MOVE B,L     ",
"MOVE C,L     ",
"MOVE D,L     ",
"MOVE E,L     ",
"MOVE L,L     ",
"MOVE H,L     ",
"MOVE M,L     ",

"ADC A,L      ",
"SBC A,L      ",
"CMP A,L      ",
"IOR A,L      ",
"AND A,L      ",
"XOR A,L      ",
"BT A,L       ",
"LD abs,X     ",
"MOVE A,H     ",
"MOVE B,H     ",
"MOVE C,H     ",
"MOVE D,H     ",
"MOVE E,H     ",
"MOVE L,H     ",
"MOVE H,H     ",
"MOVE M,H     ",

"ADC A,H      ",
"SBC A,H      ",
"CMP A,H      ",
"IOR A,H      ",
"AND A,H      ",
"XOR A,H      ",
"BT A,H       ",
"ILLEGAL     ",
"MOVE A,M     ",
"MOVE B,M     ",
"MOVE C,M     ",
"MOVE D,M     ",
"MOVE E,M     ",
"MOVE L,M     ",
"MOVE H,M     ",
"MOVE -,-     ",

"ADC A,M      ",
"SBC A,M      ",
"CMP A,M      ",
"IOR A,M      ",
"AND A,M      ",
"XOR A,M      ",
"BT A,M       ",
"ILLEGAL     ",
"ILLEGAL     ",
"ILLEGAL     ",
"JMP abs      ",
"ILLEGAL     ",
"ILLEGAL     ",
"ILLEGAL     ",
"ILLEGAL     ",
"ILLEGAL     ",

};

////////////////////////////////////////////////////////////////////////////////
//                           Simulator/Emulator (Start)                       //
////////////////////////////////////////////////////////////////////////////////

/*--------------------------------------------------------------------------------------
			Function Name: fetch()
			Parameter list: N/A
			Description: Increases the program counter every time the function is called
			Return: (BYTE) byte
----------------------------------------------------------------------------------------*/
BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}

/*--------------------------------------------------------------------------------------
			Function Name: set_flag_z
			Parameter list: (BYTE) inReg
			Description: Sets the zero flag
----------------------------------------------------------------------------------------*/
void set_flag_z(BYTE inReg) {
	BYTE reg;
	reg = inReg;

	if (reg == 0)
	{
		Flags = Flags | FLAG_Z;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_Z);
	}
}

/*--------------------------------------------------------------------------------------
			Function Name: set_flag_n
			Parameter list: (BYTE) inReg
			Description: Sets the negative flag
----------------------------------------------------------------------------------------*/
void set_flag_n(BYTE inReg) {
	BYTE reg;
	reg = inReg;
	if ((reg & 0x80) != 0)
	{
		Flags = Flags | FLAG_N;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_N);
	}
}

/*--------------------------------------------------------------------------------------
			Function Name: set_flag_c
			Parameter list: (BYTE) inReg
			Description: Sets the carry flag
----------------------------------------------------------------------------------------*/
void set_flag_c(BYTE inReg) {
	BYTE reg;
	reg = inReg;
	if (reg >= 0x100)
	{
		Flags = Flags | FLAG_C;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_C);
	}
}
/*--------------------------------------------------------------------------------------
			Function Name: set_flag_v
			Parameter list: (BYTE) in1, (BYTE) in2 & (BYTE) out1
			Description: Sets the overflow flag
----------------------------------------------------------------------------------------*/

void set_flag_v(BYTE in1, BYTE in2, BYTE out1)
{
	BYTE reg1in;
	BYTE reg2in;
	BYTE regOut;
	reg1in = in1;
	reg2in = in2;
	regOut = out1;
	if ((((reg1in & 0x80) == 0x80) && ((reg2in & 0x80) == 0x80) && ((regOut & 0x80) != 0x80)) //overflow
		|| (((reg1in & 0x80) != 0x80) && ((reg2in & 0x80) != 0x80) && ((regOut & 0x80) == 0x80))) //overflow
	{
		Flags = Flags | FLAG_V;
	}

	else
	{
		Flags = Flags & (0xFF - FLAG_V);
	}
}
/*--------------------------------------------------------------------------------------
			Function Name: set_zn_flags
			Parameter list: (WORD) result
			Description: Sets the zero and negative flags
----------------------------------------------------------------------------------------*/
void set_zn_flags(WORD result)
{
	set_flag_z(result);
	set_flag_n(result);
}

/*--------------------------------------------------------------------------------------
			Function Name: MVI
			Parameter list: (WORD) regi
			Description: Register added to the accumulator with carry
			Flags: N, Z, V, C
----------------------------------------------------------------------------------------*/
void mvi(WORD regi)
{
	WORD data = fetch();
	Registers[regi] = data;
	set_zn_flags(Registers[regi]);
	Flags = Flags & (0xFF - FLAG_C);
}

/*--------------------------------------------------------------------------------------
			Function Name: ADC
			Parameter list: (WORD) reg_One
			Description: Register added to the accumulator with carry
			Flags: N, Z, V, C
----------------------------------------------------------------------------------------*/
void adc(WORD reg_One)
{
	WORD param1 = Registers[REGISTER_A];
	WORD param2 = Registers[reg_One];
	WORD temp_word = (WORD)param1 + (WORD)param2;
	if ((Flags & FLAG_C) != 0) {
		temp_word++;
	}
	if (temp_word >= 0x100) {
		Flags = Flags | FLAG_C;
	}
	else {
		Flags = Flags & (0xFF - FLAG_C);
	}
	set_zn_flags((BYTE)temp_word);
	set_flag_v(param1, param2, (BYTE)temp_word);
	Registers[REGISTER_A] = (BYTE)temp_word;
}

/*--------------------------------------------------------------------------------------
			Function Name: cmp
			Parameter list: (WORD) reg_Two
			Description: Registers compared to the accumulator
			Flags: N, Z, V, C
----------------------------------------------------------------------------------------*/
void cmp(WORD reg_Two)
{
	WORD param1 = Registers[REGISTER_A];
	WORD param2 = Registers[reg_Two];
	WORD temp_word = (WORD)param1 - (WORD)param2;
	if (temp_word >= 0x100) {
		Flags = Flags | FLAG_C;
	}
	else {
		Flags = Flags & (0xFF - FLAG_C);
	}
	set_zn_flags((BYTE)temp_word);
	set_flag_v(param1, -param2, (BYTE)temp_word);
}

/*--------------------------------------------------------------------------------------
			Function Name: clear_Flag
			Parameter list: (BYTE) flag
			Description: Clears any flag that is parsed through its peramemters
			Flags: N/A
----------------------------------------------------------------------------------------*/
void clear_Flag(BYTE flag)
{
	Flags = Flags & (0xFF - flag);
}

/*--------------------------------------------------------------------------------------
			Function Name: set_Flag
			Parameter list: (BYTE) flag
			Description: Sets any flag that is parsed through its peramemters
			Flags: N/A
----------------------------------------------------------------------------------------*/
void set_Flag(BYTE flag)
{
	Flags = Flags | flag;
}

/*--------------------------------------------------------------------------------------
			Function Name: push
			Parameter list: (WORD) reg_Type
			Description: Pushes registers onto the stack
			Flags: N/A
----------------------------------------------------------------------------------------*/
void push(WORD reg_Type)
{
	if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
	{
		Memory[StackPointer] = Registers[reg_Type];
		StackPointer--;
	}
}

/*--------------------------------------------------------------------------------------
			Function Name: pop
			Parameter list: (WORD) reg_Type
			Description: Pops the top of the stack into the a register
			Flags: N/A
----------------------------------------------------------------------------------------*/
void pop(WORD reg_Type)
{
	if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1)) {
		StackPointer++;
		Registers[reg_Type] = Memory[StackPointer];
	}
}

/*--------------------------------------------------------------------------------------
			Function Name: and
			Parameter list: (WORD) reg
			Description: Register bitwise and with accumulator
			Flags: Z, N, C
----------------------------------------------------------------------------------------*/
void and (WORD reg)
{
	Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[reg];
	set_zn_flags(Registers[REGISTER_A]);
	Flags = Flags & (0xFF - FLAG_V);
}

/*--------------------------------------------------------------------------------------
			Function Name: register_Bit
			Parameter list: (WORD) reg
			Description: Register Bit tested with the accumulator
			Flags: Z, N
----------------------------------------------------------------------------------------*/
void register_Bit(WORD reg)
{
	Registers[reg] = Registers[REGISTER_A] & Registers[reg];
	set_zn_flags(Registers[reg]);
	Flags = Flags & (0xFF - FLAG_V);
}
/*--------------------------------------------------------------------------------------
			Function Name: sbc
			Parameter list: (WORD) reg
			Description: Registers subtracted to the accumulator with carry
			Flags: N, Z, V, C
----------------------------------------------------------------------------------------*/
void sbc(WORD reg)
{
	WORD param1 = Registers[REGISTER_A];
	WORD param2 = Registers[reg];
	WORD temp_word = (WORD)param1 - (WORD)param2;
	if (temp_word >= 0x100) {
		Flags = Flags | FLAG_C;
	}
	else {
		Flags = Flags & (0xFF - FLAG_C);
	}
	set_zn_flags((BYTE)temp_word);
	set_flag_v(param1, -param2, (BYTE)temp_word);
	Registers[REGISTER_A] = (BYTE)temp_word;
}
/*--------------------------------------------------------------------------------------
			Function Name: ior
			Parameter list: (WORD) reg
			Description: Register bitwise inclusive or with the accumulator
			Flags: N, Z
----------------------------------------------------------------------------------------*/
void ior(WORD reg)
{
	Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[reg];
	set_zn_flags(Registers[REGISTER_A]);
	Flags = Flags & (0xFF - FLAG_V);
}
/*--------------------------------------------------------------------------------------
			Function Name: xor
			Parameter list: (WORD) reg
			Description: Register bitwise exclusive or with the accumulator
			Flags: N, Z
----------------------------------------------------------------------------------------*/
void xor (WORD reg)
{
	Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[reg];
	set_zn_flags(Registers[REGISTER_A]);
	Flags = Flags & (0xFF - FLAG_V);
}

/*--------------------------------------------------------------------------------------
			Function Name: Group_1
			Parameter list: (BYTE) opcode
			Description: Loads data
			Return: N/A
----------------------------------------------------------------------------------------*/
void Group_1(BYTE opcode) {
	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;
	WORD data = 0;
	WORD temp_word;
	WORD param1;
	WORD param2;
	WORD offset;
	BYTE saved_flags;
	WORD PC = ProgramCounter;


	switch (opcode) {
		/*---------------------------------------------------------
			Opcode: LD
			Case opcode: 0xB7
			Case description: Loads memory into accumulator
			Flags: N, Z, C
			Addresssing type: Immidiate
	---------------------------------------------------------------*/
	case 0xB7:
		data = fetch();
		Registers[REGISTER_A] = data;
		set_zn_flags(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_C);
		break;
		/*---------------------------------------------------------
				Opcode: LD
				Case opcode: 0xC7
				Case description: Loads memory into accumulator
				Flags: N, Z, C
				Addresssing type: Absolute
		----------------------------------------------------------*/
	case 0xC7:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_A] = Memory[address];
		}
		set_zn_flags(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_C);
		break;
		/*---------------------------------------------------------
				Opcode: LD
				Case opcode: 0xD7
				Case description: Loads memory into accumulator
				Flags: N, Z, C
				Addresssing type: Indexed Absolute
		----------------------------------------------------------*/
	case 0xD7:
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_A] = Memory[address];
		}
		set_zn_flags(Registers[REGISTER_A]);

		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*---------------------------------------------------------
			Opcode: ST
			Case opcode: 0x10
			Case description: Stores accumulator into memory
			Flags: N, Z, C
			Addresssing type: immidiate
	----------------------------------------------------------*/
	case 0x10:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_A];
		}
		set_zn_flags(Memory[address]);

		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*---------------------------------------------------------
			Opcode: ST
			Case opcode: 0x10
			Case description: Stores accumulator into memory
			Flags: N, Z, C
			Addresssing type: Absolute
	----------------------------------------------------------*/
	case 0x20:
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_A];
		}
		set_zn_flags(Memory[address]);

		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*---------------------------------------------------------
				Opcode: MVI
				Case opcode: 0x2A
				Case description: Stores accumulator into memory by
								  calling the mvi function
				Flags: N, Z, C
				Addresssing type: N/A
				Register: REGISTER_B
		----------------------------------------------------------*/
	case 0x2A:
		mvi(REGISTER_B);
		break;
		/*---------------------------------------------------------
				Opcode: MVI
				Case opcode: 0x2B
				Case description: Stores accumulator into memory by
								  calling the mvi function
				Flags: N, Z, C
				Addresssing type: N/A
				Register: REGISTER_C
		----------------------------------------------------------*/
	case 0x2B:
		mvi(REGISTER_C);
		break;
		/*---------------------------------------------------------
				Opcode: MVI
				Case opcode: 0x2C
				Case description: Stores accumulator into memory by
								  calling the mvi function
				Flags: N, Z, C
				Addresssing type: N/A
				Register: REGISTER_D
		----------------------------------------------------------*/
	case 0x2C:
		mvi(REGISTER_D);
		break;
		/*---------------------------------------------------------
				Opcode: MVI
				Case opcode: 0x2D
				Case description: Stores accumulator into memory by
								  calling the mvi function
				Flags: N, Z, C
				Addresssing type: N/A
				Register: REGISTER_E
		----------------------------------------------------------*/
	case 0x2D:
		mvi(REGISTER_E);
		break;
		/*---------------------------------------------------------
				Opcode: MVI
				Case opcode: 0x2E
				Case description: Stores accumulator into memory by
								  calling the mvi function
				Flags: N, Z, C
				Addresssing type: N/A
				Register: REGISTER_L
		----------------------------------------------------------*/
	case 0x2E:
		mvi(REGISTER_L);
		break;
		/*---------------------------------------------------------
				Opcode: MVI
				Case opcode: 0x2F
				Case description: Stores accumulator into memory by
								  calling the mvi function
				Flags: N, Z, C
				Addresssing type: N/A
				Register: REGISTER_H
		----------------------------------------------------------*/
	case 0x2F:
		mvi(REGISTER_H);
		break;

		/*---------------------------------------------------------
				Opcode: LDX
				Case opcode: 0x4E
				Case description: Loads Memory into the IndexRegister
				Flags: N, Z, C
				Addresssing type: Immidiate
		----------------------------------------------------------*/
	case 0x4E:
		data = fetch();
		IndexRegister = data;
		set_zn_flags(IndexRegister);
		Flags = Flags & (0xFF - FLAG_C);
		break;
		/*---------------------------------------------------------
				Opcode: LDX
				Case opcode: 0x5E
				Case description: Loads Memory into the IndexRegister
				Flags: N, Z, C
				Addresssing type: Absolute
		----------------------------------------------------------*/
	case 0x5E:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			IndexRegister = Memory[address];
		}
		set_zn_flags(IndexRegister);
		Flags = Flags & (0xFF - FLAG_C);
		break;
		/*---------------------------------------------------------
				Opcode: LDX
				Case opcode: 0x6E
				Case description: Loads Memory into the IndexRegister
				Flags: N, Z, C
				Addresssing type: Indexed Absolute
		----------------------------------------------------------*/
	case 0x6E:
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			IndexRegister = Memory[address];
		}
		set_zn_flags(Memory[address]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*---------------------------------------------------------
				Opcode: STOX
				Case opcode: 0x50
				Case description: Stores the IndexRegister in Memory
				Flags: N, Z, C
				Addresssing type: Absolute
		----------------------------------------------------------*/
	case 0x50:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = IndexRegister;
		}
		set_zn_flags(Memory[address]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*---------------------------------------------------------
				Opcode: STOX
				Case opcode: 0x60
				Case description: Stores the IndexRegister in Memory
				Flags: N, Z, C
				Addresssing type: Indexed Absolute
		----------------------------------------------------------*/
	case 0x60:
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = IndexRegister;
		}
		set_zn_flags(Memory[address]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*---------------------------------------------------------
			Opcode: LODS
			Case opcode: 0x4F
			Case description: Loads Memory into the StackPointer
			Flags: N, Z, C
			Addresssing type: Immidiate
	----------------------------------------------------------*/
	case 0x4F:
		data = fetch();
		StackPointer = data << 8;
		StackPointer += fetch();
		set_zn_flags(data);
		Flags = Flags & (0xFF - FLAG_C);
		break;
		/*---------------------------------------------------------
				Opcode: LODS
				Case opcode: 0x5F
				Case description: Loads Memory into the StackPointer
				Flags: N, Z, C
				Addresssing type: Absolute
		----------------------------------------------------------*/
	case 0x5F:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		set_zn_flags(Memory[address]);
		Flags = Flags & (0xFF - FLAG_C);
		break;
		/*---------------------------------------------------------
			Opcode: LODS
			Case opcode: 0x6F
			Case description: Loads Memory into the StackPointer
			Flags: N, Z, C
			Addresssing type: Indexed Absolute
	----------------------------------------------------------*/
	case 0x6F:
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		set_zn_flags(Memory[address]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*---------------------------------------------------------
			Opcode: TAS
			Case opcode: 0x74
			Case description: Transfers the accumulator to the
							  status register
			Flags: N/A
			Addresssing type: Implied
	----------------------------------------------------------*/
	case 0x74:
		Flags = Registers[REGISTER_A];
		break;

		/*---------------------------------------------------------
			Opcode: TSA
			Case opcode: 0x75
			Case description: Transfers the the status register to
							  the accumulator
			Flags: N/A
			Addresssing type: Implied
	----------------------------------------------------------*/
	case 0x75:
		Registers[REGISTER_A] = Flags;
		break;

		/*-----------------------------------------------------------------
			Opcode: ADC
			Case opcode: 0x90
			Case description: Calls the ADC function, which adds registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_B
		------------------------------------------------------------------*/
	case 0x90:
		adc(REGISTER_B);
		break;

		/*-----------------------------------------------------------------
			Opcode: ADC
			Case opcode: 0xA0
			Case description: Calls the ADC function, which adds registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_C
		------------------------------------------------------------------*/
	case 0xA0:
		adc(REGISTER_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: ADC
			Case opcode: 0xB0
			Case description: Calls the ADC function, which adds registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_D
		------------------------------------------------------------------*/
	case 0xB0:
		adc(REGISTER_D);
		break;

		/*-----------------------------------------------------------------
			Opcode: ADC
			Case opcode: 0xC0
			Case description: Calls the ADC function, which adds registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_E
		------------------------------------------------------------------*/
	case 0xC0:
		adc(REGISTER_E);
		break;
		/*-----------------------------------------------------------------
			Opcode: ADC
			Case opcode: 0xD0
			Case description: Calls the ADC function, which adds registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_L
		------------------------------------------------------------------*/
	case 0xD0:
		adc(REGISTER_L);
		break;

		/*-----------------------------------------------------------------
			Opcode: ADC
			Case opcode: 0xE0
			Case description: Calls the ADC function, which adds registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_H
		------------------------------------------------------------------*/
	case 0xE0:
		adc(REGISTER_H);
		break;
		/*-----------------------------------------------------------------
			Opcode: ADC
			Case opcode: 0xF0
			Case description: Calls the adc function, which adds registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_M
		------------------------------------------------------------------*/
	case 0xF0:
		adc(REGISTER_M);
		break;


		/*-----------------------------------------------------------------
			Opcode: CMP
			Case opcode: 0x92
			Case description: Calls the cmp function, which compares registers
							  to the accumulator
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_B
		------------------------------------------------------------------*/
	case 0x92:
		cmp(REGISTER_B);
		break;

		/*-----------------------------------------------------------------
			Opcode: CMP
			Case opcode: 0xA2
			Case description: Calls the cmp function, which compares registers
							  to the accumulator
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_C
		------------------------------------------------------------------*/
	case 0xA2:
		cmp(REGISTER_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: CMP
			Case opcode: 0xB2
			Case description: Calls the cmp function, which compares registers
							  to the accumulator
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_D
		------------------------------------------------------------------*/
	case 0xB2:
		cmp(REGISTER_D);
		break;

		/*-----------------------------------------------------------------
			Opcode: CMP
			Case opcode: 0xC2
			Case description: Calls the cmp function, which compares registers
							  to the accumulator
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_E
		------------------------------------------------------------------*/
	case 0xC2:
		cmp(REGISTER_E);
		break;

		/*-----------------------------------------------------------------
			Opcode: CMP
			Case opcode: 0xD2
			Case description: Calls the cmp function, which compares registers
							  to the accumulator
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_D
		------------------------------------------------------------------*/
	case 0xD2:
		cmp(REGISTER_L);
		break;

		/*-----------------------------------------------------------------
			Opcode: CMP
			Case opcode: 0xE2
			Case description: Calls the cmp function, which compares registers
							  to the accumulator
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_H
		------------------------------------------------------------------*/
	case 0xE2:
		cmp(REGISTER_H);
		break;

		/*-----------------------------------------------------------------
			Opcode: CMP
			Case opcode: 0xF2
			Case description: Calls the cmp function, which compares registers
							  to the accumulator
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_M
		------------------------------------------------------------------*/
	case 0xF2:
		cmp(REGISTER_M);
		break;

		/*-----------------------------------------------------------------
			Opcode: CLC
			Case opcode: 0x15
			Case description: Calls the clear_Flag function, which clears any
							  flag that is parsed into it
			Flags: C
			Addresssing type: Immidiate
		------------------------------------------------------------------*/
	case 0x15:
		clear_Flag(FLAG_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: SEC
			Case opcode: 0x16
			Case description: Calls the set_Flag function, which sets any
							  flag that is parsed into it
			Flags: C
			Addresssing type: Immidiate
		------------------------------------------------------------------*/
	case 0x16:
		set_Flag(FLAG_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: CLI
			Case opcode: 0x17
			Case description: Calls the clear_Flag function, which clears any
							  flag that is parsed into it
			Flags: I
			Addresssing type: Immidiate
		------------------------------------------------------------------*/
	case 0x17:
		clear_Flag(FLAG_I);
		break;

		/*-----------------------------------------------------------------
			Opcode: STI
			Case opcode: 0x18
			Case description: Calls the set_Flag function, which sets any
							  flag that is parsed into it
			Flags: I
			Addresssing type: Immidiate
		------------------------------------------------------------------*/
	case 0x18:
		set_Flag(FLAG_I);
		break;


		/*-----------------------------------------------------------------
			Opcode: SEV
			Case opcode: 0x19
			Case description: Calls the clear_Flag function, which clears any
							  flag that is parsed into it
			Flags: V
			Addresssing type: Immidiate
		------------------------------------------------------------------*/
	case 0x19:
		clear_Flag(FLAG_V);
		break;

		/*-----------------------------------------------------------------
			Opcode: CLV
			Case opcode: 0x1A
			Case description: Calls the set_Flag function, which sets any
							  flag that is parsed into it
			Flags: V
			Addresssing type: Immidiate
		------------------------------------------------------------------*/
	case 0x1A:
		set_Flag(FLAG_V);
		break;

		/*-----------------------------------------------------------------
			Opcode: PSH
			Case opcode: 0x11
			Case description: Calls the push function, which pushes registers
							  onto the stack
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x11:
		push(REGISTER_A);
		break;

		/*-----------------------------------------------------------------
			Opcode: PSH
			Case opcode: 0x21
			Case description: Calls the push function, which pushes registers
							  onto the stack
			Flags: N/A
			Addresssing type: N/A
			Register: Status Register (Flags)
		------------------------------------------------------------------*/
	case 0x21:
		push(Flags);
		break;

		/*-----------------------------------------------------------------
			Opcode: PSH
			Case opcode: 0x31
			Case description: Calls the push function, which pushes registers
							  onto the stack
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_B
		------------------------------------------------------------------*/
	case 0x31:
		push(REGISTER_B);
		break;

		/*-----------------------------------------------------------------
			Opcode: PSH
			Case opcode: 0x41
			Case description: Calls the push function, which pushes registers
							  onto the stack
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_C
		------------------------------------------------------------------*/
	case 0x41:
		push(REGISTER_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: PSH
			Case opcode: 0x51
			Case description: Calls the push function, which pushes registers
							  onto the stack
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_D
		------------------------------------------------------------------*/
	case 0x51:
		push(REGISTER_D);
		break;

		/*-----------------------------------------------------------------
			Opcode: PSH
			Case opcode: 0x61
			Case description: Calls the push function, which pushes registers
							  onto the stack
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_E
		------------------------------------------------------------------*/
	case 0x61:
		push(REGISTER_E);
		break;

		/*-----------------------------------------------------------------
			Opcode: PSH
			Case opcode: 0x71
			Case description: Calls the push function, which pushes registers
							  onto the stack
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_L
		------------------------------------------------------------------*/
	case 0x71:
		push(REGISTER_L);
		break;

		/*-----------------------------------------------------------------
			Opcode: PSH
			Case opcode: 0x81
			Case description: Calls the push function, which pushes registers
							  onto the stack
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_H
		------------------------------------------------------------------*/
	case 0x81:
		push(REGISTER_H);
		break;


		/*-----------------------------------------------------------------
			Opcode: POP
			Case opcode: 0x12
			Case description: Calls the pop function, which Pops the top of
							  the stack into the a register
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x12:
		pop(REGISTER_A);
		break;

		/*-----------------------------------------------------------------
			Opcode: POP
			Case opcode: 0x22
			Case description: Calls the pop function, which Pops the top of
							  the stack into the a register
			Flags: N/A
			Addresssing type: N/A
			Register: Status register (Flags)
		------------------------------------------------------------------*/
	case 0x22:
		pop(Flags);
		break;

		/*-----------------------------------------------------------------
			Opcode: POP
			Case opcode: 0x32
			Case description: Calls the pop function, which Pops the top of
							  the stack into the a register
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_B
		------------------------------------------------------------------*/
	case 0x32:
		pop(REGISTER_B);
		break;

		/*-----------------------------------------------------------------
			Opcode: POP
			Case opcode: 0x42
			Case description: Calls the pop function, which Pops the top of
							  the stack into the a register
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_C
		------------------------------------------------------------------*/
	case 0x42:
		pop(REGISTER_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: POP
			Case opcode: 0x52
			Case description: Calls the pop function, which Pops the top of
							  the stack into the a register
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_D
		------------------------------------------------------------------*/
	case 0x52: 
		pop(REGISTER_D);
		break;

		/*-----------------------------------------------------------------
			Opcode: POP
			Case opcode: 0x62
			Case description: Calls the pop function, which Pops the top of
							  the stack into the a register
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_E
		------------------------------------------------------------------*/
	case 0x62:
		pop(REGISTER_E);
		break;

		/*-----------------------------------------------------------------
			Opcode: POP
			Case opcode: 0x72
			Case description: Calls the pop function, which Pops the top of
							  the stack into the a register
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_L
		------------------------------------------------------------------*/
	case 0x72:
		pop(REGISTER_L);
		break;

		/*-----------------------------------------------------------------
			Opcode: POP
			Case opcode: 0x82
			Case description: Calls the pop function, which Pops the top of
							  the stack into the a register
			Flags: N/A
			Addresssing type: N/A
			Register: REGISTER_H
		------------------------------------------------------------------*/
	case 0x82:
		pop(REGISTER_H);
		break;


		/*-----------------------------------------------------------------
			Opcode: ADI
			Case opcode: 0x25
			Case description: Adds data to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: Immidiate
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x25:
		param1 = Registers[REGISTER_A];
		param2 = fetch();
		temp_word = (WORD)param1 + (WORD)param2;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (BYTE)temp_word;

		set_flag_v(Registers[REGISTER_A], data, (BYTE)temp_word);
		set_zn_flags(temp_word);
		break;

		/*-----------------------------------------------------------------
			Opcode: SBI
			Case opcode: 0x26
			Case description: Subtracts data from the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: Immidiate
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x26:
		param1 = Registers[REGISTER_A];
		param2 = fetch();
		temp_word = (WORD)param1 - (WORD)param2;

		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}
		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (BYTE)temp_word;

		set_zn_flags(temp_word);
		set_flag_v(Registers[REGISTER_A], data, temp_word);
		break;

		/*-----------------------------------------------------------------
			Opcode: CPI
			Case opcode: 0x27
			Case description: Compares data to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: Immidiate
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x27:
		param1 = Registers[REGISTER_A];
		param2 = fetch();
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (BYTE)temp_word;
		set_zn_flags(temp_word);
		set_flag_v(Registers[REGISTER_A], data, (BYTE)temp_word);
		break;

		/*-----------------------------------------------------------------
			Opcode: ANI
			Case opcode: 0x28
			Case description: Data bitwise and with accumulator
			Flags: N, Z
			Addresssing type: Immidiate
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x28:
		data = fetch();
		Registers[REGISTER_A] = Registers[REGISTER_A] & (WORD)data;
		set_zn_flags(Registers[REGISTER_A]);
		break;

		/*-----------------------------------------------------------------
			Opcode: XRI
			Case opcode: 0x29
			Case description: Data bitwise exclusive or with accumulator
			Flags: N, Z
			Addresssing type: Immidiate
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x29:
		data = fetch();
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ (WORD)data;
		set_zn_flags(Registers[REGISTER_A]);
		break;


		/*-----------------------------------------------------------------
			Opcode: JMP
			Case opcode: 0xFA
			Case description: Loads memory into the ProgramCounter
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0xFA:
		HB = fetch();
		LB = fetch();
		address = ((WORD)HB << 8) + (WORD)LB;
		ProgramCounter = address;
		break;

		/*-----------------------------------------------------------------
			Opcode: JPR
			Case opcode: 0x33
			Case description: Jump to subroutine
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x33:
		HB = fetch();
		LB = fetch();
		address = ((WORD)HB << 8) + (WORD)LB;
		if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE)) {
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
		}
		ProgramCounter = address;
		break;


		/*-----------------------------------------------------------------
			Opcode: RTN
			Case opcode: 0x0E
			Case description: Return to subroutine
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x0E:
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 2))
		{
			StackPointer++;
			HB = Memory[StackPointer];
			StackPointer++;
			LB = Memory[StackPointer];
			ProgramCounter = ((WORD)HB << 8) + (WORD)LB;

		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BRA
			Case opcode: 0x00
			Case description: Branch always
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x00:
		LB = fetch();
		ProgramCounter = address;
		offset = (WORD)LB;
		if ((offset & 0x80) != 0)
		{
			offset = offset + 0xFF00;
		}
		address = ProgramCounter + offset;
		break;

		/*-----------------------------------------------------------------
			Opcode: BCC
			Case opcode: 0x01
			Case description: Branch on carry clear
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x01:
		LB = fetch();
		if ((Flags & FLAG_C) != FLAG_C) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}

			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BCS
			Case opcode: 0x02
			Case description: Branch on carry set
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x02:
		LB = fetch();
		if ((Flags & FLAG_C) == FLAG_C) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xff00;
			}

			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BNE
			Case opcode: 0x03
			Case description: Branch on result not zero
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x03:
		LB = fetch();
		if ((Flags & FLAG_Z) == 0x00) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}

			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BEQ
			Case opcode: 0x04
			Case description: Branch on result equal to zero
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x04:
		LB = fetch();
		if ((Flags & FLAG_Z) == FLAG_Z) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}

			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BVC
			Case opcode: 0x05
			Case description: Branch on overflow clear
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x05:

		LB = fetch();
		if ((Flags & FLAG_V) != FLAG_V) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}

			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BVS
			Case opcode: 0x06
			Case description: Branch on overflow set
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x06:
		LB = fetch();
		if ((Flags & FLAG_V) == FLAG_V) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}

			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BMI
			Case opcode: 0x07
			Case description: Branch on negative result
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x07:
		LB = fetch();
		if ((Flags & FLAG_N) == FLAG_N) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}

			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BPL
			Case opcode: 0x08
			Case description: Branch on positive result
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x08:
		LB = fetch();
		if ((Flags & FLAG_N) != FLAG_N) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}

			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BGE
			Case opcode: 0x09
			Case description: Branch on result less than or equal to zero
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x09:
		LB = fetch();
		if (!(((Flags & FLAG_N) == 0x00) ^ (Flags & FLAG_V) == 0x00)) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}

			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BLE
			Case opcode: 0x0A
			Case description: Branch on result more than or equal to zero
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x0A:
		LB = fetch();
		if ((((Flags & FLAG_N) == 0x00) || ((Flags & FLAG_Z) != 0x00)) ^ ((Flags & FLAG_V) == 0x00))
		{
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BLS
			Case opcode: 0x0B
			Case description: Branch on result same or lower
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x0B:
		LB = fetch();
		if ((Flags & FLAG_Z) | (Flags & FLAG_C) != 0x00) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}

			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: BHI
			Case opcode: 0x0C
			Case description: Branch on result higher
			Flags: N/A
			Addresssing type: Offset
		------------------------------------------------------------------*/
	case 0x0C:
		LB = fetch();
		if ((Flags & FLAG_Z) | (Flags & FLAG_C) == 0x00) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;


		/*-----------------------------------------------------------------
			Opcode: CCC
			Case opcode: 0x34
			Case description: Call on carry clear
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x34:
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_C) != FLAG_C)
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{

				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;

				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: CCS
			Case opcode: 0x35
			Case description: Call on carry set
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x35:
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_C) == FLAG_C)
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{

				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;

				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: CNE
			Case opcode: 0x36
			Case description: Call on result not zero
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x36:
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_Z) == 0x00)
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{

				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: CEQ
			Case opcode: 0x37
			Case description: Call on result equal to zero
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x37:
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_Z) != 0x00)
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{

				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: CVC
			Case opcode: 0x38
			Case description: Call on overflow clear
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x38:
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_V) != FLAG_V)
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{

				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: CVS
			Case opcode: 0x39
			Case description: Call on overflow set
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x39:
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_V) == FLAG_V)
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: CMI
			Case opcode: 0x3A
			Case description: Call on negative result
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x3A:
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_N) == FLAG_N)
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: CPL
			Case opcode: 0x3B
			Case description: Call on positive result
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x3B:
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_N) != FLAG_N)
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: CHI
			Case opcode: 0x3C
			Case description: Call on result same or lower
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x3C:
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_C) | (Flags & FLAG_Z) == 0x01)
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: CLE
			Case opcode: 0x3D
			Case description: Call on result higher
			Flags: N/A
			Addresssing type: Absolute
		------------------------------------------------------------------*/
	case 0x3D:
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_Z) | (Flags & FLAG_C) == 0x00)
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;


		/*-----------------------------------------------------------------
			Opcode: INCA
			Case opcode: 0x64
			Case description: Increment accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x64:
		Registers[REGISTER_A]++;
		set_zn_flags(Registers[REGISTER_A]);
		break;

		/*-----------------------------------------------------------------
			Opcode: DEX
			Case opcode: 0x1B
			Case description: Decrements the IndexRegister
			Flags: Z
			Addresssing type: Immidiate
		------------------------------------------------------------------*/
	case 0x1B:
		IndexRegister--;
		set_flag_z(IndexRegister);
		break;

		/*-----------------------------------------------------------------
			Opcode: INX
			Case opcode: 0x1C
			Case description: Increments the IndexRegister
			Flags: Z
			Addresssing type: Immidiate
		------------------------------------------------------------------*/
	case 0x1C:
		IndexRegister++;
		set_flag_z(IndexRegister);
		break;


		/*-----------------------------------------------------------------
			Opcode: AND
			Case opcode: 0x94
			Case description: Calls the and function, which bitwise ands
							  Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_B
		------------------------------------------------------------------*/
	case 0x94:
		and (REGISTER_B);
		break;

		/*-----------------------------------------------------------------
			Opcode: AND
			Case opcode: 0xA4
			Case description: Calls the and function, which bitwise ands
							  Registers with accumulator
			Flags : N, Z
			Addresssing type: N/A
			Register: REGISTER_C
		------------------------------------------------------------------*/
	case 0xA4:
		and (REGISTER_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: AND
			Case opcode: 0xB4
			Case description: Calls the and function, which bitwise ands
							  Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_D
		------------------------------------------------------------------*/
	case 0xB4:
		and (REGISTER_D);
		break;

		/*-----------------------------------------------------------------
			Opcode: AND
			Case opcode: 0xC4
			Case description: Calls the and function, which bitwise ands
							  Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_E
		------------------------------------------------------------------*/
	case 0xC4:
		and (REGISTER_E);
		break;

		/*-----------------------------------------------------------------
			Opcode: AND
			Case opcode: 0xD4
			Case description: Calls the and function, which bitwise ands
							  Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_L
		------------------------------------------------------------------*/
	case 0xD4:
		and (REGISTER_L);
		break;

		/*-----------------------------------------------------------------
			Opcode: AND
			Case opcode: 0xE4
			Case description: Calls the and function, which bitwise ands
							  Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_H
		------------------------------------------------------------------*/
	case 0xE4:
		and (REGISTER_H);
		break;
		/*-----------------------------------------------------------------
			Opcode: AND
			Case opcode: 0xF4
			Case description: Calls the and function, which bitwise ands
							  Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_M
		------------------------------------------------------------------*/
	case 0xF4:
		and (REGISTER_M);
		break;


		/*-----------------------------------------------------------------
			Opcode: BT
			Case opcode: 0x94
			Case description: Calls the register_Bit function, which tests
							  the register bit with the accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x96:
		register_Bit(REGISTER_B);
		break;

		/*-----------------------------------------------------------------
			Opcode: BT
			Case opcode: 0xA4
			Case description: Calls the register_Bit function, which tests
							  the register bit with the accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_C
		------------------------------------------------------------------*/
	case 0xA6:
		register_Bit(REGISTER_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: BT
			Case opcode: 0xB4
			Case description: Calls the register_Bit function, which tests
							  the register bit with the accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_D
		------------------------------------------------------------------*/
	case 0xB6:
		register_Bit(REGISTER_D);
		break;

		/*-----------------------------------------------------------------
			Opcode: BT
			Case opcode: 0xC4
			Case description: Calls the register_Bit function, which tests
							  the register bit with the accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_E
		------------------------------------------------------------------*/
	case 0xC6:
		register_Bit(REGISTER_E);
		break;

		/*-----------------------------------------------------------------
			Opcode: BT
			Case opcode: 0xD4
			Case description: Calls the register_Bit function, which tests
							  the register bit with the accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_L
		------------------------------------------------------------------*/
	case 0xD6:
		register_Bit(REGISTER_L);
		break;

		/*-----------------------------------------------------------------
			Opcode: BT
			Case opcode: 0xE4
			Case description: Calls the register_Bit function, which tests
							  the register bit with the accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_H
		------------------------------------------------------------------*/
	case 0xE6:
		register_Bit(REGISTER_H);
		break;

		/*-----------------------------------------------------------------
			Opcode: BT
			Case opcode: 0xF4
			Case description: Calls the register_Bit function, which tests
							  the register bit with the accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_M
		------------------------------------------------------------------*/
	case 0xF6:
		register_Bit(REGISTER_M);
		break;


		/*-----------------------------------------------------------------
			Opcode: INC
			Case opcode: 0x44
			Case description: Increments memory
			Flags: N, Z
			Addresssing type: Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x44:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address]++;
		}
		set_zn_flags(Memory[address]);
		break;

		/*-----------------------------------------------------------------
			Opcode: INC
			Case opcode: 0x54
			Case description: Increments memory
			Flags: N, Z
			Addresssing type: Indexed Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x54:
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address]++;
		}
		set_zn_flags(Memory[address]);
		break;


		/*-----------------------------------------------------------------
			Opcode: SBC
			Case opcode: 0x91
			Case description: Calls the sbc function, which subtracts registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_B
		------------------------------------------------------------------*/
	case 0x91: 
		sbc(REGISTER_B);
		break;

		/*-----------------------------------------------------------------
			Opcode: SBC
			Case opcode: 0xA1
			Case description: Calls the sbc function, which subtracts registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_C
		------------------------------------------------------------------*/
	case 0xA1:
		sbc(REGISTER_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: SBC
			Case opcode: 0xB1
			Case description: Calls the sbc function, which subtracts registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_D
		------------------------------------------------------------------*/
	case 0xB1:
		sbc(REGISTER_D);
		break;

		/*-----------------------------------------------------------------
			Opcode: SBC
			Case opcode: 0xC1
			Case description: Calls the sbc function, which subtracts registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_E
		------------------------------------------------------------------*/
	case 0xC1:
		sbc(REGISTER_E);
		break;

		/*-----------------------------------------------------------------
			Opcode: SBC
			Case opcode: 0xD1
			Case description: Calls the sbc function, which subtracts registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_L
		------------------------------------------------------------------*/
	case 0xD1:
		sbc(REGISTER_L);
		break;

		/*-----------------------------------------------------------------
			Opcode: SBC
			Case opcode: 0xE1
			Case description: Calls the sbc function, which subtracts registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_H
		------------------------------------------------------------------*/
	case 0xE1:
		sbc(REGISTER_H);
		break;

		/*-----------------------------------------------------------------
			Opcode: SBC
			Case opcode: 0xF1
			Case description: Calls the sbc function, which subtracts registers
							  to the accumulator with carry
			Flags: N, Z, V, C
			Addresssing type: N/A
			Register: REGISTER_M
		------------------------------------------------------------------*/
	case 0xF1:
		sbc(REGISTER_M);
		break;



		/*-----------------------------------------------------------------
			Opcode: IOR
			Case opcode: 0x93
			Case description: Calls the ior function, which bitwise inclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_B
		------------------------------------------------------------------*/
	case 0x93:
		ior(REGISTER_B);
		break;

		/*-----------------------------------------------------------------
			Opcode: IOR
			Case opcode: 0xA3
			Case description: Calls the ior function, which bitwise inclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_C
		------------------------------------------------------------------*/
	case 0xA3:
		ior(REGISTER_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: IOR
			Case opcode: 0xB3
			Case description: Calls the ior function, which bitwise inclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_D
		------------------------------------------------------------------*/
	case 0xB3:
		ior(REGISTER_D);
		break;

		/*-----------------------------------------------------------------
			Opcode: IOR
			Case opcode: 0xC3
			Case description: Calls the ior function, which bitwise inclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_E
		------------------------------------------------------------------*/
	case 0xC3:
		ior(REGISTER_E);
		break;

		/*-----------------------------------------------------------------
			Opcode: IOR
			Case opcode: 0xD3
			Case description: Calls the ior function, which bitwise inclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_L
		------------------------------------------------------------------*/
	case 0xD3:
		ior(REGISTER_L);
		break;

		/*-----------------------------------------------------------------
			Opcode: IOR
			Case opcode: 0xE3
			Case description: Calls the ior function, which bitwise inclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_H
		------------------------------------------------------------------*/
	case 0xE3:
		ior(REGISTER_H);
		break;

		/*-----------------------------------------------------------------
			Opcode: IOR
			Case opcode: 0xF3
			Case description: Calls the ior function, which bitwise inclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_M
		------------------------------------------------------------------*/
	case 0xF3:
		ior(REGISTER_M);
		break;


		/*-----------------------------------------------------------------
			Opcode: XOR
			Case opcode: 0x95
			Case description: Calls the xor function, which bitwise exclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_B
		------------------------------------------------------------------*/
	case 0x95:
		xor (REGISTER_B);
		break;

		/*-----------------------------------------------------------------
			Opcode: XOR
			Case opcode: 0xA5
			Case description: Calls the xor function, which bitwise exclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_C
		------------------------------------------------------------------*/
	case 0xA5:
		xor (REGISTER_C);
		break;

		/*-----------------------------------------------------------------
			Opcode: XOR
			Case opcode: 0xB5
			Case description: Calls the xor function, which bitwise exclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_D
		------------------------------------------------------------------*/
	case 0xB5:
		xor (REGISTER_D);
		break;

		/*-----------------------------------------------------------------
			Opcode: XOR
			Case opcode: 0xC5
			Case description: Calls the xor function, which bitwise exclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_E
		------------------------------------------------------------------*/
	case 0xC5:
		xor (REGISTER_E);
		break;

		/*-----------------------------------------------------------------
			Opcode: XOR
			Case opcode: 0xD5
			Case description: Calls the xor function, which bitwise exclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_L
		------------------------------------------------------------------*/
	case 0xD5:
		xor (REGISTER_L);
		break;

		/*-----------------------------------------------------------------
			Opcode: XOR
			Case opcode: 0xE5
			Case description: Calls the xor function, which bitwise exclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_H
		------------------------------------------------------------------*/
	case 0xE5:
		xor (REGISTER_H);
		break;

		/*-----------------------------------------------------------------
			Opcode: XOR
			Case opcode: 0xF5
			Case description: Calls the xor function, which bitwise exclusive
							  or Registers with accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_M
		------------------------------------------------------------------*/
	case 0xF5:
		xor (REGISTER_M);
		break;

		/*-----------------------------------------------------------------
			Opcode: NOT
			Case opcode: 0x4A
			Case description: Negates memory
			Flags: N, Z, C
			Addresssing type: Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x4A:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = ~Memory[address];

			if (Memory[address] >= 0x10)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
		}
		set_zn_flags(Memory[address]);
		break;

		/*-----------------------------------------------------------------
			Opcode: NOT
			Case opcode: 0x5A
			Case description: Negates memory
			Flags: N, Z, C
			Addresssing type: Indexed Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x5A:
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = ~Memory[address];

			if (Memory[address] >= 0x10)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
		}

		set_zn_flags(Memory[address]);
		break;


		/*-----------------------------------------------------------------
			Opcode: NOTA
			Case opcode: 0x6A
			Case description: Negates the accumulator
			Flags: N, Z, C
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x6A:
		Registers[REGISTER_A] = ~Registers[REGISTER_A];
		if ((WORD)Registers[REGISTER_A] >= 0x10)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		set_zn_flags(Registers[REGISTER_A]);
		break;


		/*-----------------------------------------------------------------
			Opcode: TST
			Case opcode: 0x43
			Case description: Bit tests memory
			Flags: N, Z
			Addresssing type: Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x43:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Memory[address] - 0x00;
		}
		set_zn_flags(Memory[address]);
		break;

		/*-----------------------------------------------------------------
			Opcode: TST
			Case opcode: 0x53
			Case description: Bit tests memory
			Flags: N, Z
			Addresssing type: Indexed Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x53:
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Memory[address] - 0x00;
		}
		set_zn_flags(Memory[address]);
		break;

		/*-----------------------------------------------------------------
			Opcode: TSTA
			Case opcode: 0x63
			Case description: Bit tests the accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x63:
		temp_word = Registers[REGISTER_A] - 0x00;
		Registers[REGISTER_A] = (BYTE)temp_word;
		set_zn_flags(Registers[REGISTER_A]);
		break;

		/*-----------------------------------------------------------------
			Opcode: DEC
			Case opcode: 0x45
			Case description: Decrements memory
			Flags: N, Z
			Addresssing type: Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x45:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address]--;
		}
		set_zn_flags(Memory[address]);
		break;

		/*-----------------------------------------------------------------
			Opcode: DEC
			Case opcode: 0x55
			Case description: Decrements memory
			Flags: N, Z
			Addresssing type: IndexedAbsolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x55:
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address]--;
		}
		set_zn_flags(Memory[address]);
		break;


		/*-----------------------------------------------------------------
			Opcode: DECA
			Case opcode: 0x65
			Case description: decrement accumulator
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x65:
		Registers[REGISTER_A]--;
		set_zn_flags(Registers[REGISTER_A]);
		break;

		/*-----------------------------------------------------------------
			Opcode: WAI
			Case opcode: 0x1E
			Case description: Wait for interupt
			Flags: N/A
			Addresssing type: Immidiate
			Register: N/A
		------------------------------------------------------------------*/
	case 0x1E:
		halt = true;
		break;

		/*-----------------------------------------------------------------
			Opcode: WAI
			Case opcode: 0x1D
			Case description: No operation
			Flags: N/A
			Addresssing type: Immidiate
			Register: N/A
		------------------------------------------------------------------*/
	case 0x1D:
		halt = true;
		break;


		/*-----------------------------------------------------------------
			Opcode: RCR
			Case opcode: 0x46
			Case description: Rotates right through memory with carry
			Flags: N, Z, C
			Addresssing type: Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x46:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			if ((Memory[address] & 0x01) == 0x01)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			Memory[address] = (Memory[address] >> 1);
			set_zn_flags(Memory[address]);
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: RCR
			Case opcode: 0x56
			Case description: Rotates right through memory with carry
			Flags: N, Z, C
			Addresssing type: Indexed Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x56: 
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			if ((Memory[address] & 0x01) == 0x01)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			Memory[address] = (Memory[address] >> 1) & 0x7F;
			set_zn_flags(Memory[address]);
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: RCRA
			Case opcode: 0x66
			Case description: Rotates right through the accumulator with carry
			Flags: N, Z, C
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x66: 
		saved_flags = Flags;
		if ((Registers[REGISTER_A] & 0x01) == 0x01)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0x7F;
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}
		set_zn_flags(Registers[REGISTER_A]);
		break;

		/*-----------------------------------------------------------------
			Opcode: RCL
			Case opcode: 0x47
			Case description: Rotates left through memory with carry
			Flags: N, Z, C
			Addresssing type: Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x47:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;

		if (address >= 0 && address < MEMORY_SIZE)
		{
			if ((Memory[address] & 0x80) == 0x80)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			Memory[address] = (Memory[address] << 1);
			set_zn_flags(Memory[address]);
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: RCL
			Case opcode: 0x57
			Case description: Rotates left through memory with carry
			Flags: N, Z, C
			Addresssing type: Indexed Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x57: 
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			if ((Memory[address] & 0x80) == 0x80)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			Memory[address] = (Memory[address] << 1);
			set_zn_flags(Memory[address]);
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: RCLA
			Case opcode: 0x67
			Case description: Rotates left through the accumulator with carry
			Flags: N, Z, C
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x67: 
		saved_flags = Flags;
		if ((Registers[REGISTER_A] & 0x80) == 0x80)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;

		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}
		set_zn_flags(Registers[REGISTER_A]);
		break;

		/*-----------------------------------------------------------------
			Opcode: SAL
			Case opcode: 0x48
			Case description: Arithmetic shifts left through memory with carry
			Flags: N, Z, C
			Addresssing type: Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x48: 
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{

			if ((Memory[address] & 0x80) == 0x80)
			{
				Flags = Flags | FLAG_C; 
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			Memory[address] = Memory[address] << 1;
			set_zn_flags(Memory[address]);
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: SAL
			Case opcode: 0x58
			Case description: Arithmetic shifts left through memory with carry
			Flags: N, Z, C
			Addresssing type: Indexed Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x58: 
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			if ((Memory[address] & 0x80) == 0x80)
			{
				Flags = Flags | FLAG_C; 
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C); 
			}
			Memory[address] = (Memory[address] << 1);
			set_zn_flags(Memory[address]);
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: SALA
			Case opcode: 0x68
			Case description: Arithmetic shifts left through the accumulator
							  with carry
			Flags: N, Z, C
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x68: 

		if ((Registers[REGISTER_A] & 0x80) == 0x80)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] << 1);

		set_zn_flags(Registers[REGISTER_A]);
		break;

		/*-----------------------------------------------------------------
			Opcode: ASR
			Case opcode: 0x49
			Case description: Arithmetic shifts right through memory with carry
			Flags: N, Z, C
			Addresssing type: Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x49: 
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{

			if ((Memory[address] & 0x01) == 0x01)
			{
				Flags = Flags | FLAG_C; 
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C); 
			}
			Memory[address] = (Memory[address] >> 1) & 0x7F;
			set_zn_flags(Memory[address]);
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: ASR
			Case opcode: 0x59
			Case description: Arithmetic shifts right through memory with carry
			Flags: N, Z, C
			Addresssing type: Indexed Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x59: 
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			if ((Memory[address] & 0x01) == 0x01)
			{
				Flags = Flags | FLAG_C; 
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C); 
			}
			Memory[address] = (Memory[address] >> 1) & 0x7F;
			set_zn_flags(Memory[address]);
		}
		break;

		/*-----------------------------------------------------------------
			Opcode: ASRA
			Case opcode: 0x69
			Case description: Arithmetic shifts right through the accumulator
							  with carry
			Flags: N, Z, C
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x69: 
		if ((Registers[REGISTER_A] & 0x01) == 0x01)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0x7F;

		if ((Flags & FLAG_N) == FLAG_N)
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x80;
		}
		set_zn_flags(Registers[REGISTER_A]);
		break;

		/*-----------------------------------------------------------------
			Opcode: ROL
			Case opcode: 0x4B
			Case description: Arithmetic shifts left through memory without
							  carry
			Flags: N, Z
			Addresssing type: Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x4B: 
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;

		if (address >= 0 && address < MEMORY_SIZE)
		{
			if ((Memory[address] & 0x80) == 0x80)
			{
				Memory[address] = (Memory[address] << 1);
				Memory[address] = Memory[address] | 0x01;
			}
			else
			{
				Memory[address] = (Memory[address] << 1);
				Memory[address] = Memory[address] & 0xFE;
			}

		} 
		set_zn_flags(Memory[address]);
		break;

		/*-----------------------------------------------------------------
			Opcode: ROL
			Case opcode: 0x5B
			Case description: Arithmetic shifts left through memory without
							  carry
			Flags: N, Z
			Addresssing type: Indexed Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x5B: 
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			if ((Memory[address] & 0x80) == 0x80)
			{
				Memory[address] = (Memory[address] << 1);
				Memory[address] = Memory[address] | 0x01;
			}
			else
			{
				Memory[address] = (Memory[address] << 1);
				Memory[address] = Memory[address] & 0xFE;
			}
		}
		set_zn_flags(Memory[address]);
		break;

		/*-----------------------------------------------------------------
			Opcode: ROLA
			Case opcode: 0x6B
			Case description: Arithmetic shifts left through the accumulator
							  without carry
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x6B: 
		if ((Registers[REGISTER_A] & 0x80) == 0x80)
		{
			Registers[REGISTER_A] = (Registers[REGISTER_A] << 1);
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}
		else
		{
			Registers[REGISTER_A] = (Registers[REGISTER_A] << 1);
			Registers[REGISTER_A] = Registers[REGISTER_A] & 0xFE;
		}

		set_zn_flags(Registers[REGISTER_A]);
		break;

		/*-----------------------------------------------------------------
			Opcode: ROR
			Case opcode: 0x4C
			Case description: Arithmetic shifts right through memory without
							  carry
			Flags: N, Z
			Addresssing type: Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x4C: 
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			if ((Memory[address] & 0x01) == 0x01)
			{
				Memory[address] = (Memory[address] >> 1);
				Memory[address] = Memory[address] | 0x80;
			}
			else
			{
				Memory[address] = (Memory[address] >> 1);
				Memory[address] = Memory[address] & 0x7F;
			}

		}
		set_zn_flags(Memory[address]);
		break;

		/*-----------------------------------------------------------------
			Opcode: ROR
			Case opcode: 0x5C
			Case description: Arithmetic shifts right through memory without
							  carry
			Flags: N, Z
			Addresssing type: Indexed Absolute
			Register: N/A
		------------------------------------------------------------------*/
	case 0x5C: 
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			if ((Memory[address] & 0x01) == 0x01)
			{
				Memory[address] = (Memory[address] >> 1);
				Memory[address] = Memory[address] | 0x80;
			}
			else
			{
				Memory[address] = (Memory[address] >> 1);
				Memory[address] = Memory[address] & 0x7F;
			}
		}

		set_zn_flags(Memory[address]);
		break;

		/*-----------------------------------------------------------------
			Opcode: RORA
			Case opcode: 0x6C
			Case description: Arithmetic shifts right through the accumulator
							  without carry
			Flags: N, Z
			Addresssing type: N/A
			Register: REGISTER_A
		------------------------------------------------------------------*/
	case 0x6C: 
		if ((Registers[REGISTER_A] & 0x01) == 0x01)
		{
			Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1);
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x80;
		}
		else
		{
			Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1);
			Registers[REGISTER_A] = Registers[REGISTER_A] & 0x7F;
		}
		set_zn_flags(Registers[REGISTER_A]);
		break;

	}



}

/*--------------------------------------------------------------------------------------
			Function Name: Group_2_Move
			Parameter list: (BYTE) opcode
			Description: Transfers from one register to another
			Return: N/A
----------------------------------------------------------------------------------------*/
void Group_2_Move(BYTE opcode)
{
	int destination;
	int source;

	int destReg;
	int sourceReg;

	WORD address;

	destination = opcode & 0x0F;

	switch (destination)
	{
	case 0x08:
		destReg = REGISTER_A;
		break;

	case 0x09:
		destReg = REGISTER_B;
		break;

	case 0x0A:
		destReg = REGISTER_C;
		break;

	case 0x0B:
		destReg = REGISTER_D;
		break;

	case 0x0C:
		destReg = REGISTER_E;
		break;

	case 0x0D:
		destReg = REGISTER_L;
		break;

	case 0x0E:
		destReg = REGISTER_H;
		break;

	case 0x0F:
		destReg = REGISTER_M;
		break;
	}

	source = (opcode >> 4) & 0x0F;

	switch (source)
	{
	case 0x07:
		sourceReg = REGISTER_A;
		break;
	case 0x08:
		sourceReg = REGISTER_B;
		break;
	case 0x09:
		sourceReg = REGISTER_C;
		break;
	case 0x0A:
		sourceReg = REGISTER_D;
		break;
	case 0x0B:
		sourceReg = REGISTER_E;
		break;
	case 0x0C:
		sourceReg = REGISTER_L;
		break;
	case 0x0D:
		sourceReg = REGISTER_H;
		break;
	case 0x0E:
		sourceReg = REGISTER_M;
		break;
	}

	if (sourceReg == REGISTER_M)
	{
		address = (WORD)Registers[REGISTER_H] << 8 + (WORD)Registers[REGISTER_L];
		Registers[REGISTER_M] = Memory[address];
	}

	Registers[destReg] = Registers[sourceReg];

	if (destReg == REGISTER_M)
	{
		address = (WORD)Registers[REGISTER_H] << 8 + (WORD)Registers[REGISTER_L];
		Memory[address] = Registers[REGISTER_M];
	}
}


void execute(BYTE opcode)
{
	if (((opcode >= 0x78) && (opcode <= 0x7F))
		|| ((opcode >= 0x88) && (opcode <= 0x8F))
		|| ((opcode >= 0x98) && (opcode <= 0x9F))
		|| ((opcode >= 0xA8) && (opcode <= 0xAF))
		|| ((opcode >= 0xB8) && (opcode <= 0xBF))
		|| ((opcode >= 0xC8) && (opcode <= 0xCF))
		|| ((opcode >= 0xD8) && (opcode <= 0xDF))
		|| ((opcode >= 0xE8) && (opcode <= 0xEF)))
	{
		Group_2_Move(opcode);
	}
	else
	{
		Group_1(opcode);
	}
}

void emulate()
{
	BYTE opcode;
	int sanity;
	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;
	sanity = 0;
	printf("                    A  B  C  D  E  L  H  X    SP\n");

	while ((!halt) && (memory_in_range)) {
		sanity++;
		if (sanity > 500) halt = true;
		printf("%04X ", ProgramCounter);           // Print current address
		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);
		printf("%02X ", Registers[REGISTER_B]);
		printf("%02X ", Registers[REGISTER_C]);
		printf("%02X ", Registers[REGISTER_D]);
		printf("%02X ", Registers[REGISTER_E]);
		printf("%02X ", Registers[REGISTER_L]);
		printf("%02X ", Registers[REGISTER_H]);
		printf("%04X ", IndexRegister);
		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_I) == FLAG_I)
		{
			printf("I=1 ");
		}
		else
		{
			printf("I=0 ");
		}
		if ((Flags & FLAG_N) == FLAG_N)
		{
			printf("N=1 ");
		}
		else
		{
			printf("N=0 ");
		}
		if ((Flags & FLAG_V) == FLAG_V)
		{
			printf("V=1 ");
		}
		else
		{
			printf("V=0 ");
		}
		if ((Flags & FLAG_Z) == FLAG_Z)
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}
		if ((Flags & FLAG_C) == FLAG_C)
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
	}

	printf("\n");  // New line
}


////////////////////////////////////////////////////////////////////////////////
//                            Simulator/Emulator (End)                        //
////////////////////////////////////////////////////////////////////////////////


void initialise_filenames() {
	int i;

	for (i = 0; i < MAX_FILENAME_SIZE; i++) {
		hex_file[i] = '\0';
		trc_file[i] = '\0';
	}
}




int find_dot_position(char* filename) {
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		if (chr == '.') {
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char* filename) {
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char* filename) {
	bool exists;
	FILE* ifp;

	exists = false;

	if ((ifp = fopen(filename, "r")) != NULL) {
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char* filename) {
	FILE* ofp;

	if ((ofp = fopen(filename, "w")) != NULL) {
		fclose(ofp);
	}
}



bool getline(FILE* fp, char* buffer) {
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect) {
		c = getc(fp);

		switch (c) {
		case EOF:
			if (i > 0) {
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0) {
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}






void load_and_run(int args, _TCHAR** argv) {
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE* ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	if (args == 2) {
		ln = 0;
		chr = argv[1][ln];
		while (chr != '\0')
		{
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file[ln] = chr;
				trc_file[ln] = chr;
				ln++;
			}
			chr = argv[1][ln];
		}
	}
	else {
		ln = 0;
		chr = '\0';
		while (chr != '\n') {
			chr = getchar();

			switch (chr) {
			case '\n':
				break;
			default:
				if (ln < MAX_FILENAME_SIZE) {
					hex_file[ln] = chr;
					trc_file[ln] = chr;
					ln++;
				}
				break;
			}
		}

	}
	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0) {
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	}
	else {
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0) {
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	}
	else {
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file)) {
		// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		Registers[REGISTER_E] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		IndexRegister = 0;
		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;

		for (i = 0; i < MEMORY_SIZE; i++) {
			Memory[i] = 0x00;
		}

		// Load hex file

		if ((ifp = fopen(hex_file, "r")) != NULL) {
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer)) {
				if (sscanf(InputBuffer, "L=%x", &address) == 1) {
					load_at = address;
				}
				else if (sscanf(InputBuffer, "%x", &code) == 1) {
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				}
				else {
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate

		emulate();
	}
	else {
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}

void building(int args, _TCHAR** argv) {
	char buffer[1024];
	load_and_run(args, argv);
	sprintf(buffer, "0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X",
		Memory[TEST_ADDRESS_1],
		Memory[TEST_ADDRESS_2],
		Memory[TEST_ADDRESS_3],
		Memory[TEST_ADDRESS_4],
		Memory[TEST_ADDRESS_5],
		Memory[TEST_ADDRESS_6],
		Memory[TEST_ADDRESS_7],
		Memory[TEST_ADDRESS_8],
		Memory[TEST_ADDRESS_9],
		Memory[TEST_ADDRESS_10],
		Memory[TEST_ADDRESS_11],
		Memory[TEST_ADDRESS_12]
	);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));
}



void test_and_mark() {
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;
	int  passed;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete) {
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&client_addr, &len) != SOCKET_ERROR) {
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 1)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1) {
				testing_complete = true;
				printf("Current mark = %d\n", mark);

			}
			else if (sscanf(buffer, "Tests passed %d", &passed) == 1) {
				//testing_complete = true;
				printf("Passed = %d\n", passed);

			}
			else if (strcmp(buffer, "Error") == 0) {
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			}
			else {
				// Clear Registers and Memory

				Registers[REGISTER_A] = 0;
				Registers[REGISTER_B] = 0;
				Registers[REGISTER_C] = 0;
				Registers[REGISTER_D] = 0;
				Registers[REGISTER_E] = 0;
				Registers[REGISTER_L] = 0;
				Registers[REGISTER_H] = 0;
				IndexRegister = 0;
				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i = 0; i < MEMORY_SIZE; i++) {
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				FILE* ofp;
				fopen_s(&ofp, "branch.txt", "a");

				while (!end_of_program) {
					chr = buffer[i];
					switch (chr) {
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1) {
							load_at = address;
						}
						else if (sscanf(InputBuffer, "%x", &code) == 1) {
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
								Memory[load_at] = (BYTE)code;
								fprintf(ofp, "%02X\n", (BYTE)code);
							}
							load_at++;
						}
						else {
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}
				fclose(ofp);
				// Emulate

				if (load_at > 1) {
					emulate();
					// Send and store results
					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X",
						Memory[TEST_ADDRESS_1],
						Memory[TEST_ADDRESS_2],
						Memory[TEST_ADDRESS_3],
						Memory[TEST_ADDRESS_4],
						Memory[TEST_ADDRESS_5],
						Memory[TEST_ADDRESS_6],
						Memory[TEST_ADDRESS_7],
						Memory[TEST_ADDRESS_8],
						Memory[TEST_ADDRESS_9],
						Memory[TEST_ADDRESS_10],
						Memory[TEST_ADDRESS_11],
						Memory[TEST_ADDRESS_12]
					);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock) {
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		if (argc == 2) { building(argc, argv); exit(0); }
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run(argc, argv);
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}


