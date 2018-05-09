from pwn import *
from Crypto.Cipher import AES

def subproblem1():
    print 'Subproblem 1:'
    m = 'QQ Homework is too hard, how2decrypt QQ'
    c = '296e12d608ad04bd3a10b71b9eef4bb6ae1d697d1495595a5f5b98e409d7a7c437f24e69feb250b347db0877a40085a9'.decode('hex')
    [m1, m2, m3] = [m[16*i:16*(i+1)] for i in range(3)]
    [c1, c2, c3] = [c[16*i:16*(i+1)] for i in range(3)]
    r = remote('140.112.31.96', 10124)
    r.recvuntil('> ')
    r.sendline('1')
    r.recvuntil(': ')
    r.sendline(''.join([m1, c1, c2, c3]).encode('hex'))
    msg = r.recvline().strip()
    IV = msg[16:32]
    print 'Flag =', IV
    r.close()

def subproblem2():
    print 'Subproblem 2:'
    # CBC padding oracle attack
    r = remote('140.112.31.96', 10125)
    r.recvuntil('> ')
    r.sendline('1')
    r.recvuntil(':\n')
    IV = r.recvline().strip()
    r.recvuntil(':\n')
    c = r.recvline().strip().decode('hex')
    cb = [c[16*i:16*(i+1)] for i in range(len(c) / 16)]
    cb = [IV] + cb

    m = []
    r.recvuntil('QQ\n\n')
    for i in range(1, len(cb)):
        iv = cb[i-1]
        iv = [ord(j) for j in iv]
        b = cb[i]
        z = [0]*16
        for padding in range(1, 16+1):
            r.close()
            r = remote('140.112.31.96', 10125)
            r.recvuntil('> ')
            r.sendline('1')
            r.recvuntil(':\n')
            r.recvline()
            r.recvuntil(':\n')
            r.recvline()
            r.recvuntil('QQ\n\n')
            for j in range(32, 127):
                z[-padding] = j
                piv = [iv[p] ^ z[p] for p in range(16)]
                piv = [piv[p] if p < 16-padding else piv[p]^padding for p in range(16)]
                piv = [chr(p) for p in piv]
                piv = ''.join(piv)
                r.sendline((piv+b).encode('hex'))
                result = r.readline().strip()
                if result == 'Success':
                    break
        m.append(''.join([chr(p) for p in z]))
        print '[Log] partial flag:', ''.join(m)

    Flag = ''.join(m)
    print 'Flag =', Flag
    r.close()

def subproblem3():
    print 'Subproblem 3:'
    # Flag length = 64 bits = 4 blocks
    r = remote('140.112.31.96', 10126)

    z = [['-'] * 16 for _ in range(4)]
    for i in range(16):
        m1 = 'a' * i
        m2 = ''
        r.close()
        r = remote('140.112.31.96', 10126)
        r.recvuntil('> ')
        r.sendline('1')
        r.recvuntil(':')
        r.sendline(m1)
        r.recvuntil(':')
        r.sendline(m2)
        r.recvline()
        c = r.recvline().strip().decode('hex')
        CB = [ c[16*k:16*(k+1)] for k in range(len(c) / 16)]
        IV = CB[0]
        assert(len(CB) == 7)
        for j in range(4):
            c0 = CB[j]
            c1 = CB[j+1]
            for k in range(32, 127):
                nc0 = [ord(p) for p in c0]
                nc0[-1] ^= k ^ (32 + 16 - i - 1)
                nc0 = ''.join([chr(p) for p in nc0])
                r.recvuntil('> ')
                r.sendline('2')
                r.recvuntil(': ')
                r.sendline(''.join(CB + [nc0, c1]).encode('hex'))
                msg = r.recvline()
                if 'MAC' in msg:
                    # wrong guess try again
                    pass
                elif 'Success' in msg:
                    z[j][-i-1] = chr(k)
                    break
                else:
                    print ('ERROR: Server decryption fail.')
        print ''.join([''.join(p) for p in z])

    Flag = ''.join([''.join(p) for p in z])
    print 'Flag =', Flag
    r.close()


#subproblem1()
#subproblem2()
subproblem3()
