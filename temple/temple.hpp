#ifndef VEILER_TEMPLE_HPP_INCLUDED
#define VEILER_TEMPLE_HPP_INCLUDED

#include<type_traits>
#include<array>
#include<utility>
#include<algorithm>
#include<veiler/refil.hpp>

#ifdef VEILER_TEMPLE_USE_STRICT_AND_POSITIVE_INSTANTIATION
#define VEILER_TEMPLE_STRICT_CHECK(...) typename std::enable_if<(__VA_ARGS__)>::type* = nullptr
//#define VEILER_TEMPLE_STRICT_CHECK(...) typename = typename std::enable_if<(__VA_ARGS__)>::type
#define VEILER_TEMPLE_POSITIVE_NOEXCEPT(...) noexcept(__VA_ARGS__)
#else
#define VEILER_TEMPLE_STRICT_CHECK(...) void* = nullptr
#define VEILER_TEMPLE_POSITIVE_NOEXCEPT(...)
#endif


namespace veiler{

namespace detail{

namespace temple{



template<typename...>struct type_tuple;


template<typename T>
struct type_wrapper{using type = T;};


template<typename,typename>struct make_type_tuple_impl_impl;
template<typename... Lefts, typename... Rights>
struct make_type_tuple_impl_impl<type_tuple<Lefts...>, type_tuple<Rights...>>{
  using type = type_tuple<Lefts..., Rights...>;
};

template<std::size_t N, typename T>
struct make_type_tuple_impl :
       make_type_tuple_impl_impl<
typename make_type_tuple_impl<    N/2, T>::type,
typename make_type_tuple_impl<N - N/2, T>::type
       >{};
template<typename T>
struct make_type_tuple_impl<1, T>{using type = type_tuple<T>;};
template<typename T>
struct make_type_tuple_impl<0, T>{using type = type_tuple< >;};

template<std::size_t N, typename T = void>
using make_type_tuple = typename make_type_tuple_impl<N, T>::type;


template<typename>struct type_at_impl_impl_impl;
template<typename... Types>
struct type_at_impl_impl_impl<type_tuple<Types...>>{template<typename T>static T eval(Types*..., T*, ...);};

template<std::size_t N, typename... Types>
struct type_at_impl_impl{
  using type = decltype(type_at_impl_impl_impl<make_type_tuple<N>>::eval(static_cast<Types*>(nullptr)...));
};

template<typename T, std::size_t N>
class type_at_impl{
  template<template<typename...>class Type, class... Args>
  static auto impl(type_wrapper<Type<Args...>>)->typename type_at_impl_impl<N, type_wrapper<Args>...>::type;
 public:
  using type = typename decltype(impl(type_wrapper<T>{}))::type;
};

template<typename T, std::size_t N>
using type_at = typename type_at_impl<typename std::remove_cv<typename std::remove_reference<T>::type>::type, N>::type;

template<typename>struct tuple_size_impl;
template<template<typename...>class Tuple, typename... Types>
struct tuple_size_impl<Tuple<Types...>>{
  static constexpr std::size_t value = sizeof...(Types);
  constexpr operator std::size_t()const{return value;}
  constexpr tuple_size_impl(){}
};

template<typename T>
using tuple_size = tuple_size_impl<typename std::remove_cv<typename std::remove_reference<T>::type>::type>;

struct duplicate_type;
template<long long A, long long B>struct add{static const long long value = A+B;static const long long default_value = 0;};
template<typename...>struct unique_types_impl;
template<typename SourceType, typename Type>struct filter{using type = typename std::conditional<std::is_same<SourceType, Type>::value, duplicate_type, Type>::type;};
template<typename T, typename... Remaining, typename... Uniques>
struct unique_types_impl<type_tuple<T, Remaining...>, Uniques...>{
  using type = typename unique_types_impl<type_tuple<typename filter<T, Remaining>::type...>, Uniques..., T>::type;
};
template<typename... Remaining, typename... Uniques>
struct unique_types_impl<type_tuple<duplicate_type, Remaining...>, Uniques...>:unique_types_impl<type_tuple<Remaining...>, Uniques...>{};
template<typename... Uniques>
struct unique_types_impl<type_tuple<>, Uniques...>{using type = type_tuple<Uniques...>;};

template<typename T>
using unique_types = typename unique_types_impl<T>::type;


template<typename... Types>
struct type_tuple{
  //template<std::size_t N>
  //using at = type_at<type_tuple<Types...>, N>;
  static constexpr std::size_t size(){return sizeof...(Types);}
};



template<long long...>struct index_tuple;


template<typename>struct index_at_impl;
template<typename... Types>
struct index_at_impl<type_tuple<Types...>>{template<typename T>static constexpr T eval(Types..., T t, ...){return t;}};

template<typename T, std::size_t N>
class index_at{
  template<template<long long...>class IndexTuple, long long... Indices>
  static constexpr long long impl(IndexTuple<Indices...> t)noexcept{return index_at_impl<make_type_tuple<N, long long>>::eval(Indices...);}
 public:
  static constexpr long long value = impl(T{});
  constexpr operator long long()const noexcept{return value;}
};

template<typename>struct index_size_impl;
template<template<long long...>class IndexTuple, long long... Indices>
struct index_size_impl<IndexTuple<Indices...>>{
  static constexpr std::size_t value = sizeof...(Indices);
  constexpr operator std::size_t()const{return value;}
  constexpr index_size_impl(){}
};

template<typename T>
using index_size = index_size_impl<typename std::remove_cv<typename std::remove_reference<T>::type>::type>;


template<typename,long long>struct make_index_range_next;
template<long long... Indices, long long Next>
struct make_index_range_next<index_tuple<Indices...>, Next>{using type = index_tuple<Indices..., (Indices+Next)...>;};

template<typename,long long,long long>struct make_index_range_next_;
template<long long... Indices, long long Next, long long Tail>
struct make_index_range_next_<index_tuple<Indices...>, Next, Tail>{using type = index_tuple<Indices..., (Indices+Next)..., Tail>;};

template<long long,long long,long long,typename = void>struct make_index_range_impl;
template<long long Begin, long long Step, long long Next>
struct make_index_range_impl<Begin, Step, Next, typename std::enable_if<(Next==0 || Next==1)>::type>{
  using type = typename std::conditional<Next==0, index_tuple<>, index_tuple<Begin>>::type;
};
template<long long Begin, long long Step, long long Next>
struct make_index_range_impl<Begin, Step, Next, typename std::enable_if<(Next>1)>::type>{
  using type = typename std::conditional<Next % 2,
                 typename make_index_range_next_<
                   typename make_index_range_impl<Begin, Step, Next/2>::type,
                            Begin +  Next/2  * Step,
                            Begin + (Next-1) * Step
                          >::type,
                 typename make_index_range_next<
                   typename make_index_range_impl<Begin, Step, Next/2>::type,
                            Begin +  Next/2  * Step
                          >::type
                        >::type;
};

template<long long Begin, long long End, long long Step = (Begin<End ? 1 : -1)>
using make_index_range = typename make_index_range_impl<Begin, Step, (End - Begin + (Step>0 ? Step-1 : Step+1)) / Step>::type;


template<typename,typename>struct reverse_index_tuple_impl_impl;
template<long long... Indices, long long... IndicesIndices>
struct reverse_index_tuple_impl_impl<index_tuple<Indices...>, index_tuple<IndicesIndices...>>{
  using type = index_tuple<index_at<index_tuple<Indices...>, sizeof...(IndicesIndices) - 1 - IndicesIndices>::value...>;
};

template<typename>struct reverse_index_tuple_impl;
template<long long... Indices>
struct reverse_index_tuple_impl<index_tuple<Indices...>> : 
       reverse_index_tuple_impl_impl<index_tuple<Indices...>, make_index_range<0,sizeof...(Indices)>>{};

template<typename T>
using reverse_index_tuple = typename reverse_index_tuple_impl<T>::type;


template<typename... Types>
using make_indexes = make_index_range<0, sizeof...(Types)>;

template<typename... Types>
using make_reverse_indexes = make_index_range<sizeof...(Types)-1, -1>;

template<long long... Indices>
struct index_tuple{
  static constexpr std::size_t size()noexcept{return sizeof...(Indices);}
};



template<typename...>class value_btree;
template<typename T, typename Left, typename Right>
class value_btree<T, Left, Right>{
  T t;
  Left left;
  Right right;
 public:
  static constexpr std::size_t size = 3;
  constexpr value_btree(T _, Left l, Right r):
    t(std::forward<T>(_)), left(std::forward<Left>(l)), right(std::forward<Right>(r)){}
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr T& get(value_btree& btree)noexcept{return btree.t;}
  template<std::size_t N, typename std::enable_if<N==1>::type* = nullptr>
  static constexpr Left& get(value_btree& btree)noexcept{return btree.left;}
  template<std::size_t N, typename std::enable_if<N==2>::type* = nullptr>
  static constexpr Right& get(value_btree& btree)noexcept{return btree.right;}
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr T&& get(value_btree&& btree)noexcept{return std::forward<T>(btree.t);}
  template<std::size_t N, typename std::enable_if<N==1>::type* = nullptr>
  static constexpr Left&& get(value_btree&& btree)noexcept{return std::forward<Left>(btree.left);}
  template<std::size_t N, typename std::enable_if<N==2>::type* = nullptr>
  static constexpr Right&& get(value_btree&& btree)noexcept{return std::forward<Right>(btree.right);}
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr const T& get(const value_btree& btree)noexcept{return btree.t;}
  template<std::size_t N, typename std::enable_if<N==1>::type* = nullptr>
  static constexpr const Left& get(const value_btree& btree)noexcept{return btree.left;}
  template<std::size_t N, typename std::enable_if<N==2>::type* = nullptr>
  static constexpr const Right& get(const value_btree& btree)noexcept{return btree.right;}
  void swap(value_btree& rhs)
    noexcept(noexcept(std::swap(std::declval<T&>(),     std::declval<T&>()))    &&
             noexcept(std::swap(std::declval<Left&>(),  std::declval<Left&>())) &&
             noexcept(std::swap(std::declval<Right&>(), std::declval<Right&>()))){
    using std::swap;
    swap(t, rhs.t);
    swap(left, rhs.left);
    swap(right, rhs.right);
  }
  template<typename F>
  static constexpr auto btree_apply(value_btree&& btree, const F& f)
    ->decltype((f(std::forward<value_btree>(btree)))){
         return f(std::forward<value_btree>(btree));
  }
  template<typename F>
  static constexpr auto btree_apply(const value_btree& btree, const F& f)
    ->decltype((f(btree))){
         return f(btree);
  }
  template<typename U, typename L, typename R>
  friend constexpr bool operator==(const value_btree<T, Left, Right>& lhs, const value_btree<U, L, R>& rhs)
    noexcept(noexcept(bool(lhs.t == rhs.t) && bool(lhs.left == rhs.left) && bool(lhs.right == rhs.right))){
               return bool(lhs.t == rhs.t) && bool(lhs.left == rhs.left) && bool(lhs.right == rhs.right);
  }
  template<typename U, typename L, typename R>
  friend constexpr bool operator!=(const value_btree<T, Left, Right>& lhs, const value_btree<U, L, R>& rhs)
    noexcept(noexcept(!(lhs == rhs))){
               return !(lhs == rhs);
  }
  template<typename U, typename L, typename R>
  friend constexpr bool operator< (const value_btree<T, Left, Right>& lhs, const value_btree<U, L, R>& rhs)
    noexcept(noexcept(bool(lhs.t     < rhs.t)    || (!bool(rhs.t    < lhs.t)    && (
                      bool(lhs.left  < rhs.left) || (!bool(rhs.left < lhs.left) &&
                      bool(lhs.right < rhs.right)))))){
               return bool(lhs.t     < rhs.t)    || (!bool(rhs.t    < lhs.t)   && (
                      bool(lhs.left  < rhs.left) || (!bool(rhs.left < lhs.left)&&
                      bool(lhs.right <  rhs.right))));
  }
  template<typename U, typename L, typename R>
  friend constexpr bool operator> (const value_btree<T, Left, Right>& lhs, const value_btree<U, L, R>& rhs)
    noexcept(noexcept(rhs < lhs)){
               return rhs < lhs;
  }
  template<typename U, typename L, typename R>
  friend constexpr bool operator<=(const value_btree<T, Left, Right>& lhs, const value_btree<U, L, R>& rhs)
    noexcept(noexcept(!(rhs < lhs))){
               return !(rhs < lhs);
  }
  template<typename U, typename L, typename R>
  friend constexpr bool operator>=(const value_btree<T, Left, Right>& lhs, const value_btree<U, L, R>& rhs)
    noexcept(noexcept(!(lhs < rhs))){
               return !(lhs < rhs);
  }
};
template<typename T, typename Left>
class value_btree<T, Left>{
  T t;
  Left left;
 public:
  static constexpr std::size_t size = 2;
  constexpr value_btree(T _, Left l):
    t(std::forward<T>(_)), left(std::forward<Left>(l)){}
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr T& get(value_btree& btree)noexcept{return btree.t;}
  template<std::size_t N, typename std::enable_if<N==1>::type* = nullptr>
  static constexpr Left& get(value_btree& btree)noexcept{return btree.left;}
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr T&& get(value_btree&& btree)noexcept{return std::forward<T>(btree.t);}
  template<std::size_t N, typename std::enable_if<N==1>::type* = nullptr>
  static constexpr Left&& get(value_btree&& btree)noexcept{return std::forward<Left>(btree.left);}
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr const T& get(const value_btree& btree)noexcept{return btree.t;}
  template<std::size_t N, typename std::enable_if<N==1>::type* = nullptr>
  static constexpr const Left& get(const value_btree& btree)noexcept{return btree.left;}
  void swap(value_btree& rhs)
    noexcept(noexcept(std::swap(std::declval<T&>(),    std::declval<T&>())) &&
             noexcept(std::swap(std::declval<Left&>(), std::declval<Left&>()))){
    using std::swap;
    swap(t, rhs.t);
    swap(left, rhs.left);
  }
  template<typename F>
  static constexpr auto btree_apply(value_btree&& btree, const F& f)
    ->decltype((f(std::forward<value_btree>(btree)))){
         return f(std::forward<value_btree>(btree));
  }
  template<typename F>
  static constexpr auto btree_apply(const value_btree& btree, const F& f)
    ->decltype((f(btree))){
         return f(btree);
  }
  template<typename U, typename L>
  friend constexpr bool operator==(const value_btree<T, Left>& lhs, const value_btree<U, L>& rhs)
    noexcept(noexcept(bool(lhs.t == rhs.t) && bool(lhs.left == rhs.left))){
               return bool(lhs.t == rhs.t) && bool(lhs.left == rhs.left);
  }
  template<typename U, typename L>
  friend constexpr bool operator!=(const value_btree<T, Left>& lhs, const value_btree<U, L>& rhs)
    noexcept(noexcept(!(lhs == rhs))){
               return !(lhs == rhs);
  }
  template<typename U, typename L>
  friend constexpr bool operator< (const value_btree<T, Left>& lhs, const value_btree<U, L>& rhs)
    noexcept(noexcept(bool(lhs.t     < rhs.t)    || (!bool(rhs.t    < lhs.t) &&
                      bool(lhs.left  < rhs.left)))){
               return bool(lhs.t     < rhs.t)    || (!bool(rhs.t    < lhs.t) &&
                      bool(lhs.left  < rhs.left));
  }
  template<typename U, typename L>
  friend constexpr bool operator> (const value_btree<T, Left>& lhs, const value_btree<U, L>& rhs)
    noexcept(noexcept(rhs < lhs)){
               return rhs < lhs;
  }
  template<typename U, typename L>
  friend constexpr bool operator<=(const value_btree<T, Left>& lhs, const value_btree<U, L>& rhs)
    noexcept(noexcept(!(rhs < lhs))){
               return !(rhs < lhs);
  }
  template<typename U, typename L>
  friend constexpr bool operator>=(const value_btree<T, Left>& lhs, const value_btree<U, L>& rhs)
    noexcept(noexcept(!(lhs < rhs))){
               return !(lhs < rhs);
  }
};
template<typename T>
class value_btree<T>{
  T t;
 public:
  static constexpr std::size_t size = 1;
  constexpr value_btree(T _):
    t(std::forward<T>(_)){}
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr T& get(value_btree& btree)noexcept{return btree.t;}
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr T&& get(value_btree&& btree)noexcept{return std::forward<T>(btree.t);}
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr const T& get(const value_btree& btree)noexcept{return btree.t;}
  void swap(value_btree& rhs)
    noexcept(noexcept(std::swap(std::declval<T&>(),std::declval<T&>()))){
    using std::swap;
    swap(t, rhs.t);
  }
  template<typename F>
  static constexpr auto btree_apply(value_btree&& btree, const F& f)
    ->decltype((f(std::forward<value_btree>(btree)))){
         return f(std::forward<value_btree>(btree));
  }
  template<typename F>
  static constexpr auto btree_apply(const value_btree& btree, const F& f)
    ->decltype((f(btree))){
         return f(btree);
  }
  template<typename U>
  friend constexpr bool operator==(const value_btree<T>& lhs, const value_btree<U>& rhs)
    noexcept(noexcept(bool(lhs.t == rhs.t))){
               return bool(lhs.t == rhs.t);
  }
  template<typename U>
  friend constexpr bool operator!=(const value_btree<T>& lhs, const value_btree<U>& rhs)
    noexcept(noexcept(!(lhs == rhs))){
               return !(lhs == rhs);
  }
  template<typename U>
  friend constexpr bool operator< (const value_btree<T>& lhs, const value_btree<U>& rhs)
    noexcept(noexcept(bool(lhs.t < rhs.t))){
               return bool(lhs.t < rhs.t);
  }
  template<typename U>
  friend constexpr bool operator> (const value_btree<T>& lhs, const value_btree<U>& rhs)
    noexcept(noexcept(rhs < lhs)){
               return rhs < lhs;
  }
  template<typename U>
  friend constexpr bool operator<=(const value_btree<T>& lhs, const value_btree<U>& rhs)
    noexcept(noexcept(!(rhs < lhs))){
               return !(rhs < lhs);
  }
  template<typename U>
  friend constexpr bool operator>=(const value_btree<T>& lhs, const value_btree<U>& rhs)
    noexcept(noexcept(!(lhs < rhs))){
               return !(lhs < rhs);
  }
};
template<typename... LArgs, typename... RArgs, typename T, typename... Lefts, typename... Rights>
class value_btree<type_tuple<LArgs...>, type_tuple<RArgs...>, T, value_btree<Lefts...>, value_btree<Rights...>>{
  T t;
  value_btree<Lefts...> left;
  value_btree<Rights...> right;
 public:
  static constexpr std::size_t size = 1 + value_btree<Lefts...>::size + value_btree<Rights...>::size;
  constexpr value_btree(T _, LArgs... largs, RArgs... rargs):
    t(std::forward<T>(_)), left(std::forward<LArgs>(largs)...), right(std::forward<RArgs>(rargs)...){}
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr T& get(value_btree& btree)noexcept{return btree.t;}
  template<std::size_t N, typename std::enable_if<N-1 < (size-1)/2>::type* = nullptr>
  static constexpr auto get(value_btree& btree)noexcept
    ->decltype((value_btree<Lefts...>::template get<N-1>(btree.left))){
         return value_btree<Lefts...>::template get<N-1>(btree.left);
  }
  template<std::size_t N, typename std::enable_if<(size-1)/2 <= N-1>::type* = nullptr>
  static constexpr auto get(value_btree& btree)noexcept
    ->decltype((value_btree<Rights...>::template get<N-1 - (size-1)/2>(btree.right))){
         return value_btree<Rights...>::template get<N-1 - (size-1)/2>(btree.right);
  }
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr T&& get(value_btree&& btree)noexcept{return std::forward<T>(btree.t);}
  template<std::size_t N, typename std::enable_if<N-1 < (size-1)/2>::type* = nullptr>
  static constexpr auto get(value_btree&& btree)noexcept
    ->decltype((value_btree<Lefts...>::template get<N-1>(std::forward<value_btree<Lefts...>>(btree.left)))){
         return value_btree<Lefts...>::template get<N-1>(std::forward<value_btree<Lefts...>>(btree.left));
  }
  template<std::size_t N, typename std::enable_if<(size-1)/2 <= N-1>::type* = nullptr>
  static constexpr auto get(value_btree&& btree)noexcept
    ->decltype((value_btree<Rights...>::template get<N-1 - (size-1)/2>(std::forward<value_btree<Rights...>>(btree.right)))){
         return value_btree<Rights...>::template get<N-1 - (size-1)/2>(std::forward<value_btree<Rights...>>(btree.right));
  }
  template<std::size_t N, typename std::enable_if<N==0>::type* = nullptr>
  static constexpr const T& get(const value_btree& btree)noexcept{return btree.t;}
  template<std::size_t N, typename std::enable_if<N-1 < (size-1)/2>::type* = nullptr>
  static constexpr auto get(const value_btree& btree)noexcept
    ->decltype((value_btree<Lefts...>::template get<N-1>(btree.left))){
         return value_btree<Lefts...>::template get<N-1>(btree.left);
  }
  template<std::size_t N, typename std::enable_if<(size-1)/2 <= N-1>::type* = nullptr>
  static constexpr auto get(const value_btree& btree)noexcept
    ->decltype((value_btree<Rights...>::template get<N-1 - (size-1)/2>(btree.right))){
         return value_btree<Rights...>::template get<N-1 - (size-1)/2>(btree.right);
  }
  static constexpr value_btree<Lefts...>& get_left(value_btree& btree)noexcept{return btree.left;}
  static constexpr value_btree<Lefts...>&& get_left(value_btree&& btree)noexcept{return std::forward<value_btree<Lefts...>>(btree.left);}
  static constexpr const value_btree<Lefts...>& get_left(const value_btree& btree)noexcept{return btree.left;}
  static constexpr value_btree<Rights...>& get_right(value_btree& btree)noexcept{return btree.right;}
  static constexpr value_btree<Rights...>&& get_right(value_btree&& btree)noexcept{return std::forward<value_btree<Rights...>>(btree.right);}
  static constexpr const value_btree<Rights...>& get_right(const value_btree& btree)noexcept{return btree.right;}
 
