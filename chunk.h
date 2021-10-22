#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
	OP_RETURN,
	OP_CONSTANT
} OpCode;

typedef struct {
	int count;
	int capacity;
	uint8_t* code;
	int lineCount;
	int lineCapacity;
	int* lines;
	ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);
int writeConstant(Chunk* chunk, Value value, int line);

#endif
