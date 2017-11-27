# arpsc #
----
The *arpsc* is a simple program for continuous ARP scanning on the network. Where the program tries to follow the GNU principle, do one thing and do it well.

The program can be configured for performing the ARP scanning differently, time interval, interface specified, display format and etc. See *arpsc*(1) for more information. 


# Motivation #
The program was designed for allowing for a continuous *ARP* scanning on the network. Allowing for the program be used for device detection. 

# Examples #
Common example of using the *arpsc* would be as followed:
```
arpsc --interface=eth0
```

Remark that the program has to be executed with root permission. Because it uses raw sockets, which can manipulate each bit that is being transmitted from the network card.

# Dependencies #
The program does not have any library dependencies that is not installed along with the kernel.

# License #
This project is licensed under the GPL+3 License - see the [LICENSE](LICENSE) file for details.

