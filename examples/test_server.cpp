#include "test_server.hpp"

#include <iostream>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

test_server::test_server() {
	// Clear all logging
	m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
	m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
	
	// Allow the endpoint address to be reused (only for this example,
	// because we might run back to back servers quickly, and the endpoint addr
	// might not be completely freed
	m_endpoint.set_reuse_addr(true);
	
	// Initialize the asio
	m_endpoint.init_asio();
	
	m_endpoint.set_open_handler(bind(&test_server::on_open,this,::_1));
	m_endpoint.set_message_handler(bind(&test_server::on_message,this,::_1,::_2));
	m_endpoint.set_close_handler(bind(&test_server::on_close,this,::_1));
}

test_server::~test_server() {
	stop();
}

void test_server::start(uint16_t port) {
	if(m_endpoint.is_listening())
	{
		websocketpp::lib::error_code ec;
		m_endpoint.stop_listening(ec);
	}
	
	// Reset just in case
	m_endpoint.stop();
	m_endpoint.reset();
	
	
	// Now start up the new one
	m_endpoint.listen(port);
	m_endpoint.start_accept();
	
	// Start the ASIO io_service run loop
	try {
		//thread t(bind(&server::run, &m_endpoint));
		m_ptrThread.reset(new websocketpp::lib::thread(&server::run, &m_endpoint));
	} catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	} catch (websocketpp::lib::error_code e) {
		std::cout << e.message() << std::endl;
	} catch (...) {
		std::cout << "test_server: other exception" << std::endl;
	}
}

void test_server::stop()
{
	std::cout << "Server Shutting down" << std::endl;
	// Stop accepting new connections
	if(m_endpoint.is_listening())
	{
		websocketpp::lib::error_code ec;
		m_endpoint.stop_listening(ec);
		
		if(ec)
		{
			std::cout << "test_server: ecmsg - " << ec.message() << std::endl;
		}
	}
	
	// Loop through all existing connections and close them.
	for(websocketpp::connection_hdl hdl : connections)
	{
		websocketpp::lib::error_code ec;
		m_endpoint.close(hdl, websocketpp::close::status::going_away, "", ec);
		if (ec)
		{
			std::cout << "test_server: Error closing connection " << std::endl;
		}
	}
	// Give a bit of time for the server to close the connections and the messages to propigate to the client
	std::this_thread::sleep_for(std::chrono::seconds(1));
	
	if(m_endpoint.stopped() == false)
	{
		m_endpoint.stop();
	}
	
	if(m_ptrThread && m_ptrThread->joinable())
	{
		// If your application freezes here because this thread is still
		// running, then you still have an active connection that isn't closed.
		// You must close it before ::run will return
		m_ptrThread->join();
	}
}

void test_server::on_open(websocketpp::connection_hdl hdl)
{
	connections.insert(hdl);
}

void test_server::on_close(websocketpp::connection_hdl hdl)
{
	connections.erase(hdl);
}

void test_server::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
	if(msg->get_payload() == "close")
	{
		std::cout << "test_server close received, closing..." << std::endl;
		
		websocketpp::lib::error_code ec;
		m_endpoint.close(hdl, websocketpp::close::status::going_away, "", ec);
		if (ec) {
			std::cout << "test_server > Error closing connection " << std::endl;
		}
	}
	else
	{
		// Echo all unknown message types
		std::cout << "test_server Server echoing message: " << msg->get_payload() << std::endl;
		m_endpoint.send(hdl, msg->get_payload(), msg->get_opcode());
	}
}
