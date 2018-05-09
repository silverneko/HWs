from OpenSSL import crypto
from base64 import b64decode, b64encode
from pwn import *

r = remote('140.112.31.96', 10129)
#r = process(['python3', 'check.py'], cwd='release/tls-cert/')

my_cert = crypto.X509()
with open('csie.pem', 'rb') as f:
    csie_cert = crypto.load_certificate(crypto.FILETYPE_PEM, f.read())

my_cert.set_serial_number(0x112233)
my_cert.gmtime_adj_notBefore(0)
my_cert.gmtime_adj_notAfter(1000000000)
my_cert.set_subject(csie_cert.get_subject())
my_cert.set_pubkey(csie_cert.get_pubkey())

with open('rootCA.crt', 'rb') as f:
    root_cert = crypto.load_certificate(crypto.FILETYPE_PEM, f.read())
    #print crypto.dump_certificate(crypto.FILETYPE_TEXT, root_cert)

my_cert.set_issuer(root_cert.get_subject())

with open('rootCA.key', 'r') as f:
    rsa_key = crypto.load_privatekey(crypto.FILETYPE_PEM, f.read())
    my_cert.sign(rsa_key, 'sha256WithRSAEncryption')

print crypto.dump_certificate(crypto.FILETYPE_TEXT, my_cert)

my_cert_pem = crypto.dump_certificate(crypto.FILETYPE_PEM, my_cert)
r.sendline(b64encode(my_cert_pem))

r.interactive()
