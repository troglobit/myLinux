#!/usr/bin/env python3
"""Set an interface address

Add a dummy interface with an IPv4 address over NETCONF, confirm the
system applied it, give it one more IPv4 address and an IPv6 one, then
remove it and confirm it is gone.  The management interface (eth0) is
left untouched.

Verification reads state data only (<get content="nonconfig">), which is
answered purely from the system's live kernel state -- so the address
shows up only once the system has pushed it to the kernel, not merely
because it sits in running.
"""
import my

IF_NS = "urn:ietf:params:xml:ns:yang:ietf-interfaces"
IP_NS = "urn:ietf:params:xml:ns:yang:ietf-ip"
IANA_NS = "urn:ietf:params:xml:ns:yang:iana-if-type"
NETCONF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0"

IFACE = "dummy0"
ADDR = "10.10.10.1"
PLEN = "24"
MORE = {"ipv4": ("10.10.20.1", "24"), "ipv6": ("2001:db8::1", "64")}

IF_FILTER = f"""
  <interfaces xmlns="{IF_NS}">
    <interface><name>{IFACE}</name></interface>
  </interfaces>"""


@my.case
def test(dut):
    with dut.step(f"Add {IFACE} with {ADDR}/{PLEN} over NETCONF"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <interfaces xmlns="{IF_NS}" xmlns:ianaift="{IANA_NS}">
                  <interface>
                    <name>{IFACE}</name>
                    <type>ianaift:ethernetCsmacd</type>
                    <ipv4 xmlns="{IP_NS}">
                      <address>
                        <ip>{ADDR}</ip>
                        <prefix-length>{PLEN}</prefix-length>
                      </address>
                    </ipv4>
                  </interface>
                </interfaces>
              </config>""")
            nc.commit()

    with dut.step(f"Confirm the system applied {ADDR} on {IFACE} (operational)"):
        my.until(lambda: ADDR in dut.get_oper(IF_FILTER))

    with dut.step(f"Add {' and '.join(ip for ip, _ in MORE.values())} to {IFACE}"):
        more = "".join(f"""
                    <{fam} xmlns="{IP_NS}">
                      <address><ip>{ip}</ip><prefix-length>{plen}</prefix-length></address>
                    </{fam}>""" for fam, (ip, plen) in MORE.items())
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <interfaces xmlns="{IF_NS}">
                  <interface>
                    <name>{IFACE}</name>{more}
                  </interface>
                </interfaces>
              </config>""")
            nc.commit()

    with dut.step(f"Confirm {IFACE} has all three addresses (operational)"):
        def applied():
            xml = dut.get_oper(IF_FILTER)
            return all(f"<ip>{ip}</ip>" in xml for ip in (ADDR, *(ip for ip, _ in MORE.values())))
        my.until(applied)

    with dut.step(f"Remove {IFACE} over NETCONF"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}" xmlns:nc="{NETCONF_NS}">
                <interfaces xmlns="{IF_NS}">
                  <interface nc:operation="delete">
                    <name>{IFACE}</name>
                  </interface>
                </interfaces>
              </config>""")
            nc.commit()

    with dut.step(f"Confirm {ADDR} is gone from {IFACE} (operational)"):
        my.until(lambda: ADDR not in dut.get_oper(IF_FILTER))
