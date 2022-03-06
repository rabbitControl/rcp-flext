# RabbitControl for Pd/Max

RabbitControl allows you to control parameters (values) in an easy way.

Expose parameters from Pd/Max and use a rcp client to remotely control your patch.  
Or use the rcp-client in Pd/Max to control parameters on a remote rcp-server.

##### More information:
[https://rabbitcontrol.cc](https://rabbitcontrol.cc)  
[https://github.com/rabbitcontrol/](https://rabbitcontrol.cc)


# Building

## Requirements

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
Rename `Makefile_mingw` to `Makefile` and run make in a mingw-console

```
> make -j 8
```