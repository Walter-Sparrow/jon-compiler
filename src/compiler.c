#include <windows.h>
#include <direct.h>
#include "parser.h"
#include "interpreter.h"
#include "jon.h"

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

    if (fill_jon_object(object, c_file.fields_count, &c_file) < 0) {
      perror("could not fill jon object");
      return EXIT_FAILURE;
    }

    printf("jon object:\n");
    for (int i = 0; i < c_file.fields_count; i++) {
      printf("key: %s, type: %x\n", object[i].key, object[i].value.type);
    }

    FILE *json_file = fopen("output.json", "w");
    if (!json_file) {
      perror("could not open json file");
      return EXIT_FAILURE;
    }

    if (serialize_jon_object_as_json(object, c_file.fields_count, json_file) <
        0) {
      perror("could not serialize jon object as json");
      return EXIT_FAILURE;
    }
  }

  free(buffer);
  return EXIT_SUCCESS;
}
