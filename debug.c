#include <stdio.h>

#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char* name) {
	int lineNumber;
	int lineRepeat = 0; // how many types can this line number be applied
	int lineIndex = 0; // where was this line number taken from

	printf("== %s ==\n", name);
	for (int offset = 0; offset < chunk->count;) {
		// decode run-length encoding for line numbers
		if (!(lineRepeat > 0)) {
			lineRepeat = chunk->lines[lineIndex];
			lineNumber = chunk->lines[lineIndex + 1];
			lineIndex += 2;
		}
		lineRepeat--;
		offset = disassembleInstruction(chunk, offset, lineNumber);
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

int disassembleInstruction(Chunk* chunk, int offset, int line) {
	printf("%04d [Line %d] ", offset, line);
	uint8_t instruction = chunk->code[offset];
	switch (instruction) {
		case OP_RETURN:
			return simpleInstruction("OP_RETURN", offset);
		case OP_CONSTANT:
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
		case OP_NIL:
			return simpleInstruction("OP_NIL", offset);
		case OP_TRUE:
			return simpleInstruction("OP_TRUE", offset);
		case OP_FALSE:
			return simpleInstruction("OP_FALSE", offset);
		case OP_NOT:
			return simpleInstruction("OP_NOT", offset);
		case OP_EQUAL:
			return simpleInstruction("OP_EQUAL", offset);
		case OP_LESS:
			return simpleInstruction("OP_LESS", offset);
		case OP_GREATER:
			return simpleInstruction("OP_GREATER", offset);
		default:
			printf("Unknown opcode %d\n", instruction);
			return offset + 1;
	}
}
