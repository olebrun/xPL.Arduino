xPL.Arduino is an implementation of the xPL Protocol for Arduino.
Old repository can be found at https://code.google.com/p/xpl-arduino/

It works independently of the ethernet shield.

There are four examples with the library, two for the official wiznet ethershield (send and receive) and two for the enc28j60 based ethershield.



Test on Arduino Mega with enc28j60 ethershield and the ethercard library : https://github.com/jcw/ethercard

Features:

    No link with hardware
    Auto send heartbeat messages and answer to heartbeat request
    Parse received xPL messages and send result to a callback define by you
    Send xPL message 



You can find some help here (in french) : http://connectingstuff.net/blog/xpl-arduino/
