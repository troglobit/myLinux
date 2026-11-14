#!/usr/bin/env python3
"""RESTCONF notifications

The web interface learns of configuration changes from the NETCONF event
stream, read as server-sent events over HTTPS.  Subscribe, change the
configuration with a RESTCONF PATCH, and confirm the netconf-config-change
notification (RFC 6470) arrives on the open stream.
"""
import my


@my.case
def test(dut):
    with dut.step("Subscribe to the NETCONF event stream"):
        stream = dut.http("GET", "/streams/NETCONF", accept="text/event-stream")
        assert stream.status == 200
        assert stream.headers["Content-Type"].startswith("text/event-stream")

    with dut.step("Change the system contact with a PATCH"):
        res = dut.http("PATCH", "/restconf/data/ietf-system:system",
                       body={"ietf-system:system": {"contact": "notify"}})
        assert res.status == 204

    with dut.step("Receive netconf-config-change on the stream"):
        event = ""
        while "</notification>" not in event:
            line = stream.readline().decode()
            assert line, "event stream closed"
            event += line
        assert "<netconf-config-change" in event
        assert "<datastore>running</datastore>" in event
        stream.close()
