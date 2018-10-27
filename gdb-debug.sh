#! /bin/bash

PORT=3000
BIN_FILE="./kernel/kernel.raw"

# msim -g $PORT must run in other terminal
mipsel-linux-gnu-gdb "$BIN_FILE" -ex "target remote localhost:$PORT"
