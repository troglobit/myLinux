#!/usr/bin/env python3
"""Local users

Add users over NETCONF -- one with the CLI (clish) shell and a password,
one with defaults -- confirm the UNIX accounts and the applied shell, read
the applied shell and password back from the operational datastore (mech
derives them from /etc/passwd and /etc/shadow), then delete the users and
confirm they are gone.

A password may also be given as clear text, marked with a '$0$' prefix
(iana-crypt-hash).  The system then stores the hash of it.
"""
import re

import my

SYS_NS = "urn:ietf:params:xml:ns:yang:ietf-system"
MYSYS_NS = "urn:mylinux:system"
NETCONF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0"

# A valid SHA-512 crypt hash; set_password stores it verbatim, so it must
# read back byte for byte.
HASH = ("$6$testsalt00000000$uBcwY02gRpwXNzEpaF.Xs5GxMuYg/Ym2LSpA."
        "r2leu2456bQjMu1MOBgbp2lKVZyVBXgpzhpHYqNS/FTvK9Ds/")

AUTH_FILTER = f'<system xmlns="{SYS_NS}"><authentication/></system>'


def shell_of(xml, name):
    m = re.search(rf"<name>{name}</name>.*?<shell[^>]*>([^<]+)</shell>", xml, re.S)
    return m.group(1) if m else None


def password_of(xml, name):
    m = re.search(rf"<name>{name}</name>.*?<password>([^<]+)</password>", xml, re.S)
    return m.group(1) if m else None


def in_passwd(dut, name):
    rc, _ = dut.ssh(f'grep -qs "^{name}:" /etc/passwd')
    return rc == 0


@my.case
def test(dut):
    with dut.step("Add 'op1' (clish shell + password) and 'op2' (defaults)"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <system xmlns="{SYS_NS}">
                  <authentication>
                    <user>
                      <name>op1</name>
                      <password>{HASH}</password>
                      <shell xmlns="{MYSYS_NS}">clish</shell>
                    </user>
                    <user>
                      <name>op2</name>
                    </user>
                  </authentication>
                </system>
              </config>""")
            nc.commit()

    with dut.step("Confirm both UNIX accounts exist; op1 got the clish shell"):
        assert in_passwd(dut, "op1"), "op1 was not created"
        assert in_passwd(dut, "op2"), "op2 was not created"
        rc, out = dut.ssh('grep "^op1:" /etc/passwd')
        assert "/usr/bin/clish" in out, out

    with dut.step("Read the applied shells and password back from operational"):
        def ready():
            xml = dut.get_oper(AUTH_FILTER)
            return xml if shell_of(xml, "op1") else False
        xml = my.until(ready)
        assert shell_of(xml, "op1") == "clish", xml
        assert shell_of(xml, "op2") == "sh", xml
        assert password_of(xml, "op1") == HASH, xml

    with dut.step("Set the password of 'op2' as clear text"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <system xmlns="{SYS_NS}">
                  <authentication>
                    <user>
                      <name>op2</name>
                      <password>$0$clear text</password>
                    </user>
                  </authentication>
                </system>
              </config>""")
            nc.commit()

    with dut.step("Confirm running-config holds a hash of it, the one applied"):
        with dut.netconf() as nc:
            stored = password_of(nc.get_config("running", filter=("subtree", AUTH_FILTER)).xml, "op2")
        assert stored.startswith("$6$"), stored
        my.until(lambda: password_of(dut.get_oper(AUTH_FILTER), "op2") == stored)

    with dut.step("Log in as 'op2' with that password"):
        rc, out = dut.ssh("id -un", user="op2", password="clear text")
        assert rc == 0 and out.strip() == "op2", out

    with dut.step("Delete both users over NETCONF"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}" xmlns:nc="{NETCONF_NS}">
                <system xmlns="{SYS_NS}">
                  <authentication>
                    <user nc:operation="delete"><name>op1</name></user>
                    <user nc:operation="delete"><name>op2</name></user>
                  </authentication>
                </system>
              </config>""")
            nc.commit()

    with dut.step("Confirm both accounts were removed"):
        assert not in_passwd(dut, "op1"), "op1 still present after delete"
        assert not in_passwd(dut, "op2"), "op2 still present after delete"
