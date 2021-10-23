#include <stdio.h>

#include "debug.h"
#include "value.h"

int lineNumber;
int lineRepeat = 0; // how many types can this line number be applied
int lineIndex = 0; // where was this line number taken from

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
	int constant = chunk->code[offset + 1];
	printf("%s %d ", name, constant);
	printValue(chunk->constants.values[constant]);
	printf("\n");
	return offset + 2;
}

int dissassembleInstruction(Chunk* chunk, int offset) {
	if (!(lineRepeat > 0)) {
		lineRepeat = chunk->lines[lineIndex];
		lineNumber = chunk->lines[lineIndex + 1];
		lineIndex += 2;
	}
	lineRepeat--;
	printf("%04d [Line %d] ", offset, lineNumber);
	uint8_t instruction = chunk->code[offset];
	switch (instruction) {
		case OP_RETURN:
			return simpleInstruction("OP_RETURN", offset);
		case OP_CONSTANT:
			lineRepeat--;
			return constantInstruction("OP_CONSTANT", chunk, offset);
		case OP_NEGATE:
			return simpleInstruction("OP_NEGATE", offset);
		case OP_ADD:
			return simpleInstruction("OP_ADD", offset);
		case OP_SUBTRACT:
			return simpleInstruction("OP_SUBTRACT", offset);
		case OP_MULTIPLY:
			return simpleInstruction("OP_MULTIPLY", offset);
		case OP_DIVIDE:
			return simpleInstruction("OP_DIVIDE", offset);
		default:
			printf("Unknown opcode %d\n", instruction);
			return offset + 1;
	}
}
