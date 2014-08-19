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

void term_handler(client* c, client::connection_ptr ptr)
{
	std::cout << "terminated..." << std::endl;
}

void configure_con(client* c, websocketpp::connection_hdl hdl) {
	// Do all our necessary configurations before attempting to connect
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	std::cout << "Connection Attempt: " << con->m_attempt_count << std::endl;
	
	con->set_open_handler(bind(&on_open,c,::_1));
	con->set_termination_handler(bind(&term_handler,c,::_1));
}

int main(int argc, char* argv[])
{
	std::string uri("ws://echo.websocket.org");
	
	if (argc == 2) {
	    uri = argv[1];
	} else {
		std::cout << "No Uri provided in argument, defaulting to 'ws://echo.websocket.org'" << std::endl;
	}
	
	std::cout << "Setting up connection to attempt to connect to: " << uri << std::endl;
	
	// Remove superfluous logging
	//test_client.set_access_channels(websocketpp::log::alevel::all);
    //test_client.set_error_channels(websocketpp::log::elevel::all);
        
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
