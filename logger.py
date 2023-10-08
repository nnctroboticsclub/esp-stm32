import socket

PORT = 9088

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', PORT))

print(f"Listening on port {PORT}...")
while True:
    rcv_data, addr = sock.recvfrom(1024)

    addr_formatted = addr.rjust(20)
    print(f"{addr_formatted} | {rcv_data}")

sock.close()