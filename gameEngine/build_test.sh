#!/bin/bash
set -e  # Stoppe le script si une commande échoue

# --- Paramètres ---
TEST_DIR="tests"
BUILD_DIR="$TEST_DIR/.build"

echo "🔧 Configuration du dossier de build..."
mkdir -p "$BUILD_DIR"

echo "🧱 Génération des fichiers CMake..."
cmake -S "$TEST_DIR" -B "$BUILD_DIR"

echo "⚙️  Compilation des tests..."
cmake --build "$BUILD_DIR" -- -j$(nproc)

echo "🚀 Exécution des tests..."
cd "$BUILD_DIR"
ctest --output-on-failure
cd ../..