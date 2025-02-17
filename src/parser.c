#include "parser.h"
#include <string.h>
#include <stdlib.h>

void read_class_info(FILE *file, class_info *info) {
  READ_U2(file, info->name_index);
}

void read_ref_info(FILE *file, ref_info *info) {
  READ_U2(file, info->class_index);
  READ_U2(file, info->name_and_type_index);
}

void read_name_and_type_info(FILE *file, name_and_type_info *info) {
  READ_U2(file, info->name_index);
  READ_U2(file, info->descriptor_index);
}

void read_utf8_info(FILE *file, utf8_info *info) {
  READ_U2(file, info->length);
  info->bytes = (u1 *)malloc(info->length * sizeof(u1));
  fread(info->bytes, info->length, 1, file);
  info->bytes[info->length] = '\0';
}

void read_string_info(FILE *file, string_info *info) {
  READ_U2(file, info->string_index);
}

void read_integer_info(FILE *file, integer_info *info) {
  READ_U4(file, info->bytes);
}

void read_float_info(FILE *file, float_info *info) {
  READ_U4(file, info->bytes);
}

void read_long_info(FILE *file, long_info *info) {
  READ_U4(file, info->high_bytes);
  READ_U4(file, info->low_bytes);
}

void read_double_info(FILE *file, double_info *info) {
  READ_U4(file, info->high_bytes);
  READ_U4(file, info->low_bytes);
}

int read_constant_pool(FILE *file, cp_info *pool, u2 count) {
  for (int i = 1; i < count; i++) {
    cp_info *pool_entry = &pool[i - 1];
    READ_U1(file, pool_entry->tag);

    printf("pool_index: %d, tag: %d ", i, pool_entry->tag);
    switch (pool_entry->tag) {
    case CONSTANT_Class: {
      class_info *c_info = (class_info *)malloc(sizeof(class_info));
      c_info->tag = pool_entry->tag;
      read_class_info(file, c_info);
      pool_entry->info = (u1 *)c_info;
      printf("found class, name index: %d\n", c_info->name_index);
    } break;

    case CONSTANT_Fieldref:
    case CONSTANT_Methodref:
    case CONSTANT_InterfaceMethodref: {
      ref_info *r_info = (ref_info *)malloc(sizeof(ref_info));
      r_info->tag = pool_entry->tag;
      read_ref_info(file, r_info);
      pool_entry->info = (u1 *)r_info;
      printf("found ref, class index: %d, name and type index: %d\n",
             r_info->class_index, r_info->name_and_type_index);
    } break;

    case CONSTANT_NameAndType: {
      name_and_type_info *n_info =
          (name_and_type_info *)malloc(sizeof(name_and_type_info));
      n_info->tag = pool_entry->tag;
      read_name_and_type_info(file, n_info);
      pool_entry->info = (u1 *)n_info;
      printf("found name and type: name index: %d, descriptor index: %d\n",
             n_info->name_index, n_info->descriptor_index);
    } break;

    case CONSTANT_Utf8: {
      utf8_info *u_info = (utf8_info *)malloc(sizeof(utf8_info));
      u_info->tag = pool_entry->tag;
      read_utf8_info(file, u_info);
      pool_entry->info = (u1 *)u_info;
      printf("found utf8: length: %d, bytes: %s\n", u_info->length,
             u_info->bytes);
    } break;

    case CONSTANT_String: {
      string_info *s_info = (string_info *)malloc(sizeof(string_info));
      read_string_info(file, s_info);
      pool_entry->info = (u1 *)s_info;
      printf("found string: string index: %d\n", s_info->string_index);
    } break;

    case CONSTANT_Integer: {
      integer_info *i_info = (integer_info *)malloc(sizeof(integer_info));
      i_info->tag = pool_entry->tag;
      read_integer_info(file, i_info);
      pool_entry->info = (u1 *)i_info;
      printf("found integer: %d\n", i_info->bytes);
    } break;

    case CONSTANT_Float: {
      float_info *f_info = (float_info *)malloc(sizeof(float_info));
      f_info->tag = pool_entry->tag;
      read_float_info(file, f_info);
      pool_entry->info = (u1 *)f_info;
      printf("found float: %x\n", f_info->bytes);
    } break;

    // https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html#jvms-4.4.5
    // CONSTANT_Long and CONSTANT_Double will skip one info entry
    case CONSTANT_Long: {
      long_info *l_info = (long_info *)malloc(sizeof(long_info));
      l_info->tag = pool_entry->tag;
      read_long_info(file, l_info);
      pool_entry->info = (u1 *)l_info;
      printf("found long: high bytes: %x, low bytes: %x\n", l_info->high_bytes,
             l_info->low_bytes);
      i++;
    } break;

    case CONSTANT_Double: {
      double_info *d_info = (double_info *)malloc(sizeof(double_info));
      d_info->tag = pool_entry->tag;
      read_double_info(file, d_info);
      pool_entry->info = (u1 *)d_info;
      printf("found double: high bytes: %x, low bytes: %x\n",
             d_info->high_bytes, d_info->low_bytes);
      i++;
    } break;

    default:
      printf("unknown tag: %d\n", pool_entry->tag);
      return -1;
    }
  }

  return 0;
}

