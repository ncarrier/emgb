#ifndef __ROM__
#define __ROM__

#define MAX_TITLE_LENGTH 16
#define LOGO_LENGTH 48
#define HEADER_OFFSET_S 0x0100
#define HEADER_OFFSET_E 0x014F

#pragma pack(push, 1)

struct              s_romHeader
{
	int		entrypoint;
	char		nlogo[LOGO_LENGTH];
	char		title[MAX_TITLE_LENGTH];
	unsigned short	manufacturerCode;
	unsigned char	cgbFlag;
	unsigned char	cartridgeType;
	unsigned char	romSize;
	unsigned char	ramSize;
	unsigned char	destCode; //00 JPN 01 N-JPN
	unsigned char	oldLicenseeCode;
	unsigned char	gameVersion;
	unsigned char	headerCheckSum;
	unsigned short	glbCheckSum;
} __attribute__((__packed__));

struct					s_rom
{
	unsigned int			size;
	unsigned char	*		rom;
	struct s_romHeader		romheader;
} __attribute__((__packed__));

#pragma pack(pop)

int initRom(struct s_rom *rom, const char *filename);
void displayHeader(struct s_romHeader *romheader);

#endif
