#!/bin/bash

# Vérifier si un fichier a été passé en paramètre
if [ -z "$1" ]; then
  echo "Veuillez spécifier un nom de fichier en tant que paramètre"
  exit 1
fi

# Récupérer le nom de fichier en tant que paramètre
file="$1"

# Modifier les chaînes de caractères
sed -i '' 's|/assets/index-[0-9a-zA-Z]*.js|index.js|g' $file
sed -i '' 's|/assets/index-[0-9a-zA-Z]*.css|index.css|g' $file