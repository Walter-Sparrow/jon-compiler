#include "value.h"
#include <stdlib.h>
#include <string.h>

void array_init(array *arr, size_t length) {
  arr->length = length;
  arr->elements = (value *)malloc(length * sizeof(value));
}

array *array_new(size_t length) {
  array *arr = (array *)malloc(sizeof(array));
  array_init(arr, length);
  return arr;
}

void object_init(object *obj, size_t capacity) {
  obj->capacity = capacity;
  obj->size = 0;
  obj->keys = (char **)malloc(capacity * sizeof(char *));
  obj->values = (value *)malloc(capacity * sizeof(value));
}

bool object_get_value(object *obj, const char *key, value *out) {
  for (size_t i = 0; i < obj->size; i++) {
    if (strcmp(obj->keys[i], key) == 0) {
      *out = obj->values[i];
      return true;
    }
  }

  return false;
}

bool object_set_value(object *obj, const char *key, value val) {
  if (obj->size == obj->capacity) {
    return false;
  }

  obj->keys[obj->size] = strdup(key);
  obj->values[obj->size] = val;
  obj->size++;

  return true;
}
