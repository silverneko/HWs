% Generate random 500x500 positive definite matrix
while true
  A = rand(500, 500);
  A = A * A';
  [L, p] = chol(A, 'lower');
  if p == 0
    break
  end
end

% Warm up
chol(A, 'lower');
tic;
for i = 1:100
  chol(A, 'lower');
end
choltime = toc / 100

% Warm up
threeFor(A);
tic;
for i = 1:100
  threeFor(A);
end
threeFortime = toc / 100

% Warm up
twoFor(A);
tic;
for i = 1:100
  twoFor(A);
end
twoFortime = toc / 100

% Warm up
oneFor(A);
tic;
for i = 1:100
  oneFor(A);
end
oneFortime = toc / 100

function L = threeFor(A)
  L = tril(A);
  n = size(L, 1);
  for k = 1:n
    L(k, k) = sqrt(L(k, k));
    L(k+1:n, k) = L(k+1:n, k) / L(k, k);
    for j = k+1:n
      for i = j:n
        L(i, j) = L(i, j) - L(i, k) * L(j, k);
      end
    end
  end
end

function L = twoFor(A)
  L = tril(A);
  n = size(L, 1);
  for k = 1:n
    L(k, k) = sqrt(L(k, k));
    L(k+1:n, k) = L(k+1:n, k) / L(k, k);
    for j = k+1:n
      L(j:n, j) = L(j:n, j) - L(j:n, k) * L(j, k);
    end
  end
end

function L = oneFor(A)
  L = tril(A);
  n = size(L, 1);
  for k = 1:n
    L(k, k) = sqrt(L(k, k));
    L(k+1:n, k) = L(k+1:n, k) / L(k, k);
    L(k+1:n, k+1:n) = L(k+1:n, k+1:n) - L(k+1:n, k) * L(k+1:n, k)';
  end
end
