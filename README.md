# FROM BELOW

A brand new game for the Nintendo Entertainment System!

Details on the game, along with downloads of compiled ROM can be found here: https://mhughson.itch.io/from-below

# COMPILING

Everything you need to build the ROM from scratch can be found in the repro.

On Windows, using VSCode use the following steps:

1) Open mbh-firstnes folder in VS Code.
2) Press CTRL+SHIFT+B to open the "Build" dropdown.
3) Choose BUILD ALL. This will build the entire game and output a .nes file.
4) In windows explorer navigate to ./mbh-firstgame/game/BUILD/
5) You should find main.nes which is the compiled ROM.

NOTE: If you have .nes files associated with an emulator (so that double-clicking them automatically opens them) you can use BUILD ALL & RUN instead, which will automatically open the ROM in your default emulator automatically.

On non-windows machines please see https://github.com/mhughson/mbh-firstnes/blob/master/game/Makefile. I don't actually use this myself; it was contributed by @teleozoic.

# SOURCE

All of the source for the game is found in [here](game/). The 2 files you will be most interested in are [main.h](game/main.h) and [main.c](game/main.c) which contain all the logic for the game.
