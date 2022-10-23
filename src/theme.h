#ifndef THEME_H
#define THEME_H 1

#include "browser.h"

GPtrArray *list_themes();
void print_themes();
void load_theme_config();
void load_theme(Browser *browser);

#endif
