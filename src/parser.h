#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef int32_t i4;

#define READ_U1(file, name) fread(&name, sizeof(name), 1, file);

#define READ_U2(file, name)                                                    \
  fread(&name, sizeof(name), 1, file);                                         \
  name = _byteswap_ushort(name);

#define READ_U4(file, name)                                                    \
  fread(&name, sizeof(name), 1, file);                                         \
  name = _byteswap_ulong(name);

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

typedef struct ref_info {
  u1 tag;
  u2 class_index;
  u2 name_and_type_index;
} ref_info;

typedef struct name_and_type_info {
  u1 tag;
  u2 name_index;
  u2 descriptor_index;
} name_and_type_info;

typedef struct utf8_info {
  u1 tag;
  u2 length;
  u1 *bytes;
} utf8_info;

typedef struct string_info {
  u1 tag;
  u2 string_index;
} string_info;

typedef struct integer_info {
  u1 tag;
  u4 bytes;
} integer_info;

typedef struct float_info {
  u1 tag;
  u4 bytes;
} float_info;

typedef struct long_info {
  u1 tag;
  u4 high_bytes;
  u4 low_bytes;
} long_info;

typedef struct double_info {
  u1 tag;
  u4 high_bytes;
  u4 low_bytes;
} double_info;

void read_class_info(FILE *file, class_info *info);
void read_ref_info(FILE *file, ref_info *info);
void read_name_and_type_info(FILE *file, name_and_type_info *info);
void read_utf8_info(FILE *file, utf8_info *info);
void read_string_info(FILE *file, string_info *info);
void read_integer_info(FILE *file, integer_info *info);
void read_float_info(FILE *file, float_info *info);
void read_long_info(FILE *file, long_info *info);
void read_double_info(FILE *file, double_info *info);
bool read_constant_pool(FILE *file, cp_info *pool, u2 count);

typedef enum class_access_flags {
  CLASS_ACC_PUBLIC = 0x0001,
  CLASS_ACC_FINAL = 0x0010,
  CLASS_ACC_SUPER = 0x0020,
  CLASS_ACC_INTERFACE = 0x0200,
  CLASS_ACC_ABSTRACT = 0x0400,
  CLASS_ACC_SYNTHETIC = 0x1000,
  CLASS_ACC_ANNOTATION = 0x2000,
  CLASS_ACC_ENUM = 0x4000,
} class_access_flags;

typedef struct attribute_info {
  u2 attribute_name_index;
  u4 attribute_length;
  u1 *info;
} attribute_info;

typedef struct signature_attribute {
  u2 attribute_name_index;
  u4 attribute_length;
  u2 signature_index;
} signature_attribute;

typedef struct enum_const_value {
  u2 type_name_index;
  u2 const_name_index;
} enum_const_value;

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

typedef struct annotation {
  u2 type_index;
  u2 num_element_value_pairs;
  element_value_pair *element_value_pairs;
} annotation;

typedef struct runtime_visible_annotations_attribute {
  u2 attribute_name_index;
  u4 attribute_length;
  u2 num_annotations;
  annotation *annotations;
} runtime_visible_annotations_attribute;

typedef struct exception_table {
  u2 start_pc;
  u2 end_pc;
  u2 handler_pc;
  u2 catch_type;
} exception_table;

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

typedef struct line_number_table {
  u2 start_pc;
  u2 line_number;
} line_number_table;

typedef struct line_number_table_attribute {
  u2 attribute_name_index;
  u4 attribute_length;
  u2 line_number_table_length;
  line_number_table *line_number_table;
} line_number_table_attribute;

typedef struct source_file_attribute {
  u2 attribute_name_index;
  u4 attribute_length;
  u2 sourcefile_index;
} source_file_attribute;

void print_all_class_access_flags(u2 access_flags);

bool read_attributes(FILE *file, attribute_info *attributes, u2 count,
                     cp_info *pool);

void read_signature_attribute(FILE *file, signature_attribute *info,
                              cp_info *pool);
void read_element_value_pair(FILE *file, element_value_pair *info,
                             cp_info *pool);
void read_annotation(FILE *file, annotation *info, cp_info *pool);
void read_runtime_visible_annotations_attribute(
    FILE *file, runtime_visible_annotations_attribute *info, cp_info *pool);
void read_exception_table(FILE *file, exception_table *table);
bool read_code_attribute(FILE *file, code_attribute *info, cp_info *pool);
void read_line_number_table(FILE *file, line_number_table *table);
bool read_line_number_table_attribute(FILE *file,
                                      line_number_table_attribute *info);
void read_source_file_attribute(FILE *file, source_file_attribute *info,
                                cp_info *pool);
bool read_attributes(FILE *file, attribute_info *attributes, u2 count,
                     cp_info *pool);

typedef enum field_access_flags {
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

typedef struct field_info {
  u2 access_flags;
  u2 name_index;
  u2 descriptor_index;
  u2 attributes_count;
  attribute_info *attributes;
} field_info;

void print_all_field_access_flags(u2 access_flags);

bool parse_fields(FILE *file, field_info *fields, u2 count, cp_info *pool);

typedef enum method_access_flags {
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

typedef struct method_info {
  u2 access_flags;
  u2 name_index;
  u2 descriptor_index;
  u2 attributes_count;
  attribute_info *attributes;
} method_info;

void print_all_method_access_flags(u2 access_flags);
bool read_methods(FILE *file, method_info *methods, u2 count, cp_info *pool);

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

bool parse_file(char *path, class_file *c_file);
