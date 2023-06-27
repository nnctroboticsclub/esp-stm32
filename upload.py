import socket
import sys

def wait_ok(sock, name):
    ret = sock.recv(1024)
    ret = ret.strip(b"\x00")
    if ret != b"OK":
        print(f"[-] Error: {name} - " + repr(ret))
        exit(1)

argv = sys.argv

if len(argv) != 3:
    print("Usage: python3 upload.py <ip> <file>")
    exit(1)

ip = argv[1]
filename = argv[2]

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((ip, 4007))
sock.settimeout(60)

with open(filename, "rb") as f:
    flash = f.read()


print("[+] Booting BootLoader...")
sock.send(b"\x10")
wait_ok(sock, "BootLoader")

print("[+] Uploading...")
sock.send(b"\x11")
sock.send(len(flash).to_bytes(4, "big"))
sock.send(flash)
wait_ok(sock, "Upload")

print("[+] Go!")
sock.send(b"\x12")
wait_ok(sock, "Go")