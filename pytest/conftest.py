import os
import pytest

BINDIR = "./bin"


@pytest.fixture(autouse=True)
def run_around_tests():
    os.environ['LD_LIBRARY_PATH'] = '/home/dokastho/code/drpc'
    os.system('make all')
    yield