  void swap(value_btree& rhs)
    noexcept(noexcept(std::swap(std::declval<T&>(), std::declval<T&>()))                                    &&
             noexcept(std::declval<value_btree<Lefts...>&>() .swap(std::declval<value_btree<Lefts...>&>())) &&
             noexcept(std::declval<value_btree<Rights...>&>().swap(std::declval<value_btree<Rights...>&>()))){
    using std::swap;
    swap(t, rhs.t);
    left.swap(rhs.left);
    right.swap(rhs.right);
  }
  template<typename F>
  static constexpr auto btree_apply(value_btree&& btree, const F& f)
    noexcept(noexcept(f(value_btree<T,
                                    decltype(value_btree<Lefts... >::btree_apply(std::forward<value_btree<Lefts... >>(btree.left),  f)),
                                    decltype(value_btree<Rights...>::btree_apply(std::forward<value_btree<Rights...>>(btree.right), f))>(
                                      std::forward<T>(btree.t),
                                      value_btree<Lefts... >::btree_apply(std::forward<value_btree<Lefts... >>(btree.left),  f),
                                      value_btree<Rights...>::btree_apply(std::forward<value_btree<Rights...>>(btree.right), f)))))
           ->decltype(f(value_btree<T,
                                    decltype(value_btree<Lefts... >::btree_apply(std::forward<value_btree<Lefts... >>(btree.left),  f)),
                                    decltype(value_btree<Rights...>::btree_apply(std::forward<value_btree<Rights...>>(btree.right), f))>(
                                      std::forward<T>(btree.t),
                                      value_btree<Lefts... >::btree_apply(std::forward<value_btree<Lefts... >>(btree.left),  f),
                                      value_btree<Rights...>::btree_apply(std::forward<value_btree<Rights...>>(btree.right), f)))){
               return f(value_btree<T,
                                    decltype(value_btree<Lefts... >::btree_apply(std::forward<value_btree<Lefts... >>(btree.left),  f)),
                                    decltype(value_btree<Rights...>::btree_apply(std::forward<value_btree<Rights...>>(btree.right), f))>(
                                      std::forward<T>(btree.t),
                                      value_btree<Lefts... >::btree_apply(std::forward<value_btree<Lefts... >>(btree.left),  f),
                                      value_btree<Rights...>::btree_apply(std::forward<value_btree<Rights...>>(btree.right), f)));
  }
  template<typename F>
  static constexpr auto btree_apply(const value_btree& btree, const F& f)
    noexcept(noexcept(f(value_btree<T,
                                    decltype(value_btree<Lefts... >::btree_apply(btree.left,  f)),
                                    decltype(value_btree<Rights...>::btree_apply(btree.right, f))>(
                                      btree.t,
                                      value_btree<Lefts... >::btree_apply(btree.left,  f),
                                      value_btree<Rights...>::btree_apply(btree.right, f)))))
           ->decltype(f(value_btree<T,
                                    decltype(value_btree<Lefts... >::btree_apply(btree.left,  f)),
                                    decltype(value_btree<Rights...>::btree_apply(btree.right, f))>(
                                      btree.t,
                                      value_btree<Lefts... >::btree_apply(btree.left,  f),
                                      value_btree<Rights...>::btree_apply(btree.right, f)))){
               return f(value_btree<T,
                                    decltype(value_btree<Lefts... >::btree_apply(btree.left,  f)),
                                    decltype(value_btree<Rights...>::btree_apply(btree.right, f))>(
                                      btree.t,
                                      value_btree<Lefts... >::btree_apply(btree.left,  f),
                                      value_btree<Rights...>::btree_apply(btree.right, f)));
  }
  template<typename... As, typename... Bs, typename U, typename... Ls, typename... Rs>
  friend constexpr bool operator==(const value_btree<type_tuple<LArgs...>, type_tuple<RArgs...>, T, value_btree<Lefts...>, value_btree<Rights...>>& lhs,
                                   const value_btree<type_tuple<As...>,    type_tuple<Bs...>,    U, value_btree<Ls...>,    value_btree<Rs...>>&     rhs)
    noexcept(noexcept(bool(lhs.t == rhs.t) && lhs.left == rhs.left && lhs.right == rhs.right)){
               return bool(lhs.t == rhs.t) && lhs.left == rhs.left && lhs.right == rhs.right;
  }
  template<typename... As, typename... Bs, typename U, typename... Ls, typename... Rs>
  friend constexpr bool operator!=(const value_btree<type_tuple<LArgs...>, type_tuple<RArgs...>, T, value_btree<Lefts...>, value_btree<Rights...>>& lhs,
                                   const value_btree<type_tuple<As...>,    type_tuple<Bs...>,    U, value_btree<Ls...>,    value_btree<Rs...>>&     rhs)
    noexcept(noexcept(!(lhs == rhs))){
               return !(lhs == rhs);
  }
  template<typename... As, typename... Bs, typename U, typename... Ls, typename... Rs>
  friend constexpr bool operator< (const value_btree<type_tuple<LArgs...>, type_tuple<RArgs...>, T, value_btree<Lefts...>, value_btree<Rights...>>& lhs,
                                   const value_btree<type_tuple<As...>,    type_tuple<Bs...>,    U, value_btree<Ls...>,    value_btree<Rs...>>&     rhs)
    noexcept(noexcept(bool(lhs.t     < rhs.t)    || (!bool(rhs.t    < lhs.t)    && (
                      bool(lhs.left  < rhs.left) || (!bool(rhs.left < lhs.left) &&
                      bool(lhs.right < rhs.right)))))){
               return bool(lhs.t     < rhs.t)    || (!bool(rhs.t    < lhs.t)    && (
                      bool(lhs.left  < rhs.left) || (!bool(rhs.left < lhs.left) &&
                      bool(lhs.right <  rhs.right))));
  }
  template<typename... As, typename... Bs, typename U, typename... Ls, typename... Rs>
  friend constexpr bool operator> (const value_btree<type_tuple<LArgs...>, type_tuple<RArgs...>, T, value_btree<Lefts...>, value_btree<Rights...>>& lhs,
                                   const value_btree<type_tuple<As...>,    type_tuple<Bs...>,    U, value_btree<Ls...>,    value_btree<Rs...>>&     rhs)
    noexcept(noexcept(rhs < lhs)){
               return rhs < lhs;
  }
  template<typename... As, typename... Bs, typename U, typename... Ls, typename... Rs>
  friend constexpr bool operator<=(const value_btree<type_tuple<LArgs...>, type_tuple<RArgs...>, T, value_btree<Lefts...>, value_btree<Rights...>>& lhs,
                                   const value_btree<type_tuple<As...>,    type_tuple<Bs...>,    U, value_btree<Ls...>,    value_btree<Rs...>>&     rhs)
    noexcept(noexcept(!(rhs < lhs))){
               return !(rhs < lhs);
  }
  template<typename... As, typename... Bs, typename U, typename... Ls, typename... Rs>
  friend constexpr bool operator>=(const value_btree<type_tuple<LArgs...>, type_tuple<RArgs...>, T, value_btree<Lefts...>, value_btree<Rights...>>& lhs,
                                   const value_btree<type_tuple<As...>,    type_tuple<Bs...>,    U, value_btree<Ls...>,    value_btree<Rs...>>&     rhs)
    noexcept(noexcept(!(lhs < rhs))){
               return !(lhs < rhs);
  }
};
template<>
class value_btree<>{
 public:
  static constexpr std::size_t size = 0;
  constexpr value_btree() = default;
  void swap(value_btree<>&)const noexcept{}
  friend constexpr bool operator==(const value_btree<>&, const value_btree<>&)noexcept{return true;}
  friend constexpr bool operator!=(const value_btree<>&, const value_btree<>&)noexcept{return false;}
  friend constexpr bool operator< (const value_btree<>&, const value_btree<>&)noexcept{return false;}
  friend constexpr bool operator> (const value_btree<>&, const value_btree<>&)noexcept{return false;}
  friend constexpr bool operator<=(const value_btree<>&, const value_btree<>&)noexcept{return true;}
  friend constexpr bool operator>=(const value_btree<>&, const value_btree<>&)noexcept{return true;}
};


template<std::size_t,typename>struct make_value_btree;
template<std::size_t N, typename T, typename... Args>
struct make_value_btree<N, type_tuple<T, Args...>>{
 private:
  using Left  = make_value_btree<      (N-1)/2, type_tuple<Args...>>;
  using Right = make_value_btree<(N-1)-(N-1)/2, typename Left::remaining>;
  template<typename U, typename... LArgs, typename... RArgs>
  static auto impl(type_tuple<U>, type_tuple<LArgs...>, type_tuple<RArgs...>)->type_tuple<U, LArgs..., RArgs...>;
 public:
  using remaining = typename Right::remaining;
  using args = decltype(impl(std::declval<type_tuple<T>>(), std::declval<typename Left::args>(), std::declval<typename Right::args>()));
  using type = value_btree<typename Left::args, typename Right::args, T, typename Left::type, typename Right::type>;
};
template<typename T, typename U, typename V, typename... Args>
struct make_value_btree<3, type_tuple<T, U, V, Args...>>{
  using args = type_tuple<T, U, V>;
  using type = value_btree<T, U, V>;
  using remaining = type_tuple<Args...>;
};
template<typename T, typename U, typename... Args>
struct make_value_btree<2, type_tuple<T, U, Args...>>{
  using args = type_tuple<T, U>;
  using type = value_btree<T, U>;
  using remaining = type_tuple<Args...>;
};
template<typename T, typename... Args>
struct make_value_btree<1, type_tuple<T, Args...>>{
  using args = type_tuple<T>;
  using type = value_btree<T>;
  using remaining = type_tuple<Args...>;
};
template<>
struct make_value_btree<0,type_tuple<>>{
  using type = value_btree<>;
};



namespace{

#define VEILER_TEMPLE_DECL_OPERATE(name, ope, ...) \
struct name ## t{\
  constexpr name ## t(){}\
  template<typename T, typename Left, typename Right>\
  constexpr auto operator()(const value_btree<T, Left, Right>& arg)const\
    noexcept(noexcept(value_btree<T, Left, Right>::template get<0>(arg) ope\
                      value_btree<T, Left, Right>::template get<1>(arg) ope\
                      value_btree<T, Left, Right>::template get<2>(arg)))\
           ->decltype(value_btree<T, Left, Right>::template get<0>(arg) ope\
                      value_btree<T, Left, Right>::template get<1>(arg) ope\
                      value_btree<T, Left, Right>::template get<2>(arg)){\
               return value_btree<T, Left, Right>::template get<0>(arg) ope\
                      value_btree<T, Left, Right>::template get<1>(arg) ope\
                      value_btree<T, Left, Right>::template get<2>(arg);\
  }\
  template<typename T, typename Left>\
  constexpr auto operator()(const value_btree<T, Left>& arg)const\
    noexcept(noexcept(value_btree<T, Left>::template get<0>(arg) ope\
                      value_btree<T, Left>::template get<1>(arg)))\
           ->decltype(value_btree<T, Left>::template get<0>(arg) ope\
                      value_btree<T, Left>::template get<1>(arg)){\
               return value_btree<T, Left>::template get<0>(arg) ope\
                      value_btree<T, Left>::template get<1>(arg);\
  }\
  template<typename T>\
  constexpr auto operator()(const value_btree<T>& arg)const\
    noexcept(noexcept(value_btree<T>::template get<0>(arg)))\
           ->decltype(value_btree<T>::template get<0>(arg)){\
               return value_btree<T>::template get<0>(arg);\
  }\
  __VA_ARGS__\
  template<typename Dummy1, typename Dummy2, typename T, typename... Lefts, typename... Rights>\
  constexpr auto operator()(const value_btree<Dummy1, Dummy2, T, value_btree<Lefts...>, value_btree<Rights...>>& arg)const\
    noexcept(noexcept(value_btree<Dummy1, Dummy2, T, value_btree<Lefts...>, value_btree<Rights...>>::btree_apply(arg, std::declval<name ## t>())))\
           ->decltype(value_btree<Dummy1, Dummy2, T, value_btree<Lefts...>, value_btree<Rights...>>::btree_apply(arg, *this)){\
               return value_btree<Dummy1, Dummy2, T, value_btree<Lefts...>, value_btree<Rights...>>::btree_apply(arg, *this);\
  }\
}constexpr name
VEILER_TEMPLE_DECL_OPERATE(add_, + , constexpr std::nullptr_t operator()(const value_btree<>& arg)const noexcept;);
VEILER_TEMPLE_DECL_OPERATE(mul_, * , constexpr std::nullptr_t operator()(const value_btree<>& arg)const noexcept;);
VEILER_TEMPLE_DECL_OPERATE(and_, &&, constexpr bool operator()(const value_btree<>& arg)const noexcept{return false;});
VEILER_TEMPLE_DECL_OPERATE(or_ , ||, constexpr bool operator()(const value_btree<>& arg)const noexcept{return false;});
#undef VEILER_TEMPLE_DECL_OPERATE

}


template<typename T, std::size_t N>
class operate_tuple{
  using impl_type = typename make_value_btree<N, make_type_tuple<N, T>>::type;
  impl_type impl;
 public:
  template<typename... Args>
  constexpr operate_tuple(Args&&... args)
    noexcept(std::is_nothrow_constructible<impl_type, decltype(std::forward<Args>(args))...>::value) :
    impl(std::forward<Args>(args)...){}
  constexpr const impl_type& get_btree()const noexcept{return impl;}
  constexpr auto add_()const noexcept(noexcept(temple::add_(impl)))->decltype(temple::add_(impl)){return temple::add_(impl);}
  constexpr auto mul_()const noexcept(noexcept(temple::mul_(impl)))->decltype(temple::mul_(impl)){return temple::mul_(impl);}
  constexpr auto and_()const noexcept(noexcept(temple::and_(impl)))->decltype(temple::and_(impl)){return temple::and_(impl);}
  constexpr auto or_ ()const noexcept(noexcept(temple::or_ (impl)))->decltype(temple::or_ (impl)){return temple::or_ (impl);}
};


template<typename T, typename... Args>
constexpr operate_tuple<T, sizeof...(Args)> make_operate_tuple(Args&&... args)
  noexcept(std::is_nothrow_constructible<operate_tuple<T, sizeof...(Args)>, decltype(std::forward<Args>(args))...>::value){
  return operate_tuple<T, sizeof...(Args)>(std::forward<Args>(args)...);
}



template<typename... Types>
class tuple{
  using impl_type = typename make_value_btree<sizeof...(Types), type_tuple<Types...>>::type;
  impl_type impl;
  template<typename... UTypes, long long... Indices>
  constexpr tuple(const tuple<UTypes...>& u, index_tuple<Indices...>)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_constructible<Types, const UTypes&>::value...).and_()) :
    impl(tuple<UTypes...>::template get<Indices>(u)...){}
  template<typename... UTypes, long long... Indices>
  constexpr tuple(tuple<UTypes...>&& u, index_tuple<Indices...>)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_constructible<Types, UTypes&&>::value...).and_()) :
    impl(tuple<UTypes...>::template get<Indices>(std::forward<tuple<UTypes...>>(u))...){}
  template<typename... Args>
  static void expand_variadic_templates(Args&&...){}
  template<long long... Indices>
  tuple& move_assign(tuple&& src, index_tuple<Indices...>)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_move_assignable<Types>::value...).and_()){
    expand_variadic_templates((
          get<Indices>(*this) = get<Indices>(std::forward<tuple>(src))
    ,nullptr)...);
    return *this;
  }
 public:
  static constexpr std::size_t size()noexcept{return sizeof...(Types);}
  template<VEILER_TEMPLE_STRICT_CHECK(make_operate_tuple<bool>(std::is_default_constructible<Types>::value...).and_())>
  constexpr tuple()
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_default_constructible<Types>::value...).and_()) :
    impl{}{}
  template<VEILER_TEMPLE_STRICT_CHECK(make_operate_tuple<bool>(std::is_copy_constructible<Types>::value...).and_())>
  explicit constexpr tuple(const Types&... args)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_copy_constructible<Types>::value...).and_()) :
    impl(args...){}
  template<typename... UTypes,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(UTypes) &&
                     make_operate_tuple<bool>(std::is_constructible<Types, UTypes&&>::value...).and_())>
  explicit constexpr tuple(UTypes&&... args)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_constructible<Types, UTypes&&>::value...).and_()) :
    impl(std::forward<UTypes>(args)...){}
  tuple(const tuple& t) = default;
  tuple(tuple&& t) = default;
  template<typename... UTypes,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(UTypes) &&
                     make_operate_tuple<bool>(std::is_constructible<Types, const UTypes&>::value...).and_())>
  constexpr tuple(const tuple<UTypes...>& u)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_constructible<Types, const UTypes&>::value...).and_()) :
    tuple(u, make_indexes<UTypes...>{}){}
  template<typename... UTypes,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(UTypes) && make_operate_tuple<bool>(std::is_constructible<Types, UTypes&&>::value...).and_())>
  constexpr tuple(tuple<UTypes...>&& u)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_constructible<Types, UTypes&&>::value...).and_()) :
    tuple(std::forward<tuple<UTypes...>>(u), make_indexes<UTypes...>{}){}
  template<typename U1, typename U2,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == 2                                      &&
                             std::is_constructible<type_at<tuple, 0>, const U1&>::value &&
                             std::is_constructible<type_at<tuple, 1>, const U2&>::value)>
  constexpr tuple(const std::pair<U1, U2>& u)
    noexcept(std::is_nothrow_constructible<type_at<tuple, 0>, const U1&>::value         &&
             std::is_nothrow_constructible<type_at<tuple, 1>, const U2&>::value) :
    impl(u.first, u.second){}
  template<typename U1, typename U2,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == 2                                 &&
                             std::is_constructible<type_at<tuple, 0>, U1&&>::value &&
                             std::is_constructible<type_at<tuple, 1>, U2&&>::value)>
  constexpr tuple(std::pair<U1, U2>&& u)
    noexcept(std::is_nothrow_constructible<type_at<tuple, 0>, U1&&>::value         &&
             std::is_nothrow_constructible<type_at<tuple, 1>, U2&&>::value) :
    impl(std::forward<U1>(u.first), std::forward<U2>(u.second)){}
  template<VEILER_TEMPLE_STRICT_CHECK(make_operate_tuple<bool>(std::is_copy_assignable<Types>::value...).and_())>
  tuple& operator=(const tuple& src)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_copy_assignable<Types>::value...).and_()){
      this->impl = src.impl;
      return *this;
  }
  template<VEILER_TEMPLE_STRICT_CHECK(make_operate_tuple<bool>(std::is_move_assignable<Types>::value...).and_())>
  tuple& operator=(tuple&& src)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_move_assignable<Types>::value...).and_()){
      this->impl = std::forward<impl_type>(src.impl);
      return *this;
  }
  template<typename... UTypes,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(UTypes) &&
                     make_operate_tuple<bool>(std::is_assignable<Types&, const UTypes&>::value...).and_())>
  tuple& operator=(const tuple<UTypes...>& src)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_constructible<Types, const UTypes&>::value...).and_()){
      this->impl = std::forward<impl_type>(tuple(src).impl);
      return *this;
  }
  template<typename... UTypes,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(UTypes) &&
                     make_operate_tuple<bool>(std::is_assignable<Types&,UTypes&&>::value...).and_())>
  tuple& operator=(tuple<UTypes...>&& src)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_constructible<Types, UTypes&&>::value...).and_()){
      this->impl = std::forward<impl_type>(tuple(std::forward<tuple<UTypes...>>(src)).impl);
      return *this;
  }
  template<typename U1, typename U2,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == 2                                                   &&
                             std::is_assignable<type_at<tuple, 0>&, const U1&>::value                &&
                             std::is_assignable<type_at<tuple, 1>&, const U2&>::value)>
  tuple& operator=(const std::pair<U1,U2>& src)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_nothrow_assignable<type_at<tuple, 0>&, const U1&>::value &&
                                    std::is_nothrow_assignable<type_at<tuple, 1>&, const U2&>::value){
      get<0>(*this) = src.first;
      get<1>(*this) = src.second;
      return *this;
  }
  template<typename U1, typename U2,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == 2                                              &&
                             std::is_assignable<type_at<tuple, 0>&, U1&&>::value                &&
                             std::is_assignable<type_at<tuple, 1>&, U2&&>::value)>
  tuple& operator=(std::pair<U1,U2>&& src)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_nothrow_assignable<type_at<tuple, 0>&, U1&&>::value &&
                                    std::is_nothrow_assignable<type_at<tuple, 1>&, U2&&>::value){
    get<0>(*this) = std::forward<U1>(src.first);
    get<1>(*this) = std::forward<U2>(src.second);
    return *this;
  }
  template<std::size_t N>
  using at = type_at<tuple, N>;
  void swap(tuple& rhs)
    noexcept(noexcept(std::declval<impl_type&>().swap(std::declval<impl_type&>()))){
    return impl.swap(rhs.impl);
  }
  template<std::size_t N>
  static constexpr auto get(tuple& tpl)noexcept
    ->decltype((impl_type::template get<N>(tpl.impl))){
         return impl_type::template get<N>(tpl.impl);
  }
  template<std::size_t N>
  static constexpr auto get(const tuple& tpl)noexcept
    ->decltype((impl_type::template get<N>(tpl.impl))){
         return impl_type::template get<N>(tpl.impl);
  }
  template<std::size_t N>
  static constexpr auto get(tuple&& tpl)noexcept
    ->decltype((impl_type::template get<N>(std::forward<impl_type>(tpl.impl)))){
         return impl_type::template get<N>(std::forward<impl_type>(tpl.impl));
  }
  template<typename... Rights,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(Rights) &&
                     std::is_convertible<
                       decltype(impl == std::declval<typename tuple<Rights...>::impl_type>()),
                       bool
                     >::value)>
  friend constexpr bool operator==(const tuple& lhs, const tuple<Rights...>& rhs)
    noexcept(noexcept(lhs.impl == rhs.impl)){
               return lhs.impl == rhs.impl;
  }
  template<typename... Rights,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(Rights) &&
                     std::is_convertible<
                       decltype(impl != std::declval<typename tuple<Rights...>::impl_type>()),
                       bool
                     >::value)>
  friend constexpr bool operator!=(const tuple& lhs, const tuple<Rights...>& rhs)
    noexcept(noexcept(lhs.impl != rhs.impl)){
               return lhs.impl != rhs.impl;
  }
  template<typename... Rights,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(Rights) &&
                     std::is_convertible<
                       decltype(impl <  std::declval<typename tuple<Rights...>::impl_type>()),
                       bool
                     >::value)>
  friend constexpr bool operator< (const tuple& lhs, const tuple<Rights...>& rhs)
    noexcept(noexcept(lhs.impl <  rhs.impl)){
               return lhs.impl <  rhs.impl;
  }
  template<typename... Rights,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(Rights) &&
                     std::is_convertible<
                       decltype(impl >  std::declval<typename tuple<Rights...>::impl_type>()),
                       bool
                     >::value)>
  friend constexpr bool operator> (const tuple& lhs, const tuple<Rights...>& rhs)
    noexcept(noexcept(lhs.impl >  rhs.impl)){
               return lhs.impl >  rhs.impl;
  }
  template<typename... Rights,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(Rights) &&
                     std::is_convertible<
                       decltype(impl <=  std::declval<typename tuple<Rights...>::impl_type>()),
                       bool
                     >::value)>
  friend constexpr bool operator<=(const tuple& lhs, const tuple<Rights...>& rhs)
    noexcept(noexcept(lhs.impl <= rhs.impl)){
               return lhs.impl <= rhs.impl;
  }
  template<typename... Rights,
  VEILER_TEMPLE_STRICT_CHECK(sizeof...(Types) == sizeof...(Rights) &&
                     std::is_convertible<
                       decltype(impl >=  std::declval<typename tuple<Rights...>::impl_type>()),
                       bool
                     >::value)>
  friend constexpr bool operator>=(const tuple& lhs, const tuple<Rights...>& rhs)
    noexcept(noexcept(lhs.impl >= rhs.impl)){
               return lhs.impl >= rhs.impl;
  }
};
template<typename Type>
class tuple<Type>{
  using impl_type = value_btree<Type>;
  impl_type impl;
 public:
  static constexpr std::size_t size()noexcept{return 1;}
  template<VEILER_TEMPLE_STRICT_CHECK(std::is_default_constructible<Type>::value)>
  constexpr tuple()
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_nothrow_default_constructible<Type>::value) :
    impl{}{}
  template<VEILER_TEMPLE_STRICT_CHECK(std::is_copy_constructible<Type>::value)>
  explicit constexpr tuple(const Type& arg)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_nothrow_copy_constructible<Type>::value) :
    impl(arg){}
  template<typename UType,
  VEILER_TEMPLE_STRICT_CHECK(std::is_constructible<Type, UType&&>::value)>
  explicit constexpr tuple(UType&& arg)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_nothrow_constructible<Type, UType&&>::value) :
    impl(std::forward<UType>(arg)){}
  tuple(const tuple& t) = default;
  tuple(tuple&& t) = default;
  template<typename UType,
  VEILER_TEMPLE_STRICT_CHECK(std::is_constructible<Type, const UType&>::value)>
  constexpr tuple(const tuple<UType>& u)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_nothrow_constructible<Type, const UType&>::value) :
    impl(tuple<UType>::template get<0>(u)){}
  template<typename UType,
  VEILER_TEMPLE_STRICT_CHECK(std::is_constructible<Type, UType&&>::value)>
  constexpr tuple(tuple<UType>&& u)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_constructible<Type, UType&&>::value) :
    impl(tuple<UType>::template get<0>(std::forward<tuple<UType>>(u))){}
  template<VEILER_TEMPLE_STRICT_CHECK(std::is_copy_assignable<Type>::value)>
  tuple& operator=(const tuple& src)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_nothrow_copy_assignable<Type>::value){
      this->impl = src.impl;
      return *this;
  }
  template<VEILER_TEMPLE_STRICT_CHECK(std::is_move_assignable<Type>::value)>
  tuple& operator=(tuple&& src)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_nothrow_move_assignable<Type>::value){
      this->impl = std::forward<impl_type>(src.impl);
      return *this;
  }
  template<typename UType,
  VEILER_TEMPLE_STRICT_CHECK(std::is_assignable<Type&, const UType&>::value)>
  tuple& operator=(const tuple<UType>& src)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_nothrow_constructible<Type, const UType&>::value){
      this->impl = std::forward<impl_type>(tuple(src).impl);
      return *this;
  }
  template<typename UType,
  VEILER_TEMPLE_STRICT_CHECK(std::is_assignable<Type&,UType&&>::value)>
  tuple& operator=(tuple<UType>&& src)
    VEILER_TEMPLE_POSITIVE_NOEXCEPT(std::is_nothrow_constructible<Type, UType&&>::value){
      this->impl = std::forward<impl_type>(tuple(std::forward<tuple<UType>>(src)).impl);
      return *this;
  }
  template<std::size_t N>
  using at = type_at<tuple, N>;
  void swap(tuple& rhs)
    noexcept(noexcept(std::declval<impl_type&>().swap(std::declval<impl_type&>()))){
    return impl.swap(rhs.impl);
  }
  template<std::size_t N>
  static constexpr auto get(tuple& tpl)noexcept
    ->decltype((impl_type::template get<N>(tpl.impl))){
         return impl_type::template get<N>(tpl.impl);
  }
  template<std::size_t N>
  static constexpr auto get(const tuple& tpl)noexcept
    ->decltype((impl_type::template get<N>(tpl.impl))){
         return impl_type::template get<N>(tpl.impl);
  }
  template<std::size_t N>
  static constexpr auto get(tuple&& tpl)noexcept
    ->decltype((impl_type::template get<N>(std::forward<impl_type>(tpl.impl)))){
         return impl_type::template get<N>(std::forward<impl_type>(tpl.impl));
  }
  template<typename Right,
  VEILER_TEMPLE_STRICT_CHECK(std::is_convertible<decltype(impl == std::declval<typename tuple<Right>::impl_type>()),bool>::value)>
  friend constexpr bool operator==(const tuple& lhs, const tuple<Right>& rhs)
    noexcept(noexcept(lhs.impl == rhs.impl)){
               return lhs.impl == rhs.impl;
  }
  template<typename Right,
  VEILER_TEMPLE_STRICT_CHECK(std::is_convertible<decltype(impl != std::declval<typename tuple<Right>::impl_type>()),bool>::value)>
  friend constexpr bool operator!=(const tuple& lhs, const tuple<Right>& rhs)
    noexcept(noexcept(lhs.impl != rhs.impl)){
               return lhs.impl != rhs.impl;
  }
  template<typename Right,
  VEILER_TEMPLE_STRICT_CHECK(std::is_convertible<decltype(impl <  std::declval<typename tuple<Right>::impl_type>()),bool>::value)>
  friend constexpr bool operator< (const tuple& lhs, const tuple<Right>& rhs)
    noexcept(noexcept(lhs.impl <  rhs.impl)){
               return lhs.impl <  rhs.impl;
  }
  template<typename Right,
  VEILER_TEMPLE_STRICT_CHECK(std::is_convertible<decltype(impl >  std::declval<typename tuple<Right>::impl_type>()),bool>::value)>
  friend constexpr bool operator> (const tuple& lhs, const tuple<Right>& rhs)
    noexcept(noexcept(lhs.impl >  rhs.impl)){
               return lhs.impl >  rhs.impl;
  }
  template<typename Right,
  VEILER_TEMPLE_STRICT_CHECK(std::is_convertible<decltype(impl <= std::declval<typename tuple<Right>::impl_type>()),bool>::value)>
  friend constexpr bool operator<=(const tuple& lhs, const tuple<Right>& rhs)
    noexcept(noexcept(lhs.impl <= rhs.impl)){
               return lhs.impl <= rhs.impl;
  }
  template<typename Right,
  VEILER_TEMPLE_STRICT_CHECK(std::is_convertible<decltype(impl >= std::declval<typename tuple<Right>::impl_type>()),bool>::value)>
  friend constexpr bool operator>=(const tuple& lhs, const tuple<Right>& rhs)
    noexcept(noexcept(lhs.impl >= rhs.impl)){
               return lhs.impl >= rhs.impl;
  }
};
template<>
class tuple<>{
  using impl_type = typename make_value_btree<0, type_tuple<>>::type;
  impl_type impl;
 public:
  constexpr tuple() = default;
  constexpr tuple(const tuple&) = default;
  constexpr tuple(tuple&&) = default;
  void swap(tuple&)const noexcept{}
  friend constexpr bool operator==(const tuple& lhs,const tuple&)noexcept{return lhs.impl == lhs.impl;}
  friend constexpr bool operator!=(const tuple& lhs,const tuple&)noexcept{return lhs.impl != lhs.impl;}
  friend constexpr bool operator< (const tuple& lhs,const tuple&)noexcept{return lhs.impl <  lhs.impl;}
  friend constexpr bool operator> (const tuple& lhs,const tuple&)noexcept{return lhs.impl >  lhs.impl;}
  friend constexpr bool operator<=(const tuple& lhs,const tuple&)noexcept{return lhs.impl <= lhs.impl;}
  friend constexpr bool operator>=(const tuple& lhs,const tuple&)noexcept{return lhs.impl >= lhs.impl;}
};


