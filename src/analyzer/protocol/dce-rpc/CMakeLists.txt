zeek_add_plugin(
    Zeek DCE_RPC
    SOURCES DCE_RPC.cc Plugin.cc
    BIFS consts.bif types.bif events.bif
    PAC dce_rpc.pac dce_rpc-protocol.pac dce_rpc-analyzer.pac dce_rpc-auth.pac endpoint-atsvc.pac
        endpoint-epmapper.pac)
