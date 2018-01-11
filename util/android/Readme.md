To build:

1. setup an Android NDK build environment and build SFML, per the tutorial on the SFML wiki:

      https://github.com/SFML/SFML/wiki/Tutorial%3A-Building-SFML-for-Android

2. from Attract-Mode's util/android directory, run the following commands:

      android update project --target "android-25" --path .

      ndk-build APP_BUILD_SCRIPT=./jni/Android.mk APP_ABI=x86 APP_STL=c++_shared

      ant debug

3. This will create the AttractMode-debug.apk file in the bin directory.
