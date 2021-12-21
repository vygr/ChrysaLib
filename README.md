# ChrysaLib

C++ ChrysaLib !

A version of ChrysaLisp system concepts written in C++.

## Dependencies

### Mac via Brew

`brew install libusb asio`

### Linux via apt-get

`sudo apt-get install libusb-dev libasio-dev`

### Windows via Googling

Download libusb v1.0.24 from https://github.com/libusb/libusb/releases and asio
1.20.0 (not the boost version) from https://think-async.com/Asio/

`libusb`

Add the include and lib paths for libusb to your visual studio project and add
libusb-1.0.lib as an additional linker dependency.

`asio`

Add the include path to asio.hpp to your visual studio project there are no
link dependencies as including the header also brings in any necessary
libraries.

Add the following preprocessor definitions:

ASIO_STANDALONE;BOOST_ALL_NO_LIB;BOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING;
ASIO_WINDOWS;ASIO_MSVC

## Make

Make with:

```text
make -j
```

## Clean

Clean with:

```text
make clean
```

## Run

Run with:

```text
hub [switches] ip_addr ...
eg. hub -t 10000 -usb -ip 192.168.0.64 192.168.0.65
-h:    this help info
-t ms: exit timeout, default 0, ie never
-usb:  start the usb link manager
-ip:   start the ip link manager server
```

## Usage

So what is it ? How would I use it ? Is this all your going to provide ?

This is the lowest layer of the ChrysaLisp messaging and services system
implemented in C++. You can use this to create applications and services in C++
that can be wired together via USB or IP links.

Applications and services will automatically be kept informed as other peers
join and leave the network. Whatever services these peers advertise will become
available to use and you talk to them via the messaging system.

More examples will be forthcoming, but a simple File_Service example source is
provided to give the flavour of what you might build as a service, but clearly
whatever you can dream up.

DNS resolution for easier setup will follow soon as well as some auto wiring
aid using broadcasts may be possible.

### Likely configuration

While you could use the code to construct an arbitrary setup of what process
runs what services there are some practical matters to consider about who can
own USB handles and such that make me suggest the following.

Run a `hub` on each machine and wire them to other peers `hubs` with:

`./hub -ip -usb peer_ip peer_ip ...`

These hubs take care of USB links getting plugged in between machines and run a
server for IP link connections.

For an application or stand alone service, they should `dial` the `localhost`
`hub` to connect to the network with:

`./hub 127.0.0.1`

Nothing stops you from having a bundle of services as threads of a single
process, but the router for that bundle `dials` the local `hub` to give them
all access to the network.

Subnets can exist on the same Ethernet network with no issue. Only the
applications and services that have `dialed` another member will be seen to be
part of that subnet.

Subnets can join to each other dynamically if a link comes up between them, and
they will split back into individual subnets if the link connecting them goes
down.

Applications and services can be created that dynamically probe the service
directory and farm out work to external resources and applications. Robust
failure is easy to arrange with the timed message reading and selection lists.
Discarding mailboxes and allocating fresh ones as you need allows you to
silently ignore any in flight messages that may still be on route.

### File_Service

The `File_Service` standalone example app/service can be launched with:

`files 127.0.0.1`

This example is only a base class, you would provide a subclass overriding the
virtual methods to provide an implementation for a specific OS or filesystem
API etc.
