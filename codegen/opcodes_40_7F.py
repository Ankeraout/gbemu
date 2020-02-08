#encoding: utf-8
data1 = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
data2 = ["GB_CPU.b", "GB_CPU.c", "GB_CPU.d", "GB_CPU.e", "GB_CPU.h", "GB_CPU.l", "gb_mmu_read8_cycle(GB_CPU.hl)", "GB_CPU.a"]

for i in range(0, 8):
    for j in range(0, 8):
        casenum = 0x40 + i * 8 + j

        if casenum == 0x76:
            print("\n\t\tcase 0x76: // HALT")
            print("\t\t\tGB_CPU.halted = true;")
            print("\t\t\tbreak;")
        else:
            print("\n\t\tcase " + hex(casenum) + ": // LD " + data1[i] + ", " + data1[j])

            if(i == 6):
                print("\t\t\tgb_mmu_write8_cycle(GB_CPU.hl, " + data2[j] + ");")
            else:
                print("\t\t\t" + data2[i] + " = " + data2[j] + ";")

            print("\t\t\tbreak;")
