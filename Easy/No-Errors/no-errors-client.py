try:
    from pwn import hexdump, log, remote
except ModuleNotFoundError:
    import socket

    class _Log:
        @staticmethod
        def info(message):
            print(f"[*] {message}")

    class remote:
        def __init__(self, host, port):
            print(f"[+] Opening connection to {host} on port {port}: Done")
            self.sock = socket.create_connection((host, port), timeout=5)

        def send(self, data):
            self.sock.sendall(data)

        def recvall(self, timeout=3):
            self.sock.settimeout(timeout)
            chunks = []
            while True:
                try:
                    chunk = self.sock.recv(4096)
                except socket.timeout:
                    break
                if not chunk:
                    break
                chunks.append(chunk)
            return b"".join(chunks)

        def close(self):
            self.sock.close()

    def hexdump(data):
        rows = []
        for offset in range(0, len(data), 16):
            chunk = data[offset:offset + 16]
            hex_bytes = " ".join(f"{byte:02x}" for byte in chunk)
            ascii_bytes = "".join(chr(byte) if 32 <= byte <= 126 else "." for byte in chunk)
            rows.append(f"{offset:08x}  {hex_bytes:<47}  |{ascii_bytes}|")
        return "\n".join(rows)

    log = _Log()

import argparse
import struct


SPACECRAFT_ID = 12
VIRTUAL_CHANNEL_ID = 3
APID = 42
USER_PAYLOAD = b"GIVE-ME-THE-FLAG"
HOST = "154.57.164.81"
PORT = 31423


def crc16_ccitt_false(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def generate_space_packet(apid: int, packet_count: int, payload: bytes) -> bytes:
    version = 0
    packet_type = 1
    secondary_header_flag = 0
    sequence_flags = 0b11

    packet_id = (
        (version << 13)
        | (packet_type << 12)
        | (secondary_header_flag << 11)
        | (apid & 0x7FF)
    )
    sequence_control = (sequence_flags << 14) | (packet_count & 0x3FFF)
    packet_data_length = len(payload) - 1

    return struct.pack(">HHH", packet_id, sequence_control, packet_data_length) + payload


def generate_tc_frame(
    spacecraft_id: int,
    virtual_channel_id: int,
    tc_packet_count: int,
    payload: bytes,
) -> bytes:
    version = 0
    bypass_flag = 1
    control_command_flag = 0
    reserved = 0

    first_word = (
        (version << 14)
        | (bypass_flag << 13)
        | (control_command_flag << 12)
        | (reserved << 10)
        | (spacecraft_id & 0x3FF)
    )

    total_len = 5 + len(payload) + 2
    frame_length = total_len - 1
    second_word = ((virtual_channel_id & 0x3F) << 10) | (frame_length & 0x3FF)

    frame_without_fecf = struct.pack(">HHB", first_word, second_word, tc_packet_count & 0xFF) + payload
    fecf = crc16_ccitt_false(frame_without_fecf)
    return frame_without_fecf + struct.pack(">H", fecf)


def parse_args():
    parser = argparse.ArgumentParser(description="Protected-mode CCSDS TC client.")
    parser.add_argument("--host", default=HOST)
    parser.add_argument("--port", type=int, default=PORT)
    return parser.parse_args()


def main():
    args = parse_args()

    space_packet = generate_space_packet(
        apid=APID,
        packet_count=0,
        payload=USER_PAYLOAD,
    )
    frame = generate_tc_frame(
        spacecraft_id=SPACECRAFT_ID,
        virtual_channel_id=VIRTUAL_CHANNEL_ID,
        tc_packet_count=0,
        payload=space_packet,
    )

    log.info(f"Space Packet length: {len(space_packet)} bytes")
    log.info(f"TC frame length: {len(frame)} bytes")
    log.info(hexdump(frame))

    r = remote(args.host, args.port)
    r.send(frame)
    response = r.recvall(timeout=3)
    log.info(f"Server response: {response!r}")
    r.close()


if __name__ == "__main__":
    main()
