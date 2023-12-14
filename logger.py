import socket
import threading


def logger(port: int) -> None:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('', port))

    print(f"Listening on port {port}...")
    while True:
        rcv_data, addr = sock.recvfrom(1024)

        addr_formatted = addr.rjust(20)
        print(f"{port}| {addr_formatted} | {rcv_data}")

    sock.close()


threading.Thread(target=logger, args=(1234,)).start()
threading.Thread(target=logger, args=(5678,)).start()