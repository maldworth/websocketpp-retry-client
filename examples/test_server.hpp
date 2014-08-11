#include <set>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

class test_server
{
public:
	test_server();
	
	~test_server();
	
	void start(uint16_t port);
	
	void stop();
	
	void on_open(websocketpp::connection_hdl hdl);
	
	void on_close(websocketpp::connection_hdl hdl);
	
	void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
	
private:
	typedef std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> con_list;
	
	con_list connections;
	server m_endpoint;
	websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_ptrThread;
};
