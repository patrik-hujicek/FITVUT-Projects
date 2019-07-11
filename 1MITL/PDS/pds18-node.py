import sys
import os
import argparse
import socket
import threading
import signal
import bencode
import error

ACK_WAIT_SECONDS = 2.0
REPEAT_UPDATE_SECONDS = 4.0
NODE_LIVENESS_CHECK_SECONDS = 12.0
PEER_LIVENESS_CHECK_SECONDS = 30.0

g_id = None
g_node = None
g_reg_ip = None
g_reg_port = None
g_node_run = True
g_rpc_listener_run = True
g_rpc_client = None
g_seq_number = 0
g_ACK = {}
g_neighbours = []
g_database = {}
g_update_timers = {}
g_disconnect_timers = {}
g_dead_node_timers = {}
g_list_timer = None
g_my_key = None
g_dead_peer_timers = {}


def inc_seq_num():
    # Increase sequence number, wrap it to unsigned short range
    global g_seq_number
    g_seq_number = (g_seq_number + 1) % 65536


def send_ACK(txid, ip, port):
    global g_node
    # Send ACK
    m = {}
    m["type"] = "ack"
    m["txid"] = txid
    ack_msg = bencode.bencode(m)
    g_node.sendto(ack_msg.encode(), ((ip, port)))


def send_error(message, txid, ip, port):
    global g_node
    # Send ERROR
    m = {}
    m["type"] = "error"
    m["txid"] = txid
    m["verbose"] = message
    error_msg = bencode.bencode(m)
    g_node.sendto(error_msg.encode(), ((ip, port)))

    sys.stderr.write("%s\n" % message)


def remove_node(ip_port):
    global g_neighbours
    global g_database
    global g_update_timers

    # Remove node from my DB, list of neighbours
    if ip_port in g_neighbours:
        g_neighbours.remove(ip_port)
        g_database.pop(ip_port)
        g_update_timers[ip_port].cancel()


def get_list():
    global g_database
    peers = {}
    cnt = 0
    # Create peers in LIST message
    for _, node_peers in g_database.items():
        for peer in node_peers:
            peers[str(cnt)] = peer
            cnt = cnt + 1
    return peers


def send_list(txid, ip, port):
    global g_list_timer
    m = {}
    # Send LIST
    m["type"] = "list"
    m["txid"] = txid
    m["peers"] = get_list()

    # Mark as not ACK
    g_ACK[txid] = False

    def check_ack(seq_number):
        if g_ACK[seq_number] == False:
            sys.stderr.write(
                "No ACK for LIST with seq num %d.\n" % seq_number)

    list_msg = bencode.bencode(m)
    g_node.sendto(list_msg.encode(), ((ip, port)))

    g_list_timer = threading.Timer(ACK_WAIT_SECONDS, check_ack, [txid])
    g_list_timer.start()


def manage_peer(username, ip, port):
    global g_database, g_reg_ip, g_reg_port, g_dead_peer_timers
    changed = False
    # Remove peer
    if ip == "0.0.0.0" and port == 0:
        # Create new list of peers except this one
        peers = g_database[g_my_key]
        new_peers = []
        for p in peers:
            if p["username"] != username:
                new_peers.append(p)
        g_database[g_my_key] = new_peers
        if username in g_dead_peer_timers:
            # Cancel timer for this peer
            g_dead_peer_timers[username].cancel()
        changed = True
    else:
        # Maybe update peer
        exists = False
        for p in g_database[g_my_key]:
            # Update record for existing peer
            if p["username"] == username:
                exists = True
                if p["ipv4"] != ip:
                    p["ipv4"] = ip
                    changed = True
                if p["port"] != port:
                    p["port"] = port
                    changed = True

        # New peer
        if not exists:
            new_peer = {}
            new_peer["username"] = username
            new_peer["ipv4"] = ip
            new_peer["port"] = port
            g_database[g_my_key].append(new_peer)
            changed = True

        if username in g_dead_peer_timers:
            g_dead_peer_timers[username].cancel()

        g_dead_peer_timers[username] = threading.Timer(
            PEER_LIVENESS_CHECK_SECONDS, manage_peer, [username, "0.0.0.0", 0])
        g_dead_peer_timers[username].start()
    return changed


def get_db_for_update():
    global g_database
    update_db = {}
    # Create database for UPDATE message
    for node_ip_port, node_peers in g_database.items():
        update_db[node_ip_port] = {}
        cnt = 0
        for peer in node_peers:
            update_db[node_ip_port][str(cnt)] = peer
            cnt = cnt + 1
    return update_db


