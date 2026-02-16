## Per-Country or Per-device settings

Per-Country settings can be provided by:

- *UWB_COUNTRY_CODE_CAPS*
- or Extra calibrations files.

If the same parameter was specified by both *UWB_COUNTRY_CODE_CAPS* and
Extra calibration files, *UWB_COUNTRY_CODE_CAPS* has higher priority.
For example, if *UWB_COUNTRY_CODE_CAPS* has 0x5 (Tx power) entry, *cal.antX.chX.tx_power* from extra calibration file is ignored.

### UWB_COUNTRY_CODE_CAPS

This is for per-country device settings and calibrations.

Multiple *UWB_COUNTRY_CODE_CAPS* files can define this values.
Only one file with highest *VERSION* will be picked by HAL. And one parameter holds settings for each country code.

Directories contain *UWB_COUNTRY_CODE_CAPS* are listed by *COUNTRY_CODE_CAP_FILE_LOCATION*
in the main configuration file.

Example:

``````
# /vendor/etc/libuwb-nxp.conf:

COUNTRY_CODE_CAP_FILE_LOCATION={  "/vendor/etc/uwb/", "/data/vendor/uwb/" }
``````

From the above example, `/vendor/etc/uwb/libuwb-countrycode.conf` and
`/data/vendor/uwb/libuwb-country.conf` will be looked up by HAL and only one file with higher VERSION will be choosen.

* *UWB_COUNTRY_CODE_CAPS*
  * First octec has number of TLV entries and TLVs are followed.
  * TLV types:
     * 00 : country code in two letters
     * 01 : 0=Disable UWB, 1=Enable UWB(default)
     * 02 : 0=Disable Channel 5, 1=Enable Channel 5 (default)
     * 03 : 0=Disable Channel 9, 1=Enable Channel 9 (default)
     * 05 : Tx power adjustment values in 2 octects
* *VERSION*
  * string formatted number

Example:

```
# /data/vendor/uwb/libuwb-countrycode.conf:

VERSION="02"
UWB_COUNTRY_CODE_CAPS={
    03,                 #  3 countries
    00, 02, 52, 55,     # 'RU'
    03, 01, 00,         # Disable Channel 9
    05, 02, 81, 05,     # TX_POWER = {81, 05}

    00, 02, 55, 41,     # 'UA'
    01, 01, 00,         # Disable UWB at all

    00, 02, 4b, 52,     # 'KR'
    02, 01, 00,         # Disable Channel 5
    05, 02, 00, 00,     # TX_POWER = {00, 00}
}
```

### Extra calibrations files

Main configuration can specifies additional extra calibrations with *EXTRA_CONF_PATH_[1..10]* parameters.

* *EXTRA_CONF_PATH[N+1]* has higher priority over *EXTRA_CONF_PATH[N]*.
* if the file path has `<country>` in it, `<country>` part will be replaced with country code (or region string)
* if the file path has `<sku>` in it, `<sku>` part will be replace with the 'persist.vendor.uwb.cal.sku' property value.
  if `persist.vendor.uwb.cal.sku` is unspecified, HAL will try to use `defaultsku` as a default.

Example:

```
# /vendor/etc/libuwb-nxp.conf:

EXTRA_CONF_PATH_1="/vendor/etc/uwb/cal-base.conf"
EXTRA_CONF_PATH_2="/vendor/etc/uwb/cal-<sku>.conf"
EXTRA_CONF_PATH_3="/vendor/etc/uwb/cal-<country>.conf"
EXTRA_CONF_PATH_4="/mnt/vendor/persist/uwb/cal-factory.conf"
```

#### Region mapping

To reduce the duplicated settings between countries, multiple country codes can be grouped into region string. If main configuration file has *REGION_MAP_PATH*, HAL will load region mapping file and use it for region mapping to country codes.

Example:

```
# /vendor/etc/libuwb-nxp.conf:
REGION_MAP_PATH="/vendor/etc/uwb/regions.conf"

# /vendor/etc/uwb/regions.conf:
FCC="US CA"
```

In the above example, 'US' and 'CA' country codes are grouped into 'FCC' region string.

When the system provides country code with 'US', `EXTRA_CONF_PATH_2=/vendor/etc/uwb/cal-<country>.conf*` will be evaluated as `/vendor/etc/uwb/cal-FCC.conf` instead of `/vendor/etc/uwb/cal-US.conf` and `cal-FCC.conf` will be loaded.

