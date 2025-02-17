#pragma once

#include <stdbool.h>
#include <stdint.h>

struct array;
struct object;

typedef enum value_type {
  VALUE_NULL,
  VALUE_BOOLEAN,
  VALUE_INT,
  VALUE_FLOAT,
  VALUE_DOUBLE,
  VALUE_STRING,
  VALUE_ARRAY,
  VALUE_OBJECT,
} value_type;

typedef struct value {
  value_type type;
  union {
    bool boolean_value;
    int int_value;
    float float_value;
    double double_value;
    char *string_value;
    struct array *array_value;
    struct object *object_value;
  } as;
} value;

typedef struct array {
  size_t length;
  value *elements;
} array;

void array_init(array *arr, size_t length);
array *array_new(size_t length);

typedef struct object {
  size_t capacity;
  size_t size;
  char **keys;
  value *values;
} object;

void object_init(object *obj, size_t capacity);
bool object_get_value(object *obj, const char *key, value *out);
bool object_set_value(object *obj, const char *key, value val);
