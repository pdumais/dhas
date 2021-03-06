# Dumais Home Automation System (DHAS)
This is a home automation software controller. The software acts as a hub for multiple technologies.
The project started as an "Insteon" software controller but I eventually added support for
other technologies such as interfacing my Asterisk PBX and accessing a [WebRelay device](http://www.controlbyweb.com/webrelay/). The software triggers events (such as lightswitch on/of etc...) by invoking a Lua or JS script. So it is possible to 
control everything by modifying the script instead of having to hardcode behaviours in the software.

The project initially ran on a [rack-mounted raspberry pi](http://www.dumaisnet.ca/index.php?article=205f6247f86b88ae8941496569b5cd07) but it is now a x86-64 binary.

Modules
============== 
DHAS is modular. Every different technology is represented by a module. The current modules are:

    - IO
      Used for controlling input and output pins on a webrelay and on an arduino connected to a USB port
      and running the DHAS arduino firmware. This is basically used for reading digital inputs and controlling
      relays. 
    - Insteon
      Used to control Insteon devices such as light switches, christmas tree, garage door, sprinklers.
    - Phone
      Used to generate calls and play sounds on the calls and receiving BLF notifications. Will register
      with a SIP service such as Asterisk. Uses reSIProcate undeneath. I mainly use it to generate wake-up calls.
    - SMTP
      Used as an SMTP server to receive emails from my alarm system since it is the only kind of 
      events I can get out it. So using this module, I can get notification about arming/disarming/alarm events.
    - Sound
      Used to play sounds on the local soundard. Can be used to play alerts or notifications.
    - Weather
      Used to query temperature from webrelay temperature modules.
    - DHASWifiNodes
      Used to interface with homemade wifi devices based on ESP8266

Script
==============
DHAS supports Lua and Javascript scripts. The Makefile must be modified to support Lua as it will 
support Javascript by default. Javascript scripting is made possible with [Duktape](https://github.com/svaarala/duktape)

When DHAS loads, it will run the code from the Lua or JS script (path defined in config.h).
So it is necessary to initialize things such as Webrelay IP address, insteon device database etc..
at the begining of the script

Since DHAS is REST based, any action is triggered with a URL. From the script you can do that with:
```
initiateAction("/insteon/switch?id=0xaabbcc&action=off")
```

When an event is generated by DHAS, the onEvent function will be invoked from the script.
```
DHAS.onevent = function(st)
{
    var v = JSON.parse(st);

    if (v["event"]=="sunset")
    {
        // on sunset, turn on outside light and close garage door
        DHAS.initiateAction("/insteon/switch?id=0x445566&action=on");
        DHAS.initiateAction("/insteon/switch?id=0x112233&action=off");
    }
};
```
Or if using Lua scripting:

```
function onEvent(st)
    JSON = (loadfile "/opt/ha/JSON.lua")()
    local v = JSON:decode(st)

    if v["event"]=="sunset" then
        -- on sunset, turn on outside light and close garage door
        initiateAction("/insteon/switch?id=0x445566&action=on")
        initiateAction("/insteon/switch?id=0x112233&action=off")
    end
end
``` 

Since the onEvent function receives a JSON string, you will need a Lua JSON decoder if using Lua. I am using the one at
[http://regex.info/blog/lua/json](http://regex.info/blog/lua/json). The file is not distributed in this software

An example script can be found in the documentation folder for both Lua and Javascript

Events 
============== 
Not documented yet... please refer to the source code and the example scripts.

API Documentation
==============
DHAS uses my [REST engine](https://github.com/pdumais/rest), which is self-documenting. The documentation is generated in [documentation/api.json](documentation/api.json)

Sounds
==============
Sound files may be usefull to play on a phone call or on the local sound card. Each sound files needed
should be converted to S16 and u-law to be used by DHAS. a script called "haconvertsounds" is deployed with
the homeautomation executable. invoke that script from the directory where your sound files are located and
the script will convert the files (using sox) and copy them to the proper folder as defined in config.h

Dependencies
==============

    - reSIProcate
    - berkeleyDB
    - liblua (if using Lua)
    - libasound (Alsa)
    - libudev
    - libmysqlclient
    - ortp
    - git submodules: dumaislib (clone the project with --recursive to get them automatically)

Invoking
==============
    
    - homeautomation -d: start the server in daemon mode
    - homeautomation -g: generate API documentation
    - homeautomation -r: send SIGHUP to server process. This will reload the script file

Makefile
==============
The Makefile is customized for my system so you may have to modify it to make it more generic.

    - make: will generate the executable and the api documentation
    - make install: will build then kill the running DHAS, deploy files and restart DHAS
    - make upgrade: will kill the running process, install, then lauch the process
    - make tests: will generate tests included in the tests folder (but will not run them)

