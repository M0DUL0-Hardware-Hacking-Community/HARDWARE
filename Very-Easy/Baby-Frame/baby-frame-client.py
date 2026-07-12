from pwn import *
import struct

def crc16_ccitt_false(data: bytes) -> int:
    crc = 0xffff
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xffff
            else:
                crc = (crc << 1) & 0xffff
    return crc

def generate_space_packet(apid: int, packet_count: int, payload: bytes) -> bytes:
    version = 0
    packet_type = 1          # 1 = telecommand
    secondary_header = 0     # try 0 first unless challenge says otherwise
    sequence_flags = 0b11    # unsegmented user data

    packet_id = (
        (version << 13)
        | (packet_type << 12)
        | (secondary_header << 11)
        | (apid & 0x7ff)
    )

    sequence_control = (
        (sequence_flags << 14)
        | (packet_count & 0x3fff)
    )

    packet_data_length = len(payload) - 1

    return struct.pack(">HHH", packet_id, sequence_control, packet_data_length) + payload

def generate_tc_frame(
    spacecraft_id: int,
    virtual_channel_id: int,
    tc_packet_count: int,
    payload: bytes,
    use_fecf: bool = True,
    bypass: bool = True,
    segment_header: bool = False,
) -> bytes:
    version = 0
    bypass_flag = 1 if bypass else 0
    control_command_flag = 0
    reserved = 0

    data = payload

    # Optional TC segment header. Try both with and without this.
    if segment_header:
        seq_flags = 0b11
        map_id = 0
        data = bytes([(seq_flags << 6) | (map_id & 0x3f)]) + data

    first_word = (
        (version << 14)
        | (bypass_flag << 13)
        | (control_command_flag << 12)
        | (reserved << 10)
        | (spacecraft_id & 0x3ff)
    )

    total_len = 5 + len(data) + (2 if use_fecf else 0)
    frame_length = total_len - 1

    second_word = (
        ((virtual_channel_id & 0x3f) << 10)
        | (frame_length & 0x3ff)
    )

    header = struct.pack(">HHB", first_word, second_word, tc_packet_count & 0xff)
    frame = header + data

    if use_fecf:
        frame += struct.pack(">H", crc16_ccitt_false(frame))

    return frame

def main():
    HOST = "154.57.164.80"
    PORT = 32757

    space_packet = generate_space_packet(
        apid=42,
        packet_count=0,
        payload=b"HEALTHCHECK",
    )

    frame = generate_tc_frame(
        spacecraft_id=12,
        virtual_channel_id=3,
        tc_packet_count=0,
        payload=space_packet,
        use_fecf=True,
        bypass=True,
        segment_header=False,
    )

    log.info(f"Sending {len(frame)} bytes")
    log.info(hexdump(frame))

    r = remote(HOST, PORT)
    r.send(frame)
    response = r.recvall(timeout=3)
    log.info(f"Server response: {response!r}")


if __name__ == "__main__":
    main()
