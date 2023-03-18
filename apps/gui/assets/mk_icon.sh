#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
srcSVG="$SCRIPT_DIR/gpo_icon.svg"
sizes=("64" "128" "256" "512")

for size in $sizes; do
    inkscape -w $size -h $size -o $size.png $srcSVG
done

convert 64.png 128.png 256.png 512.png icon.ico