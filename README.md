# Pixl.js Games Firmware Branch

This branch is an unsupported build branch for users who still want the old built-in Games app.

The official firmware does not include built-in games anymore. The games were removed from the main firmware line because of firmware size and maintenance trade-offs, and there is no plan to add them back to regular releases. This branch keeps a buildable games variant for advanced users who are comfortable compiling firmware themselves.

## Branch Status

- Base release: `2.16.0`
- Target users: advanced users building custom firmware
- Maintenance: best-effort only, not continuously maintained with every main firmware change
- Official releases: use the main release packages, not this branch
- Games included: Tiny Invaders, Tiny Lander, Tiny Tris, Tiny Arkanoid

Future firmware features may make the games build too large to fit without removing something else. If that happens, prefer the official main firmware unless you are comfortable resolving firmware-size trade-offs yourself.

## Build With Docker

Install Docker first, then build from a clean checkout:

```sh
git clone https://github.com/solosky/pixl.js
cd pixl.js
git checkout game
git submodule update --init --recursive
```

Start the Nordic SDK build container with the repository mounted:

```sh
docker run -it --rm \
  -v "$PWD:/builds/pixl.js" \
  -w /builds/pixl.js \
  solosky/nrf52-sdk:latest \
  bash
```

Inside the container, build the firmware variant for your hardware:

```sh
# LCD hardware
cd fw
make all BOARD=LCD RELEASE=1
```

```sh
# OLED hardware
cd fw
make all BOARD=OLED RELEASE=1
```

The generated files are written under `fw/_build/`, including:

- `pixljs.hex`
- `pixljs_all.hex`
- `pixjs_ota_v*.zip`

Use the firmware package that matches your hardware variant. Flashing the wrong LCD/OLED firmware can make recovery harder.

## Notes

- The Games app is enabled in both LCD and OLED board configs on this branch.
- The games were originally ported from [wagiminator/CH32V003-GameConsole](https://github.com/wagiminator/CH32V003-GameConsole).
- This branch exists for custom builds only; issues in the games branch may not be prioritized with the main firmware.

## Main Documentation

- [Chinese Documentation](docs/zh/README.md)
- [English Documentation](docs/en/README.md)
- [Italian Documentation](docs/it/README.md)

## License

This project is released under GPL 2.0. If you modify and publish the firmware, publish your modified source under the same license.
