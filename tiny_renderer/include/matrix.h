#ifndef MATRIX_H
#define MATRIX_H

#include "vec.h"
#include <cassert>
#include <cmath>
#include <initializer_list>
#include <vector>

template <size_t N> class matrix {
public:
  double arr[N][N]{};

  /* constructors */
  matrix() = default;

  matrix(std::initializer_list<std::initializer_list<double>> list) {
    int i = 0;
    for (const std::initializer_list row : list) {
      int j = 0;
      for (const double elm : row) {
        arr[i][j] = elm;
        j++;
      }
      i++;
    }
  }
  /* access */
  double *operator[](size_t i) { return arr[i]; }
  const double *operator[](size_t i) const { return arr[i]; }

  double &operator()(size_t r, size_t c) { return arr[r][c]; }
  double operator()(size_t r, size_t c) const { return arr[r][c]; }

  /* column extraction */
  std::vector<double> get_col(size_t i) const {
    std::vector<double> col(N);
    for (size_t j = 0; j < N; j++)
      col[j] = arr[j][i];
    return col;
  }

  /* arithmetic */
  matrix operator-() const {
    matrix r;
    for (size_t i = 0; i < N; i++)
      for (size_t j = 0; j < N; j++)
        r.arr[i][j] = -arr[i][j];
    return r;
  }

  matrix &operator+=(const matrix &m) {
    for (size_t i = 0; i < N; i++)
      for (size_t j = 0; j < N; j++)
        arr[i][j] += m.arr[i][j];
    return *this;
  }

  matrix &operator-=(const matrix &m) {
    for (size_t i = 0; i < N; i++)
      for (size_t j = 0; j < N; j++)
        arr[i][j] -= m.arr[i][j];
    return *this;
  }

  matrix &operator*=(double t) {
    for (size_t i = 0; i < N; i++)
      for (size_t j = 0; j < N; j++)
        arr[i][j] *= t;
    return *this;
  }

  matrix &operator/=(double t) { return (*this) *= (1.0 / t); }

  matrix transpose() const {
    matrix r;
    for (size_t i = 0; i < N; i++)
      for (size_t j = 0; j < N; j++)
        r.arr[j][i] = arr[i][j];
    return r;
  }

  /* required declarations */
  double determinant() const;
  matrix inverse() const;
};

/* free operators */

template <size_t N> matrix<N> operator+(matrix<N> a, const matrix<N> &b) {
  return a += b;
}

template <size_t N> matrix<N> operator-(matrix<N> a, const matrix<N> &b) {
  return a -= b;
}

template <size_t N>
matrix<N> operator*(const matrix<N> &a, const matrix<N> &b) {
  matrix<N> r;
  for (size_t i = 0; i < N; i++)
    for (size_t j = 0; j < N; j++) {
      r[i][j] = 0.0;
      for (size_t k = 0; k < N; k++)
        r[i][j] += a[i][k] * b[k][j];
    }
  return r;
}

template <size_t N> vec<N> operator*(const matrix<N> &a, const vec<N> &v) {
  vec<N> r;
  for (size_t i = 0; i < N; i++) {
    r[i] = 0.0;
    for (size_t k = 0; k < N; k++)
      r[i] += a[i][k] * v[k];
  }
  return r;
}

/* ==== 3×3 SPECIALIZATION ==== */

template <> inline double matrix<3>::determinant() const {
  return arr[0][0] * (arr[1][1] * arr[2][2] - arr[1][2] * arr[2][1]) -
         arr[0][1] * (arr[1][0] * arr[2][2] - arr[1][2] * arr[2][0]) +
         arr[0][2] * (arr[1][0] * arr[2][1] - arr[1][1] * arr[2][0]);
}

template <> inline matrix<3> matrix<3>::inverse() const {
  matrix<3> inv;
  double det = determinant();
  assert(std::fabs(det) > 1e-12);

  double inv_det = 1.0 / det;

  inv[0][0] = (arr[1][1] * arr[2][2] - arr[1][2] * arr[2][1]) * inv_det;
  inv[0][1] = -(arr[0][1] * arr[2][2] - arr[0][2] * arr[2][1]) * inv_det;
  inv[0][2] = (arr[0][1] * arr[1][2] - arr[0][2] * arr[1][1]) * inv_det;

  inv[1][0] = -(arr[1][0] * arr[2][2] - arr[1][2] * arr[2][0]) * inv_det;
  inv[1][1] = (arr[0][0] * arr[2][2] - arr[0][2] * arr[2][0]) * inv_det;
  inv[1][2] = -(arr[0][0] * arr[1][2] - arr[0][2] * arr[1][0]) * inv_det;

  inv[2][0] = (arr[1][0] * arr[2][1] - arr[1][1] * arr[2][0]) * inv_det;
  inv[2][1] = -(arr[0][0] * arr[2][1] - arr[0][1] * arr[2][0]) * inv_det;
  inv[2][2] = (arr[0][0] * arr[1][1] - arr[0][1] * arr[1][0]) * inv_det;

  return inv;
}

#endif
