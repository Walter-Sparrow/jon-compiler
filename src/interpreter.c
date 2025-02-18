#include "interpreter.h"
#include <math.h>
#include <string.h>

const char *string_class_name = "java/lang/String";

float convert_float(u4 bytes) {
  if (bytes == 0x7f800000)
    return INFINITY;
  if (bytes == 0xff800000)
    return -INFINITY;
  if ((bytes >= 0x7f800001 && bytes <= 0x7fffffff) ||
      (bytes >= 0xff800001 && bytes <= 0xffffffff))
    return NAN;

  int s = ((bytes >> 31) == 0) ? 1 : -1;
  int e = ((bytes >> 23) & 0xff);
  u4 m = (e == 0) ? (bytes & 0x7fffff) << 1 : (bytes & 0x7fffff) | 0x800000;

  return s * m * pow(2, e - 150);
}

double convert_double(u4 high_bytes, u4 low_bytes) {
  u8 bits = ((u8)high_bytes << 32) | low_bytes;

  if (bits == 0x7ff0000000000000)
    return INFINITY;
  if (bits == 0xfff0000000000000)
    return -INFINITY;
  if ((bits >= 0x7ff0000000000001 && bits <= 0x7fffffffffffffff) ||
      (bits >= 0xfff0000000000001 && bits <= 0xffffffffffffffff))
    return NAN;

  int s = ((bits >> 63) == 0) ? 1 : -1;
  int e = (int)((bits >> 52) & 0x7ff);
  u8 m = (bits & 0xfffffffffffff);

  if (e == 0)
    m <<= 1;
  else
    m |= 0x10000000000000;

  return s * m * pow(2, e - 1075);
}

utf8_info *find_in_ref_info(cp_info *pool, ref_info *ref) {
  name_and_type_info *name_and_type =
      (name_and_type_info *)pool[ref->name_and_type_index - 1].info;

  return (utf8_info *)pool[name_and_type->name_index - 1].info;
}

method_info *find_clinit(class_file *c_file) {
  for (int i = 0; i < c_file->methods_count; i++) {
    method_info *method = &c_file->methods[i];
    utf8_info *name =
        (utf8_info *)c_file->constant_pool[method->name_index - 1].info;
    if (strcmp((char *)name->bytes, "<clinit>") == 0) {
      return method;
    }
  }

  return NULL;
}

code_attribute *find_code_attribute(method_info *method, cp_info *pool) {
  for (int i = 0; i < method->attributes_count; i++) {
    attribute_info *attribute = &method->attributes[i];
    utf8_info *name =
        (utf8_info *)pool[attribute->attribute_name_index - 1].info;
    if (strcmp((char *)name->bytes, "Code") == 0) {
      return (code_attribute *)attribute->info;
    }
  }

  return NULL;
}

static instruction_state iconst_m1(interpreter_state *state) {
  stack_push_int(&state->s, -1);
  return instruction_state_continue;
}

static instruction_state iconst_0(interpreter_state *state) {
  stack_push_int(&state->s, 0);
  return instruction_state_continue;
}

static instruction_state iconst_1(interpreter_state *state) {
  stack_push_int(&state->s, 1);
  return instruction_state_continue;
}

static instruction_state iconst_2(interpreter_state *state) {
  stack_push_int(&state->s, 2);
  return instruction_state_continue;
}

static instruction_state iconst_3(interpreter_state *state) {
  stack_push_int(&state->s, 3);
  return instruction_state_continue;
}

static instruction_state iconst_4(interpreter_state *state) {
  stack_push_int(&state->s, 4);
  return instruction_state_continue;
}

static instruction_state iconst_5(interpreter_state *state) {
  stack_push_int(&state->s, 5);
  return instruction_state_continue;
}

static instruction_state fconst_0(interpreter_state *state) {
  stack_push_float(&state->s, 0.0f);
  return instruction_state_continue;
}

static instruction_state fconst_1(interpreter_state *state) {
  stack_push_float(&state->s, 1.0f);
  return instruction_state_continue;
}

static instruction_state fconst_2(interpreter_state *state) {
  stack_push_float(&state->s, 2.0f);
  return instruction_state_continue;
}

static instruction_state dconst_0(interpreter_state *state) {
  stack_push_double(&state->s, 0.0);
  return instruction_state_continue;
}

static instruction_state dconst_1(interpreter_state *state) {
  stack_push_double(&state->s, 1.0);
  return instruction_state_continue;
}

static instruction_state sipush(interpreter_state *state) {
  u2 value = (state->code[state->pc + 1] << 8) | state->code[state->pc + 2];
  state->pc += 2;

  stack_push_int(&state->s, value);
  return instruction_state_continue;
}

static instruction_state ldc(interpreter_state *state) {
  u1 index = state->code[state->pc + 1];
  state->pc += 1;

  cp_info *pool_entry = &state->pool[index - 1];
  switch (pool_entry->tag) {
  case CONSTANT_Integer: {
    integer_info *i_info = (integer_info *)pool_entry->info;
    stack_push_int(&state->s, (int)i_info->bytes);
  } break;
  case CONSTANT_Float: {
    float_info *f_info = (float_info *)pool_entry->info;
    stack_push_float(&state->s, convert_float(f_info->bytes));
  } break;
  case CONSTANT_String: {
    string_info *s_info = (string_info *)pool_entry->info;
    utf8_info *utf8 = (utf8_info *)state->pool[s_info->string_index - 1].info;
    stack_push_string(&state->s, (char *)utf8->bytes);
  } break;
  default:
    printf("unknown ldc type: %d\n", pool_entry->tag);
    return instruction_state_error;
  }

  return instruction_state_continue;
}

