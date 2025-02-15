#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

void print_stack_entry(stack_entry *entry) {
  switch (entry->tag) {
    case type_boolean:
      printf("%s", entry->value.boolean_value ? "true" : "false");
      break;
    case type_int:
      printf("%d", entry->value.int_value);
      break;
    case type_float:
      printf("%f", entry->value.float_value);
      break;
    case type_double:
      printf("%f", entry->value.double_value);
      break;
    case type_string:
      printf("\"%s\"", entry->value.string_value);
      break;
    case type_array:
      printf("array: [");
      for (size_t i = 0; i < entry->value.array_value.length; i++) {
        stack_entry e;
        e.tag = entry->value.array_value.tag;
        e.value = entry->value.array_value.elements[i];
        print_stack_entry(&e);
        if (i != entry->value.array_value.length - 1) {
          printf(", ");
        }
      }
      printf("]");
      break;
    default:
      printf("unknown tag: %d\n", entry->tag);
      break;
  }
}

void stack_init(stack *s, uint16_t size) {
  s->entries = (stack_entry *)malloc(size * sizeof(stack_entry));
  s->size = size;
  s->sp = 0;
}

void stack_reset(stack *s) { s->sp = 0; }

bool stack_empty(stack *s) { return s->sp == 0; }

#define VALIDATE_STACK(s)    \
  if (s->sp == s->size) {    \
    perror("stack is full"); \
    return;                  \
  }

void stack_push(stack *s, bool value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].tag = type_boolean;
  s->entries[s->sp++].value.boolean_value = value;
}

void stack_push(stack *s, int value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].tag = type_int;
  s->entries[s->sp++].value.int_value = value;
}

void stack_push(stack *s, float value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].tag = type_float;
  s->entries[s->sp++].value.float_value = value;
}

void stack_push(stack *s, double value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].tag = type_double;
  s->entries[s->sp++].value.double_value = value;
}

void stack_push(stack *s, const char *value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].tag = type_string;
  s->entries[s->sp++].value.string_value = (char *)value;
}

void stack_push(stack *s, array value) {
  VALIDATE_STACK(s);
  s->entries[s->sp].tag = type_array;
  s->entries[s->sp++].value.array_value = value;
}

void stack_push(stack *s, stack_entry entry) {
  VALIDATE_STACK(s);
  s->entries[s->sp++] = entry;
}

void stack_peek(stack *s, stack_entry *entry) {
  if (s->sp == 0) {
    perror("stack is empty");
    return;
  }

  *entry = s->entries[s->sp - 1];
}

void stack_pop(stack *s, stack_entry *entry) {
  if (s->sp == 0) {
    perror("stack is empty");
    return;
  }

  *entry = s->entries[--s->sp];
}