template<std::size_t N, typename... Types>
constexpr auto get(tuple<Types...>& tpl)noexcept
  ->decltype((tuple<Types...>::template get<N>(tpl))){
       return tuple<Types...>::template get<N>(tpl);
}
template<std::size_t N, typename... Types>
constexpr auto get(tuple<Types...>&& tpl)noexcept
  ->decltype((tuple<Types...>::template get<N>(std::forward<tuple<Types...>>(tpl)))){
       return tuple<Types...>::template get<N>(std::forward<tuple<Types...>>(tpl));
}
template<std::size_t N, typename... Types>
constexpr auto get(const tuple<Types...>& tpl)noexcept
  ->decltype((tuple<Types...>::template get<N>(tpl))){
       return tuple<Types...>::template get<N>(tpl);
}

template<long long... Conds, long long... Indices>
constexpr std::size_t calc_index_from_type(index_tuple<Conds...>, index_tuple<Indices...>)noexcept{
  return static_cast<std::size_t>(make_operate_tuple<long long>(Conds*Indices...).add_());
}

template<typename T, typename... Types,
VEILER_TEMPLE_STRICT_CHECK(make_operate_tuple<long long>(std::is_same<T,Types>::value...).add_() == 1)>
constexpr auto get(tuple<Types...>& tpl)noexcept
  ->decltype((tuple<Types...>::template get<calc_index_from_type(index_tuple<std::is_same<T, Types>::value...>{}, make_indexes<Types...>{})>(tpl))){
       return tuple<Types...>::template get<calc_index_from_type(index_tuple<std::is_same<T, Types>::value...>{}, make_indexes<Types...>{})>(tpl);
}
template<typename T, typename... Types,
VEILER_TEMPLE_STRICT_CHECK(make_operate_tuple<long long>(std::is_same<T,Types>::value...).add_() == 1)>
constexpr auto get(tuple<Types...>&& tpl)noexcept
  ->decltype((tuple<Types...>::template get<calc_index_from_type(index_tuple<std::is_same<T, Types>::value...>{}, make_indexes<Types...>{})>
                                          (std::forward<tuple<Types...>>(tpl)))){
       return tuple<Types...>::template get<calc_index_from_type(index_tuple<std::is_same<T, Types>::value...>{}, make_indexes<Types...>{})>
                                          (std::forward<tuple<Types...>>(tpl));
}
template<typename T, typename... Types,
VEILER_TEMPLE_STRICT_CHECK(make_operate_tuple<long long>(std::is_same<T,Types>::value...).add_() == 1)>
constexpr auto get(const tuple<Types...>& tpl)noexcept
  ->decltype((tuple<Types...>::template get<calc_index_from_type(index_tuple<std::is_same<T, Types>::value...>{}, make_indexes<Types...>{})>(tpl))){
       return tuple<Types...>::template get<calc_index_from_type(index_tuple<std::is_same<T, Types>::value...>{}, make_indexes<Types...>{})>(tpl);
}


