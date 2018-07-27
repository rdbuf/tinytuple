#pragma once

#include <utility>
#include <cstdint>
#include <functional>
#include <type_traits>

namespace tiny {
namespace impl {
using std::size_t;
using std::conditional_t, std::declval;

template<class fun, class... Ts>
using call = typename fun::template result<Ts...>;

template<size_t>
struct nat {};

template<template<class...> class eager>
struct cfe {
	template<class... Ts>
	using result = eager<Ts...>;
};

template<class...>
struct parameter_pack_wrapper {};

template<class holder, class cont>
struct zip_with_index {
	template<class... Ts, size_t... Indices>
	static auto helper(parameter_pack_wrapper<Ts...>, std::integer_sequence<size_t, Indices...>)
		-> typename cont::template result<typename holder::template result<nat<Indices>, Ts>...>;
	template<class... Ts>
	using result = decltype(helper(parameter_pack_wrapper<Ts...>(), std::index_sequence_for<Ts...>()));
};

template<class Index, class T>
struct Indexed { T value; };

struct dummy {
	template<class...>
	using result = int;
};

call<zip_with_index<cfe<Indexed>, dummy>, double> a;

template<size_t a, size_t b>
constexpr auto subtract(nat<a>, nat<b>) -> nat<a - b>;

template<class a>
using pred = decltype(subtract(a{}, nat<1>{}));

template<class n, class cont>
struct drop {
	template<class Head, class... Tail>
	struct result_helper {
		using result = call<drop<pred<n>, cont>, Tail...>;
	};

	template<class... Ts>
	using result = typename result_helper<Ts...>::result;
};
template<class cont>
struct drop<nat<0>, cont> {
	template<class... Ts>
	using result = call<cont, Ts...>;
};

template<class n, class cont>
struct rotate_left {
	template<class Head, class... Tail>
	struct result_helper {
		using result = typename rotate_left<pred<n>, cont>::template result<Tail..., Head>;
	};

	template<class... Ts>
	using result = typename result_helper<Ts...>::result;
};
template<class cont>
struct rotate_left<nat<0>, cont> {
	template<class... Ts>
	using result = call<cont, Ts...>;
};

template<class n, class cont>
struct take {
	template<class... Ts>
	using result = call<rotate_left<n, drop<decltype(subtract(nat<sizeof...(Ts)>{}, n{})), cont>>, Ts...>;
};
call<take<nat<4>, dummy>, int, int, int, int, int, int> smth;

template<class cont>
struct head {
	template<class Head, class...>
	struct result_helper { using result = Head; };

	template<class... Ts>
	using result = call<cont, typename result_helper<Ts...>::result>;
};

struct id {
	template<class T>
	struct result_helper {
		using result = T;
	};

	template<class... Ts>
	using result = typename result_helper<Ts...>::result;
};

template<class cont>
struct tail {
	template<class... Ts>
	using result = typename drop<nat<1>, cont>::template result<Ts...>;
};

template<class comparator, class cont>
struct sort {
	struct merge_left_head;
	struct merge_right_head;

	template<class... Ts1, class... Ts2, class... Accum>
	static auto merge(parameter_pack_wrapper<Ts1...>, parameter_pack_wrapper<Ts2...>, parameter_pack_wrapper<Accum...>)
		-> call<conditional_t<
			comparator::template result<call<head<id>, Ts1...>, call<head<id>, Ts2...>>,
			merge_left_head,
			merge_right_head
		>, parameter_pack_wrapper<Ts1...>, parameter_pack_wrapper<Ts2...>, parameter_pack_wrapper<Accum...>>;
	template<class... Ts, class... Accum>
	static auto merge(parameter_pack_wrapper<Ts...>, parameter_pack_wrapper<>, parameter_pack_wrapper<Accum...>)
		-> call<cont, Accum..., Ts...>;
	template<class... Ts, class... Accum>
	static auto merge(parameter_pack_wrapper<>, parameter_pack_wrapper<Ts...>, parameter_pack_wrapper<Accum...>)
		-> call<cont, Accum..., Ts...>;

	struct recur {
		template<class... Ts>
		using result = decltype(merge(
			declval<call<take<nat<sizeof...(Ts) / 2>, sort<comparator, cfe<parameter_pack_wrapper>>>, Ts...>>(),
			declval<call<drop<nat<sizeof...(Ts) / 2>, sort<comparator, cfe<parameter_pack_wrapper>>>, Ts...>>(),
			declval<parameter_pack_wrapper<>>()
		));
	};

