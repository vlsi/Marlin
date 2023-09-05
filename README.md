# Прошивка для принтера FlyingBear Reborn 2 на основе Marlin

<p align="center">
    <a href="/LICENSE"><img alt="GPL-V3.0 License" src="https://img.shields.io/github/license/vlsi/reborn2-marlin.svg"></a>
    <a href="https://github.com/vlsi/reborn2-marlin/graphs/contributors"><img alt="Contributors" src="https://img.shields.io/github/contributors/vlsi/reborn2-marlin.svg"></a>
    <a href="https://github.com/vlsi/reborn2-marlin/releases"><img alt="Last Release Date" src="https://img.shields.io/github/release-date/vlsi/reborn2-marlin"></a>
    <a href="https://github.com/vlsi/reborn2-marlin/actions"><img alt="CI Status" src="https://github.com/vlsi/reborn2-marlin/actions/workflows/test-builds.yml/badge.svg"></a>
</p>

Additional documentation can be found at the [Marlin Home Page](https://marlinfw.org/).
Please test this firmware and let us know if it misbehaves in any way. Volunteers are standing by!

## О прошивке

Прошивка основана на форке Marlin 2.1.x, ветка `2.1.x` (т.е. последний стабильный релиз).
Прошивка подходит к принтерам FlyingBear Reborn 2.

Особенности:
* ColorUI
* Быстрая парковка: оси X и Y начинают парковаться одновременно
* С датчиком Z
  * `G34`: выравнивание платформы по Z через опускание до конца
  * `G35`: помощник регулировки стола на основе автоуровня
  * `G29`: построение сетки стола
* Без датчика Z
  * Парковка выравнивает Z оси по концевикам Z
  * `G35`: помощник регулировки стола, нужно калибровать стол на 0.1мм (~лист A4)
  * `G29`: построение сетки стола: нужно через меню двигать головку на каждую точку и регулировать стол (напр. на 0.1мм)
* Шаговые двигатели
  * `M114`: отладочная информация с драйверов шаговых двигателей TMC
  * Работа с двигателями по UART, поэтому двигатели тише, а их параметры (ток) можно менять из меню
* S-curve acceleration (более плавные ускорения и замедления, а не трапецевидные)
* Unified bed leveling (UBL), сетка 5x5
* Запуск обдува с толкача на низких скоростях: если указать обдув и 5%, то сначала будет 100мс импульс более сильного обдува, а потом уже 5% на время печати
* Поддерживаемые датчики Z:
  * Ручной
  * Индуктивный датчик
* Linear advance
* Input shaping

## Скачать прошивку

Прошивка рассчитана на плату MKS Robin Nano V3.2 и драйвера TMC2208 (2225) на осях XYZ и TMC2209 на экструдере.

Выпуски прошивки можно найти на странице с [релизами](https://github.com/vlsi/reborn2-marlin/releases).

**Все версии прошивки требуют перевода драйверов шаговых двигателей в UART**: https://telegra.ph/Reborn-UART-Drivers-01-02

* `reborn2_manual` -- "stock reborn2, но с UART драйверами". Версия подходит для ручной регулировки стола, без датчиков.
* `reborn2_inductive` -- для работы с индуктивным датчиком стола. Датчик подключается на разъём `ZMIN` (см инструкцию FlyingBear).
* `reborn2_3dtouch` -- пока нет.

## About

This is a fork of Marlin 2.1.x, optimized for the FlyingBear Reborn 2 3D printer.

Key features:
* ColorUI
* Quick home: X and Y axes start homing at the same time
* With Z probe
  * `G34`: autolevel the plane by moving Z to MAX (smashing the bed down)
  * `G35`: wizard for tuning the bed screws
  * `G29`: wizard for generating bed mesh
* Without Z probe
  * Homing aligns Z axes to Z endstops
  * `G35`: wizard for tuning bed screws. It moves nozzle to 0.1mm above the bed, so you can use 0.1mm gauge or A4 paper for tuning screws
  * `G29`: wizard for building the bed mesh. It will move the head to the appropriate XY position, and you'll need to move Z axis on the menu.
* Stepper motors
  * `M114`: TMC debug information
  * TMC drivers connect via UART, so they are silent, and the parameters (currents) can be tuned on the menu
* S-curve acceleration
* Unified bed leveling (UBL), 5x5 grid
* FAN kickstart when starting with low fan speeds
* Supported Z-sensors:
  * Manual
  * Inductive sensor
* Linear advance
* Input shaping

TODO:
* Print over Wi-Fi (support MKS Wi-Fi transfer)
* Load settings from a file on SD card (e.g. initially, and on-demand in the menu)

## Download

You can download the latest release from the [Releases page](https://github.com/vlsi/reborn2-marlin/releases)

## Building firmware

To build and upload Marlin you will use one of these tools:

- The free [Visual Studio Code](https://code.visualstudio.com/download) using the [Auto Build Marlin](https://marlinfw.org/docs/basics/auto_build_marlin.html) extension.
- The free [Arduino IDE](https://www.arduino.cc/en/main/software) : See [Building Marlin with Arduino](https://marlinfw.org/docs/basics/install_arduino.html)
- You can also use VSCode with devcontainer : See [Installing Marlin (VSCode devcontainer)](http://marlinfw.org/docs/basics/install_devcontainer_vscode.html).

Marlin is optimized to build with the **PlatformIO IDE** extension for **Visual Studio Code**. You can still build Marlin with **Arduino IDE**, and we hope to improve the Arduino build experience, but at this time PlatformIO is the better choice.

## Submitting Patches

Proposed patches should be submitted as a Pull Request against the ([vlsi-2.1.x](https://github.com/vlsi/reborn2-marlin/tree/vlsi-2.1.x)) branch.

- This branch is for fixing bugs and integrating any new features for the duration of the Marlin 2.1.x life-cycle.
- Follow the [Coding Standards](https://marlinfw.org/docs/development/coding_standards.html) to gain points with the maintainers.
- Please submit Feature Requests and Bug Reports to the [Issue Queue](https://github.com/vlsi/reborn2-marlin/issues/new/choose). Support resources are also listed there.
- Whenever you add new features, be sure to add tests to `buildroot/tests` and then run your tests locally, if possible.
  - It's optional: Running all the tests on Windows might take a long time, and they will run anyway on GitHub.
  - If you're running the tests on Linux (or on WSL with the code on a Linux volume) the speed is much faster.
  - You can use `make tests-all-local` or `make tests-single-local TEST_TARGET=...`.
  - If you prefer Docker you can use `make tests-all-local-docker` or `make tests-all-local-docker TEST_TARGET=...`.

## Contributors

Marlin is constantly improving thanks to a huge number of contributors from all over the world bringing their specialties and talents. Huge thanks are due to [all the contributors](https://github.com/vlsi/reborn2-marlin/graphs/contributors) who regularly patch up bugs, help direct traffic, and basically keep Marlin from falling apart. Marlin's continued existence would not be possible without them.

## License

Marlin is published under the [GPL license](/LICENSE) because we believe in open development. The GPL comes with both rights and obligations. Whether you use Marlin firmware as the driver for your open or closed-source product, you must keep Marlin open, and you must provide your compatible Marlin source code to end users upon request. The most straightforward way to comply with the Marlin license is to make a fork of Marlin on Github, perform your modifications, and direct users to your modified fork.

While we can't prevent the use of this code in products (3D printers, CNC, etc.) that are closed source or crippled by a patent, we would prefer that you choose another firmware or, better yet, make your own.
