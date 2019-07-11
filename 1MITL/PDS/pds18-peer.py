import os
import sys
import argparse
import socket
import signal
import threading
import bencode
import time
import error

# Time constants
LIST_WAIT_SECONDS = 2.0
ACK_WAIT_SECONDS = 2.0
REPEAT_HELLO_SECONDS = 10.0

# Variables shared by threads and timers
g_id = None
g_reg_ip = None
g_reg_port = None
g_chat_ip = None
g_chat_port = None
g_username = None
g_rpc_listener_run = True
g_chat_listener_run = True
g_rpc_send_list_peers = False
g_seq_number = 0
g_ACK = {}
g_hello_timer = None
g_getlist_timer = None
g_peers_timer = None
g_message_timer = None
g_rpc_client = None
g_peers_dict_cache = None
g_peer = None
g_list_new = None


def inc_seq_num():
    # Increase sequence number, wrap it to unsigned short range
    global g_seq_number
    g_seq_number = (g_seq_number + 1) % 65536


def getlist():
    global g_ACK, g_seq_number, g_getlist_timer, g_peer
    m = {}
    m["type"] = "getlist"
    m["txid"] = g_seq_number
    hello_msg = bencode.bencode(m)
    # Send GETLIST
    g_peer.sendto(hello_msg.encode(), ((g_reg_ip, g_reg_port)))
    g_ACK[g_seq_number] = False

    def check_ack(seq_number):
        global g_list_new
        if g_ACK[seq_number] == False:
            sys.stderr.write(
                "No ACK for GETLIST with seq num %d.\n" % seq_number)
            g_list_new = False

    g_getlist_timer = threading.Timer(
        ACK_WAIT_SECONDS, check_ack, [g_seq_number])
    g_getlist_timer.start()
    inc_seq_num()


def peers():
    global g_ACK, g_seq_number, g_peers_timer, g_rpc_send_list_peers, g_peer
    # Indicate that we must send received LIST to RPC
    g_rpc_send_list_peers = True
    m = {}
    m["type"] = "getlist"
    m["txid"] = g_seq_number
    hello_msg = bencode.bencode(m)
    # Send GETLIST
    g_peer.sendto(hello_msg.encode(), ((g_reg_ip, g_reg_port)))
    g_ACK[g_seq_number] = False

    def check_ack(seq_number):
        global g_list_new
        if g_ACK[seq_number] == False:
            sys.stderr.write(
                "No ACK for GETLIST in PEERS with seq num %d.\n" % seq_number)
            g_list_new = False

    g_peers_timer = threading.Timer(
        ACK_WAIT_SECONDS, check_ack, [g_seq_number])
    g_peers_timer.start()

    inc_seq_num()


def disconnect():
    global g_seq_number, g_reg_ip, g_reg_port, g_username, g_peer
    #  Say goodbye
    m = {}
    m["type"] = "hello"
    m["username"] = g_username
    m["ipv4"] = "0.0.0.0"
    m["port"] = 0
    m["txid"] = g_seq_number
    hello_msg = bencode.bencode(m)
    g_peer.sendto(hello_msg.encode(), ((g_reg_ip, g_reg_port)))

    inc_seq_num()


def reconnect(ip, port):
    global g_seq_number, g_reg_ip, g_reg_port, g_chat_ip, g_chat_port, g_peer
    # Disconnect
    disconnect()
    # Reconnect, ie. send HELLO message to the new reg node
    g_reg_ip = ip
    g_reg_port = port
    m = {}
    m["type"] = "hello"
    m["username"] = g_username
    m["ipv4"] = g_chat_ip
    m["port"] = g_chat_port
    m["txid"] = g_seq_number
    hello_msg = bencode.bencode(m)
    g_peer.sendto(hello_msg.encode(), ((g_reg_ip, g_reg_port)))
    inc_seq_num()


def send_ACK(txid, ip, port):
    global g_peer
    # Send ACK
    m = {}
    m["type"] = "ack"
    m["txid"] = txid
    ack_msg = bencode.bencode(m)
    g_peer.sendto(ack_msg.encode(), ((ip, port)))


def send_error(message, txid, ip, port):
    global g_peer
    # Send ERROR
    m = {}
    m["type"] = "error"
    m["txid"] = txid
    m["verbose"] = message
    error_msg = bencode.bencode(m)
    g_peer.sendto(error_msg.encode(), ((ip, port)))

    sys.stderr.write("%s\n" % message)


