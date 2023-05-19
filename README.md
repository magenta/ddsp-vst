<div align="center">
    <img src="https://storage.googleapis.com/ddsp/github_images/ddsp_logo.png" width="200px" alt="logo"></img>
</div>

# [DDSP-VST](https://magenta.tensorflow.org/ddsp-vst) #

![Build](https://github.com/magenta/ddsp-vst/actions/workflows/build_and_test.yaml/badge.svg)
![Format](https://github.com/magenta/ddsp-vst/actions/workflows/format.yaml/badge.svg)

VST3/AU plugins and desktop applications built using the [JUCE](https://juce.com/) framework and [DDSP](https://github.com/magenta/ddsp).

<div align="center">
    <img width="400" alt="DDSP Effect" src="https://user-images.githubusercontent.com/7446124/167872854-ba8ddf52-e27f-4563-8d92-8e2da87573d4.png">
    <img width="400" alt="DDSP Synth" src="https://user-images.githubusercontent.com/7446124/167882854-4d15a746-1d01-4634-877e-afe4f90710f2.png">

</div>

## Helpful Links ##

* [Home page](https://g.co/magenta/ddsp-vst)
* [Blog post](https://magenta.tensorflow.org/ddsp-vst-blog)
* Download the latest plugins
    * [macOS](https://g.co/magenta/ddsp-vst-mac)
    * [Windows](https://g.co/magenta/ddsp-vst-windows)
* [Train a new model](https://g.co/magenta/train-ddsp-vst)
* [Join our DDSP community!](https://discord.gg/eyzhzMJMx5)

## Installation ##

### macOS ###

1. Download the plugin binaries from [here](https://g.co/magenta/ddsp-vst-mac).
2. Unzip and copy the two plugins (DDSP Effect and DDSP Synth) to their respective system folders:
    - Audio Unit: `/Library/Audio/Plug-Ins/Components`
    - VST3: `/Library/Audio/Plug-Ins/VST3`
3. Rescan for new plugins in your respective DAW (verified on Ableton/Logic Pro/FL Studio/Reaper).

### Windows ###

1. Download the plugin binaries from [here](https://g.co/magenta/ddsp-vst-windows).
2. Unzip and copy the two VST3 plugins (DDSP Effect and DDSP Synth) to `C:\Program Files\Common Files\VST3`
3. Rescan for new plugins in your respective DAW (verified on Ableton//Reaper).

## Usage ##
Take a look at [this guide](docs/getting-started.md) on how to use the plugins!

# Build #

## Supported Platforms ##

* macOS
* Windows

## Setup ##

### Prerequisites ###

* CMake 3.15 or above
* macOS
    * Xcode
    * Ninja (optional)
* Windows
    * Visual Studio 2022
    * Git Bash

### Initialize ###

Clone this repo and run the following script to initialize the submodules and download DDSP models.

``` shell
./repo-init.sh
```

### macOS ###

* Generate Xcode project files (recommended for development and debugging):
    * `cmake -B build -S . -G Xcode`
    * Open DDSP.xcodeproj, select a target and build.
* For release, we recommend building with [Ninja](https://ninja-build.org/) or CMake since building XNNPACK is unsupported on Xcode. Additionally, Ninja builds are faster compared to CMake.
    * `cmake -B build-ninja -S . -G Ninja`
    * Build all targets: `cmake --build build-ninja`
* Plugins will be copied to `~/Library/Audio/Plug-Ins` directory automatically post-build.

### Windows ###

* Generate Visual Studio 2022 solution:
    * `cmake -B build -G "Visual Studio 17 2022"`
* CMake adds `m.lib` as a dependency which causes a build error, this can be removed by running the following script after generating build files.
    * `./scripts/remove-m-lib-win.sh`
* You may encounter errors when building TFLite, they can be fixed by applying [this](https://stackoverflow.com/a/67374211) patch.
* Build the VST3 targets and copy plugin binaries to `C:\Program Files\Common Files\VST3`
* If the plugin UI looks blurred on Ableton, right click on the plug-in title bar and deselect "Auto-Scale Plugin Window". More information can be found [here](https://help.ableton.com/hc/en-us/articles/209775985-High-DPI-monitor-support)

### CMake ###

* Edit `cmake/FileList.cmake` to add new source files to the project.
* Compiler/linker options and project version can be found in `cmake/Config.cmake`.

## Contributing ##

We're eager to collaborate with you! Take a look at the [contribution guidelines](CONTRIBUTING.md) on how to contribute.

## Disclaimer ##

This is not an official Google product.
