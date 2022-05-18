## Context
There are many possible different permutations of machines, DAWs, and plugin builds, and we want to make sure our plugin works on as many configurations as possible.
If you manage to get your plugin to run succesfully, feel free to add a PR to this file! If DDSP.vst fails and crashes, even better: attach a pastebin containing the relevant error message(s) in the "Error Message" column, we can try to pinpoint the problem later. 

## Verified Builds

| Date    | Reporter    | Build #   | VST Deployment Target | OS + Version | Silicon | DAW + Version    | Successful Load | Error Message |
|---------|-------------|-----------|-------------------|--------------|---------|------------------|-----------------|---------------|
| 5/12/22 | wilzh40     | 1.0.0 | 10.11             | OSX 12.3.1   | M1      | FL Studio 20.9.0 | Y               | N/A           |
| 5/12/22 | wilzh40     | 1.0.0 | 10.11             | OSX 12.3.1   | M1      | Ableton 11.1.1   | Y               | N/A           |
| 5/12/22 | jesseengel  | 1.0.0 | 10.11             | OSX 12.3.1   | Intel   | Ableton 10.1.42  | Y               | N/A           |
| 5/17/22 | vvolhejn    | 1.0.0 | 10.11(?)          | OSX 10.15.7  | Intel   | Ableton 10.1.42  | N               | "Failed to create the Audio Unit "DDSP effect." This Audio Unit plug-in could not be opened."  [#5](https://github.com/magenta/ddsp-vst/issues/5)         |

