import socket
import sys

def wait_ok(sock, name):
    ret = sock.recv(1024)
    if ret != b"OK":
        print(f"[-] Error: {name} - " + ret.decode("utf-8"))
        exit(1)

argv = sys.argv

if len(argv) != 2:
    print("Usage: python3 upload.py <file>")
    exit(1)

filename = argv[1]

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("172.16.34.39", 4007))
sock.settimeout(60)

with open(filename, "rb") as f:
    flash = f.read()


print("[+] Booting BootLoader...")
sock.send(b"\x07")
wait_ok(sock, "BootLoader")

print("[+] Uploading...")
sock.send(b"\x08")
sock.send(len(flash).to_bytes(4, "big"))
sock.send(flash)
wait_ok(sock, "Upload")

print("[+] Go!")
sock.send(b"\x09")
wait_ok(sock, "Go")