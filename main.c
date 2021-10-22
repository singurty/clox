#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]) {
	Chunk chunk;
	initChunk(&chunk);
	writeChunk(&chunk, OP_RETURN);
	int constant = addConstant(&chunk, 1.3);
	writeChunk(&chunk, OP_CONSTANT);
	writeChunk(&chunk, constant);
	dissassembleChunk(&chunk, "test chunk");
	freeChunk(&chunk);
	return 0;
}
