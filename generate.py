#!/usr/bin/python

operations = [
        "rrc",
        "swpb",
        "rra",
        "sxt",
        "push",
        "call",
        "reti",
        "jne",
        "jeq",
        "jnc",
        "jc",
        "jn",
        "jge",
        "jl",
        "jmp",
        "mov",
        "add",
        "addc",
        "subc",
        "sub",
        "cmp",
        "dadd",
        "bit",
        "bic",
        "bis",
        "xor",
        "and",
]

registers = [
        "pc",
        "sp",
        "sr",
        "cg",
        "r4",
        "r5",
        "r6",
        "r7",
        "r8",
        "r9",
        "r10",
        "r11",
        "r12",
        "r13",
        "r14",
        "r15",
]


def gen_enum(prefix, items):
    print "enum {"
    print "\t%s_UNKNOWN = 0," % prefix
    for item in items:
        print "\t%s_%s," % (prefix, item.upper())
    print "};"
    print ""

def gen_lookup(fnname, argname,  prefix, items):
    print "char *%s(int %s){" % (fnname, argname)
    print "\tswitch(%s){" % (argname, )
    for item in items:
        print '\t\tcase %s_%s:\treturn "%s";' % (prefix.upper(), item.upper(), item)
    print '\t\tdefault:\treturn 0;'
    print "\t}"
    print "}"
    print ""

gen_enum("OPER", operations)
gen_enum("REG", registers)

gen_lookup("lookup_mnemonic_for_operation", "operation", "OPER", operations)
gen_lookup("lookup_reg_string", "reg", "REG", registers)