void print_all_class_access_flags(u2 access_flags) {
  if (access_flags & CLASS_ACC_PUBLIC) {
    printf("ACC_PUBLIC ");
  }
  if (access_flags & CLASS_ACC_FINAL) {
    printf("ACC_FINAL ");
  }
  if (access_flags & CLASS_ACC_SUPER) {
    printf("ACC_SUPER ");
  }
  if (access_flags & CLASS_ACC_INTERFACE) {
    printf("ACC_INTERFACE ");
  }
  if (access_flags & CLASS_ACC_ABSTRACT) {
    printf("ACC_ABSTRACT ");
  }
  if (access_flags & CLASS_ACC_SYNTHETIC) {
    printf("ACC_SYNTHETIC ");
  }
  if (access_flags & CLASS_ACC_ANNOTATION) {
    printf("ACC_ANNOTATION ");
  }
  if (access_flags & CLASS_ACC_ENUM) {
    printf("ACC_ENUM ");
  }
}

int read_attributes(FILE *file, attribute_info *attributes, u2 count,
                    cp_info *pool);

void read_signature_attribute(FILE *file, signature_attribute *info,
                              cp_info *pool) {
  READ_U2(file, info->signature_index);
  utf8_info *utf8 = (utf8_info *)pool[info->signature_index - 1].info;
  printf("      signature: %s\n", utf8->bytes);
}

void read_element_value_pair(FILE *file, element_value_pair *info,
                             cp_info *pool) {
  READ_U2(file, info->element_name_index);
  utf8_info *element_name =
      (utf8_info *)pool[info->element_name_index - 1].info;
  printf("        element name: %s\n", element_name->bytes);
  // TODO(ilya): read element value
}

void read_annotation(FILE *file, annotation *info, cp_info *pool) {
  READ_U2(file, info->type_index);
  utf8_info *type_name = (utf8_info *)pool[info->type_index - 1].info;
  printf("      type name: %s\n", type_name->bytes);

  READ_U2(file, info->num_element_value_pairs);
  printf("      num element value pairs: %d\n", info->num_element_value_pairs);

  info->element_value_pairs = (element_value_pair *)malloc(
      info->num_element_value_pairs * sizeof(element_value_pair));
  for (int i = 0; i < info->num_element_value_pairs; i++) {
    read_element_value_pair(file, &info->element_value_pairs[i], pool);
  }
}

void read_runtime_visible_annotations_attribute(
    FILE *file, runtime_visible_annotations_attribute *info, cp_info *pool) {
  READ_U2(file, info->num_annotations);
  printf("      num annotations: %d\n", info->num_annotations);

  for (int i = 0; i < info->num_annotations; i++) {
    info->annotations =
        (annotation *)malloc(info->num_annotations * sizeof(annotation));
    read_annotation(file, info->annotations, pool);
  }
}

void read_exception_table(FILE *file, exception_table *table) {
  READ_U2(file, table->start_pc);
  READ_U2(file, table->end_pc);
  READ_U2(file, table->handler_pc);
  READ_U2(file, table->catch_type);
}

