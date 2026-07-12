Further analysis of the spacecraft link revealed that the onboard system has transitioned into protected transmission mode. The diagnostic service on spacecraft ID 12 still listens on APID 42 over virtual channel 3, but telemetry frames are now validated using the TM Frame Error Control Field defined by the CCSDS standard.

To communicate successfully, you must construct a fully valid CCSDS frame with the user payload GIVE-ME-THE-FLAG, including the correct Frame Error Control Field (CRC) calculation described in section 4.1.4 of the CCSDS TC Space Data Link Protocol specification.
