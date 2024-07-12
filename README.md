# motion-blur-test
A mod for mcpelauncher-manifest that mimics motion blur in a ***weird way*** ( ͡° ͜ʖ ͡°)

## Usage:
1. Compile the source code. <!--Or download from [releases](https://github.com/CrackedMatter/motion-blur-test/releases/latest)-->
2. Open mcpelauncher-manifest, go to settings -> storage -> Open Data Root
3. Make a folder named `mods` there and put the compiled .so file
4. Done

#### How to turn off?
Change mod extension to .bak or delete the .so file.

## Building

Download [Android NDK](https://developer.android.com/ndk/downloads).

`PATH_TO_NDK="/path/to/ndk"`
- **x86**

  ```
  cmake -DCMAKE_TOOLCHAIN_FILE=$PATH_TO_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=x86 ..
  ```
- **x86_64**

  ```
  cmake -DCMAKE_TOOLCHAIN_FILE=$PATH_TO_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=x86_64 ..
  ```
- **armeabi-v7a**

  ```
  cmake -DCMAKE_TOOLCHAIN_FILE=$PATH_TO_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=armeabi-v7a ..
  ```
- **arm64-v8a**

  ```
  cmake -DCMAKE_TOOLCHAIN_FILE=$PATH_TO_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a ..
  ```
