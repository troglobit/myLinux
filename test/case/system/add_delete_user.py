#!/usr/bin/env python3
"""Add and delete a local user

Add a user over NETCONF and confirm a UNIX account has been created,
then delete it and confirm it's been removed.

"""
import my

SYSTEM_NS = "urn:ietf:params:xml:ns:yang:ietf-system"
NETCONF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0"
NAME = "test1"


@my.case
def test(dut):
    with dut.step(f"Add user '{NAME}' over NETCONF"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <system xmlns="{SYSTEM_NS}">
                  <authentication>
                    <user>
                      <name>{NAME}</name>
                    </user>
                  </authentication>
                </system>
              </config>""")
            nc.commit()

    with dut.step(f"Confirm the '{NAME}' UNIX account exists on the DUT"):
        rc, out = dut.ssh(f'grep -qs "^{NAME}:" /etc/passwd')
        assert rc == 0, f"user {NAME} was not created ({out!r})"

    with dut.step(f"Delete user '{NAME}' over NETCONF"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}" xmlns:nc="{NETCONF_NS}">
                <system xmlns="{SYSTEM_NS}">
                  <authentication>
                    <user nc:operation="delete">
                      <name>{NAME}</name>
                    </user>
                  </authentication>
                </system>
              </config>""")
            nc.commit()

    with dut.step(f"Confirm the '{NAME}' account was removed"):
        rc, out = dut.ssh(f'grep -qs "^{NAME}:" /etc/passwd')
        assert rc != 0, f"user {NAME} still present after delete ({out!r})"
