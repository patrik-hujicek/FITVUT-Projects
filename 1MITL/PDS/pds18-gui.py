import rpc
import ast
from tkinter import scrolledtext
from tkinter import messagebox
from tkinter import Tk
from tkinter import END
from tkinter.ttk import *


def refresh_peers(sender_peer_id_str, peers_box):
    if sender_peer_id_str == "":
        return
    try:
        sender_peer_id = int(sender_peer_id_str)
    except Exception:
        messagebox.showinfo("Invalid peer ID", "Please enter numeric peer ID")
        return

    rpc_client = rpc.RPC(sender_peer_id, True)
    success = rpc_client.init()
    if not success:
        messagebox.showinfo(
            "Connection failed", "Unable to connect to the peer with instance ID %d." % sender_peer_id)
        return

    rpc_client.peers()
    current_peers = ast.literal_eval(rpc_client.recv_out())
    peers = []
    for _, peer in current_peers.items():
        peers.append(peer["username"])

    peers_box['values'] = peers
    peers_box.current(0)


def send_msg(sender_peer_id_str, sender_username, message, receiver_username, text_area):
    if sender_peer_id_str == "":
        return
    try:
        sender_peer_id = int(sender_peer_id_str)
    except Exception:
        messagebox.showinfo("Invalid peer ID", "Please enter numeric peer ID")
        return

    rpc_client = rpc.RPC(sender_peer_id, True)
    success = rpc_client.init()
    if not success:
        messagebox.showinfo(
            "Connection failed", "Unable to connect to the peer with instance ID %d." % sender_peer_id)
        return
    rpc_client.message(sender_username, receiver_username, message)

    text_area.delete(1.0, END)


def main():
    window = Tk()
    window.title("P2P Peer Chat")
    window.geometry('420x230')
    peers_box = Combobox(window)
    peer_id_label = Label(window, text="Peer ID:")
    peer_id_label.grid(column=0, row=0)
    peer_id = Entry(window, width=10)
    peer_id.grid(column=1, row=0)
    peer_name_label = Label(window, text="Username:")
    peer_name_label.grid(column=2, row=0)
    peer_name = Entry(window, width=10)
    peer_name.grid(column=3, row=0)
    text_area = scrolledtext.ScrolledText(window, width=40, height=10)
    text_area.grid(columnspan=4, row=1)
    sender_label = Label(window, text="Send to:")
    sender_label.grid(column=0, row=2)
    peers_box.grid(column=1, row=2)
    refresh_button = Button(window, text="Refresh peers",
                            command=lambda: refresh_peers(peer_id.get(), peers_box))
    refresh_button.grid(column=2, row=2)
    send_button = Button(window, text="Send", command=lambda: send_msg(peer_id.get(
    ), peer_name.get(), text_area.get("1.0", END), peers_box.get(), text_area))
    send_button.grid(column=3, row=2)
    window.mainloop()


if __name__ == "__main__":
    main()
