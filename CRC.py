
def calcula_crc(bits,div):
    CRC = []
    for i in range(len(div)-1):
        bits.append(0)
    while len(bits) != (len(div)-1):
        if len(bits) < (len(div)-1):
            bits = [0]*((len(div)-1) - len(bits)) + bits
            break
        for j in range(len(div)):
            if ((bits[j]==1 and div[j]==1) or ((bits[j]==0) and (div[j]==0))):
                bits[j] = 0
            else:
                bits[j] = 1
        non_zero_bit = 0
        for k in range(len(bits)-1):
            if bits[k]==0:
                non_zero_bit += 1
            else:
                break
        bits = bits[non_zero_bit:]
    return bits

data_bits=[1,1,0,1,0,1,0,0,0]
div = [1,0,0,1]
data_bits2=[1,0,1,1,0,1,0,1]
div2 = [1,0,0,1,1]
print(calcula_crc(data_bits, div))
print(calcula_crc(data_bits2, div2))
