g_tab = "    "
g_tabCount = 4
g_regTable = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
g_operationTable = ["RES", "SET"]
g_codeTable = {
    "RES": "{:s} & ~(1 << {:d})",
    "SET": "{:s} | (1 << {:d})"
}
g_setTable = {
    "B": "s_coreCpuRegisterBC.byte.high = {:s};",
    "C": "s_coreCpuRegisterBC.byte.low = {:s};",
    "D": "s_coreCpuRegisterDE.byte.high = {:s};",
    "E": "s_coreCpuRegisterDE.byte.low = {:s};",
    "H": "s_coreCpuRegisterHL.byte.high = {:s};",
    "L": "s_coreCpuRegisterHL.byte.low = {:s};",
    "(HL)": "coreBusWrite(s_coreCpuRegisterHL.word, {:s});",
    "A": "s_coreCpuRegisterAF.byte.high = {:s};"
}
g_getTable = {
    "B": "s_coreCpuRegisterBC.byte.high",
    "C": "s_coreCpuRegisterBC.byte.low",
    "D": "s_coreCpuRegisterDE.byte.high",
    "E": "s_coreCpuRegisterDE.byte.low",
    "H": "s_coreCpuRegisterHL.byte.high",
    "L": "s_coreCpuRegisterHL.byte.low",
    "(HL)": "coreBusRead(s_coreCpuRegisterHL.word)",
    "A": "s_coreCpuRegisterAF.byte.high"
}

g_currentOpcode = 0x80

for l_operation in g_operationTable:
    for l_bit in range(0, 8):
        for l_register in g_regTable:
            print(
                "{:s}case 0x{:02x}: // {:s} {:d}, {:s}".format(
                    g_tab * g_tabCount,
                    g_currentOpcode,
                    l_operation,
                    l_bit,
                    l_register
                )
            )

            print(
                "{:s}{:s}".format(
                    g_tab * (g_tabCount + 1),
                    g_setTable[l_register].format(
                        g_codeTable[l_operation].format(
                            g_getTable[l_register],
                            l_bit
                        )
                    )
                )
            )

            print("{:s}break;".format(g_tab * (g_tabCount + 1)))

            print()

            g_currentOpcode += 1