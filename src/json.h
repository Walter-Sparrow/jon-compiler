#pragma once

#include "value.h"
#include <stdio.h>

void value_to_json(FILE *file, value val);
void array_to_json(FILE *file, array *arr);
void object_to_json(FILE *file, object *obj);
