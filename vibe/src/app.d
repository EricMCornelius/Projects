import vibe.d;

void index(HttpServerRequest req, HttpServerResponse res)
{
  res.renderCompat!("index.dt", HttpServerRequest, "req")(req);

  // not recommended alternative, may cause memory corruption due to a DMD bug
  //res.render!("index.dt", req);
}

void page(HttpServerRequest req, HttpServerResponse res)
{
  string page = req.params["page"];
  std.stdio.writeln(page);
  //res.renderCompat!("page.dt", HttpServerRequest, "req")(req);
  res.render!("page.dt", page);
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
