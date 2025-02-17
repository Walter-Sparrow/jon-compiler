#pragma once

#include "value.h"

typedef struct stack {
  value *entries;
  size_t size;
  size_t sp;
} stack;

void stack_init(stack *s, size_t size);
void stack_reset(stack *s);
bool stack_empty(stack *s);

void stack_push_bool(stack *s, bool value);
void stack_push_int(stack *s, int value);
void stack_push_float(stack *s, float value);
void stack_push_double(stack *s, double value);
void stack_push_string(stack *s, const char *value);
void stack_push_array(stack *s, array *value);
void stack_push(stack *s, value val);

void stack_peek(const stack *s, value *out);
void stack_pop(stack *s, value *out);
