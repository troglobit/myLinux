# myLinux tests

Stand-alone, self-booting tests: `make test` boots a throwaway qemu DUT, runs
every case under `test/case/`, and tears it down again.  There is no need to
`make run` first.

Host requirements:

 - python3
 - ncclient

On a Debian/Ubuntu/Linux Mint system:

```bash
$ sudo apt install python3-ncclient
```

## Run

```bash
$ make test          # boot a DUT, run all cases, report pass/fail
$ make test-sh       # boot a DUT and open an interactive test shell
```

The DUT boots clean from factory each run (a throwaway config image), so test
cases start from a known state.  Its console is captured to a log whose path
is printed at start-up -- handy when a boot fails.

## Developing tests interactively

`make test-sh` boots one DUT and drops you into a shell in `test/` with that
DUT kept running and *shared* -- so you can iterate without rebooting:

```bash
$ ./case/system/hostname.py    # run one case against the running DUT
$ ./run.py                     # run all cases against it
$ ./console                    # attach to its serial console (Ctrl-] detaches)
$ ssh -p 2222 admin@localhost  # log in (admin/admin)
```

The DUT is shut down when you exit the shell.  Each case is also runnable on
its own outside `test-sh` (`./case/system/hostname.py`), in which case it
boots its own throwaway DUT just for that run.

## Writing a test case

Test cases live under `test/case/<model>/<name>.py`.  `import my`, and mark the
`test(dut)` function with `@my.case`; use `dut.netconf()` for an ncclient
session:

```python
#!/usr/bin/env python3
import my

@my.case
def test(dut):
    with dut.netconf() as nc:
        cfg = nc.get_config(source="running").data_xml
        assert "<name>admin</name>" in cfg
```

Raise (e.g. via `assert`) to fail; return normally to pass.  The `@my.case`
decorator is all the boilerplate there is: run the file directly and it boots
(or, inside `make test-sh`, attaches to) a DUT and reports; under the runner
it is just an ordinary `test(dut)`.

## Steps

Wrap each stage in `dut.step("...")`.  The descriptions are live progress, a
machine-readable [TAP][] report, and -- read together with the docstring --
the manual procedure for the test:

```python
@my.case
def test(dut):
    with dut.netconf() as nc:
        with dut.step("Set the hostname to 'testhost' over NETCONF"):
            nc.edit_config(target="candidate", config=EDIT)
            nc.commit()
        with dut.step("Confirm the new hostname in running-config"):
            assert "<hostname>testhost</hostname>" in \
                nc.get_config(source="running").data_xml
```

Output is TAP: `ok N - <desc>` / `not ok N - <desc>` per step, then a `1..N`
plan; plain `print()` output and the case name are `#` diagnostics.

[TAP]: https://testanything.org/tap-specification.html
