#!/usr/bin/env python3
"""Show interface

Configure a dummy interface with an address, then check that the CLI's
'show interface' overview and per-interface detail render it.  The CLI
reads the same operational data a remote NETCONF client would see, so
this also exercises that path end to end.
"""
import my

IF_NS = "urn:ietf:params:xml:ns:yang:ietf-interfaces"
IP_NS = "urn:ietf:params:xml:ns:yang:ietf-ip"
IANA_NS = "urn:ietf:params:xml:ns:yang:iana-if-type"
NETCONF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0"

IFACE = "dummy0"
ADDR = "10.10.10.1"
PLEN = "24"


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

    with dut.step(f"'show interface' overview lists eth0 and {IFACE}"):
        def overview():
            out = dut.cli("show interface")
            return out if IFACE in out else False
        out = my.until(overview)
        assert "eth0" in out, out
        assert f"{ADDR}/{PLEN}" in out, out

    with dut.step("overview is ordered loopback-first, then by if-index"):
        names = [ln.split()[0] for ln in out.splitlines()
                 if ln and not ln[0].isspace() and not ln.startswith("INTERFACE")]
        assert names[0] == "lo", names
        assert names.index("eth0") < names.index(IFACE), names

    with dut.step(f"'show interface {IFACE}' details the address"):
        out = dut.cli(f"show interface {IFACE}")
        assert IFACE in out, out
        assert "ethernet" in out, out
        assert "operational status" in out, out
        assert f"{ADDR}/{PLEN} (static)" in out, out

    with dut.step("'show interface eth0' details the management link"):
        out = dut.cli("show interface eth0")
        assert "eth0" in out, out
        assert "physical address" in out, out
