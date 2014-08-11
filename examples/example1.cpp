#include "retry_client_endpoint.hpp"

#include <websocketpp/config/asio_no_tls_client.hpp>

#include <iostream>

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
typedef websocketpp::retry_client_endpoint<websocketpp::retry_config<websocketpp::config::asio_client>> client;

websocketpp::lib::shared_ptr<websocketpp::lib::thread> test_thread;
client test_client;

void on_open(client* c, websocketpp::connection_hdl hdl) {
	std::cout << "connected" << std::endl;
    // Now it is safe to use the connection
    c->send(hdl, "Hello Example 1!", websocketpp::frame::opcode::text);
}

void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "Received Echo: " << msg->get_payload() << std::endl;
    websocketpp::lib::error_code ec;
	c->close(hdl, websocketpp::close::status::going_away, "", ec);
	if (ec)
	{
		std::cout << "Error closing connection " << std::endl;
	}
}

int main()
{
	// Remove superfluous logging
	test_client.clear_access_channels(websocketpp::log::alevel::all);
    test_client.clear_error_channels(websocketpp::log::elevel::all);
        
	// Normal endpoint setup (just as you would with the regular websocketpp::client)
	test_client.init_asio();
	
	test_client.set_message_handler(bind(&on_message,&test_client,::_1,::_2));
	test_client.set_open_handler(bind(&on_open,&test_client,::_1));
	
	// The endpoint must be perpetual. TODO look at supporting non perpetual (will have to use .reset())
	test_client.start_perpetual();
	// Start spinning the thread
	test_thread.reset(new websocketpp::lib::thread(&client::run, &test_client));
	
	// Done boilerplate initialization, now our connection code:
	websocketpp::lib::error_code ec;
	client::connection_ptr con = test_client.get_connection("ws://echo.websocket.org", ec);
	if(ec)
	{
		throw ec;
	}
	
	
	
    // Now connect will start attempting to connect
    test_client.connect(con);
    
    // Sleep so there's time for the echo to go out and come back
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    test_client.stop_perpetual();
    test_thread->join();
    return 0;
}
