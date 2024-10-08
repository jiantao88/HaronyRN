---
id: react-native-integration
title: React Native Integration
---

## Using a custom Hermes build in a React Native app

To make Hermes usable in a React Native app we need to perform the following steps:

1. Package a Hermes build into a npm package structure.
2. Register this npm package with Yarn for use in other local projects.
3. Link the npm package registered with Yarn into the React Native tree for our app.

### Making a Hermes npm package

A Hermes npm package combines:
* Hermes CLI tools in binary form. These are used to compile JavaScript to bytecode when packaging up an app release.
* Pre-compiled release and debug versions of the native Hermes library for Android targets.

The interface between Hermes and React Native changes over time, so if you are
not using the main branch of both, you may want to consider matching the
Hermes version with your app's React Native version. E.g. `git checkout v0.5.0`
to build for `react-native@0.63`.

#### Dependencies

To make a Hermes npm package, first follow the instructions on [building Hermes](BuildingAndRunning.md) to create a **Release Build** of Hermes. This will be the source of CLI binaries for use in the npm.

As we will be compiling native libraries for Android, we first need to setup a suitable development environment. This subject is covered in more detail elsewhere, for example in the guide on [building React Native From Source](https://github.com/facebook/react-native/wiki/Building-from-source). However, a rough guide to the external dependencies for macOS/Linux is as follows:

* Node.js
* JDK (1.8+)
* Yarn
* The Android Native Development Kit (NDK)
* The Android Software Development Kit (SDK)

> Note the Android SDK is typically downloaded and installed automatically as part of the flow in Android Studio. If you are following this common route, be sure to note the location Android Studio uses to install the SDK.

#### Build environment

Ensure the following variables are present and set correctly in your environment:

* `ANDROID_NDK` - the root of your Android NDK install. E.g. `/opt/android_ndk/r15c`.
* `ANDROID_SDK`, `ANDROID_HOME`, and `ANDROID_SDK_ROOT` - the root of your Android SDK install. E.g. `/opt/android_sdk`.
* `JAVA_HOME` - the root of your Java Runtime Environment (JRE) install. Note a JRE instance is typically available in a sub-directory of a JDK install. E.g. `/opt/jdk-1.8/jre`
* `HERMES_WS_DIR` - The root of your workspace where the `hermes` git checkout directory and `build_release` Release Build directory should already be subdirectories. E.g. `$HOME/workspace`.

Make sure the `node` and `yarn` binaries are available in your system PATH.

#### Package build

To make a Hermes npm package, check that you already followed the [Building Hermes](BuildingAndRunning.md) instructions for a **Release Build**. The `build_release` directory will be the source of CLI binaries for use in the npm.

```shell
# Package CLI binaries for the current platform
(cd $HERMES_WS_DIR/build_release && ninja github-cli-release)

# Package Android libraries
(cd $HERMES_WS_DIR/hermes/android && ./gradlew githubRelease)

# Copy the above to the NPM directory
cp $HERMES_WS_DIR/build_android/distributions/hermes-runtime-android-v*.tar.gz $HERMES_WS_DIR/hermes/npm
cp $HERMES_WS_DIR/build_release/github/hermes-cli-*-v*.tar.gz $HERMES_WS_DIR/hermes/npm

# Create NPMs
(cd $HERMES_WS_DIR/hermes/npm && yarn install && yarn run unpack-builds-dev && yarn run create-npms-dev)
```

The NPM bundles can now be found in `$HERMES_WS_DIR/hermes/npm/*.tgz`


### Linking Hermes into a React Native app

To use your custom Hermes npm package in an app, first make sure the app works with a normal release of Hermes by following [instructions in the React Native docs](https://reactnative.dev/docs/hermes).

Next, install the Hermes npm package into your app as a dependency, and (if
running react-native from source) into react-native as well.  For example,
assuming your project is in the directory `$AWESOME_PROJECT` you would run this
command:

```shell
# Install the Hermes NPM to be used by the project
( cd ${AWESOME_PROJECT?} && yarn add $HERMES_WS_DIR/hermes/npm/hermes-engine-v*.tgz )

# If running react-native from source, install it there as well
( cd ${AWESOME_PROJECT?}/node_modules/react-native && yarn add $HERMES_WS_DIR/hermes/npm/hermes-engine-v*.tgz )
```

You can now develop your app in the normal way.

> Note: if you are making changes to the compiler in Hermes, be sure to make sure you test your app in release mode as this enables bytecode compilation in advance.

## Reporting native crashes

If Hermes causes a native crash in your application, a stack trace is critical for us to be able to understand where the crash occurred. 

If you have a native crash to report, please be aware that in most cases, it is not feasible for us to debug crashes in earlier versions of Hermes. Please update your app to the latest React Native version, as that contains the latest hermes-engine versions. Specifically since React Native 0.69, Hermes is now coming [bundled together with React Native](https://reactnative.dev/architecture/bundled-hermes).

Including the symbolicated stack trace will make it easier for us to address your bug report more quickly.

### How to symbolicate a native crash on Android

To symbolicate a native crash on Android, you need to use the [`ndk-stack`](https://developer.android.com/ndk/guides/ndk-stack) command.
This command is part of the Android SDK and you should be able to access with `$ANDROID_HOME/ndk/<ndk_version>/ndk-stack` (provided you replace the URL with your NDK version like `$ANDROID_HOME/ndk/21.4.7075529/ndk-stack`).

To use the `ndk-stack` command you will need the debug symbols (check the paragraph below to see where to find them).

You can then symbolicate a stacktrace as follows:

```bash
$ANDROID_HOME/ndk/21.4.7075529/ndk-stack -sym ./<path-to-debug-symbols>/obj/local/arm64-v8a < crash.txt
```

provided that `crash.txt` contains your stacktrace in plain text, and you replace `armeabi-v7a` with the correct architecture you used when the native crash was recorded.

As an alternative, you can trigger a crash and have a symbolicated logcat in real time with the following command:

```bash
adb logcat | $ANDROID_HOME/ndk/21.4.7075529/ndk-stack -sym ./<path-to-debug-symbols>/obj/local/arm64-v8a
```

### Where to find the debug symbols

#### React Native < 0.69

For React Native versions before 0.69 (i.e. hermes-engine 0.11.0 and previous versions), you can find the debug symbols in the [GitHub release](https://github.com/facebook/hermes/releases) corresponding to the Hermes NPM you used to build your application. 

Download the `hermes-runtime-android-vX.Y.Z.tar.gz` file for your version, unpack the tar file, then run `ndk-stack` using the contained directory as described in the paragraph above.

#### React Native >= 0.69

For React Native versions after and including 0.69, you can find the debug symbols in the [GitHub release](https://github.com/facebook/react-native/releases) corresponding to the React Native version you used to build your application. 

For instance for [React Native 0.70.6](https://github.com/facebook/react-native/releases/tag/v0.70.6), you can find the `hermes-native-symbols-v0.70.6.zip` file with the debug symbols inside.
