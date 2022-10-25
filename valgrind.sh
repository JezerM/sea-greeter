#!/bin/sh
G_DEBUG=resident-modules valgrind \
  --suppressions=/usr/share/glib-2.0/valgrind/glib.supp \
  --suppressions=/usr/share/gtk-4.0/valgrind/gtk.supp \
  --num-callers=20 \
  --log-file=valgrind.log \
  $@
