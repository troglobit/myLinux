#!/usr/bin/env python3
"""NTP client (sntpd)

Configure NTP servers over NETCONF and confirm mech drives sntpd as the
system NTP client: the per-server settings land in the sntpd env file,
the daemon runs with that failover list, its live state reads back
through the operational datastore, the stored config starts it again on
boot, and clearing 'enabled' stops it.
"""
import my

SYS_NS = "urn:ietf:params:xml:ns:yang:ietf-system"
NETCONF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0"


def sntpd_cmdline(dut):
    """The running sntpd's argv, space-joined (empty if not running)."""
    _, out = dut.ssh("pid=$(pgrep -n sntpd) && "
                     "cat /proc/$pid/cmdline 2>/dev/null | tr '\\0' ' '")
    return out


def sntpd_running(dut):
    cmd = sntpd_cmdline(dut)
    return cmd if "192.0.2.1" in cmd else False


CONF = f"""
  <config xmlns="{NETCONF_NS}">
    <system xmlns="{SYS_NS}">
      <ntp>
        <enabled>true</enabled>
        <server>
          <name>local</name>
          <udp><address>192.0.2.1</address><port>1234</port></udp>
          <iburst>true</iburst>
          <prefer>true</prefer>
        </server>
        <server>
          <name>pool</name>
          <udp><address>pool.ntp.org</address></udp>
        </server>
      </ntp>
    </system>
  </config>"""


@my.case
def test(dut):
    with dut.step("Configure two NTP servers over NETCONF"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=CONF)
            nc.commit()

    with dut.step("mech rendered the server list into the sntpd env file"):
        def envfile():
            _, out = dut.ssh("cat /etc/default/sntpd")
            return out if "192.0.2.1" in out else False
        out = my.until(envfile)
        assert "192.0.2.1:1234,iburst,prefer" in out, out
        assert "pool.ntp.org" in out, out

    with dut.step("sntpd runs as the NTP client with that failover list"):
        cmd = my.until(lambda: sntpd_running(dut))
        assert "192.0.2.1:1234,iburst,prefer" in cmd, cmd
        assert "pool.ntp.org" in cmd, cmd

    with dut.step("The live server list reads back from operational"):
        xml = dut.get_oper(f'<system xmlns="{SYS_NS}"><ntp/></system>')
        assert "<address>192.0.2.1</address>" in xml, xml
        assert "<port>1234</port>" in xml, xml
        assert "<iburst>true</iburst>" in xml, xml
        assert "<prefer>true</prefer>" in xml, xml
        assert "<address>pool.ntp.org</address>" in xml, xml

    with dut.step("Stored config starts sntpd again on boot"):
        dut.copy("running", "startup")
        dut.reboot()
        cmd = my.until(lambda: sntpd_running(dut))
        assert "192.0.2.1:1234,iburst,prefer" in cmd, cmd

    with dut.step("Clearing 'enabled' stops sntpd"):
        with dut.netconf() as nc:
            nc.edit_config(target="candidate", config=f"""
              <config xmlns="{NETCONF_NS}">
                <system xmlns="{SYS_NS}">
                  <ntp><enabled>false</enabled></ntp>
                </system>
              </config>""")
            nc.commit()
        my.until(lambda: dut.ssh("pgrep sntpd")[0] != 0)
