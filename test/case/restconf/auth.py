#!/usr/bin/env python3
"""RESTCONF authentication

RESTCONF, and with it the web interface, is open to the local users of
ietf-system that have administrator rights, with their system password.
Confirm that nobody else gets in: no credentials, a wrong password, and
a user with the right password but no administrator rights.
"""
import json

import my

SYS_NS = "urn:ietf:params:xml:ns:yang:ietf-system"
NETCONF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0"

HOSTNAME = "/restconf/data/ietf-system:system/hostname"

# SHA-512 crypt of the password "guest"
GUEST = ("$6$mylinuxtest$IlAMHO4HTSW/dfBdtoFcssFfGAyytiqp8kfcFsYK."
         "uTMiiOv15hZOhtpRBPeFR4SAkGbXBFRId37dc9NUeX8u/")


@my.case
def test(dut):
    with dut.step("Add user 'guest', who is not an administrator"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <system xmlns="{SYS_NS}">
                  <hostname>restconf</hostname>
                  <authentication>
                    <user>
                      <name>guest</name>
                      <password>{GUEST}</password>
                    </user>
                  </authentication>
                </system>
              </config>""")
            nc.commit()

    with dut.step("Refuse a request without credentials"):
        assert dut.http("GET", HOSTNAME, auth=None).status == 401

    with dut.step("Refuse a wrong password"):
        assert dut.http("GET", HOSTNAME, auth=(my.USER, "wrong")).status == 401

    with dut.step("Refuse an unknown user"):
        assert dut.http("GET", HOSTNAME, auth=("nobody", "guest")).status == 401

    with dut.step("Refuse 'guest' with the right password"):
        assert dut.http("GET", HOSTNAME, auth=("guest", "guest")).status == 401

    with dut.step("Refuse the event stream without credentials"):
        res = dut.http("GET", "/streams/NETCONF", auth=None, accept="text/event-stream")
        assert res.status == 401

    with dut.step("Let the administrator in"):
        res = dut.http("GET", HOSTNAME)
        assert res.status == 200
        assert json.load(res)["ietf-system:hostname"] == "restconf"
