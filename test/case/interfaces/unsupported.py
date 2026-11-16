#!/usr/bin/env python3
"""Settings the system has no use for

The standard models have more in them than the system acts on, and the
myLinux models say which parts with deviations.  Confirm that setting
one of those is refused, where taking it and doing nothing would have
looked like success, and that what is next to it still goes through.
"""
from ncclient.operations.rpc import RPCError

import my

IF_NS = "urn:ietf:params:xml:ns:yang:ietf-interfaces"
IP_NS = "urn:ietf:params:xml:ns:yang:ietf-ip"
MYIF_NS = "urn:mylinux:interfaces"

IFACE = "dummy0"
ADDR = "2001:db8::10"


def edit(dut, ipv6):
    dut.edit(f"""
      <interfaces xmlns="{IF_NS}">
        <interface>
          <name>{IFACE}</name>
          <type xmlns:myif="{MYIF_NS}">myif:dummy</type>
          <ipv6 xmlns="{IP_NS}">{ipv6}</ipv6>
        </interface>
      </interfaces>""")


@my.case
def test(dut):
    with dut.step(f"Be refused an MTU on {IFACE}, which is not supported"):
        try:
            edit(dut, "<mtu>1400</mtu>")
        except RPCError as err:
            assert "mtu is not supported" in str(err), err
        else:
            raise AssertionError("an unsupported setting was accepted")

    with dut.step(f"Give {IFACE} an IPv6 address, next to what is not supported"):
        edit(dut, f"<address><ip>{ADDR}</ip><prefix-length>64</prefix-length></address>")
        my.until(lambda: ADDR in dut.get_oper(f'<interfaces xmlns="{IF_NS}"/>'))
