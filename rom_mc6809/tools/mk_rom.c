#include <stdint.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
	time_t t;
	time(&t);

	uint8_t romdata[1024];

	// read
	FILE *f;
	f = fopen("rom_unpatched.bin", "r");

	fseek(f, 0L, SEEK_END);
	long pos = ftell(f);

	printf("[mk_rom] rom_unpatched.bin size: %lu bytes\n", pos);
	if (pos > 1024L) {
		printf("[mk_rom] too large, exiting...\n");
		fclose(f);
		return 1;
	}

	// go back to beginning of file
	// read data
	rewind(f);
	fread(romdata, pos, 1, f);

	// fill up the rest of the final rom with zeroes
	for (int i=pos; i < 1024; i++) {
		romdata[i] = 0x00;
	}

	// close original unpatched bin file
	fclose(f);

	// write 8k output to cpp file
	printf("[mk_rom] writing 1024b image 'rom.bin' and 'rom.cpp' for inclusion in punch\n");
	f = fopen("rom.cpp","w");

	fprintf(f, "/*\n");
	fprintf(f, " * rom.cpp\n");
	fprintf(f, " * rom image for punch\n");
	fprintf(f, " *\n");
	fprintf(f, " * Copyright (C)2023-2024 elmerucr. All rights reserved.\n");
	fprintf(f, " * %s",ctime(&t));
	fprintf(f, " */\n\n");
	fprintf(f, "#include <cstdint>\n\n");
	fprintf(f, "uint8_t rom[1024] = {");

	for(int i = 0; i < 1023; i++) {
		if(i%16 == 0) fprintf(f, "\n\t");
		fprintf(f, "0x%02x,", romdata[i]);
	}

	fprintf(f, "0x%02x\n};\n", romdata[(1024)-1]);

	fclose(f);

	// write 512b output to bin file
	f = fopen("rom.bin","wb");
	fwrite(romdata, sizeof(romdata), 1, f);
	fclose(f);

	return 0;
}
