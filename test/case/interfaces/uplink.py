#!/usr/bin/env python3
"""The first port

The first Ethernet port is in the factory configuration with a DHCP
client, and is configured like any other interface.  Confirm it has the
address it was handed, add a static one next to it over the very link
this session runs on, and take it away again.
"""
import my

IF_NS = "urn:ietf:params:xml:ns:yang:ietf-interfaces"
IP_NS = "urn:ietf:params:xml:ns:yang:ietf-ip"

PORT = "eth0"
LEASE = "10.0.2.15"         # what qemu's DHCP server hands out
ADDR = "192.168.77.1"

IF_FILTER = f'<interfaces xmlns="{IF_NS}"><interface><name>{PORT}</name></interface></interfaces>'


def edit(dut, ipv4):
    dut.edit(f"""
      <interfaces xmlns="{IF_NS}">
        <interface>
          <name>{PORT}</name>
          <ipv4 xmlns="{IP_NS}">{ipv4}</ipv4>
        </interface>
      </interfaces>""")


@my.case
def test(dut):
    with dut.step(f"The factory configuration has {PORT} with a DHCP client"):
        with dut.netconf() as nc:
            xml = nc.get_config("running", filter=("subtree", IF_FILTER)).xml
        assert f"<name>{PORT}</name>" in xml and "<dhcp" in xml, xml

    with dut.step(f"Confirm {PORT} got its address from DHCP"):
        xml = my.until(lambda: LEASE in (xml := dut.get_oper(IF_FILTER)) and xml)
        assert "<origin>dhcp</origin>" in xml, xml

    with dut.step(f"Add {ADDR} to {PORT}"):
        edit(dut, f"<address><ip>{ADDR}</ip><prefix-length>24</prefix-length></address>")

    with dut.step("Confirm both addresses, over a new session"):
        def both():
            xml = dut.get_oper(IF_FILTER)
            return ADDR in xml and LEASE in xml
        my.until(both, attempts=20)

    with dut.step(f"Remove {ADDR} again"):
        edit(dut, f'<address nc:operation="delete"><ip>{ADDR}</ip></address>')
        def one():
            xml = dut.get_oper(IF_FILTER)
            return ADDR not in xml and LEASE in xml
        my.until(one, attempts=20)
