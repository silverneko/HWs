from pwn import *
import hashlib

"""
r = remote('140.112.31.96', 10121)

r.recvuntil('sha1(X)=')
hash = r.recvuntil(':')
hash = [c for c in hash if c not in '?:']
hash = ''.join(hash)
"""
hash = '5e3196'

print 'hash = {}'.format(hash)

counter = 0
table = {}
while True:
    m = '{:032x}'.format(counter).decode('hex')
    sig = hashlib.sha1(m).hexdigest()
    if sig[-6:] == hash:
        print sig
        if table.has_key(sig[:-6]):
            X = table[sig[:-6]]
            Y = counter
            break
        table[sig[:-6]] = counter
    counter += 1

print X, Y
print 'X = {}'.format(m.encode('hex'))
print 'H(X) = {}'.format(sig)
r.interactive()
