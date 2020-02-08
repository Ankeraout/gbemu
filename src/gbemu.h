#ifndef __GBEMU_H_INCLUDED__
#define __GBEMU_H_INCLUDED__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define PACKED_STRUCT(__declaration__) struct __pragma(pack(push, 1)) __declaration__ __pragma(pack(pop))
#elif defined(__GNUC__)
#define PACKED_STRUCT(__declaration__) struct __declaration__ __attribute__((packed))
#else
#error Unsupported build environment.
#endif

#define UNUSED_PARAMETER(__parameter__) (void)__parameter__

#ifdef DEBUG
#define gbemu_debug(message) \
fprintf(stdout, "%s\n", message);
#else
#define gbemu_debug(message)
#endif

#define gbemu_error(errorMessage) \
fprintf(stderr, "Fatal error: %s\n", errorMessage); \
exit(EXIT_FAILURE);

typedef struct {
	bool printOamAccessWarnings;
	bool printVramAccessWarnings;
	bool printCartridgeInfo;
	bool disableFpsLimit;
	bool redirectSerial;
	bool disableGraphics;
	bool disableSramLoad;
	bool disableSramSave;
	bool serialOutputHex;
	bool printDmaAccessWarnings;
	bool printDmaDebug;
	bool enableDmaAccess;
	unsigned int maxClocks;
	FILE *serialOutputFile;
	unsigned int screenScale;
} gbemu_options_t;

extern gbemu_options_t gbemu_options;

size_t getFileSize(FILE *file);
int readFile(FILE *file, uint8_t *buffer, size_t size);

#endif