template<typename... Types>
void swap(tuple<Types...>& lhs, tuple<Types...>& rhs)
  noexcept(noexcept(lhs.swap(rhs))){
                    lhs.swap(rhs);
}


template<typename... Args>
constexpr tuple<Args&&...> forward_as_tuple(Args&&... args)noexcept{
  return tuple<Args&&...>(std::forward<Args>(args)...);
}


template<typename U>struct expand_reference_wrapper{using type = U;};
template<typename X>struct expand_reference_wrapper<std::reference_wrapper<X>>{using type = X&;};
template<typename X>struct expand_reference_wrapper<veiler::refil<X>>{using type = X&;};

template<typename... Args>
constexpr auto make_tuple(Args&&... args)
  VEILER_TEMPLE_POSITIVE_NOEXCEPT(make_operate_tuple<bool>(std::is_nothrow_constructible<
                                            typename expand_reference_wrapper<typename std::decay<Args>::type>::type,
                                            Args&&
                                          >::value...).and_())
       ->tuple<typename expand_reference_wrapper<typename std::decay<Args>::type>::type...>{
  return tuple<typename expand_reference_wrapper<typename std::decay<Args>::type>::type...>(std::forward<Args>(args)...);
}


namespace{

struct _ignore_t{
  constexpr _ignore_t(){}
  template<typename T>
  constexpr const _ignore_t& operator=(T&&)const noexcept{return *this;}
}constexpr ignore;


class _apply_impl_t{
  template<typename F>
  class curried_apply{
    F f;
    template<typename Func, typename Tuple, long long... Indices>
    static constexpr auto impl(Func&& f, Tuple&& tup, index_tuple<Indices...>)
      ->decltype((std::forward<Func>(f)(get<Indices>(std::forward<Tuple>(tup))...))){
      return      std::forward<Func>(f)(get<Indices>(std::forward<Tuple>(tup))...);
    }
   public:
    constexpr curried_apply(F&& f):f(std::forward<F>(f)){}
    template<typename Tuple>
    constexpr auto operator()(Tuple&& tup)const
      ->decltype((impl(f, std::forward<Tuple>(tup), make_index_range<0, static_cast<long long>(tuple_size<Tuple>())>{}))){
           return impl(f, std::forward<Tuple>(tup), make_index_range<0, static_cast<long long>(tuple_size<Tuple>())>{});
    }
    template<typename Tuple>
    friend constexpr auto operator|(const curried_apply<F>& f, Tuple&& tup)
      ->decltype((f(std::forward<Tuple>(tup)))){
         return   f(std::forward<Tuple>(tup));
    }
  };
 public:
  constexpr _apply_impl_t(){}
  template<typename F, typename Tuple, long long... Indices>
  constexpr auto impl(F&& f, Tuple&& tup, index_tuple<Indices...>)const
  ->decltype((std::forward<F>(f)(get<Indices>(std::forward<Tuple>(tup))...))){
       return std::forward<F>(f)(get<Indices>(std::forward<Tuple>(tup))...);
  }
  template<typename F, typename Tuple>
  constexpr auto operator()(F&& f, Tuple&& tup)const
  ->decltype((impl(std::forward<F>(f), std::forward<Tuple>(tup), make_index_range<0, static_cast<long long>(tuple_size<Tuple>())>{}))){
       return impl(std::forward<F>(f), std::forward<Tuple>(tup), make_index_range<0, static_cast<long long>(tuple_size<Tuple>())>{});
  }
  template<typename F>
  friend constexpr curried_apply<F> operator|(F&& f, const _apply_impl_t&){
    return curried_apply<F>(std::forward<F>(f));
  }
}constexpr apply;

}


