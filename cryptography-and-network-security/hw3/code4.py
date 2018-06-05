from Crypto.Hash import keccak

def hash(x):
    sha3 = keccak.new(digest_bits=256)
    sha3.update(x)
    return sha3.hexdigest()

# sha3(146) = 0x313b2ea16b36f2e78c1275bfcca4e31f1e51c3a5d60beeefe6f4ec441e6f1dfc
for i in range(256):
    msg = chr(i)
    print (i, hash(msg))
