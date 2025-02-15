#include <windows.h>
#include <direct.h>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include "stack.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef int32_t i4;

#define READ_U1(file, name) fread(&name, sizeof(name), 1, file);

#define READ_U2(file, name)            \
  fread(&name, sizeof(name), 1, file); \
  name = ntohs(name);

#define READ_U4(file, name)            \
  fread(&name, sizeof(name), 1, file); \
  name = ntohl(name);

typedef enum constant_tag {
  CONSTANT_Class = 7,
  CONSTANT_Fieldref = 9,
  CONSTANT_Methodref = 10,
  CONSTANT_InterfaceMethodref = 11,
  CONSTANT_String = 8,
  CONSTANT_Integer = 3,
  CONSTANT_Float = 4,
  CONSTANT_Long = 5,
  CONSTANT_Double = 6,
  CONSTANT_NameAndType = 12,
  CONSTANT_Utf8 = 1,
  CONSTANT_MethodHandle = 15,
  CONSTANT_MethodType = 16,
  CONSTANT_InvokeDynamic = 18,
} constant_tag;

typedef struct cp_info {
  u1 tag;
  u1 *info;
} cp_info;

typedef struct class_info {
  u1 tag;
  u2 name_index;
} class_info;

void read_class_info(FILE *file, class_info *info) {
  READ_U2(file, info->name_index);
}

typedef struct ref_info {
  u1 tag;
  u2 class_index;
  u2 name_and_type_index;
} ref_info;

void read_ref_info(FILE *file, ref_info *info) {
  READ_U2(file, info->class_index);
  READ_U2(file, info->name_and_type_index);
}

typedef struct name_and_type_info {
  u1 tag;
  u2 name_index;
  u2 descriptor_index;
} name_and_type_info;

void read_name_and_type_info(FILE *file, name_and_type_info *info) {
  READ_U2(file, info->name_index);
  READ_U2(file, info->descriptor_index);
}

typedef struct utf8_info {
  u1 tag;
  u2 length;
  u1 *bytes;
} utf8_info;

void read_utf8_info(FILE *file, utf8_info *info) {
  READ_U2(file, info->length);
  info->bytes = (u1 *)malloc(info->length * sizeof(u1));
  fread(info->bytes, info->length, 1, file);
  info->bytes[info->length] = '\0';
}

typedef struct string_info {
  u1 tag;
  u2 string_index;
} string_info;

void read_string_info(FILE *file, string_info *info) {
  READ_U2(file, info->string_index);
}

typedef struct integer_info {
  u1 tag;
  u4 bytes;
} integer_info;

void read_integer_info(FILE *file, integer_info *info) {
  READ_U4(file, info->bytes);
}

typedef struct float_info {
  u1 tag;
  u4 bytes;
} float_info;

void read_float_info(FILE *file, float_info *info) {
  READ_U4(file, info->bytes);
}

typedef struct long_info {
  u1 tag;
  u4 high_bytes;
  u4 low_bytes;
} long_info;

void read_long_info(FILE *file, long_info *info) {
  READ_U4(file, info->high_bytes);
  READ_U4(file, info->low_bytes);
}

typedef struct double_info {
  u1 tag;
  u4 high_bytes;
  u4 low_bytes;
} double_info;

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
        printf("found long: high bytes: %x, low bytes: %x\n",
               l_info->high_bytes, l_info->low_bytes);
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

typedef enum class_access_flags : u2 {
  CLASS_ACC_PUBLIC = 0x0001,
  CLASS_ACC_FINAL = 0x0010,
  CLASS_ACC_SUPER = 0x0020,
  CLASS_ACC_INTERFACE = 0x0200,
  CLASS_ACC_ABSTRACT = 0x0400,
  CLASS_ACC_SYNTHETIC = 0x1000,
  CLASS_ACC_ANNOTATION = 0x2000,
  CLASS_ACC_ENUM = 0x4000,
} class_access_flags;

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

