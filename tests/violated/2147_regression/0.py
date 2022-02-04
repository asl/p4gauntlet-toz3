from p4z3 import *



def p4_program(prog_state):
    prog_state.declare_global(
        Enum( "error", ["NoError", "PacketTooShort", "NoMatch", "StackOutOfBounds", "HeaderTooShort", "ParserTimeout", "ParserInvalidArgument", ])
    )
    prog_state.declare_global(
        P4Extern("packet_in", type_params=[], methods=[P4Declaration("extract", P4Method("extract", type_params=(None, [
            "T",]), params=[
            P4Parameter("out", "hdr", "T", None),])), P4Declaration("extract", P4Method("extract", type_params=(None, [
            "T",]), params=[
            P4Parameter("out", "variableSizeHeader", "T", None),
            P4Parameter("in", "variableFieldSizeInBits", z3.BitVecSort(32), None),])), P4Declaration("lookahead", P4Method("lookahead", type_params=("T", [
            "T",]), params=[])), P4Declaration("advance", P4Method("advance", type_params=(None, []), params=[
            P4Parameter("in", "sizeInBits", z3.BitVecSort(32), None),])), P4Declaration("length", P4Method("length", type_params=(z3.BitVecSort(32), []), params=[])), ])
    )
    prog_state.declare_global(
        P4Extern("packet_out", type_params=[], methods=[P4Declaration("emit", P4Method("emit", type_params=(None, [
            "T",]), params=[
            P4Parameter("in", "hdr", "T", None),])), ])
    )
    prog_state.declare_global(
        P4Declaration("verify", P4Method("verify", type_params=(None, []), params=[
            P4Parameter("in", "check", z3.BoolSort(), None),
            P4Parameter("in", "toSignal", "error", None),]))
    )
    prog_state.declare_global(
        P4Declaration("match_kind", ["exact", "ternary", "lpm", ])
    )
    prog_state.declare_global(
        P4Declaration("match_kind", ["range", "optional", "selector", ])
    )
    prog_state.declare_global(
        StructType("standard_metadata_t", prog_state, fields=[("ingress_port", z3.BitVecSort(9)), ("egress_spec", z3.BitVecSort(9)), ("egress_port", z3.BitVecSort(9)), ("instance_type", z3.BitVecSort(32)), ("packet_length", z3.BitVecSort(32)), ("enq_timestamp", z3.BitVecSort(32)), ("enq_qdepth", z3.BitVecSort(19)), ("deq_timedelta", z3.BitVecSort(32)), ("deq_qdepth", z3.BitVecSort(19)), ("ingress_global_timestamp", z3.BitVecSort(48)), ("egress_global_timestamp", z3.BitVecSort(48)), ("mcast_grp", z3.BitVecSort(16)), ("egress_rid", z3.BitVecSort(16)), ("checksum_error", z3.BitVecSort(1)), ("parser_error", "error"), ("priority", z3.BitVecSort(3)), ], type_params=[])
    )
    prog_state.declare_global(
        Enum( "CounterType", ["packets", "bytes", "packets_and_bytes", ])
    )
    prog_state.declare_global(
        Enum( "MeterType", ["packets", "bytes", ])
    )
    prog_state.declare_global(
        P4Extern("counter", type_params=[], methods=[P4Declaration("counter", P4Method("counter", type_params=(None, []), params=[
            P4Parameter("none", "size", z3.BitVecSort(32), None),
            P4Parameter("none", "type", "CounterType", None),])), P4Declaration("count", P4Method("count", type_params=(None, []), params=[
            P4Parameter("in", "index", z3.BitVecSort(32), None),])), ])
    )
    prog_state.declare_global(
        P4Extern("direct_counter", type_params=[], methods=[P4Declaration("direct_counter", P4Method("direct_counter", type_params=(None, []), params=[
            P4Parameter("none", "type", "CounterType", None),])), P4Declaration("count", P4Method("count", type_params=(None, []), params=[])), ])
    )
    prog_state.declare_global(
        P4Extern("meter", type_params=[], methods=[P4Declaration("meter", P4Method("meter", type_params=(None, []), params=[
            P4Parameter("none", "size", z3.BitVecSort(32), None),
            P4Parameter("none", "type", "MeterType", None),])), P4Declaration("execute_meter", P4Method("execute_meter", type_params=(None, [
            "T",]), params=[
            P4Parameter("in", "index", z3.BitVecSort(32), None),
            P4Parameter("out", "result", "T", None),])), ])
    )
    prog_state.declare_global(
        P4Extern("direct_meter", type_params=[
            "T",], methods=[P4Declaration("direct_meter", P4Method("direct_meter", type_params=(None, []), params=[
            P4Parameter("none", "type", "MeterType", None),])), P4Declaration("read", P4Method("read", type_params=(None, []), params=[
            P4Parameter("out", "result", "T", None),])), ])
    )
    prog_state.declare_global(
        P4Extern("register", type_params=[
            "T",], methods=[P4Declaration("register", P4Method("register", type_params=(None, []), params=[
            P4Parameter("none", "size", z3.BitVecSort(32), None),])), P4Declaration("read", P4Method("read", type_params=(None, []), params=[
            P4Parameter("out", "result", "T", None),
            P4Parameter("in", "index", z3.BitVecSort(32), None),])), P4Declaration("write", P4Method("write", type_params=(None, []), params=[
            P4Parameter("in", "index", z3.BitVecSort(32), None),
            P4Parameter("in", "value", "T", None),])), ])
    )
    prog_state.declare_global(
        P4Extern("action_profile", type_params=[], methods=[P4Declaration("action_profile", P4Method("action_profile", type_params=(None, []), params=[
            P4Parameter("none", "size", z3.BitVecSort(32), None),])), ])
    )
    prog_state.declare_global(
        Enum( "HashAlgorithm", ["crc32", "crc32_custom", "crc16", "crc16_custom", "random", "identity", "csum16", "xor16", ])
    )
    prog_state.declare_global(
        P4Extern("action_selector", type_params=[], methods=[P4Declaration("action_selector", P4Method("action_selector", type_params=(None, []), params=[
            P4Parameter("none", "algorithm", "HashAlgorithm", None),
            P4Parameter("none", "size", z3.BitVecSort(32), None),
            P4Parameter("none", "outputWidth", z3.BitVecSort(32), None),])), ])
    )
    prog_state.declare_global(
        P4Extern("Checksum16", type_params=[], methods=[P4Declaration("Checksum16", P4Method("Checksum16", type_params=(None, []), params=[])), P4Declaration("get", P4Method("get", type_params=(z3.BitVecSort(16), [
            "D",]), params=[
            P4Parameter("in", "data", "D", None),])), ])
    )
    prog_state.declare_global(
        ControlDeclaration(P4ParserType("Parser", params=[
            P4Parameter("none", "b", "packet_in", None),
            P4Parameter("out", "parsedHdr", "H", None),
            P4Parameter("inout", "meta", "M", None),
            P4Parameter("inout", "standard_metadata", "standard_metadata_t", None),], type_params=[
            "H",
            "M",]))
    )
    prog_state.declare_global(
        ControlDeclaration(P4ControlType("VerifyChecksum", params=[
            P4Parameter("inout", "hdr", "H", None),
            P4Parameter("inout", "meta", "M", None),], type_params=[
            "H",
            "M",]))
    )
    prog_state.declare_global(
        ControlDeclaration(P4ControlType("Ingress", params=[
            P4Parameter("inout", "hdr", "H", None),
            P4Parameter("inout", "meta", "M", None),
            P4Parameter("inout", "standard_metadata", "standard_metadata_t", None),], type_params=[
            "H",
            "M",]))
    )
    prog_state.declare_global(
        ControlDeclaration(P4ControlType("Egress", params=[
            P4Parameter("inout", "hdr", "H", None),
            P4Parameter("inout", "meta", "M", None),
            P4Parameter("inout", "standard_metadata", "standard_metadata_t", None),], type_params=[
            "H",
            "M",]))
    )
    prog_state.declare_global(
        ControlDeclaration(P4ControlType("ComputeChecksum", params=[
            P4Parameter("inout", "hdr", "H", None),
            P4Parameter("inout", "meta", "M", None),], type_params=[
            "H",
            "M",]))
    )
    prog_state.declare_global(
        ControlDeclaration(P4ControlType("Deparser", params=[
            P4Parameter("none", "b", "packet_out", None),
            P4Parameter("in", "hdr", "H", None),], type_params=[
            "H",]))
    )
    prog_state.declare_global(
        ControlDeclaration(P4Package("V1Switch", params=[
            P4Parameter("none", "p", TypeSpecializer("Parser", "H", "M", ), None),
            P4Parameter("none", "vr", TypeSpecializer("VerifyChecksum", "H", "M", ), None),
            P4Parameter("none", "ig", TypeSpecializer("Ingress", "H", "M", ), None),
            P4Parameter("none", "eg", TypeSpecializer("Egress", "H", "M", ), None),
            P4Parameter("none", "ck", TypeSpecializer("ComputeChecksum", "H", "M", ), None),
            P4Parameter("none", "dep", TypeSpecializer("Deparser", "H", ), None),],type_params=[
            "H",
            "M",]))
    )
    prog_state.declare_global(
        HeaderType("ethernet_t", prog_state, fields=[("dstAddr", z3.BitVecSort(48)), ("srcAddr", z3.BitVecSort(48)), ("etherType", z3.BitVecSort(16)), ], type_params=[])
    )
    prog_state.declare_global(
        HeaderType("H", prog_state, fields=[("a", z3.BitVecSort(8)), ], type_params=[])
    )
    prog_state.declare_global(
        StructType("Parsed_packet", prog_state, fields=[("eth", "ethernet_t"), ("h", "H"), ], type_params=[])
    )
    prog_state.declare_global(
        StructType("Metadata", prog_state, fields=[], type_params=[])
    )
    prog_state.declare_global(
        ControlDeclaration(P4Control(
            name="deparser",
            type_params=[],
            params=[
                P4Parameter("none", "packet", "packet_out", None),
                P4Parameter("in", "hdr", "Parsed_packet", None),],
            const_params=[],
            body=BlockStatement([
                MethodCallStmt(MethodCallExpr(P4Member("packet", "emit"), ["Parsed_packet", ], "hdr", )),]
            ),
            local_decls=[]
        ))
    )
    prog_state.declare_global(
        ControlDeclaration(P4Parser(
            name="p",
            type_params=[],
            params=[
                P4Parameter("none", "pkt", "packet_in", None),
                P4Parameter("out", "hdr", "Parsed_packet", None),
                P4Parameter("inout", "meta", "Metadata", None),
                P4Parameter("inout", "stdmeta", "standard_metadata_t", None),],
            const_params=[],
            local_decls=[],
            body=ParserTree([
                ParserState(name="start", select="accept",
                components=[
                MethodCallStmt(MethodCallExpr(P4Member("pkt", "extract"), ["ethernet_t", ], P4Member("hdr", "eth"), )),
                MethodCallStmt(MethodCallExpr(P4Member("pkt", "extract"), ["H", ], P4Member("hdr", "h"), )),                ]),
                ])
))
    )
    prog_state.declare_global(
        ControlDeclaration(P4Control(
            name="ingress",
            type_params=[],
            params=[
                P4Parameter("inout", "hdr", "Parsed_packet", None),
                P4Parameter("inout", "meta", "Metadata", None),
                P4Parameter("inout", "stdmeta", "standard_metadata_t", None),],
            const_params=[],
            body=BlockStatement([
                MethodCallStmt(MethodCallExpr("do_action_0", [], P4Slice(P4Member(P4Member("hdr", "h"), "a"), 7, 1), )),]
            ),
            local_decls=[
P4Declaration("do_action_0", P4Action("do_action", params=[
                    P4Parameter("inout", "in_bit", z3.BitVecSort(7), None),],                 body=BlockStatement([
                    P4Noop(),]
                )                )), ]
        ))
    )
    prog_state.declare_global(
        ControlDeclaration(P4Control(
            name="egress",
            type_params=[],
            params=[
                P4Parameter("inout", "hdr", "Parsed_packet", None),
                P4Parameter("inout", "meta", "Metadata", None),
                P4Parameter("inout", "stdmeta", "standard_metadata_t", None),],
            const_params=[],
            body=BlockStatement([]
            ),
            local_decls=[]
        ))
    )
    prog_state.declare_global(
        ControlDeclaration(P4Control(
            name="vrfy",
            type_params=[],
            params=[
                P4Parameter("inout", "hdr", "Parsed_packet", None),
                P4Parameter("inout", "meta", "Metadata", None),],
            const_params=[],
            body=BlockStatement([]
            ),
            local_decls=[]
        ))
    )
    prog_state.declare_global(
        ControlDeclaration(P4Control(
            name="update",
            type_params=[],
            params=[
                P4Parameter("inout", "hdr", "Parsed_packet", None),
                P4Parameter("inout", "meta", "Metadata", None),],
            const_params=[],
            body=BlockStatement([]
            ),
            local_decls=[]
        ))
    )
    prog_state.declare_global(
        InstanceDeclaration("main", TypeSpecializer("V1Switch", "Parsed_packet", "Metadata", ), ConstCallExpr("p", ), ConstCallExpr("vrfy", ), ConstCallExpr("ingress", ), ConstCallExpr("egress", ), ConstCallExpr("update", ), ConstCallExpr("deparser", ), )
    )
    var = prog_state.get_main_function()
    return var if isinstance(var, P4Package) else None
