<div align="center">
<h1>Gpay Country Spoofer</h1>

![downloads](https://img.shields.io/github/downloads/BlueCat300/GpayCountrySpoofer)
[![GitHub release](https://img.shields.io/github/v/release/BlueCat300/GpayCountrySpoofer)](https://github.com/Xposed-Modules-Repo/ru.bluecat.novpndetectenhanced/releases)
[![GitHub Release Date](https://img.shields.io/github/release-date/BlueCat300/GpayCountrySpoofer)](https://github.com/Xposed-Modules-Repo/ru.bluecat.novpndetectenhanced/releases)
[![Telegram](https://img.shields.io/badge/Telegram-Channel-blue.svg?logo=telegram)](https://t.me/lsposed_workshop)
</div>

### Description:
This module is designed to hook SIM card geo, restoring Google Pay on devices using unsupported SIM cards. It employs techniques from [PlayIntegrityFork](https://github.com/osm0sis/PlayIntegrityFork) to spoof the MCC, MNC, ISO, Operator Name.

### Attention:
- The module performs location spoofing exclusively for Google Play Services.
- You still need a foreign SIM card to register a bank card, as additional data is verified during registration.
- The module only spoofs geolocation data; it does not handle Play Integrity certification or other checks.
- This module is an experimental project and has not been tested on an actual payment terminal.


### Usage:
By default, module uses Georgia.

You can switch to the carrier details of another country. Go to the module folder and edit the MCC, MNC, ISO, NAME in the config.conf file. Restart the Google Play Services process.
You can find the necessary carrier information on this [website](https://en.wikipedia.org/wiki/Mobile_country_code).

