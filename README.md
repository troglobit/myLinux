<img align="right" src="doc/example.png" alt="Example Boot">
<img class="left" src="doc/logo.png" alt="myLinux">

* [Introduction](#introduction)
* [Building](#building)
* [Testing SNMP](#testing-snmp)
* [Dropbear SSH](#dropbear-ssh)
* [Using Telnet](#using-telnet)
* [Bugs & Feature Requests](#bugs--feature-requests)
- [Licensing & References](licensing--references)


Introduction
------------

myLinux is a UNIX like OS for embedded systems based on [Buildroot][].
It serves as a testing ground for various embedded networking hardware
and open source software projects by the main author.

myLinux can be used to test software components in Qemu before deploying
to an embedded target, or as a reference to other embedded Linux systems.


Building
--------

Buildroot is almost stand-alone, but need a few locally installed tools
to bootstrap itself.  For details, see the [excellent manual][manual].
Briefly, to build a myLinux image; select the target and then make:

    make espressobin_defconfig
    make

Online help is available:

    make help

To see available defconfigs for supported targets, use:

    make list-defconfigs


Monitoring with SNMP
--------------------

myLinux  use [mini-snmpd](https://troglobit.com/mini-snmpd.html)  as its
SNMP  agent.  It  is  very  small and  therefore  also  very limited  in
functionality, but it is enough to monitor myLinux by remote if needed.

    initctl enable snmpd
    initctl reload

To test  it you  need an  SNMP client.   The following  command installs
`snmpset`,  `snmpget`,  `snmpwalk`,  base  MIBs and  all  standard  MIBs
needed.  You  may also  be interested in  a more  graphical alternative,
[snmpB](http://sourceforge.net/projects/snmpb/)

<kbd>sudo apt-get install snmp libsnmp-base snmp-mibs-downloader</kbd>

When done you should be able to do the following:

<kbd>snmpwalk -v2c -c public 192.0.2.42</kbd>

    SNMPv2-MIB::sysDescr.0 = STRING: myLinux Linux Virtual Devboard
    SNMPv2-MIB::sysObjectID.0 = OID: SNMPv2-SMI::enterprises
    SNMPv2-MIB::sysUpTime.0 = Timeticks: (465) 0:00:04.65
    SNMPv2-MIB::sysContact.0 = STRING: troglobit@gmail.com
    SNMPv2-MIB::sysName.0 = STRING: chaos
    SNMPv2-MIB::sysLocation.0 = STRING: GitHub
    IF-MIB::ifNumber.0 = INTEGER: 1
    IF-MIB::ifIndex.1 = INTEGER: 1
    IF-MIB::ifDescr.1 = STRING: eth0
    IF-MIB::ifOperStatus.1 = INTEGER: up(1)
    IF-MIB::ifInOctets.1 = Counter32: 5557
    IF-MIB::ifInUcastPkts.1 = Counter32: 45
    IF-MIB::ifInDiscards.1 = Counter32: 0
    IF-MIB::ifInErrors.1 = Counter32: 0
    IF-MIB::ifOutOctets.1 = Counter32: 2958
    IF-MIB::ifOutUcastPkts.1 = Counter32: 19
    IF-MIB::ifOutDiscards.1 = Counter32: 0
    IF-MIB::ifOutErrors.1 = Counter32: 0
    SNMPv2-SMI::mib-2.25.1.1.0 = Timeticks: (71983) 0:11:59.83
    
<kbd>snmpget -c public -v 2c 192.0.2.42 system.sysUpTime.0</kbd>

    SNMPv2-MIB::sysUpTime.0 = Timeticks: (2344) 0:00:23.44

**Note:** Other SNMP agents are also available in Buildroot, but
  mini-snmpd is pre-selected for myLinux targets.


Dropbear SSH
------------

The most  common embedded SSH  daemon in  use on embedded  Linux systems
today  is [Dropbear](https://matt.ucc.asn.au/dropbear/dropbear.html)  by
the incredibly humble [Matt Johnston](https://matt.ucc.asn.au/).

Dropbear is  one of  a few  services enabled by  default in  myLinux, it
allows `root` access,  but we recommend disabling this and  setting up a
regular user account after install.


Using Telnet
------------

The Busybox `telnetd`  is available in myLinux, for  security reasons it
is disabled by default, so you have to enable it:

    initctl enable telnetd
    initctl reload


Bugs & Feature Requests
-----------------------

Feel free to report bugs and request features, or even submit your own
[pull requests](https://help.github.com/articles/using-pull-requests/)
using [GitHub](https://github.com/troglobit/myLinux)


Licensing & References
----------------------

With the  exceptions listed below,  myLinux v2 is distributed  under the
same terms as [Buildroot][], the [GNU GPL][].  myLinux is only the build
system, or glue, that ties  the various Open Source components together.
Each project included comes with  source code, and sometimes local patch
files, all with their own license and restrictions.

[GNU GPL]:   COPYING
[Buildroot]: https://buildroot.org
[manual]:    https://buildroot.org/downloads/manual/manual.html
