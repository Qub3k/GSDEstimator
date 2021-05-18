import pandas
import sys
import struct

if len(sys.argv) != 2:
    print("ERROR")
    sys.exit(1)

filename = sys.argv[1]

try:
    dataframe = pandas.read_pickle(filename)
except IOError:
    print("File does not exists")
    sys.exit(1)

print(dataframe)
out_array = []

for (psi, ro), row in dataframe.iterrows():
    out_array.append(psi)
    out_array.append(ro)
    for idx in range(1, 6):
        out_array.append(row[idx])

out_file = open(filename.split(".")[0] + ".bin", "wb")
wrote = out_file.write(struct.pack('f'*len(out_array), *out_array))
out_file.close()
