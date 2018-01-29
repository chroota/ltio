#ifndef EXAMPLE_APP_CONTENT_H
#define EXAMPLE_APP_CONTENT_H

#include "app_content.h"

#include <memory>
#include "net/server.h"
#include <thirdparty/json/json.hpp>
#include <net/clients/client_router.h>

using json = nlohmann::json;

namespace content {

class GeneralServerApp : public App,
                         public net::SrvDelegate {
public:
  GeneralServerApp();
  ~GeneralServerApp();

  void ContentMain() override;

  void BeforeApplicationRun() override {
  };
  void AfterApplicationRun() override {
  };
protected:
  void LoadConfig();
private:
  json manifest_;
  std::shared_ptr<net::Server> server_;

  //typedef std::vector<RefClientRouter> ClientRouterList;
  std::map<std::string, net::RefClientRouter> clients_;
};

}
#endif
