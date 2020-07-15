STM32MP157C RPMSG test tool
----

[![dimtass](https://circleci.com/gh/dimtass/stm32mp1-cmake-rpmsg-test.svg?style=svg)](https://circleci.com/gh/dimtass/stm32mp1-cmake-rpmsg-test)

This code is based on the cmake template for STM32MP157C which is located [here](https://github.com/dimtass/stm32mp1-cmake-template).

This repo contains the source code of the firwmare for the CM4 MPU on the STM32MP1
and a Linux tool for the CA CPU. Both are using OpenAMP to transfer data between
the MPU and CPU via the virtual UART/TTY.

> Note: There is a blog post here which explains how to use this test [here](https://www.stupid-projects.com/?p=881&preview=true).

## Build the CM firmware
To build the firmware you need to clone the repo in any directory and then inside
that directrory run the command:

```sh
SRC=src_hal ./build.sh
```

The above command assumes that you have a toolchain in your `/opt` folder. In case,
you want to point to a specific toolchain path, then run:

```sh
TOOLCHAIN_DIR=/path/to/toolchain SRC=src_hal ./build.sh
```

Or you can edit the `build.sh` script and add your toolchain path.

It's better to use Docker to build the image. To do that run this command:
```sh
docker run --rm -it -v $(pwd):/tmp -w=/tmp dimtass/stm32-cde-image:latest -c "SRC=src_hal ./build.sh"
```

In order to remove any previous builds, then run:
```sh
docker run --rm -it -v $(pwd):/tmp -w=/tmp dimtass/stm32-cde-image:latest -c "CLEANBUILD=true SRC=src_hal ./build.sh"
```

## Build the CA tool
The CA tool is located in the `CA-source` folder and it's a cmake project. You can build it
on you x86_64 host, but that doesn't make much sense. Therefore, you need to cross-compile
the tool. I'm using a Yocto SDK that I built my self for testing, but there's also a recipe
[here]() that you can use bitbake to built it. In case you use the Yocto SDK, then you need
to source the SDK environment and then build like this:

```sh
cd CA-source
mkdir build-armhf
cd build-armhf
source /opt/st/stm32mp1-discotest/3.1-snapshot/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi 
cmake ..
make
```

Then you need to copy the executable to your `/home/root` folder and run it like this:
```sh
./tty-test-client /dev/ttyRPMSG0
```

This is a sample of the output:

```
- 15:43:54.953 INFO: Application started
- 15:43:54.954 INFO: Connected to /dev/ttyRPMSG0
- 15:43:54.962 INFO: Initialized buffer with CRC16: 0x1818
- 15:43:54.962 INFO: ---- Creating tests ----
- 15:43:54.962 INFO: -> Add test: block=512, blocks: 1
- 15:43:54.962 INFO: -> Add test: block=512, blocks: 2
- 15:43:54.962 INFO: -> Add test: block=512, blocks: 4
- 15:43:54.963 INFO: -> Add test: block=512, blocks: 8
- 15:43:54.963 INFO: -> Add test: block=1024, blocks: 1
- 15:43:54.964 INFO: -> Add test: block=1024, blocks: 2
- 15:43:54.964 INFO: -> Add test: block=1024, blocks: 4
- 15:43:54.964 INFO: -> Add test: block=1024, blocks: 5
- 15:43:54.964 INFO: -> Add test: block=2048, blocks: 1
- 15:43:54.964 INFO: -> Add test: block=2048, blocks: 2
- 15:43:54.964 INFO: -> Add test: block=4096, blocks: 1
- 15:43:54.964 INFO: ---- Starting tests ----
- 15:43:54.977 INFO: -> b: 512, n:1, nsec: 11970765, bytes sent: 512
- 15:43:54.996 INFO: -> b: 512, n:2, nsec: 18770380, bytes sent: 1024
- 15:43:55.027 INFO: -> b: 512, n:4, nsec: 31022063, bytes sent: 2048
- 15:43:55.083 INFO: -> b: 512, n:8, nsec: 56251848, bytes sent: 4096
- 15:43:55.099 INFO: -> b: 1024, n:1, nsec: 15322572, bytes sent: 1024
- 15:43:55.124 INFO: -> b: 1024, n:2, nsec: 24824116, bytes sent: 2048
- 15:43:55.168 INFO: -> b: 1024, n:4, nsec: 43830248, bytes sent: 4096
- 15:43:55.221 INFO: -> b: 1024, n:5, nsec: 53327292, bytes sent: 5120
- 15:43:55.243 INFO: -> b: 2048, n:1, nsec: 21742144, bytes sent: 2048
- 15:43:55.281 INFO: -> b: 2048, n:2, nsec: 37633885, bytes sent: 4096
- 15:43:55.318 INFO: -> b: 4096, n:1, nsec: 37649011, bytes sent: 4096
```

In the last rows `b` is the block size, `n` is the number of blocks and `nsec` is the number
of nsecs that the transfer lasted. The timer used for the benchmark is running on the Linux
side, so there it might not be very accurate, but that doesn't really matter as the time the
transactions take are in the range of msecs.

> Note: For some reason when sending more than 5KB the virtual TTY in the Linux side hangs.
It doesn't matter if the block size is small or large, as long a single transaction sends more
than 5KB the issue occures.

## Loading the firmware to CM4
To load the firmware on the Cortex-M4 MCU you need to scp the firmware `.elf` file in the
`/lib/firmware` folder of the Linux instance of the STM32MP1. Then you also need to copy the
`fw_cortex_m4.sh` script on the `/home/root` (or anywhere you like) and then run this command
as root.
```sh
./fw_cortex_m4.sh start
```

To stop the firmware run:
```sh
./fw_cortex_m4.sh stop
```

> Note: The console of the STM32MP1 is routed in the micro-USB connector `STLINK CN11` which
in case of my Ubuntu shows up as `/dev/ttyACMx`.

When you copy the `./fw_cortex_m4.sh` you need also to enable the execution flag with:
```sh
chmod +x fw_cortex_m4.sh
```

If the firmware is loaded without problem you should see an output like this:
```sh
fw_cortex_m4.sh: fmw_name=stm32mp157c-cmake-template.elf
[  162.549297] remoteproc remoteproc0: powering up m4
[  162.588367] remoteproc remoteproc0: Booting fw image stm32mp157c-cmake-template.elf, size 704924
[  162.596199]  mlahb:m4@10000000#vdev0buffer: assigned reserved memory node vdev0buffer@10042000
[  162.607353] virtio_rpmsg_bus virtio0: rpmsg host is online
[  162.615159]  mlahb:m4@10000000#vdev0buffer: registered virtio0 (type 7)
[  162.620334] virtio_rpmsg_bus virtio0: creating channel rpmsg-tty-channel addr 0x0
[  162.622155] rpmsg_tty virtio0.rpmsg-tty-channel.-1.0: new channel: 0x400 -> 0x0 : ttyRPMSG0
[  162.633298] remoteproc remoteproc0: remote processor m4 is now up
[  162.648221] virtio_rpmsg_bus virtio0: creating channel rpmsg-tty-channel addr 0x1
[  162.671119] rpmsg_tty virtio0.rpmsg-tty-channel.-1.1: new channel: 0x401 -> 0x1 : ttyRPMSG1
 ```

This means that the firmware is loaded and the virtual tty port is mapped.

## Testing the firmware
When this example firmware loads then two new tty ports will be created in the Linux side,
which are `/dev/ttyRPMSG0` and `/dev/ttyRPMSG1`. Now to test that the firmware is working
properly run these commands on the Linux terminal.

```sh
stty -onlcr -echo -F /dev/ttyRPMSG0
cat /dev/ttyRPMSG0 &
stty -onlcr -echo -F /dev/ttyRPMSG1
cat /dev/ttyRPMSG1 &
echo "Hello Virtual UART0" >/dev/ttyRPMSG0
echo "Hello Virtual UART1" >/dev/ttyRPMSG1
```

You should see the same strings received back (echo).

## Debug serial port
The firmware also supports a debug UART on the CM4. This port is mapped to UART7 and the
Arduino connector pins. The pinmap is the following:

pin | Function
-|-
D0 | Rx
D1 | Tx

You can connect a USB-to-UART module to those pins and the GND and then open the tty port
on your host. The port supports 115200 baudrate. When the firmware loads on the CM4 then
you should see this messages:

```sh
[00000.008][INFO ]Cortex-M4 boot successful with STM32Cube FW version: v1.2.0
[00000.015][INFO ]Virtual UART0 OpenAMP-rpmsg channel creation
[00000.021][INFO ]Virtual UART1 OpenAMP-rpmsg channel creation
```

## Using the cmake template in Yocto
TBD

## License
Just MIT.

## Author
Dimitris Tassopoulos <dimtass@gmail.com>