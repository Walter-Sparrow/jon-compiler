#include <windows.h>
#include <direct.h>
#include "value.h"
#include "parser.h"
#include "interpreter.h"
#include "json.h"

void get_package(const char *class_name, char *out) {
  strcpy(out, class_name);
  char *last_slash = strrchr(out, '/');
  if (last_slash) {
    *last_slash = '\0';
  }
}

void get_class_name(const char *descriptor, char *out) {
  char *class_name = (char *)descriptor + 1;
  char *last_slash = strrchr(class_name, '/');
  if (last_slash) {
    class_name = last_slash + 1;
  }
  char *last_semicolon = strrchr(class_name, ';');
  if (last_semicolon) {
    *last_semicolon = '\0';
  }
  strcpy(out, class_name);
}

int same_package(const char *desc_package, const char *my_package) {
  return strcmp(desc_package, my_package) == 0;
}

object create_object_recursively(const char *class_path) {
  class_file c_file;
  if (!parse_file((char *)class_path, &c_file)) {
    fprintf(stderr, "parse_file failed for: %s\n", class_path);
    exit(EXIT_FAILURE);
  }

  object obj;
  object_init(&obj, c_file.fields_count);

  method_info *clinit = find_clinit(&c_file);
  if (!clinit) {
    perror("could not find <clinit>");
    exit(EXIT_FAILURE);
  } else {
    code_attribute *code_attr =
        find_code_attribute(clinit, c_file.constant_pool);
    if (!code_attr) {
      perror("could not find code attribute for <clinit>");
      exit(EXIT_FAILURE);
    }
    if (!interpret_code(code_attr, &obj, c_file.constant_pool)) {
      perror("could not interpret code for <clinit>");
      exit(EXIT_FAILURE);
    }
  }

  class_info *c_info =
      (class_info *)c_file.constant_pool[c_file.this_class - 1].info;
  utf8_info *class_name =
      (utf8_info *)c_file.constant_pool[c_info->name_index - 1].info;

  char package[class_name->length + 1];
  get_package((char *)class_name->bytes, package);

  for (u2 i = 0; i < c_file.fields_count; i++) {
    field_info *field = &c_file.fields[i];
    utf8_info *name =
        (utf8_info *)c_file.constant_pool[field->name_index - 1].info;
    utf8_info *descriptor =
        (utf8_info *)c_file.constant_pool[field->descriptor_index - 1].info;

    if (descriptor->bytes[0] != 'L') {
      continue;
    }

    char *full_class_name = strdup((char *)descriptor->bytes + 1);
    char *semicolon = strchr(full_class_name, ';');
    if (semicolon) {
      *semicolon = '\0';
    }

    char field_package[strlen(full_class_name) + 1];
    get_package(full_class_name, field_package);

    if (!same_package(field_package, package)) {
      free(full_class_name);
      continue;
    }

    char field_class_name[strlen(full_class_name) + 1];
    get_class_name(full_class_name, field_class_name);

    char field_class_path[FILENAME_MAX];
    snprintf(field_class_path, sizeof(field_class_path), "%s.class",
             field_class_name);

    object *nested_obj = (object *)malloc(sizeof(object));
    *nested_obj = create_object_recursively(field_class_path);

    value field_value;
    field_value.type = VALUE_OBJECT;
    field_value.as.object_value = nested_obj;
    object_set_value(&obj, (char *)name->bytes, field_value);

    free(full_class_name);
  }

  return obj;
}

int main(int argc, char *argv[]) {
  char *buffer = _getcwd(NULL, 0);
  if (!buffer) {
    perror("_getcwd error");
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; i++) {
    char path[FILENAME_MAX];
    snprintf(path, sizeof(path), "%s\\%s", buffer, argv[i]);
    printf("Processing path: %s\n", path);

    object obj = create_object_recursively(path);

    FILE *json_file = fopen("output.json", "w");
    if (!json_file) {
      perror("could not open json file");
      free(buffer);
      return EXIT_FAILURE;
    }

    printf("Serializing to json...\n");
    object_to_json(json_file, &obj);
    fclose(json_file);
  }

  free(buffer);
  return EXIT_SUCCESS;
}
