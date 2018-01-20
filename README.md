# eoiptapd

Yet another implementation of MikroTik's EoIP protocol on Linux and TAP devices.

## Build

To build this repository, CMake and a compiling toolchain (GCC) are required.

On Linux, simply run following commands:

    cmake .
    make
    
And the executable `eoiptapd` is now ready for you.

## Usage

    eoiptapd -i IF_NAME -l LOCAL_ADDR -r REMOTE_ADDR -t TUNNEL_ID  
    
    LOCAL_ADDR:     local address to bind
    REMOTE_ADDR:    remote address of EoIP tunnel peer
    TUNNEL_ID:      tunnel id
    
## Example

Assumes you have `10.0.0.1` on a MikroTik box, and `10.0.0.2` on a Linux box.

    /interface eoip add local-address=10.0.0.1 remote-address=10.0.0.2 tunnel-id=114
    
    eoiptapd -i tap-eoip -l 10.0.0.2 -r 10.0.0.1 -t 114
    
## License

Source code is published under MIT license. Thanks to @magicnat for the protocol analysis.
 