template<typename... Types1, long long... Indices1, typename... Types2, long long... Indices2, typename... Types3, long long... Indices3>
constexpr tuple<Types1..., Types2..., Types3...> tuple_cat_impl_impl(tuple<Types1...>&& t1, index_tuple<Indices1...>,
                                                                     tuple<Types2...>&& t2, index_tuple<Indices2...>,
                                                                     tuple<Types3...>&& t3, index_tuple<Indices3...>){
  return tuple<Types1..., Types2..., Types3...>(get<Indices1>(std::forward<tuple<Types1...>>(t1))...,
                                                get<Indices2>(std::forward<tuple<Types2...>>(t2))...,
                                                get<Indices3>(std::forward<tuple<Types3...>>(t3))...);
}

namespace{

struct _tuple_cat_impl_t{
template<typename... Types, typename... Lefts, typename... Rights>
constexpr auto operator()(value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>&& btree)const
  ->decltype((tuple_cat_impl_impl(value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>::template get<0>
                                    (std::forward<value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>>(btree)),
                                  make_indexes<Types...>{},
                                  value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>::template get<1>
                                    (std::forward<value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>>(btree)),
                                  make_indexes<Lefts...>{},
                                  value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>::template get<2>
                                    (std::forward<value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>>(btree)),
                                  make_indexes<Rights...>{}))){
       return tuple_cat_impl_impl(value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>::template get<0>
                                    (std::forward<value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>>(btree)),
                                  make_indexes<Types...>{},
                                  value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>::template get<1>
                                    (std::forward<value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>>(btree)),
                                  make_indexes<Lefts...>{},
                                  value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>::template get<2>
                                    (std::forward<value_btree<tuple<Types...>, tuple<Lefts...>, tuple<Rights...>>>(btree)),
                                  make_indexes<Rights...>{});
}
template<typename... Types, typename... Lefts>
constexpr auto operator()(value_btree<tuple<Types...>,tuple<Lefts...>>&& btree)const
  ->decltype((tuple_cat_impl_impl(value_btree<tuple<Types...>, tuple<Lefts...>>::template get<0>
                                    (std::forward<value_btree<tuple<Types...>, tuple<Lefts...>>>(btree)),
                                  make_indexes<Types...>{},
                                  value_btree<tuple<Types...>, tuple<Lefts...>>::template get<1>
                                    (std::forward<value_btree<tuple<Types...>, tuple<Lefts...>>>(btree)),
                                  make_indexes<Lefts...>{},
                                  tuple<>{},
                                  index_tuple<>{}))){
       return tuple_cat_impl_impl(value_btree<tuple<Types...>, tuple<Lefts...>>::template get<0>
                                    (std::forward<value_btree<tuple<Types...>, tuple<Lefts...>>>(btree)),
                                  make_indexes<Types...>{},
                                  value_btree<tuple<Types...>, tuple<Lefts...>>::template get<1>
                                    (std::forward<value_btree<tuple<Types...>, tuple<Lefts...>>>(btree)),
                                  make_indexes<Lefts...>{},
                                  tuple<>{},
                                  index_tuple<>{});
}
template<typename... Types>
constexpr tuple<Types...>&& operator()(value_btree<tuple<Types...>>&& btree)const noexcept{
  return std::forward<tuple<Types...>>(value_btree<tuple<Types...>>::template get<0>(std::forward<value_btree<tuple<Types...>>>(btree)));
}
constexpr tuple<> operator()(value_btree<>)const noexcept{return tuple<>{};}
template<typename Dummy1, typename Dummy2, typename... Types, typename... Lefts, typename... Rights>
constexpr auto operator()(value_btree<Dummy1, Dummy2, tuple<Types...>, value_btree<Lefts...>, value_btree<Rights...>>&& btree)const
  ->decltype((value_btree<Dummy1, Dummy2, tuple<Types...>, value_btree<Lefts...>, value_btree<Rights...>>::btree_apply(
                std::forward<value_btree<Dummy1, Dummy2, tuple<Types...>, value_btree<Lefts...>, value_btree<Rights...>>>(btree),
                *this))){
       return value_btree<Dummy1, Dummy2, tuple<Types...>, value_btree<Lefts...>, value_btree<Rights...>>::btree_apply(
                std::forward<value_btree<Dummy1, Dummy2, tuple<Types...>, value_btree<Lefts...>, value_btree<Rights...>>>(btree),
                *this);
}
constexpr _tuple_cat_impl_t(){}
}constexpr tuple_cat_impl;

}

