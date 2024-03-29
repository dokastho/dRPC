import os
from conftest import BINDIR

def run_command(cmd: str):
    x = os.system(f'{BINDIR}/{cmd}')
    return (x & 0xff00) >> 8


def test_basic():
    cmd = 'test_basic'
    assert(run_command(cmd) == 0)
    pass


def test_many():
    cmd = 'test_many'
    assert(run_command(cmd) == 0)
    pass


def test_concurrent():
    cmd = 'test_concurrent'
    assert(run_command(cmd) == 0)
    pass


def test_unreliable():
    cmd = 'test_unreliable'
    assert(run_command(cmd) == 0)
    pass


def test_kill():
    cmd = 'test_kill'
    assert(run_command(cmd) == 0)
    pass
