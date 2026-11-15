#!/usr/bin/env python3
"""VLAN interface

Put a VLAN interface with an address on top of a dummy interface and
confirm the system created it with that VLAN ID.  Then change the
interface below it, which has that one created anew, and confirm the
VLAN interface is back on top of it.
"""
import re

import my

IF_NS = "urn:ietf:params:xml:ns:yang:ietf-interfaces"
IP_NS = "urn:ietf:params:xml:ns:yang:ietf-ip"
IANA_NS = "urn:ietf:params:xml:ns:yang:iana-if-type"
MYIF_NS = "urn:mylinux:interfaces"

LOWER = "trunk0"
VID = 10
VLAN = f"{LOWER}.{VID}"
ADDR = "10.40.0.1"

IF_FILTER = f'<interfaces xmlns="{IF_NS}"/>'


def edit(dut, interfaces):
    dut.edit(f'<interfaces xmlns="{IF_NS}">{interfaces}</interfaces>')


def vlan_of(xml, name):
    """(VLAN ID, lower interface) of a VLAN interface, as the kernel has it."""
    m = re.search(r"<id>(\d+)</id>\s*<lower-layer-if>([^<]+)</lower-layer-if>", my.interface(xml, name))
    return (int(m.group(1)), m.group(2)) if m else None


@my.case
def test(dut):
    with dut.step(f"Add {LOWER} and VLAN {VID} on top of it, as {VLAN}"):
        edit(dut, f"""
          <interface>
            <name>{LOWER}</name>
            <type xmlns:myif="{MYIF_NS}">myif:dummy</type>
          </interface>
          <interface>
            <name>{VLAN}</name>
            <type xmlns:ianaift="{IANA_NS}">ianaift:l2vlan</type>
            <vlan xmlns="{MYIF_NS}"><id>{VID}</id><lower-layer-if>{LOWER}</lower-layer-if></vlan>
            <ipv4 xmlns="{IP_NS}">
              <address><ip>{ADDR}</ip><prefix-length>24</prefix-length></address>
            </ipv4>
          </interface>""")

    with dut.step(f"Confirm {VLAN} is VLAN {VID} of {LOWER}, with its address"):
        def applied():
            xml = dut.get_oper(IF_FILTER)
            return vlan_of(xml, VLAN) == (VID, LOWER) and ADDR in xml
        my.until(applied)

    with dut.step(f"Give {LOWER} an address, which has it created anew"):
        edit(dut, f"""
          <interface>
            <name>{LOWER}</name>
            <ipv4 xmlns="{IP_NS}">
              <address><ip>10.40.1.1</ip><prefix-length>24</prefix-length></address>
            </ipv4>
          </interface>""")

    with dut.step(f"Confirm {VLAN} is back on top of it"):
        def back():
            xml = dut.get_oper(IF_FILTER)
            return "10.40.1.1" in xml and vlan_of(xml, VLAN) == (VID, LOWER) and ADDR in xml
        my.until(back)

    with dut.step("Remove both"):
        edit(dut, "".join(f'<interface nc:operation="delete"><name>{name}</name></interface>'
                          for name in (VLAN, LOWER)))
        my.until(lambda: LOWER not in dut.get_oper(IF_FILTER))
