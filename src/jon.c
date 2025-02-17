#include "jon.h"
#include <string.h>

void fill_jon_value_type(jon_value *value, utf8_info *descriptor) {
  if (descriptor->bytes[0] == boolean_descriptor) {
    value->type = jon_value_type_boolean;
  } else if (descriptor->bytes[0] == int_descriptor) {
    value->type = jon_value_type_int;
  } else if (descriptor->bytes[0] == float_descriptor) {
    value->type = jon_value_type_float;
  } else if (descriptor->bytes[0] == double_descriptor) {
    value->type = jon_value_type_float;
  } else if (descriptor->bytes[0] == ref_descriptor &&
             strcmp((char *)descriptor->bytes + 1, string_class_name) == 0) {
    value->type = jon_value_type_string;
  } else if (descriptor->bytes[0] == array_descriptor) {
    value->type = jon_value_type_array;
  } else {
    value->type = jon_value_type_object;
  }
  printf("type: %s\n", descriptor->bytes);
}

int jon_object_set_value(jon_value_pair *object, u2 object_size, char *key,
                         jon_value value) {
  for (int i = 0; i < object_size; i++) {
    if (strcmp(object[i].key, key) == 0) {
      object[i].value = value;
      return 0;
    }
  }

  printf("could not find key: %s\n", key);
  return -1;
}

int stack_value_to_jon_value(type_tag tag, stack_value s_value,
                             jon_value *value) {
  switch (tag) {
  case type_boolean: {
    value->type = jon_value_type_boolean;
    value->value.boolean_value = s_value.boolean_value;
  } break;
  case type_int: {
    value->type = jon_value_type_int;
    value->value.int_value = s_value.int_value;
  } break;
  case type_float: {
    value->type = jon_value_type_float;
    value->value.float_value = s_value.float_value;
  } break;
  case type_double: {
    value->type = jon_value_type_double;
    value->value.double_value = s_value.double_value;
  } break;
  case type_string: {
    value->type = jon_value_type_string;
    value->value.string_value = s_value.string_value;
  } break;
  case type_array: {
    value->type = jon_value_type_array;
    value->value.array_value.length = s_value.array_value.length;
    value->value.array_value.values =
        (jon_value *)malloc(s_value.array_value.length * sizeof(jon_value));
    for (size_t i = 0; i < s_value.array_value.length; i++) {
      if (stack_value_to_jon_value(s_value.array_value.tag,
                                   s_value.array_value.elements[i],
                                   &value->value.array_value.values[i]) < 0) {
        return -1;
      }
    }
  } break;
  default:
    printf("unknown type tag: %d\n", tag);
    return -1;
  }

  return 0;
}

int fill_jon_object(jon_value_pair *object, u2 object_size,
                    class_file *c_file) {
  for (int i = 0; i < c_file->fields_count; i++) {
    field_info *field = &c_file->fields[i];
    utf8_info *name =
        (utf8_info *)c_file->constant_pool[field->name_index - 1].info;
    object[i].key = (char *)name->bytes;

    utf8_info *descriptor =
        (utf8_info *)c_file->constant_pool[field->descriptor_index - 1].info;

    fill_jon_value_type(&object[i].value, descriptor);
  }

  method_info *clinit = find_clinit(c_file);
  if (!clinit) {
    perror("could not find clinit");
    return -1;
  }

  code_attribute *code_attr =
      find_code_attribute(clinit, c_file->constant_pool);
  if (!code_attr) {
    perror("could not find code attribute");
    return -1;
  }

  if (interpret_code(code_attr, object, object_size, c_file->constant_pool) <
      0) {
    perror("could not interpret code");
    return -1;
  }

  return 0;
}

int serialize_jon_value_as_json(jon_value_type j_type, jon_value j_value,
                                FILE *out) {
  switch (j_type) {
  case jon_value_type_boolean: {
    fprintf(out, "%s", j_value.value.boolean_value ? "true" : "false");
  } break;
  case jon_value_type_int: {
    fprintf(out, "%d", j_value.value.int_value);
  } break;
  case jon_value_type_float: {
    fprintf(out, "%f", j_value.value.float_value);
  } break;
  case jon_value_type_double: {
    fprintf(out, "%f", j_value.value.double_value);
  } break;
  case jon_value_type_string: {
    fprintf(out, "\"%s\"", j_value.value.string_value);
  } break;
  case jon_value_type_array: {
    fprintf(out, "[");
    for (int i = 0; i < j_value.value.array_value.length; i++) {
      serialize_jon_value_as_json(j_value.value.array_value.values[i].type,
                                  j_value.value.array_value.values[i], out);
      if (i < j_value.value.array_value.length - 1) {
        fprintf(out, ", ");
      }
    }
    fprintf(out, "]");
  } break;
  case jon_value_type_object: {
    // TODO(ilya): implement
    fprintf(out, "{}");
  } break;
  default:
    perror("unknown jon value type");
    return -1;
  }

  return 0;
}

int serialize_jon_object_as_json(jon_value_pair *object, u2 object_size,
                                 FILE *out) {
  fprintf(out, "{");
  for (int i = 0; i < object_size; i++) {
    fprintf(out, "\"%s\": ", object[i].key);
    if (serialize_jon_value_as_json(object[i].value.type, object[i].value,
                                    out) < 0) {
      perror("could not serialize jon value as json");
      return -1;
    }

    if (i < object_size - 1) {
      fprintf(out, ", ");
    }
  }
  fprintf(out, "}");
  return 0;
}
