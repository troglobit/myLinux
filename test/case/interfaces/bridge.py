#!/usr/bin/env python3
"""Bridge

Set up a bridge with an address and two dummy interfaces as its ports,
and confirm from the kernel's side that the ports are in it.  Change the
address of the bridge, which has it created anew, and confirm the ports
are put back.  Take one port out again without the other one or the
bridge noticing.  A port with an address of its own is refused.
"""
import re

from ncclient.operations import RPCError

import my

IF_NS = "urn:ietf:params:xml:ns:yang:ietf-interfaces"
IP_NS = "urn:ietf:params:xml:ns:yang:ietf-ip"
IANA_NS = "urn:ietf:params:xml:ns:yang:iana-if-type"
MYIF_NS = "urn:mylinux:interfaces"

BRIDGE = "br0"
PORTS = ("port1", "port2")

IF_FILTER = f'<interfaces xmlns="{IF_NS}"/>'


def address(ip):
    return f"""<ipv4 xmlns="{IP_NS}">
                 <address><ip>{ip}</ip><prefix-length>24</prefix-length></address>
               </ipv4>"""


def port(name, extra=""):
    return f"""<interface>
                 <name>{name}</name>
                 <type xmlns:myif="{MYIF_NS}">myif:dummy</type>
                 <bridge-port xmlns="{MYIF_NS}"><bridge>{BRIDGE}</bridge></bridge-port>
                 {extra}
               </interface>"""


def edit(dut, interfaces):
    dut.edit(f'<interfaces xmlns="{IF_NS}">{interfaces}</interfaces>')


def bridge_of(xml, name):
    """The bridge an interface is a port of, by the kernel's account."""
    m = re.search(r"<bridge>([^<]+)</bridge>", my.interface(xml, name))
    return m.group(1) if m else None


@my.case
def test(dut):
    with dut.step(f"Add bridge {BRIDGE} with ports {' and '.join(PORTS)}"):
        edit(dut, f"""
          <interface>
            <name>{BRIDGE}</name>
            <type xmlns:ianaift="{IANA_NS}">ianaift:bridge</type>
            {address("10.20.0.1")}
          </interface>""" + "".join(port(name) for name in PORTS))

    with dut.step("Confirm the bridge has its address and both ports"):
        def applied():
            xml = dut.get_oper(IF_FILTER)
            return "10.20.0.1" in xml and all(bridge_of(xml, p) == BRIDGE for p in PORTS)
        my.until(applied)

    with dut.step("Change the address of the bridge"):
        edit(dut, f"""
          <interface>
            <name>{BRIDGE}</name>
            {address("10.20.1.1").replace("<ipv4 ", '<ipv4 nc:operation="replace" ')}
          </interface>""")

    with dut.step("Confirm the new address, and both ports still in the bridge"):
        def moved():
            xml = dut.get_oper(IF_FILTER)
            return ("10.20.1.1" in xml and "10.20.0.1" not in xml
                    and all(bridge_of(xml, p) == BRIDGE for p in PORTS))
        my.until(moved)

    with dut.step(f"Take {PORTS[1]} out of the bridge"):
        edit(dut, f"""
          <interface>
            <name>{PORTS[1]}</name>
            <bridge-port xmlns="{MYIF_NS}" nc:operation="delete"/>
          </interface>""")

    with dut.step(f"Confirm {PORTS[1]} is out and up, {PORTS[0]} still in"):
        def out():
            xml = dut.get_oper(IF_FILTER)
            return bridge_of(xml, PORTS[1]) is None and bridge_of(xml, PORTS[0]) == BRIDGE
        my.until(out)

    with dut.step("Refuse a port with an address of its own"):
        try:
            edit(dut, port(PORTS[1], address("10.20.2.1")))
        except RPCError as err:
            assert "no addresses" in str(err), err
        else:
            raise AssertionError("a bridge port with an address went through")

    with dut.step("Remove the bridge and its ports"):
        edit(dut, "".join(f'<interface nc:operation="delete"><name>{name}</name></interface>'
                          for name in (BRIDGE, *PORTS)))
        my.until(lambda: BRIDGE not in dut.get_oper(IF_FILTER))
