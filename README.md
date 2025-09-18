# Smol Pong Game

Make sure to clone this repository recursively!
```
git clone https://github.com/L2-CS-Suffering-gang/smol-untitled-pong-game --recursive
```

If you have cloned it non-recursively:
```
git submodule update --init
```

## TODO (in order)
- [x] initial concept
- [ ] 1v1 (offline)
- [ ] powerups, obstacles???, etc.
- [ ] multiplayer - networking
- [ ] 10v10, battle royale, more game modes
- [ ] COMPLETE - READY TO SHIP ON STEAM (LMAO)

## Preview
![preview](./assets/preview.gif)


## Building and running

### Linux, Mac OS, MSYS (Windows)

```sh
./build.sh
```

```
./build/pong audio-file background-image osu-file
```

```
./build/pong "./data/yoru/audio.mp3" "./data/yoru/bg.png" "./data/yoru/YOASOBI - Yoru ni Kakeru (9ami) [2lewd's Insane].osu"
```

### Windows - Visual Studio

```bat
.\build.bat
```

## Contributing

See style guide [here](./docs/STYLEGUIDE.md)
