#include <windows.h>
#include <direct.h>
#include "value.h"
#include "parser.h"
#include "interpreter.h"
#include "json.h"

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

    object obj;
    object_init(&obj, c_file.fields_count);

    FILE *json_file = fopen("output.json", "w");
    if (!json_file) {
      perror("could not open json file");
      return EXIT_FAILURE;
    }

    method_info *clinit = find_clinit(&c_file);
    if (!clinit) {
      perror("could not find clinit");
      return -1;
    }

    code_attribute *code_attr =
        find_code_attribute(clinit, c_file.constant_pool);
    if (!code_attr) {
      perror("could not find code attribute");
      return -1;
    }

    if (!interpret_code(code_attr, &obj, c_file.constant_pool)) {
      perror("could not interpret code");
      return -1;
    }

    object_to_json(json_file, &obj);
  }

  free(buffer);
  return EXIT_SUCCESS;
}
