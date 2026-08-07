#!/usr/bin/env python3
"""Set hostname

Verify changing hostname using NETCONF.
"""
import my

SYSTEM_NS = "urn:ietf:params:xml:ns:yang:ietf-system"
NETCONF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0"


@my.case
def test(dut):
    with dut.netconf() as nc:
        with dut.step("Set the hostname to 'testhost' over NETCONF"):
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <system xmlns="{SYSTEM_NS}">
                  <hostname>testhost</hostname>
                </system>
              </config>""")
            nc.commit()

        with dut.step("Confirm the new hostname in running-config"):
            cfg = nc.get_config(source="running").data_xml
            assert "<hostname>testhost</hostname>" in cfg, \
                "hostname edit did not round-trip"
