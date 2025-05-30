/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <unifex/delay.hpp>
#include <unifex/for_each.hpp>
#include <unifex/inplace_stop_token.hpp>
#include <unifex/range_stream.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/timed_single_thread_context.hpp>
#include <unifex/typed_via_stream.hpp>
#include <unifex/scheduler_concepts.hpp>
#include <unifex/then.hpp>
#include <unifex/stop_when.hpp>

#include <chrono>
#include <cstdio>
#include <thread>

using namespace unifex;

int main(void) {
  using namespace std::chrono;

  timed_single_thread_context context;

  auto startTime = steady_clock::now();

  sync_wait(
      stop_when(
          for_each(
              delay(range_stream{0, 100}, context.get_scheduler(), 100ms),
              [startTime](int value) {
                auto ms = duration_cast<milliseconds>(steady_clock::now() - startTime);
                std::printf("[%i ms] %i\n", (int)ms.count(), value);
              }),
          then(
            schedule_after(context.get_scheduler(), 500ms),
            [] { std::printf("cancelling\n"); })));

  return 0;
}
