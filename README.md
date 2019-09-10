# udp-multicast
A simple c++ library to send and receive udp multicast. Goal of this library is to provide a cleaner interface for sending and receiving UDP Multicast data.


Multicast is a kind of UDP traffic similar to BROADCAST, but only hosts that have explicitly requested to receive this kind     of traffic will get it.

This means that you have to JOIN a multicast group if you want to receive traffic that belongs to that group.
IP addresses in the range 224.0.0.0 to 239.255.255.255 ( Class D addresses) belongs to multicast.
No host can have this as IP address, but every machine can join a multicast address group.

Multicast traffic is only UDP (not reliable).
Multicast migth be 1 to many or 1 to none.
Not all networks are multicast enabled (Some routers do not forward Multicast).



Range Start Address     Range End Address           Description


224.0.0.0               224.0.0.255                 Reserved for special “well-known” multicast addresses.

224.0.1.0               238.255.255.255             Globally-scoped (Internet-wide) multicast addresses.

239.0.0.0               239.255.255.255             Administratively-scoped (local) multicast addresses.




