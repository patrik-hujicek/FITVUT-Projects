import sys
import argparse

import error
import rpc


def main():
    parser = argparse.ArgumentParser(description='RPC client')
    parser.add_argument('--id', required=True, type=int, help='instance id')
    parser.add_argument('--peer', action='store_true', help='peer mode')
    parser.add_argument('--node', action='store_true', help='node mode')
    parser.add_argument('--command', required=True, type=str, help='command')
    base_args = sys.argv[1:6]
    parsed_args = parser.parse_args(base_args)

    cmd_args = sys.argv[6:]
    cmd_args_dict = {}
    for i, arg in enumerate(cmd_args):
        if i % 2 == 0:
            opt = arg[2:]
            if not arg.startswith("--"):
                error.fatal_error("Invalid option '%s'." % arg)
        else:
            cmd_args_dict[opt] = arg

    if len(cmd_args) % 2 != 0:
        error.fatal_error("Invalid command arguments.")

    is_peer = False
    if parsed_args.peer and not parsed_args.node:
        is_peer = True

    rpc_client = rpc.RPC(parsed_args.id, is_peer)
    success = rpc_client.init()
    if not success:
        error.fatal_error("Cannot connect to RPC server (%s %d)." % (
            ("peer" if is_peer else "node"), parsed_args.id))

    if parsed_args.command == "message":
        if "from" in cmd_args_dict and "to" in cmd_args_dict and "message" in cmd_args_dict:
            rpc_client.message(
                cmd_args_dict["from"], cmd_args_dict["to"], cmd_args_dict["message"])
        else:
            error.fatal_error("Missing arguments for command message.")
    elif parsed_args.command == "getlist":
        rpc_client.getlist()
    elif parsed_args.command == "peers":
        rpc_client.peers()
        print(rpc_client.recv_out())
    elif parsed_args.command == "reconnect":
        if "reg-ipv4" in cmd_args_dict and "reg-port" in cmd_args_dict:
            rpc_client.reconnect(
                cmd_args_dict["reg-ipv4"], cmd_args_dict["reg-port"])
        else:
            error.fatal_error("Missing arguments for command reconnect.")
    elif parsed_args.command == "database":
        rpc_client.database()
        print(rpc_client.recv_out())
    elif parsed_args.command == "neighbors":
        rpc_client.neighbors()
        print(rpc_client.recv_out())
    elif parsed_args.command == "connect":
        if "reg-ipv4" in cmd_args_dict and "reg-port" in cmd_args_dict:
            rpc_client.connect(
                cmd_args_dict["reg-ipv4"], cmd_args_dict["reg-port"])
        else:
            error.fatal_error("Missing arguments for command connect.")
    elif parsed_args.command == "disconnect":
        rpc_client.disconnect()
    elif parsed_args.command == "sync":
        rpc_client.sync()
    else:
        error.fatal_error("Unknown command '%s'." % parsed_args.command)


if __name__ == "__main__":
    main()
