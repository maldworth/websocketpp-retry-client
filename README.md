websocketpp-retry-client
========================

This library is a header only client endpoint with some built in features for attempting to retry if the websocket server is unreachable, or becomes unreachable.
It's built off of the structure of [client_endpoint.hpp](https://github.com/zaphoyd/websocketpp/blob/0.3.0/websocketpp/roles/client_endpoint.hpp) which is included in the websocketpp library.

Prerequisites
-------------
* [WebSocket++](https://github.com/zaphoyd/websocketpp) dynamically or statically linked in your program (whatever you prefer)
  * Familiar with WebSocket++. This [tutorial](https://github.com/zaphoyd/websocketpp/blob/0.3.0/tutorials/utility_client/utility_client.md) will help, and some of the [examples](https://github.com/zaphoyd/websocketpp/tree/0.3.0/examples)
  * Using a perpetual client (start_perpetual() and stop_perpetual())

2 Minute Tutorial
-----------------
When making the retry-client, I tried to make it as similar to using the provided [client_endpoint.hpp](https://github.com/zaphoyd/websocketpp/blob/0.3.0/websocketpp/roles/client_endpoint.hpp) to minimize any learning curve. Below is a simple retry client which will attempt to connect to a websocket server, and retry or fail.
This example is very similar to example4.cpp.

```cpp
#include "retry_client_endpoint.hpp"

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <iostream>

typedef websocketpp::retry_client_endpoint<websocketpp::retry_config<websocketpp::config::asio_client>> client;

websocketpp::lib::shared_ptr<websocketpp::lib::thread> test_thread;
client test_client;
websocketpp::connection_hdl test_con_hdl;

void on_open(client* c, websocketpp::connection_hdl hdl) {
	std::cout << "Connected..." << std::endl;
    test_con_hdl = hdl;
}

void configure_con(client* c, websocketpp::connection_hdl hdl) {
	// Do all our necessary configurations before attempting to connect
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	std::cout << "Connection Attempt: " << con->m_attempt_count << std::endl;
	
	con->set_open_handler(bind(&on_open,c,::_1));
	con->set_termination_handler(bind(&term_handler,c,::_1));
}

int main()
{
	std::string uri("ws://some.websocket.address"); // try replacing with ws://echo.websocket.org
	
	std::cout << "Setting up connection to attempt to connect to: " << uri << std::endl;
	
	// Remove superfluous logging
	test_client.set_access_channels(websocketpp::log::alevel::all);
    test_client.set_error_channels(websocketpp::log::elevel::all);
        
	// Normal endpoint setup (just as you would with the regular websocketpp::client)
	test_client.init_asio();
	
	// The endpoint must be perpetual. TODO look at supporting non perpetual (will have to use .reset())
	test_client.start_perpetual();
	// Start spinning the thread
	test_thread.reset(new websocketpp::lib::thread(&client::run, &test_client));
	
	// Done boilerplate initialization, now our connection code:
	websocketpp::lib::error_code ec;
	client::connection_ptr con = test_client.get_connection(uri, ec);
	
	/// Everything up to here has been standard websocket++ stuff
	/// But now, this is unique settings to tell our endpoint to retry
	con->m_retry = true;		// Indicates that we do want to attempt to retry connecting (if first attempt fails)
	con->m_retry_delay = 400;	// Will wait 400ms between delays
	con->m_max_attempts = 3;	// Only make 3 attempts.
	con->set_configure_handler(bind(&configure_con, &test_client, ::_1));
	
    // Now connect will start attempting to connect
    test_client.connect(con);
    
    // Sleep so there's time to retry if necessary.
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    // It's important to note that we can't use the original connection ptr from above, because
    // when retrying, it must get a new connection each time in the helper class
    // so the actual connection ptr that is successfully connection might be different from
    // the one we started with. You can see in on_open(...) we assign the test_con_hdl
    // which we know is our successfull connection ptr.
    con = test_client.get_con_from_hdl(test_con_hdl, ec);
    if(ec)
    {
		// Couldn't get a connection_ptr from the connection_hdl (weak_ptr), so it either never connected, or connection was already closed.
	}
	else
	{
		con->close(websocketpp::close::status::going_away,"");
		
		// Give a bit of time for the close message to be sent
		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		std::cout << "Connection closed" << std::endl;
	}
    
    test_client.stop_perpetual();
    test_thread->join();
    return 0;
}
```

Retry Configuration
-------------------
The connection_base class that we use as our helper for retries has a few configurable parameters. These are all explained in comments in the header, but I will describe them here as well.
**Parameters**
* ``m_handle_id`` -> User can assign this to an identifier to keep track of their retrying connections. Otherwise it goes unused (stays at value -1)
* ``m_retry`` -> 
  * if set to **false** (default), will behave exactly as websocket++ client_endpoint.hpp
  * otherwise set it to **true**, and it will attempt to retry if a connection cannot be established
* ``m_retry_delay`` -> the amount of time (in milliseconds) to sleep the thread before attempting another connection
* ``m_max_attempts`` -> will only attempt to connect this many times. 0 = infinite
* ``set_configure_handler(...)`` -> this callback is called before each connection attempt. This has to be done, because there are connection specific configuration that the user may want to perform on the connection_ptr, and we have to create a new connection_ptr each attempt, hence losing the old settings. So it's required to configure your settings here.

Examples
--------
They can be built with a simple g++ command, or you can install CMake on your linux box, using your distro's package manager.

The easiest way to build the examples is CMake. After you clone the repository, browse to the root of the repository and run the command:
```chmod +x ./cmake.sh```
Then execute it
```./cmake.sh```
It will create a ./build folder in the root. Browse into the build folder ie. ``cd build/`` and then type ``make``. It should build the examples and to execute them type ``./bin/exampleX`` (replacing X with the example number).
