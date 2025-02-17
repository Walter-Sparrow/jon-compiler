#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

void stack_init(stack *s, size_t size) {
  s->entries = (value *)malloc(size * sizeof(value));
  s->size = size;
  s->sp = 0;
}

void stack_reset(stack *s) { s->sp = 0; }
bool stack_empty(stack *s) { return s->sp == 0; }

#define VALIDATE_STACK(s)                                                      \
  if (s->sp == s->size) {                                                      \
    perror("stack is full");                                                   \
    return;                                                                    \
  }

void stack_push_bool(stack *s, bool value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].type = VALUE_BOOLEAN;
  s->entries[s->sp++].as.boolean_value = value;
}

void stack_push_int(stack *s, int value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].type = VALUE_INT;
  s->entries[s->sp++].as.int_value = value;
}

void stack_push_float(stack *s, float value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].type = VALUE_FLOAT;
  s->entries[s->sp++].as.float_value = value;
}

void stack_push_double(stack *s, double value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].type = VALUE_DOUBLE;
  s->entries[s->sp++].as.double_value = value;
}

void stack_push_string(stack *s, const char *value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].type = VALUE_STRING;
  s->entries[s->sp++].as.string_value = (char *)value;
}

void stack_push_array(stack *s, array *value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].type = VALUE_ARRAY;
  s->entries[s->sp++].as.array_value = value;
}

void stack_push(stack *s, value val) {
  VALIDATE_STACK(s);
  s->entries[s->sp++] = val;
}

void stack_peek(const stack *s, value *out) {
  if (s->sp == 0) {
    perror("stack is empty");
    return;
  }

  *out = s->entries[s->sp - 1];
}

void stack_pop(stack *s, value *out) {
  if (s->sp == 0) {
    perror("stack is empty");
    return;
  }

  *out = s->entries[--s->sp];
}