def send_message(sender, receiver, message):
    global g_ACK, g_seq_number, g_reg_ip, g_reg_port, g_username, g_peers_dict_cache, g_list_new
    # Username mismatch check
    if g_username != sender:
        sys.stderr.write("Username mismatch. I am %s, but you named me as %s. Sender of this message will be set as %s.\n" % (
            g_username, sender, sender))
    receiver_ip = None
    receiver_port = None
    if g_peers_dict_cache:
        # Find receiver in local pairs cache
        for _, peer_info in g_peers_dict_cache.items():
            if peer_info["username"] == receiver:
                receiver_ip = peer_info["ipv4"]
                receiver_port = peer_info["port"]
    # We didnt find receiver, try refresh local peers cache
    if receiver_ip is None:
        g_list_new = None
        getlist()
        # Chat thread sets g_list_new if new list arrives
        while g_list_new is None:
            time.sleep(0.1)
        # Search again
        if not g_peers_dict_cache:
            # No received LIST, finish
            return
        for _, peer_info in g_peers_dict_cache.items():
            if peer_info["username"] == receiver:
                receiver_ip = peer_info["ipv4"]
                receiver_port = peer_info["port"]

        if receiver_ip is None:
            sys.stderr.write(
                "List of peers was refreshed, but user was not found.\n")
            return

    # Create MESSAGE
    m = {}
    m["type"] = "message"
    m["from"] = sender
    m["to"] = receiver
    m["message"] = message
    m["txid"] = g_seq_number
    msg = bencode.bencode(m)
    # Send MESSAGE
    try:
        g_peer.sendto(msg.encode(), ((receiver_ip, receiver_port)))
    except Exception:
        sys.stderr.write("Unable to send message. Message is too long.\n")
        return
    g_ACK[g_seq_number] = False

    def check_ack(seq_number):
        if g_ACK[seq_number] == False:
            sys.stderr.write(
                "No ACK for MESSAGE with seq num %d.\n" % seq_number)

    g_message_timer = threading.Timer(
        ACK_WAIT_SECONDS, check_ack, [g_seq_number])
    g_message_timer.start()

    inc_seq_num()


def signal_handler(signum, frame):
    global g_rpc_listener_run, g_chat_listener_run, g_list_new, g_hello_timer, g_getlist_timer, g_peers_timer, g_peer
    # Remove file with connection info
    if os.path.exists("peer_" + str(g_id) + ".rpc"):
        os.remove("peer_" + str(g_id) + ".rpc")
    # Stop timers
    if g_hello_timer is not None:
        g_hello_timer.cancel()
    if g_getlist_timer is not None:
        g_getlist_timer.cancel()
    if g_message_timer is not None:
        g_message_timer.cancel()
    if g_peers_timer is not None:
        g_peers_timer.cancel()
    # Stop waiting loops
    g_list_new = False
    g_rpc_listener_run = False
    g_chat_listener_run = False
    # Disconnect from reg node
    disconnect()
    # Close socket
    g_peer.close()
    sys.exit(0)


def rpc_listener():
    global g_rpc_listener_run
    global g_rpc_client
    global g_list_new
    rpc_control_file = "peer_" + str(g_id) + ".rpc"
    if os.path.exists(rpc_control_file):
        os.remove(rpc_control_file)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Find free port
    s.bind(('127.0.0.1', 0))
    s.listen()
    s.settimeout(0.1)

    ip_port = s.getsockname()[0] + ":" + str(s.getsockname()[1])
    # Write peer's IP and port to file, so RPC client can connect to this peer
    with open(rpc_control_file, 'w') as file:
        file.write(ip_port)

    # Wait for RPC client
    while g_rpc_listener_run:
        try:
            g_rpc_client, _ = s.accept()
        except socket.timeout:
            continue
        cmd = ""
        # Handle RPC commands
        while True:
            data = g_rpc_client.recv(1024)
            if not data:
                break
            cmd += data.decode()
            if cmd == "getlist$$":
                getlist()
                g_rpc_client.close()
                break
            elif cmd == "peers$$":
                g_list_new = None
                peers()
                # Chat thread sets g_list_new if new list arrives
                while g_list_new is None:
                    time.sleep(0.1)
                g_rpc_client.close()
                break
            elif cmd.startswith("reconnect") and cmd.endswith("$$"):
                cmd = cmd[len("reconnect") + 1:-2].split(":")
                ip = cmd[0]
                port = int(cmd[1])
                reconnect(ip, port)
                g_rpc_client.close()
                break
            elif cmd.startswith("message") and cmd.endswith("$$"):
                cmd = cmd[len("message") + 1:-2].split(":")
                send_message(cmd[0], cmd[1], cmd[2])
                g_rpc_client.close()
                break
            else:
                break


