#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"
#include "memory.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) (isObjType(value, OBJ_STRING))

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
	OBJ_STRING
} ObjType;

struct Obj {
	ObjType type;
};

struct ObjString {
	Obj obj;
	int length; // not necessary but lets us tell how much memory
                // is allocated for the string without walking the
                // chars array to find the null terminator
	char* chars;
};

ObjString* copyString(const char* chars, int length);
void printObject(Value value);

#endif