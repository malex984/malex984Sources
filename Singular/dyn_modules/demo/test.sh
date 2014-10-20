#!/bin/sh

export SINGULAR_EXECUTABLE="$SINGULAR_BIN_DIR/Singular"

"$SINGULAR_EXECUTABLE" -tec 'ring r; r; listvar(Top); my_sq_poly_add(var(1), var(2)); load("demo.so"); listvar(Demo); Demo::my_sq_poly_add(var(1), var(2)); my_sq_poly_add(var(1), var(2));$' || exit 1

exit 0
