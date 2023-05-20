import sys
import socket

DEFAULT_TIMEOUT = 3000


class drpc_arg_wrapper:
    args = None
    size: int = 0

    def __init__(self, args):
        self.args = args
        self.size = len(bytes(self.args))

    def __bytes__(self) -> bytes:
        return self.size.to_bytes(8, "big") + bytes(self.args)

    def __repr__(self) -> str:
        return f'{bytes(self.args)}\n{self.size}'
    pass


class drpc_msg:
    target: str = ""
    req: drpc_arg_wrapper = None
    rep: drpc_arg_wrapper = None
    pass


class drpc_host:
    port = 0
    hostname = ""
    pass


class drpc_client:
    def __init__(self, timeout_val=DEFAULT_TIMEOUT):
        self.timeout_val = timeout_val
        pass

    def Call(self, dh: drpc_host, m: drpc_msg):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((dh.hostname, dh.port))
            # send RPC
            # target function
            target_str_len = len(m.target)
            sock.sendall(target_str_len.to_bytes(8, "little"))
            sock.sendall(m.target.encode("ascii"))
            # request args
            sock.sendall(m.req.size.to_bytes(8, "little"))
            sock.sendall(bytes(m.req.args))
            # reply
            sock.sendall(m.rep.size.to_bytes(8, "little"))
            sock.sendall(bytes(m.rep.args))
            # checksum
            # todo

            sock.settimeout(self.timeout_val)
            # receive RPC reply
            # reply
            b_size = sock.recv(8, socket.MSG_WAITALL)
            m.rep.size = int.from_bytes(b_size, "little")
            m.rep.args.serialize(sock.recv(m.rep.size, socket.MSG_WAITALL))

        pass
