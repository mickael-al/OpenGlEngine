#!/bin/bash

run_update_path() {
  chmod +x LastSignalProject/update_path.sh
  cd LastSignalProject || { echo "Error: LastSignalProject directory not found"; return 1; }
  ./update_path.sh
  cd - || { echo "Error: cannot return to previous directory"; return 1; }
}
run_update_path
# --- Initial configuration ---
projectDir=$(pwd)
buildType=${1:-Debug}  # Default to Debug if not specified (can pass "Release" as argument)
buildDir="$projectDir/Bin/$buildType"

# --- Check if cmake is installed ---
if ! command -v cmake &> /dev/null; then
    echo "❌ Error: cmake is not installed."
    exit 1
fi

# --- Detect available C++ compiler ---
if command -v gcc &> /dev/null; then
    cxx_compiler=$(command -v g++)
    compiler_name="GCC"
elif command -v clang &> /dev/null; then
    cxx_compiler=$(command -v clang++)
    compiler_name="Clang"
else
    echo "❌ Error: No compatible compiler found (GCC or Clang required)."
    exit 1
fi

echo "✅ Detected compiler: $compiler_name ($cxx_compiler)"

# --- Create build output directory ---
mkdir -p "$buildDir"
cd "$buildDir"

# --- Run CMake generation ---
echo "🛠️  Running CMake in $buildDir"
cmake "$projectDir" \
    -DCMAKE_BUILD_TYPE=$buildType \
    -DCMAKE_CXX_COMPILER=$cxx_compiler \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$buildDir"

# --- Build the project ---
echo "🔧 Building..."
cmake --build . --config $buildType

echo "✅ Build completed. Executable is in: $buildDir"
