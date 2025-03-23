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
                   | public IP0 | NIC2 with   |  LAN subnet 1                       | NIC3 for with public  IP3 | <---> ISP mobile data
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
  subnet 0 network   address = 192.168.1.0   <--- not assigned to any device in subnet 0 (logical IP as subnet identifier)
  subnet 0 broadcast address = 192.168.1.255 <--- not assigned to any device in subnet 0 (logical IP as broadcast channel)
  subnet 0 1st  usable IP    = 192.168.1.1   <--- assigned to router, i.e. IP1           (physical IP to real device)
  subnet 0 2nd  usable IP    = 192.168.1.2   <--- assigned to device 1                   (physical IP to real device)
  subnet 0 last usable IP    = 192.168.1.254                                             (physical IP to real device)
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




Who assign the IPs?
* public    IPs are assigned by ISP
* private   IPs are assigned by router
* multicast IPs (channels) are configured by developer / organization




*************************************
*** ifconfig / netstat / ip route ***
*************************************
We can get the NIC info with
* ipconfig in windows
* ifconfig in linux
* ifconfig in WSL cannot detect wifi LAN (so better use ipconfig in windows)



Running "ipconfig" can list all NICs, for example, running ipconfig in current PC : 
 
* Ethernet . . . . . . . . . . . . . : Media disconnected <--- since I have not connected cable yet
* Wireless LAN 
    IPv4 Address . . . . . . . . . . : 192.168.1.156      <--- this is IP of current PC
    Subnet Mask. . . . . . . . . . . : 255.255.255.0
    Default Gateway. . . . . . . . . : 192.168.1.1        <--- this is IP of router NIC for this subnet

In order to communicate with internet, current PC has to send message to gateway (192.168.1.1) first.   



Running "netstat", while current PC runs TCP echo server and TCP echo client, possible options 
* t = TCP
* u = UDP
* a = all states (LISTEN + ESTABLISHED)
* p = process ID

> netstat -tuanp

Proto   Recv-Q    Send-Q    Local Address     Foreign Address   State         PID      Program name
---------------------------------------------------------------------------------------------------------
tcp          0         0      0.0.0.0:12345     0.0.0.0:*       LISTEN        145054   ./build/debug/Test <--- server acccepting new client
tcp          0         0    127.0.0.1:12345   127.0.0.1:49400   ESTABLISHED   145054   ./build/debug/Test <--- server socket connected to client
tcp          0         0    127.0.0.1:49400   127.0.0.1:12345   ESTABLISHED   145149   ./build/debug/Test <--- client socket connected to server



Running "ip route" 
> ip route get xxx.yyy.zzz.ttt   <--- will give you the NIC it needs to reach that address 




**************************
*** Socket programming ***
**************************
What is bind?
* for server in both TCP and UDP, we need to call bind, which picks the desired NIC (or all NICs if you like)
* for client in both TCP and UDP, we do not need to call bind


Difference between recv() and recvfrom(), send() and sendto()
* recv()     = recv message from specific client with connection, for TCP unicast
* send()     = send message from specific client with connection, for TCP unicast
* recvfrom() = recv message from specific client without connection, for UDP unicast
* sendto()   = send message from specific client without connection, for UDP unicast

We can still call connect() for UDP, but ...
* it does not mean a real connection, a UDP is always connectionless
* it means remote address is fixed, no need to call recvfrom()/sendto()




************************************
*** DNS, NAT and port forwarding ***
************************************
How does a device A in LAN under router A, communicate with another device B in LAN under router B, via WAN?

Suppose we have :
* router A public IP is IP_A
* device A private IP in its LAN is IP_a
* router B public IP is IP_B
* device B private IP in its LAN is IP_b
* device A is client requesting for service from device B (via Chrome, for example)
* device B is server with URL www.B.com

It needs to go through the following steps :  
* DNS server 
* network address translation (NAT) 
* port forwarding




[Domain Name System DNS]
When we type URL into Chrome, Chrome needs to convert this URL into public IP, via these steps :
* check if URL exists in Chrome's cache, if yes, just read the corresponding IP
* check if URL exists in windows' cache, if yes, just read the corresponding IP 
* send DNS query to DNS resolver, there are :
- ISP provided DNS resolver, or
- public DNS resolver like google's 8.8.8.8 = dns.google
* finally, device A gets the value IP_B, make connection to IP_B




[Network Address Translation NAT]
Device A sends request-packet to router A :
{
    source IP   : IP_a   
    source port : 49400 (randomly assigned port by OS when connection is made)
    destin IP   : IP_B
    destin port : 80    (depends on the service device A needs from device B)
}

Router A replaces source address by its public IP, this conversion is called NAT :
{
    source IP   : IP_A   
    source port : 12345 (randomly assigned port by router)
    destin IP   : IP_B
    destin port : 80 
}

Router A sends the packet to router B via WAN, and stores the following key-value in a map :

   {IP_a, 49400} : {IP_A, 12345}




[Port forwarding]
Router B receives the packet, check against a manual config, to forward all requests for that service to device B : 

   {IP_B, 80} : {IP_b, 80} 

Router B replaces destination address by device B private IP, this conversion is called port forwarding :
{
    source IP   : IP_A   
    source port : 12345 
    destin IP   : IP_b  (device B private IP, as configured in router B)
    destin port : 80 
}




[Return trip]
Device B processes the request and creates a reply, send it back to router B with : 
{
    source IP   : IP_b 
    source port : 80 
    destin IP   : IP_A   
    destin port : 12345 
}

Router B receives the reply, performs NAT to replace source address by public IP :
{
    source IP   : IP_B 
    source port : 80 
    destin IP   : IP_A   
    destin port : 12345 
}
 
Router A receives the reply, performs port forwarding to replace destination address by private IP : 
{
    source IP   : IP_B 
    source port : 80 
    destin IP   : IP_a   
    destin port : 49400
}




*********************************
*** North tier and south tier ***
*********************************
These are terms in network of datacentre. 
* Within the rack of machines in colocation, it is a graph of devices connecting together.
* North tier is considered the   upstream of network, which include routers.
* South tier is considered the downstream of network, which include servers.



*********************************
*** TCP onload vs TCP offload ***
*********************************
Kernel bypass = general term to skip slow "kernel space" system call

it can be implemented as :
* TCP  onload (OpenOnload) = implementing network stack in user space in CPU (need special NIC)
* TCP offload engine (TOE) = implementing network stack in NIC               (need special NIC)

NIC that is designed for TCP onload usually does not support TCP offload
NIC that is designed for TCP offload usually does not support TCP onload

Other ways to do kernel bypass (not sure what they are???) :
* DPDK
* XDP
* RDMA