int read_code_attribute(FILE *file, code_attribute *info, cp_info *pool) {
  READ_U2(file, info->max_stack);
  printf("      max stack: %d\n", info->max_stack);

  READ_U2(file, info->max_locals);
  printf("      max locals: %d\n", info->max_locals);

  READ_U4(file, info->code_length);
  printf("      code length: %d\n", info->code_length);

  info->code = (u1 *)malloc(info->code_length * sizeof(u1));
  fread(info->code, info->code_length, 1, file);
  printf("      code: ");
  for (u4 i = 0; i < info->code_length; i++) {
    printf("%x ", info->code[i]);
  }
  printf("\n");

  READ_U2(file, info->exception_table_length);
  printf("      exception table length: %d\n", info->exception_table_length);

  info->exception_table = (exception_table *)malloc(
      info->exception_table_length * sizeof(exception_table));
  for (int i = 0; i < info->exception_table_length; i++) {
    read_exception_table(file, &info->exception_table[i]);
  }

  READ_U2(file, info->attributes_count);
  printf("      attributes count: %d\n", info->attributes_count);

  info->attributes =
      (attribute_info *)malloc(info->attributes_count * sizeof(attribute_info));
  if (read_attributes(file, info->attributes, info->attributes_count, pool) <
      0) {
    perror("could not read attributes");
    return -1;
  }

  return 0;
}

void read_line_number_table(FILE *file, line_number_table *table) {
  READ_U2(file, table->start_pc);
  READ_U2(file, table->line_number);
}

int read_line_number_table_attribute(FILE *file,
                                     line_number_table_attribute *info) {
  READ_U2(file, info->line_number_table_length);
  printf("      line number table length: %d\n",
         info->line_number_table_length);

  info->line_number_table = (line_number_table *)malloc(
      info->line_number_table_length * sizeof(line_number_table));
  for (int i = 0; i < info->line_number_table_length; i++) {
    read_line_number_table(file, &info->line_number_table[i]);
    printf("        start pc: %d, line number: %d\n",
           info->line_number_table[i].start_pc,
           info->line_number_table[i].line_number);
  }

  return 0;
}

void read_source_file_attribute(FILE *file, source_file_attribute *info,
                                cp_info *pool) {
  READ_U2(file, info->sourcefile_index);
  utf8_info *utf8 = (utf8_info *)pool[info->sourcefile_index - 1].info;
  printf("      source file: %s\n", utf8->bytes);
}

int read_attributes(FILE *file, attribute_info *attributes, u2 count,
                    cp_info *pool) {
  for (int i = 0; i < count; i++) {
    attribute_info *current_info = &attributes[i];
    printf("  attribute_index: %d\n", i);

    READ_U2(file, current_info->attribute_name_index);
    utf8_info *utf8 =
        (utf8_info *)pool[current_info->attribute_name_index - 1].info;
    printf("    attribute name: %s\n", utf8->bytes);

    READ_U4(file, current_info->attribute_length);
    printf("    attribute length: %d\n", current_info->attribute_length);

    if (strcmp((char *)utf8->bytes, "Signature") == 0) {
      signature_attribute *signature =
          (signature_attribute *)malloc(sizeof(signature_attribute));
      signature->attribute_name_index = current_info->attribute_name_index;
      signature->attribute_length = current_info->attribute_length;
      read_signature_attribute(file, signature, pool);
      current_info->info = (u1 *)signature;
      continue;
    }

    if (strcmp((char *)utf8->bytes, "RuntimeVisibleAnnotations") == 0) {
      runtime_visible_annotations_attribute *annotations =
          (runtime_visible_annotations_attribute *)malloc(
              sizeof(runtime_visible_annotations_attribute));
      annotations->attribute_name_index = current_info->attribute_name_index;
      annotations->attribute_length = current_info->attribute_length;
      read_runtime_visible_annotations_attribute(file, annotations, pool);
      current_info->info = (u1 *)annotations;
      continue;
    }

    if (strcmp((char *)utf8->bytes, "Code") == 0) {
      code_attribute *code = (code_attribute *)malloc(sizeof(code_attribute));
      code->attribute_name_index = current_info->attribute_name_index;
      code->attribute_length = current_info->attribute_length;
      if (read_code_attribute(file, code, pool) < 0) {
        perror("could not read code attribute");
        return -1;
      }
      current_info->info = (u1 *)code;
      continue;
    }

    if (strcmp((char *)utf8->bytes, "LineNumberTable") == 0) {
      line_number_table_attribute *line_number_table =
          (line_number_table_attribute *)malloc(
              sizeof(line_number_table_attribute));
      line_number_table->attribute_name_index =
          current_info->attribute_name_index;
      line_number_table->attribute_length = current_info->attribute_length;
      if (read_line_number_table_attribute(file, line_number_table) < 0) {
        perror("could not read line number table attribute");
        return -1;
      }
      current_info->info = (u1 *)line_number_table;
      continue;
    }

    if (strcmp((char *)utf8->bytes, "SourceFile") == 0) {
      source_file_attribute *source_file =
          (source_file_attribute *)malloc(sizeof(source_file_attribute));
      source_file->attribute_name_index = current_info->attribute_name_index;
      source_file->attribute_length = current_info->attribute_length;
      read_source_file_attribute(file, source_file, pool);
      current_info->info = (u1 *)source_file;
      continue;
    }

    printf("    unknown attribute: %s\n", utf8->bytes);
  }

  return 0;
}

