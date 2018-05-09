import hashlib
from pwn import *

def connect():
    return remote('140.112.31.96', 10127)

p = 262603487816194488181258352326988232210376591996146252542919605878805005469693782312718749915099841408908446760404481236646436295067318626356598442952156854984209550714670817589388406059285064542905718710475775121565983586780136825600264380868770029680925618588391997934473191054590812256197806034618157751903

pwd = [0] * 3
for i in range(3):
    for k in range(1, 21):
        g = int(hashlib.sha512(str(k)).hexdigest(), 16)
        g = pow(g, 2, p)
        if not 514 <= g < p-514:
            continue

        r1 = connect()
        r2 = connect()
        for j in range(3):
            r1.recvuntil(':')
            B1 = r1.recvline().strip()
            r2.recvuntil(':')
            B2 = r2.recvline().strip()
            if j == i:
                B1B2 = int(hashlib.sha512(B1).hexdigest(), 16)
                B1B2 ^= int(hashlib.sha512(B2).hexdigest(), 16)
                r1.recvuntil(':')
                r1.sendline(str(g))
                r2.recvuntil(':')
                r2.sendline(str(g))
            else:
                r1.recvuntil(':')
                r1.sendline(B2)
                r2.recvuntil(':')
                r2.sendline(B1)

        r1.recvuntil(':')
        r2.recvuntil(':')
        f1 = int(r1.recvline().strip())
        f2 = int(r2.recvline().strip())
        r1.close()
        r2.close()
        if f1 ^ f2 == B1B2:
            pwd[i] = k
            break

print 'PWD =', pwd
r = connect()
key = 0
for i in range(3):
    g = int(hashlib.sha512(str(pwd[i])).hexdigest(), 16)
    g = pow(g, 2, p)
    r.recvuntil(':')
    B = r.recvline().strip()
    r.recvuntil(':')
    r.sendline(str(g))
    key ^= int(hashlib.sha512(B).hexdigest(), 16)

r.recvuntil(':')
Flag = int(r.recvline().strip())
Flag ^= key
Flag = '{:x}'.format(Flag).decode('hex')

print 'Flag =', Flag
