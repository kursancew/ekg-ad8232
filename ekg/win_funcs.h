#ifndef _WIN_FUNCS_H_
#define _WIN_FUNCS_H_

template <typename T,
          int N>
class AvgWindow {
    T w[N];
    T sum;
    size_t i;
    void incr() {
        ++i;
        i %= N;
    }
public:
    AvgWindow() : w{}, sum(0), i(0) {}
    T push(const T& v) {
        sum += v - w[i];
        w[i] = v;
        incr();
        return sum/N;
    }
    T avg() {
        return sum/N;
    }
};

template <typename T,
          int N,
          int fSCALE=1>
class ConvWindow {
    T w[N];
    T f[N];
    size_t i;
    T sfact;
    void incr_mod(size_t& i) {
        ++i;
        i %= N;
    }
    T conv() {
        auto tmpi = i;
        T sum = 0;
        for(size_t j = 0; j < N; ++j) {
            sum += f[j] * w[tmpi];
            incr_mod(tmpi);
        }
        return sum/(N*fSCALE);
    }
public:
    ConvWindow(const T* f_, T sfact_=1) : w{}, f{}, i(0), sfact(sfact_) {for(int j=0; j<N;++j)f[j]=f_[j];}
    T push(const T& v) {
        w[i] = v;
        auto r = conv();
        incr_mod(i);
        return r;
    }
};

template <int N, bool MIN, int LIM=10000>
class MinMaxN {
  int m[N];
  unsigned long t[N];
public:
  MinMaxN() {reset();}
  //void print() { for(auto i : m) cout << i << '\n';}
  void reset() {
    memset(m, MIN?LIM:-LIM, sizeof(m[0])*N);
  }
  void push(int v, unsigned long vt) {
    for(int i = 0; i < N; ++i) {
      bool pred = MIN?v < m[i]:v > m[i];
      if (pred) {
        for (int j = N-1; j > i; --j) {
          m[j] = m[j - 1];
          t[j] = t[j - 1];
        }
        t[i] = vt;
        m[i] = v;
        break;
      }
    }
  }
};
#endif // _WIN_FUNCS_H_
