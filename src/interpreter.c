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

bool interpret_code(code_attribute *code_attr, object *obj, cp_info *pool) {
  stack s;
  stack_init(&s, code_attr->max_stack);
  for (u4 i = 0; i < code_attr->code_length; i++) {
    u1 opcode = code_attr->code[i];
    switch (opcode) {
    case op_iconst_m1: {
      stack_push_int(&s, -1);
    } break;
    case op_iconst_0: {
      stack_push_int(&s, 0);
    } break;
    case op_iconst_1: {
      stack_push_int(&s, 1);
    } break;
    case op_iconst_2: {
      stack_push_int(&s, 2);
    } break;
    case op_iconst_3: {
      stack_push_int(&s, 3);
    } break;
    case op_iconst_4: {
      stack_push_int(&s, 4);
    } break;
    case op_iconst_5: {
      stack_push_int(&s, 5);
    } break;
    case op_fconst_0: {
      stack_push_float(&s, 0.0f);
    } break;
    case op_fconst_1: {
      stack_push_float(&s, 1.0f);
    } break;
    case op_fconst_2: {
      stack_push_float(&s, 2.0f);
    } break;
    case op_dconst_0: {
      stack_push_double(&s, 0.0);
    } break;
    case op_dconst_1: {
      stack_push_double(&s, 1.0);
    } break;
    case op_sipush: {
      u2 value = (code_attr->code[i + 1] << 8) | code_attr->code[i + 2];
      i += 2;

      stack_push_int(&s, value);
    } break;
    case op_newarray: {
      i++; // ignore atype

      value val;
      stack_pop(&s, &val);
      int count = val.as.int_value;

      array *a = array_new(count);
      stack_push_array(&s, a);
      printf("newarray: length: %lld\n", a->length);
    } break;
    case op_anewarray: {
      value val;
      stack_pop(&s, &val);
      int count = val.as.int_value;

      u2 index = (code_attr->code[i + 1] << 8) | code_attr->code[i + 2];
      i += 2;

      cp_info *class_entry = &pool[index - 1];
      class_info *c_info = (class_info *)class_entry->info;

      cp_info *name_entry = &pool[c_info->name_index - 1];
      utf8_info *name = (utf8_info *)name_entry->info;

      if (strcmp((char *)name->bytes, string_class_name) == 0) {
        array *a = array_new(count);
        stack_push_array(&s, a);
        printf("anewarray: with length: %lld\n", a->length);
      } else {
        perror("unknown class type");
        return -1;
      }
    } break;
    case op_ldc: {
      u1 index = code_attr->code[++i];
      cp_info *pool_entry = &pool[index - 1];
      switch (pool_entry->tag) {
      case CONSTANT_Integer: {
        integer_info *i_info = (integer_info *)pool_entry->info;
        stack_push_int(&s, (int)i_info->bytes);
      } break;
      case CONSTANT_Float: {
        float_info *f_info = (float_info *)pool_entry->info;
        stack_push_float(&s, convert_float(f_info->bytes));
      } break;
      case CONSTANT_String: {
        string_info *s_info = (string_info *)pool_entry->info;
        utf8_info *utf8 = (utf8_info *)pool[s_info->string_index - 1].info;
        stack_push_string(&s, (char *)utf8->bytes);
      } break;
      default:
        printf("unknown ldc type: %d\n", pool_entry->tag);
        return -1;
      }
    } break;
    case op_ldc2_w: {
      u2 index = (code_attr->code[i + 1] << 8) | code_attr->code[i + 2];
      i += 2;

      cp_info *pool_entry = &pool[index - 1];
      printf("ldc2_w index: %d, tag: %d\n", index, pool_entry->tag);
      switch (pool_entry->tag) {
      case CONSTANT_Double: {
        double_info *d_info = (double_info *)pool_entry->info;
        printf("double tag: %d\n", d_info->tag);
        printf("high bytes: %x, low bytes: %x\n", d_info->high_bytes,
               d_info->low_bytes);

        double d = convert_double(d_info->high_bytes, d_info->low_bytes);
        stack_push_double(&s, d);
      } break;
      default:
        printf("unknown ldc2_w type: %d\n", pool_entry->tag);
        return -1;
      }
    } break;
    case op_dastore: {
      value val;
      stack_pop(&s, &val);

      value index_val;
      stack_pop(&s, &index_val);

      value array_val;
      stack_pop(&s, &array_val);

      array_val.as.array_value->elements[index_val.as.int_value] = val;
      printf("dastore: %f\n", val.as.double_value);
    } break;
    case op_aastore: {
      value val;
      stack_pop(&s, &val);

      value index_val;
      stack_pop(&s, &index_val);

      value array_val;
      stack_pop(&s, &array_val);

      array_val.as.array_value->elements[index_val.as.int_value] = val;
      printf("aastore\n");
    } break;
    case op_dup: {
      value val;
      stack_peek(&s, &val);
      stack_push(&s, val);
    } break;
    case op_invokestatic: {
      u2 index = (code_attr->code[i + 1] << 8) | code_attr->code[i + 2];
      i += 2;

      utf8_info *name = (utf8_info *)pool[index - 1].info;
      printf("invokestatic: %s\n", name->bytes);
    } break;
    case op_putstatic: {
      u2 index = (code_attr->code[i + 1] << 8) | code_attr->code[i + 2];
      i += 2;

      utf8_info *name =
          find_in_ref_info(pool, (ref_info *)pool[index - 1].info);

      value val;
      stack_pop(&s, &val);

      char *key = (char *)name->bytes;
      if (!object_set_value(obj, key, val)) {
        perror("could not set value");
        return false;
      }

      printf("putstatic: %s\n", name->bytes);
    } break;

    case op_return: {
      printf("return\n");
      return true;
    } break;

    default:
      printf("unknown opcode: %x\n", opcode);
      return false;
    }
  }

  return true;
}