def send_update(ip, port):
    global g_seq_number, g_update_timers, g_node
    key = ip + "," + port
    m = {}
    # Send UPDATE
    m["type"] = "update"
    m["txid"] = g_seq_number
    m["db"] = get_db_for_update()
    msg = bencode.bencode(m)
    g_node.sendto(msg.encode(), ((ip, int(port))))
    inc_seq_num()
    # Repeat update every 4 seconds
    g_update_timers[key] = threading.Timer(
        REPEAT_UPDATE_SECONDS, send_update, [ip, port])
    g_update_timers[key].start()


def connect(ip, port):
    global g_neighbours, g_database
    key = ip + "," + port
    # Check whether node is not my neighbour actually
    if key not in g_neighbours:
        # If not, connect to this new node
        g_neighbours.append(key)
        g_database[key] = []
        send_update(ip, port)


def refresh_db(db, ip, port):
    global g_neighbours
    global g_database
    global g_reg_ip
    global g_reg_port
    global g_my_key
    # Check if neighbour
    key = ip + "," + str(port)
    if key not in db:
        # Unknown node, has no peers, try to connect
        g_database[key] = []
        connect(ip, str(port))
        return
    g_database[key] = []
    for _, peer in db[key].items():
        g_database[key].append(peer)
    # Check for new neighbours, connect if new neighbour
    for node_ip_port, _ in db.items():
        if node_ip_port not in g_neighbours and node_ip_port != g_my_key:
            ip_port = node_ip_port.split(",")
            connect(ip_port[0], ip_port[1])


def sync():
    global g_neighbours, g_seq_number, g_node, g_update_timers
    # Send UPDATE to all neighbours
    for n in g_neighbours:
        ip_port = n.split(",")
        ip = ip_port[0]
        port = ip_port[1]
        # Cancel current UPDATE timers
        g_update_timers[n].cancel()
        # Send UPDATE
        send_update(ip, port)


def disconnect(from_rpc):
    global g_neighbours, g_database, g_update_timers, g_reg_ip, g_reg_port, g_ACK
    # Drop all records except my peers
    my_peers = g_database[g_my_key]
    g_database = {}
    g_database[g_my_key] = my_peers

    def check_ack(seq_number):
        if g_ACK[seq_number] == False:
            sys.stderr.write(
                "No ACK for DISCONNECT with seq num %d.\n" % seq_number)

    # Send DISCONNECT to all neighbours
    for n in g_neighbours:
        global g_seq_number, g_disconnect_timers, g_node
        m = {}
        m["type"] = "disconnect"
        m["txid"] = g_seq_number
        g_ACK[g_seq_number] = False
        msg = bencode.bencode(m)
        ip_port = n.split(",")
        g_node.sendto(msg.encode(), ((ip_port[0], int(ip_port[1]))))
        if from_rpc:
            g_disconnect_timers[n] = threading.Timer(
                ACK_WAIT_SECONDS, check_ack, [g_seq_number])
            g_disconnect_timers[n].start()
        inc_seq_num()

    # Ok, this node have no neighbours
    g_neighbours = []
    # Turn off UPDATE timers
    for _, timer in g_update_timers.items():
        timer.cancel()


def send_neighbours():
    global g_rpc_client
    global g_neighbours
    # Send list of neighbours to RPC
    data = '\n'.join(g_neighbours)
    g_rpc_client.sendall(data.encode())


def send_database():
    global g_rpc_client
    global g_database
    data = ""
    # Send current database of nodes with their peers to RPC
    for key, node_peers in g_database.items():
        data += key + ":\n"
        for peer in node_peers:
            data += str(peer) + "\n"

    g_rpc_client.sendall(data.encode())


def rpc_listener():
    global g_rpc_listener_run
    global g_rpc_client
    rpc_control_file = "node_" + str(g_id) + ".rpc"
    if os.path.exists(rpc_control_file):
        os.remove(rpc_control_file)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Find free port
    s.bind(('127.0.0.1', 0))
    s.listen()
    s.settimeout(0.1)

    ip_port = s.getsockname()[0] + ":" + str(s.getsockname()[1])
    # Write node's IP and port to file, so RPC client can connect to this node
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
            if cmd == "neighbors$$":
                send_neighbours()
                g_rpc_client.close()
                break
            elif cmd == "database$$":
                send_database()
                g_rpc_client.close()
                break
            elif cmd == "sync$$":
                sync()
                g_rpc_client.close()
                break
            elif cmd == "disconnect$$":
                disconnect(True)
                g_rpc_client.close()
                break
            elif cmd.startswith("connect") and cmd.endswith("$$"):
                cmd = cmd[len("connect") + 1:-2].split(":")
                ip = cmd[0]
                port = cmd[1]
                connect(ip, port)
                g_rpc_client.close()
                break


