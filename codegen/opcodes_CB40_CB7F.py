g_tab = "    "
g_tabCount = 4
g_regTable = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
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
g_operation = "s_coreCpuFlagZ = ({:s} & (1 << {:d})) == 0;"

g_currentOpcode = 0x40

for l_bit in range(0, 8):
    for l_register in g_regTable:
        print(
            "{:s}case 0x{:02x}: // BIT {:d}, {:s}".format(
                g_tab * g_tabCount,
                g_currentOpcode,
                l_bit,
                l_register
            )
        )

        print(
            "{:s}{:s}".format(
                g_tab * (g_tabCount + 1),
                g_operation.format(
                    g_getTable[l_register],
                    l_bit
                )
            )
        )
        print(
            "{:s}s_coreCpuFlagN = false;".format(
                g_tab * (g_tabCount + 1)
            )
        )
        print(
            "{:s}s_coreCpuFlagH = true;".format(
                g_tab * (g_tabCount + 1)
            )
        )

        print("{:s}break;".format(g_tab * (g_tabCount + 1)))

        print()

        g_currentOpcode += 1
