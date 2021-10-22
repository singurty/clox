#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	chunk->lineCapacity = 0;
	chunk->lineCount = 0;
	chunk->lines = NULL;
	initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
	if (chunk->capacity < chunk->count + 1) {
		int oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
	}
	chunk->code[chunk->count] = byte;
	chunk->count++;

	// Run-length encoding for line numbers
	if (chunk->lineCount && chunk->lines[chunk->lineCount - 1] == line) {
		chunk->lines[chunk->lineCount - 2]++;
		return;
	} else if (chunk->lineCapacity < chunk->count + 2) {
		int oldCapacity = chunk->lineCapacity;
		chunk->lineCapacity = GROW_CAPACITY(oldCapacity);
		chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
	}
	chunk->lines[chunk->lineCount] = 1;
	chunk->lines[chunk->lineCount + 1] = line;
	chunk->lineCount += 2;
}

int addConstant(Chunk* chunk, Value value) {
	writeValueArray(&chunk->constants, value);
	return chunk->constants.count - 1;
}

void freeChunk(Chunk* chunk) {
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);
	freeValueArray(&chunk->constants);
	initChunk(chunk);
}
