# arpsc
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/voldien/arpsc.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/voldien/arpsc/context:cpp)
[![ARP Scanner Linux](https://github.com/voldien/arpsc/actions/workflows/Linux.yml/badge.svg)](https://github.com/voldien/arpsc/actions/workflows/Linux.yml)

The *arpsc* is a simple program for continuous ARP scanning on the network. Where the program tries to follow the GNU principle, do one thing and do it well.

The program can be configured for performing the ARP scanning differently, time interval, interface specified, display format and etc. See *arpsc*(1) for more information. 


# Motivation
The program was designed for allowing for a continuous *ARP* scanning on the network. Allowing for the program to be used for device detection. 


## Installation
The software can be easily installed with invoking the following command.
```bash
mkdir build && cd build
cmake ..
cmake --build .
make install
```

## Examples
Common example of using the *arpsc* would be as followed:
```
arpsc --interface=eth0
```

Remark that the program has to be executed with root permission. Because it uses raw sockets, which can manipulate each bit that is being transmitted from the network card.

## Dependencies
The program does not have any library dependencies that is not installed along with the kernel.

# License #
This project is licensed under the GPL+3 License - see the [LICENSE](LICENSE) file for details.

