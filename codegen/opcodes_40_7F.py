g_tab = "    "
g_tabCount = 2
g_regTable = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
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

g_currentOpcode = 0x40

for l_destinationOperand in g_regTable:
    for l_sourceOperand in g_regTable:
        print(
            "{:s}case 0x{:02x}: // LD {:s}, {:s}".format(
                g_tab * g_tabCount,
                g_currentOpcode,
                l_destinationOperand,
                l_sourceOperand
            )
        )

        print(
            "{:s}{:s}".format(
                g_tab * (g_tabCount + 1),
                g_setTable[l_destinationOperand].format(
                    g_getTable[l_sourceOperand]
                )
            )
        )

        print("{:s}break;".format(g_tab * (g_tabCount + 1)))

        print()

        g_currentOpcode += 1
