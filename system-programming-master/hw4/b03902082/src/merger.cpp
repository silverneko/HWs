#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <sstream>

#include <unistd.h>
#include <pthread.h>

using namespace std;

int segmentSize, n;
int * a, * b;

pthread_mutex_t IOMutex = PTHREAD_MUTEX_INITIALIZER;

void twrite(const string& s){
  pthread_mutex_lock(&IOMutex);
  cout << s;
  pthread_mutex_unlock(&IOMutex);
}

void * sortRoutine(void * arg){
  int i = *(int*) arg;
  delete (int*)arg;
  arg = NULL;
  /* end of prologue */
  int lb, rb;
  lb = segmentSize * i;
  rb = min(n, segmentSize * (i+1));
  ostringstream oss;
  oss << "Handling elements:\n";
  for(int i = lb; i < rb; ++i){
    oss << a[i];
    if(i != rb - 1){
      oss << ' ' ;
    }else{
      oss << '\n';
    }
  }
  oss << "Sorted " << rb - lb << " elements.\n";

  sort(a + lb, a + rb);

  twrite(oss.str());
  pthread_exit(NULL);
}

void * mergeRoutine(void * arg){
  int i = *(int*) arg;
  delete (int*)arg;
  arg = NULL;
  /* end of prologue */
  int lb, rb;
  lb = segmentSize * i;
  rb = min(n, segmentSize * (i+1));
  if(rb - lb <= segmentSize / 2){
    pthread_exit(NULL);
  }
  ostringstream oss;
  oss << "Handling elements:\n";
  for(int i = lb; i < rb; ++i){
    oss << a[i];
    if(i != rb - 1){
      oss << ' ' ;
    }else{
      oss << '\n';
    }
  }
  int dupCount = 0;
  int l1 = lb, r1 = l1 + segmentSize / 2, l2 = r1, r2 = rb;
  for(int i = lb; i < rb; ++i){
    if(l1 >= r1){
      b[i] = a[l2];
      l2++;
    }else if(l2 >= r2){
      b[i] = a[l1];
      l1++;
    }else{
      if(a[l1] < a[l2]){
        b[i] = a[l1];
        l1++;
      }else if(a[l1] > a[l2]){
        b[i] = a[l2];
        l2++;
      }else{
        b[i] = a[l1];
        l1++;
        dupCount++;
      }
    }
  }
  for(int i = lb; i < rb; ++i){
    a[i] = b[i];
  }
  oss << "Merged " << segmentSize / 2 << " and "
      << rb - lb - segmentSize / 2 << " elements with "
      << dupCount << " duplicates.\n";

  twrite(oss.str());
  pthread_exit(NULL);
}

int main(int argc, char *argv[]){
  if(argc < 2){
    cerr << "argv[1] should be the segment size" << endl;
    exit(1);
  }
  segmentSize = atoi(argv[1]);
  cin >> n;
  a = new int[n];
  b = new int[n];
  for(int i = 0; i < n; ++i){
    cin >> a[i];
  }
  int tn = n / segmentSize;
  if(n % segmentSize != 0){
    ++tn;
  }
  pthread_t * threads = new pthread_t[tn];
  for(int i = 0; i < tn; ++i){
    pthread_create(&threads[i], NULL, sortRoutine, new int(i));
  }
  for(int i = 0; i < tn; ++i){
    pthread_join(threads[i], NULL);
  }
  do{
    segmentSize *= 2;
    tn = n / segmentSize;
    if(n % segmentSize != 0){
      ++tn;
    }
    for(int i = 0; i < tn; ++i){
      pthread_create(&threads[i], NULL, mergeRoutine, new int(i));
    }
    for(int i = 0; i < tn; ++i){
      pthread_join(threads[i], NULL);
    }
  }while(segmentSize < n);

  for(int i = 0; i < n; ++i){
    cout << a[i];
    if(i != n-1){
      cout << ' ';
    }else{
      cout << '\n';
    }
  }
  cout << endl;
  return 0;
}

