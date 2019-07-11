import os
import socket


class RPC:
    def __init__(self, id, is_peer):
        self.tcp_socket = None
        self.host = None
        self.port = None
        filename = None

        if is_peer:
            filename = "peer_" + str(id) + ".rpc"
        else:
            filename = "node_" + str(id) + ".rpc"

        if os.path.isfile(filename):
            with open(filename, "r") as f:
                line = f.readline()
                data = line.split(':')
                self.host = data[0]
                self.port = int(data[1])

        self.id = id
        self.is_peer = is_peer

    def init(self):
        if self.host is None or self.port is None:
            return False

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            s.connect((self.host, self.port))
            self.tcp_socket = s
            return True
        except Exception:
            return False

    def __del__(self):
        if self.tcp_socket is not None:
            self.tcp_socket.close()

    def send_cmd(self, cmd):
        cmd = cmd + "$$"
        return self.tcp_socket.sendall(cmd.encode())

    def recv_out(self):
        recv_data = ""
        while True:
            data = self.tcp_socket.recv(1024)
            if not data:
                break
            recv_data += data.decode()

        return recv_data

    def message(self, author, to, message):
        self.send_cmd("message:" + author + ":" + to + ":" + message)

    def getlist(self):
        self.send_cmd("getlist")

    def peers(self):
        self.send_cmd("peers")

    def reconnect(self, ip, port):
        self.send_cmd("reconnect:" + ip + ":" + port)

    def database(self):
        self.send_cmd("database")

    def neighbors(self):
        self.send_cmd("neighbors")

    def connect(self, ip, port):
        self.send_cmd("connect:" + ip + ":" + port)

    def disconnect(self):
        self.send_cmd("disconnect")

    def sync(self):
        self.send_cmd("sync")
