#pragma once

#include <string>
#include <vector>
#include <array>
#include <exception>
#include <algorithm>
#include <type_traits>

#include "xutil.hpp"
#include "xdatastream.hpp"
#include "xarray.hpp"


namespace xmat
{

class SerializationError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class DeserializationError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};


/// \param[in] N    total size of shape buffer
template<typename T>
bool check_shape_1d(size_t numel, const T* s, size_t ndim, size_t N) {
  T k = 0, m = 0;
  for(T n = 0; n < ndim; ++n) {
    const T a = s[n];
    if(a == numel) { ++k; } 
    else if(a == 1) { ++m; }
  }
  bool out = k == 1 && m == ndim - 1;
  for(size_t n = ndim; n < N; ++n) { out = out && s[n] == 0; }
  return out;
}

bool check_shape_1d(const XBlock& block) {
  return check_shape_1d(block.numel(), block.shape().data(), 
                        block.ndim(), block.shape().size());
}


///////////////////////////////////////////////////////////////
namespace serial {

// scalar
////////////////////
template<typename T>
struct Dump<T, std::enable_if_t<DataStreamType<T>::enabled>> {
  static const bool enabled = true;

  template<typename ODStreamT>
  static void dump(XBlock& block, ODStreamT& ods, const T& x) { 
    block.o_ = 'C';
    block.t_ = DataStreamType<T>::id;
    block.s_ = 0;
    block.shape_.fill(0);
    
    block.dump(ods);
    ods.write(x);
  }
};

template<typename T>
struct LoadTo<T, std::enable_if_t<DataStreamType<T>::enabled>> {
  static const bool enabled = true;

  template<typename IDStreamT>
  static void load(const XBlock& block, IDStreamT& ids, T& y) {
    if(block.t_ != DataStreamType<T>::id) {
      throw DeserializationError("Scalar load(): wrong scalar type");
    }
    if(block.numel() != 1) {
      throw DeserializationError("Scalar load(): wrong numel|ndim");
    }
    if(!std::all_of(block.shape_.begin(), block.shape_.end(), 
                    [](sf::xsize_t i){return i == 0; })) {
      throw DeserializationError("Scalar load(): wrong shape ");
    }
    ids.read(y);
  }
};

template<typename T>
struct Load<T, std::enable_if_t<DataStreamType<T>::enabled>> { 
  static const bool enabled = true;

  template<typename IDStreamT>
  static T load(const XBlock& block, IDStreamT& ids) {
    T y;
    LoadTo<T>::load(block, ids, y);
    return y; 
  }
};


// pointer
///////////////////////////////
template<typename T>
struct DumpPtr<T, std::enable_if_t<DataStreamType<T>::enabled>> {
  static const bool enabled = true;

  template<typename ODStreamT>
  static void dump(XBlock& block, ODStreamT& ods, const T* xptr, size_t n) { 
    block.o_ = 'C';
    block.t_ = DataStreamType<T>::id;
    block.s_ = 1;
    block.shape_ = {n};
    
    block.dump(ods);
    ods.write(xptr, n);
  }
};

template<typename T>
struct LoadPtr<T, std::enable_if_t<DataStreamType<T>::enabled>> {
  static const bool enabled = true;

  template<typename IDStreamT>
  static void load(const XBlock& block, IDStreamT& ids, T* yptr) { 
    if(block.t_ != DataStreamType<T>::id) {
      throw DeserializationError("Scalar load(): wrong scalar type");
    }
    if(!check_shape_1d(block)) {
      throw DeserializationError("pointer load(): wrong shape ");
    }
    
    ids.read(yptr, block.numel());
  }
};


// ---------------------------------------------------------
// std containers continuous sequense: vector, string, array
// ---------------------------------------------------------
namespace impl_std {

  template<typename U, typename ODStreamT>
  void dump(XBlock& block, ODStreamT& ods, const U& x) {
    using T = typename U::value_type;
    return serial::DumpPtr<T>::dump(block, ods, &x[0], x.size());
  }

  template<typename U, typename IDStream>
  void load_to_resizable(const XBlock& block, IDStream& ids, U& y) {
    using T = typename U::value_type;
    y.resize(block.numel());
    serial::LoadPtr<T>::load(block, ids, &y[0]);
  }
} // namespase impl_std



// std::vector
///////////////////////////////
template<typename T, typename A>
struct Dump<std::vector<T, A>, std::enable_if_t<DataStreamType<T>::enabled>> {
  static const bool enabled = true;