static instruction_state ldc2_w(interpreter_state *state) {
  u2 index = (state->code[state->pc + 1] << 8) | state->code[state->pc + 2];
  state->pc += 2;

  cp_info *pool_entry = &state->pool[index - 1];
  switch (pool_entry->tag) {
  case CONSTANT_Double: {
    double_info *d_info = (double_info *)pool_entry->info;
    stack_push_double(&state->s,
                      convert_double(d_info->high_bytes, d_info->low_bytes));
  } break;
  default:
    printf("unknown ldc2_w type: %d\n", pool_entry->tag);
    return instruction_state_error;
  }

  return instruction_state_continue;
}

static instruction_state dastore(interpreter_state *state) {
  value val;
  stack_pop(&state->s, &val);

  value index_val;
  stack_pop(&state->s, &index_val);

  value array_val;
  stack_pop(&state->s, &array_val);

  array_val.as.array_value->elements[index_val.as.int_value] = val;
  printf("dastore: %f\n", val.as.double_value);

  return instruction_state_continue;
}

static instruction_state aastore(interpreter_state *state) {
  value val;
  stack_pop(&state->s, &val);

  value index_val;
  stack_pop(&state->s, &index_val);

  value array_val;
  stack_pop(&state->s, &array_val);

  array_val.as.array_value->elements[index_val.as.int_value] = val;
  printf("aastore\n");

  return instruction_state_continue;
}

static instruction_state dup(interpreter_state *state) {
  value val;
  stack_peek(&state->s, &val);
  stack_push(&state->s, val);
  return instruction_state_continue;
}

static instruction_state return_op(interpreter_state * /*state*/) {
  printf("return\n");
  return instruction_state_return;
}

static instruction_state putstatic(interpreter_state *state) {
  u2 index = (state->code[state->pc + 1] << 8) | state->code[state->pc + 2];
  state->pc += 2;

  cp_info *pool_entry = &state->pool[index - 1];
  ref_info *ref = (ref_info *)pool_entry->info;

  utf8_info *name = find_in_ref_info(state->pool, ref);
  value val;
  stack_pop(&state->s, &val);

  char *key = (char *)name->bytes;
  if (!object_set_value(state->obj, key, val)) {
    perror("could not set value");
    return instruction_state_error;
  }

  printf("putstatic: %s\n", name->bytes);
  return instruction_state_continue;
}

static instruction_state newarray(interpreter_state *state) {
  state->pc += 1; // ignore atype

  value val;
  stack_pop(&state->s, &val);
  int count = val.as.int_value;

  array *a = array_new(count);
  stack_push_array(&state->s, a);
  printf("newarray: length: %lld\n", a->length);

  return instruction_state_continue;
}

static instruction_state anewarray(interpreter_state *state) {
  u2 index = (state->code[state->pc + 1] << 8) | state->code[state->pc + 2];
  state->pc += 2;

  cp_info *class_entry = &state->pool[index - 1];
  class_info *c_info = (class_info *)class_entry->info;

  cp_info *name_entry = &state->pool[c_info->name_index - 1];
  utf8_info *name = (utf8_info *)name_entry->info;

  if (strcmp((char *)name->bytes, string_class_name) == 0) {
    value val;
    stack_pop(&state->s, &val);
    int count = val.as.int_value;

    array *a = array_new(count);
    stack_push_array(&state->s, a);
    printf("anewarray: length: %lld\n", a->length);
  } else {
    perror("unknown class type");
    return instruction_state_error;
  }

  return instruction_state_continue;
}

static instruction instructions[] = {
    [op_iconst_m1] = iconst_m1, [op_iconst_0] = iconst_0,
    [op_iconst_1] = iconst_1,   [op_iconst_2] = iconst_2,
    [op_iconst_3] = iconst_3,   [op_iconst_4] = iconst_4,
    [op_iconst_5] = iconst_5,   [op_fconst_0] = fconst_0,
    [op_fconst_1] = fconst_1,   [op_fconst_2] = fconst_2,
    [op_dconst_0] = dconst_0,   [op_dconst_1] = dconst_1,
    [op_sipush] = sipush,       [op_ldc] = ldc,
    [op_ldc2_w] = ldc2_w,       [op_dastore] = dastore,
    [op_aastore] = aastore,     [op_dup] = dup,
    [op_return] = return_op,    [op_putstatic] = putstatic,
    [op_newarray] = newarray,   [op_anewarray] = anewarray,
};

bool interpret_code(code_attribute *code_attr, object *obj, cp_info *pool) {
  interpreter_state state = {.code = code_attr->code,
                             .pc = 0,
                             .code_length = code_attr->code_length,
                             .obj = obj,
                             .pool = pool};

  stack_init(&state.s, code_attr->max_stack);

  while (state.pc < state.code_length) {
    u1 opcode = state.code[state.pc];
    if (instructions[opcode] == NULL) {
      printf("unknown opcode: %d\n", opcode);
      return false;
    }

    instruction_state i_state = instructions[opcode](&state);
    if (i_state == instruction_state_error) {
      return false;
    }

    if (i_state == instruction_state_return) {
      break;
    }

    state.pc++;
  }

  return true;
}
