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

// scalar
/////////////////////
template <typename T>
struct Serializer<T, std::enable_if_t<DataStreamType<T>::id>> {
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

  template<typename IDStreamT>
  static void load_to(const XBlock& block, IDStreamT& ids, T& y) {
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

  template<typename IDStreamT>
  static T load(const XBlock& block, IDStreamT& ibuf) {
    T y;
    load_to(block, ibuf, y);
    return y;
  }
};


// pointer
/////////////////////
template <typename T>
struct Serializer<T*, std::enable_if_t<DataStreamType<T>::id>> {
  static const bool enabled = true;

  template<typename ODStreamT>
  static void dump_n(XBlock& block, ODStreamT& ods, const T* xptr, size_t n) {
    block.o_ = 'C';
    block.t_ = DataStreamType<T>::id;
    block.s_ = 1;
    block.shape_ = {n};
    
    block.dump(ods);
    ods.write(xptr, n);
  }

  // not save
  template<typename IDStreamT>
  static void load_to(const XBlock& block, IDStreamT& ids, T* yptr) {
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
    return Serializer<T*>::dump_n(block, ods, &x[0], x.size());
  }

  template<typename U, typename IDStream>
  void load_to_resizable(const XBlock& block, IDStream& ids, U& y) {
    using T = typename U::value_type;
    y.resize(block.numel());
    Serializer<T*>::load_to(block, ids, &y[0]);
  }
} // namespase impl_std


// vector
////////////////////////////////
template<typename T, typename A>
struct Serializer<std::vector<T, A>, std::enable_if_t<DataStreamType<T>::id>> {
  static const bool enabled = true;
  using U = std::vector<T, A>;

  template<typename ODStreamT>
  static void dump(XBlock& block, ODStreamT& ods, const U& x) { 
    return impl_std::dump(block, ods, x); 
  }

  template<typename IDStreamT>
  static void load_to(const XBlock& block, IDStreamT& ids, U& y) {
    impl_std::load_to_resizable(block, ids, y);
  }

  template<typename IDStreamT>
  static U load(const XBlock& block, IDStreamT& ids) {
    U y; 
    impl_std::load_to_resizable(block, ids, y); 
    return y;
  }
};


// std::string
//////////////////////////////
template<>
struct Serializer<std::string, void> {
  static const bool enabled = true;
  using U = std::string;

  template<typename ODStream>
  static void dump(XBlock& block, ODStream& ods, const U& x) {
    return impl_std::dump(block, ods, x);
  }

  template<typename IDStream>
  static void load_to(const XBlock& block, IDStream& ids, U& y) {
    impl_std::load_to_resizable(block, ids, y);
  }

  template<typename IDStream>
  static U load(const XBlock& block, IDStream& ids) {
    U y; 
    impl_std::load_to_resizable(block, ids, y); 
    return y;
  }
};

// std::array
//////////////////////////////
template<typename T, size_t N>
struct Serializer<std::array<T, N>, std::enable_if_t<DataStreamType<T>::id>> {
  static const bool enabled = true;
  using U = std::array<T, N>;

  template<typename ODStreamT>
  static void dump(XBlock& block, ODStreamT& ods, const U& x) { 
    return impl_std::dump(block, ods, x); 
  }

  template<typename IDStreamT>
  static U& load_to(const XBlock& block, IDStreamT& ids, U& y) {
    if(block.t_ != DataStreamType<T>::id) {
      throw DeserializationError("std::container load(): wrong scalar type");
    }
    if(!check_shape_1d(block)) {
      throw DeserializationError("wrong shape for std::container");
    }
    if(block.numel() != y.size()) {
      throw DeserializationError("wrong size for std::array<T,N>");
    }
    Serializer<T*>::load_to(block, ids, y.data());
    return y;
  }

  template<typename IDStreamT>
  static U load(const XBlock& block, IDStreamT& ids) {
    U y;
    load_to(block, ids, y);
    return y;
  }
};


// xmat::NArrayInterface_
// ----------------------
template<typename Derived, typename T, size_t ND, MOrder MOrderT, typename IntT>
struct Serializer<NArrayInterface_<Derived, T, ND, MOrderT, IntT>, 
                  std::enable_if_t<DataStreamType<T>::id>>
  {
  static const bool enabled = true;
  using array_t = NArrayInterface_<Derived, T, ND, MOrderT, IntT>;
  
  template<typename ODStreamT>
  static void dump(XBlock& block, ODStreamT& ods, const array_t& x) {
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

  template<typename IDStreamT>
  static void load_to(const XBlock& block, IDStreamT& ids, array_t& y) {
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
    const bool iscontig1 = y.ravel().leaststride() == 1;
    if (iscontig1) {
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

// NArray_
template<typename T, size_t ND, class MemSourceT, MOrder MOrderT, typename IntT>
struct Serializer<NArray_<T, ND, MemSourceT, MOrderT, IntT>, 
                          std::enable_if_t<DataStreamType<T>::id>>
  {
  static const bool enabled = true;
  using array_t = NArray_<T, ND, MemSourceT, MOrderT, IntT>;
  using array_interface_t = typename array_t::base_t;
  
  template<typename ODStreamT>
  static void dump(XBlock& block, ODStreamT& ods, const array_t& x) {
    return Serializer<array_interface_t>::dump(block, ods, x);
  }

  template<typename IDStreamT>
  static void load_to(const XBlock& block, IDStreamT& ids, array_t& y) {
    Serializer<array_interface_t>::load_to(block, ids, y);
    return y;
  }

  template<typename IDStreamT>
  static array_t load_with_allocator(const XBlock& block, IDStreamT& ids, const MemSourceT& memsrc) {
    typename array_t::index_t shape;
    shape.fill(1);
    std::copy_n(block.shape_.cbegin() + (ND - block.ndim()), block.ndim(), shape.begin());

    array_t y{shape, MemSourceT{memsrc}};
    Serializer<array_interface_t>::load_to(block, ids, y);
    return y;
  }

  template<typename IDStreamT, 
           bool HDC = array_t::k_has_default_constructor, 
           std::enable_if_t<HDC, bool> = false>
  static array_t load(const XBlock& block, IDStreamT& ids) {
    return load_with_allocator(block, ids, MemSourceT{});
  }
};
} // namespace xmat