  template<typename ODStreamT>
  static void dump(XBlock& block, ODStreamT& ods, const std::vector<T, A>& x) { 
    return impl_std::dump(block, ods, x);
  }
};

template<typename T, typename A>
struct LoadTo<std::vector<T, A>, std::enable_if_t<DataStreamType<T>::enabled>> {
  static const bool enabled = true;

  template<typename IDStreamT>
  static void load(const XBlock& block, IDStreamT& ids, std::vector<T, A>& y) {
    impl_std::load_to_resizable(block, ids, y);
  }
};

template<typename T, typename A>
struct Load<std::vector<T, A>, std::enable_if_t<DataStreamType<T>::enabled>> {
  static const bool enabled = true;

  template<typename IDStreamT>
  static std::vector<T, A> load(const XBlock& block, IDStreamT& ids) {
    std::vector<T, A> y;
    impl_std::load_to_resizable(block, ids, y); 
    return y;
  }
};


// std::string
///////////////////////////////
template<>
struct Dump<std::string, void> {
  static const bool enabled = true;

  template<typename ODStreamT>
  static void dump(XBlock& block, ODStreamT& ods, const std::string& x) {
    return impl_std::dump(block, ods, x);
  }
};

template<>
struct LoadTo<std::string, void> {
  static const bool enabled = true;

  template<typename IDStreamT>
  static void load(const XBlock& block, IDStreamT& ids, std::string& y) {
    impl_std::load_to_resizable(block, ids, y);
  }
};

template<>
struct Load<std::string, void> {
  static const bool enabled = true;

  template<typename IDStreamT>
  static std::string load(const XBlock& block, IDStreamT& ids) {
    std::string y;
    impl_std::load_to_resizable(block, ids, y); 
    return y;
  }
};


// std::array
///////////////////////////////
template<typename T, size_t N>
struct Dump<std::array<T, N>, std::enable_if_t<DataStreamType<T>::enabled>> {
  static const bool enabled = true;

  template<typename ODStreamT>
  static void dump(XBlock& block, ODStreamT& ods, const std::array<T, N>& x) {
    return impl_std::dump(block, ods, x);
  }
};

template<typename T, size_t N>
struct LoadTo<std::array<T, N>, std::enable_if_t<DataStreamType<T>::enabled>> {
  static const bool enabled = true;

  template<typename IDStreamT>
  static void load(const XBlock& block, IDStreamT& ids, std::array<T, N>& y) {
    if(block.numel() != y.size()) {
      throw DeserializationError("wrong size for std::array<T,N>");
    }
    serial::LoadPtr<T>::load(block, ids, y.data());
  }
};

template<typename T, size_t N>
struct Load<std::array<T, N>, std::enable_if_t<DataStreamType<T>::enabled>> {
  static const bool enabled = true;