template<typename... Types>
constexpr tuple<Types...>&& convert_to_tuple(tuple<Types...>&& tpl)noexcept{return std::forward<tuple<Types...>>(tpl);}
template<typename U1, typename U2>
constexpr tuple<U1, U2> convert_to_tuple(std::pair<U1, U2>&& pair)
  noexcept(std::is_nothrow_constructible<tuple<U1, U2>, std::pair<U1, U2>&&>::value){
  return tuple<U1, U2>(std::forward<std::pair<U1, U2>>(pair));
}
template<typename T, std::size_t N, long long... Indices, typename... Types>
constexpr tuple<Types...> convert_array_to_tuple_impl(std::array<T, N>&& array, index_tuple<Indices...>, type_tuple<Types...>)
  noexcept(std::is_nothrow_constructible<tuple<Types...>, Types&&...>::value){
  return tuple<Types...>(std::forward<T>(array[Indices])...);
}
template<typename T, std::size_t N>
constexpr auto convert_to_tuple(std::array<T, N>&& array)
  noexcept(noexcept(convert_array_to_tuple_impl(std::forward<std::array<T, N>>(array),
                                       make_index_range<0, N, 1>{},
                                       make_type_tuple<N, T>{})))
        ->decltype((convert_array_to_tuple_impl(std::forward<std::array<T, N>>(array),
                                       make_index_range<0, N, 1>{},
                                       make_type_tuple<N,T>{}))){
             return convert_array_to_tuple_impl(std::forward<std::array<T, N>>(array),
                                       make_index_range<0, N, 1>{},
                                       make_type_tuple<N,T>{});
}

