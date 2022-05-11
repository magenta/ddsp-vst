<div align="center">
    <img src="https://storage.googleapis.com/ddsp/github_images/ddsp_logo.png" width="200px" alt="logo"></img>
</div>

# DDSP VST # 

VST/AU plugins and desktop applications built using the [JUCE](https://juce.com/) framework and [DDSP](https://github.com/magenta/ddsp) models.

<div align="center">
    <img width="400" alt="DDSP Effect" src="https://user-images.githubusercontent.com/7446124/167872854-ba8ddf52-e27f-4563-8d92-8e2da87573d4.png">
    <img width="400" alt="DDSP Synth" src="https://user-images.githubusercontent.com/7446124/167882854-4d15a746-1d01-4634-877e-afe4f90710f2.png">

</div>



## Helpful Links

* [Project Page](https://g.co/magenta/ddsp-vst)
* [Train a new model](https://g.co/magenta/train-ddsp-vst)
* [Download the latest plugin (OSX)](https://g.co/magenta/ddsp-vst-mac)
* [Join our DDSP community!](https://discord.gg/eyzhzMJMx5)

# Get started!

### Installation (OSX)

1. Download the zipped binaries from the [link above](https://g.co/magenta/ddsp-vst-mac).
2. Unzip and copy the contents to `/Library/Audio/Plug-Ins/Components`. There should be two plugins.
3. Rescan for AudioUnit(s) in your respective DAW (verified on Ableton/Logic Pro/FL Studio/Reaper).
4. Use (and/or abuse) your instruments in real-time!

### Using your plugin

![Help](Explain_DDSP-01.png)

### Training your own model
<div align="center">
<img width="1228" alt="image" src="https://user-images.githubusercontent.com/7446124/167896218-0245e8b5-4808-4080-83e6-2bafd3a39c51.png">
</div>
DDSP currently comes with _11_ embedded models. If you want to train and use your own model, follow these instructions:

1. Procure a 10-20 minute file of a _monophonic_ (one note at a time) recording of an instrument you want to train. 
2. Follow the directions in the [training colab](https://g.co/magenta/train-ddsp-vst) and download the trained model.
4. Click the "Folder" icon within the plugin and drag your trained model into the same directory.
5. Click the "Refresh" icon within the plugin for DDSP-VST to pick up your new model.

## Where's the code? ##

_Soon™._

Sorry for the inconvenience! We built this open-source project with the intention of it being a community effort: collecting feedback, sharing fun models and novel sounds, iterating on features, and joining forces to make this a better plugin _together_.  
 
That's why we're taking a bit more time to make sure the code is in a good enough place to share with the rest of the community; we have just a few loose ends to tie up to make it accessible and useful for everyone.  

In the meantime, you can download the codesigned binary from the links above. Please feel empowered join our Discord channel to share your experiences and to use this repo to file bugs/issues/feature requests while we get the code up and ready for public consumption. Thanks for your patience!