  template<typename IDStreamT>
  static std::array<T, N> load(const XBlock& block, IDStreamT& ids) {
    std::array<T, N> y;
    LoadTo<std::array<T, N>>::load(block, ids, y);
    return y;
  }
};


// xmat::NArrayInterface_
////////////////////////////////////////////////////////////////////////////////
template<typename Derived, typename T, size_t ND, MOrder MOrderT, typename IntT>
struct Dump<NArrayInterface_<Derived, T, ND, MOrderT, IntT>, 
            std::enable_if_t<DataStreamType<T>::enabled>>
{
  static const bool enabled = true;

  template<typename ODStreamT>
  static void dump(XBlock& block, 
                   ODStreamT& ods, 
                  const NArrayInterface_<Derived, T, ND, MOrderT, IntT>& x) 
  {
    block.o_ = 'C';
    block.t_ = DataStreamType<T>::id;
    block.s_ = x.ndim;
    
    block.shape_.fill(0);
    assert(x.ndim <= block.shape().size());
    std::copy_n(x.shape().begin(), x.ndim, block.shape_.begin());
     
    // write:
    block.dump(ods);

    const bool iscontig1 = x.ravel().leaststride() == 1;
    if (iscontig1) {
      for (auto it = x.wbegin(), end = x.wend(); it != end; ++it) {
        ods.write(it.data(), it.length());
      }
    }
    else {
      for (auto it = x.wbegin(), end = x.wend(); it != end; ++it) {
        for (auto it_ : it) { ods.write(it_); }
      }
    }
  }
};

template<typename Derived, typename T, size_t ND, MOrder MOrderT, typename IntT>
struct LoadTo<NArrayInterface_<Derived, T, ND, MOrderT, IntT>, 
              std::enable_if_t<DataStreamType<T>::enabled>>
{
  static const bool enabled = true;

  template<typename IDStreamT>
  static void load(XBlock& block, 
                   IDStreamT& ids, 
                  NArrayInterface_<Derived, T, ND, MOrderT, IntT>& y) 
  {
    if(block.t_ != DataStreamType<T>::id) {
      throw DeserializationError("std::container load(): wrong scalar type");
    }
    if(block.ndim() > ND) {
      throw DeserializationError("wrong ndim");
    }
    if(!std::equal(y.shape().cbegin(), 
                   y.shape().cbegin() + block.ndim(), 
                   block.shape().cbegin())) {
      throw DeserializationError("wrong array's shape");
    }

    // read
    const bool iscontig = y.ravel().leaststride() == 1;
    if (iscontig) {
      for (auto it = y.wbegin(), end = y.wend(); it != end; ++it) {
        ids.read(it.data(), it.length());
      }
    }
    else {
      for (auto it = y.wbegin(), end = y.wend(); it != end; ++it) {
        for (auto it_ : it) { ids.read(it_); }
      }
    }
  }
};


// xmat::NArray
////////////////////////////////////////////////////////////////////////////////
template<typename T, size_t ND, class MemSourceT, MOrder MOrderT, typename IntT>
struct Dump<NArray_<T, ND, MemSourceT, MOrderT, IntT>, 
            std::enable_if_t<DataStreamType<T>::enabled>>
{
  static const bool enabled = true;
  using array_t = NArray_<T, ND, MemSourceT, MOrderT, IntT>;

  template<typename ODStreamT>
  static void dump(XBlock& block, ODStreamT& ods, const array_t& x) {
    Dump<typename array_t::base_t>::dump(block, ods, x);
  }
};

template<typename T, size_t ND, class MemSourceT, MOrder MOrderT, typename IntT>
struct LoadTo<NArray_<T, ND, MemSourceT, MOrderT, IntT>, 
            std::enable_if_t<DataStreamType<T>::enabled>>
{
  static const bool enabled = true;
  using array_t = NArray_<T, ND, MemSourceT, MOrderT, IntT>;

  template<typename IDStreamT>
  static void load(XBlock& block, IDStreamT& ids, array_t& y) {
    Load<typename array_t::base_t>::load(block, ids, y);
  }
};

template<typename T, size_t ND, class MemSourceT, MOrder MOrderT, typename IntT>
struct LoadArgs<NArray_<T, ND, MemSourceT, MOrderT, IntT>, 
                std::enable_if_t<DataStreamType<T>::enabled>>
{
  static const bool enabled = true;
  using array_t = NArray_<T, ND, MemSourceT, MOrderT, IntT>;

  template<typename IDStreamT, typename ... Args>
  static array_t load(XBlock& block,  IDStreamT& ids, Args&&... args) {
    typename array_t::index_t shape;
    shape.fill(1);
    std::copy_n(block.shape_.cbegin() + (ND - block.ndim()), block.ndim(), shape.begin());

    array_t y{shape, MemSourceT{std::forward<Args>(args)...}};
    LoadTo<typename array_t::base_t>::load(block, ids, y);
    return y;
  }
};

template<typename T, size_t ND, class MemSourceT, MOrder MOrderT, typename IntT>
struct Load<NArray_<T, ND, MemSourceT, MOrderT, IntT>, 
            std::enable_if_t<DataStreamType<T>::enabled 
            && NArray_<T, ND, MemSourceT, MOrderT, IntT>::k_has_default_constructor>>
{
  static const bool enabled = true;
  using array_t = NArray_<T, ND, MemSourceT, MOrderT, IntT>;

  template<typename IDStreamT>
  static array_t load(XBlock& block, IDStreamT& ids) {
    return LoadArgs<array_t>::load(block, ids, MemSourceT{});
  }
};
} // namespase serial
} // namespace xmat
