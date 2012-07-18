#include "helper.hpp"

#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/creation_tags.hpp>

struct impl {
  impl()
    : m(boost::interprocess::open_or_create_t(), "test_mutex") 
  { 
    m.lock();
  }

  ~impl() 
  {
    m.unlock();
  }

  boost::interprocess::named_mutex m;
};

mutex_helper::mutex_helper()
 : _impl(new impl()) 
{ 

}

mutex_helper::~mutex_helper() 
{
  delete _impl;
}