def main():
    global g_id
    global g_reg_ip
    global g_reg_port
    global g_chat_ip
    global g_chat_port
    global g_username
    global g_ACK
    global g_rpc_client
    global g_rpc_send_list_peers
    global g_peers_dict_cache
    global g_peer
    global g_list_new
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='pds18-peer')
    parser.add_argument('--id', required=True, type=int,
                        help='Idenitificator of peer')
    parser.add_argument('--username', required=True,
                        type=str, help='Username of peer')
    parser.add_argument('--chat-ipv4', required=True,
                        type=str, help='Listening IP for chat')
    parser.add_argument('--chat-port', required=True,
                        type=int, help='Listening port for chat')
    parser.add_argument('--reg-ipv4', required=True,
                        type=str, help='IP of registration node')
    parser.add_argument('--reg-port', required=True, type=int,
                        help='Port of of registration node')
    parsed_args = parser.parse_args()

    g_id = parsed_args.id
    g_reg_ip = parsed_args.reg_ipv4
    g_reg_port = parsed_args.reg_port
    g_chat_ip = parsed_args.chat_ipv4
    g_chat_port = parsed_args.chat_port
    g_username = parsed_args.username

    # Register SIGINT handler
    signal.signal(signal.SIGINT, signal_handler)

    # Open UDP socket
    g_peer = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        g_peer.bind((g_chat_ip, g_chat_port))
        g_peer.settimeout(0.1)
    except:
        error.fatal_error("Unable to bind to IP " + parsed_args.reg_ipv4 +
                          " and port " + str(parsed_args.reg_port))

    # Start RPC listener thread
    t1 = threading.Thread(target=rpc_listener)
    t1.start()

    def say_hello():
        global g_hello_timer, g_peer, g_reg_ip, g_reg_port
        m = {}
        m["type"] = "hello"
        m["username"] = g_username
        m["ipv4"] = g_chat_ip
        m["port"] = g_chat_port
        m["txid"] = g_seq_number
        hello_msg = bencode.bencode(m)
        # Repeat HELLO
        g_peer.sendto(hello_msg.encode(), ((g_reg_ip, g_reg_port)))
        inc_seq_num()
        g_hello_timer = threading.Timer(REPEAT_HELLO_SECONDS, say_hello)
        g_hello_timer.start()

    # Send first HELLO
    say_hello()

    while g_chat_listener_run:
        try:
            protocol_data, address = g_peer.recvfrom(65565)
            dst_ip = address[0]
            dst_port = address[1]
        except socket.timeout:
            continue
        # Try to decode received data
        try:
            m = bencode.bdecode(protocol_data.decode())
        except Exception:
            send_error("Unable to decode message.", 0, dst_ip, dst_port)
            continue

        # Check if message is OK
        if "type" not in m or "txid" not in m:
            send_error("Malformed protocol message.", 0, dst_ip, dst_port)
            continue

        # Check seq number
        if m["txid"] not in range(0, 65536):
            send_error("Sequence number is out of allowed unsigned short range.", 0, dst_ip, dst_port)
            continue

        if m["type"] == "error":
            if "verbose" not in m:
                send_error("Malformed ERROR - missing verbose field.",
                           m["txid"], dst_ip, dst_port)
                continue
            sys.stderr.write("Error: %s\n" % m["verbose"])
            continue

        if m["type"] == "ack":
            g_ACK[m["txid"]] = True
            continue

        if m["type"] == "list":
            send_ACK(m["txid"], dst_ip, dst_port)
            if "peers" not in m:
                send_error("Malformed LIST - missing peers field.",
                           m["txid"], dst_ip, dst_port)
                continue
            if g_rpc_send_list_peers:
                g_rpc_client.sendall(str(m["peers"]).encode())
                g_rpc_send_list_peers = False

            # We received LIST, use it as a local cache
            g_peers_dict_cache = m["peers"]
            # Inform RPC thread that a new LIST was received
            g_list_new = True
            continue

        if m["type"] == "message":
            if "from" not in m or "message" not in m:
                send_error("Malformed MESSAGE - missing required fields.",
                           m["txid"], dst_ip, dst_port)
                continue
            print("Message from %s: %s" % (m["from"], m["message"]))
            send_ACK(m["txid"], dst_ip, dst_port)
            continue

        else:
            send_error("Unknown type of command.", m["txid"], dst_ip, dst_port)
            continue


if __name__ == "__main__":
    main()
