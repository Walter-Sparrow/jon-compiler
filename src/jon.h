#pragma once

#include "interpreter.h"

typedef enum jon_value_type {
  jon_value_type_null = 0x0,
  jon_value_type_boolean = 0x1,
  jon_value_type_int = 0x2,
  jon_value_type_float = 0x3,
  jon_value_type_double = 0x4,
  jon_value_type_string = 0x5,
  jon_value_type_array = 0x6,
  jon_value_type_object = 0x7,
} jon_value_type;

typedef struct jon_value_pair jon_value_pair;

typedef struct jon_array {
  u2 length;
  struct jon_value *values;
} jon_array;

typedef struct jon_value {
  jon_value_type type;
  union {
    bool boolean_value;
    int int_value;
    float float_value;
    double double_value;
    char *string_value;
    jon_array array_value;
    struct jon_value_pair *object_value;
  } value;
} jon_value;

typedef struct jon_value_pair {
  char *key;
  jon_value value;
} jon_value_pair;

void fill_jon_value_type(jon_value *value, utf8_info *descriptor);
int jon_object_set_value(jon_value_pair *object, u2 object_size, char *key,
                         jon_value value);
int stack_value_to_jon_value(type_tag tag, stack_value s_value,
                             jon_value *value);
int fill_jon_object(jon_value_pair *object, u2 object_size, class_file *c_file);
int serialize_jon_value_as_json(jon_value_type j_type, jon_value j_value,
                                FILE *out);
int serialize_jon_object_as_json(jon_value_pair *object, u2 object_size,
                                 FILE *out);