void print_all_field_access_flags(u2 access_flags) {
  if (access_flags & FIELD_ACC_PUBLIC) {
    printf("ACC_PUBLIC ");
  }
  if (access_flags & FIELD_ACC_PRIVATE) {
    printf("ACC_PRIVATE ");
  }
  if (access_flags & FIELD_ACC_PROTECTED) {
    printf("ACC_PROTECTED ");
  }
  if (access_flags & FIELD_ACC_STATIC) {
    printf("ACC_STATIC ");
  }
  if (access_flags & FIELD_ACC_FINAL) {
    printf("ACC_FINAL ");
  }
  if (access_flags & FIELD_ACC_VOLATILE) {
    printf("ACC_VOLATILE ");
  }
  if (access_flags & FIELD_ACC_TRANSIENT) {
    printf("ACC_TRANSIENT ");
  }
  if (access_flags & FIELD_ACC_SYNTHETIC) {
    printf("ACC_SYNTHETIC ");
  }
  if (access_flags & FIELD_ACC_ENUM) {
    printf("ACC_ENUM ");
  }
}

int parse_fields(FILE *file, field_info *fields, u2 count, cp_info *pool) {
  for (int i = 0; i < count; i++) {
    field_info *field = &fields[i];
    printf("field_index: %d\n", i);

    READ_U2(file, field->access_flags);
    printf("  access flags: ");
    print_all_field_access_flags(field->access_flags);
    printf("\n");

    READ_U2(file, field->name_index);
    utf8_info *name = (utf8_info *)pool[field->name_index - 1].info;
    printf("  name: %s\n", name->bytes);

    READ_U2(file, field->descriptor_index);
    utf8_info *descriptor = (utf8_info *)pool[field->descriptor_index - 1].info;
    printf("  descriptor: %s\n", descriptor->bytes);

    READ_U2(file, field->attributes_count);
    printf("  attributes count: %d\n", field->attributes_count);

    field->attributes = (attribute_info *)malloc(field->attributes_count *
                                                 sizeof(attribute_info));
    if (read_attributes(file, field->attributes, field->attributes_count,
                        pool) < 0) {
      perror("could not read attributes");
      return -1;
    }
  }

  return 0;
}

void print_all_method_access_flags(u2 access_flags) {
  if (access_flags & METHOD_ACC_PUBLIC) {
    printf("ACC_PUBLIC ");
  }
  if (access_flags & METHOD_ACC_PRIVATE) {
    printf("ACC_PRIVATE ");
  }
  if (access_flags & METHOD_ACC_PROTECTED) {
    printf("ACC_PROTECTED ");
  }
  if (access_flags & METHOD_ACC_STATIC) {
    printf("ACC_STATIC ");
  }
  if (access_flags & METHOD_ACC_FINAL) {
    printf("ACC_FINAL ");
  }
  if (access_flags & METHOD_ACC_SYNCHRONIZED) {
    printf("ACC_SYNCHRONIZED ");
  }
  if (access_flags & METHOD_ACC_BRIDGE) {
    printf("ACC_BRIDGE ");
  }
  if (access_flags & METHOD_ACC_VARARGS) {
    printf("ACC_VARARGS ");
  }
  if (access_flags & METHOD_ACC_NATIVE) {
    printf("ACC_NATIVE ");
  }
  if (access_flags & METHOD_ACC_ABSTRACT) {
    printf("ACC_ABSTRACT ");
  }
  if (access_flags & METHOD_ACC_STRICT) {
    printf("ACC_STRICT ");
  }
  if (access_flags & METHOD_ACC_SYNTHETIC) {
    printf("ACC_SYNTHETIC ");
  }
}

