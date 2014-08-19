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
TODO...
