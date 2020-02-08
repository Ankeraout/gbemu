#encoding: utf-8
data1 = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
data2 = ["RLC", "RRC", "RL", "RR", "SLA", "SRA", "SWAP", "SRL"]
data3 = ["GB_CPU.b", "GB_CPU.c", "GB_CPU.d", "GB_CPU.e", "GB_CPU.h", "GB_CPU.l", "gb_mmu_read8_cycle(GB_CPU.hl)", "GB_CPU.a"]
data4 = ["op_rlc", "op_rrc", "op_rl", "op_rr", "op_sla", "op_sra", "op_swap", "op_srl"];

for i in range(0, 8):
    for j in range(0, 8):
        casenum = i * 8 + j

        if(casenum < 0x10):
            hexstr = "0x0" + "0123456789abcdef"[casenum]
        else:
            hexstr = hex(casenum)

        print("\n\t\t\t\tcase " + hexstr + ": // " + data2[i] + " " + data1[j]);

        if j == 6:
            print("\t\t\t\t\tgb_mmu_write8_cycle(GB_CPU.hl, " + data4[i] + "(gb_mmu_read8_cycle(GB_CPU.hl)));")
        else:
            print("\t\t\t\t\t" + data3[j] + " = " + data4[i] + "(" + data3[j] + ");")
        
        print("\t\t\t\t\tbreak;")
