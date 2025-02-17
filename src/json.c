#include "json.h"

void value_to_json(FILE *file, value val) {
  switch (val.type) {
  case VALUE_NULL:
    fprintf(file, "null");
    break;
  case VALUE_BOOLEAN:
    fprintf(file, "%s", val.as.boolean_value ? "true" : "false");
    break;
  case VALUE_INT:
    fprintf(file, "%d", val.as.int_value);
    break;
  case VALUE_FLOAT:
    fprintf(file, "%f", val.as.float_value);
    break;
  case VALUE_DOUBLE:
    fprintf(file, "%f", val.as.double_value);
    break;
  case VALUE_STRING:
    fprintf(file, "\"%s\"", val.as.string_value);
    break;
  case VALUE_ARRAY:
    array_to_json(file, val.as.array_value);
    break;
  case VALUE_OBJECT:
    object_to_json(file, val.as.object_value);
    break;
  }
}

void array_to_json(FILE *file, array *arr) {
  printf("length: %lld\n", arr->length);
  fprintf(file, "[");
  for (size_t i = 0; i < arr->length; i++) {
    value_to_json(file, arr->elements[i]);
    if (i < arr->length - 1) {
      fprintf(file, ", ");
    }
  }
  fprintf(file, "]");
}

void object_to_json(FILE *file, object *obj) {
  fprintf(file, "{");
  for (size_t i = 0; i < obj->size; i++) {
    fprintf(file, "\"%s\": ", obj->keys[i]);
    value_to_json(file, obj->values[i]);
    if (i < obj->size - 1) {
      fprintf(file, ", ");
    }
  }
  fprintf(file, "}");
}
