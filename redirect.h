#pragma once

int chili_redirect_begin();
void chili_redirect_start(const char *name);
void chili_redirect_stop();
void chili_redirect_print(const char *name,
    const char *before, const char *after);
void chili_redirect_end();
