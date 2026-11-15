#!/usr/bin/env python3
"""RESTCONF login throttling

Whoever guesses at passwords gets a few tries, and after those has to
wait between them.  Confirm that a client with too many failed logins
behind it is turned away without a look at its password, also when it
is the right one, is told how long to wait, and gets in after that.
"""
import time

import my

HOSTNAME = "/restconf/data/ietf-system:system/hostname"
FREE_TRIES = 5


@my.case
def test(dut):
    with dut.step(f"Fail to log in {FREE_TRIES} times"):
        for _ in range(FREE_TRIES):
            res = dut.http("GET", HOSTNAME, auth=(my.USER, "wrong"))
            assert res.status == 401
            assert res.headers["Retry-After"] is None

    with dut.step("Be turned away with the right password, and told to wait"):
        res = dut.http("GET", HOSTNAME)
        assert res.status == 401
        wait = int(res.headers["Retry-After"])
        assert 0 < wait <= 30, wait

    with dut.step("Wait that long, then log in"):
        time.sleep(wait)
        assert dut.http("GET", HOSTNAME).status == 200

    with dut.step("Have the count of failed logins start over"):
        assert dut.http("GET", HOSTNAME, auth=(my.USER, "wrong")).status == 401
        assert dut.http("GET", HOSTNAME).status == 200
