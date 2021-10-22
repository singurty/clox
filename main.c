#include <stdio.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
	initVM();

	Chunk chunk;
	initChunk(&chunk);

	int constant = addConstant(&chunk, 1.3);
	writeChunk(&chunk, OP_CONSTANT, 1);
	writeChunk(&chunk, constant, 1);

	constant = addConstant(&chunk, 14);
	writeChunk(&chunk, OP_CONSTANT, 1);
	writeChunk(&chunk, constant, 1);

	constant = addConstant(&chunk, 26);
	writeChunk(&chunk, OP_CONSTANT, 2);
	writeChunk(&chunk, constant, 2);

	writeChunk(&chunk, OP_RETURN, 3);

	dissassembleChunk(&chunk, "test chunk");
	interpret(&chunk);
	freeVM();
	freeChunk(&chunk);
	return 0;
}