typedef struct attribute_info {
  u2 attribute_name_index;
  u4 attribute_length;
  u1 *info;
} attribute_info;

int read_attributes(FILE *file, attribute_info *attributes, u2 count,
                    cp_info *pool);

typedef struct signature_attribute {
  u2 attribute_name_index;
  u4 attribute_length;
  u2 signature_index;
} signature_attribute;

void read_signature_attribute(FILE *file, signature_attribute *info,
                              cp_info *pool) {
  READ_U2(file, info->signature_index);
  utf8_info *utf8 = (utf8_info *)pool[info->signature_index - 1].info;
  printf("      signature: %s\n", utf8->bytes);
}

typedef struct enum_const_value {
  u2 type_name_index;
  u2 const_name_index;
} const_value_index;

typedef struct annotation annotation;
typedef struct element_value element_value;

typedef struct array_value {
  u2 num_values;
  element_value *values;
} array_value;

typedef struct element_value {
  u1 tag;
  union {
    u2 const_value_index;
    enum_const_value enum_const_value;
    u2 class_info_index;
    struct annotation *annotation_value;
    array_value array_value;
  } value;
} element_value;

typedef struct element_value_pair {
  u2 element_name_index;
  element_value value;
} element_value_pair;

void read_element_value_pair(FILE *file, element_value_pair *info,
                             cp_info *pool) {
  READ_U2(file, info->element_name_index);
  utf8_info *element_name =
      (utf8_info *)pool[info->element_name_index - 1].info;
  printf("        element name: %s\n", element_name->bytes);
  // TODO(ilya): read element value
}

typedef struct annotation {
  u2 type_index;
  u2 num_element_value_pairs;
  element_value_pair *element_value_pairs;
} annotation;

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

typedef struct runtime_visible_annotations_attribute {
  u2 attribute_name_index;
  u4 attribute_length;
  u2 num_annotations;
  annotation *annotations;
} runtime_visible_annotations_attribute;

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

typedef struct exception_table {
  u2 start_pc;
  u2 end_pc;
  u2 handler_pc;
  u2 catch_type;
} exception_table;

void read_exception_table(FILE *file, exception_table *table) {
  READ_U2(file, table->start_pc);
  READ_U2(file, table->end_pc);
  READ_U2(file, table->handler_pc);
  READ_U2(file, table->catch_type);
}

