// Copyright (c) 2022 Xiaomi Guo
// Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan
// PSL v2. You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
// Mulan PSL v2 for more details.

#pragma once
#include "datalist.hpp"
#include "tasklist.hpp"
#include "type_traits.hpp"

namespace rgm::core {
template <typename T_tasklist, typename T_worker>
struct data {
  using T_tasks = T_tasklist::tasks;
  using T_datalist = typename T_tasklist::data::to<datalist>;

  T_datalist datalist;
  T_worker* p_worker;

  data(auto* p_worker) : datalist(), p_worker(p_worker) {
    traits::for_each<T_tasks>::before(*p_worker);
  }

  ~data() { traits::for_each<T_tasks>::after(*p_worker); }
};
}  // namespace rgm::core
