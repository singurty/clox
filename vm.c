#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"

VM vm;

static int lineNumber;
static int lineRepeat = 0; // how many types can this line number be applied
static int lineIndex = 0; // where was this line number taken from

static void resetStack() {
	vm.stackTop = vm.stack;
}

void initVM() {
	vm.stack = malloc(STACK_MAX);
	vm.capacity = STACK_MAX;
	resetStack();
	vm.objects = NULL;
}

void freeVM() {
	free(vm.stack);
	freeObjects();
}

void push(Value value) {
	// expand stack if it'd overflow
	if ((vm.stack + vm.capacity) == vm.stackTop) {
		vm.stack = realloc(vm.stack, vm.capacity + vm.capacity / 4);
		vm.capacity = vm.capacity + vm.capacity / 4;
	}
	*vm.stackTop = value;
	vm.stackTop++;
}

Value pop() {
	char* before = (char*) vm.stackTop;
	vm.stackTop--;
	return *vm.stackTop;
}

Value peek(int distance) {
	return vm.stackTop[-1 - distance];
}

static void runtimeError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n[Line %d] in script\n", lineNumber);
	resetStack();
}

static bool isFalsey(Value value) {
	return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
	ObjString* b = AS_STRING(pop());
	ObjString* a = AS_STRING(pop());
	int length = a->length + b->length;
	char* chars = ALLOCATE(char, length + 1);
	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);
	chars[length] = '\0';

	ObjString* result = takeString(chars, length);
	push(OBJ_VAL(result));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op) \
do { \
	if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
		runtimeError("Operands must be numbers.");\
		return INTERPRET_RUNTIME_ERROR; \
	} \
	double b = AS_NUMBER(pop()); \
	double a = AS_NUMBER(pop()); \
	push(valueType(a op b)); \
} while (false)
	for (;;) {
		// decode run-length encoding for line numbers
		if (!(lineRepeat > 0)) {
			lineRepeat = vm.chunk->lines[lineIndex];
			lineNumber = vm.chunk->lines[lineIndex + 1];
			lineIndex += 2;
		}
		lineRepeat--;
#ifdef DEBUG_TRACE_EXECUTION
		for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
			printf("[ ");
			printValue(*slot);
			printf(" ]");
		}
		printf("\n");
		disassembleInstruction(vm.chunk, vm.ip - vm.chunk->code, lineNumber);
#endif
		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
			case OP_RETURN:
			{
				printValue(pop());
				printf("\n");
				return INTERPRET_OK;
			}
			case OP_CONSTANT:
			{
				Value constant = READ_CONSTANT();
				push(constant);
				break;
			}
			case OP_NEGATE:
			{
				if(!IS_NUMBER(peek(0))) {
					runtimeError("Operand must be a number.");
					return INTERPRET_RUNTIME_ERROR;
				}
				vm.stackTop[-1].as.number = -vm.stackTop[-1].as.number;
				break;
			}
			case OP_ADD:
			{
				if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
					BINARY_OP(NUMBER_VAL, +);
				} else if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
					concatenate();
				}
				break;
			}
			case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
			case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
			case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
			case OP_TRUE: push(BOOL_VAL(true)); break;
			case OP_FALSE: push(BOOL_VAL(false)); break;
			case OP_NIL: push(NIL_VAL); break;
			case OP_NOT: push(BOOL_VAL(isFalsey(pop()))); break;
			case OP_EQUAL:
			{
				Value b = pop();
				Value a = pop();
				push(BOOL_VAL(valuesEqual(a, b)));
				break;
			}
			case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
			case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
		}
	}
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
	Chunk chunk;
	initChunk(&chunk);
	if (!compile(source, &chunk)) {
		freeChunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}
	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;
	InterpretResult result = run();
	freeChunk(&chunk);
	return result;
}
