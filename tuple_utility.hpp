#include <tuple>
#include <type_traits>
#include <utility>

namespace tuple_utility {

// is_tuple
//******************************

template <typename T> struct is_tuple_impl {
  static constexpr auto value = false;
};

template <typename... T> struct is_tuple_impl<std::tuple<T...>> {
  static constexpr auto value = true;
};

template <typename T> constexpr bool is_tuple = is_tuple_impl<T>::value;

//******************************

// concepts
//******************************
template <typename T>
concept has_value = requires() { T::value; };

template <typename Tpl>
concept tuple = requires() { requires is_tuple<Tpl>; };
//******************************

// transform
//******************************
template <tuple Tpl, template <typename...> typename Elm> struct transform;

template <typename... T, template <typename...> typename Elm>
struct transform<std::tuple<T...>, Elm> {
  using type = std::tuple<Elm<T>...>;
};

//******************************

// concat
//******************************
template <tuple Tpl1, tuple Tpl2> struct concat;

template <typename... TplElm1, typename... TplElm2>
struct concat<std::tuple<TplElm1...>, std::tuple<TplElm2...>> {
  using type = std::tuple<TplElm1..., TplElm2...>;
};

//******************************

// push_front
//******************************
template <tuple Tpl, typename... T> struct push_front;

template <typename... Any, typename... T>
struct push_front<std::tuple<Any...>, T...> {
  using type = std::tuple<T..., Any...>;
};
//******************************

// push_back
//******************************
template <tuple Tpl, typename... T> struct push_back;

template <typename... Any, typename... T>
struct push_back<std::tuple<Any...>, T...> {
  using type = std::tuple<Any..., T...>;
};

//******************************

// reduce
//******************************

template <tuple Tpl, template <typename, typename> typename T, typename Init>
struct reduce_impl;

template <typename First, typename Second, typename... Tail,
          template <typename, typename> typename T, typename Init>
struct reduce_impl<std::tuple<First, Second, Tail...>, T, Init> {
  using type = typename reduce_impl<std::tuple<T<First, Second>, Tail...>, T,
                                    Init>::type;
};

template <typename Last, typename Accum,
          template <typename, typename> typename T, typename Init>
struct reduce_impl<std::tuple<Accum, Last>, T, Init> {
  using type = T<Accum, Last>;
};

template <template <typename, typename> typename T, typename Init>
struct reduce_impl<std::tuple<Init>, T, Init> {
  using type = Init;
};

template <tuple Tpl, template <typename, typename> typename T, typename Init>
struct reduce {
  using type =
      typename reduce_impl<typename push_front<Tpl, Init>::type, T, Init>::type;
};

//******************************

// subtuple
//******************************
template <typename Accum, typename Current> struct reducer_subtuple {
  static constexpr auto is_in_range =
      Accum::start_index <= Accum::index &&
      Accum::index < Accum::start_index + Accum::size;

  using elm =
      std::conditional<is_in_range, std::tuple<Current>, std::tuple<>>::type;

  static constexpr auto start_index = Accum::start_index;
  static constexpr auto index = Accum::index + 1;
  static constexpr auto size = Accum::size;

  using type = concat<typename Accum::type, elm>::type;
};

template <int I, int Size> struct reducer_subtuple_init {
  static constexpr auto start_index = I;
  static constexpr auto size = Size;
  static constexpr auto index = 0;
  using type = std::tuple<>;
};

template <tuple Tpl, int I, int Size>
using subtuple =
    reduce<Tpl, reducer_subtuple, reducer_subtuple_init<I, Size>>::type;

//******************************

// concat_all
//******************************
template <typename Accum, typename Current> struct concat_all_impl {
  using type = typename concat<typename Accum::type, Current>::type;
};

struct initial_concat_tuples {
  using type = std::tuple<>;
};

template <typename... T>
using concat_all = typename reduce<std::tuple<T...>, concat_all_impl,
                                   initial_concat_tuples>::type::type;
//******************************

// insert
//******************************
template <tuple Tpl, int I, typename... T> struct insert {
  static_assert(std::tuple_size_v < Tpl >> 0, "cannot insert to empty tuple");
  static_assert(I < std::tuple_size_v<Tpl>, "invalid insert index");
  using type =
      concat_all<typename subtuple<Tpl, 0, I>::type, std::tuple<T...>,
                 typename subtuple<Tpl, I, std::tuple_size_v<Tpl> - I>::type>;
};

//******************************

// reverse
//******************************
template <typename Accum, typename Current> struct reducer_reverse {
  static constexpr auto index = Accum::index + 1;
  static constexpr auto size = Accum::size;
  using tpl = Accum::tpl;
  using elm = std::tuple_element_t<size - index, typename Accum::tpl>;
  using type = push_back<typename Accum::type, elm>::type;
};

template <tuple Tpl> struct reducer_reverse_init {
  static constexpr auto size = std::tuple_size_v<Tpl>;
  static constexpr auto index = 0;
  using tpl = Tpl;
  using type = std::tuple<>;
};

template <tuple Tpl>
using reverse = reduce<Tpl, reducer_reverse, reducer_reverse_init<Tpl>>::type;

//******************************

// fillter
//******************************
template <typename Accum, typename Current> struct reducer_fillter {
  using elm = std::conditional<Accum::template fillter<Current>::value,
                               std::tuple<Current>, std::tuple<>>::type;
  template <typename T> using fillter = Accum::template fillter<T>;
  using type = concat<typename Accum::type, elm>::type;
};

template <template <typename> typename Fillter> struct reducer_fillter_init {
  using type = std::tuple<>;
  template <typename T> using fillter = Fillter<T>;
};

template <tuple Tpl, template <typename> typename Fillter>
using fillter =
    reduce<Tpl, reducer_fillter, reducer_fillter_init<Fillter>>::type;

//******************************

// includes
//******************************

template <typename Accum, typename Current> struct reducer_includes {
  using target = Accum::target;
  static constexpr auto value = Accum::value || std::is_same_v<Current, target>;
};

template <typename T> struct reducer_includes_init {
  static constexpr auto value = false;
  using target = T;
};

template <tuple Tpl, typename T> struct includes;
template <typename... Elm, typename T> struct includes<std::tuple<Elm...>, T> {
  static constexpr auto value = reduce<std::tuple<Elm...>, reducer_includes,
                                       reducer_includes_init<T>>::type::value;
};

//******************************

// some
//******************************
template <has_value Accum, typename Current> struct reducer_some {
  template <typename T> using tester = Accum::template tester<T>;
  static constexpr auto value = Accum::value || tester<Current>::value;
};

template <template <typename> typename T> struct reducer_some_init {
  template <typename U> using tester = T<U>;
  static constexpr auto value = false;
};

template <tuple Tpl, template <typename> typename T> struct some;

template <typename... Elm, template <typename> typename T>
struct some<std::tuple<Elm...>, T> {
  static constexpr auto value = reduce<std::tuple<Elm...>, reducer_some,
                                       reducer_some_init<T>>::type::value;
};

//******************************

// flat_once
//******************************

template <typename Accum, typename Current> struct flat_reducer {
private:
  using elm =
      std::conditional<is_tuple<Current>, Current, std::tuple<Current>>::type;

public:
  using type = concat<typename Accum::type, elm>::type;
};

struct flat_reducer_init {
  using type = std::tuple<>;
};

template <tuple Tpl>
using flat_once = reduce<Tpl, flat_reducer, flat_reducer_init>::type;
//******************************

// flat_all
//******************************
template <typename T> struct tester_is_tuple {
  static constexpr auto value = is_tuple<T>;
};

template <tuple Tpl>
constexpr auto includes_tuple = some<Tpl, tester_is_tuple>::value;

template <typename T, bool all_flatten = true> struct flat_all;

template <typename... Elm> struct flat_all<std::tuple<Elm...>, true> {
private:
  using flatten = std::conditional<includes_tuple<std::tuple<Elm...>>,
                                   typename flat_once<std::tuple<Elm...>>::type,
                                   std::tuple<Elm...>>::type;

  static constexpr auto all_flatten = includes_tuple<flatten>;

public:
  using type = std::conditional<includes_tuple<flatten>,
                                typename flat_all<flatten, all_flatten>::type,
                                flatten>::type;
};
template <typename... Any> struct flat_all<std::tuple<Any...>, false> {
  using type = std::tuple<Any...>;
};

//******************************

// find
//******************************
template <tuple Tpl, template <typename> typename T> struct find {

  static_assert((std::tuple_size_v<typename fillter<Tpl, T>::type>) > 0,
                "no match type found.");
  using type = std::tuple_element<0, typename fillter<Tpl, T>::type>::type;
};

//******************************

// find_index
//******************************
template <tuple Tpl, template <typename> typename T, int index = 0,
          bool found = false>
struct find_index;

template <typename... Elm, template <typename> typename T, int index>
struct find_index<std::tuple<Elm...>, T, index, false> {
private:
  static_assert((std::tuple_size_v<std::tuple<Elm...>>) > index,
                "no match type found.");
  using at = std::tuple_element<index, std::tuple<Elm...>>::type;
  static constexpr auto found = T<at>::value;

public:
  static constexpr auto value =
      found ? index
            : find_index<std::tuple<Elm...>, T, index + 1, found>::value;
};

template <typename... Any, template <typename> typename T, int index>
struct find_index<std::tuple<Any...>, T, index, true> {
  static constexpr auto value = index;
};

//******************************

// assign
//******************************
template <tuple Tpl, template <typename...> typename T> struct assign;

template <typename... Elm, template <typename...> typename T>
struct assign<std::tuple<Elm...>, T> {
  using type = T<Elm...>;
};

//******************************

// infer
//******************************
template <typename T> struct infer;

template <template <typename...> typename T, typename... TT>
struct infer<T<TT...>> {
  using type = std::tuple<TT...>;
};

//******************************

// to_chainable
//******************************
template <tuple Tpl> struct to_chainable;
template <typename... Any> struct to_chainable<std::tuple<Any...>> {
  using type = std::tuple<Any...>;

  template <template <typename...> typename T>
  using transform =
      to_chainable<typename transform<std::tuple<Any...>, T>::type>;

  template <template <typename> typename T>
  using fillter = to_chainable<fillter<type, T>>;

  template <typename Tpl>
  using concat = to_chainable<typename concat<type, Tpl>::type>;

  template <int I, typename Tpl>
  using insert = to_chainable<typename insert<type, I, Tpl>::type>;

  template <typename... T>
  using push_back = to_chainable<typename push_back<type, T...>::type>;

  template <typename... T>
  using push_front = to_chainable<typename push_front<type, T...>::type>;

  template <typename... Tpl>
  using concat_all = to_chainable<typename concat_all<Tpl...>::type>;

  template <int I, int S>
  using subtuple = to_chainable<typename subtuple<type, I, S>::type>;

  using flat_once = to_chainable<typename flat_once<type>::type>;

  using flat_all = to_chainable<typename flat_all<type>::type>;

  using reverse = to_chainable<typename reverse<type>::type>;

  template <typename T> using infer = to_chainable<typename infer<T>::type>;

  // non monoid

  template <template <typename, typename> typename Reducer, typename Init>
  using reduce = reduce<type, Reducer, Init>::type;

  template <template <typename> typename T> using some = some<type, T>;

  template <typename T> using includes = includes<type, T>;

  template <template <typename> typename T> using find = find<type, T>;

  template <template <typename> typename T>
  using find_index = find_index<type, T>;

  template <template <typename...> typename T> using assign = assign<type, T>;
};

//******************************
} // namespace tuple_utility
