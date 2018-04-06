from pwn import *
import hashlib
import subprocess
import time

def sha256(msg):
    return hashlib.sha256(msg).digest()

def MAC(msg):
    return base64.b64encode(sha256(msg))

# Magic number!
secret_length = 21
print 'secret_length =', secret_length

r = remote('140.112.31.96', 10122)

print r.recvuntil('to me: ')

msg = 'admin||9999'
r.sendline(msg + '||' + MAC(msg))

Ns = r.recvline()
Ns = Ns.split('||')[0]

r2 = remote('140.112.31.96', 10122)
r2.recvuntil('to me: ')
msg = 'admin||' + Ns
r2.sendline(msg + '||' + MAC(msg))
forge_sig = r2.recvline().rstrip('\r\n')
forge_sig = forge_sig.split('||')[1]
forge_sig = base64.b64decode(forge_sig)
r2.close()

# forge_sig = sha256(password||admin||Ns||login)
# ext_sig = sha256(password||admin||Ns||login@@@@@@||printflag)

print 'export DATA=\'{}\''.format('admin||'+Ns+'||login')
print 'export SIG={}'.format(forge_sig.encode('hex'))
print 'Use hash_extender to extend data'
#ext_data = raw_input('New data (hex): ').strip().decode('hex')
#ext_sig = raw_input('New signature (hex): ').strip().decode('hex')

command = [
    './hash_extender/hash_extender',
    '--format', 'sha256',
    '--out-data-format', 'hex',
    '--data', 'admin||' + Ns + '||login',
    '--signature', forge_sig.encode('hex'),
    '--append', '||printflag',
    '-l', str(secret_length)
]
p = subprocess.Popen(command, stdout=subprocess.PIPE)
out, err = p.communicate()
if err:
    print (err)

out = out.strip().split('\n')
ext_data = out[-1].strip().split()[-1].decode('hex')
ext_sig = out[-2].strip().split()[-1].decode('hex')

print 'New data: ', ext_data
print 'New sig: ', ext_sig

payload = base64.b64encode(ext_data) + '||' + base64.b64encode(ext_sig)

print r.recvuntil('action))')
r.sendline(payload)
time.sleep(1)
print r.recv()
r.close()

#r.interactive()
