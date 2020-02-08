#encoding: utf-8
data1 = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
data2 = ["GB_CPU.b", "GB_CPU.c", "GB_CPU.d", "GB_CPU.e", "GB_CPU.h", "GB_CPU.l", "gb_mmu_read8_cycle(GB_CPU.hl)", "GB_CPU.a"]
data3 = ["op_bit", "op_res", "op_set"]
data4 = ["BIT", "RES", "SET"]

for k in range(0, 3):
    for i in range(0, 8):
        for j in range(0, 8):
            casenum = 0x40 + k * 0x40 + i * 8 + j

            print("\n\t\t\t\tcase " + hex(casenum) + ": // " + data4[k] + " " + str(i) + ", " + data1[j])
            
            if k == 0:
                if j == 6:
                    print("\t\t\t\t\t" + data3[k] + "(" + str(i) + ", " + "gb_mmu_read8_cycle(GB_CPU.hl));");
                else:
                    print("\t\t\t\t\t" + data3[k] + "(" + str(i) + ", " + data2[j] + ");");
            else:
                if j == 6:
                    print("\t\t\t\t\tgb_mmu_write8_cycle(GB_CPU.hl, " + data3[k] + "(" + str(i) + ", " + "gb_mmu_read8_cycle(GB_CPU.hl)));");
                else:
                    print("\t\t\t\t\t" + data2[j] + " = " + data3[k] + "(" + str(i) + ", " + data2[j] + ");");

            print("\t\t\t\t\tbreak;")
