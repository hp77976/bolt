#pragma once
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

template <typename T>
using sp = std::shared_ptr<T>;

template <typename T1, typename T2>
using umap = std::unordered_map<T1,T2>;

template <typename T1, typename T2>
using map = std::map<T1,T2>;

template <typename T>
using uset = std::unordered_set<T>;

template <typename T>
using set = std::set<T>;