from pydrpc.drpc_client import *


class basic_request:
    name: str = ""
    x: int = 0

    def __bytes__(self) -> bytes:
        return self.name.encode("ascii") + b"\x00" * (100 - len(self.name)) + self.x.to_bytes(4, "big")

    pass


class basic_reply:
    host: int = 0
    status: int = 0

    def __bytes__(self) -> bytes:
        return self.host.to_bytes(4, "big") + self.status.to_bytes(4, "big")

    def serialize(self, data: bytes):
        self.host = int.from_bytes(data[:4], "little")
        self.status = int.from_bytes(data[4:], "little")
        pass

    pass


def main():
    breq = basic_request()
    breq.name = "test python client"
    brep = basic_reply()
    req = drpc_arg_wrapper(breq)
    rep = drpc_arg_wrapper(brep)
    c = drpc_client()

    m = drpc_msg()
    m.target = "foo"
    m.req = req
    m.rep = rep

    dh = drpc_host()
    dh.hostname = "localhost"
    dh.port = 8021

    while brep.status == 0:
        err = c.Call(dh, m)
        if err == -1:
            break
        pass
    
    assert m.rep.args.status == 0xf
    print("passed!")
    pass


# pytest hook
def test_basic():
    main()


if __name__ == "__main__":
    main()
