#include "retry_client_endpoint.hpp"

#include <websocketpp/config/asio_no_tls_client.hpp>

#include <iostream>

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
typedef websocketpp::retry_client_endpoint<websocketpp::retry_config<websocketpp::config::asio_client>> client;

websocketpp::lib::shared_ptr<websocketpp::lib::thread> test_thread;
client test_client;

void configure_con(client* c, websocketpp::connection_hdl hdl) {
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	std::cout << "Connection Attempt: " << con->m_attempt_count << std::endl;
}

int main()
{
	// Remove superfluous logging
	test_client.clear_access_channels(websocketpp::log::alevel::all);
    test_client.clear_error_channels(websocketpp::log::elevel::all);
        
	// Normal endpoint setup (just as you would with the regular websocketpp::client)
	test_client.init_asio();
	// The endpoint must be perpetual. TODO look at supporting non perpetual (will have to use .reset())
	test_client.start_perpetual();
	// Start spinning the thread
	test_thread.reset(new websocketpp::lib::thread(&client::run, &test_client));
	
	// Done boilerplate initialization, now our connection code:
	websocketpp::lib::error_code ec;
	client::connection_ptr con = test_client.get_connection("ws://some.fake.server.that.should.not.exist:9001", ec);
	if(ec)
	{
		throw ec;
	}
	// Setup any retry_data settings here
	con->m_retry = true;		// Indicates that we do want to attempt to connect
	con->m_retry_delay = 300;	// Will wait 300ms between attempts
	con->m_max_attempts = 10;	// Will stop attempting to retry after 10 attempts
	
	try
	{
		// Delibrately call connect without setting a configure_handler
		// This shows the importance that when setting handlers such as
		// open, message, closed... handlers it must be done within the
		// configure handler because each retry attempt creates a new
		// connection using get_connection(...)
		test_client.connect(con);
    } catch (const websocketpp::exception & e) {
        std::cout << e.what() << std::endl;
    }
    
    // Must setup a configure handler, where we register specific connection items (eg. handlers, etc)...
	con->set_configure_handler(bind(&configure_con, &test_client, ::_1));
    
    std::cout << "Sleeping for 4 seconds to simulate a server that we cannot connect to" << std::endl;
    std::cout << "and 4 seconds is enough for the 10 retries (@ 300ms delay) to run it's course" << std::endl;
    
    // Now connect will start attempting to connect
    test_client.connect(con);
    
    std::this_thread::sleep_for(std::chrono::seconds(6));
    
    test_client.stop_perpetual();
    test_thread->join();
    return 0;
}
