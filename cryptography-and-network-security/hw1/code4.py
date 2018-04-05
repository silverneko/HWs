#!/use/bin/env python2

from pwn import *

r = remote('140.112.31.96', 10120)

# warmup
r.recvuntil('= ')
m1 = r.recvline().rstrip('\n\r')
r.recvuntil(': ')
r.sendline(m1)

# round 1
# 1 -> a, 2 -> b ...
print 'Round 1'
r.recvuntil('c1 = ')
c1 = r.recvline().rstrip('\n\r')
r.recvuntil(': ')

m1 = []
for tok in c1.split():
    res = ''
    for i in range(0, len(tok), 2):
        res += chr(ord('a')+int(tok[i:i+2])-1)
    m1.append(res)
m1 = ' '.join(m1)

print 'c1 = ', c1
print 'm1 = ', m1

r.sendline(m1)
r.recvuntil('FLAG_PIECES: ')
flag = r.recvline().rstrip('\n\r')

# round 2
# Caesar cipher
print ''
print 'Round 2'
r.recvuntil('m1 = ')
m1 = r.recvline().rstrip('\n\r')
r.recvuntil('c1 = ')
c1 = r.recvline().rstrip('\n\r')
r.recvuntil('c2 = ')
c2 = r.recvline().rstrip('\n\r')
r.recvuntil(': ')

def rot(msg, k):
    m = ''
    for c in msg:
        if c == ' ':
            m += ' '
        elif c.islower():
            m += chr((ord(c) - ord('a') + k + 26) % 26 + ord('a'))
        else:
            m += chr((ord(c) - ord('A') + k + 26) % 26 + ord('A'))
    return m

k = ord(c1[0]) - ord(m1[0])
m2 = rot(c2, -k)

print 'c1 = ', c1
print 'm1 = ', m1
print 'c2 = ', c2
print 'm2 = ', m2

r.sendline(m2)
r.recvuntil('FLAG_PIECES: ')
flag += r.recvline().rstrip('\n\r')

# round 3
# Caesar cipher again
print ''
print 'Round 3'
r.recvuntil('c1 = ')
c1 = r.recvline().rstrip('\n\r')
r.recvuntil(': ')

for k in range(26):
    print k, rot(c1, k)

k = int(raw_input('Choose a k: '))
m1 = rot(c1, k)

print 'c1 = ', c1
print 'm1 = ', m1

r.sendline(m1)
r.recvuntil('FLAG_PIECES: ')
flag += r.recvline().rstrip('\n\r')

# round 4
# character subsitution
print ''
print 'Round 4'
r.recvuntil('m1 = ')
m1 = r.recvline().rstrip('\n\r')
r.recvuntil('c1 = ')
c1 = r.recvline().rstrip('\n\r')
r.recvuntil('c2 = ')
c2 = r.recvline().rstrip('\n\r')
r.recvuntil(': ')

mapping = {' ': ' '}
for i in range(len(c1)):
    mapping[c1[i].upper()] = m1[i].upper()
    mapping[c1[i].lower()] = m1[i].lower()
m2 = ''
ambiguous = []
for i in range(len(c2)):
    if mapping.has_key(c2[i]):
        m2 += mapping[c2[i]]
    else:
        m2 += '-'
        ambiguous += c2[i].lower()
ambiguous = set(ambiguous)

print 'c1 = ', c1
print 'm1 = ', m1
print 'c2 = ', c2
if len(ambiguous) > 0:
    print 'm2 = ', m2
    print 'Solve ambiguity:'
    for c in ambiguous:
        m = raw_input(c + ': ').rstrip('\n\r')
        mapping[c] = m
        mapping[c.upper()] = m.upper()
    m2 = ''
    for i in range(len(c2)):
        m2 += mapping[c2[i]]
print 'm2 = ', m2

r.sendline(m2)
r.recvuntil('FLAG_PIECES: ')
flag += r.recvline().rstrip('\n\r')

# round 5
# m[i] = c[k * i mod len(m)] and k _|_ len(m)
print ''
print 'Round 5'
r.recvuntil('m1 = ')
m1 = r.recvline().rstrip('\n\r')
r.recvuntil('c1 = ')
c1 = r.recvline().rstrip('\n\r')
r.recvuntil('c2 = ')
c2 = r.recvline().rstrip('\n\r')
r.recvuntil(': ')

assert(len(m1) == len(c1))
assert(len(c2) == len(c1))

def round5(cipher_text, k):
    p = 0
    m = ''
    l = len(cipher_text)
    if k < 0:
        k += l
    while k < 0:
        k += len(cipher_text)
    for i in range(len(cipher_text)):
        m += cipher_text[p]
        p = p + k
        if p >= l:
            p -= l
    return m

def gcd(p, q):
    if q == 0:
        return p
    return gcd(q, p % q)

def coprime(p, q):
    return gcd(p, q) == 1

def inv(q, p):
    """
    calculate q^-1 mod p
    """
    for i in range(p):
        if q * i % p == 1:
            return i

l = len(m1)
k = 0
for i in range(l):
    if coprime(i, l):
        if round5(m1, i) == c1:
            k = i
            break
assert(k != 0)
m2 = round5(c2, inv(k, l))

print 'c1 = ', c1
print 'm1 = ', m1
print 'c2 = ', c2
print 'm2 = ', m2
print 'k, inv(k) = ', (k, inv(k, l))

r.sendline(m2)
r.recvuntil('FLAG_PIECES: ')
flag += r.recvline().rstrip('\n\r')

# round 6
# rail fence cipher
print ''
print 'Round 6'
r.recvuntil('m1 = ')
m1 = r.recvline().rstrip('\n\r')
r.recvuntil('c1 = ')
c1 = r.recvline().rstrip('\n\r')
r.recvuntil('c2 = ')
c2 = r.recvline().rstrip('\r\n')
r.recvuntil(': ')

def rail_fence_encrypt(m, k):
    assert(k >= 3)
    bucket = [''] * k
    p = 0
    d = 1
    for i in range(len(m)):
        bucket[p] += m[i]
        p += d
        if p == 0:
            d = 1
        elif p == k - 1:
            d = -1
    return ''.join(bucket)

def rail_fence_decrypt(c, k):
    t = k * 2 - 2
    l = len(c)
    cycles = int(l / t)
    p = 0
    m = [[]] * l
    for j in range(k):
        for i in range(cycles+1):
            if t * i + j < l:
                m[t * i + j] = c[p]
                p += 1
            if j != 0 and j != k-1 and t * i + t - j < l:
                m[t * i + t - j] = c[p]
                p += 1
    return ''.join(m)


k = 0
for i in range(3, len(m1)):
    if rail_fence_encrypt(m1, i) == c1:
        k = i
        break
assert(k != 0)
m2 = rail_fence_decrypt(c2, k)

print 'c1 = ', c1
print 'm1 = ', m1
print 'c2 = ', c2
print 'm2 = ', m2
print 'k = ', k

r.sendline(m2)
r.recvuntil('FLAG_PIECES: ')
flag += r.recvline().rstrip('\n\r')
r.close()

print ''
print 'flag = ', flag

flag = base64.b64decode(flag)
with open('flag4.png', 'w') as f:
    f.write(flag)