#### Parameters

##### *cal.uwb_disable`=<1|0>`*

Per-country, Disable UWB RF when it's set to 1. Default is 0.

##### *cal.rx_antenna_mask*`=<8bit unsigned>`, *cal.tx_antenna_mask*`=<8bit unsigned>`

1 octect, Antenna IDs defined by the device. b0=Antenna-ID 1, b1=Antenna-ID 2, ...

e.g. `cal.rx_antenna_mask=0x3` means this paltform has two RX antennas with ID 1 and 2.

##### *cal.restricted_channels*`=<16bit unsigned>`

Per country, Restricted channel mask, b0=Channel0, b1=Channel1, ... b15=Channel15.
e.g. `cal.restricted_channels=0x20` to deactivate channel 5.

##### *cal.otp.xtal*`=<1|0>`

Load the Crystal calibration value from OTP when it's 1. *cal.xtal* will be ignored.

##### *cal.xtal*`=<byte array>`

6 bytes Crystal calibration values (little endian)

- [5:4] : CAP1
- [3:2] : CAP2
- [1:0] : GM CURRENT CONTROL

e.g. `cal.xtal={11 00 11 00 3f 00}`

##### *cal.ant`<antenna-id>`.ch`<channel-number>`.ant_delay*`=<16bit unsigned>`

Per-country, RX antenna delay value in Q14.2. e.g. `cal.ant1.ch5.ant_delay=2000`

##### *cal.ant`<antenna-id>`.ch`<channel-number>`.tx_power*`=<byte array>`

Per-country, 4 bytes TX Power value.

- [3:2] : Delta peak
- [1:0] : ID RMS

  e.g. `cal.ant1.ch5.tx_power={01 00 00 00}`

##### *cal.tx_pulse_shape*

Per-country, Byte array for TX pulse shape data

##### *cal.ddfs_enable`=<1|0>`*

Per-country, Enable DDFS tone generation. Default is 0.

##### *cal.dc_suppress`=<1|0>`*

Per-country, Enable DC supression. Default is 0.

#### Example configuration files
```
# /vendor/etc/libuwb-nxp.conf:
...
REGION_MAP_PATH="/vendor/etc/uwb/regions.conf"

EXTRA_CONF_PATH_1="/vendor/etc/uwb/cal-base.conf"
EXTRA_CONF_PATH_2="/vendor/etc/uwb/cal-<sku>.conf"
EXTRA_CONF_PATH_3="/vendor/etc/uwb/cal-<country>.conf"
EXTRA_CONF_PATH_4="/mnt/vendor/persist/uwb/cal-factory.conf"

# /vendor/etc/uwb/cal-base.conf:
cal.rx_antenna_mask=0x03
cal.tx_antenna_mask=0x01
cal.otp.xtal=1
cal.restricted_channels=0

# /vendor/etc/uwb/cal-defaultsku.conf:
# effective when persist.vendor.uwb.cal.sku is unspecified
cal.ant1.ch5.tx_power={01, 00, 02, 00}
cal.ant1.ch9.tx_power={01, 00, 03, 00}

# /vendor/etc/uwb/cal-modelA.conf:
# effective when persist.vendor.uwb.cal.sku=modelA
cal.ant1.ch5.tx_power={01, 00, 12, 00}
cal.ant1.ch9.tx_power={01, 00, 13, 00}

# /vendor/etc/uwb/cal-FCC.conf:
cal.restricted_channel=0x0

# /vendor/etc/uwb/cal-JP.conf:
cal.restricted_channel=0x20
cal.ddfs_enable=1
cal.ddfs_tone_config={...}

# /vendor/etc/uwb/cal-KR.conf:
cal.restricted_channel=0x20

# /vendor/etc/uwb/cal-TW.conf:
cal.restricted_channel=0x20

# /vendor/etc/uwb/cal-RESTRICTED.conf:
cal.uwb_disable=1
cal.restricted_channels=0xffff

# /vendor/etc/uwb/regions.conf:
CE="AT BE BG CH CY CZ DE DK EE ES FI FR GB GR HR HU IE IS IT LI LV LT LU MT NI NL NO PL PT RO SE SK SI"
FCC="US CA"
RESTRICTED="AR AM AZ BY ID KZ KG NP PK PY RU SB TJ TM UA UZ"
```
