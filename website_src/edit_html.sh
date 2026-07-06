#!/bin/bash

if [ -z "$1" ]; then
  echo "Please specify a filename as parameter"
  exit 1
fi

file="$1"

if [[ "$OSTYPE" == "darwin"* ]]; then
  sed -i '' 's|/assets/index-[0-9a-zA-Z]*.js|index.js|g' "$file"
  sed -i '' 's|/assets/index-[0-9a-zA-Z]*.css|index.css|g' "$file"
else
  sed -i 's|/assets/index-[0-9a-zA-Z]*.js|index.js|g' "$file"
  sed -i 's|/assets/index-[0-9a-zA-Z]*.css|index.css|g' "$file"
fi
