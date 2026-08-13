#!/usr/bin/env python3
"""System operational state

Set a timezone over NETCONF, then read the applied value back from the
operational datastore.  mech reads it from the live system (/etc/TZ), so
this confirms the applied state, not a running echo.
"""
import my

SYS_NS = "urn:ietf:params:xml:ns:yang:ietf-system"
NETCONF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0"

TZ = "Europe/Stockholm"

FILTER = f'<system xmlns="{SYS_NS}"><clock/></system>'


@my.case
def test(dut):
    with dut.step(f"Set timezone {TZ} over NETCONF"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <system xmlns="{SYS_NS}">
                  <clock><timezone-name>{TZ}</timezone-name></clock>
                </system>
              </config>""")
            nc.commit()

    with dut.step("Read the applied timezone back from operational"):
        my.until(lambda: TZ in dut.get_oper(FILTER))
