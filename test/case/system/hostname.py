#!/usr/bin/env python3
"""Set hostname

Change the hostname over NETCONF and confirm the system applied it, read
back from the operational datastore rather than echoed from running.
"""
import my

SYSTEM_NS = "urn:ietf:params:xml:ns:yang:ietf-system"
NETCONF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0"

HOSTNAME = "testhost"
SYS_FILTER = f'<system xmlns="{SYSTEM_NS}"><hostname/></system>'


@my.case
def test(dut):
    with dut.step(f"Set the hostname to '{HOSTNAME}' over NETCONF"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <system xmlns="{SYSTEM_NS}">
                  <hostname>{HOSTNAME}</hostname>
                </system>
              </config>""")
            nc.commit()

    with dut.step("Confirm the applied hostname in the operational datastore"):
        my.until(lambda: f"<hostname>{HOSTNAME}</hostname>" in dut.get_oper(SYS_FILTER))
