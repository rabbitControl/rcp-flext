# RabbitControl for Pd/Max

RabbitControl allows you to control parameters (values) in an easy way.

Expose parameters from Pd/Max and use a rcp client to remotely control your patch.  
Or use the rcp-client in Pd/Max to control parameters on a remote rcp-server.

#### More information:
[https://rabbitcontrol.cc](https://rabbitcontrol.cc)  
[https://github.com/rabbitcontrol/](https://rabbitcontrol.cc)


##Pd

Please find and install the external via deken (Find externals...) and load it as library `rcp`.


## Prebuilt Binaries

You can find prebuild binaries in [releases](./releases)

Bela-baord users please use: [RabbitControl-pd-1.0.1-linux-armv7l.zip](https://github.com/rabbitControl/rcp-flext/releases/download/v1.0.1/RabbitControl-pd-1.0.1-linux-armv7l.zip)

## Max

Until the package is available in the package-manager RabbitControl is available as a pre-built package available in [releases](./releases)

To use the package extract the zip-file to e.g.:  
`~/Max 8/Packages/`

###### known issues (Max only):
- help-patches don't open -> use RabbitControl Overview.maxpat or open the help-patches manually


## Building the external

In order to build Rabbitcontrol (rcp), a C++11 standard compliant compiler is required.

Clone this repository:

```
git clone --recurse-submodules https://github.com/rabbitControl/rcp-flext.git
```

### macOS / Linux

```
$ cd rcp-flext
$ cp Makefile_darwin_linux Makefile
$ make -j 8
```

### Windows mingw

Rename `Makefile_mingw` to `Makefile` and run `make` in a mingw-console

```
> make -j 8
```

### Cross compiling for bela

To cross compile for [bela](https://bela.io/), get the sysroot from here:  
[https://github.com/thetechnobear/xcBela](https://github.com/thetechnobear/xcBela)

`$ git clone https://github.com/thetechnobear/xcBela`

Rename `Makefile_bela` to `Makefile` and run `make`

##### Why?

Compiling rcp with SSL support on a Bela Mini fails due to too less memory.

One strategy is to cross-compile the classes, copy the object-files from 'sources' to your rcp-flext/sources folder on the bela-board and link on the bela-board running `make` - see how to compile for Linux.

##### Linking

To successfully link rcp on your host-machine additional liberaries are needed:
create the following directories:

```
...path/to/xcBela/sysroot/usr/local/lib
...path/to/xcBela/sysroot/usr/xenomai
```

then use rsync to get the libraries from the board:

```
rsync -av root@bela.local:/usr/local/lib ...path/to/xcBela/sysroot/usr/local/
rsync -av root@bela.local:/usr/xenomai/lib ...path/to/xcBela/sysroot/usr/xenomai/
```

Copy the resulting `rcp.pd_linux` to the bela-board.