int read_methods(FILE *file, method_info *methods, u2 count, cp_info *pool) {
  for (int i = 0; i < count; i++) {
    method_info *method = &methods[i];
    printf("method_index: %d\n", i);

    READ_U2(file, method->access_flags);
    printf("  access flags: ");
    print_all_method_access_flags(method->access_flags);
    printf("\n");

    READ_U2(file, method->name_index);
    utf8_info *name = (utf8_info *)pool[method->name_index - 1].info;
    printf("  name: %s\n", name->bytes);

    READ_U2(file, method->descriptor_index);
    utf8_info *descriptor =
        (utf8_info *)pool[method->descriptor_index - 1].info;
    printf("  descriptor: %s\n", descriptor->bytes);

    READ_U2(file, method->attributes_count);
    printf("  attributes count: %d\n", method->attributes_count);

    method->attributes = (attribute_info *)malloc(method->attributes_count *
                                                  sizeof(attribute_info));
    if (read_attributes(file, method->attributes, method->attributes_count,
                        pool) < 0) {
      perror("could not read attributes");
      return -1;
    }
  }

  return 0;
}

int parse_file(char *path, class_file *c_file) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    perror("could not open the file");
    return -1;
  }

  u4 magic;
  READ_U4(file, magic);
  printf("magic: %x\n", magic);
  if (magic != 0xcafebabe) {
    perror("invalid magic");
    return -1;
  }

  u2 minor_version, major_version;
  READ_U2(file, minor_version);
  READ_U2(file, major_version);
  printf("version: %d.%d\n", major_version, minor_version);

  u2 constant_pool_count;
  READ_U2(file, constant_pool_count);
  printf("constant pool count: %d\n", constant_pool_count);

  cp_info *constant_pool =
      (cp_info *)malloc((constant_pool_count - 1) * sizeof(cp_info));
  if (read_constant_pool(file, constant_pool, constant_pool_count) < 0) {
    perror("could not read constant pool");
    return -1;
  }

  u2 access_flags;
  READ_U2(file, access_flags);
  printf("access flags: value: %d, ", access_flags);
  print_all_class_access_flags(access_flags);
  printf("\n");

  u2 this_class, super_class;
  READ_U2(file, this_class);
  READ_U2(file, super_class);
  class_info *this_class_info =
      (class_info *)constant_pool[this_class - 1].info;
  class_info *super_class_info =
      (class_info *)constant_pool[super_class - 1].info;
  utf8_info *class_name =
      (utf8_info *)constant_pool[this_class_info->name_index - 1].info;
  utf8_info *super_class_name =
      (utf8_info *)constant_pool[super_class_info->name_index - 1].info;
  printf("class name: %s, super class name: %s\n", class_name->bytes,
         super_class_name->bytes);

  u2 interfaces_count;
  READ_U2(file, interfaces_count);
  printf("interfaces count: %d\n", interfaces_count);

  u2 *interfaces = (u2 *)malloc(interfaces_count * sizeof(u2));
  for (int i = 0; i < interfaces_count; i++) {
    READ_U2(file, interfaces[i]);
    printf("interface %d: %d\n", i, interfaces[i]);
  }

  u2 fields_count;
  READ_U2(file, fields_count);
  printf("fields count: %d\n", fields_count);

  field_info *fields = (field_info *)malloc(fields_count * sizeof(field_info));
  if (parse_fields(file, fields, fields_count, constant_pool) < 0) {
    perror("could not read fields");
    return -1;
  }

  u2 methods_count;
  READ_U2(file, methods_count);
  printf("methods count: %d\n", methods_count);

  method_info *methods =
      (method_info *)malloc(methods_count * sizeof(method_info));
  if (read_methods(file, methods, methods_count, constant_pool) < 0) {
    perror("could not read methods");
    return -1;
  }

  u2 attributes_count;
  READ_U2(file, attributes_count);
  printf("attributes count: %d\n", attributes_count);

  attribute_info *attributes =
      (attribute_info *)malloc(attributes_count * sizeof(attribute_info));
  if (read_attributes(file, attributes, attributes_count, constant_pool) < 0) {
    perror("could not read attributes");
    return -1;
  }

  c_file->magic = magic;
  c_file->minor_version = minor_version;
  c_file->major_version = major_version;
  c_file->constant_pool_count = constant_pool_count;
  c_file->constant_pool = constant_pool;
  c_file->access_flags = access_flags;
  c_file->this_class = this_class;
  c_file->super_class = super_class;
  c_file->interfaces_count = interfaces_count;
  c_file->interfaces = interfaces;
  c_file->fields_count = fields_count;
  c_file->fields = fields;
  c_file->methods_count = methods_count;
  c_file->methods = methods;
  c_file->attributes_count = attributes_count;
  c_file->attributes = attributes;

  return 0;
}
