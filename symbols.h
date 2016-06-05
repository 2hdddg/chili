#pragma once

int chili_sym_begin(const char *path, int *count);
int chili_sym_next(int index, char **name);
void chili_sym_end();

