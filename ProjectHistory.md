# ting project history #

2014-06-24: libting-4.0.0 released. Bug fixes and refactoring. API changed slightly.

2013-08-06: libting-3.0.0 released. Added IPv6 support and Full MacOS X support.

2013-02-21: libting-2.0.0 released. Some API changes.

2012-03-13: libting-1.0.0 released. Now, since the major version is released, the API may not brake in all further 1.x.x, ABI may brake.
  * directory structure refactoring
    * ting/Socket.hpp moved to and split to several files ting/net/`*`
    * ting/File.hpp and ting/FSFile.hpp moved to ting/fs/`*`
  * some bug fixes

2012-02-03: libting-0.11.0 released.
  * ting::File and ting::FSFile
  * moved implementation to .cpp and libting.so
  * Sockets related stuff moved to ting::net namespace, ting::SocketLib renamed to ting::net::Lib
  * ting::net::HostNameResolver class added for DNS lookups, ting::SocketLib::GetHostByName() removed.
  * ting::atomic namespace added providing some primitives for cross platform atomic operations.
  * .pc file added, so ting can now be used with pkg-config
  * some other minor changes

2011-08-15: libting-0.9.0 released. ting::Signal::IsConnected() family of functions added.

2011-05-20: libting-0.8.1 released. Small bug fix was done to make tests pass on 64 bit systems. Tested on 64 bit ubuntu linux. Should work on other 64 bit systems too.

2011-05-20: libting-0.8.0 released. Bug fixes. Timer implementation heavily refactored. ting::Bool removed, use ting::Inited<bool, true/false> instead. Other minor changes.

2011-03-23: libting-0.7.0 released. Bug fixes. Experimental Mac OS support (Thanks to Jose Luis).

2011-02-26: ting-0.6.0 released. Some interface changes.

2011-01-31: ting-0.5.1 released. Bug fix.

2010-12-28: ting-0.5.0 released.

2010-04-07: ting-0.4.2 released.

2010-02-25: ting-0.4 released.

2009-11-06: ting-0.3 released.

2008-11-03: ting-0.2 released. **Note** that its interface slightly differs from ting-0.1.
The most important is that instead of std::auto\_ptr ting now uses its own auto pointer implementation ting::Ptr.