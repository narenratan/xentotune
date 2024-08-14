# Xentotune

A microtonal autotuner plugin.

Uses the [Signalsmith Stretch](https://github.com/Signalsmith-Audio/signalsmith-stretch)
library to do the pitch shifting.

Uses [MTS-ESP](https://github.com/ODDSound/MTS-ESP) to pull the tuning.

## Build

```console
    $ git clone --recurse-submodules https://github.com/narenratan/xentotune
    $ cd xentotune
    $ cmake -B build -DCMAKE_BUILD_TYPE=Release
    $ cmake --build build --config Release
```
