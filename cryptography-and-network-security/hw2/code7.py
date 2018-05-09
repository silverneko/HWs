from pwn import *

r = connect('140.112.31.96', 10128)
#r = process(['python', 'release/one-time-wallet/one-time-wallet.py'])

random_history = []

for i in range(100):
    r.recvuntil(':')
    address = r.recvline().strip()
    r.recvuntil('BTC')
    r.recvuntil(':')
    password = r.recvline().strip()

    address = [address[j*8:(j+1)*8] for j in range(len(address) / 8)]
    password = [password[j*8:(j+1)*8] for j in range(len(password) / 8)]
    for num in address+password:
        random_history.append(int(num, 16))

predictor = process(['mt19937predict'])
for num in random_history[:624]:
    predictor.sendline(str(num))
for num in random_history[624:]:
    assert(num == int(predictor.recvline()))

def genHexString(byte):
    s = ''
    for i in range(byte / 4):
        r = int(predictor.recvline())
        s += '{0:0{1}x}'.format(r, 8)
    return s

def genAddress():
    return genHexString(12)

def genPassword():
    return genHexString(20)

nextAddress = genAddress()
nextPassword = genPassword()

print 'Next address:', nextAddress
print 'Next password:', nextPassword

r.sendline(nextPassword)

r.interactive()