typedef struct code_attribute {
  u2 attribute_name_index;
  u4 attribute_length;
  u2 max_stack;
  u2 max_locals;
  u4 code_length;
  u1 *code;
  u2 exception_table_length;
  exception_table *exception_table;
  u2 attributes_count;
  attribute_info *attributes;
} code_attribute;

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
  for (int i = 0; i < info->code_length; i++) {
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

typedef struct line_number_table {
  u2 start_pc;
  u2 line_number;
} line_number_table;

void read_line_number_table(FILE *file, line_number_table *table) {
  READ_U2(file, table->start_pc);
  READ_U2(file, table->line_number);
}

typedef struct line_number_table_attribute {
  u2 attribute_name_index;
  u4 attribute_length;
  u2 line_number_table_length;
  line_number_table *line_number_table;
} line_number_table_attribute;

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

typedef struct source_file_attribute {
  u2 attribute_name_index;
  u4 attribute_length;
  u2 sourcefile_index;
} source_file_attribute;

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

typedef enum field_access_flags : u2 {
  FIELD_ACC_PUBLIC = 0x0001,
  FIELD_ACC_PRIVATE = 0x0002,
  FIELD_ACC_PROTECTED = 0x0004,
  FIELD_ACC_STATIC = 0x0008,
  FIELD_ACC_FINAL = 0x0010,
  FIELD_ACC_VOLATILE = 0x0040,
  FIELD_ACC_TRANSIENT = 0x0080,
  FIELD_ACC_SYNTHETIC = 0x1000,
  FIELD_ACC_ENUM = 0x4000,
} field_access_flags;

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

typedef struct field_info {
  u2 access_flags;
  u2 name_index;
  u2 descriptor_index;
  u2 attributes_count;
  attribute_info *attributes;
} field_info;

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

typedef enum method_access_flags : u2 {
  METHOD_ACC_PUBLIC = 0x0001,
  METHOD_ACC_PRIVATE = 0x0002,
  METHOD_ACC_PROTECTED = 0x0004,
  METHOD_ACC_STATIC = 0x0008,
  METHOD_ACC_FINAL = 0x0010,
  METHOD_ACC_SYNCHRONIZED = 0x0020,
  METHOD_ACC_BRIDGE = 0x0040,
  METHOD_ACC_VARARGS = 0x0080,
  METHOD_ACC_NATIVE = 0x0100,
  METHOD_ACC_ABSTRACT = 0x0400,
  METHOD_ACC_STRICT = 0x0800,
  METHOD_ACC_SYNTHETIC = 0x1000,
} method_access_flags;

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

typedef struct method_info {
  u2 access_flags;
  u2 name_index;
  u2 descriptor_index;
  u2 attributes_count;
  attribute_info *attributes;
} method_info;

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

typedef struct class_file {
  u4 magic;
  u2 minor_version;
  u2 major_version;
  u2 constant_pool_count;
  cp_info *constant_pool;
  u2 access_flags;
  u2 this_class;
  u2 super_class;
  u2 interfaces_count;
  u2 *interfaces;
  u2 fields_count;
  field_info *fields;
  u2 methods_count;
  method_info *methods;
  u2 attributes_count;
  attribute_info *attributes;
} class_file;

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

typedef enum jon_value_type : u1 {
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

typedef struct jon_value {
  jon_value_type type;
  union {
    bool boolean_value;
    int int_value;
    float float_value;
    double double_value;
    char *string_value;
    struct jon_value *array_value;
    struct jon_value_pair *object_value;
  } value;
} jon_value;

typedef struct jon_value_pair {
  char *key;
  jon_value value;
} jon_value_pair;

typedef enum opcode : u1 {
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
  op_dup = 0x59,
  op_return = 0xb1,
  op_putstatic = 0xb3,
  op_invokestatic = 0xb8,
  op_newarray = 0xbc,
  op_anewarray = 0xbd,
} opcode;

static const char *boolean_descriptor = "Z";
static const char *int_descriptor = "I";
static const char *float_descriptor = "F";
static const char *double_descriptor = "D";
static const char *string_descriptor = "Ljava/lang/String;";
static const char *list_descriptor = "Ljava/util/List;";

void fill_jon_value_type(jon_value *value, utf8_info *descriptor) {
  if (strcmp((char *)descriptor->bytes, boolean_descriptor) == 0) {
    value->type = jon_value_type_boolean;
  } else if (strcmp((char *)descriptor->bytes, int_descriptor) == 0) {
    value->type = jon_value_type_int;
  } else if (strcmp((char *)descriptor->bytes, float_descriptor) == 0) {
    value->type = jon_value_type_float;
  } else if (strcmp((char *)descriptor->bytes, double_descriptor) == 0) {
    value->type = jon_value_type_float;
  } else if (strcmp((char *)descriptor->bytes, string_descriptor) == 0) {
    value->type = jon_value_type_string;
  } else if (strcmp((char *)descriptor->bytes, list_descriptor) == 0) {
    value->type = jon_value_type_array;
  } else {
    value->type = jon_value_type_object;
  }
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

int jon_object_set_value(jon_value_pair *object, char *key, jon_value value) {
  size_t object_size = sizeof(object) / sizeof(jon_value_pair);
  for (int i = 0; i < object_size; i++) {
    if (strcmp(object[i].key, key) == 0) {
      object[i].value = value;
      return 0;
    }
  }

  return -1;
}

float convert_float(u4 bytes) {
  int bits = bytes;

  if (bits == 0x7f800000) return INFINITY;
  if (bits == 0xff800000) return -INFINITY;
  if ((bits >= 0x7f800001 && bits <= 0x7fffffff) ||
      (bits >= 0xff800001 && bits <= 0xffffffff))
    return NAN;

  int s = ((bits >> 31) == 0) ? 1 : -1;
  int e = ((bits >> 23) & 0xff);
  int m = (e == 0) ? (bits & 0x7fffff) << 1 : (bits & 0x7fffff) | 0x800000;

  return s * m * pow(2, e - 150);
}

double convert_double(u4 high_bytes, u4 low_bytes) {
  u8 bits = ((u8)high_bytes << 32) | low_bytes;

  if (bits == 0x7ff0000000000000) return INFINITY;
  if (bits == 0xfff0000000000000) return -INFINITY;
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

typedef enum array_type {
  array_type_boolean = 0x4,
  array_type_char = 0x5,
  array_type_float = 0x6,
  array_type_double = 0x7,
  array_type_byte = 0x8,
  array_type_short = 0x9,
  array_type_int = 0xa,
} array_type;

type_tag *init_array_type_to_type_tag_map() {
  type_tag *map = (type_tag *)malloc(11 * sizeof(type_tag));
  map[array_type_boolean] = type_boolean;
  map[array_type_char] = type_int;
  map[array_type_float] = type_float;
  map[array_type_double] = type_double;
  map[array_type_byte] = type_int;
  map[array_type_short] = type_int;
  map[array_type_int] = type_int;
  return map;
}

static const type_tag *array_type_map = init_array_type_to_type_tag_map();

int interpret_code(code_attribute *code_attr, jon_value_pair *object,
                   cp_info *pool) {
  stack s;
  stack_init(&s, code_attr->max_stack);
  for (int i = 0; i < code_attr->code_length; i++) {
    u1 opcode = code_attr->code[i];
    switch (opcode) {
      case op_iconst_m1: {
        stack_push(&s, -1);
      } break;
      case op_iconst_0: {
        stack_push(&s, 0);
      } break;
      case op_iconst_1: {
        stack_push(&s, 1);
      } break;
      case op_iconst_2: {
        stack_push(&s, 2);
      } break;
      case op_iconst_3: {
        stack_push(&s, 3);
      } break;
      case op_iconst_4: {
        stack_push(&s, 4);
      } break;
      case op_iconst_5: {
        stack_push(&s, 5);
      } break;
      case op_fconst_0: {
        stack_push(&s, 0.0f);
      } break;
      case op_fconst_1: {
        stack_push(&s, 1.0f);
      } break;
      case op_fconst_2: {
        stack_push(&s, 2.0f);
      } break;
      case op_dconst_0: {
        stack_push(&s, (double)0.0);
      } break;
      case op_dconst_1: {
        stack_push(&s, (double)1.0);
      } break;
      case op_sipush: {
        u2 value = (code_attr->code[++i] << 8) | code_attr->code[++i];
        stack_push(&s, value);
      } break;
      case op_newarray: {
        u1 atype = code_attr->code[++i];

        stack_entry entry;
        stack_pop(&s, &entry);
        int count = entry.value.int_value;

        array a;
        a.tag = array_type_map[atype];
        a.length = count;
        a.elements = (stack_value *)malloc(count * sizeof(stack_value));
        printf("newarray: %d with length: %d\n", a.tag, a.length);
        stack_push(&s, a);
      } break;
      case op_anewarray: {
        printf("anewarray\n");
      } break;
      case op_ldc: {
        u1 index = code_attr->code[++i];
        cp_info *pool_entry = &pool[index - 1];
        switch (pool_entry->tag) {
          case CONSTANT_Integer: {
            integer_info *i_info = (integer_info *)pool_entry->info;
            stack_push(&s, (int)i_info->bytes);
          } break;
          case CONSTANT_Float: {
            float_info *f_info = (float_info *)pool_entry->info;
            stack_push(&s, convert_float(f_info->bytes));
          } break;
          case CONSTANT_String: {
            string_info *s_info = (string_info *)pool_entry->info;
            utf8_info *utf8 = (utf8_info *)pool[s_info->string_index - 1].info;
            stack_push(&s, (char *)utf8->bytes);
          } break;
          default:
            printf("unknown ldc type: %d\n", pool_entry->tag);
            return -1;
        }
      } break;
      case op_ldc2_w: {
        u2 index = (code_attr->code[++i] << 8) | code_attr->code[++i];
        cp_info *pool_entry = &pool[index - 1];
        printf("ldc2_w index: %d, tag: %d\n", index, pool_entry->tag);
        switch (pool_entry->tag) {
          case CONSTANT_Double: {
            double_info *d_info = (double_info *)pool_entry->info;
            printf("double tag: %d\n", d_info->tag);
            printf("high bytes: %x, low bytes: %x\n", d_info->high_bytes,
                   d_info->low_bytes);

            double d = convert_double(d_info->high_bytes, d_info->low_bytes);
            stack_push(&s, d);
          } break;
          default:
            printf("unknown ldc2_w type: %d\n", pool_entry->tag);
            return -1;
        }
      } break;
      case op_dastore: {
        stack_entry value_entry;
        stack_pop(&s, &value_entry);

        stack_entry index;
        stack_pop(&s, &index);

        stack_entry array_entry;
        stack_pop(&s, &array_entry);

        stack_entry v;
        stack_peek(&s, &v);

        array_entry.value.array_value.elements[index.value.int_value] =
            value_entry.value;
        printf("dastore: %f\n", value_entry.value.double_value);
      } break;
      case op_dup: {
        stack_entry entry;
        stack_peek(&s, &entry);
        stack_push(&s, entry);
      } break;
      case op_invokestatic: {
        u2 index = (code_attr->code[++i] << 8) | code_attr->code[++i];
        utf8_info *name = (utf8_info *)pool[index - 1].info;
        printf("invokestatic: %s\n", name->bytes);
      } break;
      case op_putstatic: {
        u2 index = (code_attr->code[++i] << 8) | code_attr->code[++i];
        utf8_info *name =
            find_in_ref_info(pool, (ref_info *)pool[index - 1].info);
        // TODO(ilya): find descriptor and set value
        printf("putstatic: %s\n", name->bytes);
        while (!stack_empty(&s)) {
          stack_entry entry;
          stack_pop(&s, &entry);
          printf("entry: ");
          print_stack_entry(&entry);
          printf("\n");
        }
        stack_reset(&s);
      } break;

      case op_return: {
        printf("return\n");
        return 0;
      } break;

      default:
        printf("unknown opcode: %x\n", opcode);
        return -1;
    }
  }

  return 0;
}

int fill_jon_object(jon_value_pair *object, class_file *c_file) {
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

  if (interpret_code(code_attr, object, c_file->constant_pool) < 0) {
    perror("could not interpret code");
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  char *buffer;
  if ((buffer = _getcwd(NULL, 0)) == NULL) {
    perror("_getcwd error");
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; i++) {
    char path[FILENAME_MAX];
    snprintf(path, sizeof(path), "%s\\%s", buffer, argv[i]);
    printf("path: %s\n", path);

    class_file c_file;
    parse_file(path, &c_file);

    jon_value_pair *object =
        (jon_value_pair *)malloc(sizeof(jon_value_pair) * c_file.fields_count);

    if (fill_jon_object(object, &c_file) < 0) {
      perror("could not fill jon object");
      return EXIT_FAILURE;
    }

    printf("jon object:\n");
    for (int i = 0; i < c_file.fields_count; i++) {
      printf("key: %s, type: %x\n", object[i].key, object[i].value.type);
    }
  }

  free(buffer);
  return EXIT_SUCCESS;
}
