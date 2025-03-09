Internet is a interconnected graph of routers. 




**************************
*** Router, NIC and IP ***
**************************
Router is a computer with at least 2 network interface cards (NIC)
* one NIC for connecting to wide area network (WAN)
* one NIC for connecting to local area network (LAN)
* the WAN NIC has a universal-unique IP address, called  public address, assigned by ISP
* the LAN NIC has a   locally-unique IP address, called private address, configurable in router
* router may have multiple NICs for multiple subnet in the LAN, like ethernet (eth) and wifi LAN (wlan)

                   +--------------------------+  
                   | [ROUTER]                 |  LAN subnet 0
                   |            | NIC1 with   | <------------> devices  1-10        +---------------------------+
             WAN   |            | private IP1 |                                     | [device 15, mobile phone] |
  internet <-----> | NIC0 with  |             |                                     |                           |
                   | public IP0 | NIC2 with   |  LAN subnet 1                       | NIC3 for with public  IP3 | <---> ISP mobile data                           | 
                   |            | private IP2 | <------------> devices 11-20 <----> | NIC4 for with private IP4 | 
                   |            |             |                                     | NIC5 for with 127.0.0.1   | <---> same host communication  
                   +--------------------------+                                     +---------------------------+
                                                                                       


For example, if device 15 is a mobile phone, which has 3 NICs:
* NIC3 for connecting to WAN, via mobile data provided by ISP, it uses public IP (some ISPs can use private IP)
* NIC4 for connecting to LAN, via wifi provided by router, it uses public IP 
* NIC5 for loopback connection, i.e. for IPC in the same host
- NIC3 & 4 are physical NIC, NIC5 is virtual NIC (logical NIC)

There are 2 ISPs in this example :
* ISP that provides router      
* ISP that provides mobile data

Uniqueness of IPs
* IP0 and IP3   are unique in WAN <--- assigned by ISP
* IP1, IP2, IP4 are unique in LAN <--- assigned by router config



The IP address has 4 bytes, so accommodate 2^32 (around 4*10^9) devices. 

  1.  0.0.0 - 223.255.255.255 =  public IP in WAN
 10.  0.0.0 -  10.255.255.255 = private IP in LAN (for TCP connection & UDP unicast) 
172. 16.0.0 - 172. 31.255.255 = private IP in LAN (for TCP connection & UDP unicast)
192.168.0.0 - 192.168.255.255 = private IP in LAN (for TCP connection & UDP unicast)
127.  0.0.0 - 127.255.255.255 =  loop back in same host (IPC in same host)
224.  0.0.0 - 239.255.255.255 = logical IP in LAN (for UDP multicast to devices that listen to that IP)
              255.255.255.255 = logical IP in LAN (for broadcast to all devices in the LAN)



Private IPs are unique within the same LAN (across all subnets)
* each subnet is defined by subnet mask, like :
  255.255.255.0 = 24 bits for network address,  8 bits for devices in subnet
  255.255.254.0 = 23 bits for network address,  9 bits for devices in subnet
  255.255.252.0 = 22 bits for network address, 10 bits for devices in subnet

* each subnet is assigned with :
  network-address   = first IP in the IP-range of the subnet
  broadcast-address =  last IP in the IP-range of the subnet 

* network-address and broadcast-address are virtual, not binded to any device : 
  network-address   = identifier of subnet
  broadcast-address = logical IP for broadcast

* using above example
  subnet 0 = 192.168.1.0, with subnet mask = 255.255.255.0
  subnet 1 = 192.168.2.0, with subnet mask = 255.255.255.128
  
  then we have : 
  subnet 0 network   address = 192.168.1.0   <--- not assigned to any device in subnet 0
  subnet 0 broadcast address = 192.168.1.255 <--- not assigned to any device in subnet 0
  subnet 0 1st  usable IP    = 192.168.1.1   <--- assigned to router, i.e. IP1
  subnet 0 2nd  usable IP    = 192.168.1.2   <--- assigned to device 1
  subnet 0 last usable IP    = 192.168.1.254
  subnet 1 network   address = 192.168.2.0   <--- not assigned to any device in subnet 1
  subnet 1 broadcast address = 192.168.2.255 <--- not assigned to any device in subnet 1
  subnet 1 1st  usable IP    = 192.168.2.1   <--- assigned to router, i.e. IP2
  subnet 1 2nd  usable IP    = 192.168.2.2   <--- assigned to device 11 
  subnet 1 last usable IP    = 192.168.2.6   <--- assigned to device 15, i.e. IP4



Unicast IP (TCP unicast & UDP unicast) vs multicast IP in a LAN
*   unicast IP in a LAN is    real address, binding to physical NIC
* multicast IP in a LAN is virtual address, not bind to NIC, it is a channel for broadcasting



Loopback interface
* loopback interface is also virtual, for IPC in same host
* for TCP-offload (OFF), sockets are created with loopback  enabled by default
* for TCP-offload (ON),  sockets are created with loopback disabled by default
* hence different apps in same host (with TCP-offload ON) cannot receive UDP multicast of each other, unless :
- ensure sockets are created with loopback enabled ... AND
- ensure all apps use the same TCP-offload stack




**************************
*** ifconfig & netstat ***
**************************
We can get the NIC info with
* ipconfig in windows
* ifconfig in linux
* ifconfig in WSL cannot detect wifi LAN

Running ipconfig can list all NICs : 

 





ifconfig shows the default gateway for each NIC
the LAN private IP, through which the device can get access to internet WAN 

in above example
NIC4's default gateway to WAN is IP1 


dick@DellDick:/mnt/d/dev/cpp/interprocess$ netstat -tupna

Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
tcp        0      0 0.0.0.0:12345           0.0.0.0:*               LISTEN      145054/./build/debu
tcp        0      0 127.0.0.1:49400         127.0.0.1:12345         ESTABLISHED 145149/./build/debu
tcp        0      0 127.0.0.1:12345         127.0.0.1:49400         ESTABLISHED 145054/./build/debu


WSL cannot access wifi lan via ifconfig (WSL)
but i can see all interface in ipconfig (windows cmd)



Difference between recv() and recvfrom(), send() and sendto()
* recv()     = receive message from specific client with connection
* recvfrom() = receive message from specific client without connection
* send()     = 
* sendto()   = 


For server in both TCP and UDP, we need to call bind.
For client in both TCP and UDP, we do not need to call bind.

We can still call connect() for UDP, but ...
* it does not meanreal connection
* it means remote address is fixed


What is unicast IP and multicast IP?



************************************
*** DNS, NAT and port forwarding ***
************************************


How does the following work ?
* network address translation (NAT) 
* port forwarding
* DNS server 











DNS 
8.8.8.8 = dns.google





UDP   unicast IP = 
UDP multicast IP = 





TCP/UDP unicast IP =     address of a physical device (or NIC)
  UDP multicast IP = NOT address of a physical device (or NIC)
                   = logic channel / logical group, its virtual address

private unicast IP is assigned by router (via DHCP)
public  unicast IP is assigned by ISP (assigned to ISP's routers)
      multicast IP is assigned by app / developer / organization

  same-router-multicast (by default) covers local network only (i.e. under same router)
across-router-multicast needs special config ... 



