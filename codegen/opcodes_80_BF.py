g_tab = "    "
g_tabCount = 2
g_regTable = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
g_operationTable = ["ADD", "ADC", "SUB", "SBC", "AND", "XOR", "OR", "CP"]
g_codeTable = {
    "ADD": "coreCpuOpAdd({:s});",
    "ADC": "coreCpuOpAdc({:s});",
    "SUB": "coreCpuOpSub({:s});",
    "SBC": "coreCpuOpSbc({:s});",
    "AND": "coreCpuOpAnd({:s});",
    "XOR": "coreCpuOpXor({:s});",
    "OR": "coreCpuOpOr({:s});",
    "CP": "coreCpuOpCp({:s});",
}
g_operandTable = {
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
    for l_register in g_regTable:
        print(
            "{:s}case 0x{:02x}: // {:s} A, {:s}".format(
                g_tab * g_tabCount,
                g_currentOpcode,
                l_operation,
                l_register
            )
        )

        print(
            "{:s}{:s}".format(
                g_tab * (g_tabCount + 1),
                g_codeTable[l_operation].format(
                    g_operandTable[l_register]
                )
            )
        )

        print("{:s}break;".format(g_tab * (g_tabCount + 1)))

        print()

        g_currentOpcode += 1
