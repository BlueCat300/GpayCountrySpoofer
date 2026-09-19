<div align="center">
<h3><a href="https://github.com/BlueCat300/GpayCountrySpoofer/blob/master/README_en.md">English</a></h3>
<h1>Gpay Country Spoofer</h1>

![downloads](https://img.shields.io/github/downloads/BlueCat300/GpayCountrySpoofer)
[![GitHub release](https://img.shields.io/github/v/release/BlueCat300/GpayCountrySpoofer)](https://github.com/Xposed-Modules-Repo/ru.bluecat.novpndetectenhanced/releases)
[![GitHub Release Date](https://img.shields.io/github/release-date/BlueCat300/GpayCountrySpoofer)](https://github.com/Xposed-Modules-Repo/ru.bluecat.novpndetectenhanced/releases)
[![Telegram](https://img.shields.io/badge/Telegram-Channel-blue.svg?logo=telegram)](https://t.me/lsposed_workshop)
[![Telegram Group](https://img.shields.io/badge/Telegram-Group-blue.svg?logo=telegram)](https://t.me/lsposed_workshop_forum)
[![Donate](https://img.shields.io/badge/Donate_Form-blue)](https://pay.cloudtips.ru/p/85f8cf00)
</div>

### Описание:
Модуль предназначен для перехвата гео данных SIM карты для работы Google Pay на устройствах с включёнными российскими SIM картами. Модуль использует методики [PlayIntegrityFork](https://github.com/osm0sis/PlayIntegrityFork) для подмены MCC, MNC, ISO, Operator Name.

### Обратите внимание:
- Модуль выполняет точечную замену исключительно в сервисах Google Play.
- Вам всё ещё нужна иностранная SIM карта для регистрации банковской карты т.к. при регистрации проверяется больше данных.
- Модуль подменяет только гео данные, он не отвечает за Play Integrity сертификацию и другие проверки.
- Этот модуль является экспериментальным проектом и не проверялся на реальном платёжном терминале.

### Использование:
По умолчанию модуль использует Грузию.

Вы можете изменить на данные оператора другой страны. Зайдите в папку модуля и в файле config.conf измените MCC, MNC, ISO, NAME. Перезапустите процесс сервисов Google Play.
Необходимые данные операторов связи можно найти на этом [сайте](https://en.wikipedia.org/wiki/Mobile_country_code).