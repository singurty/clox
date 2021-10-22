#include <stdio.h>

#include "debug.h"
#include "value.h"

void dissassembleChunk(Chunk* chunk, const char* name) {
	printf("== %s ==\n", name);
	for (int offset = 0; offset < chunk->count;) {
		offset = dissassembleInstruction(chunk, offset);
	}
}

static int simpleInstruction(const char* name, int offset) {
	printf("%s\n", name);
	return offset + 1;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
	offset = simpleInstruction(name, offset);
	int constant = chunk->code[offset];
	printValue(chunk->constants.values[constant]);
	return offset + 1;
}

int dissassembleInstruction(Chunk* chunk, int offset) {
	printf("%04d ", offset);
	uint8_t instruction = chunk->code[offset];
	switch (instruction) {
		case OP_RETURN:
			return simpleInstruction("OP_RETURN", offset);
		case OP_CONSTANT:
			return constantInstruction("OP_CONSTANT", chunk, offset);
		default:
			printf("Unknown opcode %d\n", instruction);
			return offset + 1;
	}
}
