#!/bin/bash

IMAGE_DIR="$HOME/Pictures/terminal_pictures"
MAIN_DIRECTORY="$HOME/cpp_study/braille-image-converter-and-fetcher"
if [ ! -d "$IMAGE_DIR" ]; then
  echo "Error! Directory $IMAGE_DIR does not exist"
  exit 1
fi

RANDOM_IMAGE=$(find "$IMAGE_DIR" -maxdepth 1 -type f \( -iname "*.png" -o -iname "*.jpeg" -o -iname "*.jpg" \) | shuf -n 1)

#if file name size is zero
if [ -z "$RANDOM_IMAGE" ]; then
  echo "Error! failed to load"
  exit 1
fi

if [ ! -f "$MAIN_DIRECTORY/braille_image_drawer_2" ]; then
  echo "Executable not found"
  exit 1
fi

$MAIN_DIRECTORY/braille_image_drawer_2 "$RANDOM_IMAGE"
