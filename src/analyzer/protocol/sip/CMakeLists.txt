zeek_add_plugin(
    Zeek SIP
    SOURCES Plugin.cc SIP.cc SIP_TCP.cc
    BIFS events.bif
    PAC sip.pac sip-analyzer.pac sip-protocol.pac
    PAC sip_TCP.pac sip-protocol.pac sip-analyzer.pac)
