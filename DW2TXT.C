/*David Whittaker (GB/GBC) to MIDI converter*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt;
long bank;
long tablePtrLoc;
long tableOffset;
int i, j;
char outfile[1000000];
int songNum;
long songPtrs[4];
long firstPtrs[4];
long curSpeed;
long bankAmt;
long nextPtr;
int highestSeq;
unsigned static char* romData;
unsigned long seqList[500];

const unsigned char MagicBytes[5] = { 0x22, 0x05, 0x20, 0xFC, 0x21 };

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long ptrList[4], long nextPtr, int curSpeed);
void seqs2txt(unsigned long list[]);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("David Whittaker (GB/GBC) to TXT converter\n");
	if (args != 3)
	{
		printf("Usage: DW2TXT <rom> <bank>\n");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			bank = strtol(argv[2], NULL, 16);
			if (bank != 1)
			{
				bankAmt = bankSize;
			}
			else
			{
				bankAmt = 0;
			}
		}

		if (bank != 1)
		{
			fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize);
			fread(romData, 1, bankSize, rom);
			fclose(rom);
		}

		else
		{
			fseek(rom, ((bank - 1) * bankSize*2), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize*2);
			fread(romData, 1, bankSize*2, rom);
			fclose(rom);
		}


		/*Try to search the bank for base table*/
		for (i = 0; i < bankSize; i++)
		{
			if ((!memcmp(&romData[i], MagicBytes, 5) && ReadLE16(&romData[i+5]) < 0x8000))
			{
				tablePtrLoc = bankAmt + i + 5;
				printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
				tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
				printf("Song table starts at 0x%04x...\n", tableOffset);
				break;
			}
		}

		if (tableOffset != 0)
		{
			for (i = 0; i < 500; i++)
			{
				seqList[i] = 0;
			}
			songNum = 1;
			i = tableOffset;
			highestSeq = 0;
			firstPtrs[0] = ReadLE16(&romData[i + 1 - bankAmt]);
			firstPtrs[1] = ReadLE16(&romData[i + 3 - bankAmt]);
			firstPtrs[2] = ReadLE16(&romData[i + 5 - bankAmt]);
			while ((i < firstPtrs[0]) && (i < firstPtrs[1]) && (i < firstPtrs[2]) && ReadLE16(&romData[i + 2 - bankAmt]) != 0)
			{
				curSpeed = romData[i - bankAmt];
				printf("Song %i tempo: 0x%01x\n", songNum, curSpeed);
				songPtrs[0] = ReadLE16(&romData[i + 1 - bankAmt]);
				printf("Song %i channel 1: 0x%04x\n", songNum, songPtrs[0]);
				songPtrs[1] = ReadLE16(&romData[i + 3 - bankAmt]);
				printf("Song %i channel 2: 0x%04x\n", songNum, songPtrs[1]);
				songPtrs[2] = ReadLE16(&romData[i + 5 - bankAmt]);
				printf("Song %i channel 3: 0x%04x\n", songNum, songPtrs[2]);
				songPtrs[3] = ReadLE16(&romData[i + 7 - bankAmt]);
				printf("Song %i channel 4: 0x%04x\n", songNum, songPtrs[3]);
				nextPtr = ReadLE16(&romData[i + 10 - bankAmt]);
				song2txt(songNum, songPtrs, nextPtr, curSpeed);
				i += 9;
				songNum++;
			}
			seqs2txt(seqList);
		}
		else
		{
			printf("ERROR: Magic bytes not found!\n");
			exit(-1);
		}
		printf("The operation was successfully completed!\n");

	}
}

void song2txt(int songNum, long ptrList[4], long nextPtr, int curSpeed)
{
	int patPos = 0;
	int seqPos = 0;
	long curSeq = 0;
	long command[3];
	unsigned char lowNibble = 0;
	unsigned char highNibble = 0;
	int curChan = 0;
	int endSeq = 0;
	int endChan = 0;
	int curTempo = curSpeed;
	int transpose = 0;
	int curVol = 0;
	long jumpPos = 0;


	sprintf(outfile, "song%d.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file song%d.txt!\n", songNum);
		exit(2);
	}
	else
	{
		for (curChan = 0; curChan < 4; curChan++)
		{
			endChan = 0;
			patPos = songPtrs[curChan] - bankAmt;
			fprintf(txt, "Channel %i:\n", curChan + 1);
			while (endChan == 0)
			{
				curSeq = ReadLE16(&romData[patPos]);
				if (curSeq != 0)
				{
					seqPos = curSeq - bankAmt;
					fprintf(txt, "Sequence position: %04x\n", curSeq);
					for (j = 0; j < 500; j++)
					{
						if (seqList[j] == curSeq)
						{
							break;
						}
					}
					if (j == 500)
					{
						seqList[highestSeq] = curSeq;
						highestSeq++;
					}
					patPos += 2;
				}
				else
				{
					endChan = 1;
				}
			}
			fprintf(txt, "\n");
		}

		fclose(txt);
	}
}