def signal_handler(signum, frame):
    global g_rpc_listener_run, g_update_timers, g_disconnect_timers, g_dead_peer_timers, g_dead_node_timers, g_list_timer, g_node_run, g_node
    # Remove file with connection info
    if os.path.exists("node_" + str(g_id) + ".rpc"):
        os.remove("node_" + str(g_id) + ".rpc")
    # Stop waiting loops
    g_rpc_listener_run = False
    g_node_run = False
    disconnect(False)
    # Turn off all timers
    for _, timer in g_update_timers.items():
        timer.cancel()
    for _, timer in g_disconnect_timers.items():
        timer.cancel()
    for _, timer in g_dead_peer_timers.items():
        timer.cancel()
    for _, timer in g_dead_node_timers.items():
        timer.cancel()
    if g_list_timer is not None:
        g_list_timer.cancel()
    # Close socket
    g_node.close()
    sys.exit(0)


def main():
    global g_id
    global g_reg_ip
    global g_reg_port
    global g_database
    global g_node_run
    global g_node
    global g_ACK
    global g_neighbours
    global g_dead_node_timers
    global g_my_key
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='pds18-node')
    parser.add_argument('--id', required=True, type=int,
                        help='Identificator of node')
    parser.add_argument('--reg-ipv4', required=True,
                        type=str, help='Listining IPv4 address')
    parser.add_argument('--reg-port', required=True,
                        type=int, help='Listining port')
    parsed_args = parser.parse_args()

    g_id = parsed_args.id
    # Register SIGINT handler
    signal.signal(signal.SIGINT, signal_handler)

    # Open UDP socket
    g_node = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        g_node.bind((parsed_args.reg_ipv4, parsed_args.reg_port))
        g_node.settimeout(0.1)
    except:
        error.fatal_error("Unable to bind to IP " + parsed_args.reg_ipv4 +
                          " and port " + str(parsed_args.reg_port))

    # Start RPC listener thread
    t1 = threading.Thread(target=rpc_listener)
    t1.start()

    g_reg_ip = parsed_args.reg_ipv4
    g_reg_port = str(parsed_args.reg_port)
    g_my_key = g_reg_ip + "," + g_reg_port

    # Start with empty db, empty list of neighbours
    g_database = {}
    g_database[g_my_key] = []
    g_neighbours = []

    while g_node_run:
        try:
            protocol_data, address = g_node.recvfrom(65565)
            dst_ip = address[0]
            dst_port = address[1]
            dst_key = dst_ip + "," + str(dst_port)
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

        elif m["type"] == "ack":
            g_ACK[m["txid"]] = True

        elif m["type"] == "hello":
            if "username" not in m or "ipv4" not in m or "port" not in m:
                send_error("Malformed HELLO - missing required fields.",
                           m["txid"], dst_ip, dst_port)
                continue
            if manage_peer(m["username"], m["ipv4"], m["port"]):
                # DB changed, force sync
                sync()

        elif m["type"] == "update":
            if "db" not in m:
                send_error("Malformed UPDATE - missing db field.",
                           m["txid"], dst_ip, dst_port)
                continue
            if dst_key in g_neighbours:
                if dst_key in g_dead_node_timers:
                    # Ok, node is alive, cancel timer
                    g_dead_node_timers[dst_key].cancel()

                g_dead_node_timers[dst_key] = threading.Timer(
                    NODE_LIVENESS_CHECK_SECONDS, remove_node, [dst_key])
                g_dead_node_timers[dst_key].start()

            # Refresh database
            refresh_db(m["db"], dst_ip, dst_port)

        elif m["type"] == "getlist":
            send_ACK(m["txid"], dst_ip, dst_port)
            is_my_peer = False
            for p in g_database[g_my_key]:
                if p["ipv4"] == dst_ip and p["port"] == dst_port:
                    is_my_peer = True
            if not is_my_peer:
                send_error(
                    "Unable to send LIST. Requestor is not registered to me.", m["txid"], dst_ip, dst_port)
                continue
            send_list(m["txid"], dst_ip, dst_port)
        elif m["type"] == "disconnect":
            send_ACK(m["txid"], dst_ip, dst_port)
            remove_node(dst_key)
            if dst_key in g_dead_node_timers:
                g_dead_node_timers[dst_key].cancel()
        else:
            send_error("Unknown type of command.", m["txid"], dst_ip, dst_port)
            continue


if __name__ == "__main__":
    main()
