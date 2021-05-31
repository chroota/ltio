#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "base/utils//rand_util.h"
#include "base/time//time_utils.h"
#include "components/boolean_indexer/document.h"
#include "components/boolean_indexer/id_generator.h"
#include "components/boolean_indexer/builder/be_indexer_builder.h"
#include "components/boolean_indexer/index_scanner.h"
#include "components//boolean_indexer/mock//mock_target.h"

//#include "gperftools/profiler.h"

using namespace component;

int main(int argc, char** argv) {
  std::map<int, T*> targets;
  component::BeIndexerBuilder builder;

  for (int i=1; i< 100000; i++) {
    auto a = rand_assigns(10, 0, 100);
    T* t = new T({
      .id = i,
      .a = {"a", rand_assigns(5, 50, 75), base::RandInt(0, 100) > 80},
      .b = {"b", rand_assigns(10, 0, 500), base::RandInt(0, 100) > 80},
      .c = {"c", rand_assigns(8, 10, 170), base::RandInt(0, 100) > 80},
    });

    targets[i] = t;
    Document doc(i);
    doc.AddConjunction(new Conjunction({t->a, t->b, t->c}));
    builder.AddDocument(std::move(doc));
  }
  auto index = builder.BuildIndexer();

  auto none_index_match = [&](QueryAssigns& assigns)-> std::set<int32_t> {
    std::set<int32_t> result;
    for (const auto& t : targets) {
      int id = t.first;
      if (t.second->match(assigns)) {
        result.insert(id);
      }
    }
    return result;
  };

  std::cout << "finish index build" << std::endl;

  uint64_t start = base::time_us();
  std::vector<QueryAssigns> queries;
  for (int i; i < 1000; i++) {
    QueryAssigns assigns = {
      {"a", rand_assigns(1, 50, 100)},
      {"b", rand_assigns(2, 0, 100)},
      {"c", rand_assigns(3, 10, 70)},
    };
    queries.emplace_back(assigns);
  }
  //ProfilerStart("profile.prof");
  for (int i = 0; i < queries.size(); i++) {
    auto scanner = IndexScanner(index.get());
    auto result = scanner.Retrieve(queries[i]);
  }
  //ProfilerStop();
  std::cout << "spend:" << (base::time_us() - start) / 1000 << "(ms)" << std::endl;
}
