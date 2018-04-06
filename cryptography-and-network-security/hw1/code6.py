import string

with open('release/otp/otp.txt', 'r') as f:
    c = f.read()

c = c.rstrip()
c = [c[i:i+2] for i in range(0, len(c), 2)]
c = [int(i, 16) for i in c]

def isalphabet(c):
    if ord('a') <= c and c <= ord('z'):
        return True
    if ord('A') <= c and c <= ord('Z'):
        return True
    if ord('0') <= c and c <= ord('9'):
        return True
    if chr(c) in ',. "\'?!:{}()_-':
        return True
    return False

alphabet = [i for i in range(0, 256) if isalphabet(i)]

def dfs(pos, cycle):
    global c
    if pos == cycle:
        return [[]]
    found_keys = []
    for i in alphabet:
        k = c[pos] ^ i
        flag = True
        for j in range(cycle + pos, len(c), cycle):
            if (c[j] ^ k) not in alphabet:
                flag = False
                break
        if flag:
            key_suffix = dfs(pos+1, cycle)
            for keys in key_suffix:
                found_keys.append([k] + keys)
    return found_keys

cycle = 13

def decode(c, pos, k, m):
    for i in range(pos, len(c), cycle):
        m[i] = chr(c[i] ^ k)

def test(c, pos, k):
    for i in range(pos, len(c), cycle):
        if c[i] ^ k not in alphabet:
            return False
    return True

m = ['_'] * len(c)

key = [169, 109, 201, 15, 92, 226, 255, 144, 212, 123, 223, 119, 148]
key_text = ''.join([chr(i) for i in key])

print 'Key = {}'.format(key)

for pos in range(len(key)):
    decode(c, pos, key[pos], m)

msg = ''.join(m)
print msg
print 'Flag = {}'.format(msg[ msg.find('BALSN') : msg.find('}')+1 ])

exit()

#for cycle in range(1, len(c)):
cycle = 13
keys = dfs(0, cycle)
for key in keys:
    msg = ['x' for i in c]
    j = 0
    for i in range(len(c)):
        msg[i] = chr(c[i] ^ key[j])
        j += 1
        if j == cycle:
            j = 0
    print cycle
    print ''.join(msg)
    a=raw_input()