template<typename... Tuples>
constexpr auto tuple_cat_(Tuples&&... tpls)
  ->decltype((tuple_cat_impl(typename make_value_btree<sizeof...(Tuples), type_tuple<Tuples...>>::type(std::forward<Tuples>(tpls)...)))){
       return tuple_cat_impl(typename make_value_btree<sizeof...(Tuples), type_tuple<Tuples...>>::type(std::forward<Tuples>(tpls)...));
}

template<typename... TupleLikes>
constexpr auto tuple_cat(TupleLikes&&... tpl_likes)
  noexcept(noexcept(tuple_cat_(convert_to_tuple(std::forward<TupleLikes>(tpl_likes))...)))
        ->decltype((tuple_cat_(convert_to_tuple(std::forward<TupleLikes>(tpl_likes))...))){
             return tuple_cat_(convert_to_tuple(std::forward<TupleLikes>(tpl_likes))...);
}


template<typename... Args>
constexpr tuple<Args&...> tie(Args&... args)noexcept{return tuple<Args&...>(args...);}

}

}



using detail::temple::type_at;
using detail::temple::type_tuple;
using detail::temple::unique_types;
using detail::temple::index_at;
using detail::temple::tuple_size;
using detail::temple::index_size;
using detail::temple::reverse_index_tuple;
using detail::temple::make_index_range;
using detail::temple::make_reverse_indexes;
using detail::temple::make_indexes;
using detail::temple::index_tuple;
using detail::temple::operate_tuple;
using detail::temple::make_operate_tuple;
using detail::temple::tuple;
using detail::temple::swap;
using detail::temple::forward_as_tuple;
using detail::temple::make_tuple;
using detail::temple::ignore;
using detail::temple::apply;
using detail::temple::tuple_cat;
using detail::temple::tie;
using detail::temple::get;

}

#undef VEILER_TEMPLE_POSITIVE_NOEXCEPT
#undef VEILER_TEMPLE_STRICT_CHECK

#endif//VEILER_TEMPLE_HPP_INCLUDED

//Copyright (C) 2014 I
//  Distributed under the Veiler Source License 1.0.