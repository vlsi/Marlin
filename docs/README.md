# Прошивка для FlyingBear Reborn 2

Прошивка требует переведения драйверов шаговых двигателей в UART режим.
Перевод в UART позволяет настраивать параметры (ток, режим работы) программно.
Инструкция перевода в UART режим: https://telegra.ph/Reborn-UART-Drivers-01-02

После прошивки желательно сбросить EEPROM (настройки -> конфигурация -> расширенные -> сброс EEPROM)

Варианты прошивки

* `reborn2_manual` -- "stock reborn2, но с UART драйверами". Версия подходит для ручной регулировки стола, без датчиков.
* `reborn2_inductive_sensor_zmin` -- для работы с индуктивным датчиком стола. Датчик подключается на разъём `ZMIN` (см инструкцию FlyingBear).
* `reborn2_inductive_sensor_runout2` -- для работы с индуктивным датчиком стола. Датчик подключается на разъём `RUNOUT2` (PA4), обычные концевики **должны быть подключены**.
* `reborn2_bltouch_sensor_zmin` -- для работы с BLTouch датчиком стола. Датчик подключается на разъём `ZMIN` (см инструкцию FlyingBear).
* `reborn2_bltouch_sensor_runout2` -- для работы с BLTouch датчиком стола. Датчик подключается на разъём `RUNOUT2` (PA4), обычные концевики **должны быть подключены**.

Более подробное описание можно найти на
* https://github.com/vlsi/reborn2-marlin
* https://github.com/vlsi/reborn2-marlin/wiki

# FlyingBear Reborn 2 firmware

See details at
* https://github.com/vlsi/reborn2-marlin
* https://github.com/vlsi/reborn2-marlin/wiki

# Автор

Vladimir Sitnikov <sitnikov.vladimir@gmail.com>