	template<class... Ts>
	using result = call<conditional_t<sizeof...(Ts) <= 1, cont, recur>, Ts...>;
};
template<class comparator, class cont>
struct sort<comparator, cont>::merge_left_head {
	template<class... Ts1, class... Ts2, class... Accum>
	static auto helper(parameter_pack_wrapper<Ts1...>, parameter_pack_wrapper<Ts2...>, parameter_pack_wrapper<Accum...>)
		-> decltype(sort::merge(
			declval<call<tail<cfe<parameter_pack_wrapper>>, Ts1...>>(),
			declval<parameter_pack_wrapper<Ts2...>>(),
			declval<parameter_pack_wrapper<Accum..., call<head<id>, Ts1...>>>()
		));

	template<class WrappedTs1, class WrappedTs2, class WrappedAccum>
	using result = decltype(helper(declval<WrappedTs1>(), declval<WrappedTs2>(), declval<WrappedAccum>()));
};
template<class comparator, class cont>
struct sort<comparator, cont>::merge_right_head {
	template<class... Ts1, class... Ts2, class... Accum>
	static auto helper(parameter_pack_wrapper<Ts1...>, parameter_pack_wrapper<Ts2...>, parameter_pack_wrapper<Accum...>)
		-> decltype(sort::merge(
			declval<parameter_pack_wrapper<Ts1...>>(),
			declval<call<tail<cfe<parameter_pack_wrapper>>, Ts2...>>(),
			declval<parameter_pack_wrapper<Accum..., call<head<id>, Ts2...>>>()
		));

	template<class WrappedTs1, class WrappedTs2, class WrappedAccum>
	using result = decltype(helper(declval<WrappedTs1>(), declval<WrappedTs2>(), declval<WrappedAccum>()));
};

// test sort:
struct comp {
	template<class T1, class T2>
	static constexpr bool result = std::alignment_of_v<T1> > std::alignment_of_v<T2>;
};

using std::is_same_v;
static_assert(std::is_same_v<call<sort<comp, cfe<parameter_pack_wrapper>>, bool, int, long int, char>, parameter_pack_wrapper<long int, int, char, bool>>);
static_assert(is_same_v<call<tail<cfe<parameter_pack_wrapper>>, int, double, float>, parameter_pack_wrapper<double, float>>);
static_assert(is_same_v<call<head<id>, int, double, float>, int>);

struct alignment_comparator {
	template<class Idx1, class T1, class Idx2, class T2>
	static auto helper(Indexed<Idx1, T1>, Indexed<Idx2, T2>) ->
		std::integral_constant<bool, (std::alignment_of_v<T1> > std::alignment_of_v<T2>)>;

	template<class T1, class T2>
	static constexpr bool result = decltype(helper(declval<T1>(), declval<T2>()))::value;
};

template<class... Ts>
struct tuple_base : Ts... {};

static_assert(std::is_same_v<call<cfe<tuple_base>, int, float>, tuple_base<int, float>>);

template<class... Ts>
using make_base = call<zip_with_index<cfe<Indexed>, sort<alignment_comparator, cfe<tuple_base>>>, Ts...>;

template<class n, class cont>
struct nth_elem {
	template<class... Ts>
	using result = call<drop<pred<n>, head<cont>>, Ts...>;
};

template<class... Ts>
struct tuple : make_base<Ts...> {
	template<size_t I>
	constexpr auto& get() noexcept {
		return static_cast<Indexed<nat<I>, call<nth_elem<nat<I+1>, id>, Ts...>>*>(this)->value;
	}
	template<size_t I>
	constexpr const auto& get() const noexcept {
		return static_cast<const Indexed<nat<I>, call<nth_elem<nat<I+1>, id>, Ts...>>*>(this)->value;
	}

	template<size_t... idxs>
	constexpr bool equality_helper(std::integer_sequence<size_t, idxs...>, const tuple& that) const noexcept {
		return (... && (this->get<idxs>() == that.get<idxs>()));
	}
	constexpr bool operator==(const tuple& that) const noexcept {
		return equality_helper(std::make_index_sequence<sizeof...(Ts)>(), that);
	}
	constexpr bool operator!=(const tuple& that) const noexcept {
		return !(*this == that);
	}
};
} // namespace impl

template<class... Ts>
using tuple = impl::tuple<Ts...>;
} // namespace tiny

namespace std {
template<size_t I, class... Ts>
constexpr decltype(auto) get(tiny::tuple<Ts...>& t) noexcept {
	return t.template get<I>();
};
template<size_t I, class... Ts>
constexpr decltype(auto) get(const tiny::tuple<Ts...>& t) noexcept {
	return t.template get<I>();
};
template<size_t I, class... Ts>
constexpr decltype(auto) get(tiny::tuple<Ts...>&& t) noexcept {
	return std::move(t.template get<I>());
};
template<size_t I, class... Ts>
constexpr decltype(auto) get(const tiny::tuple<Ts...>&& t) noexcept {
	return std::move(t.template get<I>());
};
} // namespace std