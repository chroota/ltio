/*
 * Copyright 2021 <name of copyright holder>
 * Author: Huan.Gong <gonghuan.dev@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "event.h"

#include <sstream>
#include <vector>

#include "base/utils/string/str_utils.h"

namespace base {

std::string ev2str(const LtEvent& events) {
  std::vector<std::string> evs;
  if (events & LtEv::LT_EVENT_READ)
    evs.emplace_back("read");
  if (events & LtEv::LT_EVENT_WRITE)
    evs.emplace_back("write");
  if (events & LtEv::LT_EVENT_ERROR)
    evs.emplace_back("error");
  if (events & LtEv::LT_EVENT_CLOSE)
    evs.emplace_back("close");
  return StrUtil::Join(evs, "|");
}

};  // namespace base
