A secondary onboard service has been discovered on spacecraft ID 12, running on APID 83 over virtual channel 4. It appears to implement a simple stateful command interface used for ground testing and validation.

The service accepts CCSDS space packets with payload format: 0x00:BEGIN

where 0x00 is a packet counter starting at zero. The spacecraft replies with the incremented counter and an ACK.

After synchronization, the command GETFLAG can be issued using the expected counter to retrieve the flag.

Communication is done via AD-type telecommand frames with CRC, and responses must be parsed from CCSDS telemetry frames.

A modem is available for sending telecommand frames of the type AD with a CRC, and receive the answer frame from the spacecraft.
