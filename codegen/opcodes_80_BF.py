#encoding: utf-8
data1 = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
data2 = ["ADD", "ADC", "SUB", "SBC", "AND", "XOR", "OR", "CP"]
data3 = ["GB_CPU.b", "GB_CPU.c", "GB_CPU.d", "GB_CPU.e", "GB_CPU.h", "GB_CPU.l", "gb_mmu_read8_cycle(GB_CPU.hl)", "GB_CPU.a"]
data4 = ["op_add_a", "op_adc_a", "op_sub_a", "op_sbc_a", "op_and_a", "op_xor_a", "op_or_a", "op_cp_a"]

for i in range(0, 8):
    for j in range(0, 8):
        casenum = 0x80 + i * 8 + j

        print("\n\t\tcase " + hex(casenum) + ": // " + data2[i] + " A, " + data1[j])
        print("\t\t\t" + data4[i] + "(" + data3[j] + ");")
        print("\t\t\tbreak;")
