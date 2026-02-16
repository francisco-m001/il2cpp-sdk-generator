<div align="center">

# il2cpp-sdk-generator

![C++](https://img.shields.io/badge/C++-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D4?style=flat-square&logo=windows&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)

**Automated SDK generator for Unity IL2CPP binaries.**

Parses [Il2CppDumper](https://github.com/Perfare/Il2CppDumper) JSON output and generates typed C++ headers and method stubs with resolved runtime offsets.

</div>

---

## Overview

Unity's IL2CPP compiler converts C# assemblies into native C++ code, stripping type metadata in the process. This tool reconstructs that metadata into usable C++ bindings by:

1. Running Il2CppDumper against the target `GameAssembly.dll` and `global-metadata.dat`
2. Parsing the resulting `script.json` for method signatures and addresses
3. Generating a complete `Methods.h` header with forward declarations
4. Generating a `Methods.cpp` implementation with static function pointers resolved to base address + offset

## Generated Output

Given a dumped method like:

```json
{
  "Name": "BasePlayer::GetHealth",
  "Address": 1234567,
  "Signature": "float BasePlayer::GetHealth(void* __this, const void* method);"
}
```

The generator produces:

**Methods.h**
```cpp
#pragma once
#include "il2cpp.h"

float BasePlayer::GetHealth(void* __this, const void* method);
```

**Methods.cpp**
```cpp
#include "Methods.h"

float BasePlayer::GetHealth(void* __this, const void* method)
{
    static float (*BasePlayer::GetHealth)(void* __this, const void* method) =
        reinterpret_cast<decltype(BasePlayer::GetHealth)>(GetModuleBaseAddress("GameAssembly.dll") + 1234567);
    return BasePlayer::GetHealth(__this, method);
}
```

## Project Structure

```
il2cpp-sdk-generator/
  Il2cppSdkDumper.sln                Visual Studio solution
  Il2cppSdkDumper/
    Il2cppSdkDumper.cpp              Main entry point and code generation logic
    Methods.h                        Generated header (output)
    Methods.cpp                      Generated implementation (output)
```

## Building

### Requirements

- Visual Studio 2019+ with C++ desktop workload
- Windows SDK
- [nlohmann/json](https://github.com/nlohmann/json) (JSON parsing)
- [Il2CppDumper](https://github.com/Perfare/Il2CppDumper) (for generating input `script.json`)

### Steps

1. Open `Il2cppSdkDumper.sln` in Visual Studio
2. Update the path constants in `Il2cppSdkDumper.cpp`:
   ```cpp
   #define IL2CPP_DUMPER_FOLDER std::string("path/to/il2cppdumper/")
   #define RUST_FOLDER std::string("path/to/target/game/")
   ```
3. Build in **Release | x64**
4. Run -- the tool will optionally invoke Il2CppDumper, then parse `script.json` and write `Methods.h` / `Methods.cpp`

## How It Works

```
GameAssembly.dll + global-metadata.dat
            |
      Il2CppDumper
            |
       script.json
            |
   il2cpp-sdk-generator
            |
    Methods.h + Methods.cpp
```

1. **Dump** -- Il2CppDumper extracts method names, signatures, and RVA offsets from the IL2CPP binary
2. **Parse** -- The generator reads `script.json` and builds a map of method name to `{address, signature}`
3. **Generate headers** -- Writes each method's forward declaration to `Methods.h`
4. **Generate stubs** -- For each method, generates a `.cpp` stub that declares a static function pointer resolved at runtime via `GetModuleBaseAddress("GameAssembly.dll") + offset`, then forwards the call through it

## Disclaimer

This project is provided for **educational and research purposes only**. Use it responsibly and in compliance with applicable laws and terms of service.
