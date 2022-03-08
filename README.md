## Redink

A [redis] driven E-Ink Display thing.

### Components

- [ItsyBitsy M4](https://www.adafruit.com/product/3800)
- [Adafruit AirLift](https://www.adafruit.com/product/4201)
- [2.9" E-ink Monochrome display](https://www.adafruit.com/product/4262)

### Directories

```
/scripts        <- useful build scripts (wraps 'arduino-cli')
/src/redink     <- the arduino firmware for the ItsyBitsy
/src/redink-srv <- the rust webserver for pushing messages.
/src/redink-ui  <- an emberjs frontend for sending messages.
```

#### Development: Arduino/Firmware

The easiest way to build and flash the arduino source code onto the ItsyBitsy is to use the scripts found in the
`scripts` directory that wrap [arduino-cli].

Before building, create an `env.h` with the appropriate values for your wifi and redis settings:

```
$ cat env.h
#ifndef _ENV_H
#define _ENV_H 1

#define REDINK_REDIS_HOST "0.0.0.0"
#define REDINK_REDIS_PORT 6397

#define REDINK_WIFI_SSID     ""
#define REDINK_WIFI_PASSWORD ""

#endif
$ ./script/build-firmware.sh /dev/ttyAM04
```

Note: the build script _optionally_ accepts the address of a connected ItsyBitsy, which will attempt to upload
the compiled source using `arduino-cli upload` if provided. This address can be determined using:

```
$ arduino-cli board list
```

#### Development: EmberJS Frontend

The web ui (html, css + javascript) frontend lives in the `src/redink-ui` directory. This is an [emberjs] application
which requires [nodejs] to compile. The application can be started using a proxy (for handling passing xhr requests
to the rust web api) using:

```
$ cd src/redink-ui
$ npm i
$ npm run start:proxy
```

#### Development: Rust Web API

The [rust]/[tide] web server lives in `src/redink-srv` and can be compiled + run using:

```
$ cd src/redink-srv
$ cat .env
RUST_LOG=info

REDINK_ADDR=0.0.0.0:8081
REDIS_ADDR=0.0.0.0:6379
REDIS_MESSAGE_QUEUE=redink:messages
$ cargo run
```

----

| :video_camera: |
| --- |
| ![IMG_0696](https://user-images.githubusercontent.com/1545348/157158916-bed1ecfa-6968-49d5-8718-632c23f21276.gif) |


[arduino-cli]: https://github.com/arduino/arduino-cli
[emberjs]: https://emberjs.com/
[nodejs]: https://nodejs.org/en/
[tide]: https://github.com/http-rs/tide
[rust]: https://www.rust-lang.org/
[redis]: https://redis.io/
