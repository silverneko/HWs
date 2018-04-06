with open('release/mersenne-rsa/mersenne-rsa.txt', 'r') as f:
    N = int(f.readline().split()[2])
    e = int(f.readline().split()[2])
    c = int(f.readline().split()[2])

for i in range(1128):
    j = 1128 - i
    p = 2 ** i - 1
    q = 2 ** j - 1
    if p * q == N:
        break

print 'p = 2 ^ {} - 1'.format(i)
print 'p = {}'.format(p)
print 'q = 2 ^ {} - 1'.format(j)
print 'q = {}'.format(q)

phi_n = (p - 1) * (q - 1)

def euclidean(a, b, x, c, d, y):
    if y == 0:
        return a, b, x
    q = x / y
    return euclidean(c, d, y, a - q * c, b - q * d, x - q * y)

# solve d, de = 1 (mod phi_n), d = e^-1 (mod phi_n)
d, _, _ = euclidean(1, 0, e, 0, 1, phi_n)

print 'e, d = {}, {}'.format(e, d)

def fast_pow(x, a, p):
    b = 1
    while a != 0:
        if a & 1:
            b = b * x % p
        x = x * x % p
        a >>= 1
    return b

m = fast_pow(c, d, N)

msg = []
while m != 0:
    msg.append(m % 256)
    m /= 256

msg = [chr(c) for c in msg]
msg = ''.join(reversed(msg))

print msg
