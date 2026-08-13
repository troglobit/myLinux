#!/usr/bin/env python3
"""DNS resolver

Configure search domains and name servers over NETCONF, confirm they
reach the effective /etc/resolv.conf, and read the whole dns-resolver
back from the operational datastore -- server names included, which
round-trip through a "# id:" annotation mech writes into resolv.conf.head.
Also guards the search-only case, which used to wipe the config.
"""
import my

SYS_NS = "urn:ietf:params:xml:ns:yang:ietf-system"
NETCONF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0"

FILTER = f'<system xmlns="{SYS_NS}"><dns-resolver/></system>'


@my.case
def test(dut):
    with dut.step("Configure search + two name servers over NETCONF"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <system xmlns="{SYS_NS}">
                  <dns-resolver>
                    <search>corp.example</search>
                    <server>
                      <name>primary</name>
                      <udp-and-tcp><address>8.8.8.8</address></udp-and-tcp>
                    </server>
                    <server>
                      <name>secondary</name>
                      <udp-and-tcp><address>1.1.1.1</address></udp-and-tcp>
                    </server>
                  </dns-resolver>
                </system>
              </config>""")
            nc.commit()

    with dut.step("Confirm the servers reached the effective /etc/resolv.conf"):
        def applied():
            _, out = dut.ssh("cat /etc/resolv.conf")
            return out if ("8.8.8.8" in out and "1.1.1.1" in out) else False
        out = my.until(applied)
        assert "search corp.example" in out, out

    with dut.step("Read the whole dns-resolver back from operational"):
        xml = dut.get_oper(FILTER)
        assert "<search>corp.example</search>" in xml, xml
        assert "<name>primary</name>" in xml and "8.8.8.8" in xml, xml
        assert "<name>secondary</name>" in xml and "1.1.1.1" in xml, xml

    with dut.step("Search-only config still reaches resolv.conf (regression)"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <system xmlns="{SYS_NS}">
                  <dns-resolver xmlns:nc="{NETCONF_NS}" nc:operation="replace">
                    <search>solo.example</search>
                  </dns-resolver>
                </system>
              </config>""")
            nc.commit()
        my.until(lambda: "search solo.example" in dut.ssh("cat /etc/resolv.conf")[1])
