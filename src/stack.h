#pragma once
#include <cstdint>

typedef enum type_tag {
  type_boolean,
  type_int,
  type_float,
  type_double,
  type_string,
  type_array,
} type_tag;

typedef struct array {
  type_tag tag;
  size_t length;
  union stack_value *elements;
} array;

typedef union stack_value {
  bool boolean_value;
  int int_value;
  float float_value;
  double double_value;
  char *string_value;
  array array_value;
} stack_value;

typedef struct stack_entry {
  type_tag tag;
  stack_value value;
} stack_entry;

void print_stack_entry(stack_entry *entry);

typedef struct stack {
  stack_entry *entries;
  uint16_t size;
  uint16_t sp;
} stack;

void stack_init(stack *s, uint16_t size);
void stack_reset(stack *s);
bool stack_empty(stack *s);

void stack_push(stack *s, bool value);
void stack_push(stack *s, int value);
void stack_push(stack *s, float value);
void stack_push(stack *s, double value);
void stack_push(stack *s, const char *value);
void stack_push(stack *s, array value);
void stack_push(stack *s, stack_entry entry);

void stack_peek(stack *s, stack_entry *entry);
void stack_pop(stack *s, stack_entry *entry);
