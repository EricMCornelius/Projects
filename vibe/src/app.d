import vibe.d;
import vibe.http.server;

void index(HttpServerRequest req, HttpServerResponse res)
{
  render!("index.dt")(res);
}

void page(HttpServerRequest req, HttpServerResponse res)
{
  string page = req.params["page"];
  std.stdio.writeln(page);
  render!("page.dt", page)(res);
}

shared static this()
{
  auto router = new UrlRouter;
  router.get("/", &index);
  router.get("/:page", &page);

  auto settings = new HttpServerSettings;
  settings.port = 9090;
 
  listenHttp(settings, router);
}
