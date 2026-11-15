#!/usr/bin/env python3
"""veth pair

Set up a veth pair, both ends with an address, and confirm that the
system created the two as each other's peer.  One end alone, with a peer
that does not point back, is refused.  Removing the pair removes both.
"""
import re

from ncclient.operations import RPCError

import my

IF_NS = "urn:ietf:params:xml:ns:yang:ietf-interfaces"
IP_NS = "urn:ietf:params:xml:ns:yang:ietf-ip"
MYIF_NS = "urn:mylinux:interfaces"

ENDS = {"veth0a": "10.30.0.1", "veth0b": "10.30.0.2"}

IF_FILTER = f'<interfaces xmlns="{IF_NS}"/>'


def end(name, peer, ip=None):
    addr = f"""<ipv4 xmlns="{IP_NS}">
                 <address><ip>{ip}</ip><prefix-length>24</prefix-length></address>
               </ipv4>""" if ip else ""
    return f"""<interface>
                 <name>{name}</name>
                 <type xmlns:myif="{MYIF_NS}">myif:veth</type>
                 <veth xmlns="{MYIF_NS}"><peer>{peer}</peer></veth>
                 {addr}
               </interface>"""


def edit(dut, interfaces):
    dut.edit(f'<interfaces xmlns="{IF_NS}">{interfaces}</interfaces>')


def peer_of(xml, name):
    m = re.search(r"<peer>([^<]+)</peer>", my.interface(xml, name))
    return m.group(1) if m else None


@my.case
def test(dut):
    a, b = ENDS

    with dut.step(f"Add the veth pair {a} and {b}"):
        edit(dut, end(a, b, ENDS[a]) + end(b, a, ENDS[b]))

    with dut.step("Confirm both ends exist, as peers, with their addresses"):
        def applied():
            xml = dut.get_oper(IF_FILTER)
            return (peer_of(xml, a) == b and peer_of(xml, b) == a
                    and all(ip in xml for ip in ENDS.values()))
        my.until(applied)

    with dut.step("Refuse an end whose peer does not point back"):
        try:
            edit(dut, end("veth1a", a))
        except RPCError as err:
            assert "peer" in str(err), err
        else:
            raise AssertionError("half a veth pair went through")

    with dut.step("Remove the pair"):
        edit(dut, "".join(f'<interface nc:operation="delete"><name>{name}</name></interface>'
                          for name in ENDS))
        my.until(lambda: a not in dut.get_oper(IF_FILTER))
