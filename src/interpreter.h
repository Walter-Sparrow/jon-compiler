#pragma once

#include "value.h"
#include "parser.h"
#include "stack.h"

typedef enum opcode {
  op_nop = 0x0,
  op_aconst_null = 0x1,
  op_iconst_m1 = 0x2,
  op_iconst_0 = 0x3,
  op_iconst_1 = 0x4,
  op_iconst_2 = 0x5,
  op_iconst_3 = 0x6,
  op_iconst_4 = 0x7,
  op_iconst_5 = 0x8,
  op_fconst_0 = 0xb,
  op_fconst_1 = 0xc,
  op_fconst_2 = 0xd,
  op_dconst_0 = 0xe,
  op_dconst_1 = 0xf,
  op_sipush = 0x11,
  op_ldc = 0x12,
  op_ldc_w = 0x13,
  op_ldc2_w = 0x14,
  op_dastore = 0x52,
  op_aastore = 0x53,
  op_dup = 0x59,
  op_return = 0xb1,
  op_putstatic = 0xb3,
  op_invokestatic = 0xb8,
  op_newarray = 0xbc,
  op_anewarray = 0xbd,
} opcode;

typedef enum array_type {
  array_type_boolean = 0x4,
  array_type_char = 0x5,
  array_type_float = 0x6,
  array_type_double = 0x7,
  array_type_byte = 0x8,
  array_type_short = 0x9,
  array_type_int = 0xa,
} array_type;

extern const char *string_class_name;

static const char boolean_descriptor = 'Z';
static const char int_descriptor = 'I';
static const char float_descriptor = 'F';
static const char double_descriptor = 'D';
static const char ref_descriptor = 'L';
static const char array_descriptor = '[';

float convert_float(u4 bytes);
double convert_double(u4 high_bytes, u4 low_bytes);
utf8_info *find_in_ref_info(cp_info *pool, ref_info *ref);
code_attribute *find_code_attribute(method_info *method, cp_info *pool);
method_info *find_clinit(class_file *c_file);

bool interpret_code(code_attribute *code_attr, object *obj, cp_info *pool);
