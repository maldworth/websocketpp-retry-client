#include "retry_client_endpoint.hpp"

#include "test_server.hpp"

#include <websocketpp/config/asio_no_tls_client.hpp>

#include <iostream>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
typedef websocketpp::retry_client_endpoint<websocketpp::retry_config<websocketpp::config::asio_client>> client;

client test_client;
websocketpp::lib::shared_ptr<websocketpp::lib::thread> test_thread;
websocketpp::connection_hdl m_hdl;
std::string m_uri = "ws://localhost:9002";

// Handlers used for our client_endpoint
void on_open(client* c, websocketpp::connection_hdl hdl) {
	std::cout << "Connected!!" << std::endl;
    // Now it is safe to use the connection
    m_hdl = hdl;
}

void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "Received a Message: " << msg->get_payload() << std::endl;
}

void on_close(client* c, websocketpp::connection_hdl hdl) {
    std::cout << "Connection closed, attempting to retry to the server..." << std::endl;
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    
    // Reconfigure it to only have 3 attempts.
    con->m_max_attempts = 3;
    
    websocketpp::lib::error_code ec;
	client::connection_ptr new_con = test_client.get_connection(m_uri, ec);
	if(ec)
	{
		throw ec;
	}
    c->connect(con, new_con);
}

void configure_con(client* c, websocketpp::connection_hdl hdl) {
	// Do all our necessary configurations before attempting to connect
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	std::cout << "Connection Attempt: " << con->m_attempt_count << std::endl;
	
	con->set_open_handler(bind(&on_open,c,::_1));
	con->set_message_handler(bind(&on_message,c,::_1,::_2));
	con->set_close_handler(bind(&on_close,c,::_1));
}

// Helper Method
void send_message(client * client_endpoint, std::string const & msg)
{
	client_endpoint->send(m_hdl, msg.c_str(), websocketpp::frame::opcode::text);
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
	client::connection_ptr con = test_client.get_connection(m_uri, ec);
	if(ec)
	{
		throw ec;
	}
	// Setup any retry_data settings here
	con->m_retry = true;		// Indicates that we do want to attempt to retry connecting (if first attempt fails)
	con->m_retry_delay = 500;	// Will wait 500ms between delays
	con->m_max_attempts = 0;	// Never stop attempting to connect
    
    // Must setup a retry handler, where we register specific connection items (eg. handlers, etc)...
	con->set_configure_handler(bind(&configure_con, &test_client, ::_1));
    
    // Now connect will start attempting to connect
    test_client.connect(con);
    
    std::cout << "Sleeping for 2 seconds to simulate a server that we cannot connect to" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    {
		// Now we will start our remote server
		test_server test_server_endpoint;
		test_server_endpoint.start(9002);
		std::cout << "Test server is now started, client should connect shortly..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(2)); // We give a bit of time to connect, because our retry delay was set to 500ms, we give it 2 seconds to be sure that the connection established
	}
	
	// Sleep for a few seconds to allow our client's new max_retry = 3 to exhaust and give up
	std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Now stop and cleanup our client
    test_client.stop_perpetual();
    test_thread->join();
    return 0;
}
