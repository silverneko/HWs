from pwn import *
from base64 import b64encode, b64decode
import hashlib

def oracle(name, pwd):
    r = remote('140.112.31.96', 10123)
    r.recvuntil('>')
    r.sendline('0')
    r.recvuntil(':')
    r.sendline(name)
    r.recvuntil(':')
    r.sendline(pwd)
    r.recvuntil(':')
    msg = r.recvline()
    r.close()
    return b64decode(msg.strip())

#pt = 'login=' + name + '&role=guest' + '&pwd=' +pwd
# the server uses AES ECB mode to encrypt packet
# I want to forge malicious request to hack the server

# A = AES(login=A&role=guest&pwd=B12345678)
assert(len('login=A&role=guest&pwd=B12345678') == 32)
name = 'A'
pwd = 'B12345678'
A = oracle(name, pwd)[:32]

# B = AES(&pwd=12345678901)
name = 'A' * 15
pwd = 'B' * 11
B = oracle(name, pwd)[32:48]

# C = AES(login=AAAA&role=)
name = 'AAAA'
C = oracle(name, pwd)[:16]

# D = AES(admin + padding)
name = 'A'
pwd = 'B12345678admin'
D = oracle(name, pwd)[32:]

payload = b64encode(A + B + C + D)

r = remote('140.112.31.96', 10123)
r.recvuntil('>')
r.sendline('1')
r.recvuntil(':')
r.sendline(payload)
r.recvuntil(':')
r.sendline('A')
r.recvuntil(':')
r.sendline('B12345678')

r.recvuntil('>')
r.sendline('1')
r.recvuntil('p =')
p = int(r.recvline())
r.recvuntil('q =')
q = int(r.recvline())
r.recvuntil('g =')
g = int(r.recvline())
r.recvuntil('y =')
y = int(r.recvline())

r.recvuntil('>')
r.sendline('0')
r.recvuntil(':')
r.recvuntil(':')
transactionID1 = r.recvline().strip()
r.recvuntil(':')
r1, s1 = r.recvline().strip().split()
r.recvuntil(':')
transactionID2 = r.recvline().strip()
r.recvuntil(':')
r2, s2 = r.recvline().strip().split()

r1, s1, r2, s2 = int(r1), int(s1), int(r2), int(s2)
assert(r1 == r2)

h1 = int(hashlib.sha1(transactionID1).hexdigest(), 16)
h2 = int(hashlib.sha1(transactionID2).hexdigest(), 16)

def euclidean(a, b, x, c, d, y):
    if y == 0:
        return a, b, x
    q = x / y
    return euclidean(c, d, y, a - q * c, b - q * d, x - q * y)

def fast_pow(x, a, p):
    b = 1
    while a != 0:
        if a & 1:
            b = b * x % p
        x = x * x % p
        a >>= 1
    return b

def inv(x, q):
    r, _, g = euclidean(1, 0, x, 0, 1, q)
    assert(g == 1)
    return r

k, v, _ = euclidean(1, 0, s1-s2, 0, 1, q)
k = k * (h1 - h2) % q
xr = ((s1 * k - h1)% q)

h = int(hashlib.sha1('FLAG').hexdigest(), 16)
sig_r = fast_pow(g, k, p) % q
assert(sig_r == r1)
sig_s = inv(k, q) * (h + xr) % q

r.recvuntil('>')
r.sendline('2')
r.recvuntil('r =')
r.sendline(str(sig_r))
r.recvuntil('s =')
r.sendline(str(sig_s))

r.interactive()