void seqs2txt(unsigned long list[])
{
	int seqPos = 0;
	int songEnd = 0;
	int lowestSeq = 0;
	int curSeq = 0;

	long command[3];
	unsigned char lowNibble = 0;
	unsigned char highNibble = 0;
	int curChan = 0;
	int endSeq = 0;
	int endChan = 0;
	int curTempo = curSpeed;
	int transpose = 0;
	int globalTranspose = 0;
	int curNote = 0;
	int curNoteLen = 0;
	int curVol = 0;
	long jumpPos = 0;
	long jumpPosAbs = 0;

	sprintf(outfile, "seqs.txt");
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file seqs.txt!\n");
		exit(2);
	}
	else
	{
		
		seqPos = seqList[curSeq];

		while (seqList[curSeq] != 0 && endSeq == 0)
		{
			if (seqPos == seqList[curSeq])
			{
				fprintf(txt, "Sequence 0x%04X:\n", seqList[curSeq]);
			}
			command[0] = romData[seqPos - bankAmt];
			command[1] = romData[seqPos + 1 - bankAmt];
			command[2] = romData[seqPos + 2 - bankAmt];


			/*Change tempo*/
			if (command[0] == 0xF4)
			{
				curTempo = command[1];
				fprintf(txt, "Change tempo: %i\n", curTempo);
				seqPos += 2;
			}

			/*Set channel loop point and end sequence*/
			else if (command[0] == 0xF5)
			{
				jumpPos = ReadLE16(&romData[seqPos + 1 - bankAmt]);
				jumpPosAbs = ReadLE16(&romData[jumpPos - bankAmt]);
				fprintf(txt, "Set channel loop point and end sequence: %04X (%04X)\n\n", jumpPos, jumpPosAbs);
				curSeq++;
				seqPos = seqList[curSeq];
			}

			/*Set envelope fade in/out speed*/
			else if (command[0] == 0xF6)
			{
				lowNibble = command[1] >> 4;
				highNibble = command[1] & 15;
				curVol = lowNibble;
				if (curChan != 2)
				{
					fprintf(txt, "Envelope fade in: %i\n", lowNibble);
					fprintf(txt, "Envelope fade out: %i\n", highNibble);
					seqPos += 2;
				}
				else
				{
					fprintf(txt, "Envelope fade in: %i\n", command[1]);
					fprintf(txt, "Note size: %i\n", command[2]);
					seqPos += 3;
				}

			}

			/*Set vibrato*/
			else if (command[0] == 0xF7)
			{
				fprintf(txt, "Set vibrato: %i\n", command[1]);
				seqPos += 2;
			}

			/*Rest*/
			else if (command[0] == 0xF8)
			{
				fprintf(txt, "Rest\n");
				seqPos++;
			}

			/*Hold note*/
			else if (command[0] == 0xF9)
			{
				fprintf(txt, "Hold note\n");
				seqPos++;
			}

			/*Set instrument duty*/
			else if (command[0] == 0xFA)
			{
				fprintf(txt, "Set instrument duty: %i\n", command[1]);
				seqPos += 2;
			}

			/*Transpose all channels*/
			else if (command[0] == 0xFB)
			{
				transpose = (signed char)command[1];
				fprintf(txt, "Transpose all channels: %i\n", transpose);
				seqPos += 2;
			}

			/*Transpose current channel*/
			else if (command[0] == 0xFC)
			{
				transpose = (signed char)command[1];
				fprintf(txt, "Transpose current channel: %i\n", transpose);
				seqPos += 2;
			}

			/*Portamento*/
			else if (command[0] == 0xFD)
			{
				fprintf(txt, "Portamento: %i\n", command[1]);
				seqPos += 2;
			}

			/*End of channel (no loop)*/
			else if (command[0] == 0xFE)
			{
				fprintf(txt, "End of channel (no loop)\n\n");
				curSeq++;
				seqPos = seqList[curSeq];
			}

			/*End of sequence*/
			else if (command[0] == 0xFF)
			{
				fprintf(txt, "End of sequence\n\n");
				curSeq++;
				seqPos = seqList[curSeq];
			}

			/*Set note length*/
			else if (command[0] >= 0x60 && command[0] <= 0x80)
			{
				curNoteLen = command[0] - 0x60;
				fprintf(txt, "Set note length: %i\n", curNoteLen);
				seqPos++;
			}

			else if (command[0] < 0x60)
			{
				curNote = command[0];
				fprintf(txt, "Note: %i\n", curNote);
				seqPos++;
			}
		}

		fclose(txt);
	}

}