#include <stdio.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]) {
	Chunk chunk;
	initChunk(&chunk);
	writeChunk(&chunk, OP_RETURN, 1);
	int constant = addConstant(&chunk, 1.3);
	writeChunk(&chunk, OP_CONSTANT, 2);
	writeChunk(&chunk, constant, 2);
	constant = addConstant(&chunk, 14);
	writeChunk(&chunk, OP_CONSTANT, 2);
	writeChunk(&chunk, constant, 2);
	constant = addConstant(&chunk, 26);
	writeChunk(&chunk, OP_CONSTANT, 3);
	writeChunk(&chunk, constant, 3);
	dissassembleChunk(&chunk, "test chunk");
	freeChunk(&chunk);
	return 0;
}
