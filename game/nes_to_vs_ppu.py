def split_list(alist, wanted_parts=1):
    length = len(alist)
    return [ alist[i*length // wanted_parts: (i+1)*length // wanted_parts]
             for i in range(wanted_parts) ]


PPU_NES = [
333,14,6,326,403,503,510,420,320,120,31,40,22,0,0,0,
555,36,27,407,507,704,700,630,430,140,40,53,44,0,0,0,
777,357,447,637,707,737,740,750,660,360,70,276,77,444,0,0,
777,567,657,757,747,755,764,772,773,572,473,276,467,666,0,0,
    ]

RP2C04_0001 = [
    755,637,700,447,44,120,222,704,777,333,750,503,403,660,320,777,
    357,653,310,360,467,657,764,27,760,276,0,200,666,444,707,14,
    3,567,757,70,77,22,53,507,0,420,747,510,407,6,740,0,
    0,140,555,31,572,326,770,630,20,36,40,111,773,737,430,473,
]

RP2C04_0002 = [
      0,750,430,572,473,737, 44,567,700,407,773,747,777,637,467, 40,
     20,357,510,666, 53,360,200,447,222,707,  3,276,657,320,  0,326,
    403,764,740,757, 36,310,555,  6,507,760,333,120, 27,  0,660,777,
    653,111, 70,630, 22, 14,704,140,  0, 77,420,770,755,503, 31,444,
    ]

RP2C04_0003 = [
    507,737,473,555,40,777,567,120,14,0,764,320,704,666,653,467,
    447,44,503,27,140,430,630,53,333,326,0,6,700,510,747,755,
    637,20,3,770,111,750,740,777,360,403,357,707,36,444,0,310,
    77,200,572,757,420,70,660,222,31,0,657,773,407,276,760,22,
]

RP2C04_0004 = [
    430,326,44,660,0,755,14,630,555,310,70,3,764,770,40,572,
    737,200,27,747,0,222,510,740,653,53,447,140,403,0,473,357,
    503,31,420,6,407,507,333,704,22,666,36,20,111,773,444,707,
    757,777,320,700,760,276,777,467,0,750,637,567,360,657,77,120,
]

PPU_NEW = RP2C04_0004
PPU_NAME = "_RP2C04_0004"

print("PPU_NES Size: " + str(len(PPU_NES)))
print("PPU_NEW Size: " + str(len(PPU_NEW)))

missing_entries = []
PPU_OUT = []
found = False

for i in PPU_NES:
    index=0
    found = False
    # if i == 100:
    #     i = 120 #manual override
    for j in PPU_NEW:
        if i == j:
            PPU_OUT.append(hex(index))
            found = True
            break
        index+=1
    if found == False:
        missing_entries.append(i)
        PPU_NES_rgb = str(i)
        PPU_NES_rgb = [int(d) for d in str(PPU_NES_rgb)]
        while len(PPU_NES_rgb) < 3:
            PPU_NES_rgb.insert(0, 0)

        #552
        PPU_NES_rgb = [x * x for x in PPU_NES_rgb]
        #54
        PPU_NES_dist = PPU_NES_rgb[0] + PPU_NES_rgb[1] + PPU_NES_rgb[2]

        min_dist = 999999999999
        best_pick = -1
        index = 0
        for j in PPU_NEW:
            #555
            #27
            PPU_NEW_rgb = str(j)
            PPU_NEW_rgb = [int(d) for d in str(PPU_NEW_rgb)]
            while len(PPU_NEW_rgb) < 3:
                PPU_NEW_rgb.insert(0, 0)
            PPU_NEW_rgb = [x * x for x in PPU_NEW_rgb]

            #75
            #53
            PPU_NEW_dist = PPU_NEW_rgb[0] + PPU_NEW_rgb[1] + PPU_NEW_rgb[2]

            #21
            #2
            new_dist = abs((PPU_NEW_dist - PPU_NES_dist))
            new_dist = ((PPU_NEW_rgb[0] - PPU_NES_rgb[0])**2) + ((PPU_NEW_rgb[1] - PPU_NES_rgb[1])**2) + ((PPU_NEW_rgb[2] - PPU_NES_rgb[2])**2)
            if new_dist < min_dist:
                min_dist = new_dist
                best_pick = index
            index += 1

        print("; Remapping (rgb: " + str(i) + ") to [" + str(best_pick) + "] (rgb: " + str(PPU_NEW[best_pick]) + ").")
        PPU_OUT.append(hex(best_pick))

for i, string in enumerate(PPU_OUT):
    PPU_OUT[i] = string.replace("0x", "$")

chunks = split_list(PPU_OUT, 4)
final_string = ""
final_string += "palBrightTable4" +PPU_NAME+ ":\n	.byte "
for i in chunks[0]:
    final_string += i + ","
final_string = final_string[:-1]
final_string += "\npalBrightTable5" +PPU_NAME+ ":\n	.byte "
for i in chunks[1]:
    final_string += i + ","
final_string = final_string[:-1]
final_string += "\npalBrightTable6" +PPU_NAME+ ":\n	.byte "
for i in chunks[2]:
    final_string += i + ","
final_string = final_string[:-1]
final_string += "\npalBrightTable7" +PPU_NAME+ ":\n	.byte "
for i in chunks[3]:
    final_string += i + ","
final_string = final_string[:-1]

print(final_string)

#print("Could not find " + str(len(missing_entries)) + " RGB value: " + str(missing_entries))
print(PPU_OUT)
print("Size: " + str(len(PPU_OUT)))
